#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    class AutoFitImpl
    {
    public:
        AutoFitImpl();
        ~AutoFitImpl();

        void performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud);

    private:
        // Private methods
        std::unique_ptr<ccPointCloud> voxelSample(ccPointCloud* inputCloud);
        void splitPointCloudToSoa(const ccPointCloud* cloud,
                                  DeviceBuffer<float>& d_x,
                                  DeviceBuffer<float>& d_y,
                                  DeviceBuffer<float>& d_z);

        // Member variables
        ccMainAppInterface* app;
        ccPointCloud* selectedCloud;

        // CUDA-related members
        std::unique_ptr<DeviceBuffer<float>> d_pointsX;
        std::unique_ptr<DeviceBuffer<float>> d_pointsY;
        std::unique_ptr<DeviceBuffer<float>> d_pointsZ;
        std::unique_ptr<DeviceBuffer<u8>> d_pointLabels;
        std::unique_ptr<DeviceBuffer<Vec3f>> d_normals;
        std::unique_ptr<DeviceBuffer<u32>> d_normalsCount;
        std::unique_ptr<DeviceBuffer<RdvAxisAccumulator>> d_axisAccumulators;
        std::unique_ptr<DeviceBuffer<Vec3f>> d_ringBuffer;
        std::unique_ptr<DeviceBuffer<u32>> d_ringBufferPosition;
    };

} // namespace Bocari
