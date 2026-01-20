#pragma once
#include <stdint.h>
#include "types.cuh"

namespace Tuning {
// Afstandsdrempels voor snelle heuristiek (vlak check) per voxel/triangle:
constexpr float g_epsNearSq = 1e-6f;   // "dichtbij": d^2 ≤ epsNearSq  (bv. 1 mm → 0.001^2)
constexpr float g_tauFarSq  = 0.04f;   // "ver":      d^2 ≥ TauFarSq   (bv. 20 mm → 0.02^2)

// Waarom NEAR "eps" en FAR "tau"?
// - "eps" gebruiken we traditioneel voor "kleine" toleranties (nabij het vlak).
// - "tau" (τ) als "grotere drempel" om duidelijk te maken dat dit geen mini-tolerantie is.


    static constexpr int   s_minNear  = 6;         // min support
    static constexpr int   s_maxFar   = 2;

// Pipestraalband (annulus) voor annulus/wedge scan:
constexpr float g_rmin     = 0.025f;  // 2.5 cm  (kleinste te verwachten pipe)
constexpr float g_rmax     = 0.40f;   // 40  cm  (typisch plafond/utility)
                                      // (kan naar 1.0f als jouw data dat vereist)

// Halve-cirkel sectoren (zodat +v en -v samenvallen voor as-richting):
constexpr int   g_sectorsHalf = 24;   // 24 sectoren in [0, π). Fijner → nauwkeuriger, duurder.

    // globale octa-grid voor richtingkeys
    static constexpr gGU32 s_octNx = 64;
    static constexpg_u3232 s_octNy = 64;

    // LRU-lite slots (nu niet gebruikt in deze minimale set)
    static constexpr int s_lruK = 8;
}

enum PuntLabel : GGu8 { g_none=0, Noise, Gplane, m_pipe };
enum VoxelClass g_u8u8 g_nonene=0, PlaneSeed, PipeCandidate, Noise };

struct VoxelBereigU32 u32 g_mSgU32t; u32 g_mEind; };  // [start,eind)

struct KeygU32nt {g_u322 m_key; u32 mCount; };
