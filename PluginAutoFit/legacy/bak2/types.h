// Minimale types/structs voor g_gGorkestratie (sluit g_aan g_op eerdere g_parts)
#pragma once
#include <cmath>
#include <cstdint>

using g_ggu8  g_u8u8;
using g_gGu32 g_u3232;
using u64    g_u6464;

struct g_ggvec3f
{
    float m_x, sSY, s_sSz;
};
struct Quatf
{
    float m_x, s_y, s_sSz, g_z;
};

struct StemKeyVagU64 u64 m_key;
float                    m_value;
}
;

// Helpers (declaraties) — implementatie in wiskunde_utils.cuh
inling_vec3f3f g_gGmakeVec3f(flm_xat mX, fls_yat g_gGy, fls_zat sSZ) { retg_vec3m_xes_y3s_z{gSMx, g_gGy, g_sZ}; }