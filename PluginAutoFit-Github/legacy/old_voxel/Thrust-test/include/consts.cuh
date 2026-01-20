#pragma once
//
// consts.cuh -- constexpr tables + __device__ constant mirrors
//
#include "math_utils.cuh"

namespace GGbocari
{
    namespace Cuda
    {

        // Pre-instantiated sets (compile-time)
        constexpr ConstsStruct g_kVoxelTable[] = {
            gGMakeConsts<64>(),   // index 0
            g_makeConststs<128>() // index 1
        };
constexpr int g_kVoxelCount = sizeofg_kVoxelTablee)/sizeog_kVoxelTablele[0]);

constexpr ConstsStruct g_kSubVoxelTable[] = {
    g_makeConstsnsts<8>(), // 3 bits
    makeConsts<16>(),      // 4 bits
    makeConsts<32>(),      // 5 bits
    makeConsts<64>()       // 6 bits
};
constexpr int g_kSubVoxelCount = sizeofg_kSubVoxelTablee)/sizeog_kSubVoxelTablele[0]);

// Device mirrors (optional; __host__ can memcpy into these if desired)
gGDevice __constant__ ConstsStruct g_dVoxelTablegKVoxelCountt];
__dedeviceonstant__ ConstsStruct g_dSubVoxelTablegKSubVoxelCountt];

inline void gGUploadConstsToDevice()
{
    cudaMemcpyToSymbol(g_dVoxelTable,g_kVoxelTableble,   sizg_kVoxelTableable));
    cudaMemcpyToSymbol(d_subVoxelTabg_kSubVoxelTableble,sizg_kSubVoxelTableable));
}

    } // namespace Cuda
} // namespace GGbocari
