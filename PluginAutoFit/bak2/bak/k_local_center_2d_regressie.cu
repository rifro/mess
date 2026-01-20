#include "includes.cuh"

global void k_localCenter2dRegressieKernel(const float3* __restrict__ points, const u32* __restrict__ clusterindices,
                                               const u32* __restrict__ clusterStarts, u32 numClusters,
                                               float3 axis, float3* __restrict__ centers, float* __restrict__ radii)
{
    u32 clusterId = blockIdx.x;
    if(clusterId >= numClusters) return;

    u32 start  = clusterStarts[clusterId];
    u32 end    = clusterStarts[clusterId + 1];
    u32 numPoints = end - start;

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
    for(u32 i = threadIdx.x; i < numPoints; i += blockDim.x)
    {
        float3 p   = points[clusterindices[start + i]];
        projPoints[i] = make_float2(dot(p, u), dot(p, v));
    }
    __syncthreads();

    // Find median of x and y coordinates
    // use cub for block-wide sort
    typedef cub::blockradixsort<float2, 256, 1, float> blockradixsort;
    shared typename blockradixsort::tempstorage     tempstorage;

    // sort by x
    blockradixsort(tempstorage).sort(projPoints, numPoints);
    __syncthreads();
    float medianx = projPoints[numPoints / 2].x;

    // sort by y
    blockradixsort(tempstorage).sort(projPoints, numPoints, [](const float2& a) { return a.y; });
    __syncthreads();
    float mediany = projPoints[numPoints / 2].y;

    float2 center2d = make_float2(medianx, mediany);

    // calculate radii
    shared float s_radii[];
    for(u32 i = threadIdx.x; i < numPoints; i += blockDim.x)
    {
        float2 p    = projPoints[i];
        float2 diff = make_float2(p.x - center2d.x, p.y - center2d.y);
        s_radii[i]   = sqrtf(dot(diff, diff));
    }
    __syncthreads();

    // Find median radius
    cub::blocksort<float, 256, 1>             blocksort;
    shared typename blocksort::tempstorage tempstorageradii;
    blocksort(tempstorageradii).sort(s_radii, numPoints);
    __syncthreads();
    float medianRadius = s_radii[numPoints / 2];

    // write results
    if(threadIdx.x == 0)
    {
        centers[clusterId] = u * center2d.x + v * center2d.y;
        radii[clusterId]   = medianRadius;
    }
}

void h_pipesteunpunt2d(const DeviceBuffer<float3>& d_points, const DeviceBuffer<u32>& d_clusterIndices,
                       const DeviceBuffer<u32>& d_clusterStarts, u32 numClusters, const float3& axis,
                       DeviceBuffer<float3>& d_centers, DeviceBuffer<float>& d_radii)
{
    const u32 blockSize     = 256;
    const u32 sharedMemSize = blockSize * sizeof(float2) + blockSize * sizeof(float);
    k_localCenter2dRegressieKernel<<<numClusters, blockSize, sharedMemSize>>>(
        d_points.data(), d_clusterIndices.data(), d_clusterStarts.data(), numClusters, axis, d_centers.data(), d_radii.data());
}
