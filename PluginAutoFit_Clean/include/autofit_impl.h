#pragma once
#include "includes.h"
#include "strict.h"
#include "rdv_types.h"

namespace Bocari
{
    /**
     * @brief Mapping of a physical axis to its logical role (X, Y, or Z).
     */
    struct AxisMapping {
        char label;
        std::vector<i32>* mmData;
        i32 minVal;
        i32 maxVal;
        u32 range;
    };

    /**
     * @brief Represents a spatially-coherent subset of the point cloud.
     */
    struct Page {
        std::vector<u32> indices;
        u32 rX, rY, rZ;
    };

    class AutoFitImpl
    {
    public:
        AutoFitImpl();
        ~AutoFitImpl();

        void performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud);

    private:
        // Paging and Analysis methods
        void convertToSoaAndAnalyze();
        void sortAxesByRange();
        void generatePagesRecursively(std::vector<u32>& indices, u32 rX, u32 rY, u32 rZ);
        void loadPageToDevice(const Page& page);

        inline i32 toMm(float val, float origin) {
            return static_cast<i32>((val - origin) * 1000.0f + 0.5f);
        }

        // Member variables
        ccMainAppInterface* m_app;
        ccPointCloud* m_selectedCloud;
        CCVector3 m_globalBbMin;

        // Physical buffers (host side)
        std::vector<i32> m_rawMmX;
        std::vector<i32> m_rawMmY;
        std::vector<i32> m_rawMmZ;

        AxisMapping m_axes[3];
        std::vector<Page> m_pages;

        // GPU buffers
        std::unique_ptr<DeviceBuffer<u8>> m_dPageBlockBuffer; // 0.5 GB transport buffer
        std::unique_ptr<DeviceBuffer<u8>> m_dPageBuffer;      // Large VRAM page buffer

        // CUDA-related members for processing
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
