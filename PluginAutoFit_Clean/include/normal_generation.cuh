#pragma once
#include "rdv_types.h"

namespace Bocari
{
    /**
     * @brief Launches the CUDA kernel to generate surface normals for a point cloud.
     * @details This host dispatcher prepares and launches the normal generation kernel 
     * using a Structure-of-Arrays (SoA) format.
     * It resets the atomic normal counter on the device before execution.
     *
     * Axis swapping is supported by changing the order of the d_x, d_y, and d_z 
     * pointers during the call.
     *
     * @param d_x Device buffer with X coordinates (i32 mm).
     * @param d_y Device buffer with Y coordinates (i32 mm).
     * @param d_z Device buffer with Z coordinates (i32 mm).
     * @param d_types Device buffer for point classification (e.g., Chaos, Duplicate).
     * @param d_sortedIndices Morton-sorted indices for spatial locality.
     * @param pointCount Total number of points to process.
     * @param d_normals Output device buffer for generated unit normals.
     * @param d_normalsCount Atomic counter on the device for the number of generated normals.
     * @param maxNormals Maximum capacity of the d_normals buffer.
     */
    void h_generateNormals(
        const i32* d_x, 
        const i32* d_y, 
        const i32* d_z,
        u8* d_types,
        const u32* d_sortedIndices,
        u32 pointCount,
        Vec3f* d_normals,
        u32* d_normalsCount,
        u32 maxNormals
    );

} // namespace Bocari
