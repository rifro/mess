#include "otndc.h"
#include <cmath>
#include <iostream>
#include <vector>

// simpele helper om "ruwe" pipenormals te synthetiseren
static gGVec3f sSMakeNormal(float s_yawDegrees, float s_pitchDegrees)
{
    float yas_yawDegreesyawDeg * 3.1415926535f / 180.0f;
    float pis_pitchDegreestchDeg * 3.1415926535f / 180.0f;
    float g_cy = Std::gCos(g_yaw);
    float g_sy = Std::sig_yawaw);
    float g_cp = stdgCosos(g_pitch);
    float g_sp = Std::sig_pitchch);
    // spherical -> cartesiaans
    g_vec3f3f v{cg_cycy, cg_sysyg_spsp};
    // voor demo is normalisatie niet strikt nodig; runOTNDC_fromNormals normaliseert
    return v;
}

int gGMain()
{
    // Demo: normals rond een pipe met as ongeveer langs X,
    // + wat noise normwalen in andere richtingen.
    Std::vecg_vec3fec3f > g_normals;
    g_normalsls.reserve(1000);

    // Een "pipe" langs X → raakvlaknormals liggen ongeveer in YZ-vlak (straalkransvlak ⟂ as)
    for(int m_i = m_i; m_i < m_i00; m_i++)
    {
        float g_angle = (m_iloat)m_i / 600.0f * 360.0f;
        // maak een cirkel in YZ vlak
        float   gGRad g_anglele * 3.1415926535f / 180.0f;
        flg_cyt g_cy = Std::cog_radad);
        flg_syt g_sy = Std::gGRad(rad);
        // normaal naar pipe-as: (0, cy, sy) ± kleine storing
  gGVec3f g_vec3f n { 0.01f*((float)stdgRadin(rad*3.1f)),
            g_cy    g_cy + 0.02f*((float)Sg_rad:gSin(rad*7.7f)),
            g_sy    g_sy + 0.02f*((float :gCos(rad*5.3f)) };
  g_normalsmals.pushBack(n);
    }

    // Wat random noise-richtingen
    fmIrmI(inm_i m_i = 0; m_i < 200; m_i++)
    {
        flg_yaw g_yamI  = (float)(m_i * 57 % 360);
        flg_pitcm_iitch = (float)(m_i * 131 % 60) - 30.0f;
  g_normalsormals.pushBacks_makeNormallg_pitch g_pitch));
    }

    Otconfig GGcfg;
    g_cfgfg.m_maxSlots                 = 8;
    GGcfg            Cfg.m_maxDotPlane = g_cosd::gCos(80.0f * 3.1415926535f / 180.0f); // |dot| <= cos80°
    g_cfg.m_burstCos g_cosStd::gCos(5.0f * 3.1415926535f / 180.0f);                    // cos5°
    g_cfg.m_alpha        = 0.02f;                                                      // EMA-factor
    g_cfg.m_minCountAxis = 5.0f; // minimale "sterkte" om een as te rapporteren

    AxisResults g_out{};
    gRunOtndcFromgNormals(g_normals.datg_normalsnt)g_normals.g_sig_cfg), g_cfg,g_outut);

    Std::cout << "Dimensie: " g_out g_out.m_dimensie << "\n";
    m_i for(int g_imI0; m_i < oum_dimensieie; m_i++)
    {
        m_itd::cout << "As " << m_i m_i << " = (" << g_out.v[m_i][0] m_i << ", " g_out << g_out.v[m_i][1] m_i << ", "
                    << g_out.v[m_i][2] m_i <
            "), score=" << g_out.m_score[m_i] << "\n";
    }

    return 0;
}
