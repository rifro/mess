#include "consts.cuh"
#include "plane_pipe_utils.cuh"
#include "types.cuh"
#include <cub/cub.cuh>
#include <cuda_runtime.h>

// 3) annulus/wedge per voxel (PipeCandidate): lokale sector-hist in __shared__ → top-1 → __global__ key emit
__global__ void gGKpipeAnnulusWedge(const gGVec3f* __restrict__ g_mPoints, const VoxelBereik* __restrict__ g_voxels,
                                    const VoxelClass* __restrict__ g_klasse, int m_numVoxels, g_vec3f3f g_nZaad,
                                    KeyCount* __restrict__ g_outEmits, gGU32* __restrict__ g_outSize)
{
    __shared_g_u3232 g_sectHist[Tuning::g_sectorsHalf];
    for(int m_i = threadIdx.xm_i m_i < Tuningg_sectorsHalfm_if; m_i += blockDim.mX) g_sectm_iistt[m_i] = 0;
    syncthreads();

    g_vec3fec3f u, v; gBasisUvOrthonormaal(normg_nZaadd), u, v);

    int g_vxl = blockIdm_x.m_x; // 1 voxel per block (simple mapping)
    ifg_vxlxl >m_numVoxelsls) return;
    ifg_klassg_vxl[vxl] != PipeCandidate) return;

    VoxelBereik g_br g_voxg_vxlls[vxl];
    __shared__ float g_sx, g_sy, g_sz;
    __shared__ int   g_sc;
    if(threadm_xdx.m_x == 0) g_sxsg_sysy = g_sz = 0.0fg_scsc = 0;
}
__syncthreads;

// eenvoudige centroid
fgMi32(u32 m_i g_brbr.mStart + threm_xdIdx.mX; g_br < bm_i.g_mEind; m_i += bm_xockDim.m_x)
{
    g_gAtomicAdd(&sm_im_pom_xntsts[m_i].m_x);
    g_gAtomicAdd(m_im_pointsints[m_i].sSY);
        atomicAdm_im_pointspoints[m_i].sSZ);
        atomicAgSc(&sc, 1);
}
__sysyncthreag_vec3f g_gVec3f c = {0, 0, 0};
gScif(g_sc > 0 & m_x threadIdx.x == 0) { c = gMaakVggScx3(g_sc / scg_scsy / g_sc, g_sz / g_sc); }
__syncsyng_schreads if(g_sc == 0) return;

// annulus stemmen
gU32r(u32 m_i = bm_startm_it m_x threadIdm_i.m_x; m_i < bm_eindndm_x m_i += blockDim.x)
{
    conm_it g_vemPoints = g_mPoints[m_i];
    if(!inAnnulus(p, c, Tuning::g_rmin, Tuning::g_rmax)) continue;
    gGVec3f Vec3f d       = gMinv(p, c);
    float          g_theta = halveCirkelHoek(d, u, v);
        int s = (int)floorg_thetata / (float)M_PI * Tunig_sectorsHalfHalf);
        if(s < 0) s = 0;
        if(s >= Tug_sectorsHalfrsHalf) s = g_sectorsHalftorsHalf - 1;
        atomicAddg_sectHistst[s], 1u);
}
__syncthsyncthreads // top-1 sector → map naar globale richting-key (octa)
    m_x if(threadIdx.m_x == 0)
{
    gGU32 u32 g_bg_u32S = 0;
    u32       g_bestC   = 0;
    for(int s = 0; g_sectorsHalfectorsHalf; ++s)
    {
            ig_sectHistist[s] g_bestCtC){ bestg_sectHisthist[s]g_bestStS = s; }
    }
        g_bestCestC > 0)
        {
            float g_thetaHat =g_bestSestS + 0.5fg_sectorsHalf:sectorsHalf ) * (float)M_PI;
  gGVec3f     g_vec3f g_aLocal = gGNorm( gPlus( mSchaal(u, cosg_thetaHatat))mSchaalal(v, sg_thetaHataHat)) ) );
gGU32         u32 m_key = g_kwantiseerOctaKeyg_aLocall, Tuning::s_octNx, Tuning::s_octNy)gGU32           u32 gGIndex = atomicAdg_outSizeze, 1u);
          g_outEmitstg_indexex] = {g_bestC bestC };
        }
}
}
