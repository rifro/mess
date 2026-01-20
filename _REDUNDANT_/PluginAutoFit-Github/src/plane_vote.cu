#include <cuda_runtime.h>
#include <cub/cub.cuh>
#include "types.cuh"
#include "consts.cuh"
#include "plane_pipe_utils.cuh"

// 1) voxel-quickcheck: near/far tellen voor vlak- en pipeindicatie
__global__ void k_classificeer_voxels_op_vlak_pipe(
    const Vec3f* __restrict__ points, 
    const VoxelBereik* __restrict__ voxels,
    int numVoxels,
    Vec3f testNormal,
    float d0,
    VoxelClass* __restrict__ outClass
){
    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if (v >= numVoxels) return;

    VoxelBereik br = voxels[v];
    int nearCount = 0;
    int farCount = 0;

    for (u32 i = br.start; i < br.eind; ++i){
        float a2 = afstand2_vlak(testNormal, d0, points[i]);
        if (a2 <= Tuning::epsNearSq) nearCount++;
        else if (a2 >= Tuning::TauFarSq) farCount++;
    }

    if (nearCount >= Tuning::MinNear && farCount <= Tuning::MaxFar){
        outClass[v] = PlaneSeed;
    } else if (nearCount < 2 && farCount > 4){
        outClass[v] = PipeCandidate;
    } else if (nearCount==0 && farCount==0){
        outClass[v] = Noise;
    } else {
        outClass[v] = None;
    }
}

// 2) block-lokale stemmen naar compacte (key,count) records; 1 richting per pass
__global__ void k_stem_vlak_richting_block(
    const VoxelClass* __restrict__ klasse,
    int numVoxels,
    Vec3f planeNormal,
    KeyCount* __restrict__ outEmits,
    u32* __restrict__ outSize
){
    __shared__ u32 sKey;
    __shared__ u32 sCount;
    if (threadIdx.x == 0){
        u32 key = kwantiseer_octa_key(hemisfeer_fold(norm(planeNormal)), Tuning::octNx, Tuning::octNy);
        sKey = key;
        sCount = 0;
    }
    __syncthreads();

    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if (v < numVoxels && klasse[v] == PlaneSeed){
        atomicAdd(&sCount, 1u);
    }
    __syncthreads();

    if (threadIdx.x == 0 && sCount > 0){
        u32 Index = atomicAdd(outSize, 1u);
        outEmits[Index] = { sKey, sCount };
    }
}
