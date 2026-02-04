#include "autofit_impl.h"
#include "cuda_utils.cuh"
#include "includes.cuh"
#include <numeric>
#include <algorithm>

namespace Bocari
{
    AutoFitImpl::AutoFitImpl() : m_app(nullptr), m_selectedCloud(nullptr) 
    {
        m_dPointsX = std::make_unique<DeviceBuffer<i32>>();
        m_dPointsY = std::make_unique<DeviceBuffer<i32>>();
        m_dPointsZ = std::make_unique<DeviceBuffer<i32>>();
        m_dPointLabels = std::make_unique<DeviceBuffer<u8>>();
        m_dNormals = std::make_unique<DeviceBuffer<Vec3f>>();
        m_dNormalsCount = std::make_unique<DeviceBuffer<u32>>(1);
        m_dAxisAccumulators = std::make_unique<DeviceBuffer<RdvAxisAccumulator>>();
        m_dRingBuffer = std::make_unique<DeviceBuffer<Vec3f>>();
        m_dRingBufferPosition = std::make_unique<DeviceBuffer<u32>>(1);
    }

    AutoFitImpl::~AutoFitImpl() = default;

    void AutoFitImpl::performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud) 
    {
        m_app = app;
        m_selectedCloud = selectedCloud;
        if (!m_selectedCloud) return;

        // 1. Initialize host-side configuration and update GPU constant memory
        getConfig().init();
        h_updateConstantConfig(getConfig());

        // 2. Subsample the point cloud (placeholder for now)
        auto subsampledCloud = voxelSample(m_selectedCloud);
        const u32 pointCount = static_cast<u32>(subsampledCloud->size());
        if (pointCount == 0) return;

        // 3. Allocate CUDA buffers
        m_dPointsX->allocate(pointCount);
        m_dPointsY->allocate(pointCount);
        m_dPointsZ->allocate(pointCount);
        m_dPointLabels->allocate(pointCount);
        m_dNormals->allocate(pointCount);

        // 4. Split and Upload SoA in millimeters
        splitPointCloudToSoa(subsampledCloud.get(), *m_dPointsX, *m_dPointsY, *m_dPointsZ);

        // 5. Normal Generation
        // New normal generation requires a sorted indices buffer
        DeviceBuffer<u32> d_sortedIndices(pointCount);
        std::vector<u32> h_indices(pointCount);
        std::iota(h_indices.begin(), h_indices.end(), 0);
        d_sortedIndices.upload(h_indices.data(), pointCount);

        h_generateNormals(m_dPointsX->data(), m_dPointsY->data(), m_dPointsZ->data(),
                          m_dPointLabels->data(), d_sortedIndices.data(), pointCount,
                          m_dNormals->data(), m_dNormalsCount->data(), pointCount);

        // 6. RDV Voting
        m_dAxisAccumulators->allocate(getConfig().rdvVoter.cacheSize);
        m_dAxisAccumulators->memset(0);

        m_dRingBuffer->allocate(getConfig().rdvVoter.ringBufferSize);
        m_dRingBufferPosition->memset(0);

        h_adaptiveRdvVoting(m_dNormals->data(), m_dNormalsCount->data(),
                            m_dAxisAccumulators->data(), m_dRingBuffer->data(),
                            m_dRingBufferPosition->data());

        // 7. Results collection (placeholder log)
        const u32 axisCount = getConfig().rdvVoter.cacheSize;
        std::vector<RdvAxisAccumulator> h_axisAccumulators(axisCount);
        cudaMemcpy(h_axisAccumulators.data(), m_dAxisAccumulators->data(),
                   axisCount * sizeof(RdvAxisAccumulator), cudaMemcpyDeviceToHost);

        if (m_app)
        {
            m_app->dispToConsole("AutoFit: RDV process complete.", ccMainAppInterface::STD_CONSOLE_MESSAGE);
        }
    }

    std::unique_ptr<ccPointCloud> AutoFitImpl::voxelSample(ccPointCloud* inputCloud)
    {
        // Simple copy as placeholder for actual sampling
        return std::make_unique<ccPointCloud>(*inputCloud);
    }

    void AutoFitImpl::splitPointCloudToSoa(const ccPointCloud* cloud,
                                           DeviceBuffer<i32>& d_x,
                                           DeviceBuffer<i32>& d_y,
                                           DeviceBuffer<i32>& d_z)
    {
        const size_t pointCount = cloud->size();
        std::vector<i32> h_x(pointCount), h_y(pointCount), h_z(pointCount);

        CCVector3 bbMin;
        CCVector3 bbMax;
        cloud->getBoundingBox(bbMin, bbMax);

        for (size_t i = 0; i < pointCount; ++i)
        {
            const CCVector3* p = cloud->getPoint(i);
            h_x[i] = toMm(p->x, bbMin.x);
            h_y[i] = toMm(p->y, bbMin.y);
            h_z[i] = toMm(p->z, bbMin.z);
        }

        d_x.upload(h_x.data(), pointCount);
        d_y.upload(h_y.data(), pointCount);
        d_z.upload(h_z.data(), pointCount);
    }
}
