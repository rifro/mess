#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "device_buffer.cuh"
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

/**
 * h_doOptimizeFrame3
 * Herstelde versie: Gouden structuur + Nieuwe Permutatie Logica
 */
void h_doOptimizeFrame3(const std::vector<float>& nx,
                        const std::vector<float>& ny,
                        const std::vector<float>& nz,
                        const AxisPermutatie& perm,
                        CCPointCloud* cloud) 
{
    size_t N = nx.size();
    if (N == 0) return;

// 1) Voorbereiden Device Buffers (Host-side objecten die device memory beheren)
    DeviceBuffer<float> d_x(N), d_y(N), d_z(N);
    d_x.upload(nx.data(), N);
    d_y.upload(ny.data(), N);
    d_z.upload(nz.data(), N);

    DeviceBuffer<Vec3f> d_points(N);
    DeviceBuffer<Vec3f> d_pointsSorted(N);
    DeviceBuffer<u64>   d_keys(N);
    DeviceBuffer<u32>   d_index(N);
    DeviceBuffer<u8>    d_labels(N);

    // 2) Merge naar Vec3
    h_mergeXyzNaarVec3(d_x, d_y, d_z, d_points);

    // 3) Voxelize & Sort
    h_voxelizeMorton(d_points, d_keys, d_index, d_pointsSorted);

    // 4) Noise Filter
    h_noiseFilter(d_pointsSorted, d_labels);

    // 5) Voting (Detectie van assen)
    CandidateAxes axes;
    h_vlakVoteCurrentFrame(dPointsSorted, dLabels, axes);
   
    if (!axes.heeftMinstensEenAs()) {
        h_vlakVoteBruteforce(d_pointsSorted, d_labels, axes);
    }
    if (!axes.heeftMinstensEenAs()) {
        h_pipeVoteBruteforce(d_pointsSorted, d_labels, axes);
    }

    if (!axes.heeftMinstensEenAs()) {
        std::cout << "[Autofit] Geen assen gevonden voor optimalisatie.\n";
        return;
    }

    // 6) Quaternion berekening & Rotatie applicatie
    Quat q = maakQuatNaarIdeaalFrame(axes);
    h_rotatieQuatApply(d_pointsSorted, q);

    // 7) Fijnmazige labeling
    h_fineSlabsCylLabel(d_pointsSorted, d_labels);

    // 8) Split terug naar componenten (gebruik d_x, d_y, d_z)
    h_splitVec3NaarXyz(d_pointsSorted, d_x, d_y, d_z);

    // 9) Download & Terug-permuteren naar Cloud
    std::vector<float> r_x(N), r_y(N), r_z(N); // Host resultaat buffers
    d_x.download(r_x.data(), N);
    d_y.download(r_y.data(), N);
    d_z.download(r_z.data(), N);

    for (size_t i = 0; i < N; ++i) {
        float valX = r_x[i], valY = r_y[i], valZ = r_z[i];
        float outX = 0, outY = 0, outZ = 0;

        // De nieuwe 'Gedachte': Gebruik de inverse mapping van de permutatie
        auto schrijfNaarOrigineleAs = [&](char kwamVanNaam, float waarde) {
            if (kwamVanNaam == 'x')      outX = waarde;
            else if (kwamVanNaam == 'y') outY = waarde;
            else if (kwamVanNaam == 'z') outZ = waarde;
        };

        schrijfNaarOrigineleAs(perm.nieuwX.naam, valX);
        schrijfNaarOrigineleAs(perm.nieuwY.naam, valY);
        schrijfNaarOrigineleAs(perm.nieuwZ.naam, valZ);

        // Update de cloud via de adapter
        cloud->setPoint(i, CCVector3(outX, outY, outZ));
    }
}
// ============= HOOFDFLOW (In AutofitImpl.cpp) =============

void AutofitImpl::doOptimizeFrame3()
{
    using namespace Bocari;

    // Check member variabele (m_selectedCloud)
    if(!m_selectedCloud || m_selectedCloud->size() == 0)
    {
        std::cout << "[Autofit] Geen point cloud of leeg.\n";
        return;
    }

    // 1) Host buffers voorbereiden
    const size_t N = m_selectedCloud->size();
    std::vector<float> h_x(N), h_y(N), h_z(N);

    for(size_t i = 0; i < N; ++i)
    {
        const CCVector3* p = m_selectedCloud->getPoint(i);
        if(!p) continue;
        h_x[i] = p->x;
        h_y[i] = p->y;
        h_z[i] = p->z;
    }

    // Bepaal bounding box voor as-verdeling
    CCVector3 bb_min, bb_max;
    m_selectedCloud->getBoundingBox(bb_min, bb_max);

    const float l_x = bb_max.x - bb_min.x;
    const float l_y = bb_max.y - bb_min.y;
    const float l_z = bb_max.z - bb_min.z;

    // Grid schatting (Vx, Vy, Vz)
    u32 v_x = std::max(1u, (U32)(l_x > 0 ? 1024.f * (l_x / std::max({l_x, l_y, l_z})) : 1));
    u32 v_y = std::max(1u, (U32)(l_y > 0 ? 1024.f * (l_y / std::max({l_x, l_y, l_z})) : 1));
    u32 v_z = std::max(1u, (U32)(l_z > 0 ? 1024.f * (l_z / std::max({l_x, l_y, l_z})) : 1));

    Bocari::AsInfo ax{'x', h_x.data(), v_x};
    Bocari::AsInfo ay{'y', h_y.data(), v_y};
    Bocari::AsInfo az{'z', h_z.data(), v_z};

    // Permutatie bepalen
    AxisPermutatie perm = maakAxisPermutatie(ax, ay, az);
    toonAxisMapping(perm);

    // 2) De Engine aanroepen (De schone functie met d_ prefixes binnenin)
    // We geven h_x, h_y, h_z mee. De functie h_doOptimizeFrame3 regelt 
    // zelf de upload naar d_x, d_y, d_z en het terugzetten in de cloud.
    h_doOptimizeFrame3(h_x, h_y, h_z, perm, m_selectedCloud);

    std::cout << "[Autofit] doOptimizeFrame3 voltooid via engine delegatie.\n";
}
