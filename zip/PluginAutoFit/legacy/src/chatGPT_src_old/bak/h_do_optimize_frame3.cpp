#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "__device__buffer.cuh" // DeviceBuffer<T>
#include "axis_permutatie.h"    // AxisPermutatie
#include "cc_adapter.h"         // CCVector3, CCPointCloud*
#include "consts.cuh"           // thresholds/__constant__en
#include "merge_split.cuh"      // h_merge_xyz_naar_vec3 / h_split_vec3_naar_xyz
#include "orchestratie.h"       // NL orchestratie-functies
#include "types.h"              // Vec3f, u32

// Kernels/__host__-stappen (geleverde modules)
extern void h_voxelizeMorton(/*in*/ const Bocari::DeviceBuffer<Bocari::Vec3f>& d_points,
                                /*io*/ Bocari::DeviceBuffer<u64>&                 d_keys,
                                /*io*/ Bocari::DeviceBuffer<u32>&                 d_index,
                                /*out*/ Bocari::DeviceBuffer<Bocari::Vec3f>&      d_pointsSorted);

extern void h_noiseFilter(/*io*/ Bocari::DeviceBuffer<Bocari::Vec3f>& d_pointsSorted,
                          /*io*/ Bocari::DeviceBuffer<u8>&            d_labels);

extern void h_vlakVoteCurrentFrame(/*in*/ const Bocari::DeviceBuffer<Bocari::Vec3f>& d_pointsSorted,
                                      /*io*/ Bocari::DeviceBuffer<u8>&                  d_labels,
                                      /*out*/ Bocari::CandidateAxes&                   uit);

extern void h_planeVoteBruteforce(/*in*/ const Bocari::DeviceBuffer<Bocari::Vec3f>& d_pointsSorted,
                                   /*io*/ Bocari::DeviceBuffer<u8>&                  d_labels,
                                   /*out*/ Bocari::CandidateAxes&                   uit);

// NB: pipe_vote happy path pas inzetten als nodig
extern void h_pipeVoteBruteforce(/*in*/ const Bocari::DeviceBuffer<Bocari::Vec3f>& d_pointsSorted,
                                   /*io*/ Bocari::DeviceBuffer<u8>&                  d_labels,
                                   /*out*/ Bocari::CandidateAxes&                   uit);

extern void h_rotatieQuatApply(/*io*/ Bocari::DeviceBuffer<Bocari::Vec3f>& d_pointsSorted,
                                 /*in*/ const Bocari::Quat&                  q);

extern void h_fineSlabsCylLabel(/*io*/ Bocari::DeviceBuffer<Bocari::Vec3f>& d_pointsSorted,
                                   /*io*/ Bocari::DeviceBuffer<u8>&            d_labels);

// ============= HOOFDFLOW =============

