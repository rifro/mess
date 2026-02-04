#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    /**
     * @brief Calculates the weight for a vote based on the perpendicularity of the normal to the axis.
     * @details Uses sin(theta)^32 for high selectivity.
     */
    __device__ float calculatePerpendicularityWeight(float cosTheta)
    {
        float sinSq = 1.0f - cosTheta * cosTheta;
        float p2 = sinSq * sinSq;
        float p4 = p2 * p2;
        float p8 = p4 * p4;
        return p8 * p8; // Result is sin(theta)^32
    }

    /**
     * @brief Attempts to match a normal to an existing axis candidate and "nudges" it.
     */
    __device__ bool tryMatchAndNudge(const Vec3f& normal, RdvAxisAccumulator* __restrict__ d_accumulators)
    {
        float maxWeight = d_config.rdvVoter.minNudgeThreshold;
        int bestSlot = -1;

        for (u32 i = 0; i < d_config.rdvVoter.cacheSize; ++i)
        {
            // Atomically read the current weight to check if the axis is active
            float currentWeight = atomicAdd(&d_accumulators[i].voteWeightSum, 0.0f);
            if (currentWeight == 0.0f) continue;

            Vec3f axisDirection = normalize(d_accumulators[i].axis);
            float cosTheta = fabsf(dot(normal, axisDirection));

            if (cosTheta < d_config.rdvVoter.cosCutoff)
            {
                float weight = calculatePerpendicularityWeight(cosTheta);
                if (weight > maxWeight)
                {
                    maxWeight = weight;
                    bestSlot = i;
                }
            }
        }

        if (bestSlot != -1)
        {
            float currentWeight = atomicAdd(&d_accumulators[bestSlot].voteWeightSum, maxWeight);
            
            // Nudge only if the axis is not yet stable
            if (currentWeight < d_config.rdvVoter.nudgeWeightCap)
            {
                float alpha = 1.0f / (currentWeight + 1.0f); // Simple decreasing learning rate
                Vec3f nudge = normal * (alpha * maxWeight);
                atomicAdd(&d_accumulators[bestSlot].axis.x, nudge.x);
                atomicAdd(&d_accumulators[bestSlot].axis.y, nudge.y);
                atomicAdd(&d_accumulators[bestSlot].axis.z, nudge.z);
            }
            return true;
        }

        return false;
    }

    /**
     * @brief CUDA kernel for performing one step of the Radial Disk Voting algorithm.
     * @details Each thread processes one normal. It first attempts to match the normal
     * to an existing axis candidate using `tryMatchAndNudge`. If no suitable axis is
     * found, the normal is placed into a global ring buffer.
     */
    __global__ void k_adaptiveRdvVoterKernel(
        const Vec3f* __restrict__ d_normals,
        u32 normalsCount,
        RdvAxisAccumulator* __restrict__ d_axisAccumulators,
        Vec3f* __restrict__ d_ringBuffer,
        u32* __restrict__ d_ringBufferPosition)
    {
        const u32 normalIndex = blockIdx.x * blockDim.x + threadIdx.x;
        if (normalIndex >= normalsCount) return;

        Vec3f inputNormal = d_normals[normalIndex];

        if (!tryMatchAndNudge(inputNormal, d_axisAccumulators))
        {
            u32 ringBufferIndex = atomicAdd(d_ringBufferPosition, 1);
            
            // Efficient circular addressing using bitwise AND (requires power-of-two size).
            u32 pos = ringBufferIndex & d_config.rdvVoter.ringBufferMask;

            d_ringBuffer[pos] = inputNormal;
        }
    }

    /**
     * @brief Host dispatcher for the RDV voting kernel.
     */
    void h_adaptiveRdvVoting(
        const Vec3f* d_normals,
        const u32* d_normalsCount,
        RdvAxisAccumulator* d_axisAccumulators,
        Vec3f* d_ringBuffer,
        u32* d_ringBufferPosition)
    {
        // Copy the atomic count back to host to determine grid size
        u32 h_normalsCount = 0;
        cudaMemcpy(&h_normalsCount, d_normalsCount, sizeof(u32), cudaMemcpyDeviceToHost);

        if (h_normalsCount == 0) return;

        const u32 blockSize = 256;
        const u32 gridSize = (h_normalsCount + blockSize - 1) / blockSize;

        k_adaptiveRdvVoterKernel<<<gridSize, blockSize>>>(
            d_normals,
            h_normalsCount,
            d_axisAccumulators,
            d_ringBuffer,
            d_ringBufferPosition);
    }

} // namespace Bocari
