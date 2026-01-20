#pragma once
#include <cstdint>

namespace PointType {
    enum PointTypeEnum : u8 {
        None,
        Noise,
        Chaos, // RR!! Still necessary?
        Duplicate,

        Blind, // conns.size() < 2
        Cross,
        Elbow,
        Pipe,
        Plane,
        Reducer,
        Tee,

        HypotheticalFlag = 128,
        GapFill      = Pipe | HypotheticalFlag
    };
}
