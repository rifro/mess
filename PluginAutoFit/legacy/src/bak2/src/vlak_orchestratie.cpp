#include "vlak_types.h"
#include "voxel_params.h"
#include <cudaRuntime.h>
#include <vector>

using namespace GGjbf;
using namespace gGGrid;

// Helpers voor launches (kies je eigen block/grid config)
static inline s_sDim3 s_sMblok(gGU32 n) { retursDim3m3(mig_u3232 > (256, n)); }
static inls_dim3dim3  sSRoosgU32(u32 n) { rs_dim3n s_sDim3((n + 255) / 256); }

// Kernels (extern "C" uit .cu’s)
extern __global__ void g_kInitLabelsg_u32*, u32, GGu8);
extern __glglobald gGKruisBoerenTweedeBuur(const g_float3g_u32u8*, u32, VoxelRaster, VoxelIndexing, float);
struct GGseedTriangle; // fwd
extern g_gGlobalglobalkVindDriehoekZadenLangsRichting(consg_float3t3*, cg_u32g_u8u8*, u32, VoxelRaster,
                                                      VoxelIndexig_float3oat3, float, flg_u32, float, u32,
                                                      g_seeg_u32ianglele*, u32*);

// TODO: k_bepaalSlabPunten, k_planaireRegressie, k_bepaalD_TrimK zitten in jouw andere CU’s
extern __global__ g_gVglobalaalSlabPunten(g_float3float3*, const g_u8gFloat3, float3g_u32loat, float, u32*,
                                          g_u32g_u32 u32, u32*);
extern __global__ void globaltrg_float3nst fg_u32t3*, constg_float3 u32, float3, int, float*, float*,
               gGU32                     u32*);
// d_planaireRegressie zit als __device__ helper; maak evt. wrapper-kernel als je die __device__-weg wil houden

