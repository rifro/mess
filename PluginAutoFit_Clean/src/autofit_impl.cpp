#include "autofit_impl.h"
#include "config.h"
#include "cuda_utils.cuh"
#include "includes.h"
#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    AutoFitImpl::AutoFitImpl() : app(nullptr), selectedCloud(nullptr)
    {
        // Initialize smart pointers for CUDA buffers
        d_pointsX = std::make_unique<DeviceBuffer<float>>(0);
        d_pointsY = std::make_unique<DeviceBuffer<float>>(0);
        d_pointsZ = std::make_unique<DeviceBuffer<float>>(0);
        d_pointLabels = std::make_unique<DeviceBuffer<u8>>(0);
        d_normals = std::make_unique<DeviceBuffer<Vec3f>>(0);
        d_normalsCount = std::make_unique<DeviceBuffer<u32>>(1);
        d_axisAccumulators = std::make_unique<DeviceBuffer<RdvAxisAccumulator>>(0);
        d_ringBuffer = std::make_unique<DeviceBuffer<Vec3f>>(0);
        d_ringBufferPosition = std::make_unique<DeviceBuffer<u32>>(1);
    }

    AutoFitImpl::~AutoFitImpl() = default;

    void AutoFitImpl::performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud)
    {
        this->app = app;
        this->selectedCloud = selectedCloud;

        if (!this->selectedCloud) return;

        // 1. Initialize host-side configuration (pre-calculates squared values)
        getConfig().init();

        // 2. Update the constant configuration on the GPU
        h_updateConstantConfig(getConfig());

        // 3. Subsample the point cloud to a manageable size
        auto subsampledCloud = voxelSample(this->selectedCloud);
        const u32 pointCount = subsampledCloud->size();
        if (pointCount == 0) return;

        // 4. Allocate or resize CUDA buffers
        d_pointsX->allocate(pointCount);
        d_pointsY->allocate(pointCount);
        d_pointsZ->allocate(pointCount);
        d_pointLabels->allocate(pointCount);
        d_normals->allocate(pointCount);

        // 5. Split point cloud into Structure-of-Arrays (SoA) and upload to GPU
        splitPointCloudToSoa(subsampledCloud.get(), *d_pointsX, *d_pointsY, *d_pointsZ);

        // 6. Launch CUDA kernel to generate normals
        h_generateNormals(*d_pointsX, *d_pointsY, *d_pointsZ, pointCount, *d_pointLabels, *d_normals, *d_normalsCount);

        // 7. Prepare for and launch RDV Voter kernel
        d_axisAccumulators->allocate(getConfig().rdvVoter.cacheSize);
        d_axisAccumulators->memset(0);

        d_ringBuffer->allocate(getConfig().rdvVoter.ringBufferSize);
        d_ringBufferPosition->memset(0);

        h_adaptiveRdvVoting(*d_normals, *d_normalsCount, *d_axisAccumulators, *d_ringBuffer, *d_ringBufferPosition);

        // 8. Copy results back to the host
        const u32 axisCount = getConfig().rdvVoter.cacheSize;
        std::vector<RdvAxisAccumulator> h_axisAccumulators(axisCount);
        cudaMemcpy(h_axisAccumulators.data(), d_axisAccumulators->data(), axisCount * sizeof(RdvAxisAccumulator), cudaMemcpyDeviceToHost);

        // Placeholder: Log the results to the console
        if (this->app)
        {
            this->app->dispToConsole("AutoFit: RDV process complete. Results:", ccMainAppInterface::STD_CONSOLE_MESSAGE);
            for (u32 i = 0; i < axisCount; ++i)
            {
                // Display only axes that have accumulated significant weight
                if (h_axisAccumulators[i].voteWeightSum > 1.0f)
                {
                    Vec3f axisDir = normalize(h_axisAccumulators[i].axis);
                    this->app->dispToConsole(QString("  - Axis %1: weight=%2, dir=(%3, %4, %5)")
                                             .arg(i)
                                             .arg(h_axisAccumulators[i].voteWeightSum)
                                             .arg(axisDir.x)
                                             .arg(axisDir.y)
                                             .arg(axisDir.z),
                                         ccMainAppInterface::STD_CONSOLE_MESSAGE);
                }
            }
        }
    }

    std::unique_ptr<ccPointCloud> AutoFitImpl::voxelSample(ccPointCloud* inputCloud)
    {
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
