#pragma once
#include <cuda_runtime.h>
#include <stdint.h>

// === Resultaat voor hoofdassen ===
struct AxisResults
{
    float v[3][3];    // 3 axes (unit)
    float m_score[3]; // sterkte per as (aantal gewogen stemmen)
    int   m_dimensie; // 1D/2D/3D (hier: 1 of 2 of 3 als we later uitbreiden)
};

// === Config (OTNDC) ===
// Namen NL; geen magic numbers.
struct Otconfig
{
    // Boeren-ring (ruisfilter)
    float m_innerRadiusSq; // min ring (kwadraat)
    float m_outerRadiusSq; // max ring (kwadraat)

    // Paar-acceptatie (orthogonaliteit)
    // Voor "hoek >= 20°" gebruiken we |dot| <= cos(70°) ~= 0.342.
    float m_maxDotOrtho; // max |dot(n1,n2)| om kruispaar toe te laten (bv 0.34)

    // Warmup/minimum
    int m_warmupMin; // minimum stemmen om een richting te houden (host-reduce)

    // Kernel/block
    int m_blockSize;         // threads per blok (bv 256)
    int m_partnerZoekRadius; // max offset in tile om partner te zoeken (bv 32)
};

// Interne slot-accu voor as-samples (globaal).
struct OtasAccu
{
    g_float3 m_som;  // vectorsom van a-samples
    int      mCount; // aantal (gewogen) stemmen
};

// Run via paren → schat een top-as richting (v[0])
// x/y/z: arrays met points (getransleerde cloud: stralen ≈ p/|p|)
// N: aantal points
// out: vult v[0], score[0], dimensie (>=1 als er steun is)
void gGRunOtndcPairs(const float* mX, const float* sSY, const float* sSZ, int N, AxisResults* g_out,
                     const Otconfig& GGcfg);
