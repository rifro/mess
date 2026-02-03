#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    // Forward declaration for the CUDA kernel
    __global__ void k_generateNormalsKernel(
        const float* __restrict__ d_pointsX,
        const float* __restrict__ d_pointsY,
        const float* __restrict__ d_pointsZ,
        u32 pointCount,
        u8* __restrict__ d_pointLabels,
        Vec3f* __restrict__ d_normals,
        u32* __restrict__ d_normalsCount,
        u32 maxNormals);

    __device__ inline Vec3f getPoint(const float* __restrict__ x, const float* __restrict__ y, const float* __restrict__ z, u32 index)
    {
        return {x[index], y[index], z[index]};
    }

    __device__ bool isCollinear(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3)
    {
        Vec3f side1 = p2 - p1;
        Vec3f side2 = p3 - p1;
        Vec3f crossProd = cross(side1, side2);
        return dot(crossProd, crossProd) < d_config.ringFilter.minAreaSq;
    }

    __device__ bool passesPlanarityTest(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const Vec3f& pTest, Vec3f& outNormal)
    {
        if (isCollinear(p1, p2, p3)) return false;

        Vec3f normal = normalize(cross(p2 - p1, p3 - p1));
        float d = dot(normal, p1);
        float distanceSq = pow2(dot(normal, pTest) - d);

        bool success = distanceSq < d_config.ringFilter.planarEpsilonSq;
        if (success)
        {
            outNormal = normal;
        }
        return success;
    }

    void h_generateNormals(
        const DeviceBuffer<float>& d_pointsX,
        const DeviceBuffer<float>& d_pointsY,
        const DeviceBuffer<float>& d_pointsZ,
        u32 pointCount,
        DeviceBuffer<u8>& d_pointLabels,
        DeviceBuffer<Vec3f>& d_normals,
        DeviceBuffer<u32>& d_normalsCount
    )
    {
        if (pointCount == 0) return;

        d_normalsCount.memset(0);
        const u32 maxNormals = d_normals.size();

        const u32 blockSize = 256;
        const u32 gridSize = (pointCount + blockSize - 1) / blockSize;

        k_generateNormalsKernel<<<gridSize, blockSize>>>(
            d_pointsX.data(), d_pointsY.data(), d_pointsZ.data(),
            pointCount, d_pointLabels.data(), d_normals.data(),
            d_normalsCount.data(), maxNormals
        );
    }

    __global__ void k_generateNormalsKernel(
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

        for (u32 j = pointIndex + 1; j < pointCount; ++j)
        {
            if (farPointCount >= 2) break;

            Vec3f pNeighbor = getPoint(d_pointsX, d_pointsY, d_pointsZ, j);
            Vec3f diff = pNeighbor - pCenter;
            float distSq = dot(diff, diff);

            if (distSq <= d_config.ringFilter.outerRadiusSq)
            {
                if (distSq > d_config.ringFilter.innerRadiusSq)
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

        d_pointLabels[pointIndex] = PointType::Surface;
        Vec3f finalNormal;
        Vec3f p1 = ringPoints[0], p2 = ringPoints[1], p3 = ringPoints[2];

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
