#include <vector>
#include <cuda_runtime.h>
#include "vlak_types.h"
#include "voxel_params.h"

using namespace jbf;
using namespace grid;

// Helpers voor launches (kies je eigen block/grid config)
static inline dim3 blok(u32 n) { return dim3( min<u32>(256,n) ); }
static inline dim3 rooster(u32 n) { return dim3( (n + 255)/256 ); }

// Kernels (extern "C" uit .cu’s)
extern __global__ void k_initLabels(u8*, u32, u8);
extern __global__ void k_ruisBoerenTweedeBuur(const float3*, u8*, u32, VoxelRaster, VoxelIndexing, float);
struct SeedTriangle; // fwd
extern __global__ void k_vindDriehoekZadenLangsRichting(const float3*, const u8*, u32,
                                                        VoxelRaster, VoxelIndexing,
                                                        float3, float, float, float,
                                                        u32, SeedTriangle*, u32*);

// TODO: k_bepaalSlabPunten, k_planaireRegressie, k_bepaalD_TrimK zitten in jouw andere CU’s
extern __global__ void k_bepaalSlabPunten(const float3*, const u8*, u32,
                                          float3, float, float,
                                          u32*, u32, u32*);
extern __global__ void k_bepaalD_TrimK(const float3*, const u32*, u32,
                                       float3, int, float*, float*, u32*);
// d_planaireRegressie zit als __device__ helper; maak evt. wrapper-kernel als je die __device__-weg wil houden

// Orchestratie voor 1 richting
bool vindVlakkenLangsRichting(const float3* d_points,
                              u8* d_labels,
                              u32 N,
                              VoxelRaster vr,
                              VoxelIndexing vx,
                              float3 n_as,
                              const VlakTuning& tun,
                              std::vector<PlaneHyp>& uit)
{
    // 0) ruis-filter
    k_ruisBoerenTweedeBuur<<<vx.numVoxels, 128>>>(d_points, d_labels, N, vr, vx, tun.ruisStraal);
    cudaDeviceSynchronize();

    // 1) zaden (strak: kleine driehoeken, bandpass, hoek tov n_as)
    u32 maxSeeds = 1'000'000; // cap; tuning
    SeedTriangle* d_seeds = nullptr; cudaMalloc(&d_seeds, maxSeeds * sizeof(SeedTriangle));
    u32* d_zCount = nullptr;   cudaMalloc(&d_zCount, sizeof(u32));
    cudaMemset(d_zCount, 0, sizeof(u32));

    float cosAngleMin = cosf(tun.angleMaxDegrees * 3.1415926535f / 180.f);
    k_vindDriehoekZadenLangsRichting<<<vx.numVoxels, 128>>>(d_points, d_labels, N, vr, vx,
                                                            n_as,
                                                            tun.edgeMin, tun.edgeMax,
                                                            cosAngleMin,
                                                            /*maxSeedsPerVoxel*/ 64,
                                                            d_seeds, d_zCount);
    cudaDeviceSynchronize();

    u32 h_zCount=0; cudaMemcpy(&h_zCount, d_zCount, sizeof(u32), cudaMemcpyDeviceToHost);
    if (h_zCount == 0) { cudaFree(d_seeds); cudaFree(d_zCount); return false; }

    // 2) per zaad: slabpunten rond d0 = n_as·centroid → planaireRegressie → d (Trim-K) → gates → output
    //    (hier: demonstratief 1 zaad pakken; jij maakt er een batched variant van)

    // buffers voor slab-gather
    const u32 MAX_IN_SLAG = 200000; // cap; maak dynamisch/batched als nodig
    u32* d_Index = nullptr;   cudaMalloc(&d_Index, MAX_IN_SLAG * sizeof(u32));
    u32* d_cnt = nullptr;   cudaMalloc(&d_cnt, sizeof(u32));

    // kopieer eerste zaad
    SeedTriangle hz; cudaMemcpy(&hz, d_seeds, sizeof(SeedTriangle), cudaMemcpyDeviceToHost);
    float d0 = hz.n.x * hz.c.x + hz.n.y * hz.c.y + hz.n.z * hz.c.z; // of n_as·ctd

    cudaMemset(d_cnt, 0, sizeof(u32));
    k_bepaalSlabPunten<<<rooster(N), blok(N)>>>(d_points, d_labels, N,
                                                n_as, d0, tun.slabEps,
                                                d_Index, MAX_IN_SLAG, d_cnt);
    cudaDeviceSynchronize();

    u32 M=0; cudaMemcpy(&M, d_cnt, sizeof(u32), cudaMemcpyDeviceToHost);
    if (M >= 16) // minimale set
    {
        // 2b) planaire regressie (__device__ helper via wrapper-kernel of inline in aparte kernel)
        // — maak evt. een kleine wrapper: k_planaireRegressieWrapper(points, Index, M, n_in, X,Y,Z, ...)
        // Voor nu: we nemen n_as als n_in en doen d-trimK; later vervang je n_as door n* uit regressie.
        float* d_d=nullptr, *d_rms=nullptr; u32* d_kept=nullptr;
        cudaMalloc(&d_d, sizeof(float)); cudaMalloc(&d_rms, sizeof(float)); cudaMalloc(&d_kept, sizeof(u32));

        k_bepaalD_TrimK<<<1, 1, 0>>>(d_points, d_Index, M, n_as, tun.kTrim, d_d, d_rms, d_kept);
        cudaDeviceSynchronize();

        float h_d=0.f, h_rms=0.f; u32 h_kept=0;
        cudaMemcpy(&h_d, d_d, sizeof(float), cudaMemcpyDeviceToHost);
        cudaMemcpy(&h_rms, d_rms, sizeof(float), cudaMemcpyDeviceToHost);
        cudaMemcpy(&h_kept, d_kept, sizeof(u32), cudaMemcpyDeviceToHost);

        if (h_kept >= tun.minSupport && h_rms <= tun.maxRms) {
            PlaneHyp ph; ph.n = n_as; ph.d = h_d; ph.count = h_kept; ph.rms = h_rms; ph.areaHint = 0;
            uit.push_back(ph);
            // TODO: label points in slab als Plane (aparte kernel die label set op basis van n_as & d)
        }

        cudaFree(d_d); cudaFree(d_rms); cudaFree(d_kept);
    }

    cudaFree(d_Index); cudaFree(d_cnt);
    cudaFree(d_seeds); cudaFree(d_zCount);
    return !uit.empty();
}

bool vindVlakkenHuidigFrame(const float3* d_points, u8* d_labels, u32 N,
                            VoxelRaster vr, VoxelIndexing vx,
                            const VlakTuning& tun,
                            std::vector<PlaneHyp>& uit)
{
    // init labels op None (éénmalig)
    k_initLabels<<<rooster(N), blok(N)>>>(d_labels, N, None);
    cudaDeviceSynchronize();

    bool ok=false;
    ok |= vindVlakkenLangsRichting(d_points, d_labels, N, vr, vx, make_float3(0,0,1), tun, uit); // TB
    ok |= vindVlakkenLangsRichting(d_points, d_labels, N, vr, vx, make_float3(0,1,0), tun, uit); // NZ
    ok |= vindVlakkenLangsRichting(d_points, d_labels, N, vr, vx, make_float3(1,0,0), tun, uit); // OW
    return ok;
}
