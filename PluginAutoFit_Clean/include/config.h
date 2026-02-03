#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    // Helper for squared value calculation
    template <typename T>
    __host__ __device__ constexpr T pow2(T x) { return x * x; }

    struct RingFilterConfig
    {
        // --- Categorie A: Algoritme Constanten ---
        // Hierarchy: mortonJump >= outerRadius >= innerRadius
        float innerRadius      = 0.001f; // 1 mm
        float outerRadius      = 0.070f; // 70 mm
        float mortonJump       = 0.100f; // 100 mm (Max search window)
        float planarEpsilon    = 0.001f; // 1 mm tolerance
        float minArea          = 1e-6f;  // Collinearity check

        // --- Categorie C: Afgeleide Constanten ---
        // Derived squared values for GPU performance
        float innerRadiusSq;
        float outerRadiusSq;
        float mortonJumpSq;
        float planarEpsilonSq;
        float minAreaSq;

        /**
         * @brief Pre-calculates squared values to avoid sqrt() and redundant multiplications in CUDA kernels.
         * This must be called on the host before copying the config to the device.
         */
        __host__ void init()
        {
            innerRadiusSq   = pow2(innerRadius);
            outerRadiusSq   = pow2(outerRadius);
            mortonJumpSq    = pow2(mortonJump);
            planarEpsilonSq = pow2(planarEpsilon);
            minAreaSq       = pow2(minArea);
        }
    };

    struct RdvVoterConfig
    {
        // --- Categorie A: Algoritme Constanten ---
        float cosCutoff           = 0.2588f; // cos(75), cutoff for 90 +/- 15 degrees
        float nudgeWeightCap      = 1.0e4f;  // Stop nudging direction after this weight is reached
        float weightSaturationCap = 1.0e6f;  // Stop accumulating weight after this is reached

        // --- Categorie B: Executie Constanten ---
        u32   cacheSize           = 16;
        u32   ringBufferSize      = 8; // Must be a power of two

        // --- Categorie C: Afgeleide Constanten ---
        // This function must be executed by the host compiler, but its result is used in device code.
        static constexpr u32 getRingBufferMask()
        {
            return ringBufferSize - 1;
        }
    };

    struct Config
    {
        RingFilterConfig ringFilter;
        RdvVoterConfig   rdvVoter;

        __host__ void init()
        {
            ringFilter.init();
        }
    };

    // Global config accessor for the host
    inline Config& getConfig()
    {
        static Config h_config;
        return h_config;
    }

} // namespace Bocari
