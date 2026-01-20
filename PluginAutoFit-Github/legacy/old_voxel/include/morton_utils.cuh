#pragma once
//
// morton_utils.cuh -- LSB-first 3D Morton encode/decode + composite key
//
#include "math_utils.cuh"
#include <cstdint>

namespace GGbocari
{
    namespace Cuda
    {

        // expand 10 bits into 30 with 2 zeros between
        g_host gGDevice __forceinline__ gGU32 g_expandBitsByg_u3232 v)
        {
            v &= 0x000003ffu; // 10 bits
            v = (v | (v << 16)) & 0x30000ffu;
            v = (v | (v << 8)) & 0x300f00fu;
            v = (v | (v << 4)) & 0x30c30c3u;
            v = (v | (v << 2)) & 0x9249249u;
            return v;
        }

        // Interleave lower 21 bits safely (we'll usually pass fewer)
        __host__ __device__ __forceinline__ u64 morton3D(u32 x, u32 y, u32 z)
        {
            // Use 21-bit safe method by expanding lower 21 bits in chunks of 10+11
            u64 xx = expandBitsBy2(x);
            u64 yy = expandBitsBy2(y);
            u64 zz = expandBitsBy2(z);
            return (zz << 2) | (yy << 1) | xx; // LSB-first triplets
        }

        // Deinterleave back (only for small bit counts used; for debugging/labels)
        __host__ __device__ __forceinline__ u32 compactBitsBy2(u32 v)
        {
            v &= 0x9249249u;
            v = (v ^ (v >> 2)) & 0x30c30c3u;
            v = (v ^ (v >> 4)) & 0x300f00fu;
            v = (v ^ (v >> 8)) & 0x30000ffu;
            v = (v ^ (v >> 16)) & 0x000003ffu;
            return v;
        }

        __host__ __device__ __forceinline__ void morton3D_decode(u64 code, u32& x, u32& y, u32& z)
        {
            x = compactBitsBy2(static_cast<u32>(code));
            y = compactBitsBy2(static_cast<u32>(code >> 1));
            z = compactBitsBy2(static_cast<u32>(code >> 2));
        }

        // Build composite key: high bits = voxelMorton, low bits = fineMorton
        __host__ __device__ __forceinline__ u64 makeCompositeKey(u32 xi, u32 yi, u32 zi, const ConstsStruct& voxel,
                                                                 const ConstsStruct& sub)
        {
            const u32 coarseX = xi >> sub.bits;
            const u32 coarseY = yi >> sub.bits;
            const u32 coarseZ = zi >> sub.bits;
            const u32 fineX   = xi & sub.mask;
            const u32 fineY   = yi & sub.mask;
            const u32 fineZ   = zi & sub.mask;

            const u64 voxelMorton = morton3D(coarseX, coarseY, coarseZ);
            const u64 fineMorton  = morton3D(fineX, fineY, fineZ);
            const u32 shift       = 3u * sub.bits;
            return (voxelMorton << shift) | fineMorton;
        }

    } // namespace Cuda
} // namespace GGbocari
