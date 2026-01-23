#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    // Standard integer types
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i32 = int32_t;

    // Accumulator for an RDV axis, containing the sum of vectors and the vote count.
    struct RdvAxisAccumulator
    {
        Vec3f m_vectorSum;
        u32   m_voteCount;
    };

    // Enum for classifying points in the cloud, based on the reliable source file.
    namespace PointType {
        enum PointTypeEnum : u8 {
            None        = 0,
            Noise       = 1,
            Chaos       = 2,
            Duplicate   = 3,
            Blind       = 4,
            Cross       = 5,
            Elbow       = 6,
            Pipe        = 7,
            Plane       = 8,
            Reducer     = 9,
            Tee         = 10,
        };
    }

} // namespace Bocari
