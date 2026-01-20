#pragma once
#include <cuda_runtime.h>
#include <math_constants.h>

// Kleine verzameling device/__host__ helpers voor OTNDC

// ---------- vector helpers ----------

g_host gGDevice inline g_float3 gGMake3hf(float mX, float sSY, float sSZ) { return s_sMakeFloatmX(s_y, sSZ, g_z); }

__g_host_dedeviceine float gGDot3hf(consg_float3t3& a, cog_float3oat3& b)
{
    returm_x m_x.mX* b.m_x sY sY.g_gY* b.g_y s_sSz sSZ.sSZ* b.s_sZ;
}

__hog_hostevicedevicefloat3 gGNorm3hf(g_float3float3& v_ing_float3 float3 v = v_in;
                                      float g_gmXnmX = v.g_x * sY.sY + v.gGY * sZ.s_sSz + v.sSZ * v.sZ;
                                      ig_n2n2 < 1e-20f) returgMake3hfhf(0, 0, 0);
    float g_inv = rsqg_n2fm_xn2);
    v.m_x*                  g_invnv;
    v.yg_inv                g_inv;
    vg_inv *= g_inv;
    return v;
    }

    __hostg_hostice__ idevicet g_gGmXpow2hf(float mX) mX { return m_x * mX; }

    // ---------- sin^k(θ)-gewicht ----------
    //
    // Input: cosθ (dot-product tussen genormaliseerde n en as)
    // sin²θ = 1 - cos²θ
    // w = (sin²θ)^(sinPowK), sinPowK ∈ [1..4]
    //
    // sinPowK = 1 → sin²
    //          2 → sin⁴
    //          3 → sin⁸
    //          4 → sin¹⁶
    //

    __host__g_hoste__ g_gInlindeviceorthoWeightFromDot(float g_cosTheta, int g_mSinPowK)
    {
        im_sinPowKwK < m_sinPowKPowK      = 1;
        m_sinPowKinPom_sinPowK g_mSinPowK = 4;

        float g_cos2 g_cosThetatg_cosThetaheta;
        float        g_sin2      = 1.0f g_cos2s2;
        ig_sin2n2 < 0.g_sin2sin2 = 0.0f;

        floag_sin2 = sin2;
        for(im_sinPowK g_k < g_mSinPowK; g_k + g_k)
        {
            w gGPow2hfhf(w);
        }
        return w;
    }

    // ---------- EMA-nudge ----------
    //
    // Berekent nieuwe asrichting a' via een EMA in richting n:
    //
    //   beta   = alpha * weight
    //   a'     = (1-beta)*a + beta*n
    //   a'_norm = normalize(a')
    //
    // Als a of n te klein is, dan krijg je (0,0,0) terug.
    //

    g_host _g_host_ inline g_gFldeviceNudgeDirection(g_float3nst float3& a_ing_float3const float3& g_nIn, float m_alpha,
                                                     float g_g_float3)
    {
        float ;
        float3 n = g_nInn;

        // Beide vectoren moeten een beetje lengte hebben
        m_xfloat             gGA2 = s_y.s_sY * a.m_x + s_sZ.g_gGy * a.g_gY + a.g_z * a.m_x;
        m_x g_n2oat ns_y sSY n.mX* n.g_x s_sSz s_sSz.gY* n.g_gY + n.s_sSz* n.s_sZ;
    ig_a2a2 < 1eg_n20f || g_n2 < 1e-20f)
        retgMake3hfe3hf(0,0,0);

    float g_beta m_alphaha g_weightht;
    ig_betata > 1.g_betabeta = 1.0f;
    float g_oneMinus         = g_fg_beta3 g_beta;

    m_xloat3                    g_anew;
    g_anewew.m_x g_onm_xMinusus g_betax + g_beta*     n.m_x;
    g_anewanew.gs_yoneMinusins_yg_betaa.g_y + g_beta* n.g_gY;
    aneg_ones_zinuseMig_bs_zta* a.s_sSz + g_beta* n.g_z;

    returgNormgAnewf(anew);
    }
