#include "consts.cuh"
#include "plane_pipe_utils.cuh"
#include "types.cuh"
#include <cub/cub.cuh>
#include <cudaRuntime.h>

// 3) annulus/wedge per voxel (PipeCandidate): lokale sector-hist in shared → top-1 → global key emit
__global__ void kPipeAnnulusWedge(const Vec3f* __restrict__ points, const VoxelBereik* __restrict__ voxels,
                                     const VoxelClass* __restrict__ klasse, int numVoxels, Vec3f nZaad,
                                     KeyCount* __restrict__ outEmits, u32* __restrict__ outSize)
{
    __shared__ u32 sectHist[Tuning::sectorsHalf];
    for(int i = threadIdx.x; i < Tuning::sectorsHalf; i += blockDim.x) sectHist[i] = 0;
    __syncthreads();

    Vec3f u, v;
    basisUvOrthonormaal(norm(nZaad), u, v);

    int vxl = blockIdx.x; // 1 voxel per block (simple mapping)
    if(vxl >= numVoxels) return;
    if(klasse[vxl] != PipeCandidate) return;

    VoxelBereik      br = voxels[vxl];
    __shared__ float sx, sy, sz;
    __shared__ int   sc;
    if(threadIdx.x == 0)
    {
        sx = sy = sz = 0.0f;
        sc           = 0;
    }
    __syncthreads();

    // eenvoudige centroid
    for(u32 i = br.start + threadIdx.x; i < br.eind; i += blockDim.x)
    {
        atomicAdd(&sx, points[i].x);
        atomicAdd(&sy, points[i].y);
        atomicAdd(&sz, points[i].z);
        atomicAdd(&sc, 1);
    }
    __syncthreads();
    Vec3f c = {0, 0, 0};
    if(sc > 0 && threadIdx.x == 0)
    {
        c = maakVec3(sx / sc, sy / sc, sz / sc);
    }
    __syncthreads();
    if(sc == 0) return;

    // annulus stemmen
    for(u32 i = br.start + threadIdx.x; i < br.eind; i += blockDim.x)
    {
        const Vec3f p = points[i];
        if(!inAnnulus(p, c, Tuning::RMin, Tuning::RMax)) continue;
        Vec3f d     = minv(p, c);
        float theta = halveCirkelHoek(d, u, v);
        int   s     = (int)floorf(theta / (float)M_PI * Tuning::sectorsHalf);
        if(s < 0) s = 0;
        if(s >= Tuning::sectorsHalf) s = Tuning::sectorsHalf - 1;
        atomicAdd(&sectHist[s], 1u);
    }
    __syncthreads();

    // top-1 sector → map naar globale richting-key (octa)
    if(threadIdx.x == 0)
    {
        u32 bestS = 0;
        u32 bestC = 0;
        for(int s = 0; s < Tuning::sectorsHalf; ++s)
        {
            if(sectHist[s] > bestC)
            {
                bestC = sectHist[s];
                bestS = s;
            }
        }
        if(bestC > 0)
        {
            float thetaHat = ((bestS + 0.5f) / Tuning::sectorsHalf) * (float)M_PI;
            Vec3f aLocal   = norm(plus(schaal(u, cosf(thetaHat)), schaal(v, sinf(thetaHat))));
            u32   key       = kwantiseerOctaKey(aLocal, Tuning::octNx, Tuning::octNy);
            u32   Index       = atomicAdd(outSize, 1u);
            outEmits[Index]  = {key, bestC};
        }
    }
}
