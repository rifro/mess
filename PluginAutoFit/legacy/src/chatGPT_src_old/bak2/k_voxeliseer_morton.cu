#include "includes.cuh"

// ============ Kernels ============

// Kwantiseren + morton sleutel (zonder regio-codes voor nu)
__global__ void kKwantiseerEnKey(const Vec3f*** restrict** points, u32 N, Kwantisatie Q, u64*** restrict** keysOut)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;

    Vec3f p = points[i];
    u32   xi, yi, zi;
    kwantiseerPunt(p, Q, xi, yi, zi);

    // Voorlopig geen afzonderlijke regio-code (0)
    u64 key     = mortonMetRegio(xi, yi, zi, /*regio*/ 0u);
    keysOut[i] = key;
}

// Permuteer points volgens index-permutatie
__global__ void kPermuteerPunten(const Vec3f*** restrict** puntenIn, const u32*** restrict** indexPerm, u32 N,
                                   Vec3f*** restrict** pointsOut)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;
    u32 src       = indexPerm[i];
    pointsOut[i] = puntenIn[src];
}

// ============ __host__ Launch ============

static inline void check(cudaErrorT e, const char* where)
{
    if(e != cudaSuccess) throw std::runtimeError(std::string("[CUDA] ") + where + " : " + cudaGetErrorString(e));
}

void k_voxelizeMortonLaunch(const Vec3f* d_points, u32 N, Kwantisatie Q, u64* d_keys,
                                u32* /*d_regioCodes*/, // (niet gebruikt nu)
                                Vec3f* d_pointsPerm, u32* dIndexPerm)
{
    if(N == 0) return;

    // 1) Kwantiseren + morton keys
    dim3 bs(256);
    dim3 gs((N + bs.x - 1) / bs.x);
    kKwantiseerEnKey<<<gs, bs>>>(d_points, N, Q, d_keys);
    check(cudaGetLastError(), "k_quantizedKey");

    // 2) Init index [0..N)
    //    Gebruik thrust of een simpele kernel; we doen snelle CUB-sequence via transform-iterator
    //    Om het simpel te houden: kleine helper kernel hier:
    auto kInitIndices = [] __global__(u32 * Index, u32 n) {
        u32 i = blockIdx.x * blockDim.x + threadIdx.x;
        if(i < n) Index[i] = i;
    };
    kInitIndices<<<gs, bs>>>(dIndexPerm, N);
    check(cudaGetLastError(), "init indices");

    // 3) CUB radix sort (keys, indices)
    void*  dTemp     = nullptr;
    size_t tempBytes = 0;

    cub::deviceRadixSort::SortPairs(dTemp, tempBytes, d_keys, d_keys, dIndexPerm, dIndexPerm, N);
    check(cudaGetLastError(), "cub temp size");

    cudaMalloc(&dTemp, tempBytes);
    cub::deviceRadixSort::SortPairs(dTemp, tempBytes, d_keys, d_keys, dIndexPerm, dIndexPerm, N);
    check(cudaGetLastError(), "cub sort pairs");
    cudaFree(dTemp);

    // 4) Permuteer points → d_pointsPerm (coalesced read met index_perm)
    kPermuteerPunten<<<gs, bs>>>(d_points, dIndexPerm, N, d_pointsPerm);
    check(cudaGetLastError(), "k_permutePoints");

    cudaDeviceSynchronize();
}