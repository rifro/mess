#pragma once
#include "device_buffer.cuh"
#include "types.cuh"
#include "otndc.h"

void h_adaptiveRdvVoting(
    const DeviceBuffer<float3>& d_normals,
    const DeviceBuffer<uint32_t>& d_normalsCount,
    DeviceBuffer<OTAsAccu>& d_slots,
    DeviceBuffer<float3>& d_ringBuffer,
    DeviceBuffer<uint32_t>& d_ringBufferPos
);
