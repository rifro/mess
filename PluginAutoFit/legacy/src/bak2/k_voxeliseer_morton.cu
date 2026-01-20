#include "includes.cuh"


__global__ void kKwantiseerEnKey(const Vec3f* __restrict__ points, u32 N, Kwantisatie Q, u32 regioCode,
                                    u64* __restrict__ keysOut)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;

    u32 xi, yi, zi;
    kwantiseerPunt(points[i], Q, xi, yi, zi);

    u64 key = mortonMetRegio(xi, yi, zi, regioCode);
    keysOut[i]  = key;
}

__global__ void kInitIndices(u32* __restrict__ Index, u32 N)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;
    Index[i] = i;
}

__global__ void kPermuteerPunten(const Vec3f* __restrict__ puntenIn, const u32* __restrict__ indexPerm, u32 N,
                                   Vec3f* __restrict__ pointsOut)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;

    u32 src       = indexPerm[i];
    pointsOut[i] = puntenIn[src];
}

// Host wrapper:

void voxeliseerMorton(const Vec3f* d_points, u32 N, Kwantisatie Q, Vec3f* d_pointsPerm)
{
    // alloc keys
    u64* d_keys;
    cudaMalloc(&d_keys, N * sizeof(u64));

    u32* dIndexPerm;
    cudaMalloc(&dIndexPerm, N * sizeof(u32));

    dim3 bs(256);
    dim3 gs((N + bs.x - 1) / bs.x);

    kKwantiseerEnKey<<<gs, bs>>>(d_points, N, Q, d_keys);
    kInitIndices<<<gs, bs>>>(dIndexPerm, N);

    // Radix sort
    size_t tempBytes = 0;
    void*  dTemp     = nullptr;
    cub::DeviceRadixSort::SortPairs(nullptr, tempBytes, d_keys, d_keys, dIndexPerm, dIndexPerm, N);
    cudaMalloc(&dTemp, tempBytes);
    cub::DeviceRadixSort::SortPairs(dTemp, tempBytes, d_keys, d_keys, dIndexPerm, dIndexPerm, N);

    kPermuteerPunten<<<gs, bs>>>(d_points, dIndexPerm, N, d_pointsPerm);

    cudaFree(d_keys);
    cudaFree(dIndexPerm);
    cudaFree(dTemp);
}
