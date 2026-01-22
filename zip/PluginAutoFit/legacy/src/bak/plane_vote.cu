#include "consts.cuh"
#include "plane_pipe_utils.cuh"
#include "types.cuh"
#include <cub/cub.cuh>
#include <cudaRuntime.h>

// 1) voxel-quickcheck: near/far tellen voor vlak- en pipeindicatie
__global__ void kClassificeerVoxelsOpVlakPipe(const Vec3f* __restrict__ points,
                                                   const VoxelBereik* __restrict__ voxels, int aantalVoxels,
                                                   Vec3f testNormaal, float d0, VoxelClass* __restrict__ outClass)
{
    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if(v >= aantalVoxels) return;

    VoxelBereik br        = voxels[v];
    int         nearCount = 0;
    int         farCount  = 0;

    for(u32 i = br.start; i < br.eind; ++i)
    {
        float a2 = afstand2Vlak(testNormaal, d0, points[i]);
        if(a2 <= Tuning::epsNearSq)
            nearCount++;
        else if(a2 >= Tuning::TauFarSq)
            farCount++;
    }

    if(nearCount >= Tuning::MIN_NEAR && farCount <= Tuning::MAX_FAR)
    {
        outClass[v] = PlaneSeed;
    } else if(nearCount < 2 && farCount > 4)
    {
        outClass[v] = PipeCandidate;
    } else if(nearCount == 0 && farCount == 0)
    {
        outClass[v] = Noise;
    } else
    {
        outClass[v] = None;
    }
}

// 2) block-lokale stemmen naar compacte (key,count) records; 1 richting per pass
__global__ void kStemVlakRichtingBlock(const VoxelClass* __restrict__ klasse, int aantalVoxels,
                                           Vec3f vlakNormaal, KeyCount* __restrict__ outEmits,
                                           u32* __restrict__ outSize)
{
    __shared__ u32 sKey;
    __shared__ u32 sCount;
    if(threadIdx.x == 0)
    {
        u32 key = kwantiseerOctaKey(hemisfeerFold(norm(vlakNormaal)), Tuning::OCT_NX, Tuning::OCT_NY);
        sKey   = key;
        sCount = 0;
    }
    __syncthreads();

    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if(v < aantalVoxels && klasse[v] == PlaneSeed)
    {
        atomicAdd(&sCount, 1u);
    }
    __syncthreads();

    if(threadIdx.x == 0 && sCount > 0)
    {
        u32 Index        = atomicAdd(outSize, 1u);
        outEmits[Index] = {sKey, sCount};
    }
}
