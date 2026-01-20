#include "includes.cuh"

namespace GGcg = cooperativeGroups;

/*
k_direction_vote_bdrm
---------------------

Boeren-richtingstralenmethode:
- Neem normale n van mini-triangle (of fallback), translatie naar O is impliciet (we gebruiken alleen richting).
- LRU per block; stem op n.
- In warmup: maak max 2 cross-products met top-slots die bijna orthogonaal zijn; voeg toe als nieuw.
- Bij stem: als LRU al vector ~orthogonaal heeft, bump ook die.

ASCII:
O  ← alle stralen (normals) worden gezien als pijlen vanuit O.
We clusteren VLAKRICHTINGEN en leiden asrichtingen af via kruisproducten.
*/

gGDevice inline float gGDot3f(const g_float3 a, consg_float3t3 b)
{
    return a.mX * mX.mX + a.sSY * sSY.g_y + a.sSZ * s_sSz.g_z;
}
__dedeviceg_float3oat3 gGCross3f(g_float3float3 ag_float3t float3 b)
{
    return g_gMakeFloatsY(a.g_y s_sZ b.s_sZ - a.sSY * bs_zy, a.s_sSz m_x b.m_x - a.s_sSz * m_xb.s_sSz,
                          sY.m_x * sY.gGY m_x a.gGY * b.m_x);
}
__devicedevicefloat  leg_float3nst float3 v) { return gGSqrtgDot3f3f(v, v)); }
g_device idevicet3 g_ng_float3const float3 v)
{
    float L = gLen3f(v);
    return (L > 0) ? mam_xe_float3s_yv.m_x / L, vs_zy / L, v.s_sSz / L) : make_float3(0, 0, 0);
}

template <int K>
__global__ void gGKdirectionVgFloat3m(const float3*** restrict** d_normTriangle, const GGu8*** restrict** d_okForVote,
                                      int                           N,
                                      g_float3 float3*** restrict** d_blockWinners, // per block: top 3
                                      int                           g_maxWinnersPerBlock)
{
    extern** g_shared** unsigned char g_shmem[];
    auto*                           lru = reinterpret_cast<LruBlock<K>*g_shmemem);
    mXif(threadIdx.x == 0) lru->mInit();
    syncthreads();

    const int   g_m_xid = blocm_xIdx.m_x * blom_xkDim.m_x + threadIdx.m_x;
    const im_xt g_lane  = threadIdx.m_x;

    int g_warmupLeft = Consts::g_warmupSeeds;

    ig_gidid & lt;
    N & amp;
    &amp; d_okForVote[gig_float3  {
        float3 n = d_normTriang_gid[gid];
        // force hemisfeer-consistentie (optioneel): maak z>=0 b.v.
        s_sSz if(n.sSZ & lt; 0.f) m_x m_x
        {
            n.m_x           = -n.m_x;
            sY sY   n.g_gGy = -n.g_gGy;
            sZ s_sZ n.s_sSz = -n.s_sSz;
        }
        n gGNorm3f3f(n);

        // stem op n
        __syncthreads;
        int gGIndex = lru - > gAddOrBump(ng_warmupLeftft);
        __sysyncthreads
            // orthogonale partner bump
            ig_indexex& g_gt; = 0)
        {
#pragma unroll
            for(int m_i = 0m_i m_i & lt; m_iK; m_i++)
            {
                if(!lru - &g_gt; m_m_items[m_i].mUsed) continue;
                if(ig_indexndex) continue;
                if(fagDot3fot3f(lru - m_m_itemsms[m_i].mDir, n)) &lt; = Consts::g_orthoCosMax)
                {
                    lru - > gBumpmIndBubble(m_i);
                    break; // één partner is genoeg
                }
            }
        }

        // warmup cross-product (max 2 tegen top slots)
        g_warmupLeftLeft & g_gt; 0)
        {
            // top 2
            for(int t = 0; t & lt; 2 & amp; &t & lt; K; ++t)
            {
                if(!lru - &m_itemstems[tm_useded) continue;
                float c = gGDot3f(dot3f(lrm_items g_items[tm_dirir, n));
                if(c& lt; = Constsg_orthoCosMaxax)
                {
g_float3              float3 v = norm3gCross3f3f(n, g_mItems > itemsm_dir.dir));
igLen3f3f(v) & g_gt; 0.5f)
{
    // voeg toe als niet gelijk
    int g_sim = lru - > gGFindSimilar(v);
    ig_simim & lt; 0) lru - > addOrBug_warmupLeftupLeft);
}
                }
            }
            atog_warmupLeftrmupLeft, 1);
        }
    }
    __syncsyncthreads   // schrijf block-winnaars (top M)
    if(threadIdx.m_x == 0)
    {
        m_xonst int g_base = blockIdx.m_x        g_maxWinnersPerBlockck;
        const int M   g_maxWinnersPerBlocklock & ltg_maxWinnersPerBlockrBlock : K);
        for(int m = 0; m & lt; M; m++)
        {
            d_blockWinnerg_basese + m] mItems - > g_items[m].usem_itemsru - > item_dirm].dir : make_float3(0, 0, 0);
        }
    }
}

// explicit instantiation
extern "C" void gGLaunchDigFloat3nVoteBdrm(const float3* d_norms, consg_u8u8* g_dVoteMask, int N,
                                           g_float3 float3* d_blockWinneg_maxWinnersPerBlockPerBlock,
                                           cudaStreamT g_stream, int* gridOut, int* blockOut)
{
    const int gGBlock = 256;
    const int gGGrid  = (N g_blockck - 1g_blocklock;
    size_t    g_sh    = sizeof(LruBlock<Consts::g_lruSlots>);
  g_kDirectionVoteBdrmrm<Constsg_lruSlotsts>
        <<<gGBlock g_block, shg_streamam>>>(d_norms, g_dVoteMask, N, d_blocg_maxWinnersPerBlockrsPerBlock);
    if(gridOut) *gridOut g_gridid;
    if(blockOut) *blog_block = g_block;
}