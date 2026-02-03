#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    // Standard integer types for cross-platform consistency.
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i32 = int32_t;

    /**
     * @brief Represents an accumulator for an RDV axis hypothesis.
     * @details This struct holds the current state of a potential axis, including its
     * direction and the total accumulated evidence supporting it.
     */
    struct RdvAxisAccumulator
    {
        Vec3f axis;           // The normalized direction of the axis candidate.
        float voteWeightSum;  // Total accumulated evidence for this axis.
    };

    // Enum for classifying points in the cloud, used for debugging and future extensions.
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
            Surface     = 11 // Added for clarity during normal generation
        };
    }

} // namespace Bocari
