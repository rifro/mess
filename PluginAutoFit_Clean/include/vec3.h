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

        __host__ __device__ Vec3<T>& operator+=(const Vec3<T>& other)
        {
            m_x += other.m_x;
            m_y += other.m_y;
            m_z += other.m_z;
            return *this;
        }

        __host__ __device__ Vec3<T>& operator-=(const Vec3<T>& other)
        {
            m_x -= other.m_x;
            m_y -= other.m_y;
            m_z -= other.m_z;
            return *this;
        }
    };

    template <typename T>
    __host__ __device__ inline T dot(const Vec3<T>& a, const Vec3<T>& b)
    {
        return a.m_x * b.m_x + a.m_y * b.m_y + a.m_z * b.m_z;
    }

    template <typename T>
    __host__ __device__ inline Vec3<T> cross(const Vec3<T>& a, const Vec3<T>& b)
    {
        return {
            a.m_y * b.m_z - a.m_z * b.m_y,
            a.m_z * b.m_x - a.m_x * b.m_z,
            a.m_x * b.m_y - a.m_y * b.m_x};
    }

    template <typename T>
    __host__ __device__ inline Vec3<T> normalize(const Vec3<T>& v)
    {
        T lenSq = dot(v, v);
        if (lenSq > 1e-9) // Epsilon to avoid division by zero
        {
            T len = sqrt(lenSq);
            return v * (1.0 / len);
        }
        return v; // Return original vector if it's zero
    }


    // Define common type aliases
    using Vec3f = Vec3<float>;
    using Vec3d = Vec3<double>;
    using Vec3i = Vec3<int>;

} // namespace Bocari
