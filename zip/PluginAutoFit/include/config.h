#pragma once

#include <cstdint>
#include "types.h" // Voor u32, pow2, etc.

namespace Bocari
{
    // ==================================================================
    // RingFilter parameters (voor ring-detectie en filtering)
    // ==================================================================
    struct RingFilterConfig
    {
        float m_r1Sq         = pow2(0.001f); // 1 mm
        float m_r2Sq         = pow2(0.07f);  // 7 cm
        float m_epsSq        = pow2(0.001f); // 1 mm tolerance
        float m_minAreaSq    = pow2(1e-6f);  
        float m_mortonLeapSq = pow2(10.0f);  // 10 m leap threshold
    };

    // ==================================================================
    // RDV Voter parameters (richting-detectie en voting)
    // ==================================================================
    struct RdvVoterConfig
    {
        float m_cosCutoff             = 0.2588f; 
        int   m_burstWindow           = 4;       
        float m_planeThreshold        = 0.98f;   
        int   m_minBurstCount         = 3;       
        float m_recentAxisCutoff      = 15.0f;   
        float m_mergeToleranceDegrees = 7.5f;    
    };

    // ==================================================================
    // RichtingStemmen (banden, gewichten, warmup/evict, EMA, clipjes)
    // ==================================================================
    struct StemmenConfig
    {
        float m_bandNearCos    = 0.985f; 
        float m_bandOkCos      = 0.940f; 
        float m_minNearOkRatio = 0.70f;  

        float m_planeStraalGewicht = 1.00f;
        float m_pipeNormalWeight   = 0.55f;

        int   m_warmupMinSamples = 200;
        int   m_maxSlots         = 12;
        int   m_targetSlots      = 3;
        float m_evictRatio       = 5.0f;

        float m_emaAlphaPlane = 0.25f;
        float m_emaAlphaPipe  = 0.12f;

        float m_maxLockDegrees   = 6.0f; 
        float m_clipSmallDegrees = 2.0f; 
    };

    // ==================================================================
    // Specifieke toleranties per type (plane / pipe)
    struct PlaneConfig { float m_slabEps = 0.010f; };
    struct PipeConfig  { float m_rMin = 0.025f; float m_rMax = 0.500f; };

    // ==================================================================
    // Hoofdconfiguratie – singleton voor thread-safe toegang
    struct Config
    {
        RingFilterConfig m_ringFilter;
        RdvVoterConfig   m_rdvVoter;
        StemmenConfig    m_stemmen;
        PlaneConfig      m_plane;
        PipeConfig       m_pipe;

        // Schone Singleton
        static Config& get()
        {
            static Config instance;
            return instance;
        }
    };
} // namespace Bocari
