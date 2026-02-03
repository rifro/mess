#include "morton_generator.cuh"
#include "includes.h"

namespace Bocari
{
    // Expands a 21-bit integer by inserting 2 zeros between each bit for 3D Morton encoding
    __device__ inline u64 expandBits(u64 v)
    {
        v &= 0x1fffff;
        v = (v | v << 32) & 0x1f00000000ffffULL;
        v = (v | v << 16) & 0x1f0000ff0000ffULL;
        v = (v | v << 8)  & 0x100f00f00f00f00fULL;
        v = (v | v << 4)  & 0x10c30c30c30c30c3ULL;
        v = (v | v << 2)  & 0x1249249249249249ULL;
        return v;
    }

    __global__ void computeMortonKernel(u8* d_pageBuffer, u64* d_mortonKeys, u32* d_indices, u32 pointCount)
    {
        u32 idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx >= pointCount) return;

        // Start of 15-byte block for this point
        u8* ptr = d_pageBuffer + (idx * 15);

        // Extract coordinates (unaligned loads supported on modern NVIDIA hardware)
        u32 x = *reinterpret_cast<u32*>(ptr + 0);
        u32 y = *reinterpret_cast<u32*>(ptr + 4);
        u32 z = *reinterpret_cast<u32*>(ptr + 8);

        // Standard 3D Morton Interleaving (X=2, Y=1, Z=0)
        d_mortonKeys[idx] = (expandBits(static_cast<u64>(x)) << 2) | 
                            (expandBits(static_cast<u64>(y)) << 1) | 
                             expandBits(static_cast<u64>(z));
        
        // Initial identity indices for the sort
        d_indices[idx] = idx; 
    }

    void launchMortonSort(
        u8* d_pageBuffer, 
        u64* d_mortonKeys, 
        u64* d_mortonKeysAlt,
        u32* d_indices, 
        u32* d_indicesAlt,
        void* d_tempStorage,
        size_t  tempStorageBytes,
        u32     pointCount)
    {
        if (pointCount == 0) return;

        const int blockSize = 256;
        const int gridSize = (pointCount + blockSize - 1) / blockSize;
        
        computeMortonKernel<<<gridSize, blockSize>>>(d_pageBuffer, d_mortonKeys, d_indices, pointCount);

        // Setup CUB Double Buffers
        cub::DoubleBuffer<u64> d_keys(d_mortonKeys, d_mortonKeysAlt);
        cub::DoubleBuffer<u32> d_values(d_indices, d_indicesAlt);

        // Execute Radix Sort
        cub::DeviceRadixSort::SortPairs(
            d_tempStorage, 
            tempStorageBytes, 
            d_keys, 
            d_values, 
            static_cast<int>(pointCount)
        );

        // Ensure d_indices always holds the sorted results for the next stage (RDV)
        if (d_values.Current() != d_indices) 
        {
            cudaMemcpy(d_indices, d_values.Current(), pointCount * sizeof(u32), cudaMemcpyDeviceToDevice);
        }
    }
}