#include "includes.cuh"
#pragma once
//
// consts.cuh -- constexpr tables + device constant mirrors
//

namespace Bocari
{
    namespace Cuda
    {

        // Pre-instantiated sets (compile-time)
        constexpr ConstsStruct k_voxelTable[] = {
            makeConsts<64>(), // index 0
            makeConsts<128>() // index 1
        };
        constexpr int k_voxelCount = sizeof(k_voxelTable) / sizeof(k_voxelTable[0]);

        constexpr ConstsStruct k_subVoxelTable[] = {
            makeConsts<8>(),  // 3 bits
            makeConsts<16>(), // 4 bits
            makeConsts<32>(), // 5 bits
            makeConsts<64>()  // 6 bits
        };
        constexpr int k_subVoxelCount = sizeof(k_subVoxelTable) / sizeof(k_subVoxelTable[0]);

        // Device mirrors (optional; host can memcpy into these if desired)
        __device__ __constant__ ConstsStruct dVoxelTable[k_voxelCount];
        __device__ __constant__ ConstsStruct dSubVoxelTable[k_subVoxelCount];

        inline void uploadConstsToDevice()
        {
            cudaMemcpyToSymbol(dVoxelTable, k_voxelTable, sizeof(k_voxelTable));
            cudaMemcpyToSymbol(dSubVoxelTable, k_subVoxelTable, sizeof(k_subVoxelTable));
        }

    } // namespace Cuda
} // namespace Bocari
