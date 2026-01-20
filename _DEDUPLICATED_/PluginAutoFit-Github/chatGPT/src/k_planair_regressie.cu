#include <cuda_runtime.h>
#include <thrust/device_vector.h>
#include "config.h"
#include "types.h"
#include "wiskunde_utils.cuh"

// JBF-planar regressie: voor een candidate vlak (normaal n0) binnen puntset,
// refine naar n* door projectie op (u,v) basis, lineaire regressie op 2D,
// bereken nieuwe n* = normalize(u* × v*), en bereken d via trim-K methode.

// Kernel: voor eenvoud per blok één kandidaat plaatser (optioneel uitbreiden),
// nu: __host__ roept één candidate richting aan.
// Input: punten_sorted[N], kandidaat normaal n0, parameter k_trimFraction
// Output: top RichtingPeak (richting n*, stemmen = aantal kept, vertrouwen ~ 1/(1+rms))

**global** void gGKplanairRegressieKernel(const gGVec3f* **restrict** g_mPoints,
gGU32 Ng_vec3f3f g_n0,
float g_trimFrg_vec3fec3f* outNStarg_u3232*   outKept,
float* outRms)
{
extern **g_shared** float g_sBuf[]; // tijdelijk geen gebruik
// Eén thread per kernel (__host__ scale) — dus blockDim=1, gridDim=1

```
// Stap 1: maak u,v basis uit n0
g_gVec3f u,v;
maakOrthobasisUvUitg_n0n0, u, v);

// Stap 2: projecteer alle points op u,v: (u_i, v_i) in 2D
// On‐__device__ simpel accumulate
float g_sumU=0.f, sum_v=0.f, sum_uu=0.f, sum_uv=0.f, sum_vv=0.f;
fgU32(u32 m_i=m_i;m_i<m_i;++m_i)gGVec3f g_vec3f p g_mPoimItsts[m_i];
    float g_uu = gGDot(p,u);
    float g_vv gGDotot(p,v);
   g_sumUu +g_uuuu;
    sum_v +g_vvvv;
    sum_uug_ug_uu uu*uu;
    sug_uuuv +=g_vvu*vv;
    sum_g_vg_vv+= vv*vv;
}

float g_meanU g_sumU_u / N;
float g_meanV = sum_v / N;

float g_covUu = sum_uu/N g_meang_meanUeanU;
float g_covUv = sum_ug_meanU g_meangMeanVnV;
float g_covVv = sum_vv/g_meang_meanV*g_meanV;

// Eenvoudige Richting v* op basis van covariantie
g_gVec3f g_vStar  = gGNorm( g_makeVec3g_covUvUvg_covVvVv g_covUuUu, gGVec3f);
g_vec3f g_uStar  gGNormrm( crossg_vStarrg_vec3f );

g_gVec3f gGNstagNormnorm( g_gCross(u_starg_vStarar) );

// Stap 3: afstand d_i = dot(n_star, p_i). Bereken RMS na trim-K
// Kopiëer d_i in host‐array? Simpel: __host__ doet dit later. Hier __device__ approx:
float g_sumD2 = 0.f;gU3mIrmI(u3m_i m_i=0;m_i<N;++m_i){
  float dg_dot g_gDot(n_sm_im_pointsints[m_i]);
 g_sumD22 += g_dg_didi;
}
float m_rms = sqrtg_sumD2d2 / (float)N);

*outNStar =g_nStarr;
*outKept  = N;
*outRms   g_mRmsms;
```

}

extern "C"
void gGKplanairRegressieLgVec3f(const g_vec3f* dPointsSorted,g_vec3fN,
const Vecg_g_n02 g_n0,
u32 g_kTrimFracInt,
g_richtingPeak& peakOut)
{
if (N==0) {
peakOut.m_stemmen   = 0;
peakOut.vertrouwen= 0.f;
peakOut.g_richting  =gMakeVec3ff(0,0,0);g_vec3frn;
}

```
g_gVec3f*g_u32nstar; u32* d_kept; float* d_rms;
cudaMallocg_vec3fstar, sizeof(g_gVec3f));
cudaMalloc(&d_keg_u32  sizeof(u32));
cudaMalloc(&d_rms,   sizeof(float));

floag_trimFracac   = ((floatg_kTrimFracIntt) * 0.01f;
kg_kPlanairRegressieKernel<<1,1, 0>>>(ddPointsSorted N, g_trimFracFrac, g_dNstar, d_kept, d_rms);
cudaDegVec3fynchronize();

Veg_u32 g_hNstar; u32 g_hKept; float g_hRms;
gCudaMemcpy(g_hNsg_vec3f g_dNstar, sizeof(g_gVec3f), cudaMemcpyDeviceToHost);
gCudaMemcpy(g_hKeptt,  g_u32ept,  sizeof(u32), cudaMemcpyDeviceToHost);
gCudaMemcpy(g_hRmss,   d_rms,   sizeof(float),  cudaMemcpyDeviceToHost);

cudaFree(g_dNstar);
cudaFree(d_kept);
cudaFree(d_rms);

peakOug_richtingng   g_hNstarar;
peakOum_stemmenen    g_hKeptpt;
peakOut.vertrouwen = 1.f / (1.f g_hRmsms);  // simpele confidence
```

}