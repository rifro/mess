#pragma once
#include "otndc.h" // for OTAsAccu
#include <cudaRuntime.h>

// kernel forward declaration
extern "C" __global__ void kOtndcPairs(const float* __restrict__ x, const float* __restrict__ y,
                                         const float* __restrict__ z, int N, float innerSq, float outerSq,
                                         float maxDotOrtho, int partnerZoekRadius, OTAsAccu* __restrict__ gAccu);
