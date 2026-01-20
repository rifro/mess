#include "k_localcenter2dregressie.cuh"
#include "math_utils.cuh"
#include <cub/cub.cuh>
#include <cuda_runtime.h>

global void k_localCenter2dRegressieKernel(const float3* __restrict__ points, const uint32_t* __restrict__ clusterIndices,
                                               const uint32_t* __restrict__ clusterStarts, uint32_t numClusters,
                                               float3 axis, float3* __restrict__ centers, float* __restrict__ radii)
{
    uint32_t clusterId = blockIdx.x;
    if(clusterId >= numClusters) return;

    uint32_t start  = clusterStarts[clusterId];
    uint32_t end    = clusterStarts[clusterId + 1];
    uint32_t numPoints = end - start;

    if(numPoints < 3) return;

    // Project points onto 2D plane
    float3 u, v;
    // create a coordinate system from the axis
    if(abs(axis.x) > 0.9f)
    {
        u = normalize(cross(axis, make_float3(0, 1, 0)));
    } else
    {
        u = normalize(cross(axis, make_float3(1, 0, 0)));
    }
    v = cross(axis, u);

    extern shared float2 projPoints[];
    for(uint32_t i = threadIdx.x; i < numPoints; i += blockDim.x)
    {
        float3 p   = points[clusterIndices[start + i]];
        projPoints[i] = make_float2(dot(p, u), dot(p, v));
    }
    __syncthreads();

    // Find median of x and y coordinates
    // use cub for block-wide sort
    typedef cub::blockradixsort<float2, 256, 1, float> blockradixsort;
    shared typename blockradixsort::tempStorage     tempStorage;

    // sort by x
    blockradixsort(tempStorage).sort(projPoints, numPoints);
    __syncthreads();
    float medianX = projPoints[numPoints / 2].x;

    // sort by y
    blockradixsort(tempStorage).sort(projPoints, numPoints, [](const float2& a) { return a.y; });
    __syncthreads();
    float medianY = projPoints[numPoints / 2].y;

    float2 center2d = make_float2(medianX, medianY);

    // calculate radii
    shared float s_radii[];
    for(uint32_t i = threadIdx.x; i < numPoints; i += blockDim.x)
    {
        float2 p    = projPoints[i];
        float2 diff = make_float2(p.x - center2d.x, p.y - center2d.y);
        s_radii[i]   = sqrtf(dot(diff, diff));
    }
    __syncthreads();

    // Find median radius
    cub::blocksort<float, 256, 1>             blocksort;
    shared typename blocksort::tempStorage tempStorageRadii;
    blocksort(tempStorageRadii).sort(s_radii, numPoints);
    __syncthreads();
    float medianRadius = s_radii[numPoints / 2];

    // write results
    if(threadIdx.x == 0)
    {
        centers[clusterId] = u * center2d.x + v * center2d.y;
        radii[clusterId]   = medianRadius;
    }
}

void h_pipesteunpunt2d(const DeviceBuffer<float3>& d_points, const DeviceBuffer<uint32_t>& d_clusterIndices,
                       const DeviceBuffer<uint32_t>& d_clusterStarts, uint32_t numClusters, const float3& axis,
                       DeviceBuffer<float3>& d_centers, DeviceBuffer<float>& d_radii)
{
    const uint32_t blockSize     = 256;
    const uint32_t sharedMemSize = blockSize * sizeof(float2) + blockSize * sizeof(float);
    k_localCenter2dRegressieKernel<<<numClusters, blockSize, sharedMemSize>>>(
        d_points.data(), d_clusterIndices.data(), d_clusterStarts.data(), numClusters, axis, d_centers.data(), d_radii.data());
}
