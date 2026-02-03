#include "includes.cuh"
#include "rdv_types.h"

namespace Bocari
{
    /**
     * @brief Performs a planarity test to see if a test point lies on the plane defined by three other points.
     * @details This function checks for collinearity among the plane-defining points p0, p1, and p2 using the squared area 
     * of the spanning triangle. If valid, it normalizes the normal vector and calculates and checks 
     * the orthogonal distance of pTest to the plane.
     * @param p0, p1, p2 Points defining the reference plane.
     * @param pTest Point to test against the plane.
     * @param outNormal Resulting normalized vector if planarity is met.
     * @return true if pTest is coplanar within the configured linear planarEpsilon.
     */
    __device__ bool passesPlanarityTest(
        const Vec3f& p0, 
        const Vec3f& p1, 
        const Vec3f& p2, 
        const Vec3f& pTest, 
        Vec3f& outNormal)
{
        // 1. Calculate the unnormalized normal (cross product of two edges)
        const Vec3f crossVec = cross(p1 - p0, p2 - p0);
        const float areaSq = dot(crossVec, crossVec);

        // Check for collinearity: if areaSq is near zero, p0, p1, p2 are on a line, using the already calculated areaSq
        if (areaSq < d_config.ringFilter.minAreaSq) return false;

        // 2. Efficient normalization using fast inverse square root (SFU intrinsic)
        // We multiply the raw cross product by 1/sqrt(areaSq)
        const Vec3f unitNormal = crossVec * rsqrtf(areaSq);
        
        // 3. Distance check: the dot product of the unit normal and a vector from 
        // the plane to pTest gives the perpendicular (linear) distance.
        const float distance = abs(dot(pTest - p0, unitNormal));
        
        if (distance < d_config.ringFilter.planarEpsilon)
        {
            outNormal = unitNormal;
            return true;
        }
        return false;
    }

    /**
     * @brief Retrieves a 3D point from the SoA buffers and converts mm (i32) to float.
     */
    __device__ inline Vec3f getPoint(
        const i32* __restrict__ x, 
        const i32* __restrict__ y, 
        const i32* __restrict__ z, 
        u32 index)
    {
        return { static_cast<float>(x[index]), static_cast<float>(y[index]), static_cast<float>(z[index]) };
    }

    /**
     * @brief CUDA kernel for generating surface normals using a ring-neighborhood search.
     * @details This kernel operates on Morton-ordered points to ensure spatial locality.
     * It performs a neighborhood search to find 3 points within a specific ring [inner, outer].
     * Points closer than innerRadius are marked as Duplicates.
     * @param d_points Pointer to the 20-byte packed point structures.
     * @param d_sortedIndices Indices sorted by Morton code.
     * @param pointCount Total number of points in the current page.
     * @param d_normals Output buffer for computed Vec3f normals.
     * @param d_normalsCount Atomic counter for valid normals found.
     * @param maxNormals Capacity of the d_normals buffer.
     */
    __global__ void k_generateNormalsKernel(
        const i32* __restrict__ d_x,
        const i32* __restrict__ d_y,
        const i32* __restrict__ d_z,
        u8* __restrict__ d_types,
        const u32* __restrict__ d_sortedIndices,
        const u32 pointCount,
        Vec3f* __restrict__ d_normals,
        u32* __restrict__ d_normalsCount,
        const u32 maxNormals)
    {
        const u32 idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx >= pointCount) return;

        const u32 p0_idx = d_sortedIndices[idx];
        // Skip already processed duplicates
        if (d_types[p0_idx] == PointType::Duplicate) return;

        const Vec3f pCenter = getPoint(d_x, d_y, d_z, p0_idx);
        Vec3f ringPoints[3];
        u32 ringCount = 0;
        u32 noiseCounter = 0;

        // Window search logic: Jump > Outer > Inner
        for (int i = 1; i < 64 && ringCount < 3; ++i)
        {
            if (idx + i >= pointCount) break;
            
            const u32 neighborIdx = d_sortedIndices[idx + i];
            const Vec3f neighborPos = getPoint(d_x, d_y, d_z, neighborIdx);
            
            const float distanceSq = dot(neighborPos - pCenter, neighborPos - pCenter);
            
            // 1. Hard Exit: Outside Morton search window
            if (distanceSq > d_config.ringFilter.mortonJumpSq) break;

            // 2. Candidate Selection: Within the ring's outer limit
            if (distanceSq < d_config.ringFilter.outerRadiusSq) 
            {
                // 3. Duplicate Identification: Within inner radius
                if (distanceSq < d_config.ringFilter.innerRadiusSq) {
                    d_types[neighborIdx] = PointType::Duplicate;
                } else {
                    // Valid point within the [inner, outer] ring
                    ringPoints[ringCount++] = neighborPos;
                }
                // Reset noise counter because we found a relevant point
                noiseCounter = 0; 
            } 
            else {
                // Point is between outerRadius and mortonJump (Sparse/Noise area)
                if (++noiseCounter > 8) break; 
            }
        }

        // Final Classification with Early Exit
        if (ringCount < 3)
        {
            d_types[p0_idx] = PointType::Chaos;
            return;
        }

        Vec3f finalNormal;
        const Vec3f p1 = ringPoints[0];
        const Vec3f p2 = ringPoints[1];
        const Vec3f p3 = ringPoints[2];

        // Double planarity test ensures consistent local geometry
        if (passesPlanarityTest(pCenter, p1, p2, p3, finalNormal) && 
            passesPlanarityTest(p1, p2, p3, pCenter, finalNormal))
        {
            const u32 outIdx = atomicAdd(d_normalsCount, 1);
            if (outIdx < maxNormals)
            {
                d_normals[outIdx] = finalNormal;
            }
        }
        else
        {
            d_types[p0_idx] = PointType::Chaos;
        }
    }

    /**
     * @brief Launches the CUDA kernel to generate surface normals for a point cloud.
     * @details This host dispatcher prepares and launches the normal generation kernel 
     * using a Structure-of-Arrays (SoA) format.
     * It resets the atomic normal counter on the device before execution.
     *
     * Axis swapping is supported by changing the order of the d_x, d_y, and d_z 
     * pointers during the call.
     *
     * @param d_x Device buffer with X coordinates (i32 mm).
     * @param d_y Device buffer with Y coordinates (i32 mm).
     * @param d_z Device buffer with Z coordinates (i32 mm).
     * @param d_types Device buffer for point classification (e.g., Chaos, Duplicate).
     * @param d_sortedIndices Morton-sorted indices for spatial locality.
     * @param pointCount Total number of points to process.
     * @param d_normals Output device buffer for generated unit normals.
     * @param d_normalsCount Atomic counter on the device for the number of generated normals.
     * @param maxNormals Maximum capacity of the d_normals buffer.
     */
    void h_generateNormals(
        const i32* d_x, 
        const i32* d_y, 
        const i32* d_z,
        u8* d_types,
        const u32* d_sortedIndices,
        u32 pointCount,
        Vec3f* d_normals,
        u32* d_normalsCount,
        u32 maxNormals)
    {
        if (pointCount == 0) return;

        // Reset atomic counter for the new batch
        cudaMemset(d_normalsCount, 0, sizeof(u32));

        const u32 blockSize = 256;
        const u32 gridSize = (pointCount + blockSize - 1) / blockSize;

        k_generateNormalsKernel<<<gridSize, blockSize>>>(
            d_x, d_y, d_z, 
            d_types, 
            d_sortedIndices, 
            pointCount, 
            d_normals, 
            d_normalsCount, 
            maxNormals
        );
    }
} // namespace Bocari
