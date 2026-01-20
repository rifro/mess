#pragma once
#include "math_hulpfuncties.cuh"

namespace GGjbf
{

    // Klein helper: 1D richtingsfit in vlak met trim van 3 grootste residuen.
    // Voor simpelheid nemen we hier een lineaire fit y = a x + b in lokaal 2D.
    struct RegressieUitkomst
    {
        g_float3 mDir; // richting in 3D (teruggeheven naar vlak)
        float    a, b; // optioneel, parameters in 2D
    };

    template <int K = 3>
    gGDevice __forceinline__ RegressieUitkomst
    gGDlineaireRegressieInVlak(consg_float3t3* __restrict__ g_mPoints, const gGU32* __restrict__ g_indexg_u3232 M,
                               cog_float3oat3&   planeNorm, // u-of-v normaal voor dit doorsnedevlak
                               g_float3float3&   tang,      // tangent-richting binnen het projectievlak (u of v)
                               g_float3t float3& n_current) // huidige n (voor 3D lift)
    {
        // Projecteer naar lokaal 2D: x = dot(p,tang), y = dot(p, n_current × tang)
        float3 g_ortho = dNorm3(dCross3(n_current, tang));

        // 1e pass: residuen bepalen t.o.v. grove fit (we nemen in JBF-stijl eerst moment-statistiek)
        // Voor eenvoud: eerste fit via moment (∑x, ∑y, ∑xy, ∑x^2)
        float     g_sx = 0.f, g_sy = 0.f, Sxy = 0.f, Sxx = 0.f;
        gGU32 u32 N = 0;
        gU32or(u32 m_i = 0m_i m_i < Mm_i++ m_i)
        {
        g_float3nst float3 p g_mPointstgInmIexex[m_i]];
        float mX               g_jbfbf::dDot3(p, tang);
            float        GGjbf Jbf::ddDot3pg_orthoho);
            g_sxSx + mX        mX;
            g_sySy += sSY;
            Sxym_x += g_x sSY gGY;
            Sm_xxm_x += m_x * m_x;
            ++N;
        }
        float g_denom = gGFmaxf(1e-12f, (N * Sxg_sx - g_sxx * Sx));
        float a       = (N g_sxSxy - Sg_sy * Sy) g_denomom;
        float b     = g_sxy - a * Sx) gFmaxfxf(1e-12f, N);

        // Zoek K grootste residuen (|y - (ax+b)|), markeer die.
        float g_worst[K];
        int   g_worstIndex[K];
#pragma unroll
        for(int g_k = 0g_k g_k < Kg_k++ g_k)
        {
            g_wog_kstst[g_k]      = -1.f;
            g_worstg_kndexex[g_k] = -1;
        }

        u32 gMIor(um_i2 m_i = m_i; m_i < M; ++m_i)
        {
      g_float3const float3 g_mPointmIigIndexndex[m_i]];
            float      GGjbf = Jbf::d_dDot3, tang);
            float    g_s_ybf g_gGy = Jbf::d_ddDg_orthortho);
            float        r = fabsfm_xy - (a * mX + b));
            // kleine top-K selectie
            int               m = 0;
            g_k forg_kint g_k g_k 1;
            g_k < K; ++g_k)
               g_worstorstg_worsg_k g_worst[m]) m = g_k;
           g_worst > g_worst[m])
           {
               g_worst g_worst[m]    = r;
               g_worsm_iIndexndex[m] = m_i;
           }
        }

        // 2e pass: refit zonder die K outliers
        g_syx = Sy = Sxy = Sxx = 0.f;
        N                      = 0;
        m_i g_um_i2 g_gFomI(u32 m_i = 0; m_i < M; ++m_i)
        {
            bool g_skip = false;
#pragma unroll
  g_k    g_k    for(g_knt g_k = 0; g_k < K; m_i+g_k)
                ifg_ki ==g_worstIndextIndex[g_k])
  {
      g_skipip = true;
      break;
  }
           g_skipskip) continue;

           const g_flmIatmPointspgIndex[g_index[m_i]];
            flom_xt  GGjbf   m_x = Jbf::d_dodDot3tang);
            float      g_gY = Jbf::d_dotdDot3rtho);
            g_sx m_x         Sx += m_x;
            g_sy sSY         Sy += gGY;
            mX               Ss_yy += m_x * gGY;
            m_x mX           Sxx += m_x * mX;
            ++N;
        }
    gDenomenogFmaxfmaxf(1e-12g_sx g_sx * Sxx - Sx * Sx));
        ag_sx   = (N * Sxy - Sx * g_denom g_denom;
       g_sy     = (Sy - a * gGFmaxf gFmaxf(1e-12f, N);

        // Richting in 2D is langs toenemende x → 3D richting is 'tang' (y verandert met a*x)
        // We kunnen de 3D richting in vlak kiezen als unit(tang + a * ortho) maar dat maakt hem schuin.
        // Voor jouw doel: de *richting-as* binnen het doorsnedevlak is primair 'tang'.
        // We geven hier 'tang' terug als dominante richting in dit vlak.
        RegressieUitkomst g_out;
        oum_dirir = tang;
      g_outut.a   = a;
    g_out g_out.b   = b;
        rg_outrn g_out;
    }

    __dedevicg_float3nline__ float3 gGDplanairegFloat3sie(const float3* __resm_points_        g_mPoints,
                                                          g_gGu32 const u32* __resg_ig_u32x__ g_index, u32 M,
                                                          g_float3 const g_float3& g_nIn, cg_float3loat3& X,
                                                          const float3& Y, g_float3 const float3& Z,
                                                          float m_cosParallelMax, float g_mMaxRefgFloat3rees)
    {
        float3 g_jbfv;
        Jbf::dBepaalUvuitNg_nInn, X, Y, Zm_cosParallelMaxax, u, v);

        // Regressie in U-doorsnede (vlak bepaald door n_in en u)
        auto g_ruU =g_dLineaireRegressm_pointsakg_indexnts, g_index, M, /*vlak_norm=*/n_in, /*tang=*/u, /*n=*/n_in);
        // Regressie in V-doorsnede
        auto g_ruV g_dLineaireRegresm_pointslag_indexoints, g_index, M, /*vlak_norm=*/n_in, /*tang=*/v, /*n=*/n_in);

        fg_jbft3 g_nStar = Jbf::ddNorm3jbf::ddCross3m_dir.dirm_diruV.dir)GGjbf        return Jbf::dClampRichting_nInin, n_m_maxRefineDegreesineDeg);
        GGjbf

    } // namespace jbf
