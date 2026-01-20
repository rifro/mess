#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "device_buffer.cuh"    // Hersteld van __device___buffer
#include "axis_permutatie.h"
#include "cc_adapter.h"
#include "consts.cuh"
#include "merge_split.cuh"
#include "orchestratie.h"
#include "types.h"

using namespace Bocari;

// Kernels/Host-stappen
extern void h_voxelizeMorton(const DeviceBuffer<Vec3f>& dPoints,
                             DeviceBuffer<u64>& dKeys,
                             DeviceBuffer<u32>& dIndex,
                             DeviceBuffer<Vec3f>& dPointsSorted);

extern void h_noiseFilter(DeviceBuffer<Vec3f>& dPointsSorted, 
                          DeviceBuffer<u8>& dLabels);

extern void h_vlakVoteCurrentFrame(const DeviceBuffer<Vec3f>& dPointsSorted,
                                   DeviceBuffer<u8>& dLabels,
                                   CandidateAxes& uit);

extern void h_vlakVoteBruteforce(const DeviceBuffer<Vec3f>& dPointsSorted,
                                 DeviceBuffer<u8>& dLabels,
                                 CandidateAxes& uit);

extern void h_pipeVoteBruteforce(const DeviceBuffer<Vec3f>& dPointsSorted,
                                 DeviceBuffer<u8>& dLabels,
                                 CandidateAxes& uit);

extern void h_rotatieQuatApply(DeviceBuffer<Vec3f>& dPointsSorted, const Quat& q);

extern void h_fineSlabsCylLabel(const DeviceBuffer<Vec3f>& dPointsSorted, 
                                DeviceBuffer<u8>& dLabels);

// ============= HOOFDFLOW =============

void AutofitImpl::doOptimizeFrame3()
{
    if (!mSelectedCloud || mSelectedCloud->size() == 0)
    {
        std::cout << "[Autofit] Geen point cloud of leeg.\n";
        return;
    }

    // 1) Lees CC-points en bepaal as-verdeling
    CcVector3 bbMin, bbMax;
    mSelectedCloud->getBoundingBox(bbMin, bbMax);

    const size_t N = mSelectedCloud->size();
    std::vector<float> h_x(N), h_y(N), h_z(N);

    for (size_t i = 0; i < N; ++i)
    {
        const CcVector3* p = mSelectedCloud->getPoint(i);
        if (!p) continue;
        h_x[i] = p->x;
        h_y[i] = p->y;
        h_z[i] = p->z;
    }

    const float lx = bbMax.x - bbMin.x;
    const float ly = bbMax.y - bbMin.y;
    const float lz = bbMax.z - bbMin.z;
    const float lMax = std::max({lx, ly, lz});

    u32 vx = std::max(1u, (u32)(lx > 0 ? 1024.f * lx / lMax : 1));
    u32 vy = std::max(1u, (u32)(ly > 0 ? 1024.f * ly / lMax : 1));
    u32 vz = std::max(1u, (u32)(lz > 0 ? 1024.f * lz / lMax : 1));

    AsInfo as_x{'x', h_x.data(), vx};
    AsInfo as_y{'y', h_y.data(), vy};
    AsInfo as_z{'z', h_z.data(), vz};

    AxisPermutatie perm = maakAxisPermutatie(as_x, as_y, as_z);
    toonAxisMapping(perm);

    // 2) Upload nieuwe x/y/z volgens permutatie
    DeviceBuffer<float> dX(N), dY(N), dZ(N);
    dX.upload(perm.nieuwX.data, N);
    dY.upload(perm.nieuwY.data, N);
    dZ.upload(perm.nieuwZ.data, N);

    // 3) Merge naar Vec3f
    DeviceBuffer<Vec3f> dPoints(N);
    h_mergeXyzNaarVec3(dX, dY, dZ, dPoints);

    // 4) Sort + filter
    DeviceBuffer<u64> dKeys(N);
    DeviceBuffer<u32> dIndex(N);
    DeviceBuffer<Vec3f> dPointsSorted(N);
    DeviceBuffer<u8> dLabels(N);
    dLabels.memset(0);

    h_voxelizeMorton(dPoints, dKeys, dIndex, dPointsSorted);
    h_noiseFilter(dPointsSorted, dLabels);

    // 5) Vlakken zoeken
    CandidateAxes axes{};
    h_vlakVoteCurrentFrame(dPointsSorted, dLabels, axes);

    if (!axes.heeftMinstensEenAs()) {
        h_vlakVoteBruteforce(dPointsSorted, dLabels, axes);
    }
    if (!axes.heeftMinstensEenAs()) {
        h_pipeVoteBruteforce(dPointsSorted, dLabels, axes);
    }

    if (!axes.heeftMinstensEenAs()) {
        std::cout << "[Autofit] Geen as gevonden. Stop.\n";
        return;
    }

    // 6) Quaternion + Rotatie
    Quat q = maakQuatNaarIdeaalFrame(axes);
    h_rotatieQuatApply(dPointsSorted, q);

    // 7) Labeling
    h_fineSlabsCylLabel(dPointsSorted, dLabels);

    // 8) Split terug
    h_splitVec3NaarXyz(dPointsSorted, dX, dY, dZ);

    // 9) Download en terug-permuteren
    std::vector<float> rx(N), ry(N), rz(N);
    dX.download(rx.data(), N);
    dY.download(ry.data(), N);
    dZ.download(rz.data(), N);

    for (size_t i = 0; i < N; ++i) {
        float valX = rx[i], valY = ry[i], valZ = rz[i];
        float outX = 0, outY = 0, outZ = 0;

        auto schrijf = [&](char kwamVan, float waarde) {
            if (kwamVan == 'x') outX = waarde;
            else if (kwamVan == 'y') outY = waarde;
            else if (kwamVan == 'z') outZ = waarde;
        };

        schrijf(perm.nieuwX.naam, valX);
        schrijf(perm.nieuwY.naam, valY);
        schrijf(perm.nieuwZ.naam, valZ);

        CcVector3* p = mSelectedCloud->getPointMutable(i);
        if (p) {
            p->x = outX;
            p->y = outY;
            p->z = outZ;
        }
    }
    std::cout << "[Autofit] Klaar.\n";
}
