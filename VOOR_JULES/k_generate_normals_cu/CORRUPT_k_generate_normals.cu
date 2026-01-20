#include "k_generate_normals.cuh"
#include "config.h"
#include <cuda_runtime.h>
#include "math_utils.cuh"
#include "point_types.h"

__device__ bool isCollinear(const float3& p1, const float3& p2, const float3& p3) {
    float3 side1 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    float3 side2 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
    return dot(cross(side1, side2), cross(side1, side2)) < d_config.ringFilter.min_area_sq;
}

__device__ bool afstandstestSlaagt(const float3& p1, const float3& p2, const float3& p3, const float3& p_test, float3* n_out) {
    if (isCollinear(p1, p2, p3)) return false;
    
    float3 normal = normalize(cross(make_float3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z), make_float3(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z)));
    float d = dot(normal, p1);
    float distanceSq = powf(dot(normal, p_test) - d, 2);
    
    bool succeeded = distanceSq < d_config.ringFilter.epsSq;
    if (succeeded && n_out) *n_out = normal;
    
    return succeeded;
}

__global__ void k_generateNormalsKernel(
    const float* __restrict__ d_x,
    const float* __restrict__ d_y,
    const float* __restrict__ d_z,
    u32 N,
    u8* __restrict__ d_labels,
    float3* __restrict__ d_normalen,
    u32* __restrict__ d_normalsCount,
    u32 maxNormals
) {
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;

    float3 p_center = {d_x[i], d_y[i], d_z[i]};
    float3 q_ring[4];
    int ringCount = 0;
    int farPointCount = 0;

    for (int j = i + 1; j < N; ++j) {
        if (farPointCount >= 2) break;

        float3 p_neighbor = {d_x[j], d_y[j], d_z[j]};
        float3 diff = {p_neighbor.x - p_center.x, p_neighbor.y - p_center.y, p_neighbor.z - p_center.z};
        float distSq = dot(diff, diff);

        if (distSq <= d_config.ringFilter.r2Sq) {
            if (distSq > d_config.ringFilter.r1Sq) {
                if (ringCount < 4) {
                    q_ring[ringCount++] = p_neighbor;
                }
            }
        } else {
            farPointCount++;
        }
    }

    if (ringCount < 3) {
        d_labels[i] = PointType::Chaos;
        return;
    }

    float3 n_final;
    float3 p1 = q_ring[0], p2 = q_ring[1], p3 = q_ring[2];

    if (afstandstestSlaagt(p_center, p1, p2, p3, &n_final) || afstandstestSlaagt(p1, p2, p3, p_center, &n_final)) {
        u32 Index = atomicAdd(d_normalsCount, 1);
        if (Index < maxNormals) {
            d_normalen[Index] = n_final;
        }
    } else {
        d_labels[i] = PointType::Chaos;
    }
}

void h_generate_normals(
    const DeviceBuffer<float>& d_x,
    const DeviceBuffer<float>& d_y,
    const DeviceBuffer<float>& d_z,
    DeviceBuffer<u8>& d_labels,
    DeviceBuffer<float3>& d_normalen,
    DeviceBuffer<u32>& d_normalsCount
) {
    const u32 N = d_x.size();
    const u32 maxNormals = d_normalen.size();
    d_normalsCount.memset(0);

    const u32 blockSize = 256;
    const u32 grid_size = (N + blockSize - 1) / blockSize;

    k_generateNormalsKernel<<<grid_size, blockSize>>>(
        d_x.data(), d_y.data(), d_z.data(), N,
        d_labels.data(), d_normalen.data(), d_normalsCount.data(), maxNormals
    );
}