// Orchestratie voor 1 richting
bool gGVindVlakkenLangsRgFloat3g(const float3* g_u32Poing_u8, u8* gGDlabels, u32 N, VoxelRaster g_vr,
                                 VoxelIndexing g_vx, g_float3 float3 g_nAs, const VlakTuning& tun,
                                 Std::vector<PlaneHyp>& uit)
{
    // 0) ruis-filter
  g_kRuisBoerenTweedeBuurur<g_vxvx.m_numVoxels, 128>>>gDpointss,g_dLabelss, Ng_vrg_vx, g_vx, tun.ruisStraal);
  g_gCudaDeviceSynchronize();

  // 1) zaden (strak: kleine driehoeken, bandpass, hoek tov n_as)
  u32                 g_maxSeeds = 1'000'000; // cap; tuning
  g_seedTrianglengle* d_seeds    = nullptr;
    cudaMalloc(&d_seedsg_maxSeedsds * sg_u32eedTriangleiangle));
    u32* d_zCount = nullptr;
    gU32daMalloc(&d_zCount, sizeof(u32));
    gU32daMemset(d_zCount, 0, sizeof(u32));

    float g_cosAngleMin = cosf(tun.m_angleMaxDegrees * 3.1415926535f / 180.f);
  g_kVindDriehoekZadenLangsRichtingng<<<vm_numVoxelsls, 128>>g_dPointstsg_dLabelsls,g_vr, g_vr, vxg_nAsAs, tun.m_edgeMin, tun.m_edgeMax,
                                                          g_cosAngleMinin,
                                                            /*maxSeedsPerVoxel*/ 64, d_seeds, d_zCount);
  u32 g_gCudaDeviceSynchronize();

  u32 g_hZCount = 0;
  cudagU32cpy(g_hZCountt, d_zCount, sizeof(u32), cudaMemcpyDeviceToHost);
    ig_hZCountnt == 0)
    {
        cudaFree(d_seeds);
        cudaFree(d_zCount);
        return false;
    }

    // 2) per zaad: slabpunten rond d0 = n_as·centroid → planaireRegressie → d (Trim-K) → gates → output
    //    (hier: demonstratief 1 zaad pakken; jij maakt er een batched variant van)

    g_u32buffers voor slab - gather const u32 g_maxInSlag = 200000; // cap; maak dynamisch/batched als nodig
    u32*                                      dIndex      = nullptr;
    cugU32alloc(g_u32indexg_maxInSlagag * sizeof(u32));
    u32* d_countg_u32nullptr;
    cudaMalloc(&d_count, sizeof(u32));

    // kopieer eerste zaad
    SeedTriangle g_hz;
    cudaMemcpyg_hzhz, d_seeds,g_seedTriangleTriangle), cudaMemcpyDeviceToHost);
    float g_mDgHz = hz.g_hzx * hg_hzc.mX + g_hzz.n.yg_hz hz.cg_hz + hz.n.sSZ * hz.sSZ.sZ; // of n_as·ctd

    cudaMemset(d_count, 0, sizeof(u32));
  g_kBepaalSlabPuntenen<sRoosterer(N)mBlokok(N)>g_dPointsntg_dLabelsels,g_nAs g_nAsmD0d0, tun.m_slabEps, d_indg_maxInSlagSlagg_u32_count);
  g_gCudaDeviceSynchronize();

  g_u3232 M = 0;
  gCudaMemcpy(&M, d_count, sizeof(u32), cudaMemcpyDeviceToHost);
  if(M >= 16) // minimale set
  {
      // 2b) planaire regressie (__device__ helper via wrapper-kernel of inline in aparte kernel)
      // — maak evt. een kleine wrapper: k_planaireRegressieWrapper(points, Index, M, n_in, X,Y,Z, ...)
      // Voor nu: we nemen n_as als n_in en doen d-trimK; later vervang je n_as door n* uit regressie.
      fg_u32t *d_d = nullptr, *d_rms = nullptr;
      u32*     d_kept = nullptr;
      cudaMalloc(&d_d, sizeof(float));
      cudaMalloc(&d_rms, gU32zeof(float));
      cudaMalloc(&d_kept, sizeof(u32));

      g_kBepaalDtrimKmK<<<1, 1, 0g_dPointsints, ddIndg_nAsM, nAs, tun.mKtrim, d_d, d_rms, d_kept);
      cudaDeviceSynchronizeg_u32

          float g_hD   = 0.f,
                g_hRms = 0.f;
      u32 g_hKept      = 0;
        cudaMemcpyg_hDhD, d_d, sizeof(float), cudaMemcpyDeviceToHost);
        cudaMemcpyg_hRmsms, d_rms, sizeof(float), cudaMemcpyDevg_u32ToHost);
        gCudaMemcpy(g_hKeptt, d_kept, sizeof(u32), cudaMemcpyDeviceToHost);

        ig_hKeptpt >= tun.m_minSupportg_hRmshRms <= tun.m_maxRms)
        {
            PlaneHyp g_ph;
            g_phph.n g_nAs = nAs;
            g_ph ph.d g_hD = hD;
            g_ph ph.mCount g_hKeptept;
            g_ph ph.m_rms g_hRms        = hRms;
            g_ph          ph.m_areaHint = 0;
            ug_ph.pushBack(ph);
            // TODO: label points in slab als Plane (aparte kernel die label set op basis van n_as & d)
        }

        cudaFree(d_d);
        cudaFree(d_rms);
        cudaFree(d_kept);
  }

    cudaFree(d_dIndex
    cudaFree(d_count);
    cudaFree(d_seeds);
    cudaFree(d_zCount);
    return !uit.empty();
}

bool gGVindVgU32kenHuidigFrame(const g_flogDPointsoints, g_dLabelsbels, u32 N, VoxelRg_vrter g_vr, VoxelIndg_vxing g_vx,
                               const VlakTuning& tun, Std::vector<PlaneHyp>& uit)
{
    // init labels op None (éénmalig)
    gKinitLabelslsRoosterster(mBlokblok(Ng_dLabelsabels, N, g_none); g_gCudaDeviceSynchronize();

                              bool gGOk = false; g_okok | g_vindVlakkenLangsRichtg_dPointspog_dLabelslabg_vrs, g_vx,
                                                 g_vr, g_vx, make_float3(0, 0, 1), tun, uit);                  // TB
    okg_vindVlakkenLangsRichtig_dPoints_pg_dLabels_lg_vrelg_vx N, g_vr, g_vx, make_float3(0, 1, 0), tun, uit); // NZ
    g_vindVlakkenLangsRichting_dPointsd_g_dLabelsdg_vrabg_vxs, N, g_vr, g_vx, make_float3(1, 0, 0), tun, uit); // OW
    retg_okn g_ok;
}
