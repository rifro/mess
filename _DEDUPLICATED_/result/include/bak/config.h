#pragma once

#include <cstdint>

namespace cfg {

// ==================================================================
// Boerringfilter parameters (voor ring-detectie en filtering)
// ==================================================================
struct Boerringfilter {
    float r1Sq            = 0.001f * 0.001f;  // 1 mm → pow2(0.001f)
    float r2Sq            = 0.07f  * 0.07f;   // 7 cm → pow2(0.07f)
    float epsSq           = 0.001f * 0.001f;  // 1 mm tolerance
    float minAreaSq       = 1e-6f * 1e-6f;    // collinear check
    float mortonLeapSq    = 10.0f  * 10.0f;   // 10 m leap threshold
};

// ==================================================================
// RDV Voter parameters (richting-detectie en voting)
// ==================================================================
struct RdvVoter {
    float cosCutoff          = 0.2588f;  // cos(75°) voor orthogonal
    int   burstWindow        = 4;        // ring size als burst uit staat
    float planeThreshold     = 0.98f;    // cos(11.5°) voor vlak-detectie
    int   minBurstCount      = 3;        // early-stop in voting
    float recentAxisCutoff   = 15.0f;    // skip oude assen in reverse search
    float mergeToleranceDeg  = 7.5f;     // merge-tolerantie in graden
};

// ==================================================================
// RichtingStemmen (banden, gewichten, warmup/evict, EMA, clipjes)
// ==================================================================
struct RichtingStemmen {
    float bandNearCos     = 0.985f;   // ~10°
    float bandOkCos       = 0.940f;   // ~20°
    float minNearOkRatio  = 0.70f;   // ok telt mee als near/ok ≥ 0.70

    float vlakStraalGewicht = 1.00f;
    float buisStraalGewicht = 0.55f;

    int   warmupMinSamples = 200;
    int   maxSlots         = 12;
    int   targetSlots      = 3;
    float evictRatio       = 5.0f;

    float emaAlphaVlak = 0.25f;
    float emaAlphaBuis = 0.12f;

    float maxLockGraads    = 6.0f;   // reject als groter
    float clipKleinGraads  = 2.0f;   // kleine correcties negeren
};

// ==================================================================
// Specifieke toleranties per type (vlak / buis)
// ==================================================================
struct VlakToleranties {
    float slabEps = 0.010f;  // 10 mm
};

struct BuisToleranties {
    float rMin = 0.025f;  // 25 mm
    float rMax = 0.500f;  // 500 mm
};

// ==================================================================
// Hoofdconfiguratie – singleton voor thread-safe toegang
// ==================================================================
struct Config {
    Boerringfilter    boerring;
    RdvVoter          rdvVoter;
    RichtingStemmen   stemmen;
    VlakToleranties   vlak;
    BuisToleranties   buis;

    // Singleton toegang
    static const Config& get() {
        static const Config instance;  // C++11 magic static → thread-safe
        return instance;
    }

private:
    // Private constructor om directe instantiatie te voorkomen
    Config() = default;
};

} // namespace cfg

