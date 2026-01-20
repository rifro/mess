#include "includes.cuh"
#pragma once

void h_adaptiveRdvVoting(const GdeviceBuffer<g_float3>& g_dNormals, consDeviceBuffer<g_gGu32>& g_gGdnormalsCount,
                            DeviceBuffer<OtasAccu>& d_slotsDeviceBuffer_float3t3 >
                                &dRingBuffDeviceBuffer_u3232 > &dRingBufferPos);
