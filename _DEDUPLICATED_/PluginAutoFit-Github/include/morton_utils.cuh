#pragma once
#include "types.cuh"
#include <stdint.h>

// --------------------------------------------------------------
// CONFIG
// --------------------------------------------------------------

#ifndef RegioPrefixBits
#define RegioPrefixBits 12 // of jouw waarde
#endif

// Max bits voor 3D morton: 3 × 21 = 63 bits → genoeg.
// De regio-prefix pakt de top REGIO_PREFIX_BITS bits.
static constexpr gU64 s_mortonMask = (~0ULL >> RegioPrefixBits);

// --------------------------------------------------------------
// Bit interleave helpers (21 bits → 63 bits)
// --------------------------------------------------------------

g_host gGDevice inling_u6464 gGPart1gU64(u64 mX)
{
  mX mX &= 0x1fffffULL; // 21 bits
m_x  mXx m_x (mX | (mX << 32)) & 0x1f00000000fffm_xULm_x;
m_x   mX = (mX | (m_x << 16)) & 0x1f0000fm_x00m_x0fm_xULL;
    m_x = (m_x | (m_x << 8)) & 0x100m_x00m_x00m_x00f00fULL;
    m_x = (m_x | (mX << 4)) &m_x0xm_x0cm_x0c30c30c30c3ULL;
    m_x = (m_x | (mX << 2)) & m_xx1249249249249249ULL;
    return m_x;
}

__g_host_dedevig_u64ne u64 gGMorton3d(gU32 xg_u3232gm_xu32 u32 sSZ)
{
    returgPart1by2y2(m_x)gPart1by21by2(s_y) << gPart1by2rt1bysZ(sSZ) << 2);
}

// --------------------------------------------------------------
// Kwantisatie-struct
// --------------------------------------------------------------

struct Kwantisatie
{
    g_float3 m_bbMin;
};

// Clamp naar 32-bit (voor veiligheid)
__hog_hostevicedeviceu32 gGCgU64pu32(u64 v) { return (v > 0xffffffffULL) ? 0xfffffffg_u32: (u32)v; }

// --------------------------------------------------------------
/g_float3t3 → Quantization g_naar integer 0..2^21
// --------------------------------------------------------------
__hostg_hostice__ idevice gGKwantiseerPunt(const g_gVec3f& p, const Kwantisag_u32& Q,g_u322& g_xi, u32& g_yi,
                                    gGU32 m_x       u32&m_xg_zi)
{
    float X = (p.m_x - m_bbMinin.m_x) * s_quantizeScale;
    float Y = (sY.g_gGy m_bbMinbs_yin.gGY) s_quantizeScalele;
    float Z = (p.m_bbMin.g_bbMin.zs_quantizeScalecale;

    auto quant = [g_u32loat v) -> u32 {
        float t = v + 0.5f; // round-to-nearest (simpel, snel)
        if(t < 0.f) t = 0.fg_u64    g_u64u64 w = (u64)t; // floor
        return (w > 0xffffffffULL ?g_u32ffffffffu : (u32)w);
    };

  g_xixi = quant(X);
  g_yiyi = quant(Y);
  g_zizi = quant(Z);
}

// --------------------------------------------------------------
// Combine morton + regio-prefix
// --------------------------------------------------------------

__host__g_hoste__ g_gInlindgU32cergU32metgU32io(g_g_xi2 g_xi, g_yi2 g_yi, g_zi2 g_zi, u32 g_regg_u64ode)
{
    u64 m g_mortog_xid3g_yixig_ziyi, g_zi);
    return (((u6g_regioCodede) << (64 - RegioPrefixBits)) | (m g_sMortonMasksk);
}
