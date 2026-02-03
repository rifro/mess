#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
<<<<<<< HEAD
    // Helper for squared value calculation
    template <typename T>
    __host__ __device__ constexpr T pow2(T x) { return x * x; }

    struct RingFilterConfig
    {
        // --- Categorie A: Algoritme Constanten ---
=======
struct RingFilterConfig
    {
>>>>>>> origin/master
        // Hierarchy: mortonJump >= outerRadius >= innerRadius
        float innerRadius      = 0.001f; // 1 mm
        float outerRadius      = 0.070f; // 70 mm
        float mortonJump       = 0.100f; // 100 mm (Max search window)
<<<<<<< HEAD
        float planarEpsilon    = 0.001f; // 1 mm tolerance
        float minArea          = 1e-6f;  // Collinearity check

        // --- Categorie C: Afgeleide Constanten ---
=======
        
        float planarEpsilon    = 0.001f; // 1 mm tolerance
        float minArea          = 1e-6f;  // Collinearity check

>>>>>>> origin/master
        // Derived squared values for GPU performance
        float innerRadiusSq;
        float outerRadiusSq;
        float mortonJumpSq;
<<<<<<< HEAD
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
=======
        float minAreaSq;

        /**
         * @brief Pre-calculates squared values to avoid sqrt() operations in CUDA kernels.
         */
        void init()
        {
            innerRadiusSq = pow2(innerRadius);
            outerRadiusSq = pow2(outerRadius);
            mortonJumpSq  = pow2(mortonJump);
            minAreaSq     = pow2(minArea);
>>>>>>> origin/master
        }
    };

    struct RdvVoterConfig
    {
<<<<<<< HEAD
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
=======
        float cosCutoff           = 0.2588f; // cos(75 degrees)
        float minNudgeThreshold  = 0.01f;
        u32   cacheSize           = 16;
        u32   initialVoteCount    = 8;      // Base count for alpha stabilization
        
        u32   ringBufferSize      = 4096;   // Must be power of 2
        u32   ringBufferMask;

        /**
         * @brief Initializes voter constants and validates buffer constraints.
         */
        void init()
>>>>>>> origin/master
        {
            // Ensure ringBufferSize is a power of two for bitwise masking.
            bool isPow2 = (ringBufferSize > 0) && ((ringBufferSize & (ringBufferSize - 1)) == 0);
            
            // If ringBufferSize is changed at runtime, use a runtime check/assert.
            if (isPow2) {
                ringBufferMask = ringBufferSize - 1;
            } else {
                // Fallback or Error: default to 4096 if invalid
                ringBufferSize = 4096;
                ringBufferMask = 4095;
            }
        }
    };

    struct VotesConfig
    {
        float bandwidth        = 0.002f; // Voor mean-shift clustering van assen
        u32   smaWindowSize    = 10;    // Aantal frames voor de Simple Moving Average
        float stabilizationThreshold = 0.001f; // Wanneer we de as als 'fixed' beschouwen
        
        void init()
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
        void init()
        {
            cudaDeviceProp prop;
            cudaGetDeviceProperties(&prop, 0);
            totalVram = prop.totalGlobalMem;

            // Calculate available memory after the 1.5GB system margin
            size_t available = (totalVram > systemMargin) ? (totalVram - systemMargin) : 0;
            
            // Apply the 80% rule for the main application pool
            usableForPoints = static_cast<size_t>(available * 0.8);
            
            /**
             * Max points calculation (SoA):
             * 3x i32(12) + 1x u8(1) + 1x u32(4) + 1x u16(2) = 19 bytes per point.
             */
            const size_t bytesPerPoint = 19;
            maxPointsPerPage = (usableForPoints > 0) ? (usableForPoints / bytesPerPoint) : 0;
        }
    };

    struct Config
    {
        RingFilterConfig ringFilter;
        RdvVoterConfig   rdvVoter;
<<<<<<< HEAD

        __host__ void init()
        {
            ringFilter.init();
        }
    };

    // Global config accessor for the host
    inline Config& getConfig()
=======
        VotesConfig      votes;
        HardwareConfig   hardware;

        void init()
        {
            ringFilter.init(); // Bereken Sq waarden
            rdvVoter.init();   // Bereken Masker
            votes.init();      // No-op
            hardware.init();   // Bereken VRAM budget for points in bytes (1.5GB margin, 80% pool, 19 bytes/pt)
        }
    };

    inline RdvConfig& getConfig()
>>>>>>> origin/master
    {
        static Config h_config;
        return h_config;
    }
}
