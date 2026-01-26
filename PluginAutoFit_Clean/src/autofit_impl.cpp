#include "autofit_impl.h"
#include "config.h"
#include "cuda_utils.cuh"
#include "includes.h"
#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    AutoFitImpl::AutoFitImpl() : m_app(nullptr), m_selectedCloud(nullptr)
    {
        // Initialize smart pointers for CUDA buffers
        m_dPointsX = std::make_unique<DeviceBuffer<float>>(0);
        m_dPointsY = std::make_unique<DeviceBuffer<float>>(0);
        m_dPointsZ = std::make_unique<DeviceBuffer<float>>(0);
        m_dPointLabels = std::make_unique<DeviceBuffer<u8>>(0);
        m_dNormals = std::make_unique<DeviceBuffer<Vec3f>>(0);
        m_dNormalsCount = std::make_unique<DeviceBuffer<u32>>(1);
        m_dAxisAccumulators = std::make_unique<DeviceBuffer<RdvAxisAccumulator>>(0);
        m_dRingBuffer = std::make_unique<DeviceBuffer<Vec3f>>(0);
        m_dRingBufferPosition = std::make_unique<DeviceBuffer<u32>>(1);
    }

    AutoFitImpl::~AutoFitImpl() = default;

    void AutoFitImpl::performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud)
    {
        m_app = app;
        m_selectedCloud = selectedCloud;

        if (!m_selectedCloud) return;

        // 1. Update the constant configuration on the GPU
        h_updateConstantConfig(getConfig());

        // 2. Subsample the point cloud to a manageable size
        auto subsampledCloud = voxelSample(m_selectedCloud);
        const u32 pointCount = subsampledCloud->size();
        if (pointCount == 0) return;

        // 3. Allocate or resize CUDA buffers
        m_dPointsX->allocate(pointCount);
        m_dPointsY->allocate(pointCount);
        m_dPointsZ->allocate(pointCount);
        m_dPointLabels->allocate(pointCount);
        m_dNormals->allocate(pointCount); // Max possible normals is pointCount

        // 4. Split point cloud into Structure-of-Arrays (SoA) and upload to GPU
        splitPointCloudToSoa(subsampledCloud.get(), *m_dPointsX, *m_dPointsY, *m_dPointsZ);

        // 5. Launch CUDA kernel to generate normals
        h_generateNormals(*m_dPointsX, *m_dPointsY, *m_dPointsZ, pointCount, *m_dPointLabels, *m_dNormals, *m_dNormalsCount);

        // 6. Prepare for and launch RDV Voter kernel
        m_dAxisAccumulators->allocate(getConfig().m_rdvVoter.m_cacheSize);
        m_dAxisAccumulators->memset(0);

        m_dRingBuffer->allocate(getConfig().m_rdvVoter.m_ringBufferSize);
        m_dRingBufferPosition->memset(0);

        h_adaptiveRdvVoting(*m_dNormals, *m_dNormalsCount, *m_dAxisAccumulators, *m_dRingBuffer, *m_dRingBufferPosition);

        // 7. Copy results back to the host
        const u32 axisCount = getConfig().m_rdvVoter.m_cacheSize;
        std::vector<RdvAxisAccumulator> h_axisAccumulators(axisCount);
        cudaMemcpy(h_axisAccumulators.data(), m_dAxisAccumulators->data(), axisCount * sizeof(RdvAxisAccumulator), cudaMemcpyDeviceToHost);

        // Placeholder: Log the results to the console to verify data transfer
        if (m_app)
        {
            m_app->dispToConsole("AutoFit: Successfully retrieved results from GPU.", ccMainAppInterface::STD_CONSOLE_MESSAGE);
            for (u32 i = 0; i < axisCount; ++i)
            {
                if (h_axisAccumulators[i].m_voteCount > 0)
                {
                    Vec3f axis = normalize(h_axisAccumulators[i].m_vectorSum);
                    m_app->dispToConsole(QString("  - Axis %1: votes=%2, dir=(%3, %4, %5)")
                                             .arg(i)
                                             .arg(h_axisAccumulators[i].m_voteCount)
                                             .arg(axis.m_x)
                                             .arg(axis.m_y)
                                             .arg(axis.m_z),
                                         ccMainAppInterface::STD_CONSOLE_MESSAGE);
                }
            }
        }
    }

    std::unique_ptr<ccPointCloud> AutoFitImpl::voxelSample(ccPointCloud* inputCloud)
    {
        // This is a simplified stub. A real implementation would perform voxel grid sampling.
        return std::make_unique<ccPointCloud>(*inputCloud);
    }

    void AutoFitImpl::splitPointCloudToSoa(const ccPointCloud* cloud,
                                           DeviceBuffer<float>& d_x,
                                           DeviceBuffer<float>& d_y,
                                           DeviceBuffer<float>& d_z)
    {
        const size_t pointCount = cloud->size();
        std::vector<float> h_x(pointCount), h_y(pointCount), h_z(pointCount);

        for (size_t i = 0; i < pointCount; ++i)
        {
            const CCVector3* p = cloud->getPoint(i);
            h_x[i] = p->x;
            h_y[i] = p->y;
            h_z[i] = p->z;
        }

        cudaMemcpy(d_x.data(), h_x.data(), pointCount * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(d_y.data(), h_y.data(), pointCount * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(d_z.data(), h_z.data(), pointCount * sizeof(float), cudaMemcpyHostToDevice);
    }

} // namespace Bocari
