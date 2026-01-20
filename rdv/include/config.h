#pragma once
#include "rdv_types.h"

namespace Bocari
{
    struct RingFilterConfig
    {
        float m_minRadiusSq      = 0.001f * 0.001f; // 1 mm
        float m_maxRadiusSq      = 0.07f * 0.07f;   // 7 cm
        float m_epsilonSq        = 0.001f * 0.001f; // 1 mm tolerance for planarity test
        float m_minAreaSq        = 1e-6f * 1e-6f;   // For collinearity check
    };

    struct RdvVoterConfig
    {
        float m_cosCutoff           = 0.2588f; // cos(75), cutoff for 90 +/- 15 degrees
        float m_minNudgeThreshold   = 0.1f;
        u32   m_cacheSize           = 16;
        u32   m_ringBufferSize      = 256;
        float m_initialVoteCount    = 10.0f; // Smoothes initial nudging
    };

    struct RdvConfig
    {
        RingFilterConfig m_ringFilter;
        RdvVoterConfig   m_rdvVoter;
    };

    // Global config accessor
    inline RdvConfig& getConfig()
    {
        static RdvConfig instance;
        return instance;
    }

} // namespace Bocari