void AutofitImpl::doOptimizeFrame3()
{
    using namespace Bocari;

    if(!mSelectedCloud || mSelectedCloud->size() == 0)
    {
        std::cout << "[Autofit] Geen point cloud of leeg.\n";
        return;
    }

    // 1) __host__: lees CC-points en bepaal as-verdeling (Vx/Vy/Vz) + permutatie
    CCVector3 bbMin, bbMax;
    mSelectedCloud->getBoundingBox(bbMin, bbMax);

    const size_t       N = mSelectedCloud->size();
    std::vector<float> hX(N), hY(N), hZ(N);

    for(size_t i = 0; i < N; ++i)
    {
        const CCVector3* p = mSelectedCloud->getPoint(i);
        if(!p) continue;
        hX[i] = p->x;
        hY[i] = p->y;
        hZ[i] = p->z;
    }

    // Bepaal Vx,Vy,Vz (bijv via morton-driven voxelisatie ontwerp; hier simpele placeholder:
    // neem verhouding van BB-lengtes t.o.v. target gridgrootte)
    const float Lx = bbMax.x - bbMin.x;
    const float Ly = bbMax.y - bbMin.y;
    const float Lz = bbMax.z - bbMin.z;

    // Ruime schatting: grotere lengte → grotere V
    u32 Vx = std::max(1u, (u32)(Lx > 0 ? 1024.f * (Lx / std::max({Lx, Ly, Lz})) : 1));
    u32 Vy = std::max(1u, (u32)(Ly > 0 ? 1024.f * (Ly / std::max({Lx, Ly, Lz})) : 1));
    u32 Vz = std::max(1u, (u32)(Lz > 0 ? 1024.f * (Lz / std::max({Lx, Ly, Lz})) : 1));

    Bocari::AsInfo ax{'x', hX.data(), Vx};
    Bocari::AsInfo ay{'y', hY.data(), Vy};
    Bocari::AsInfo az{'z', hZ.data(), Vz};

    AxisPermutatie perm = maakAxisPermutatie(ax, ay, az);
    toonAxisMapping(perm);

    // 2) Upload nieuwe x/y/z volgens permutatie
    DeviceBuffer<float> dX(N), dY(N), dZ(N);
    dX.upload(perm.nieuwX.data, N);
    dY.upload(perm.nieuwY.data, N);
    dZ.upload(perm.nieuwZ.data, N);

    // 3) Merge naar vec3f (__device__) → d_points
    DeviceBuffer<Vec3f> d_points(N);
    hMergeXyzNaarVec3(dX, dY, dZ, d_points);

    std::cout << "[Autofit] Merge x/y/z → vec3f gedaan. Start pipeline…\n";

    // 4) Morton-driven sort + noise filter
    DeviceBuffer<u64>   d_keys(N);
    DeviceBuffer<u32>   d_index(N);
    DeviceBuffer<Vec3f> d_pointsSorted(N);
    DeviceBuffer<u8>    d_labels(N);
    d_labels.memset(0);

    h_voxelizeMorton(d_points, d_keys, d_index, d_pointsSorted);
    h_noiseFilter(d_pointsSorted, d_labels);

    // 5) Probeer huidig frame (3 richtingen) → vlakken
    CandidateAxes axes{};
    h_vlakVoteCurrentFrame(d_pointsSorted, d_labels, axes);

    if(!axes.heeftMinstensEenAs())
    {
        std::cout << "[Autofit] Geen vlak in huidig frame. Brute planes…\n";
        h_planeVoteBruteforce(d_pointsSorted, d_labels, axes);
    }

    if(!axes.heeftMinstensEenAs())
    {
        std::cout << "[Autofit] Geen vlak gevonden. Brute buizen (annulus/wedge)…\n";
        h_pipeVoteBruteforce(d_pointsSorted, d_labels, axes);
    }

    if(!axes.heeftMinstensEenAs())
    {
        std::cout << "[Autofit] Geen eerste as te vinden (vlakken/buizen). Stop.\n";
        return;
    }

    // 6) Orthonormaliseer 2e/3e as (JBF → Gram–Schmidt), maak quaternion
    Quat q = maakQuatNaarIdeaalFrame(axes); // in orchestratie.h/.cpp

    std::cout << "[Autofit] Roteer naar ideaal frame (quaternion)…\n";
    h_rotatieQuatApply(d_pointsSorted, q);

    // 7) Fine slabs + (simpele) radiale pipelabeling in ideaal frame
    h_fineSlabsCylLabel(d_pointsSorted, d_labels);

    // 8) Split terug: vec3f → x/y/z
    hSplitVec3NaarXyz(d_pointsSorted, dX, dY, dZ);

    // 9) Download en TERUG-PERMUTEER naar oorspronkelijke as-namen
    std::vector<float> rx(N), ry(N), rz(N);
    dX.download(rx.data(), N);
    dY.download(ry.data(), N);
    dZ.download(rz.data(), N);

    // Terug-mapping: we hebben invNaam: orig 'x'→'?' (x/y/z)
    auto zetTerug = [&](char orig, const std::vector<float>& nx, const std::vector<float>& ny,
                        const std::vector<float>& nz, float& out) {
        char nieuwe = perm.invNaam[(unsigned char)orig];
        if(nieuwe == 'x') out = nx[&out - &out]; // niet bruikbaar zo; we schrijven rechtstreeks per punt
    };

    // Simpeler: schrijf direct de juiste component per punt
    for(size_t i = 0; i < N; ++i)
    {
        float X = rx[i], Y = ry[i], Z = rz[i];
        // welke originele as kwam op nieuwX?
        // perm.nieuwX.naam == 'y' betekent: oude 'y' staat nu in X
        float outX = 0, outY = 0, outZ = 0;
        // inv map: welke nieuwe letter hoort bij orig?
        // We bepalen per nieuwe component naar welke originele naam die hoort:
        // nieuwX kwam van naamA → schrijf X terug naar die orig naamA
        auto schrijf = [&](char kwamVan, float waarde) {
            if(kwamVan == 'x')
                outX = waarde;
            else if(kwamVan == 'y')
                outY = waarde;
            else if(kwamVan == 'z')
                outZ = waarde;
        };
        schrijf(perm.nieuwX.naam, X);
        schrijf(perm.nieuwY.naam, Y);
        schrijf(perm.nieuwZ.naam, Z);

        // Update terug in CC cloud (alleen pos; labels kun je separaat ophalen)
        CCVector3* p = mSelectedCloud->getPointMutable(i);
        if(p)
        {
            p->x = outX;
            p->y = outY;
            p->z = outZ;
        }
    }

    std::cout << "[Autofit] Klaar: points teruggezet in originele as-namen.\n";
}