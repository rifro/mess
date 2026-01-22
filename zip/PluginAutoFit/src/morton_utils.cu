#include "morton_utils.cuh"
#include "types.cuh"
#include <cub/cub.cuh>
#include <cuda_runtime.h>

global void k_kwantiseerEnKey(const vec3f* __restrict__ points, uint32_t n, kwantisatie q,
                                 uint64_t* __restrict__ keysOut)
{
    uint32_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;

    uint32_t xi, yi, zi;
    kwantiseerPunt(points[i], q, xi, yi, zi);

    uint64_t key = morton3D(xi, yi, zi);
    keysOut[i]  = key;
}

global void k_initIndices(uint32_t* __restrict__ Index, uint32_t n)
{
    uint32_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;
    Index[i] = i;
}

global void k_permuteerPunten(const float3* __restrict__ puntenIn, const uint32_t* __restrict__ indexPerm, uint32_t n,
                                 float3* __restrict__ puntenOut)
{
    uint32_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;

    uint32_t src  = indexPerm[i];
    puntenOut[i] = puntenIn[src];
}

void h_voxeliseerMorton(const DeviceBuffer<float3>& d_points, DeviceBuffer<uint64_t>& d_keys, DeviceBuffer<uint32_t>& d_indices,
                       DeviceBuffer<float3>& d_pointsSorted, size_t n)
{
    kwantisatie q = {{0.0f, 0.0f, 0.0f}}; // Assuming origin is at (0,0,0)

    dim3 bs(256);
    dim3 gs((n + bs.x - 1) / bs.x);

    k_initIndices<<<gs, bs>>>(d_indices.data(), n);
    k_kwantiseerEnKey<<<gs, bs>>>(d_points.data(), n, q, d_keys.data());

    // radix sort
    size_t tempBytes = 0;
    void* d_temp     = nullptr;
    cub::__device__RadixSort::SortPairs(nullptr, tempBytes, d_keys.data(), d_keys.data(), d_indices.data(), d_indices.data(),
                                    n);
    cudaMalloc(&d_temp, tempBytes);
    cub::__device__RadixSort::SortPairs(d_temp, tempBytes, d_keys.data(), d_keys.data(), d_indices.data(), d_indices.data(), n);

    k_permuteerPunten<<<gs, bs>>>(d_points.data(), d_indices.data(), n, d_pointsSorted.data());

    cudaFree(d_temp);
}
