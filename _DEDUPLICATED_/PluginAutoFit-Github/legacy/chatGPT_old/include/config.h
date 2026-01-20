#pragma once
#include <cstdint>

namespace cfg {

// Globale schakelaars voor richting-stemmen
struct RichtingStemmen {
// banden op cosine(|cos(theta)|); near strakker dan ok
float bandNearCos   = 0.985f;   // ~10° → 0.9848
float bandOkCos     = 0.940f;   // ~20° → 0.9397
float minNearOkRatio = 0.70f;   // ok telt mee als near/ok ≥ 0.70


// gewichten (vlak zwaarder dan pipe)
float vlakStraalGewicht = 1.00f;
float pipeNormalWeight= 0.55f;

// warmup & evict
int   warmupMinSamples = 200;
int   maxSlots         = 12;     // initieel veel
int   targetSlots      = 3;      // na evict naar 3 (1..3)
float evictRatio       = 5.0f;   // evict als score[i]*5 < maxScore

// EMA nudge
float emaAlphaPlane   = 0.25f;
float emaAlphaPipe   = 0.12f;

// Clipjes
float maxLockGraads  = 6.0f;     // reject als groter dan dit
float clipKleinGraads= 2.0f;     // kleine correcties negeren als < 2°


};

// Voor later: pipe/vlak specifieke tolerantievelden
struct VlakToleranties {
float slabEps = 0.010f;  // 10 mm
};

struct PipeToleranties {
float rMin   = 0.025f;   // 25 mm
float rMax   = 0.500f;   // 500 mm
};

struct Config {
RichtingStemmen stemmen;
VlakToleranties vlak;
PipeToleranties pipe;
};

inline const Config& get() {
static Config C;
return C;
}

} // namespace cfg