#pragma once
#include "device_buffer.cuh"
#include "types.cuh"
#include "otndc.h"

void h_idealFrameRotation(
    DeviceBuffer<float3>& d_points,
    const AxisResults& axisResults
);
