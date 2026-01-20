#pragma once
#include "device_buffer.cuh"
#include "otndc.h"
#include <vector>

void h_reduceGlobalConsensus(
    const std::vector<std::vector<OTAsAccu>>& allChunkSlots,
    AxisResults& finalResult
);
