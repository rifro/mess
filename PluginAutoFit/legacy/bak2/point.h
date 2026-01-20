#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    namespace Math
    {
        // Template Vec3 class for float/double
        template <typename T> struct Vec3t
        {
            T x, y, z;

            Vec3t() = default;
            Vec3t(T x, T y, T z) : x(x), y(y), z(z) {}
            // Delegating constructors:
            template <typename U>
                requires(!std::isSameV<T, U>)
            Vec3t(U x, U y, U z) : Vec3t(static_cast<T>(x), static_cast<T>(y), static_cast<T>(z))
            {
            }

            // Array-like access
            T& operator[](int index)
            {
                assert(index >= 0 && index < 3);
                return (&x)[index];
            }

            const T& operator[](int index) const
            {
                assert(index >= 0 && index < 3);
                return (&x)[index];
            }

            // Converteer naar ander type
            template <typename U>
            explicit Vec3t(const Vec3t<U>& that)
                : x(static_cast<T>(that.x)), y(static_cast<T>(that.y)), z(static_cast<T>(that.z))
            {
            }

            // Constructor voor CCVector3
            explicit Vec3t(const CCVector3& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z))
            {
            }

            Vec3t& operator+=(const Vec3t& that)
            {
                x += that.x;
                y += that.y;
                z += that.z;
                return *this;
            }

            Vec3t& operator-=(const Vec3t& that)
            {
                x -= that.x;
                y -= that.y;
                z -= that.z;
                return *this;
            }

            Vec3t& operator*=(T s)
            {
                x *= s;
                y *= s;
                z *= s;
                return *this;
            }

            Vec3t& operator/=(double denom)
            {
                Vec3t* pThis = const_cast<T*>(this);
                return pThis->operator*=(1.0 / denom);
            }

            Vec3t operator+(const Vec3t& that) const { return Vec3t(x + that.x, y + that.y, z + that.z); }
            Vec3t operator-(const Vec3t& that) const { return Vec3t(x - that.x, y - that.y, z - that.z); }
            Vec3t operator*(T s) const { return Vec3t(x * s, y * s, z * s); }
            Vec3t operator/(T s) const { return this->operator*(1.0 / s); }

            T     dot(const Vec3t& that) const { return x * that.x + y * that.y + z * that.z; }
            Vec3t cross(const Vec3t& that) const
            {
                return Vec3t(y * that.z - z * that.y, z * that.x - x * that.z, x * that.y - y * that.x);
            }

            T length2() const { return Math::pow2(x) + Math::pow2(y) + Math::pow2(z); }
            T length() const { return std::sqrt(length2()); }

            void normalizeInplace(bool& ok)
            {
                T len = length();
                ok    = len > static_cast<T>(1.0e-12);
                if(!ok) return;
                T inv = static_cast<T>(1.0) / len;
                x *= inv;
                y *= inv;
                z *= inv;
            }

            static Vec3t normalFrom(const Vec3t& a, const Vec3t& b, const Vec3t& c, bool& ok)
            {
                Vec3t v1 = b - a;
                Vec3t v2 = c - a;
                Vec3t n  = v1.cross(v2);
                n.normalizeInplace(ok);
                return n;
            }
        };

        using Vec3f = Math::Vec3t<float>;
        using Vec3d = Math::Vec3t<double>;

        // --- Matrix3d + helpers (compact) ---
        struct Matrix3d
        {
            double m[3][3];

            Matrix3d() { zero(); }

            void zero()
            {
                for(int i = 0; i < 3; i++)
                    for(int j = 0; j < 3; j++) m[i][j] = 0.0;
            }

            static Matrix3d zeroMatrix() { return Matrix3d(); }

            Matrix3d& operator+=(const Matrix3d& that)
            {
                for(int i = 0; i < 3; i++)
                    for(int j = 0; j < 3; j++) this->m[i][j] += that.m[i][j];
                return *this;
            }

            Matrix3d& operator*=(double s)
            {
                for(int i = 0; i < 3; i++)
                    for(int j = 0; j < 3; j++) this->m[i][j] *= s;
                return *this;
            }

            Matrix3d operator+(const Matrix3d& that) const
            {
                Matrix3d result;
                for(int i = 0; i < 3; i++)
                    for(int j = 0; j < 3; j++) result.m[i][j] = this->m[i][j] + that.m[i][j];
                return result;
            }

            Matrix3d operator*(double s) const
            {
                Matrix3d result;
                for(int i = 0; i < 3; i++)
                    for(int j = 0; j < 3; j++) result.m[i][j] = this->m[i][j] * s;
                return result;
            }

            Matrix3d operator/(double denom) const { return (*this) * (1.0 / denom); }
        };

        inline Matrix3d outerProduct(const Math::Vec3d& a, const Math::Vec3d& b)
        {
            Matrix3d result;
            for(int i = 0; i < 3; i++)
                for(int j = 0; j < 3; j++) result.m[i][j] = a[i] * b[j];
            return result;
        }

        template <typename T> struct Quaternion
        {
            T w, x, y, z;

            Quaternion() = default;

            // Constructor voor w, x, y, z
            Quaternion(T wIn, T xIn, T yIn, T zIn) : w(wIn), x(xIn), y(yIn), z(zIn) {}

            // Generieke constructor voor een vector
            template <typename V> explicit Quaternion(const V& v) : w(0), x(v.x), y(v.y), z(v.z) {}

            // Geconjugeerde
            Quaternion conj() const { return {w, -x, -y, -z}; }

            // Magnitude (lengte) //RR!!! make normSquared, use here and in inverse
            T normSquared() const { return Math::pow2(w) + Math::pow2(x) + Math::pow2(y) + Math::pow2(z); }
            T norm() const { return std::sqrt(normSquared()); }

            // Normaliseer
            void normalize()
            {
                T norm = norm();
                if(norm > std::numericLimits<T>::epsilon())
                {
                    T invMag = static_cast<T>(1.0) / norm;
                    w *= invMag;
                    x *= invMag;
                    y *= invMag;
                    z *= invMag;
                }
            }

            // Inverse - compute on demand
            Quaternion inverse() const
            {
                T normSquared = normSquared();
                if(normSquared == 0) return {1, 0, 0, 0}; // identity as fallback
                return Quaternion(w / normSquared, -x / normSquared, -y / normSquared, -z / normSquared);
            }

            // Quaternion vermenigvuldiging
            Quaternion operator*(const Quaternion& that) const
            {
                return Quaternion(w * that.w - x * that.x - y * that.y - z * that.z,
                                  w * that.x + x * that.w + y * that.z - z * that.y,
                                  w * that.y - x * that.z + y * that.w + z * that.x,
                                  w * that.z + x * that.y - y * that.x + z * that.w);
            }

            // Roteer een Pointcloud::Point (in-place versie)
            void rotateInPlace(Pointcloud::Point& p) const;

            // Roteer een span van Points in-place
            void rotateSpanInPlace(std::span<Pointcloud::Point> points) const;
        };

        // Convert rotation matrix R (orthonormal, right-handed) to quaternion.
        // Uses robust algorithm and normalizes at the end.
        static Quaternion<double> fromMatrix(const Matrix3d& R)
        {
            double trace = R.m[0][0] + R.m[1][1] + R.m[2][2];
            double qw, qx, qy, qz;

            if(trace > 0.0)
            {
                double s = 0.5 / std::sqrt(trace + 1.0);
                qw       = 0.25 / s;
                qx       = (R.m[2][1] - R.m[1][2]) * s;
                qy       = (R.m[0][2] - R.m[2][0]) * s;
                qz       = (R.m[1][0] - R.m[0][1]) * s;
            } else
            {
                if(R.m[0][0] > R.m[1][1] && R.m[0][0] > R.m[2][2])
                {
                    double s = std::sqrt(1.0 + R.m[0][0] - R.m[1][1] - R.m[2][2]) * 2.0; // s = 4*qx
                    qw       = (R.m[2][1] - R.m[1][2]) / s;
                    qx       = 0.25 * s;
                    qy       = (R.m[0][1] + R.m[1][0]) / s;
                    qz       = (R.m[0][2] + R.m[2][0]) / s;
                } else if(R.m[1][1] > R.m[2][2])
                {
                    double s = std::sqrt(1.0 + R.m[1][1] - R.m[0][0] - R.m[2][2]) * 2.0; // s = 4*qy
                    qw       = (R.m[0][2] - R.m[2][0]) / s;
                    qx       = (R.m[0][1] + R.m[1][0]) / s;
                    qy       = 0.25 * s;
                    qz       = (R.m[1][2] + R.m[2][1]) / s;
                } else
                {
                    double s = std::sqrt(1.0 + R.m[2][2] - R.m[0][0] - R.m[1][1]) * 2.0; // s = 4*qz
                    qw       = (R.m[1][0] - R.m[0][1]) / s;
                    qx       = (R.m[0][2] + R.m[2][0]) / s;
                    qy       = (R.m[1][2] + R.m[2][1]) / s;
                    qz       = 0.25 * s;
                }
            }

            Quaternion q(qw, qx, qy, qz);
            // Normalize quaternion
            double len = std::sqrt(Math::pow2(q.w) + Math::pow2(q.x) + Math::pow2(q.y) + Math::pow2(q.z));
            if(len > 0.0)
            {
                q.w /= len;
                q.x /= len;
                q.y /= len;
                q.z /= len;
            } else
            {
                // fallback identity
                q.w = 1.0;
                q.x = q.y = q.z = 0.0;
            }
            return q;
        }

    } // namespace Math

    namespace Pointcloud
    {

        struct Point
        {
            Point() = default;
            Point(int32_t ccIndex, float x, float y, float z) : m_ccIndex(ccIndex), m_point(x, y, z) {}
            Point(int32_t ccIndex, const Math::Vec3f& vec) : m_ccIndex(ccIndex), m_point(vec) {}
            int32_t     m_ccIndex = -1;
            Math::Vec3f m_point;
            void        dbg() const
            {
                std::cout << "point=(" << m_point.x << ", " << m_point.y << ", " << m_point.z
                          << "), ccIndex = " << m_ccIndex << std::endl;
            }
            // Array-access voor de coordinaten: x->[0], y->[1], z->[2]
            float& operator[](int index)
            {
                assert(index >= 0 && index < 3);
                return (&m_point.x)[index];
            }

            const float& operator[](int index) const
            {
                assert(index >= 0 && index < 3);
                return (&m_point.x)[index];
            }
        };

        ccPointCloud* toCcPointCloud(std::span<const Pointcloud::Point> points);
        ccPointCloud* createRotatedPointCloud(const Math::Quaternion<double>& quat, const ccPointCloud* original);

    } // namespace Pointcloud
} // namespace Bocari