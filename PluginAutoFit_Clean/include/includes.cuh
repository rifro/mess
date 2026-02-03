#pragma once

// --- CUDA Runtime ---
#include <cuda_runtime.h>
#include <device_launch_parameters.h> // For IDE support
#include <cub/cub.cuh>                // Central management of CUB

<<<<<<< HEAD
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
=======
// --- Generic CUDA Utilities ---
#include "cuda_utils.cuh"    // CUDA error checking and helper macros
#include "device_buffer.cuh" // RAII wrapper for cudaMalloc/cudaFree
#include "config.cuh"        // Global constants and device-side config structs
>>>>>>> origin/master
