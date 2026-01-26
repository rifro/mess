#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    // Forward declaration for the CUDA kernel
    __global__ void k_adaptiveRdvVoterKernel(
        const Vec3f* __restrict__ d_normals,
        u32 normalsCount,
        RdvAxisAccumulator* __restrict__ d_axisAccumulators,
        Vec3f* __restrict__ d_ringBuffer,
        u32* __restrict__ d_ringBufferPosition);

    /**
     * @brief Calculates a weight based on the perpendicularity of two vectors.
     * @details This function implements a highly sensitive weighting curve. Given cos(theta),
     * it calculates (sin^2(theta))^16. This is computationally efficient as it reuses
     * the dot product result (cos) and avoids trigonometric functions. The high power (32)
     * strongly rewards vectors that are nearly perpendicular (cos(theta) close to 0)
     * and heavily penalizes those that are not, creating a sharp cutoff.
     * @param cosTheta The cosine of the angle between the normal and an axis.
     * @return A float weight between 0.0 and 1.0.
     */
    __device__ float calculatePerpendicularityWeight(float cosTheta)
    {
        // sin^2(t) = 1 - cos^2(t). This avoids a costly sin() or asin() call.
        float sinSq = 1.0f - cosTheta * cosTheta;
        // Efficiently calculate (sinSq)^16 via repeated squaring.
        float p2 = sinSq * sinSq;
        float p4 = p2 * p2;
        float p8 = p4 * p4;
        return p8 * p8; // p16
    }

    /**
     * @brief Attempts to match a normal to an existing axis and "nudge" it.
     * @details Iterates through the cached axes (accumulators). If a normal is found to be
     * nearly perpendicular to an axis (within a cosine cutoff), it contributes to that axis.
     * The contribution "nudges" the axis's average direction via a weighted sum, refining it.
     * The weight is determined by `calculatePerpendicularityWeight`.
     * @param normal The input surface normal to process.
     * @param d_accumulators Pointer to the array of axis accumulators in device memory.
     * @return `true` if the normal was matched and used to nudge an axis, `false` otherwise.
     */
    __device__ bool tryMatchAndNudge(const Vec3f& normal, RdvAxisAccumulator* __restrict__ d_accumulators)
    {
        float maxWeight = d_config.m_rdvVoter.m_minNudgeThreshold;
        int bestSlot = -1;

        for (u32 i = 0; i < d_config.m_rdvVoter.m_cacheSize; ++i)
        {
            if (d_accumulators[i].m_voteCount == 0) continue;

            Vec3f axisDirection = normalize(d_accumulators[i].m_vectorSum);
            float cosTheta = fabsf(dot(normal, axisDirection));

            if (cosTheta < d_config.m_rdvVoter.m_cosCutoff)
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
            u32 oldCount = atomicAdd(&d_accumulators[bestSlot].m_voteCount, 1);
            // The learning rate `alpha` decreases as more votes are cast, stabilizing the axis.
            float alpha = 1.0f / (oldCount + d_config.m_rdvVoter.m_initialVoteCount);
            Vec3f nudge = normal * (alpha * maxWeight);
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_x, nudge.m_x);
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_y, nudge.m_y);
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_z, nudge.m_z);
            return true;
        }

        return false;
    }

    void h_adaptiveRdvVoting(
        const DeviceBuffer<Vec3f>& d_normals,
        const DeviceBuffer<u32>& d_normalsCount,
        DeviceBuffer<RdvAxisAccumulator>& d_axisAccumulators,
        DeviceBuffer<Vec3f>& d_ringBuffer,
        DeviceBuffer<u32>& d_ringBufferPosition)
    {
        // The number of normals is determined by the previous kernel, so we must
        // copy it back to the host to determine the correct grid size for this kernel.
        u32 h_normalsCount = 0;
        cudaMemcpy(&h_normalsCount, d_normalsCount.data(), sizeof(u32), cudaMemcpyDeviceToHost);

        if (h_normalsCount == 0) return;

        const u32 blockSize = 256;
        const u32 gridSize = (h_normalsCount + blockSize - 1) / blockSize;

        k_adaptiveRdvVoterKernel<<<gridSize, blockSize>>>(
            d_normals.data(),
            h_normalsCount,
            d_axisAccumulators.data(),
            d_ringBuffer.data(),
            d_ringBufferPosition.data());
    }

    /**
     * @brief CUDA kernel for performing one step of the Radial Disk Voting algorithm.
     * @details Each thread processes one normal. It first attempts to match the normal
     * to an existing axis candidate using `tryMatchAndNudge`. If no suitable axis is
     * found, the normal is considered an "orphan" and is placed into a global ring
     * buffer, where it might be used later to form a new axis with other orphans.
     *
     * @param d_normals Array of input normals.
     * @param normalsCount The total number of normals to process (passed by value).
     * @param d_axisAccumulators Array of candidate axes.
     * @param d_ringBuffer Circular buffer for orphan normals.
     * @param d_ringBufferPosition Atomic counter for the ring buffer.
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
            /**
             * @brief Ring Buffer Eviction Strategy
             * @details When a normal fails to match an existing axis, it is placed in a FIFO
             * ring buffer. In the current strategy, when the buffer is full, the oldest
             * normal is simply overwritten and discarded. This is the simplest approach,
             * minimizing computation but potentially losing information from normals that
             * are evicted before their corresponding axis emerges.
             */
            u32 ringBufferIndex = atomicAdd(d_ringBufferPosition, 1);
            // Use bitwise AND with a mask for efficient circular addressing.
            // This is much faster than a modulo operation, but requires the buffer size to be a power of two.
            u32 pos = ringBufferIndex & RdvVoterConfig::getRingBufferMask();

            // The evicted normal is the one currently at `pos` before we overwrite it.
            Vec3f evictedNormal = d_ringBuffer[pos];

            d_ringBuffer[pos] = inputNormal;

            /**
             * @brief Optional "Last Vote" for Evicted Normals
             * @details Enabling the code below gives an evicted normal one last chance to vote.
             * This ensures no information is lost, as every normal contributes to an axis at
             * some point. However, this may not always be desirable. If an axis has already
             * stabilized, a late vote from a noisy or outlier normal could slightly corrupt
             * its orientation. It is disabled by default to favor stability over utilizing
             * every last piece of data.
             */
            // if (ringBufferIndex >= d_config.m_rdvVoter.m_ringBufferSize) {
            //     tryMatchAndNudge(evictedNormal, d_axisAccumulators);
            // }
        }
    }

} // namespace Bocari
