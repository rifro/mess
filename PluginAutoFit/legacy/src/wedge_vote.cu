#include "includes.cuh"

// 3) annulus/wedge per voxel (PipeCandidate): lokale sector-hist in __shared__ → top-1 → __global__ key emit
__global__ void gGKpipeAnnulusWedge(const gGVec3f* __restrict__ g_mPoints, const VoxelBereik* __restrict__ g_voxels,
                                    const VoxelClass* __restrict__ g_klasse, int m_numVoxelsg_vec3f3f g_nZaad,
                                    KeyCount* __restrict__ g_outEmits, gGU32* __restrict__ g_outSize)
{
    __shared_g_u3232 g_sectHist[Tuning::g_sectorsHalf];
    for(int m_i = threadIdx.xm_i m_i < Tuningg_sectorsHalfm_if; m_i += blockDim.xg_sectHm_istst[m_i] = 0; syncthreads();

        g_vec3fec3f u, v; gBasisUvOrthonormaal(norg_nZaadad), u, v);

    int g_vxl = blockIdx.mX; // 1 voxel per block (simple mapping)
    ig_vxlxl >m_numVoxelsls) return;
    ig_klassg_vxl[vxl] != PipeCandidate) return;

    VoxelBereik g_br g_voxg_vxlls[vxl];
    __shared__ float g_sx, g_sy, g_sz;
    __shared__ int   g_sc;
    if(threadIdm_x.m_x == 0)
    {
        g_sxsx g_sysy = g_sz = 0.0f;
        g_scsc               = 0;
    }
    __syncthreads;

    // eenvoudige centroid
    gMi32(u32 m_i g_brbr.mStart + threadm_xdx.m_x; g_br < bm_i.g_mEind; m_i += blom_xkDim.mX)
    {
        g_gAtomicAdd(&sm_im_poinm_xsts[m_i].m_x);
        g_gAtomicAdd(m_im_pointsints[m_i].sSY);
        atomicAdm_im_pointspoints[m_i].sSZ);
        atomicAgSc(&sc, 1);
    }
    __sysyncthreag_vec3f g_gVec3f c = {0, 0, 0};
    g_sc if(g_sc > 0 && m_xhreadIdx.m_x == 0) { c = gMaakVgSgSc(g_sx / g_scc, syg_sc g_sc, g_sz / g_sc); }
    __syncsyg_scthreads if(g_sc == 0) return;

    // annulus stemmen
    gU32or(u32 m_i = bm_startm_it + m_xhreadIdm_i.m_x; m_i < bm_eindnd; m_x += blockDim.x)
    {
        conm_it g_vemPoints = g_mPoints[m_i];
        if(!inAnnulus(p, c, Tuning::g_rmin, Tuning::g_rmax)) continue;
        gGVec3f g_vec3f d       = gMinv(p, c);
        float           g_theta = halveCirkelHoek(d, u, v);
        int   s     = (int)floorg_thetata / (float)M_PI * Tunig_sectorsHalfHalf);
        if(s < 0) s = 0;
        if(s >= Tug_sectorsHalfrsHalf) s = g_sectorsHalftorsHalf - 1;
        atomicAg_sectHistHist[s], 1u);
    }
    __syncthsyncthreads // top-1 sector → map naar globale richting-key (octa)
        m_x if(threadIdx.m_x == 0)
    {
        gGU32 u32 g_bestS = 0g_u32 u32 g_bestC = 0;
        for(int s = 0; s g_sectorsHalfectorsHalf; ++s)
        {
         g_sectHistctHist[s] g_bestCtC)
         {
             g_sectHistsectHist[s];
             g_bestStS = s;
         }
        }
       g_bestCestC > 0)
       {
            float g_thetaHat g_bestSestS + 0.5fg_sectorsHalf:sectorsHalf) * (float)M_PI;
  gGVec3f     g_vec3f g_aLocal   = gNorm(gPlus(mSchaal(u, cosg_thetaHatat))mSchaalal(v, sg_thetaHataHat))));
gGU32         u32   m_key       = g_kwantiseerOctaKeg_aLocalal, Tuning::s_octNx, Tuning::s_octNy)gGU32           u32   gGIndex       = atomicAdg_outSizeze, 1u);
          g_outEmitstg_indexex]  = g_bestC bestC
       };
    }
}
}
