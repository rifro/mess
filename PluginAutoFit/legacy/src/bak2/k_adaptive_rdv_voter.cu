#include "includes.cuh"

__device__ float cos32Pow(float cosVal)
{
    float x = cosVal * cosVal;
    x       = x * x;
    x       = x * x;
    x       = x * x;
    x       = x * x;
    return x;
}

__device__ float sin32Pow(float angleRad) { return cos32Pow(cosf(dConfig.math.PI / 2.0f - angleRad)); }

__device__ bool tryMatchAndNudgeMainCache(const float3& nIn, OTAsAccu* slots)
{
    float maxWeight = dConfig.global.minNudgeThreshold;
    int   bestSlot  = -1;

    for(int i = 0; i < dConfig.global.cacheSizeK; ++i)
    {
        if(slots[i].count == 0) continue;
        float3 vAs      = slots[i].som;
        float  cosTheta = fabsf(dot(nIn, vAs));

        float wFlat = cos32Pow(cosTheta);
        float wTube = sin32Pow(acosf(cosTheta));

        float wFinal = fmaxf(wFlat, wTube);

        if(wFinal > maxWeight)
        {
            maxWeight = wFinal;
            bestSlot  = i;
        }
    }

    if(bestSlot != -1)
    {
        float alpha          = 1.0f / (slots[bestSlot].count + dConfig.global.initCount);
        slots[bestSlot].som = normalize((slots[bestSlot].som * (1.0f - alpha)) + (nIn * (alpha * maxWeight)));
        atomicAdd(&slots[bestSlot].count, 1);
        return true;
    }

    return false;
}

__global__ void k_adaptiveRdvVoterKernel(const float3* __restrict__ normalen, u32 normalsCount,
                                            OTAsAccu* __restrict__ slots, float3* __restrict__ ringBuffer,
                                            u32* __restrict__ ringBufferPos)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= normalsCount) return;

    float3 nIn = normalen[i];
    if(tryMatchAndNudgeMainCache(nIn, slots))
    {
        return; // Success
    }

    u32 pos    = atomicAdd(ringBufferPos, 1) % dConfig.global.ringBufferN;
    ringBuffer[pos] = nIn;
}

void h_adaptiveRdvVoting(const DeviceBuffer<float3>& dNormalen, const DeviceBuffer<u32>& dNormalenCount,
                           DeviceBuffer<OTAsAccu>& dSlots, DeviceBuffer<float3>& dRingbuffer,
                           DeviceBuffer<u32>& dRingbufferPos)
{
    u32 hNormalenCount;
    dNormalenCount.download(&hNormalenCount, 1);
    if(hNormalenCount == 0) return;

    const u32 blockSize = 256;
    const u32 gridSize  = (hNormalenCount + blockSize - 1) / blockSize;

    k_adaptiveRdvVoterKernel<<<gridSize, blockSize>>>(dNormalen.data(), hNormalenCount, dSlots.data(),
                                                           dRingbuffer.data(), dRingbufferPos.data());
}
