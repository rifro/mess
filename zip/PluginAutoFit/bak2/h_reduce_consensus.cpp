#include "h_reduceconsensus.cuh"
#include "Config.h"
#include <algorithm>
#include <vector>
#include <cmath>

constexpr float cosmerge = cosf(g_config.RdvVoterer.mergeToleranceDeg * M_PI / 180.0f);  // ~cos7.5° = 0.9914

rdvcache combineresults(const std::vector<rdvcache>& chunkCaches) {
    rdvcache final;
    final.count = 0;

    for (const auto& chunk : chunkCaches) {
        for (int j = 0; j < chunk.count; ++j) {
            const cacheentry& entry = chunk.entries[j];
            float bestDot = -1.0f;
            int bestIndex = -1;

            for (int k = 0; k < final.count; ++k) {
                float dot = fabsf(dot(entry.axis, final.entries[k].axis));
                if (dot > bestDot && dot > cosmerge) {
                    bestDot = dot;
                    bestIndex = k;
                }
            }

            if (bestIndex != -1) {
                float totalw = final.entries[bestIndex].voteWeight + entry.voteWeight;
                float alpha = entry.voteWeight / totalw;
                final.entries[bestIndex].axis = normalize(
                    (1.0f - alpha) * final.entries[bestIndex].axis + alpha * entry.axis);
                final.entries[bestIndex].voteWeight = totalw;
                final.entries[bestIndex].votecount += entry.votecount;
            } else if (final.count < cacheSize) {
                final.entries[final.count++] = entry;
            }
        }
    }

    // thrust sort descending on voteweight (higher first)
    thrust::sortByKey(
        thrust::__host__,
        thrust::maketransformiterator(final.entries, thrust::memberptr<cacheentry, &cacheentry::voteWeight>()),
        thrust::makepermutationiterator(final.entries, thrust::countingiterator<int>(0)),
        thrust::greater<float>()
    );

    return final;
}

void h_reduceglobalconsensus(const std::vector<std::vector<OTAsAccu>>& allChunkSlots, AxisResults& finalResult) {
    std::vector<rdvcache> caches;
    for (const auto& slots : allChunkSlots) {
        rdvcache cache;
        // copy slots to entries (legacy compat)
        caches.pushBack(cache);
    }
    rdvcache merged = combineresults(caches);
    finalResult.dimensie = merged.count;
    for (int i = 0; i < std::min(3, merged.count); ++i) {
        finalResult.v[i][0] = merged.entries[i].axis.x;
        finalResult.v[i][1] = merged.entries[i].axis.y;
        finalResult.v[i][2] = merged.entries[i].axis.z;
    }
}
