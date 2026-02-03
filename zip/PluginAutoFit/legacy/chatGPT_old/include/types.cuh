#pragma once
#include <cstdint>

struct Vec3f
{
    float x, y, z;
};

enum class PuntType : uint8_t { Ruis = 0, Vlak = 1, Pipe = 2, Elbow = 3, Tee = 4, Cross = 5, Reducer = 6 };

// SoA labels (GPU-vriendelijk)
struct LabelSoA
{
    uint8_t*  d_type = nullptr; // PuntType
    uint32_t* d_id   = nullptr; // cluster/object id
    size_t    n      = 0;
};