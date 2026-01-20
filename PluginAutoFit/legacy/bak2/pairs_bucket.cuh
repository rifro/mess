#include "includes.cuh"
#pragma once

// kernel forward declaration
extern "C" __global__ void g_gGkotndcPairs(const float* __restrict__ m_x, const float* __restrict__ sSY,
                                           const float* __restrict__ s_sSz, int N, float g_innerSq, float g_outerSq,
                                           float m_maxDotOrtho, int m_partnerZoekRadius,
                                           OtasAccu* __restrict__ g_gAccu);
