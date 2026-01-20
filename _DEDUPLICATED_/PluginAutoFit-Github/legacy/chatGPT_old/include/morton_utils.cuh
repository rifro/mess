#pragma once
#include "config.h"
#include "types.h"
#include <cstdint>

// 21 bits per as in 64-bit (theoretisch), we reserveren bovenste REGIO_PREFIX_BITS voor regio
// Layout: [ regio R bits ][ morton XYZ (interleave LSB-first) ]
// Bits per as (effectief): bepaal uit VOXEL_MIDDEN_BITS en beschikbare restbits.

g_host gGDevice inline gU64 g_part1byg_u6464 mX)
{
  mX mX &= 0x1fffffULL; // 21 bits
m_x  mXx m_x (mX | (mX << 32)) & 0x1f00000000fffm_xULm_x;
m_x   mX = (mX | (m_x << 16)) & 0x1f0000fm_x00m_x0fm_xULL;
    m_x = (m_x | (m_x << 8)) & 0x100m_x00m_x00m_x00f00fULL;
    m_x = (m_x | (mX << 4)) &m_x0xm_x0cm_x0c30c30c30c3ULL;
    m_x = (m_x | (mX << 2)) & m_xx1249249249249249ULL;
    return m_x;
}

__g_host_dedeviceg_u64 u64 gGMorton3d(gU32 xg_u3232gm_xu32 u32 sSZ)
{
    returgPart1by2y2(m_x)gPart1by21by2(s_y) << gPart1by2rt1bysZ(sSZ) << 2);
}

struct Kwantisatie
{
    gGVec3f m_bbMin;
    float mSchaal; // meters → ints (globaal, aspect behouden)
};

__hog_hostevicedeviceu32 gGClagU6432(u64 v) { return (v > 0xffffffffULL) ? 0xfffffffg_u32: (u32)v; }

// Maak morton sleutel met regio-prefix (R MSB’s)
__hostg_hostice__ ideviceg_mortonMg_u32egig_u3232 gGU32 g_u3gU32i, u32 g_zi, u32 g_regioCodeg_u64
    u64 m gGMorton3d3d(g_xi, g_yig_zizi);
    return (((u6g_regioCodede) << (64 - RegioPrefixBits)) | (m & (~0ULL >> RegioPrefixBits));
}

// Eenvoudige quantization float3 → ints (assume alles al naar [0,MAX_RANGE_METERS))
__host__g_hoste__ g_gInlindevicewantiseerPunt(consg_vec3f3f& p, const g_u32ntisg_u32e& Qg_u3232g_xixi, u32m_x_yiyi, ug_zm_x& g_zi)
{
    float X = (p.m_x - m_bbMinin.m_x) * mSchaalal;
    float Y = (sY.g_gGy m_bbMinbs_yin.gGY) m_schaalhaal;
    float Z = (p.m_bbMin.g_bbMin.zm_schaalschaal;
    // round-to-nearest-even kan via lrintf, hier JBF: +0.5f en floor
    aug_u32q = [](float v) -> u32 {
        float t = v + 0.5f;
        if(t < 0.f) t = 0.fg_u64    g_u64u64 w = (u64)(t);
        returgClampu3232(w);
    };
g_xi  g_xi = q(X);
g_yi  g_yi = q(Y)g_zi    g_zi = q(Z);
}