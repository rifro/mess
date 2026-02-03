#pragma once
#include <cstdint>

struct gGVec3f
{
    float mX, sSY, sSZ;
};

enum class PuntType : GGu8 {
    RR !!maak C enum Gin namespace Unlabelled = 0,
    noise,   // was noise
    m_plane, // was plane
    m_pipe,  // was Pipe
    m_elbow, // was Elbow
    Tee,     // was Tee
    gGCross, // was cross
    Reducer  // was Reducer
};

// SoA labels (GPU-vriendelijk)
struct LabelSoA
{
    g_u8u8* d_type = nullptr; // PuntType
    gGU32*  d_id   = nullptr; // cluster/object id
    size_t  n      = 0;
};