#include "includes.cuh"

global void k_quantizedKey(const vec3f* restrict punten, u32 n, Quantization q,
                                 u64* restrict keysOut)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;

    u32 xi, yi, zi;
    kwantiseerPunt(punten[i], q, xi, yi, zi);

    u64 key = morton3d(xi, yi, zi);
    keysOut[i]  = key;
}

global void k_initIndices(u32* restrict idx, u32 n)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;
    idx[i] = i;
}

global void k_permutePoints(const float3* restrict puntenIn, const u32* restrict indexPerm, u32 n,
                                 float3* restrict pointsOut)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;

    u32 src  = indexPerm[i];
    pointsOut[i] = puntenIn[src];
}

void h_voxeliseerMorton(const deviceBuffer<float3>& d_points, deviceBuffer<u64>& d_keys, deviceBuffer<u32>& d_indices,
                       deviceBuffer<float3>& d_pointsSorted, sizeT n)
{
    Quantization q = {{0.0f, 0.0f, 0.0f}}; // assuming origin is at (0,0,0)

    dim3 bs(256);
    dim3 gs((n + bs.x - 1) / bs.x);

    k_initIndices<<<gs, bs>>>(d_indices.data(), n);
    k_quantizedKey<<<gs, bs>>>(d_points.data(), n, q, d_keys.data());

    // radix sort
    sizeT tempBytes = 0;
    void* d_temp     = nullptr;
    cub::deviceRadixSort::sortPairs(nullptr, tempBytes, d_keys.data(), d_keys.data(), d_indices.data(), d_indices.data(),
                                    n);
    cudaMalloc(&d_temp, tempBytes);
    cub::deviceRadixSort::sortPairs(d_temp, tempBytes, d_keys.data(), d_keys.data(), d_indices.data(), d_indices.data(), n);

    k_permutePoints<<<gs, bs>>>(d_points.data(), d_indices.data(), n, d_pointsSorted.data());

    cudaFree(d_temp);
}
