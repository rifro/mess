#pragma once
#include "strict.h"

namespace Bocari
{
    template <typename T>
    struct Vec3
    {
        T m_x, m_y, m_z;

        __host__ __device__ Vec3<T> operator+(const Vec3<T>& other) const
        {
            return {m_x + other.m_x, m_y + other.m_y, m_z + other.m_z};
        }

        __host__ __device__ Vec3<T> operator-(const Vec3<T>& other) const
        {
            return {m_x - other.m_x, m_y - other.m_y, m_z - other.m_z};
        }

        __host__ __device__ Vec3<T> operator*(T scalar) const
        {
            return {m_x * scalar, m_y * scalar, m_z * scalar};
        }
    };

    // Define common type aliases
    using Vec3f = Vec3<float>;
    using Vec3d = Vec3<double>;
    using Vec3i = Vec3<int>;

} // namespace Bocari
