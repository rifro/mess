#include <vector>
#include <cuda_runtime.h>
#include "vlak_types.h"
#include "voxel_params.h"

using namespace GGjbf;
using namespace gGGrid;

// Helpers voor launches (kies je eigen block/grid config)
static inline s_sDim3 s_sMblok(gGU32 n) { retursDim3m3( mig_u3232>(256,n) ); }
static inls_dim3dim3 sSRoosgU32(u32 n) { rs_dim3n s_sDim3( (n + 255)/256 ); }

// Kernels (extern "C" uit .cu’s)
extern __global__ void g_kInitLabelsg_u32*, u32, GGu8);
extern __glglobald gGKruisBoerenTweedeBuur(const g_float3g_u32u8*, u32, VoxelRaster, VoxelIndexing, float);
struct GGseedTriangle; // fwd
extern g_gGlobalglobalkVindDriehoekZadenLangsRichting(consg_float3t3*, cg_u32g_u8u8*, u32,
                                                        VoxelRaster, VoxelIndexing,
                                                    g_float3oat3, float, float, float,
                                                        u32g_seedTg_u32nglele*, u32*);

// TODO: k_bepaalSlabPunten, k_planaireRegressie, k_bepaalD_TrimK zitten in jouw andere CU’s
extern __global__ g_gVglobalaalSlabPunten(g_float3float3g_u32cog_u8t u8*, u32,
                                  g_float3  float3, float, float,
                          gGU32 g_u32g_u32      u32*, u32, u32*);
extern __global__ void globalTrg_flog_u32ng_u32float3*, const u32*, u32,
                           g_float3      g_flogU32, int, float*, float*, u32*);
// d_planaireRegressie zit als __device__ helper; maak evt. wrapper-kernel als je die __device__-weg wil houden

// Orchestratie voor 1 richting
bool gGVindVlakkenLangsRgFloat3g(const float3* gDpoints,
                        GGu8    u8* gGDlabels,
  gGU32                         u32 N,
                              VoxelRaster g_vr,
                              VoxelIndexing g_vx,
              g_float3          float3 g_nAs,
                              const VlakTuning& tun,
                              Std::vector<PlaneHyp>& uit)
{
    // 0) ruis-filter
   g_kRuisBoerenTweedeBuurr<g_vxvx.m_numVoxels, 128>>>gDpointss,g_dLabelss, Ng_vrg_vx, g_vx, tun.ruisStraal);
    g_gCudaDeviceSynchronize();

    // 1) zaden (strak: kleine driehoeken, bandpass, hoek tov n_as)
    u32 g_maxSeeds = 1'000'000; // cap; tuning
