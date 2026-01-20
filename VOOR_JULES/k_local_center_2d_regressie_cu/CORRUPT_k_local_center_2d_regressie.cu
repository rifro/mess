#include "k_local_center2d_regressie.cuh"
#include <cuda_runtime.h>
#include "math_utils.cuh"
#include <cub/cub.cuh>

__global__ void k_local_center2d_regressie_kernel(
    const float3* __restrict__ points,
    const u32* __restrict__ clusterIndices,
    const u32* __restrict__ clusterStarts,
    u32 numClusters,
    float3 axis,
    float3* __restrict__ centers,
    float* __restrict__ radii
) {
    u32 clusterId = blockIdx.x;
    if (clusterId >= numClusters) return;

    u32 start = clusterStarts[clusterId];
    u32 end = clusterStarts[clusterId + 1];
    u32 numPoints = end - start;

    if (numPoints < 3) return;

    // Project points onto 2D plane
    float3 u, v;
    // Create a coordinate system from the axis
    if (abs(axis.x) > 0.9f) {
        u = normalize(cross(axis, make_float3(0, 1, 0)));
    } else {
        u = normalize(cross(axis, make_float3(1, 0, 0)));
    }
    v = cross(axis, u);

    extern __shared__ float2 projPoints[];
    for (u32 i = threadIdx.x; i < numPoints; i += blockDim.x) {
        float3 p = points[clusterIndices[start + i]];
        projPoints[i] = make_float2(dot(p, u), dot(p, v));
    }
    __syncthreads();

    // Find median of x and y coordinates
    // Use CUB for block-wide sort
    typedef cub::BlockRadixSort<float2, 256, 1, float> BlockRadixSort;
    __shared__ typename BlockRadixSort::TempStorage temp_storage;
    
    // Sort by x
    BlockRadixSort(temp_storage).Sort(projPoints, numPoints);
    __syncthreads();
    float median_x = projPoints[numPoints / 2].x;

    // Sort by y
    BlockRadixSort(temp_storage).Sort(projPoints, numPoints, [](const float2& a){ return a.y; });
    __syncthreads();
    float median_y = projPoints[numPoints / 2].y;

    float2 center2d = make_float2(median_x, median_y);
    
    // Calculate radii
    __shared__ float s_radii[];
    for (u32 i = threadIdx.x; i < numPoints; i += blockDim.x) {
        float2 p = projPoints[i];
        float2 diff = make_float2(p.x - center2d.x, p.y - center2d.y);
        s_radii[i] = sqrtf(dot(diff, diff));
    }
    __syncthreads();

    // Find median radius
    cub::BlockSort<float, 256, 1> BlockSort;
    __shared__ typename BlockSort::TempStorage tempStorageRadii;
    BlockSort(tempStorageRadii).Sort(s_radii, numPoints);
    __syncthreads();
    float median_radius = s_radii[numPoints / 2];

    // Write results
    if (threadIdx.x == 0) {
        centers[clusterId] = u * center2d.x + v * center2d.y;
        radii[clusterId] = median_radius;
    }
}


void h_pipe_steunpunt_2d(
    const DeviceBuffer<float3>& d_points,
    const DeviceBuffer<u32>& d_clusterIndices,
    const DeviceBuffer<u32>& d_clusterStarts,
    u32 numClusters,
    const float3& axis,
    DeviceBuffer<float3>& d_centers,
    DeviceBuffer<float>& d_radii
) {
    const u32 blockSize = 256;
    const u32 shared_mem_size = blockSize * sizeof(float2) + blockSize * sizeof(float);
    k_local_center2d_regressie_kernel<<<numClusters, blockSize, shared_mem_size>>>(
        d_points.data(),
        d_clusterIndices.data(),
        d_clusterStarts.data(),
        numClusters,
        axis,
        d_centers.data(),
        d_radii.data()
    );
}
