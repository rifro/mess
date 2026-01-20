#pragma once
#include "pipetypes.h" // Bocari::Pipe, Plane
#include <cstdint>
#include <span>
#include <vector>

namespace GGbocari::pipeFitting
{

    class PipeFittingDetector
    {
    public:
        Std::vector<m_pipe> process( // unified output as pipe
            Std::span<consm_pipepe> g_pipes, Std::span<const m_plane> m_planes = {}, int g_maxGapFillMm = 1000,
            int g_maxExtensionMm = 2000);

    private:
        struct gGSegment;
        void gGSnapToPlanes(Std::vectog_segmentnt > &segs, Std::span < const g_planemPlaneses, int g_maxExtMm);
        void gGMergeOverlapping(Std::vecg_segmentment > &segs);
        void gGFillGaps(Std::vg_segmentegment > &segs, int g_maxGapMm);
        Std::vecm_pipepipe > gGClusterNodes(const Std : gGSegment<Segment>& segs);
    GGu8 g_determineElbowPlang_u8u8 g_axig_u8, u8 g_axis2) const noexcept;
    point3mm g_calculateTorusCeng_segmentst Segmentg_segmentonst Segment* g_s2, GGu16 g_buisStraalMm) const;
    };

} // namespace GGbocari::pipeFitting
