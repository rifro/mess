#include "k_localcenter2dregressie.cuh"
#include "math_utils.cuh"
#include <cub/cub.cuh>
#include <cuda_runtime.h>

global void k_localcenter2dregressiekernel(const float3* __restrict__ points, const uint32_t* __restrict__ clusterindices,
                                               const uint32_t* __restrict__ clusterstarts, uint32_t numclusters,
                                               float3 axis, float3* __restrict__ centers, float* __restrict__ radii)
{
    uint32_t clusterId = blockIdx.x;
    if(clusterId >= numclusters) return;

    uint32_t start  = clusterstarts[clusterId];
    uint32_t end    = clusterstarts[clusterId + 1];
    uint32_t numpoints = end - start;

    if(numpoints < 3) return;

    // Project points onto 2D plane
    float3 u, v;
    // create a coordinate system from the axis
    if(abs(axis.x) > 0.9f)
    {
        u = normalize(cross(axis, makeFloat3(0, 1, 0)));
    } else
    {
        u = normalize(cross(axis, makeFloat3(1, 0, 0)));
    }
    v = cross(axis, u);

    extern shared float2 projpoints[];
    for(uint32_t i = threadIdx.x; i < numpoints; i += blockDim.x)
    {
        float3 p   = points[clusterindices[start + i]];
        projpoints[i] = makefloat2(dot(p, u), dot(p, v));
    }
    __syncthreads();

    // Find median of x and y coordinates
    // use cub for block-wide sort
    typedef cub::blockradixsort<float2, 256, 1, float> blockradixsort;
    shared typename blockradixsort::tempstorage     tempstorage;

    // sort by x
    blockradixsort(tempstorage).sort(projpoints, numpoints);
    __syncthreads();
    float medianx = projpoints[numpoints / 2].x;

    // sort by y
    blockradixsort(tempstorage).sort(projpoints, numpoints, [](const float2& a) { return a.y; });
    __syncthreads();
    float mediany = projpoints[numpoints / 2].y;

    float2 center2d = makefloat2(medianx, mediany);

    // calculate radii
    shared float s_radii[];
    for(uint32_t i = threadIdx.x; i < numpoints; i += blockDim.x)
    {
        float2 p    = projpoints[i];
        float2 diff = makefloat2(p.x - center2d.x, p.y - center2d.y);
        s_radii[i]   = sqrtf(dot(diff, diff));
    }
    __syncthreads();

    // Find median radius
    cub::blocksort<float, 256, 1>             blocksort;
    shared typename blocksort::tempstorage tempstorageradii;
    blocksort(tempstorageradii).sort(s_radii, numpoints);
    __syncthreads();
    float medianradius = s_radii[numpoints / 2];

    // write results
    if(threadIdx.x == 0)
    {
        centers[clusterId] = u * center2d.x + v * center2d.y;
        radii[clusterId]   = medianradius;
    }
}

void h_buissteunpunt2d(const DeviceBuffer<float3>& d_points, const DeviceBuffer<uint32_t>& d_clusterindices,
                       const DeviceBuffer<uint32_t>& d_clusterstarts, uint32_t numclusters, const float3& axis,
                       DeviceBuffer<float3>& d_centers, DeviceBuffer<float>& d_radii)
{
    const uint32_t blocksize     = 256;
    const uint32_t sharedmemsize = blocksize * sizeof(float2) + blocksize * sizeof(float);
    k_localcenter2dregressiekernel<<<numclusters, blocksize, sharedmemsize>>>(
        d_points.data(), d_clusterindices.data(), d_clusterstarts.data(), numclusters, axis, d_centers.data(), d_radii.data());
}
