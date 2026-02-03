#include "autofit_impl.h"

namespace Bocari
{
    AutoFitImpl::AutoFitImpl() : m_app(nullptr), m_selectedCloud(nullptr) 
    {
        // Ensure hardware limits are calculated once
        getConfig().hardware.initialize();
        
        // Allocate the transfer block buffer (0.5 GB) on the device
        m_dPageBlockBuffer = std::make_unique<DeviceBuffer<u8>>(getConfig().hardware.pageBlockBufferSize);
    }

    AutoFitImpl::~AutoFitImpl() = default;

    void AutoFitImpl::performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud) 
    {
        m_app = app;
        m_selectedCloud = selectedCloud;
        if (!m_selectedCloud) return;

        // 1. Convert CC AoS to Host SoA in millimeters
        convertToSoaAndAnalyze();

        // 2. Swapping: Sort logical axes so m_axes[0] is the longest (logical X)
        sortAxesByRange();

        // 3. Recursive Page generation (median-split on logical X)
        std::vector<u32> initialIndices(m_selectedCloud->size());
        std::iota(initialIndices.begin(), initialIndices.end(), 0);
        
        // Clear previous pages if any
        m_pages.clear();
        generatePagesRecursively(initialIndices, m_axes[0].range, m_axes[1].range, m_axes[2].range);

        // 4. Allocate the large Page Buffer on GPU for this run
        // Based on the aligned calculation in hardware config
        size_t requiredGpuSize = getConfig().hardware.maxPointsPerPage * 15;
        m_dPageBuffer = std::make_unique<DeviceBuffer<u8>>(requiredGpuSize);

        // 5. Process each page sequentially
        for (const auto& page : m_pages) 
        {
            loadPageToDevice(page);
            
            // TODO: Launch Morton Sort and RDV Kernels
            // These will consume the 15-byte data from m_dPageBuffer
        }
    }

    void AutoFitImpl::convertToSoaAndAnalyze() 
    {
        const u32 count = m_selectedCloud->size();
        CCVector3 bbMax;
        m_selectedCloud->getBoundingBox(m_globalBbMin, bbMax);

        m_rawMmX.resize(count); 
        m_rawMmY.resize(count); 
        m_rawMmZ.resize(count);

        for (u32 i = 0; i < count; ++i) 
        {
            const CCVector3* p = m_selectedCloud->getPoint(i);
            m_rawMmX[i] = toMm(p->x, m_globalBbMin.x);
            m_rawMmY[i] = toMm(p->y, m_globalBbMin.y);
            m_rawMmZ[i] = toMm(p->z, m_globalBbMin.z);
        }

        // Point mappings to the raw physical buffers
        m_axes[0] = { 'x', &m_rawMmX, 0, 0, 0 };
        m_axes[1] = { 'y', &m_rawMmY, 0, 0, 0 };
        m_axes[2] = { 'z', &m_rawMmZ, 0, 0, 0 };

        // Analyze ranges for logical sorting
        for (int i = 0; i < 3; ++i) 
        {
            const auto& data = *(m_axes[i].mmData);
            auto [minIt, maxIt] = std::minmax_element(data.begin(), data.end());
            m_axes[i].minVal = *minIt;
            m_axes[i].maxVal = *maxIt;
            m_axes[i].range  = *maxIt - *minIt;
        }
    }

    void AutoFitImpl::sortAxesByRange() 
    {
        // Sort the structs by range descending (logical X becomes the largest)
        std::sort(std::begin(m_axes), std::end(m_axes), 
            [](const AxisMapping& a, const AxisMapping& b) {
                return a.range > b.range;
            });
    }

    void AutoFitImpl::generatePagesRecursively(std::vector<u32>& indices, u32 rX, u32 rY, u32 rZ) 
    {
        const size_t count = indices.size();
        const float ratio = static_cast<float>(rX) / rY;
        const size_t limit = getConfig().hardware.maxPointsPerPage;

        // Check if page fits VRAM and satisfies Morton's 1:2 aspect ratio
        if (count <= limit && ratio <= 2.0f) 
        {
            m_pages.push_back({std::move(indices), rX, rY, rZ});
            return;
        }

        // Median-split on the logical X axis (m_axes[0])
        auto mid = indices.begin() + count / 2;
        std::nth_element(indices.begin(), mid, indices.end(), [&](u32 a, u32 b) {
            return (*m_axes[0].mmData)[a] < (*m_axes[0].mmData)[b];
        });

        std::vector<u32> left(indices.begin(), mid);
        std::vector<u32> right(mid, indices.end());
        
        // Update ranges for the next recursion depth
        u32 newRX_L = (*m_axes[0].mmData)[left.back()] - (*m_axes[0].mmData)[left.front()];
        u32 newRX_R = (*m_axes[0].mmData)[right.back()] - (*m_axes[0].mmData)[right.front()];

        generatePagesRecursively(left, newRX_L, rY, rZ);
        generatePagesRecursively(right, newRX_R, rY, rZ);
    }

    void AutoFitImpl::loadPageToDevice(const Page& page) 
    {
        const size_t bytesPerPoint = 15;
        const size_t blockSize = getConfig().hardware.pageBlockBufferSize;
        const size_t pointsPerBlock = blockSize / bytesPerPoint;
        
        size_t processed = 0;
        while (processed < page.indices.size()) 
        {
            size_t currentBatch = std::min(pointsPerBlock, page.indices.size() - processed);
            
            // Reuse or allocate a staging buffer on the host (DDR5 handles this easily)
            std::vector<u8> h_staging(currentBatch * bytesPerPoint);

            for (size_t i = 0; i < currentBatch; ++i) 
            {
                u32 idx = page.indices[processed + i];
                size_t offset = i * bytesPerPoint;

                // Packed 15-byte layout: u32(X), u32(Y), u32(Z), u8(Type), u16(ID)
                // Using logical axis pointers to ensure swapped order
                std::memcpy(&h_staging[offset + 0], &(*m_axes[0].mmData)[idx], 4);
                std::memcpy(&h_staging[offset + 4], &(*m_axes[1].mmData)[idx], 4);
                std::memcpy(&h_staging[offset + 8], &(*m_axes[2].mmData)[idx], 4);
                
                h_staging[offset + 12] = 0; // Type placeholder
                u16 idPlaceholder = 0;
                std::memcpy(&h_staging[offset + 13], &idPlaceholder, 2);
            }

            // Transfer batch to the 0.5 GB block buffer on GPU
            m_dPageBlockBuffer->upload(h_staging.data(), currentBatch * bytesPerPoint);
            
            // Final move within VRAM to the large page buffer
            size_t deviceByteOffset = processed * bytesPerPoint;
            cudaMemcpy(m_dPageBuffer->data() + deviceByteOffset, 
                       m_dPageBlockBuffer->data(), 
                       currentBatch * bytesPerPoint, 
                       cudaMemcpyDeviceToDevice);

            processed += currentBatch;
        }
    }
}