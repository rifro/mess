#include "includes.cuh"

__global__ void k_quantizedKey(const Vec3f* __restrict__ points, u32 N, Kwantisatie Q, u64* __restrict__ keys_out)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;

    u32 xi, yi, zi;
    kwantiseer_punt(points[i], Q, xi, yi, zi);

    u64 key = morton3D(xi, yi, zi);
    keys_out[i]  = key;
}

__global__ void k_initIndices(u32* __restrict__ Index, u32 N)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;
    Index[i] = i;
}

__global__ void k_permutePoints(const float3* __restrict__ pointsIn, const u32* __restrict__ index_perm, u32 N,
                                   float3* __restrict__ pointsOut)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;

    u32 src       = index_perm[i];
    pointsOut[i] = pointsIn[src];
}

void h_voxelizeMorton(
    const DeviceBuffer<float3>& d_points,
    DeviceBuffer<u64>& d_keys,
    DeviceBuffer<u32>& d_indices,
    DeviceBuffer<float3>& d_pointsSorted,
    size_t N
)
{
    Kwantisatie Q = {{0.0f, 0.0f, 0.0f}}; // Assuming origin is at (0,0,0)

    dim3 bs(256);
    dim3 gs((N + bs.x - 1) / bs.x);

    k_initIndices<<<gs, bs>>>(d_indices.data(), N);
    k_quantizedKey<<<gs, bs>>>(d_points.data(), N, Q, d_keys.data());

    // Radix sort
    size_t tempBytes = 0;
    void*  d_temp     = nullptr;
    cub::DeviceRadixSort::SortPairs(nullptr, tempBytes, d_keys.data(), d_keys.data(), d_indices.data(), d_indices.data(), N);
    cudaMalloc(&d_temp, tempBytes);
    cub::DeviceRadixSort::SortPairs(d_temp, tempBytes, d_keys.data(), d_keys.data(), d_indices.data(), d_indices.data(), N);

    k_permutePoints<<<gs, bs>>>(d_points.data(), d_indices.data(), N, d_pointsSorted.data());

    cudaFree(d_temp);
}
