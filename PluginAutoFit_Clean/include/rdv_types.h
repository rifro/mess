#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
<<<<<<< HEAD
    // Standard integer types for cross-platform consistency.
=======
    // Standaard integer types
>>>>>>> origin/master
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i32 = int32_t;

<<<<<<< HEAD
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
=======
>>>>>>> origin/master
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

    /**
     * @brief De centrale 20-byte GPU structuur.
     * @details Compacte layout geoptimaliseerd voor 4-byte alignment.
     * X, Y, Z zijn signed integers (i32) om negatieve waarden na rotatie te ondersteunen.
     */
    struct GpuPoint {
        i32 x, y, z;      // 12 bytes: Millimeters (signed voor rotatie)
        u8  type;         // 1 byte:  PointTypeEnum
        u8  objectID[3];  // 3 bytes: 24-bit uniek object/cluster nummer
        u32 ccIndex;      // 4 bytes: Onveranderlijk anker naar de bron-cloud
    };

    /**
     * @brief Accumulator voor RDV as-detectie.
     * @details Gebruikt float gewichten op basis van sin^16(theta) voor precisie.
     */
    struct RdvAxisAccumulator
    {
        Vec3f m_vectorSum;     // Gewogen som van normaal-vectoren
        float m_voteWeightSum; // Totale som van alle float weights
    };

} // namespace Bocari
