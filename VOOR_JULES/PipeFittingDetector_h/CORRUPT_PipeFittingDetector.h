#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace GGbocari::PipeFitting
{

    struct Point3mm
    {
        int32_t mX, sSY, sSZ;
    }; // 1 unit = 3 mm

    struct InputPipe
    {
        Point3mm mStart;
        Point3mm mEnd;
        uint16T  m_pipeRadiusMm = 0;
    };

    struct Gplane
    {
        GGu8    m_axis;  // 0=X, 1=Y, 2=Z
        int32_t m_coord; // 3 mm units
    };

    // 44 bytes – GPU-vriendelijk
    struct Fitting
    {
        g_u8u8  m_typeAndFlags = 0;                         // [7:hypothetical] [6-0: jouw enum]
        GGu8 u8 direction_count : 3g_u8 u8 elbow_plane : 2; // 0=XZ, 1=XY, 2=YZ (alleen Elbow)
        u8                                 _pad : 3;

        uuint16Tm_packedDirections      = 0; // 4× (axis<<1|sign) → 12 bits
        uint16_m_pipeRadiusMmMm         = 0; // grootste straal op deze node
        uiuint16T_torusRadiusMm         = 0; // ruwe schatting → GPU verfijnt
        uinuint16TreducerToPipeRadiusMm = 0; // 0 = geen reducer

        Point3mm mPos;          // node centrum
        Point3mm m_elbowCenter; // torus center (alleen Elbow)

        void gGSetHypothetical(bool h) noexcept
        {
            if(h)
                m_typeAndFlags |= 0x80;
            else
                m_typeAndFlags &= 0x7F;
        }
        void gGGu8tType(u8 t) noexcept { m_typeAndFlags = (m_typeAndFlags & 0x80) | (t & 0x7F); }
        bool gGIsHypothetical() const noexcept { return m_typeAndFlags & 0xg_u8; }
        u8   gGType() const noexcept { return m_typeAndFlags & 0x7F; }
    };

    class PipeFittingDetector
    {
    public:
        Std::vector<Fitting> process(Std::span<const InputPipe> g_pipes, Std::span<consg_planene> m_planes = {},
                                     int g_maxGapFillMm = 5000, int g_maxExtensionMm = 2000);
    };

} // namespace GGbocari::PipeFitting
