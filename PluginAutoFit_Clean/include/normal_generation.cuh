#pragma once

namespace Bocari
{
    /**
     * @brief Launches the CUDA kernel to generate surface normals for a point cloud.
     * @details This host function prepares and launches the normal generation kernel.
     * It sets up the grid and block dimensions based on the total number of points
     * and initializes the output normal counter to zero before launching the kernel.
     * The point cloud data is expected in a Structure-of-Arrays (SoA) format.
     *
     * @param d_pointsX Device buffer with X coordinates of the points.
     * @param d_pointsY Device buffer with Y coordinates of the points.
     * @param d_pointsZ Device buffer with Z coordinates of the points.
     * @param pointCount The total number of points in the cloud, passed by value for efficiency.
     * @param d_pointLabels Output device buffer to store point classifications (e.g., Chaos, Surface).
     * @param d_normals Output device buffer for the calculated normals.
     * @param d_normalsCount An atomic counter on the device for the total number of generated normals.
     */
    void h_generateNormals(
        const DeviceBuffer<float>& d_pointsX,
        const DeviceBuffer<float>& d_pointsY,
        const DeviceBuffer<float>& d_pointsZ,
        u32 pointCount,
        DeviceBuffer<u8>& d_pointLabels,
        DeviceBuffer<Vec3f>& d_normals,
        DeviceBuffer<u32>& d_normalsCount
    );

} // namespace Bocari
