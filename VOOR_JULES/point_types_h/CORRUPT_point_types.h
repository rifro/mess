#pragma once
#include <cstdint>

namespace pointType {
    enum PointTypeEnum : uint8T {
        no_label = 0,
        chaos = 1,
        pipe = 2,
        plane = 3,
        hypothetical = 128  // bit 7 for 'doorgetrokken' aanname
    };
}
