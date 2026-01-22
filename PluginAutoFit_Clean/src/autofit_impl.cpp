#include "PluginAutoFit_Clean/include/autofit_impl.h"
#include "PluginAutoFit_Clean/src/rdv/include/normal_generation.cuh"
#include "PluginAutoFit_Clean/src/rdv/include/rdv_voter.cuh"
#include "ccPointCloud.h"
#include "ccMainAppInterface.h"

#include <memory>

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

        // 1. Subsample the point cloud to a manageable size
        auto subsampledCloud = voxelSample(m_selectedCloud);
        const size_t pointCount = subsampledCloud->size();
        if (pointCount == 0) return;

        // 2. Allocate or resize CUDA buffers
        m_dPointsX->allocate(pointCount);
        m_dPointsY->allocate(pointCount);
        m_dPointsZ->allocate(pointCount);
        m_dPointLabels->allocate(pointCount);
        m_dNormals->allocate(pointCount); // Max possible normals is pointCount

        // 3. Split point cloud into Structure-of-Arrays (SoA) and upload to GPU
        splitPointCloudToSoa(subsampledCloud.get(), *m_dPointsX, *m_dPointsY, *m_dPointsZ);

        // 4. Launch CUDA kernel to generate normals
        k_generateNormals(*m_dPointsX, *m_dPointsY, *m_dPointsZ, *m_dPointLabels, *m_dNormals, *m_dNormalsCount);

        // 5. Prepare for and launch RDV Voter kernel
        m_dAxisAccumulators->allocate(getConfig().m_rdvVoter.m_cacheSize);
        m_dAxisAccumulators->memset(0);

        m_dRingBuffer->allocate(getConfig().m_rdvVoter.m_ringBufferSize);
        m_dRingBufferPosition->memset(0);

        k_adaptiveRdvVoting(*m_dNormals, *m_dNormalsCount, *m_dAxisAccumulators, *m_dRingBuffer, *m_dRingBufferPosition);
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
