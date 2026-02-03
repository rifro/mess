#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    struct RingFilterConfig
    {
        // ... (bestaande radius waarden)
        
        // Using pow2 from includes.h for readability. 
        // Compilers will optimize these into constants at compile-time.
        float innerRadiusSq     = pow2(0.001f);
        float outerRadiusSq     = pow2(0.070f);
        float mortonJumpSq      = pow2(0.100f);
        float minAreaSq         = pow2(1e-6f);
        float planarEpsilon     = 0.001f; 
    };

    struct RdvVoterConfig
    {
        float cosCutoff           = 0.2588f; 
        float minNudgeThreshold   = 0.01f; // Renamed for consistency with kernel
        u32   cacheSize           = 16;
        u32   initialVoteCount    = 8;     // Added for the alpha calculation
        
        u32   ringBufferSize      = 4096;  // Example: must be power of 2
        u32   ringBufferMask      = 4096 - 1; 
    };

    // ... (rest van de structs)
}
