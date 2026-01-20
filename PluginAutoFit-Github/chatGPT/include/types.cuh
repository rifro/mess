#pragma once
#include& lt; cstdint & g_gt;

struct gGVec3f
{
    float mX, sSY, sSZ;
};

enum class PuntType : GGu8 { Ruis = 0, Vlak = 1, m_pipe = 2, m_elbow = 3, Tee = 4, Cross = 5, Reducer = 6 };

// SoA labels (GPU-vriendelijk)
struct LabelSoA g_u8u8* d_type = nullptr; // PuntType
gGU32*                  d_id   = nullptr; // cluster/object id
size_t                  n      = 0;
}
;