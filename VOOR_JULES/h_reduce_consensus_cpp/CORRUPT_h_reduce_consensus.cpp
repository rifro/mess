#include "h_reduce_consensus.cuh"
#include "math_utils.cuh"
#include <algorithm>

void h_reduce_global_consensus(
    const std::vector<std::vector<OTAsAccu>>& allChunkSlots,
    AxisResults& final_result
) {
    std::vector<OTAsAccu> all_slots;
    for (const auto& chunkSlots : allChunkSlots) {
        for (const auto& slot : chunkSlots) {
            if (slot.count > 0) {
                all_slots.push_back(slot);
            }
        }
    }

    if (all_slots.empty()) {
        final_result.dimensie = 0;
        return;
    }

    // Cluster slots
    std::vector<OTAsAccu> clustered_slots;
    std::vector<bool> used(all_slots.size(), false);

    for (size_t i = 0; i < all_slots.size(); ++i) {
        if (used[i]) continue;

        OTAsAccu current_cluster = all_slots[i];
        used[i] = true;

        for (size_t j = i + 1; j < all_slots.size(); ++j) {
            if (used[j]) continue;

            if (dot(normalize(current_cluster.som), normalize(all_slots[j].som)) > 0.95f) { // 5 degrees
                current_cluster.som.x += all_slots[j].som.x * all_slots[j].count;
                current_cluster.som.y += all_slots[j].som.y * all_slots[j].count;
                current_cluster.som.z += all_slots[j].som.z * all_slots[j].count;
                current_cluster.count += all_slots[j].count;
                used[j] = true;
            }
        }
        clustered_slots.push_back(current_cluster);
    }

    // Sort clustered slots
    std::sort(clustered_slots.begin(), clustered_slots.end(), [](const OTAsAccu& a, const OTAsAccu& b) {
        return a.count > b.count;
    });

    // Extract final axes
    final_result.dimensie = 0;
    if (clustered_slots.size() > 0 && clustered_slots[0].count > 0) {
        float3 axis1 = normalize(clustered_slots[0].som);
        final_result.v[0][0] = axis1.x;
        final_result.v[0][1] = axis1.y;
        final_result.v[0][2] = axis1.z;
        final_result.score[0] = clustered_slots[0].count;
        final_result.dimensie = 1;

        if (clustered_slots.size() > 1 && clustered_slots[1].count > 0) {
            float3 axis2 = normalize(clustered_slots[1].som);
            final_result.v[1][0] = axis2.x;
            final_result.v[1][1] = axis2.y;
            final_result.v[1][2] = axis2.z;
            final_result.score[1] = clustered_slots[1].count;
            final_result.dimensie = 2;

            if (clustered_slots.size() > 2 && clustered_slots[2].count > 0) {
                float3 axis3 = normalize(clustered_slots[2].som);
                final_result.v[2][0] = axis3.x;
                final_result.v[2][1] = axis3.y;
                final_result.v[2][2] = axis3.z;
                final_result.score[2] = clustered_slots[2].count;
                final_result.dimensie = 3;
            }
        }
    }
}
