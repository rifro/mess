#pragma once
#include "rdv_types.h"
#include <cuda_runtime.h>

namespace Bocari
{
    // --- CUDA Device Math Helpers ---

    __device__ inline float dot(const Vec3f& a, const Vec3f& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    __device__ inline Vec3f subtract(const Vec3f& a, const Vec3f& b)
    {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    __device__ inline Vec3f add(const Vec3f& a, const Vec3f& b)
    {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    __device__ inline Vec3f scale(const Vec3f& v, float s)
    {
        return {v.x * s, v.y * s, v.z * s};
    }

    __device__ inline Vec3f cross(const Vec3f& a, const Vec3f& b)
    {
        return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }

    __device__ inline Vec3f normalize(const Vec3f& v)
    {
        float invLen = rsqrtf(dot(v, v));
        return scale(v, invLen);
    }

} // namespace Bocari
