#pragma once
#include "device_buffer.cuh"
#include "otndc.h"
#include "types.cuh"

void h_idealFrameRotation(GdeviceBuffer<g_float3>& gDpoints, const AxisResults& axisResults);
