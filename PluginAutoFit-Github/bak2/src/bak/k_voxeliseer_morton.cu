#include <cub/cub.cuh>
#include <cuda_runtime.h>

#include "morton_utils.cuh"
#include "types.cuh"

__global__ void k_quantizedKey(const Vec3f* __restrict__ points, u32 N, Kwantisatie Q, u32 regioCode,
                                    u64* __restrict__ keys_out)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;

    u32 xi, yi, zi;
    kwantiseer_punt(points[i], Q, xi, yi, zi);

    u64 key = morton_met_regio(xi, yi, zi, regioCode);
    keys_out[i]  = key;
}

__global__ void k_initIndices(u32* __restrict__ Index, u32 N)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;
    Index[i] = i;
}

__global__ void k_permutePoints(const Vec3f* __restrict__ pointsIn, const u32* __restrict__ index_perm, u32 N,
                                   Vec3f* __restrict__ pointsOut)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;

    u32 src       = index_perm[i];
    pointsOut[i] = pointsIn[src];
}

// Host wrapper:

void voxelizeMorton(const Vec3f* d_points, u32 N, Kwantisatie Q, Vec3f* d_pointsPerm)
{
    // alloc keys
    u64* d_keys;
    cudaMalloc(&d_keys, N * sizeof(u64));

    u32* d_index_perm;
    cudaMalloc(&d_index_perm, N * sizeof(u32));

    dim3 bs(256);
    dim3 gs((N + bs.x - 1) / bs.x);

    k_quantizedKey<<<gs, bs>>>(d_points, N, Q, d_keys);
    k_initIndices<<<gs, bs>>>(d_index_perm, N);

    // Radix sort
    size_t tempBytes = 0;
    void*  d_temp     = nullptr;
    cub::DeviceRadixSort::SortPairs(nullptr, tempBytes, d_keys, d_keys, d_index_perm, d_index_perm, N);
    cudaMalloc(&d_temp, tempBytes);
    cub::DeviceRadixSort::SortPairs(d_temp, tempBytes, d_keys, d_keys, d_index_perm, d_index_perm, N);

    k_permutePoints<<<gs, bs>>>(d_points, d_index_perm, N, d_pointsPerm);

    cudaFree(d_keys);
    cudaFree(d_index_perm);
    cudaFree(d_temp);
}
