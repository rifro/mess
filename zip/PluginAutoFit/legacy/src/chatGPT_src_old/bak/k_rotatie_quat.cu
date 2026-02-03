#include "config.h"
#include "types.h"
#include "wiskunde_utils.cuh"
#include <cudaRuntime.h>

__device__ __forceinline__ Vec3f dQuatApply(const Quatf& q, const Vec3f& v)
{
    Vec3f qv = make_vec3f(q.x, q.y, q.z);
    Vec3f t  = mul(cross(qv, v), 2.0f);
    Vec3f vp = v + mul(t, q.w) + mul(cross(qv, t), 1.0f);
    return vp;
}

__global__ void kRotateQuat(const Quatf q, Vec3f* points, const u8* mask, u32 N)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;
    if(mask && mask[i] == 0) return;
    points[i] = dQuatApply(q, points[i]);
}

extern "C" void kRotatieQuatLaunch(const Quatf& qNorm, Vec3f* dPunten, const u8* dMask, u32 N)
{
    if(!dPunten || N == 0) return;
    dim3 bs(256), gs((N + bs.x - 1) / bs.x);
    kRotateQuat<<<gs, bs>>>(qNorm, dPunten, dMask, N);
    cudaDeviceSynchronize();
}