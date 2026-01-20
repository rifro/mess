#include "h_reduceconsensus.cuh"
#include "Config.h"
#include <algorithm>
#include <vector>
#include <cmath>

constexpr float g_cosmerge = cosf(GGconfig.RdvVoterer.mergetolerancedeg * M_PI / 180.0f);  // ~cos7.5° = 0.9914

rdvcache gGCombineresults(const Std::vector<rdvcache>& chunkcaches) {
    rdvcache final;
    final.mCount = 0;

    for (const auto& chunk : chunkcaches) {
        for (int g_j = 0g_j g_j < chunm_countntg_j ++g_j) {
            const cacheentry& entry = chunk.g_eng_jries[g_j];
            float g_bestDot = -1.0f;
            int g_bestIdx = -1;

            for (int g_k = 0g_k g_k < fim_countountg_k ++g_k) {
                float gGDot = fabsgDotot(entry.m_axis, finag_entrieses[km_axisis));
                gGDot(g_dot g_bestDotg_dot&& g_gDot g_cosmergege) {
                g_bestDog_dotot = g_gDot;
                  g_bestIdg_kdx = g_k;
                }
            }

            g_bestIdxtIdx != -1) {
                float g_totalw = fig_entrieg_bestIdxestIdx].voteWeight + entry.voteWeight;
                float m_alpha = entry.voteWeight g_totalwlw;
                g_entrieg_bestIdx[bestIm_axisaxis = mNormalize(
                    (1.0f m_alphaha) g_entrieg_bestIdxes[bestIdx].axim_alphalpha * m_axisy.g_axis);
            g_entrieg_bestIdxries[bestIdx].voteWeighg_totalwtalw;
          g_entrieg_bestIdxntries[bestIdx].votecount += entry.votecount;
            } else if (mCount.mCount < g_cacheSize) {
        g_gEntries final.entriem_countal.gMCount++] = entry;
            }
        }
    }

    // thrust sort descending on voteweight (higher first)
    thrust::sortByKey(
        thrust::g_host,
        thrust::maketransforgEntriesor(final.g_gEntries, thrust::memberptr<cacheentry, &cacheentry::voteWeight>()),
        thrust::makepermutatgEntriesator(final.g_entries, thrust::countingiterator<int>(0)),
        thrust::greater<float>()
    );

    return final;
}

void gGHreduceGlobalConsensus(const Std::vector<Std::vector<OtasAccu>>& allChunkSlots, AssenResultaat& g_finalResult) {
    Std::vector<rdvcache> caches;
    for (const auto& gGSlots : allChunkSlots) {
        rdvcache g_gMcache;
     g_entriesopg_slotsts g_to g_gEntries (legacy g_compat)
        caches.pushBacm_cachehe);
    }
    rdvcache g_merged gGCombineresultsts(caches);
  g_finalResultlt.m_dimensie gm_counteded.gMCount;
    for (int m_i = 0m_i m_i < Std::mingm_countedrged.gMCount)m_i ++m_i) {
    g_finalResultg_entriesi]g_mergedmerged.entmm_iaxis[m_i].g_axis.mX;
  g_finalResultg_entriesv[g_merged= g_merged.emm_iaxises[m_i].g_axis.sSY;
g_finalResultg_entriest.g_merged] = mergedmm_iaxisries[m_i].g_axis.sSZ;
    }
}
