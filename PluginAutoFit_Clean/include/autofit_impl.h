#pragma once
#include "includes.h"
#include "strict.h"
#include "rdv_types.h"

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
                                  DeviceBuffer<i32>& d_x,
                                  DeviceBuffer<i32>& d_y,
                                  DeviceBuffer<i32>& d_z);

        inline i32 toMm(float val, float origin) {
            return static_cast<i32>((val - origin) * 1000.0f + 0.5f);
        }

        // Member variables
        ccMainAppInterface* m_app;
        ccPointCloud* m_selectedCloud;

        // CUDA-related members
        std::unique_ptr<DeviceBuffer<i32>> m_dPointsX;
        std::unique_ptr<DeviceBuffer<i32>> m_dPointsY;
        std::unique_ptr<DeviceBuffer<i32>> m_dPointsZ;
        std::unique_ptr<DeviceBuffer<u8>> m_dPointLabels;
        std::unique_ptr<DeviceBuffer<Vec3f>> m_dNormals;
        std::unique_ptr<DeviceBuffer<u32>> m_dNormalsCount;
        std::unique_ptr<DeviceBuffer<RdvAxisAccumulator>> m_dAxisAccumulators;
        std::unique_ptr<DeviceBuffer<Vec3f>> m_dRingBuffer;
        std::unique_ptr<DeviceBuffer<u32>> m_dRingBufferPosition;
    };

} // namespace Bocari
