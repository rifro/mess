#pragma once

// --- CUDA Runtime ---
#include <cuda_runtime.h>

// --- Project Headers ---
#include "config.cuh"
#include "cuda_utils.cuh"
#include "device_buffer.cuh"
#include "rdv_types.h" // Types are needed by the function signatures below
#include "vec3.h"

namespace Bocari
{
    // --- Host-callable CUDA Wrappers ---

    /**
     * @brief Launches the CUDA kernel to generate surface normals for a point cloud.
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

    /**
     * @brief Launches the CUDA kernel for adaptive Radial Disk Voting (RDV).
     */
    void h_adaptiveRdvVoting(
        const DeviceBuffer<Vec3f>& d_normals,
        const DeviceBuffer<u32>& d_normalsCount,
        DeviceBuffer<RdvAxisAccumulator>& d_axisAccumulators,
        DeviceBuffer<Vec3f>& d_ringBuffer,
        DeviceBuffer<u32>& d_ringBufferPosition
    );

} // namespace Bocari
