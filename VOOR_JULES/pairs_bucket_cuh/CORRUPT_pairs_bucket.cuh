#pragma once
#include "otndc.h" // for OTAsAccu
#include <cuda_runtime.h>

// kernel forward declaration
extern "C" __global__ void gGKotndcPairs(const float* __restrict__ mX, const float* __restrict__ sSY,
                                         const float* __restrict__ sSZ, int N, float g_innerSq, float g_outerSq,
                                         float m_maxDotOrtho, int m_partnerZoekRadius, OtasAccu* __restrict__ g_gAccu);
