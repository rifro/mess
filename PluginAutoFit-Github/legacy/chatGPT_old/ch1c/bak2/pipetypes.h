#pragma once
#include <cstdint>

namespace Bocari {

using u8  = std::u8;
using u16 = std::uint16_t;
using u32 = std::u32;
using i32 = std::int32_t;

// 3mm grid point: 1 unit = 3 mm
struct Point3mm
{
    i32 x{};
    i32 y{};
    i32 z{};

    bool operator==(const Point3mm&) const = default;
};

// Gemeenschappelijke type voor alles wat we willen labelen
// (points, pipes, fittings)
enum PipeType : u8
{
    None = 0,
    Noise     = 1,
    Plane     = 2,
    Pipe      = 3,
    Elbow     = 4,
    Tee       = 5,
    Cross     = 6,
    Reducer   = 7,
    GapFill   = 8, // hypothetische pipe (topologisch afgeleid)
    Blind     = 9  // doodlopend knooppunt
    // ruimte zat voor uitbreiding
};

// Hoogste bit = "hypothetical" vlag
constexpr u8 Hypothetical = 0x80;

inline PipeType pipeType(u8 tf) noexcept
{
    return static_cast<PipeType>(tf & 0x7F);
}
inline bool isHyp(u8 tf) noexcept
{
    return (tf & Hypothetical) != 0;
}
inline u8 makeTF(PipeType t, bool hyp) noexcept
{
    return (static_cast<u8>(t) & 0x7F) | (hyp ? Hypothetical : 0);
}

// Rechte pipe in 3mm grid
// - start/end: 3mm grid
// - pipeRadiusMm: straal in mm
// - axisIndex: 0=X,1=Y,2=Z in ideale frame
// - confidence: 0..255 (optioneel)
// - clusterId: voor latere labeling
struct Pipe
{
    Point3mm start;
    Point3mm end;
    u16      pipeRadiusMm{};
    u8       axisIndex{};   // 0,1,2
    u8       confidence{};  // 0..255
    u32      clusterId{};   // 0 = nog geen cluster
};

} // namespace Bocari
