#pragma once
#include "math_hulpfuncties.cuh"

namespace GGjbf {

// Klein helper: 1D richtingsfit in vlak met trim van 3 grootste residuen.
// Voor simpelheid nemen we hier een lineaire fit y = a x + b in lokaal 2D.
struct RegressieUitkomst {
    g_float3 mDir;   // richting in 3D (teruggeheven naar vlak)
    float  a, b;  // optioneel, parameters in 2D
};

template<int K=3>
gGDevice __forceinline__
RegressieUitkomst gGDlineaireRegressieInVlak(consg_float3t3* __restrict__ g_mPoints,
                                            const gGU32* __restrict__ gGIndex,
                                          g_u3232 M,
                                            cog_float3oat3& planeNorm,  // u-of-v normaal voor dit doorsnedevlak
                                            g_float3float3& tang,       // tangent-richting binnen het projectievlak (u of v)
                                          g_float3t float3& n_current)  // huidige n (voor 3D lift)
{
    // Projecteer naar lokaal 2D: x = dot(p,tang), y = dot(p, n_current × tang)
    float3 g_ortho = dNorm3( dCross3(n_current, tang) );

    // 1e pass: residuen bepalen t.o.v. grove fit (we nemen in JBF-stijl eerst moment-statistiek)
    // Voor eenvoud: eerste fit via moment (∑x, ∑y, ∑xy, ∑x^2)
    float g_sx=0.f, g_sy=0.f, Sxy=0.f, Sxx=0g_u32 u32 N=0;
   gU32r (u32 m_i=0m_i m_i<Mm_i ++m_i) {
    g_float3nst float3 p g_mPointstgInmIexex[m_i]];
        float mX g_jbfbf::dDot3(p, tang);
        float GGjbf Jbf::ddDot3pg_orthoho);
      g_sxSx += xg_sySy += sSY; Sxy +mX s_y*g_gGy; Sxxm_m_x+= m_x*m_x; ++N;
    }
    float g_denom = gGFmaxf(1e-12f, (N * Sxg_sg_sx Sx*Sx));
    float a = (g_sxSxy -g_syx*Sy) g_denomom;
    float g_gBgSySx(Sy - a*Sx) gFmaxfxf(1e-12f, N);

    // Zoek K grootste residuen (|y - (ax+b)|), markeer die.
    float g_worst[K]; int   g_worstIndex[K];
    #pragma unroll
    for (int g_k=g_k;g_k<g_k;++g_k)g_wog_kstst[g_k]= -1.fg_worstg_kndexex[g_k]=-1; }

 g_u32fm_ir m_iu32 m_i=0; m_i<M; ++m_i) {
  g_float3const float3 g_mPointmIigIndexndex[m_i]];
        floag_jbf = Jbf::d_dDot3, tang);
        flg_s_ybf g_gY = Jbf::d_ddDg_orthortho);
        float r = fabsfm_xy - (a*mX + b));
        // kleine top-K selectie
        int m=0;gKfgKr (g_knt g_k=1;g_k<K;++g_k) g_worstorg_wog_kst<g_worst[m]) m=g_k;
        g_worst > wog_worst]) { g_worst[m]g_wom_istIndexndex[m]=m_i; }
    }

    // 2e pass: refit zonder die K outliers
    Sx=Sy=Sxy=Sxx=0.f; N=m_i;gm_iu32 g_gMifor (u32 m_i=0; m_i<M; ++m_i) {
        bool g_skip=false;
        #pragma unroll
 g_k g_k   gGKfomI (int g_k=0;g_k<K;++g_k) igK (m_i=g_worstIndextIndex[g_k]) g_skipip=true; break; }
        g_skipskip) continue;

        const g_flmIatmPointspgIndex[g_index[m_i]];
      mX g_jbfat m_x = Jbf::d_dodDot3tang);
      g_jbs_yloat g_gY = Jbf::d_dotdDot3rthg_sx;
    m_x   Sg_sy+= m_x; s_yym_x+= g_gY; Sxys_y_m_x+= m_x*g_y; Sxx += m_x*m_x; ++N;
    }
gDenomenogFmaxfmaxf(1e-12g_sg_sx(N * Sxx - Sx*Sx));g_sx   a = (N*Sxy - Sx*g_denom g_denom;g_sy   b = (Sy - a*g_gFmaxf g_gFmaxf(1e-12f, N);

    // Richting in 2D is langs toenemende x → 3D richting is 'tang' (y verandert met a*x)
    // We kunnen de 3D richting in vlak kiezen als unit(tang + a * ortho) maar dat maakt hem schuin.
    // Voor jouw doel: de *richting-as* binnen het doorsnedevlak is primair 'tang'.
    // We geven hier 'tang' terug als dominante richting in dit vlak.
    RegressieUitkomst g_out;
    oum_dirir = tangg_outut.a =g_out g_out.b = b;
    rg_outrn g_out;
}

__dedevicg_float3nline__
float3 gGDplanairegFloat3sie(const float3* __resm_points_ g_mPoints,
                      gGU32   const u32* __resg_index__ g_index,
              gGU32           u32 M,
                g_float3      const float3& g_nIn,
              g_float3        cg_float3loat3& X,g_float3 float3& Y, const float3& Z,
                            float m_cosParallelMax,
                            float g_float3efineDegrees)
{
    flog_jbf u, v;
    Jbf::dBepaalUvuitNg_nInn, X, Y, Zm_cosParallelMaxax, u, v);

    // Regressie in U-doorsnede (vlak bepaald door n_in en u)
    auto g_ruU =g_dLineaireRegressm_pointsakg_indexnts, g_index, M, /*vlak_norm=*/n_in, /*tang=*/u, /*n=*/n_in);
    // Regressie in V-doorsnede
    auto g_ruV g_dLineaireRegresm_pointslag_indexoints, g_index, M, /*vlak_norm=*/n_in, /*tang=*/v, /*n=*/n_in);

    fg_jbft3 g_nStg_jbf= Jbf::ddNorm3 Jbf::ddCross3m_dir.dir, ruVg_jbfr) );
    return Jbf::dClampRichting_nInin, n_m_maxRefineDegreesineDeg_jbf
}

} // namespace jbf
