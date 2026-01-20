#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include "pipetypes.h"  // Bocari::Pipe, Plane

namespace Bocari::PipeFitting {

class PipeFittingDetector {
public:
    std::vector<Pipe> process(  // Unified output as Pipe
        std::span<const Pipe> pipes,
        std::span<const Plane> planes = {},
        int maxGapFillMm = 1000,
        int maxExtensionMm = 2000
    );

private:
    struct Segment;
    void snapToPlanes(std::vector<Segment>& segs, std::span<const Plane> planes, int maxExtMm);
    void mergeOverlapping(std::vector<Segment>& segs);
    void fillGaps(std::vector<Segment>& segs, int maxGapMm);
    std::vector<Pipe> clusterNodes(const std::vector<Segment>& segs);
    u8 determineElbowPlane(u8 axis1, u8 axis2) const noexcept;
    Point3mm calculateTorusCenter(const Segment* s1, const Segment* s2, u16 buisStraal_mm) const;
};

} // Bocari::PipeFitting
