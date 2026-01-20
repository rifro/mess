#include <cuda_runtime.h>
#include <cub/cub.cuh>
#include <stdexcept>
#include "config.h"
#include "types.h"

// reduce-by-key op device naar unieke (key, count) paren.
// Input: d_keys_in[N], d_counts_in[N] (vaak counts=1)
// Output: d_keys_out[M], d_counts_out[M], host verkrijgt M via memcpy(d_out_count).
//
// Simpele implementatie: radix-sort → reduceByKey (CUB).

template <typename KeyT, typename ValT>
void device_reduce_by_key(const KeyT* d_keys_in,
const ValT* d_valuesIn,
u32         N,
KeyT*&      d_keys_out,
ValT*&      d_valuesOut,
u32&        M)
{
if (N == 0) {
d_keys_out = nullptr;
d_valuesOut = nullptr;
M = 0;
return;
}

```
KeyT* d_keysSorted = nullptr;
ValT* d_vals_sorted = nullptr;
cudaMalloc(&d_keysSorted, N*sizeof(KeyT));
cudaMalloc(&d_vals_sorted, N*sizeof(ValT));

// Sort input pairs (keys_in, vals_in)
void* d_temp = nullptr;
size_t tempBytes = 0;
cub::DeviceRadixSort::SortPairs(d_temp, tempBytes,
                                d_keys_in, d_keysSorted,
                                d_valuesIn, d_vals_sorted, N);
cudaMalloc(&d_temp, tempBytes);
cub::DeviceRadixSort::SortPairs(d_temp, tempBytes,
                                d_keys_in, d_keysSorted,
                                d_valuesIn, d_vals_sorted, N);
cudaFree(d_temp);

// Allocate output worst case = N
KeyT* d_keys_red = nullptr;
ValT* d_vals_red = nullptr;
u32*  d_M       = nullptr;
cudaMalloc(&d_keys_red,  N*sizeof(KeyT));
cudaMalloc(&d_vals_red,  N*sizeof(ValT));
cudaMalloc(&d_M,        sizeof(u32));
cudaMemset(d_M, 0, sizeof(u32));

// ReduceByKey
cub::DeviceReduce::ReduceByKey(d_temp, tempBytes,
                               d_keysSorted, d_keys_red,
                               d_vals_sorted, d_vals_red,
                               d_M, cub::Equality(), cub::Sum(), N);
cudaMalloc(&d_temp, tempBytes);
cub::DeviceReduce::ReduceByKey(d_temp, tempBytes,
                               d_keysSorted, d_keys_red,
                               d_vals_sorted, d_vals_red,
                               d_M, cub::Equality(), cub::Sum(), N);
cudaFree(d_temp);

// Copy M to host
cudaMemcpy(&M, d_M, sizeof(u32), cudaMemcpyDeviceToHost);

// Resize output to M (optional)
d_keys_out = d_keys_red;
d_valuesOut = d_vals_red;

cudaFree(d_keysSorted);
cudaFree(d_vals_sorted);
cudaFree(d_M);
```

}

extern "C"
void k_reduce_groepen_launch(const u64* d_keys_in,
const u32* d_counts_in,
u32         N_pairs,
u64*&      d_uniqueKeys,
u32*&      d_uniqueCounts,
u32&       M_out)
{
device_reduce_by_key<u64, u32>(d_keys_in, d_counts_in, N_pairs,
d_uniqueKeys, d_uniqueCounts, M_out);
}