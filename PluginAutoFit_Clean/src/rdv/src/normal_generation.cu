#include "rdv/include/normal_generation.cuh"
#include "rdv/include/config.h"
#include "rdv/include/rdv_types.h"
#include "rdv/include/cuda_utils.cuh"
#include <cuda_runtime.h>

// Forward declaration of the kernel
namespace Bocari
{
namespace detail
{
    __global__ void k_generateNormals(
        const float* __restrict__ d_pointsX,
        const float* __restrict__ d_pointsY,
        const float* __restrict__ d_pointsZ,
        u32 pointCount,
        u8* __restrict__ d_pointLabels,
        Vec3f* __restrict__ d_normals,
        u32* __restrict__ d_normalsCount,
        u32 maxNormals
    );
}
}

// Host function implementation
void Bocari::k_generateNormals(
    const DeviceBuffer<float>& d_pointsX,
    const DeviceBuffer<float>& d_pointsY,
    const DeviceBuffer<float>& d_pointsZ,
    DeviceBuffer<u8>& d_pointLabels,
    DeviceBuffer<Vec3f>& d_normals,
    DeviceBuffer<u32>& d_normalsCount
)
{
    const u32 pointCount = d_pointsX.size();
    if (pointCount == 0) return;

    const u32 maxNormals = d_normals.size();
    d_normalsCount.memset(0);

    const u32 blockSize = 256;
    const u32 gridSize = (pointCount + blockSize - 1) / blockSize;

    detail::k_generateNormals<<<gridSize, blockSize>>>(
        d_pointsX.data(),
        d_pointsY.data(),
        d_pointsZ.data(),
        pointCount,
        d_pointLabels.data(),
        d_normals.data(),
        d_normalsCount.data(),
        maxNormals
    );
}


// --- Kernel Implementation ---

namespace Bocari
{
    // Device helper to create a Vec3f from individual pointers and index
    __device__ inline Vec3f getPoint(const float* x, const float* y, const float* z, u32 index)
    {
        return {x[index], y[index], z[index]};
    }

    // Cleaned-up collinearity check
    __device__ bool isCollinear(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3)
    {
        Vec3f side1 = subtract(p2, p1);
        Vec3f side2 = subtract(p3, p1);
        Vec3f crossProd = cross(side1, side2);
        return dot(crossProd, crossProd) < getConfig().m_ringFilter.m_minAreaSq;
    }

    // Cleaned-up planarity test
    __device__ bool passesPlanarityTest(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const Vec3f& pTest, Vec3f& outNormal)
    {
        if (isCollinear(p1, p2, p3)) return false;

        Vec3f normal = normalize(cross(subtract(p2, p1), subtract(p3, p1)));
        float d = dot(normal, p1);
        float distanceSq = powf(dot(normal, pTest) - d, 2);

        bool success = distanceSq < getConfig().m_ringFilter.m_epsilonSq;
        if (success)
        {
            outNormal = normal;
        }
        return success;
    }


    __global__ void detail::k_generateNormals(
        const float* __restrict__ d_pointsX,
        const float* __restrict__ d_pointsY,
        const float* __restrict__ d_pointsZ,
        u32 pointCount,
        u8* __restrict__ d_pointLabels,
        Vec3f* __restrict__ d_normals,
        u32* __restrict__ d_normalsCount,
        u32 maxNormals
    )
    {
        const u32 pointIndex = blockIdx.x * blockDim.x + threadIdx.x;
        if (pointIndex >= pointCount) return;

        Vec3f pCenter = getPoint(d_pointsX, d_pointsY, d_pointsZ, pointIndex);
        Vec3f ringPoints[4];
        int ringCount = 0;
        int farPointCount = 0;

        // Search for neighbors in a ring around the center point
        for (u32 j = pointIndex + 1; j < pointCount; ++j)
        {
            if (farPointCount >= 2) break;

            Vec3f pNeighbor = getPoint(d_pointsX, d_pointsY, d_pointsZ, j);
            Vec3f diff = subtract(pNeighbor, pCenter);
            float distSq = dot(diff, diff);

            if (distSq <= getConfig().m_ringFilter.m_maxRadiusSq)
            {
                if (distSq > getConfig().m_ringFilter.m_minRadiusSq)
                {
                    if (ringCount < 4)
                    {
                        ringPoints[ringCount++] = pNeighbor;
                    }
                }
            }
            else
            {
                farPointCount++;
            }
        }

        if (ringCount < 3)
        {
            d_pointLabels[pointIndex] = PointType::Chaos;
            return;
        }

        Vec3f finalNormal;
        Vec3f p1 = ringPoints[0], p2 = ringPoints[1], p3 = ringPoints[2];

        // Double planarity test
        if (passesPlanarityTest(pCenter, p1, p2, p3, finalNormal) && passesPlanarityTest(p1, p2, p3, pCenter, finalNormal))
        {
            u32 index = atomicAdd(d_normalsCount, 1);
            if (index < maxNormals)
            {
                d_normals[index] = finalNormal;
            }
        }
        else
        {
            d_pointLabels[pointIndex] = PointType::Chaos;
        }
    }

} // namespace Bocari
