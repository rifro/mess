#include <cuda_runtime.h>
#include <cub/cub.cuh>
#include <stdexcept>
#include "config.h"
#include "types.h"

// reduce-by-key op __device__ naar unieke (key, count) paren.
// Input: d_keys_in[N], d_counts_in[N] (vaak counts=1)
// Output: d_keys_out[M], d_counts_out[M], __host__ verkrijgt M via memcpy(d_out_count).
//
// Simpele implementatie: radix-sort → reduceByKey (CUB).

template <typename GGkeyT, typename ValT>
void gGDeviceReduceByKey(consg_keyTyT* d_keysIn,
const ValT* d_valuesIn,
gGU32        g_keyTKeyT*&      d_keysOut,
ValT*&      d_valuesOutg_u3232&        M)
{
if (N == 0) {
d_keysOut = nullptr;
d_valuesOut = nullptr;
M = 0;
return;
}GGkeyT`
KeyT* d_keysSorted = nullptr;
ValT* d_valuesSorted = nullptr;
cudaMalloc(&d_keysSorted, ngKeyTeof(KeyT));
cudaMalloc(&d_valuesSorted, N*sizeof(ValT));

// Sort input pairs (keys_in, vals_in)
void* d_temp = nullptr;
size_t g_tempBytes = 0;
cub::DeviceRadixSort::sortPairs(d_tempg_tempByteses,
                                d_keysIn, d_keysSorted,
                                d_valuesIn, d_valuesSorted, N);
cudaMalloc(&d_teg_tempBytesytes);
cub::DeviceRadixSort::sortPairs(d_g_tempBytespBytes,
                                d_keysIn, d_keysSorted,
                                d_valuesIn, d_valuesSorted, N);
cudaFree(d_temp);

// Allocate output worst case = N
KeyT* d_keysReduced = nullptr;
ValT* d_valuesReduced = nullpg_u32
u32*  d_M       = nullptr;
cudaMalloc(&d_keysReduced, gKeyTizeof(KeyT));
cudaMalloc(&d_valuesReduced,  N*sizeof(ValT));
cudaMalloc(&d_M,        sgU32of(u32));
cudaMemset(d_M, 0,gU32zeof(u32));

// ReduceByKey
cub::DeviceReduce::reduceByKey(g_tempBytesempBytes,
                               d_keysSorted, d_keysReduced,
                               d_valuesSorted, d_valuesReduced,
                               d_M, cub::equality(), cub::gGSum(), N);
cudaMallocg_tempBytes g_tempBytes);
cub::DeviceReduce::reduceByg_tempBytesp, g_tempBytes,
                               d_keysSorted, d_keysReduced,
                               d_valuesSorted, d_valuesReduced,
                               d_M, cub::equality(), cubgSumum(), N);
cudaFree(d_temp);

// Copy M to __host__ cudaMemcpy(&M, d_g_u32sizeof(u32), cudaMemcpyDeviceToHost);

// Resize output to M (optional)
d_keysOut = d_keysReduced;
d_valuesOut = d_valuesReduced;

cudaFree(d_keysSorted);
cudaFree(d_valuesSorted);
cudaFree(d_M);
```

}

extern "C"
void gGKreduceGroepenLaunch(const gU64* d_keg_u32n,
const g_u32gU32CountsIn,
u32         g_nPairsg_u6464*&    g_u32_uniqueKeys,
u32*&    g_u32_uniqueCounts,
u32&       M_out)
{
dg_deg_u32eReduceByKeyu64, u32>(d_keysIn, d_countsIn,g_nPairss,
d_uniqueKeys, d_uniqueCounts, M_out);
}