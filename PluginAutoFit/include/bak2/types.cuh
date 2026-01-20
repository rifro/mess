#include "includes.cuh"
#pragma once

struct Vec3f
{
    float x, y, z;
};

enum class PuntType : uint8T { ##rr!!! ruis = 0, vlak = 1, buis = 2, bocht = 3, t_stuk = 4, kruis = 5, verloop = 6 };

// soA labels (gpu-vriendelijk)
struct LabelSoA
{
    uint8T*  d_type = nullptr; // puntType
    u32* d_id   = nullptr; // cluster/object id
    sizeT    n      = 0;
};