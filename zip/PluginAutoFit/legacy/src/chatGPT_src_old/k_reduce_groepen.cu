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
void deviceReduceByKey(const KeyT* d_keysIn, const ValT* d_valuesIn, u32 N, KeyT*& d_keysOut, ValT*& d_valuesOut,
                             u32& M)
{
    if(N == 0)
    {
        d_keysOut = nullptr;
        d_valuesOut = nullptr;
        M          = 0;
        return;
    }

    KeyT* d_keysSorted = nullptr;
    ValT* d_valuesSorted = nullptr;
    cudaMalloc(&d_keysSorted, N * sizeof(KeyT));
    cudaMalloc(&d_valuesSorted, N * sizeof(ValT));

    // Sort input pairs (keys_in, vals_in)
    void*  d_temp     = nullptr;
    size_t tempBytes = 0;
    cub::deviceRadixSort::SortPairs(d_temp, tempBytes, d_keysIn, d_keysSorted, d_valuesIn, d_valuesSorted, N);
    cudaMalloc(&d_temp, tempBytes);
    cub::deviceRadixSort::SortPairs(d_temp, tempBytes, d_keysIn, d_keysSorted, d_valuesIn, d_valuesSorted, N);
    cudaFree(d_temp);

    // Allocate output worst case = N
    KeyT* d_keysReduced= nullptr;
    ValT* d_valuesReduced= nullptr;
    u32*  d_m        = nullptr;
    cudaMalloc(&d_keysReduced, N * sizeof(KeyT));
    cudaMalloc(&d_valuesReduced, N * sizeof(ValT));
    cudaMalloc(&d_m, sizeof(u32));
    cudaMemset(d_m, 0, sizeof(u32));

    // ReduceByKey
    cub::deviceReduce::ReduceByKey(d_temp, tempBytes, d_keysSorted, d_keysReduced, d_valuesSorted, d_valuesReduced, d_m,
                                       cub::Equality(), cub::Sum(), N);
    cudaMalloc(&d_temp, tempBytes);
    cub::deviceReduce::ReduceByKey(d_temp, tempBytes, d_keysSorted, d_keysReduced, d_valuesSorted, d_valuesReduced, d_m,
                                       cub::Equality(), cub::Sum(), N);
    cudaFree(d_temp);

    // Copy M to __host__
    cudaMemcpy(&M, d_m, sizeof(u32), cudaMemcpyDeviceToHost);

    // Resize output to M (optional)
    d_keysOut = d_keysReduced;
    d_valuesOut = d_valuesReduced;

    cudaFree(d_keysSorted);
    cudaFree(d_valuesSorted);
    cudaFree(d_m);
}

extern "C" void k_ReduceGroepenLaunch(const u64* d_keysIn, const u32* d_countsIn, u32 N_pairs, u64*& d_uniqueKeys,
                                        u32*& d_uniqueCounts, u32& M_out)
{
    deviceReduceByKey<u64, u32>(d_keysIn, d_countsIn, N_pairs, d_uniqueKeys, d_uniqueCounts, M_out);
}