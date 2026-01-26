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

    /**
     * @brief Retrieves a 3D point from the Structure-of-Arrays (SoA) buffers.
     */
    __device__ inline Vec3f getPoint(const float* __restrict__ x, const float* __restrict__ y, const float* __restrict__ z, u32 index)
    {
        return {x[index], y[index], z[index]};
    }

    /**
     * @brief Checks if three points are collinear (lie on the same line).
     * @details This is determined by checking if the area of the parallelogram formed by the
     * vectors (p2-p1) and (p3-p1) is close to zero. The squared magnitude of the cross
     * product is used to avoid a costly square root operation.
     * @return `true` if the points are collinear, `false` otherwise.
     */
    __device__ bool isCollinear(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3)
    {
        Vec3f side1 = p2 - p1;
        Vec3f side2 = p3 - p1;
        Vec3f crossProd = cross(side1, side2);
        return dot(crossProd, crossProd) < d_config.ringFilter.minAreaSq;
    }

    /**
     * @brief Performs a planarity test to see if a test point lies on the plane defined by three other points.
     * @details This function first checks for collinearity among the plane-defining points.
     * It then calculates the plane's normal and checks if the squared distance from the
     * `pTest` point to this plane is within a small tolerance (`epsilonSq`).
     * If the test passes, the calculated normal is returned via `outNormal`.
     * @return `true` if the test point is on the plane, `false` otherwise.
     */
    __device__ bool passesPlanarityTest(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const Vec3f& pTest, Vec3f& outNormal)
    {
        if (isCollinear(p1, p2, p3)) return false;

        Vec3f normal = normalize(cross(p2 - p1, p3 - p1));
        float d = dot(normal, p1);
        float distanceSq = powf(dot(normal, pTest) - d, 2);

        bool success = distanceSq < d_config.ringFilter.epsilonSq;
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

    /**
     * @brief CUDA kernel to generate surface normals from a point cloud.
     * @details Each thread processes a single point (`pCenter`). It searches for neighbors within
     * a ring defined by a min and max radius. This avoids numerical instability from points
     * that are too close. It uses a "double planarity" test: a valid normal is found only if
     * a test point `p3` lies on the plane `(pCenter, p1, p2)` AND `pCenter` also lies on
     * the plane `(p1, p2, p3)`. This robustly confirms a local planar structure.
     * If a valid normal is found, it is atomically added to the output buffer.
     */
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

        // This simplified search is for demonstration. A real implementation
        // would use a more efficient spatial data structure (e.g., Morton-ordered search).
        for (u32 j = pointIndex + 1; j < pointCount; ++j)
        {
            if (farPointCount >= 2) break;

            Vec3f pNeighbor = getPoint(d_pointsX, d_pointsY, d_pointsZ, j);
            Vec3f diff = pNeighbor - pCenter;
            float distSq = dot(diff, diff);

            if (distSq <= d_config.ringFilter.maxRadiusSq)
            {
                if (distSq > d_config.ringFilter.minRadiusSq)
                {
                    if (ringCount < 4)
                    {
                        ringPoints[ringCount++] = pNeighbor;
                    }
                }
            }
            else
            {
                // Early-out optimization: if too many points are far away, this region is likely noise.
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

        // The double planarity test ensures the local geometry is consistent.
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
