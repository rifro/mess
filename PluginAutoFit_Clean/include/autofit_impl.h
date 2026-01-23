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
        ccMainAppInterface* m_app;
        ccPointCloud* m_selectedCloud;

        // CUDA-related members
        std::unique_ptr<DeviceBuffer<float>> m_dPointsX;
        std::unique_ptr<DeviceBuffer<float>> m_dPointsY;
        std::unique_ptr<DeviceBuffer<float>> m_dPointsZ;
        std::unique_ptr<DeviceBuffer<u8>> m_dPointLabels;
        std::unique_ptr<DeviceBuffer<Vec3f>> m_dNormals;
        std::unique_ptr<DeviceBuffer<u32>> m_dNormalsCount;
        std::unique_ptr<DeviceBuffer<RdvAxisAccumulator>> m_dAxisAccumulators;
        std::unique_ptr<DeviceBuffer<Vec3f>> m_dRingBuffer;
        std::unique_ptr<DeviceBuffer<u32>> m_dRingBufferPosition;
    };

} // namespace Bocari
