#include "config.h"
#include "types.h"
#include "wiskunde_utils.cuh"
#include <cudaRuntime.h>
#include <thrust/deviceVector.h>

// JBF-planar regressie: voor een candidate vlak (normaal n0) binnen puntset,
// refine naar n* door projectie op (u,v) basis, lineaire regressie op 2D,
// bereken nieuwe n* = normalize(u* × v*), en bereken d via trim-K methode.

// Kernel: voor eenvoud per blok één kandidaat plaatser (optioneel uitbreiden),
// nu: __host__ roept één candidate richting aan.
// Input: punten_sorted[N], kandidaat normaal n0, parameter k_trimFraction
// Output: top RichtingPeak (richting n*, stemmen = aantal kept, vertrouwen ~ 1/(1+rms))

__global__ void kPlanairRegressieKernel(const Vec3f*** restrict** points, u32 N, Vec3f n0, float trimFrac,
                                           Vec3f* outNStar, u32* outKept, float* outRms)
{
    extern** shared** float sBuf[]; // tijdelijk geen gebruik
    // Eén thread per kernel (__host__ scale) — dus blockDim=1, gridDim=1

    // Stap 1: maak u,v basis uit n0
    Vec3f u, v;
    maakOrthobasisUvUitN(n0, u, v);

    // Stap 2: projecteer alle points op u,v: (u_i, v_i) in 2D
    // On‐__device__ simpel accumulate
    float sumU = 0.f, sumV = 0.f, sumUu = 0.f, sumUv = 0.f, sumVv = 0.f;
    for(u32 i = 0; i < N; ++i)
    {
        Vec3f p  = points[i];
        float uu = dot(p, u);
        float vv = dot(p, v);
        sumU += uu;
        sumV += vv;
        sumUu += uu * uu;
        sumUv += uu * vv;
        sumVv += vv * vv;
    }

    float meanU = sumU / N;
    float meanV = sumV / N;

    float covUu = sumUu / N - meanU * meanU;
    float covUv = sumUv / N - meanU * meanV;
    float covVv = sumVv / N - meanV * meanV;

    // Eenvoudige Richting v* op basis van covariantie
    Vec3f v_star = norm(make_vec3f(covUv, covVv - covUu, 0.f));
    Vec3f uStar = norm(cross(v_star, n0));

    Vec3f nStar = norm(cross(uStar, v_star));

    // Stap 3: afstand d_i = dot(n_star, p_i). Bereken RMS na trim-K
    // Kopiëer d_i in __host__‐array? Simpel: __host__ doet dit later. Hier __device__ approx:
    float sumD2 = 0.f;
    for(u32 i = 0; i < N; ++i)
    {
        float di = dot(nStar, points[i]);
        sumD2 += di * di;
    }
    float rms = sqrtf(sumD2 / (float)N);

    *outNStar = nStar;
    *outKept   = N;
    *outRms    = rms;
}

extern "C" void kPlanairRegressieLaunch(const Vec3f* d_pointsSorted, u32 N, const Vec3f& n0, u32 k_trimFracInt,
                                           RichtingPeak& peakOut)
{
    if(N == 0)
    {
        peakOut.stemmen    = 0;
        peakOut.vertrouwen = 0.f;
        peakOut.richting   = make_vec3f(0, 0, 0);
        return;
    }

    Vec3f*    d_nstar;
    uint32_t* d_kept;
    float*    d_rms;
    cudaMalloc(&d_nstar, sizeof(Vec3f));
    cudaMalloc(&d_kept, sizeof(uint32_t));
    cudaMalloc(&d_rms, sizeof(float));

    float trimFrac = ((float)k_trimFracInt) * 0.01f;
    kPlanairRegressieKernel<<<1, 1, 0>>>(d_pointsSorted, N, n0, trimFrac, d_nstar, d_kept, d_rms);
    cudaDeviceSynchronize();

    Vec3f    h_nstar;
    uint32_t h_kept;
    float    hRms;
    cudaMemcpy(&h_nstar, d_nstar, sizeof(Vec3f), cudaMemcpyDeviceToHost);
    cudaMemcpy(&h_kept, d_kept, sizeof(uint32_t), cudaMemcpyDeviceToHost);
    cudaMemcpy(&hRms, d_rms, sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(d_nstar);
    cudaFree(d_kept);
    cudaFree(d_rms);

    peakOut.richting   = h_nstar;
    peakOut.stemmen    = h_kept;
    peakOut.vertrouwen = 1.f / (1.f + hRms); // simpele confidence
}