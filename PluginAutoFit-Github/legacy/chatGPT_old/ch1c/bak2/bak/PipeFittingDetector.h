#pragma once

#include <cstdint>
#include <vector>
#include <span>

#include "pipetypes.h"

namespace Bocari::PipeFitting {

using Bocari::u8;
using Bocari::u16;
using Bocari::u32;
using Bocari::i32;
using Bocari::Point3mm;
using Bocari::Pipe;
using Bocari::PipeType;
using Bocari::pipeType;
using Bocari::isHyp;
using Bocari::makeTF;

// Optionele vlak-informatie (bijv. vloer/muur-planes) in 3mm grid
struct Plane
{
    u8  axis;   // 0=X,1=Y,2=Z
    i32 coord;  // in 3mm units
};

// Output-node (elbow, tee, cross, reducer, gapfill, blind)
// 44 bytes, GPU-vriendelijk
struct Fitting
{
    u8        typeAndFlags = 0; // [7:hyp] [6-0: PipeType]

    Point3mm  pos{};              // center van de fitting (node)
    Point3mm  elbowCenter{};     // alleen bij Elbow: torus center
    Point3mm  elbowCorner{};     // alleen bij Elbow: snijpunt axes

    u16       pipeRadiusMm = 0;          // grootste pipe op deze node
    u16       torusRadiusMm = 0;         // geschatte bochtradius (GPU verfijnt)
    u16       reducer_to_pipeRadiusMm=0; // bij Reducer: doel-straal

    u8        directionCount : 3 {0};    // 0–4
    u8        elbowPlane     : 2 {0};    // 0=XZ, 1=XY, 2=YZ
    u8        reserved        : 3 {0};

    // 4 richtingen × 3 bits = 12 bits → past in u16
    // per richting: 2 bits as (0..2) + 1 bit sign
    u16       packedDirections = 0;

    // helpers
    void setHyp(bool h) noexcept     { typeAndFlags = makeTF(type(), h); }
    void setType(PipeType t) noexcept{ typeAndFlags = makeTF(t, isHyp()); }
    bool     isHyp() const noexcept  { return Bocari::isHyp(typeAndFlags); }
    PipeType type()  const noexcept  { return pipeType(typeAndFlags); }
};

// Hoofdklasse: CPU-topologie op 3mm-grid
class PipeFittingDetector
{
public:
    // pipes: rechte buizen (3mm grid), rechtstreeks uit jouw GPU seeds
    // planes: optionele globale vlakken (vloer/muren)
    std::vector<Fitting> process(
        std::span<const Pipe>  pipes,
        std::span<const Plane> planes        = {},
        int maxGapFillMm                  = 5000,
        int maxExtensionMm                 = 2000
    );

private:
    struct Segment;

    void snap_to_planes(std::vector<Segment>& segs,
                        std::span<const Plane> planes,
                        int maxExtMm);

    void merge_overlapping(std::vector<Segment>& segs);

    void fill_gaps(std::vector<Segment>& segs,
                   int maxGapFillMm);

    void cluster_nodes(const std::vector<Segment>& segs,
                       std::vector<Fitting>& fittings);

    u8 determine_elbow_plane(u8 axis1, u8 axis2) const noexcept;

    Point3mm calculate_torus_center(const Segment* s1,
                                    const Segment* s2,
                                    u16 pipeRadiusMm) const;
};

} // namespace Bocari::PipeFitting
