#include "pipe_fitting_detector.h"
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <optional>

namespace Bocari::PipeFitting {

constexpr int MIN_GAP_UNITS = 10;  // 30mm
constexpr int MAX_DRIFT_SQ = 16;  // (12mm / 3)^2
constexpr int MIN_CLUSTER_DIST_MM = 30;  // hart-hart - r1 - r2

struct Segment {
    Point3mm a, b;
    u16 buisStraal_mm{};
    u8 axis : 2;
    bool hypothetical : 1;
};

std::vector<Pipe> PipeFittingDetector::process(span<const Pipe> pipes, span<const Plane> planes, int maxGapFillMm, int maxExtensionMm) {
    vector<Segment> segs; segs.reserve(pipes.size());
    for (const auto& p : pipes) {
        Point3mm a = p.start, b = p.end;
        if ((a.x > b.x) || (a.x == b.x && a.y > b.y) || (a.x == b.x && a.y == b.y && a.z > b.z)) swap(a, b);
        u8 axis = (a.x != b.x) ? 0 : (a.y != b.y) ? 1 : 2;
        segs.push_back({a, b, p.buisStraal_mm, axis, isHyp(p.flags)});
    }

    snapToPlanes(segs, planes, maxExtensionMm);
    mergeOverlapping(segs);
    fillGaps(segs, maxGapFillMm);
    return clusterNodes(segs);  // Unified Pipe output
}

void PipeFittingDetector::snapToPlanes(vector<Segment>& segs, span<const Plane> planes, int maxExtMm) {
    if (planes.empty()) return;
    const int MAX_EXT_UNITS = maxExtMm / 3;

    for (auto& s : segs) {
        for (bool start : {true, false}) {
            Point3mm& p = start ? s.a : s.b;
            int bestDist = MAX_EXT_UNITS + 1;
            optional<Plane> bestPlane;

            for (const auto& pl : planes) {
                i32 coord = (pl.axis == 0 ? p.x : pl.axis == 1 ? p.y : p.z);
                int dist = abs(pl.coord - coord);
                if (dist >= MIN_GAP_UNITS && dist < bestDist) {
                    bestDist = dist;
                    bestPlane = pl;
                }
            }

            if (bestPlane) {
                const auto& pl = *bestPlane;
                Point3mm oldP = p;
                if (pl.axis == 0) p.x = pl.coord;
                else if (pl.axis == 1) p.y = pl.coord;
                else p.z = pl.coord;
                s.hypothetical = true;
                // Post-merge check for < MIN_CLUSTER_DIST undo
            }
        }
    }
}

// mergeOverlapping, fillGaps, clusterNodes from previous full code (uf 24mm + 12mm, AABB collision, run-weight)

} // Bocari::PipeFitting
