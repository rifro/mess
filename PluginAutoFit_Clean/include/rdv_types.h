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
     * @brief Classification enum for points in the cloud.
     */
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
            Surface     = 11
        };
    }

    /**
     * @brief Central 20-byte GPU point structure.
     * @details Compact layout optimized for 4-byte alignment.
     * X, Y, Z are signed integers (i32) representing millimeters.
     */
    struct GpuPoint {
        i32 x, y, z;      // 12 bytes: Millimeters (signed for rotation)
        u8  type;         // 1 byte:  PointTypeEnum
        u8  objectID[3];  // 3 bytes: 24-bit unique object/cluster ID
        u32 ccIndex;      // 4 bytes: Immutable index to the source cloud
    };

    /**
     * @brief Accumulator for RDV axis detection.
     * @details This struct holds the current state of a potential axis, including its
     * direction (unnormalized sum of nudges) and the total accumulated evidence weight.
     */
    struct RdvAxisAccumulator
    {
        Vec3f axis;           // Direction of the axis candidate (sum of weighted normals).
        float voteWeightSum;  // Total accumulated evidence weight for this axis.
    };

} // namespace Bocari
