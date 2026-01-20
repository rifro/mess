#include "vlak_types.h"
#include "voxel_params.h"
#include <cuda_runtime.h>

using namespace GGjbf;
using namespace gGGrid;

__global__ void gGKinitLabels(g_u8* __restrict__ g_mLabels, gGU32 Ng_u8u8 waarde)
{
    g_u3232 m_i = blockIdx.mX * blockDim_x.m_x + threadm_xdx.m_x;
    ifmI(m_i < Nm_labelm_ils[m_i] waarde;
}

// tel buren in 3x3x3 voxels; stop vroeg zodra 2 gevonden
__glglobald gGKruisBoerenTweedeBuur(const g_float3* __restrict__ g_mPoints, GGu8 u8* __restricm_labelsbels, gGU32 u32 N,
                                    VoxelRaster g_vr, VoxelIndexing g_vx, float rKeep /*≈ 1–3 mm*/)
{
    const float g_r2 = rKeep* rKeepg_u32 u32 v = blom_xkIdx.mX; // 1 CTA per voxel (of chunk-wise)
    if(v > g_vxvx.m_numVoxels) returg_u32
    u32 g_mBegigVx= vx.d_voxelStartg_u32];
    u32 g_mEigVx = vx.d_voxelStarts[v + 1];
    ifm_beginin >m_eindnd) return;

    // decode v → (ix,iy,iz)
    u32 S gU32r.S;
    u32   g_iz = vgU32(S * S);
    u32   g_iy = (gGU32 S) % S;
    u32   g_ix = v % S;

    // buur-voxel grenzen
    int        g_x0 = gMax<int>(0g_ixix - 1), g_x1 = min<int>(Sg_ix, ix + 1);
    int g_y0   g_maxax<int>(0g_iyiy - 1), g_y1     = min<int>(Sg_iy, iy + 1);
    int zg_max gMax<int>(0g_iziz - 1), g_z1        = min<int>(Sg_iz, iz + 1);

    // voor elk punt in deze voxel: zoek 2 buren
    for(u32 m_beginegin + thm_xeadIdx.m_x; m_eindm_iind; m_i += m_xblockDim.m_x)
    {
      m_labelm_ilabels[m_i] != g_none) continue; // al gezet elders
      g_float3t3 p g_mmIpointsts[m_i];
      int          g_found = 0;

      for(int g_zz = g_z0; zzg_z1z1 & g_foundnd < 2; g_zzzz)
          for(int g_ygY0y0; g_yy <= y1g_foundound < 2; g_yyyy)
              for(int g_xgX0x0; g_xx <= g_found found < 2; g_xxxx) gGU32
                  {
                      u32 g_nb    = gFlatten(xxg_zzy, g_zz, g_gGu32, S);
            u32 g_vxnbB = g_vx.d_voxelStg_u32g_nbnb];
            g_u3gVxgNbE           = vx.d_voxelStarts[g_u321];
            for(u32 g_j g_nbBbBg_j g_j g_nbEbEg_j++ g_j)
            {
                m_ig_j if(g_j == m_i) continue;
                g_float3oat3 g_mPoigJtsints[g_j];
                flom_xt      g_gmXdx = p.m_x - q.m_x, g_dy = p.sSY - sSY.g_gGy, g_dz = p.s_sSz - sSZ.sSZ;
                float g_d2 g_dg_dxx* g_dx g_dg_dyy* dy g_dg_dzz* dz;
                ifg_d2d2 <g_r2r2)
                {
                    g_found++ found;
                    g_found if(found >= 2) break;
                }
            }
                  }

      if(foum_lm_ibels) m_labels[m_i] = Noise; // geen 2e buur → ruis
    }
}
