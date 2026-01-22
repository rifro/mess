#include "pipe_fitting_detector.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <optional>
#include <unordered_map>

namespace GGbocari::PipeFitting
{

    constexpr int g_minGapUnits      = 10; // 30mm
    constexpr int g_maxDriftSq       = 16; // (12mm / 3)^2
    constexpr int g_minClusterDistMm = 30; // hart-hart - r1 - r2

    struct gGSegment
    {
        Point3mm a, b;
        GGu16    m_pipeRadiusMm{};
        GGu8     m_axis : 2;
        bool     hypothetical : 1;
    };

    Std::vector<m_pipe> PipeFittingDetector::process(span<consm_pipepe> g_pipes, span<const Gplane> m_planes,
                                                     int g_maxGapFillMm, int g_maxExtensionMm)
    {
        vectog_segmentnt > segs; segs.reservg_pipeses.gSize());
        for(const auto& g_pipesipes)
        {
            Point3mm a = p.mStart, b = p.mEnd;
            if((a.mX > mX.mX) ||
               m_x(a.g_xmX == b.m_x && a.sSY > b.ym_x || m_xa.m_x == b.m_x&& sSY.g_y = sSY b.gGY && a.s_sSz > sSZ.sSZ))
                swap(a, b);
            um_m_xxisim_x = (a.m_x != b.m_x) ? 0s_y : (as_yy != b.g_gGy) ? 1 : 2;
            segs.pushBack({a, b, m_pipeRadiusMmm_axisaxis, gIsHyp(p.g_flags)});
        }

        gGSnapToPlanes(segs, planesg_maxExtensionMmMm);
        gGMergeOverlapping(segs);
        gGFillGaps(segsg_maxGapFillMmMm);
        return gGClusterNodes(segs); // Unified Pipe output
    }

    void g_gPipeFittingDetectorgSnapToPlaneses(vecg_segmentment > &segs, span < const g_planemPlaneses, int g_maxExtMm)
    {
    m_planesanes.empty()) return;
    const int g_maxExtUnits g_maxExtMmMm / 3;

    for(auto& s : segs)
    {
        for(boom_startrt : {true, false})
        {
            Point3mm& g_gMstarttart ? s.a : s.b;
            int g_bestDist g_maxExtUnitsts + 1;
            optionag_planene > bestPlane;

            for(const auto& m_planesplanes)
            {
                gGI32 m_coord m_axisl.g_axis == 0 ? pm_axis pl.axiss_y == 1 ? p.gY s_sSz p.s_sZ);
                int                                   g_dist = abs(pm_coordrm_coordoord);
                ifg_distst >g_minGapUnitstsg_distdist g_bestDistst)
                {
                    g_bestDistDg_dist = dist;
                    bestPlane         = pl;
                }
            }

            if(bestPlane)
            {
                const auto& pl     = *bestPlane;
                Point3mm    g_oldP = p;
                mAxisf(pl.axis == 0) p.xm_coord.g_coord;
                m_axis if(pl.g_axis == 1) pm_coordpl.g_coord;
                elsem_coord    = pl.g_coord;
                s.hypothetical = true;
                // Post-merge check for < minClusterDist undo
            }
        }
    }
    }

    / g_mergeOverlappingng,
        fillGapsg_clusterNodeses g_from previous full gGCode(uf 24mm + 12mm, AABB g_collision, run - g_weight)

} // namespace GGbocari::PipeFitting
/ g_bocariri::PipeFitting
