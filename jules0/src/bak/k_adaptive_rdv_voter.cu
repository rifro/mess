#include "includes.cuh"

__device__ float cos32_pow(float cos_val) {
    float x = cos_val * cos_val;
    x = x * x; x = x * x; x = x * x; x = x * x;
    return x;
}

__device__ float sin32_pow(float angle_rad) {
    return cos32_pow(cosf(d_config.math.PI / 2.0f - angle_rad));
}

__device__ bool tryMatchAndNudgeMainCache(const float3& n_in, OTAsAccu* slots) {
    float maxWeight = d_config.global.min_nudge_threshold;
    int best_slot = -1;

    for (int i = 0; i < d_config.global.cache_size_k; ++i) {
        if (slots[i].count == 0) continue;
        float3 v_as = slots[i].som;
        float cos_theta = fabsf(dot(n_in, v_as));
        
        float w_flat = cos32_pow(cos_theta);
        float w_tube = sin32_pow(acosf(cos_theta));
        
        float w_final = fmaxf(w_flat, w_tube);
        
        if (w_final > maxWeight) {
            maxWeight = w_final;
            best_slot = i;
        }
    }
    
    if (best_slot != -1) {
        float alpha = 1.0f / (slots[best_slot].count + d_config.global.init_count);
        slots[best_slot].som = normalize((slots[best_slot].som * (1.0f - alpha)) + (n_in * (alpha * maxWeight)));
        atomicAdd(&slots[best_slot].count, 1);
        return true;
    }
    
    return false;
}

__global__ void k_adaptiveRdvVoterKernel(
    const float3* __restrict__ normalen,
    u32 normalsCount,
    OTAsAccu* __restrict__ slots,
    float3* __restrict__ ringBuffer,
    u32* __restrict__ ringBuffer_pos
) {
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= normalsCount) return;

    float3 n_in = normalen[i];
    if (tryMatchAndNudgeMainCache(n_in, slots)) {
        return; // Success
    }

    u32 pos = atomicAdd(ringBuffer_pos, 1) % d_config.global.ringBuffer_n;
    ringBuffer[pos] = n_in;
}


void h_adaptive_rdv_voting(
    const DeviceBuffer<float3>& d_normalen,
    const DeviceBuffer<u32>& d_normalsCount,
    DeviceBuffer<OTAsAccu>& d_slots,
    DeviceBuffer<float3>& d_ringBuffer,
    DeviceBuffer<u32>& d_ringBuffer_pos
) {
    u32 h_normalsCount;
    d_normalsCount.download(&h_normalsCount, 1);
    if (h_normalsCount == 0) return;

    const u32 blockSize = 256;
    const u32 grid_size = (h_normalsCount + blockSize - 1) / blockSize;

    k_adaptiveRdvVoterKernel<<<grid_size, blockSize>>>(
        d_normalen.data(), h_normalsCount, d_slots.data(),
        d_ringBuffer.data(), d_ringBuffer_pos.data()
    );
}
