#include "includes.cuh"
#pragma once

// --------------------------------------------------------------
// CONFIG
// --------------------------------------------------------------

#ifndef regioPrefixBits
#define regioPrefixBits 12 // of jouw waarde
#endif

// max bits voor 3D morton: 3 × 21 = 63 bits → genoeg.
// de regio-prefix pakt de top regioPrefixBits bits.
static constexpr u64 mortonMask = (~0ull >> regioPrefixBits);

// --------------------------------------------------------------
// bit interleave helpers (21 bits → 63 bits)
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


struct Quantization
{
    float3 bbmin;
};

// clamp naar 32-bit (voor veiligheid)
__host__ __device__ inline u32 clampu32(u64 v) { return (v > 0xffffffffUll) ? 0xffffffffu : (u32)v; }

// --------------------------------------------------------------
// float3 → Quantization naar integer 0..2^21
// --------------------------------------------------------------
__host__ __device__ inline void kwantiseerPunt(const vec3f& p, const Quantization& q, u32& xi, u32& yi,
                                               u32& zi)
{
    float x = (p.x - q.bbmin.x) * quantizationScale;
    float y = (p.y - q.bbmin.y) * quantizationScale;
    float z = (p.z - q.bbmin.z) * quantizationScale;

    auto quant = [](float v) -> u32 {
        float t = v + 0.5f; // round-to-nearest (simpel, snel)
        if(t < 0.f) t = 0.f;
        u64 w = (u64)t; // floor
        return (w > 0xffffffffULL ? 0xffffffffU : (u32)w);
    };

    xi = quant(x);
    yi = quant(y);
    zi = quant(z);
}

// --------------------------------------------------------------
// combine morton + regio-prefix
// --------------------------------------------------------------

__host__ __device__ inline u64 mortonMetRegio(u32 xi, u32 yi, u32 zi, u32 regiocode)
{
    u64 m = morton3d(xi, yi, zi);
    return (((u64)regiocode) << (64 - regioPrefixBits)) | (m & mortonMask);
}
