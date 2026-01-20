#include <cuda_runtime.h>
#include "vlak_types.h"
#include "math_hulpfuncties.cuh"
#include "planaire_regressie.cuh"

using namespace jbf;

extern "C" {

__global__ void k_bepaalSlabPunten(const float3* __restrict__ points,
                                   const u8* __restrict__ labels,
                                   u32 N,
                                   float3 n,
                                   float  d0,
                                   float  slabEps,
                                   u32* __restrict__ outIndex,
                                   u32  maxOut,
                                   u32* __restrict__ outCount)
{
    // eenvoudige grid-stride scan
    u32 tid = blockIdx.x * blockDim.x + threadIdx.x;
    u32 stride = blockDim.x * gridDim.x;

    for (u32 i=tid; i<N; i+=stride) {
        if (labels[i] != None) continue;
        float s = d_dot3(n, points[i]);
        if (fabsf(s - d0) <= slabEps) {
            u32 pos = atomicAdd(outCount, 1u);
            if (pos < maxOut) outIndex[pos] = i; // eenvoudige overrun bescherming
        }
    }
}

__global__ void k_bepaalD_TrimK(const float3* __restrict__ points,
                                const u32* __restrict__ Index,
                                u32 M,
                                float3 n_star,
                                int k_trim,
                                float* d_out,
                                float* rmsOut,
                                u32* kept_out)
{
    // Single-block kernel verwacht; voor eenvoud (kun je later parallen)
    float sum = 0.f; u32 cnt = 0;

    // kleine buffers top-k
    extern __shared__ float sh[]; // we gebruiken registers hieronder voor simpelheid
    float minK[8]; float maxK[8]; // neem k_trim<=8
    #pragma unroll
    for (int k=0;k<8;++k){ minK[k]= CUDART_INF_F; maxK[k]= -CUDART_INF_F; }

    for (u32 j=0; j<M; ++j) {
        float r = d_dot3(n_star, points[Index[j]]);
        sum += r; ++cnt;

        // plaats in minK (kleinste)
        int m_min=0; for (int k=1;k<k_trim;++k) if (minK[k] > minK[m_min]) m_min=k;
        if (r < minK[m_min]) minK[m_min] = r;

        // plaats in maxK (grootste)
        int m_max=0; for (int k=1;k<k_trim;++k) if (maxK[k] < maxK[m_max]) m_max=k;
        if (r > maxK[m_max]) maxK[m_max] = r;
    }

    float sumMin=0.f, sumMax=0.f;
    for (int k=0;k<k_trim;++k){ sumMin+=minK[k]; sumMax+=maxK[k]; }
    u32 kept = (cnt > (u32)(2*k_trim)) ? (cnt - 2*k_trim) : 0u;
    float d = (kept? (sum - sumMin - sumMax) / (float)kept : 0.f);

    // RMS (optioneel): tweede pass over kept
    float sse=0.f; u32 kc=0;
    for (u32 j=0; j<M; ++j) {
        float r = d_dot3(n_star, points[Index[j]]);
        bool isMin=false, isMax=false;
        #pragma unroll
        for (int k=0;k<k_trim;++k){ if (fabsf(r - minK[k])<1e-12f) {isMin=true; break;} }
        #pragma unroll
        for (int k=0;k<k_trim;++k){ if (fabsf(r - maxK[k])<1e-12f) {isMax=true; break;} }
        if (isMin || isMax) continue;
        float e = r - d; sse += e*e; ++kc;
    }
    float rms = (kc? sqrtf(sse/(float)kc) : 0.f);

    if (threadIdx.x==0) {
        *d_out = d; *rmsOut = rms; *kept_out = kept;
    }
}

} // extern "C"
