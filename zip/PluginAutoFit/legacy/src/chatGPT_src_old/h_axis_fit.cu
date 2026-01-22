#include "pairs_bucket.cuh"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// small helper
static inline void cudaCheck(cudaErrorT e, const char* msg)
{
    if(e != cudaSuccess)
    {
        std::cerr << "CUDA fout: " << msg << ": " << cudaGetErrorString(e) << "\n";
        std::abort();
    }
}
static inline float3 make3(float x, float y, float z) { return make_float3(x, y, z); }

void runOtndcPairs(const float* x, const float* y, const float* z, int N, AxisResults* out, const OTConfig& cfg)
{
    // Globale accumulator
    OTAsAccu* d_accu = nullptr;
    cudaCheck(cudaMalloc(&d_accu, sizeof(OTAsAccu)), "malloc dAccu");
    cudaCheck(cudaMemset(d_accu, 0, sizeof(OTAsAccu)), "memset dAccu");

    // Kernelconfig
    const int    B     = (cfg.blockSize > 0 ? cfg.blockSize : 256);
    const int    G     = (N + B - 1) / B;
    const size_t shmem = 3 * B * sizeof(float); // sx,sy,sz

    // Launch
    kOtndcPairs<<<G, B, shmem>>>(x, y, z, N, cfg.innerRadiusSq, cfg.outerRadiusSq, cfg.maxDotOrtho,
                                   cfg.partnerZoekRadius, d_accu);
    cudaCheck(cudaGetLastError(), "launch k_otndc_pairs");
    cudaCheck(cudaDeviceSynchronize(), "sync k_otndc_pairs");

    // __host__ reduce
    OTAsAccu hAccu{};
    cudaCheck(cudaMemcpy(&hAccu, d_accu, sizeof(OTAsAccu), cudaMemcpyDeviceToHost), "cpy accu");
    cudaCheck(cudaFree(d_accu), "free accu");

    // Normeer richting
    float len = std::sqrt(hAccu.som.x * hAccu.som.x + hAccu.som.y * hAccu.som.y + hAccu.som.z * hAccu.som.z);
    int   dim = 0;
    if(hAccu.count >= cfg.warmupMin && len > 1e-8f)
    {
        out->v[0][0]  = hAccu.som.x / len;
        out->v[0][1]  = hAccu.som.y / len;
        out->v[0][2]  = hAccu.som.z / len;
        out->score[0] = static_cast<float>(hAccu.count);
        dim           = 1;
    }
    out->dimensie = dim;

    // (Slots 2 en 3 blijven leeg in deze fase)
    for(int i = 1; i < 3; i++)
    {
        out->v[i][0] = out->v[i][1] = out->v[i][2] = 0.f;
        out->score[i]                              = 0.f;
    }
}
