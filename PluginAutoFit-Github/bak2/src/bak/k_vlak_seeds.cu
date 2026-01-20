#include <cuda_runtime.h>
#include "vlak_types.h"
#include "voxel_params.h"
#include "math_utils.cuh"          // jouw device dot/cross/norm helpers (d_*)

using namespace jbf;
using namespace grid;

// Output-struct (compact) voor een driehoek-zaad
struct SeedTriangle {
    u32 i, j, k; // indices in d_points
    float3   n;       // unit normaal van de driehoek
    float3   c;       // centroid (optioneel voor d0)
};

// NB: we limiteren zaden per voxel om combinatorische explosie te voorkomen.
__global__ void k_vindDriehoekZadenLangsRichting(const float3* __restrict__ points,
                                                 const u8* __restrict__ labels,
                                                 u32 N,
                                                 VoxelRaster vr,
                                                 VoxelIndexing vx,
                                                 float3 n_doel,           // (0,0,1) etc.
                                                 float edgeMin, float edgeMax,
                                                 float cosAngleMin,        // cos(maxΔhoek)
                                                 u32 maxSeedsPerVoxel,
                                                 SeedTriangle* __restrict__ zaden,
                                                 u32* __restrict__ seedCount)
{
    u32 v = blockIdx.x; // 1 CTA per voxel (kan ook tiled)
    if (v >= vx.numVoxels) return;

    u32 begin = vx.d_voxelStarts[v];
    u32 eind  = vx.d_voxelStarts[v+1];
    if (begin + 2 >= eind) return;

    // decode v → (ix,iy,iz)
    u32 S = vr.S;
    u32 iz = v / (S*S);
    u32 iy = (v / S) % S;
    u32 ix = v % S;

    // neem buurt (3x3x3)
    int x0 = max<int>(0, ix-1), x1 = min<int>(S-1, ix+1);
    int y0 = max<int>(0, iy-1), y1 = min<int>(S-1, iy+1);
    int z0 = max<int>(0, iz-1), z1 = min<int>(S-1, iz+1);

    // verzamel lokale index-range lijst (optioneel: direct itereren zoals hieronder)
    u32 lokaalMax = 4096; // cap; pas aan of maak dynamisch
    __shared__ u32 lokIndex[4096];
    __shared__ u32 L;
    if (threadIdx.x == 0) L = 0;
    __syncthreads();

    for (int zz=z0; zz<=z1; ++zz)
    for (int yy=y0; yy<=y1; ++yy)
    for (int xx=x0; xx<=x1; ++xx)
    {
        u32 nb = flatten(xx,yy,zz,S,S,S);
        u32 nbB = vx.d_voxelStarts[nb];
        u32 nbE = vx.d_voxelStarts[nb+1];

        for (u32 t=nbB + threadIdx.x; t<nbE; t+=blockDim.x) {
            if (labels[t] != None) continue;
            u32 pos = atomicAdd(&L, 1u);
            if (pos < lokaalMax) lokIndex[pos] = t;
        }
        __syncthreads();
    }
    if (L < 3) return;

    // elke thread pakt een i en maakt enkele combinaties j,k in de buurt
    u32 seedsEmitted = 0;
    for (u32 a = threadIdx.x; a < L && seedsEmitted < maxSeedsPerVoxel; a += blockDim.x)
    {
        u32 i = lokIndex[a];
        float3 Pi = points[i];

        // kies beperkt aantal buren rondom a (bandbreedte beperken)
        const u32 KMAX = 16;
        for (u32 b = a+1; b < min(L, a+1+KMAX) && seedsEmitted < maxSeedsPerVoxel; ++b)
        for (u32 c = b+1; c < min(L, b+1+KMAX) && seedsEmitted < maxSeedsPerVoxel; ++c)
        {
            u32 j = lokIndex[b], k = lokIndex[c];
            if (labels[j] != None || labels[k] != None) continue;
            float3 Pj = points[j], Pk = points[k];

            // edge-lengtes
            float3 e1 = make_float3(Pj.x-Pi.x, Pj.y-Pi.y, Pj.z-Pi.z);
            float3 e2 = make_float3(Pk.x-Pi.x, Pk.y-Pi.y, Pk.z-Pi.z);
            float L1 = sqrtf(e1.x*e1.x + e1.y*e1.y + e1.z*e1.z);
            float L2 = sqrtf(e2.x*e2.x + e2.y*e2.y + e2.z*e2.z);
            float L3 = sqrtf((Pk.x-Pj.x)*(Pk.x-Pj.x) + (Pk.y-Pj.y)*(Pk.y-Pj.y) + (Pk.z-Pj.z)*(Pk.z-Pj.z));

            if (L1 < edgeMin || L2 < edgeMin || L3 < edgeMin) continue;
            if (L1 > edgeMax || L2 > edgeMax || L3 > edgeMax) continue;

            // triangle normaal
            float3 n = jbf::d_norm3( jbf::d_cross3(e1, e2) );
            float c = fabsf(jbf::d_dot3(n, n_doel));
            if (c < cosAngleMin) continue; // wijkt teveel af van doel-normaal

            // centroid
            float3 ctd = make_float3((Pi.x+Pj.x+Pk.x)/3.f,
                                     (Pi.y+Pj.y+Pk.y)/3.f,
                                     (Pi.z+Pj.z+Pk.z)/3.f);

            // schrijf uit
            u32 outPos = atomicAdd(seedCount, 1u);
            zaden[outPos] = { i, j, k, n, ctd };
            ++seedsEmitted;
        }
    }
}
