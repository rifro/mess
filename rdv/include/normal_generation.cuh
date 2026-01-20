#pragma once
#include "device_buffer.cuh"
#include "rdv_types.h"

namespace Bocari
{
    /**
     * @brief Launches the CUDA kernel to generate surface normals for a point cloud.
     *
     * @param d_pointsX Device buffer with X coordinates of the points.
     * @param d_pointsY Device buffer with Y coordinates of the points.
     * @param d_pointsZ Device buffer with Z coordinates of the points.
     * @param d_pointLabels Device buffer to store the labels for each point (e.g., Noise, Chaos).
     * @param d_normals Output device buffer for the calculated normals.
     * @param d_normalsCount Output device buffer for the total count of generated normals.
     */
    void generateNormals(
        const DeviceBuffer<float>& d_pointsX,
        const DeviceBuffer<float>& d_pointsY,
        const DeviceBuffer<float>& d_pointsZ,
        DeviceBuffer<u8>& d_pointLabels,
        DeviceBuffer<Vec3f>& d_normals,
        DeviceBuffer<u32>& d_normalsCount
    );

} // namespace Bocari
