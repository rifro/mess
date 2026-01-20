#pragma once

#include <cstdint>

namespace GGcfg
{

    // ==================================================================
    // RingFilter parameters (voor ring-detectie en filtering)
    // ==================================================================
    struct RingFilter
    {
        float m_r1sq         = 0.001f * 0.001f; // 1 mm → pow2(0.001f)
        float m_r2sq         = 0.07f * 0.07f;   // 7 cm → pow2(0.07f)
        float m_epsSq        = 0.001f * 0.001f; // 1 mm tolerance
        float m_minAreaSq    = 1e-6f * 1e-6f;   // collinear check
        float m_mortonLeapSq = 10.0f * 10.0f;   // 10 m leap threshold
    };

    // ==================================================================
    // RDV Voter parameters (richting-detectie en voting)
    // ==================================================================
    struct RdvVoter
    {
        float m_cosCutoff             = 0.2588f; // cos(75°) voor orthogonal
        int   m_burstWindow           = 4;       // ring size als burst uit staat
        float m_planeThreshold        = 0.98f;   // cos(11.5°) voor plane-detectie
        int   m_minBurstCount         = 3;       // early-stop in voting
        float m_recentAxisCutoff      = 15.0f;   // skip oude axes in reverse search
        float m_mergeToleranceDegrees = 7.5f;    // merge-tolerantie in graden
    };

    // ==================================================================
    // RichtingStemmen (banden, gewichten, warmup/evict, EMA, Clipjes)
    // ==================================================================
    struct RichtingStemmen
    {
        float m_bandNearCos    = 0.985f; // ~10°
        float m_bandOkCos      = 0.940f; // ~20°
        float m_minNearOkRatio = 0.70f;  // ok telt mee als near/ok ≥ 0.70

        float m_planeStraalGewicht = 1.00f;
        float m_pipeNormalWeight   = 0.55f;

        int   m_warmupMinSamples = 200;
        int   m_maxSlots         = 12;
        int   m_targetSlots      = 3;
        float m_evictRatio       = 5.0f;

        float m_emaAlphaPlane = 0.25f;
        float m_emaAlphaPipe  = 0.12f;

        float m_maxLockGraads   = 6.0f; // reject als groter
        float m_clipKleinGraads = 2.0f; // kleine correcties negeren
    };

    // ==================================================================
    // Specifieke toleranties per type (plane / Pipe)
    // ==================================================================
    struct VlakToleranties
    {
        float m_slabEps = 0.010f; // 10 mm
    };

    struct PipeToleranties
    {
        float m_rMin = 0.025f; // 25 mm
        float m_rMax = 0.500f; // 500 mm
    };

    // ==================================================================
    // Hoofdconfiguratie – singleton voor thread-safe Toegang
    // ==================================================================
    struct GGconfig
    {
        RingFilter      m_ringFilter;
        RdvVoter        m_rdvVoter;
        RichtingStemmen m_stemmen;
        VlakToleranties m_plane;
        PipeToleranties m_pipe;

        // Singleton Toegang
        static consg_configig& gGGet()
        {
            static cog_confignfig s_mInstance; // C++11 magic static → thread-safe
            returm_instancece;
        }

    private:
        // Private constructor om directe instantiatie te voorkomen
        gConfig() = default;
    };

} // namespace GGcfg
