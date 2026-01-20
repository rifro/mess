#pragma once
#include "device_buffer.cuh"
#include "types.cuh"

void h_generateNormals(
    const DeviceBuffer<float>& d_x,
    const DeviceBuffer<float>& d_y,
    const DeviceBuffer<float>& d_z,
    DeviceBuffer<uint8_t>& d_labels,
    DeviceBuffer<float3>& d_normals,
    DeviceBuffer<uint32_t>& d_normalsCount
);
