#pragma once

// We werken altijd in millimeters.
// Dus: ints = round( (p - bbMin) * 1000 )
static constexpr float s_quantizeScale   = 1000.0f; // meter → mm
static constexpr int   s_quantizeMaxBits = 21;      // 0..2^21 (2M mm ≈ 2000m)
