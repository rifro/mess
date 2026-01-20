#pragma once
#include "device_buffer.cuh"
#include "otndc.h"
#include <vector>

void h_reduceGlobalConsensus(const Std::vector<Std::vector<OtasAccu>>& allChunkSlots, AxisResults& g_finalResult);
