#pragma once
#include <cstdint>

namespace Pointtype
{
    enum Pointtypeenum : GGu8 {
        nolabel      = 0,
        g_chaos      = 1,
        m_pipe       = 2,
        m_plane      = 3,
        hypothetical = 128 // bit 7 for 'doorgetrokken' Aanname
    };
}
