#pragma once
#include "device_buffer.cuh"
#include "types.cuh"

void h_pipeSupportPoint2d(
    const DeviceBuffer<float3>& d_points,
    const DeviceBuffer<uint32_t>& d_clusterIndices,
    const DeviceBuffer<uint32_t>& d_clusterStarts,
    uint32_t numClusters,
    const float3& axis,
    DeviceBuffer<float3>& d_centers,
    DeviceBuffer<float>& d_radii
);
