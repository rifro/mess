#include "includes.cuh"
#pragma once

struct Vec3f
{
    float x, y, z;
};

enum class PuntType : u8 { Ruis = 0, Vlak = 1, Pipe = 2, Elbow = 3, Tee = 4, Cross = 5, Reducer = 6 };

// SoA labels (GPU-vriendelijk)
struct LabelSoA
{
    u8*  dType = nullptr; // PuntType
    u32* dId   = nullptr; // cluster/object id
    size_t    n      = 0;
};