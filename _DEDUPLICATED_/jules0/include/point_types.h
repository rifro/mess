#pragma once
#include <cstdint>

namespace PointType {
    enum PointTypeEnum : uint8_t {
        None = 0,
        Noise     = 1,
        Plane     = 2,
        Pipe      = 3,
        Elbow     = 4,
        Tee       = 5,
        Cross     = 6,
        Reducer   = 7,
        Blind     = 8,
        Chaos     = 9,
        Duplicate = 10,

        HypotheticalFlag = 128,
        GapFill      = Pipe | HypotheticalFlag
    };
}
