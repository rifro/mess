#pragma once

// --- CUDA Runtime ---
#include <cuda_runtime.h>
#include <device_launch_parameters.h> // For IDE support
#include <cub/cub.cuh>                // Central management of CUB

// --- Project Headers ---
#include "cuda_utils.cuh"    // CUDA error checking and helper macros
#include "device_buffer.cuh" // RAII wrapper for cudaMalloc/cudaFree
#include "config.cuh"        // Global constants and device-side config structs
#include "rdv_types.h"       // Types are needed by the function signatures below
#include "vec3.h"

namespace Bocari
{
    // --- Host-callable CUDA Wrappers ---

    /**
     * @brief Launches the CUDA kernel to generate surface normals for a point cloud.
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

    /**
     * @brief Launches the CUDA kernel for adaptive Radial Disk Voting (RDV).
     */
    void h_adaptiveRdvVoting(
        const Vec3f* d_normals,
        const u32* d_normalsCount,
        RdvAxisAccumulator* d_axisAccumulators,
        Vec3f* d_ringBuffer,
        u32* d_ringBufferPosition
    );

} // namespace Bocari
