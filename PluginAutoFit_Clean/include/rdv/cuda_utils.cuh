#pragma once
#include "strict.h"
#include "rdv_types.h"
#include <cuda_runtime.h>

namespace Bocari
{
    // --- CUDA Device Math Helpers ---

    __device__ inline float dot(const Vec3f& a, const Vec3f& b)
    {
        return a.m_x * b.m_x + a.m_y * b.m_y + a.m_z * b.m_z;
    }

    __device__ inline Vec3f cross(const Vec3f& a, const Vec3f& b)
    {
        return {a.m_y * b.m_z - a.m_z * b.m_y, a.m_z * b.m_x - a.m_x * b.m_z, a.m_x * b.m_y - a.m_y * b.m_x};
    }

    __device__ inline Vec3f normalize(const Vec3f& v)
    {
        return v * rsqrtf(dot(v, v));
    }

} // namespace Bocari
