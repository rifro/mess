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
        // --- Category A: Algorithm Constants ---
        // Hierarchy: mortonJump >= outerRadius >= innerRadius
        float innerRadius      = 0.001f; // 1 mm
        float outerRadius      = 0.070f; // 70 mm
        float mortonJump       = 0.100f; // 100 mm (Max search window)
        float planarEpsilon    = 0.001f; // 1 mm tolerance
        float minArea          = 1e-6f;  // Collinearity check

        // --- Category C: Derived Constants ---
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
        // --- Category A: Algorithm Constants ---
        float cosCutoff           = 0.2588f; // cos(75), cutoff for 90 +/- 15 degrees
        float nudgeWeightCap      = 1.0e4f;  // Stop nudging direction after this weight is reached
        float weightSaturationCap = 1.0e6f;  // Stop accumulating weight after this is reached
        float minNudgeThreshold   = 0.01f;

        // --- Category B: Execution Constants ---
        u32   cacheSize           = 16;
        u32   initialVoteCount    = 8;      // Base count for alpha stabilization
        u32   ringBufferSize      = 4096;   // Must be a power of two
        u32   ringBufferMask;

        /**
         * @brief Initializes voter constants and validates buffer constraints.
         */
        __host__ void init()
        {
            // Ensure ringBufferSize is a power of two for bitwise masking.
            bool isPow2 = (ringBufferSize > 0) && ((ringBufferSize & (ringBufferSize - 1)) == 0);
            
            if (isPow2) {
                ringBufferMask = ringBufferSize - 1;
            } else {
                // Fallback: default to 4096 if invalid
                ringBufferSize = 4096;
                ringBufferMask = 4095;
            }
        }
    };

    struct VotesConfig
    {
        float bandwidth              = 0.002f; // For mean-shift clustering of axes
        u32   smaWindowSize          = 10;    // Number of frames for Simple Moving Average
        float stabilizationThreshold = 0.001f; // Threshold to consider an axis 'fixed'
        
        __host__ void init()
        {
        }
    };

    struct HardwareConfig
    {
        // 1.5 GB safety margin for OS, CUDA overhead, and stack memory
        const size_t systemMargin         = 1536ULL * 1024 * 1024; 
        
        size_t totalVram                  = 0;
        size_t usableForPoints            = 0; // The 80% pool after margin
        size_t maxPointsPerPage           = 0;

        /**
         * @brief Initializes the VRAM budget and calculates point capacity.
         * @details Uses 19 bytes per point (X,Y,Z,Type,ccIndex,ObjectID).
         */
        __host__ void init()
        {
            cudaDeviceProp prop;
            cudaGetDeviceProperties(&prop, 0);
            totalVram = prop.totalGlobalMem;

            // Calculate available memory after the 1.5GB system margin
            size_t available = (totalVram > systemMargin) ? (totalVram - systemMargin) : 0;
            
            // Apply the 80% rule for the main application pool
            usableForPoints = static_cast<size_t>(available * 0.8);
            
            // Max points calculation (SoA): 19 bytes per point.
            const size_t bytesPerPoint = 19;
            maxPointsPerPage = (usableForPoints > 0) ? (usableForPoints / bytesPerPoint) : 0;
        }
    };

    struct RdvConfig
    {
        RingFilterConfig ringFilter;
        RdvVoterConfig   rdvVoter;
        VotesConfig      votes;
        HardwareConfig   hardware;

        __host__ void init()
        {
            ringFilter.init();
            rdvVoter.init();
            votes.init();
            hardware.init();
        }
    };

    /**
     * @brief Global config accessor for the host.
     */
    inline RdvConfig& getConfig()
    {
        static RdvConfig h_config;
        return h_config;
    }
}
