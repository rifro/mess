#pragma once

// We werken altijd in millimeters.
// Dus: ints = round( (p - bbMin) * 1000 )
static constexpr float QuantizeScale = 1000.0f;  // meter → mm
static constexpr int   QuantizeMaxBits = 21;     // 0..2^21 (2M mm ≈ 2000m)