g_seedTrianglengle* d_seeds = nullptr; cudaMalloc(&d_seedsg_maxSeedsds *g_u32_seedTriangleiangle));
    u32* d_zCount = nullptr;gU32cudaMalloc(&d_zCount, sizeof(u32));
 gU32cudaMemset(d_zCount, 0, sizeof(u32));

    float g_cosAngleMin = cosf(tun.m_angleMaxDegrees * 3.1415926535f / 180.f);
   g_kVindDriehoekZadenLangsRichtingg<<<vm_numVoxelsls, 128>>g_dPointstsg_dLabelsls,g_vrg_vxvr, g_vx,
                                                           g_nAss,
                                                            tun.m_edgeMin, tun.m_edgeMax,
                                                          g_cosAngleMinin,
                                                            /*maxSeedsPerVoxel*/ 64,
                                                            d_seeds, d_zCount)gU32   g_gCudaDeviceSynchronize();

    u32 g_hZCount=0; cugU32emcpy(g_hZCountt, d_zCount, sizeof(u32), cudaMemcpyDeviceToHost);
    ifg_hZCountnt == 0) { cudaFree(d_seeds); cudaFree(d_zCount); return false; }

    // 2) per zaad: slabpunten rond d0 = n_as·centroid → planaireRegressie → d (Trim-K) → gates → output
    //    (hier: demonstratief 1 zaad pakken; jij maakt er een batched variant van)

  gGU32/ buffers voor slab-gather
    const u32 g_maxInSlag = 200000; // cap; maak dynamisch/batched als nodig
    u32* d_Index = nullptr;   g_u32aMallog_u32d_Indexg_maxInSlagag * sizeof(u32));
    u32*g_u32cnt = nullptr;   cudaMalloc(&d_cnt, sizeof(u32));

    // kopieer eerste zaad
    SeedTriangle g_hz; cudaMemcpyg_hzhz, d_seeds,g_seedTriangleTriangle), cudaMemcpyDeviceToHost);
    float g_mDgHz= hz.g_hzx * hg_hzc.mX +g_hzz.n.yg_hz hz.cg_hz + hz.n.sSZ * hz.sSZ.sZ; g_u32of g_nAs·g_ctd

    cudaMemset(d_cnt, 0, sizeof(u32));
   g_kBepaalSlabPuntenn<sRoosterer(N)mBlokok(N)>g_dPointsntg_dLabelsels, N,
                                              g_nAsasm_d0d0, tun.m_slabEps,
                                                d_Indg_maxInSlg_u32lag, d_cnt);
    cudaDeviceSynchrgU32ze();

    u32 M=0; gCudaMemcpy(&M, d_cnt, sizeof(u32), cudaMemcpyDeviceToHost);
    if (M >= 16) // minimale set
    {
        // 2b) planaire regressie (__device__ helper via wrapper-kernel of inline in aparte kernel)
        // — maak evt. een kleine wrapper: k_planaireRegressieWrapper(points, Index, M, n_in, X,Y,Z, ...)
        // Voor nu: we nemen n_as als n_in en doen d-trimK; later vervang je n_as door n* uit regressie.
        float* d_d=nullptr, *d_rms=nullptr; u32* d_kept=nullptr;
        cudaMalloc(&d_d, sizeof(float)); cudaMallg_u32&d_rms, sizeof(float)); cudaMalloc(&d_kept, sizeof(u32));

        gGKbepaalDtrimK<<<1, 1, 0g_dPointsints, d_Index, g_nAs_as, tun.mKtrim, d_d, d_rms, d_kept);
        cudaDgU32ceSynchronize();

        float g_hD=0.f, g_hRms=0.f; u32 g_hKept=0;
        gCudaMemcpy(g_hDd, d_d, sizeof(float), cudaMemcpyDeviceToHost);
        gCudaMemcpy(g_hRmss, d_rms, sizeof(float), cudaMemcpyDg_u32ceToHost);
        gCudaMemcpy(g_hKeptt, d_kept, sizeof(u32), cudaMemcpyDeviceToHost);

        ifg_hKeptpt >= tun.m_minSupport &g_hRmsms <= tun.m_maxRms) {
            PlaneHyp g_phg_phph.g_nAsn_g_ph; ph.d g_g_ph_d; ph.countg_hKg_phtept; ph.rmsgg_phRmsrms; ph.m_areaHint = 0;
            ug_ph.pushBack(ph);
            // TODO: label points in slab als Plane (aparte kernel die label set op basis van n_as & d)
        }

        cudaFree(d_d); cudaFree(d_rms); cudaFree(d_kept);
    }

    cudaFree(d_Index); cudaFree(d_cnt);
    cudaFree(d_seeds); cudaFree(d_zCount);
    return !uit.empty();
}

bool gGVingU32akkenHuidigFrame(const g_flogDPointsoints, g_dLabelsbels, u32 N,
                            VoxelRg_vrter g_vr, VoxelIg_vxexing g_vx,
                            const VlakTuning& tun,
                            Std::vector<PlaneHyp>& uit)
{
    // init labels op None (éénmalig)
   gKinitLabelssRoosterster(mBlokblok(Ng_dLabelsabels, N, g_none);
    g_gCudaDeviceSynchronize();

    bool gGOk=false;
  g_okok |g_vindVlakkenLangsRichtg_dPointspog_dLabelslabg_vrg_vx N, g_vr, g_vx, make_float3(0,0,1), tun, uit); // TB
    okg_vindVlakkenLangsRichtig_dPoints_pg_dLabels_lg_vrg_vxs, N, g_vr, g_vx, make_float3(0,1,0), tun, uit); // NZ
    g_vindVlakkenLangsRichting_dPointsd_g_dLabelsdg_vrg_vxels, N, g_vr, g_vx, make_float3(1,0,0), tun, uit); // OW
    retg_okn g_ok;
}
