#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    struct RingFilterConfig
    {
        float minRadiusSq      = 0.001f * 0.001f; // 1 mm
        float maxRadiusSq      = 0.07f * 0.07f;   // 7 cm
        float epsilonSq        = 0.001f * 0.001f; // 1 mm tolerance for planarity test
        float minAreaSq        = 1e-6f * 1e-6f;   // For collinearity check
    };

    struct RdvVoterConfig
    {
        float cosCutoff           = 0.2588f; // cos(75), cutoff for 90 +/- 15 degrees
        float minNudgeThreshold   = 0.1f;
        u32   cacheSize           = 16;
        u32   ringBufferSize      = 8; // Must be a power of two
        float initialVoteCount    = 10.0f; // Smoothes initial nudging

        // This function must be executed by the host compiler, but its result is used in device code.
        // It calculates the bitwise mask for the ring buffer's circular addressing.
        static constexpr u32 getRingBufferMask()
        {
            return ringBufferSize - 1;
        }
    };

    struct RdvConfig
    {
        RingFilterConfig ringFilter;
        RdvVoterConfig   rdvVoter;
    };

    // Global config accessor
    inline RdvConfig& getConfig()
    {
        static RdvConfig instance;
        return instance;
    }

} // namespace Bocari
