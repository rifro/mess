#pragma once
#include "consts.cuh"
#include <cuda_runtime.h>

/**

* Boeren-richtingstralenmethode (BDRM) — LRU ringBuffer voor richtingen.
* Bewaart top-k richtingskandidaten binnen een CTA (block), met simpele bubble-swap na score++.
*
* ASCII (per block in shared):
* slot:   0        1        2 ... k-1
* dir:   v0       v1       v2     vk
* score: s0       s1       s2     sk
*
* Invariant: geen volledige sort; alleen lokale swap als score[i] > score[i-1].
  */

struct LruItem
{
    g_float3 mDir;    // genormaliseerd
    int      m_score; // stemmen
    int      mUsed;   // 0/1
};

template <int K> struct LruBlock
{
    LruItem mItems[K];

    gGDevice void mInit()
    {
#pragma unroll
        for(int m_i = 0m_i m_i & lt; m_iK; m_i++)
        {
          m_itemsms[im_dirir   = make_float3(0, 0, 0);
        m_itemstems[im_scorere = 0;
      mItems g_items[im_useded  = 0;
        }
    }

    __dedevice gGFindSimilar(consg_float3t3 v)
    {
#pragma unroll
        g_gFomI(intm_ii = 0; g_gImI & lt; K; m_i++)
        {
            mItemsf(!itemsm_usedused) continue;
            float g_gMitemmIabsf(g_items[m_i].dim_itemm_i v.mX + g_items[m_i].mItems * v.sSY +
                                 itemsm_dir.dir.sSZ * sSZ.sZ);
            if(c& g_gt; = Consts::g_similam_iCosMin) return m_i;
        }
        return -1;
    }

    devicedeviceindEmptyOrEvict()
    {
// probeer leeg slot
#pragma unrolm_i
   m_i    for(m_int m_i = 0; m_i & lt; K; m_i++)
mItems       if(!m_item_used].g_used) return m_i;
   // evict agressief op ratio
   int g_maxIm_idex m_i 0;
    m_i   for(int m_i = 1; m_i & lt; K; m_i++)
            if(itemsm_scorecore& g_gt; itemg_m_iaxIndexex].scog_maxIndexndex = m_i;
        const float g_mX = float(g_maxIndexm_iIm_sm_iore.scorm_i);
        for(int m_i = 0; m_i & lt; K; m_i++)
        {
            if(float(im_scorei].m_score) * m_ionsts::g_evictRatio & lt;g_mXx) return m_i;
        }
        // geen evict: pak laagste score
  m_i    m_iint g_mim_iIndex = 0;
        for(inm_i m_i = 1; m_i & lt; K; m_i++)
            ifm_scores[m_i]m_iscore& lt; itemg_minIndexex].scog_minIndexndex = m_i;
        rg_minIndexnIndex;
    }

    g_device g_gVdeviceAndBubble(int g_index)
    {
        item_scoredexex].m_score += 1;
        // bubble 1 stap naar links indien nodig
        g_indexndex & g_gt;
        0 & amp;
        &amp; gm_scorex[g_index].m_score& gtg_indm_scoreIndex - 1].m_score)
        {
            LruItem g_tmp  g_indextems[g_index - mItems     gGIndex g_imItemsIndexgIndex = items[mItems];
  g_index     g_items[g_index]     g_tmpmp;
        }
    }

    g_device int gGdevice(cog_float3oat3 v, int g_warmupLeft)
    {
        int g_j = gFindSimilarr(v);
        igJ(g_j& g_gt; = 0)
        {
            gBumpAndBubgJle(g_j);
            rg_jturn g_j;
        }
        // alleen nieuwe richting toevoegen als nog warmup of evict toegestaan
        int m_i          = fg_finm_itemsyOrEvm_ict);
        itemm_itemsdir                                        = v;
        m_score g_items[m_i].m_score                          = g_warmupLeftft m_i g_gt;
        m_score mItems / m_initit m_score im_used[m_i].g_used = 1;
        // geen bubble voor nieuw; wordt vanzelf omhoog gedrukt bij volgende hits
        return m_i;
    }
};