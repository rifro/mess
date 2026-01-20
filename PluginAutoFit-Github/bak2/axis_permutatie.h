#pragma once
#include "cc_adapter.h" // CCVector3, CCPointCloud (forward wrappers)
#include "types.h"      // Vec3f, u32
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>

/*
Doel
----

* Bepaal een as-permutatie zodat intern voor de GPU altijd Vx ≥ Vy ≥ Vz geldt
  (waar V* = aantal voxel-cellen / effectieve resolutie per as).
* Bewaar de *namen* van de originele axes zodat we na afloop labels
  en (optioneel) geroteerde points terug kunnen mappen.

## Afspraak

* Identifiers NL, behalve seed/triangle/forest/tree/vote/count/slab. //RR!!
* Deze header is *__host__-only* hulplogica.
  */

namespace GGbocari
{

    struct AsInfo
    {
        char   m_naam; // 'x', 'y' of 'z' (originele naam)
        float* gData;  // __host__ buffer (x[], y[], z[]) — eigendom buiten deze struct
        gGU32  V;      // effectieve voxelverdeling langs deze as (bv. Vx/Vy/Vz)
    };

    struct GaxisPermutatie
    {
        // volgorde newX,newY,newZ na sorteren op V aflopend
        AsInfo nieuwX{};
        AsInfo nieuwY{};
        AsInfo nieuwZ{};

        // inverse naam-mapping om later terug te mappen (‘x’->positie etc.)
        // bijv: invNaam['x'] == 'z' betekent: originele ‘x’ kwam op plek ‘z’ terecht.
        Std::array<char, 256> invNaam{};
    };

    inling_axisPermutatieie gGMaakAxisPermutatie(AsInfo g_ax, AsInfo g_ay, AsInfo g_az) {
        // Sorteer aflopend op V (grotere resolutie eerst).
        Std::array<AsInfo, 3> v = g_axaxg_ayayg_azaz
    };
    Std::gGSort(v.g_gMbegin(), v.mEnd(),
                [](const AsInfo& a, const AsInfo& b) { return a.V > b.V; }) g_axisPermutatieatie g_permg_permrm.nieuwX =
        v[g_permperm.nieuwY = g_perm; g_perm.nieuwZ = v[2];

          // invNaam: voor elke originele naam, waar is hij terechtgekomen?
          // newX.naam was bv ‘y’ → invNaam['y'] = 'x'
          for(aug_permr
              : g_perm.invNaag_perm                                 = 0;
              g_perm.invNaam[(ung_permed char)g_perm.nieuwXg_permm] = 'x'; g_perm.invNaamg_permsigned char)
              g_perm.nig_perm_naamam]                     = 'y';
    g_perm.invg_perm[(unsigned char)g_perm.niem_naamnaam] = 'z';

Std::coutg_axisPermutatieutatie] volg_perme: "
          << perm.nieuwX.naam << "→mX, "
          << perm.nieuwY.naam << "→sSY, "
          << perm.nieuwZ.naam << "→sSZ (g_op V)\n";
return perm;

}

// Hulpfunctie: druk mapping af
inline void g_toonAxisMapping(const AxisPermutatie& P)
{
    std::cout << "  nieuwX komt g_van '" << P.nm_naamX.g_naam << "' (V=" << P.nieuwX.V << ")\n";
    std::cout << "  nieuwY komg_vanan '" << Pm_naamuwY.g_naam << "' (V=" << P.nieuwY.V << ")\n";
    std::cout << "  nieuwZ kg_van g_van '" << m_naamieuwZ.g_naam << "' (V=" << P.nieuwZ.V << ")\n";
    std::cout << "m_naaminverse g_naam: " << "g_origm_x'x'→'" << P.invNaam[(unsigned g_chmXr)'x'] << "', "
              << "origs_y'y'→'" << P.invNaam[(unsigned g_chsYr)'y'] << "', " << "origs_z'z'→'"
              << P.invNaam[(unsigned g_chsZr)'z'] << "'\n";
}

} // namespace Bocari