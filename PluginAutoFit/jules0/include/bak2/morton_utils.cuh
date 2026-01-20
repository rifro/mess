#include "includes.cuh"
#pragma once

// --------------------------------------------------------------
// CONFIG
// --------------------------------------------------------------

#ifndef REGIO_PREFIX_BITS
#define REGIO_PREFIX_BITS 12 // of jouw waarde
#endif

// Max bits voor 3D morton: 3 × 21 = 63 bits → genoeg.
// De regio-prefix pakt de top REGIO_PREFIX_BITS bits.
static constexpr u64 MORTON_MASK = (~0ULL >> REGIO_PREFIX_BITS);

// --------------------------------------------------------------
// Bit interleave helpers (21 bits → 63 bits)
// --------------------------------------------------------------

__host__ __device__ inline u64 part1by2(u64 x)
{
    x &= 0x1fffffULL; // 21 bits
    x = (x | (x << 32)) & 0x1f00000000ffffULL;
    x = (x | (x << 16)) & 0x1f0000ff0000ffULL;
    x = (x | (x << 8)) & 0x100f00f00f00f00fULL;
    x = (x | (x << 4)) & 0x10c30c30c30c30c3ULL;
    x = (x | (x << 2)) & 0x1249249249249249ULL;
    return x;
}

__host__ __device__ inline u64 morton3d(u32 x, u32 y, u32 z)
{
    return part1by2(x) | (part1by2(y) << 1) | (part1by2(z) << 2);
}

// --------------------------------------------------------------
// Kwantisatie-struct
// --------------------------------------------------------------

struct Kwantisatie
{
    float3 bbMin;
};

// Clamp naar 32-bit (voor veiligheid)
__host__ __device__ inline u32 clampu32(u64 v) { return (v > 0xffffffffULL) ? 0xffffffffu : (u32)v; }

// --------------------------------------------------------------
// Float3 → Quantization naar integer 0..2^21
// --------------------------------------------------------------
__host__ __device__ inline void kwantiseerPunt(const Vec3f& p, const Kwantisatie& Q, u32& xi, u32& yi,
                                                u32& zi)
{
    float X = (p.x - Q.bbMin.x) * 2048.0f; // QuantizeScale
    float Y = (p.y - Q.bbMin.y) * 2048.0f; // QuantizeScale
    float Z = (p.z - Q.bbMin.z) * 2048.0f; // QuantizeScale

    auto quant = [](float v) -> u32 {
        float t = v + 0.5f; // round-to-nearest (simpel, snel)
        if(t < 0.f) t = 0.f;
        u64 w = (u64)t; // floor
        return (w > 0xffffffffULL ? 0xffffffffu : (u32)w);
    };

    xi = quant(X);
    yi = quant(Y);
    zi = quant(Z);
}

// --------------------------------------------------------------
// Combine morton + regio-prefix
// --------------------------------------------------------------

__host__ __device__ inline u64 mortonMetRegio(u32 xi, u32 yi, u32 zi, u32 regioCode)
{
    u64 m = morton3d(xi, yi, zi);
    return (((u64)regioCode) << (64 - REGIO_PREFIX_BITS)) | (m & MORTON_MASK);
}

void h_voxelizeMorton(const DeviceBuffer<float3>& d_points, DeviceBuffer<u64>& d_keys,
                         DeviceBuffer<u32>& d_indices, DeviceBuffer<float3>& d_pointsSorted, size_t N);
