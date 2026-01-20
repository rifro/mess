#pragma once
#include <cstdint>
#include <string>
#include <cuda_runtime.h>

#pragma pack(push, 1)
struct RdvConfig {
    struct Mathematical {
        constexpr static float PI = 3.14159265359f;
        constexpr static float DEG_TO_RAD = PI / 180.0f;
        constexpr static float cos75 = 0.2588190451f;
    } math;

    struct Global
    {
        float minNudgeThreshold = 0.1f;
        int   cacheSizeK        = 16;
        int   ringBufferN        = 8;
        float initCount          = 1.0f;
    } global;

    struct RingFilter
    {
        float r1Sq       = 0.000001f;
        float r2Sq       = 0.0049f; // 7cm
        float epsSq      = 1e-6f;
        float minAreaSq = 1e-12f;
    } ringFilter;

    struct RdvVoter
    {
        float cosCutoff      = 0.2588f;
        int   burstWindow    = 4;
        float planeThreshold = 0.98f;
        int   minBurstCount = 3;
    } rdvVoter;
};
#pragma pack(pop)

__constant__ RdvConfig d_config;

inline RdvConfig loadConfig()
{
    RdvConfig cfg;
    // In the future, this will load from JSON
    return cfg;
}
