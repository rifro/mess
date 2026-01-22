#include "config.h"
#include "types.h"
#include <cub/cub.cuh>
#include <cudaRuntime.h>
#include <stdexcept>

// reduce-by-key op __device__ naar unieke (key, count) paren.
// Input: d_keys_in[N], d_counts_in[N] (vaak counts=1)
// Output: d_keys_out[M], d_counts_out[M], __host__ verkrijgt M via memcpy(d_out_count).
//
// Simpele implementatie: radix-sort → reduceByKey (CUB).

template <typename KeyT, typename ValT>
void deviceReduceByKey(const KeyT* dKeysIn, const ValT* dValsIn, u32 N, KeyT*& d_keysOut, ValT*& dValsOut,
                             u32& M)
{
    if(N == 0)
    {
        d_keysOut = nullptr;
        dValsOut = nullptr;
        M          = 0;
        return;
    }

    KeyT* dKeysSorted = nullptr;
    ValT* d_valuesSorted = nullptr;
    cudaMalloc(&dKeysSorted, N * sizeof(KeyT));
    cudaMalloc(&d_valuesSorted, N * sizeof(ValT));

    // Sort input pairs (keys_in, vals_in)
    void*  dTemp     = nullptr;
    size_t tempBytes = 0;
    cub::deviceRadixSort::SortPairs(dTemp, tempBytes, dKeysIn, dKeysSorted, dValsIn, d_valuesSorted, N);
    cudaMalloc(&dTemp, tempBytes);
    cub::deviceRadixSort::SortPairs(dTemp, tempBytes, dKeysIn, dKeysSorted, dValsIn, d_valuesSorted, N);
    cudaFree(dTemp);

    // Allocate output worst case = N
    KeyT* d_keysReduced= nullptr;
    ValT* d_valuesReduced= nullptr;
    u32*  dM        = nullptr;
    cudaMalloc(&dKeysRed, N * sizeof(KeyT));
    cudaMalloc(&dValsRed, N * sizeof(ValT));
    cudaMalloc(&dM, sizeof(u32));
    cudaMemset(dM, 0, sizeof(u32));

    // ReduceByKey
    cub::deviceReduce::ReduceByKey(dTemp, tempBytes, dKeysSorted, dKeysRed, d_valuesSorted, dValsRed, dM,
                                       cub::Equality(), cub::Sum(), N);
    cudaMalloc(&dTemp, tempBytes);
    cub::deviceReduce::ReduceByKey(dTemp, tempBytes, dKeysSorted, dKeysRed, d_valuesSorted, dValsRed, dM,
                                       cub::Equality(), cub::Sum(), N);
    cudaFree(dTemp);

    // Copy M to __host__
    cudaMemcpy(&M, dM, sizeof(u32), cudaMemcpyDeviceToHost);

    // Resize output to M (optional)
    d_keysOut = dKeysRed;
    dValsOut = dValsRed;

    cudaFree(dKeysSorted);
    cudaFree(d_valuesSorted);
    cudaFree(dM);
}

extern "C" void k_ReduceGroepenLaunch(const u64* dKeysIn, const u32* d_countsIn, u32 N_pairs, u64*& d_uniqueKeys,
                                        u32*& d_uniqueCounts, u32& M_out)
{
    deviceReduceByKey<u64, u32>(dKeysIn, d_countsIn, N_pairs, d_uniqueKeys, d_uniqueCounts, M_out);
}