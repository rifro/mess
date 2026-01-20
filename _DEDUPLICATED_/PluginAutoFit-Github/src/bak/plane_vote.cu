#include <cuda_runtime.h>
#include <cub/cub.cuh>
#include "types.cuh"
#include "consts.cuh"
#include "plane_pipe_utils.cuh"

// 1) voxel-quickcheck: near/far tellen voor vlak- en pipeindicatie
__global__ void k_classificeer_voxels_op_vlak_pipe(
    const Vec3f* __restrict__ points, 
    const VoxelBereik* __restrict__ voxels,
    int aantal_voxels,
    Vec3f test_normaal,
    float d0,
    VoxelClass* __restrict__ outClass
){
    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if (v >= aantal_voxels) return;

    VoxelBereik br = voxels[v];
    int nearCount = 0;
    int farCount = 0;

    for (u32 i = br.start; i < br.eind; ++i){
        float a2 = afstand2_vlak(test_normaal, d0, points[i]);
        if (a2 <= Tuning::epsNearSq) nearCount++;
        else if (a2 >= Tuning::TauFarSq) farCount++;
    }

    if (nearCount >= Tuning::MIN_NEAR && farCount <= Tuning::MAX_FAR){
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
    int aantal_voxels,
    Vec3f vlakNormaal,
    KeyCount* __restrict__ out_emits,
    u32* __restrict__ out_size
){
    __shared__ u32 sKey;
    __shared__ u32 sCount;
    if (threadIdx.x == 0){
        u32 key = kwantiseer_octa_key(hemisfeer_fold(norm(vlakNormaal)), Tuning::OCT_NX, Tuning::OCT_NY);
        sKey = key;
        sCount = 0;
    }
    __syncthreads();

    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if (v < aantal_voxels && klasse[v] == PlaneSeed){
        atomicAdd(&sCount, 1u);
    }
    __syncthreads();

    if (threadIdx.x == 0 && sCount > 0){
        u32 Index = atomicAdd(out_size, 1u);
        out_emits[Index] = { sKey, sCount };
    }
}
