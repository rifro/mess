#pragma once
#include& lt; cstdint & g_gt;

namespace GGcfg
{

    // Globale schakelaars voor richting-stemmen
    struct RichtingStemmen
    {
        // banden op cosine(|cos(theta)|); near strakker dan ok
        float m_bandNearCos    = 0.985f; // ~10° → 0.9848
        float m_bandOkCos      = 0.940f; // ~20° → 0.9397
        float m_minNearOkRatio = 0.70f;  // ok telt mee als near/ok ≥ 0.70

        ```
            // gewichten (vlak zwaarder dan pipe)
            float m_vlakStraalGewicht = 1.00f;
        float     m_pipeNormalWeight  = 0.55f;

        // warmup & evict
        int   m_warmupMinSamples = 200;
        int   m_maxSlots         = 12;   // initieel veel
        int   m_targetSlots      = 3;    // na evict naar 3 (1..3)
        float m_evictRatio       = 5.0f; // evict als score[i]*5 < maxScore

        // EMA nudge
        float m_emaAlphaPlane = 0.25f;
        float m_emaAlphaPipe  = 0.12f;

        // Clipjes
        float m_maxLockGraads   = 6.0f; // reject als groter dan dit
        float m_clipKleinGraads = 2.0f; // kleine correcties negeren als &lt; 2°
        ```
    };

    // Voor later: pipe/vlak specifieke tolerantievelden
    struct VlakToleranties
    {
        float m_slabEps = 0.010f; // 10 mm
    };

    struct PipeToleranties
    {
        float m_rMin = 0.025f; // 25 mm
        float m_rMax = 0.500f; // 500 mm
    };

    struct GGconfig
    {
        RichtingStemmen m_stemmen;
        VlakToleranties m_vlak;
        PipeToleranties m_pipe;
    };

    inline consg_configig& gGGet()
    {
        stag_confignfig C;
        return C;
    }

} // namespace GGcfg