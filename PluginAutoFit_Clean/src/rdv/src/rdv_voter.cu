#include "rdv/include/rdv_voter.cuh"
#include "rdv/include/config.h"
#include "rdv/include/rdv_types.h"
#include "rdv/include/cuda_utils.cuh"
#include <cuda_runtime.h>

// --- Forward Declarations ---
namespace Bocari
{
namespace detail
{
    __global__ void k_adaptiveRdvVoting(
        const Vec3f* __restrict__ d_normals,
        const u32* __restrict__ d_normalsCount,
        RdvAxisAccumulator* __restrict__ d_axisAccumulators,
        Vec3f* __restrict__ d_ringBuffer,
        u32* __restrict__ d_ringBufferPosition
    );
}

    __device__ bool tryMatchAndNudge(const Vec3f& normal, RdvAxisAccumulator* d_accumulators);
}


// --- Host Function Implementation ---
void Bocari::k_adaptiveRdvVoting(
    const DeviceBuffer<Vec3f>& d_normals,
    const DeviceBuffer<u32>& d_normalsCount,
    DeviceBuffer<RdvAxisAccumulator>& d_axisAccumulators,
    DeviceBuffer<Vec3f>& d_ringBuffer,
    DeviceBuffer<u32>& d_ringBufferPosition
)
{
    // Launch a grid large enough to handle the entire buffer capacity.
    // The kernel itself will check the actual number of normals.
    const u32 maxNormals = d_normals.size();
    if (maxNormals == 0) return;

    const u32 blockSize = 256;
    const u32 gridSize = (maxNormals + blockSize - 1) / blockSize;

    detail::k_adaptiveRdvVoting<<<gridSize, blockSize>>>(
        d_normals.data(),
        d_normalsCount.data(), // Pass the count pointer to the kernel
        d_axisAccumulators.data(),
        d_ringBuffer.data(),
        d_ringBufferPosition.data()
    );
}

// --- Kernel and Device Implementation ---
namespace Bocari
{
    /**
     * @brief Calculates (sin(theta))^32, where theta is the angle between the normal and the axis.
     * This gives a high weight when the normal is nearly perpendicular to the axis.
     */
    __device__ float calculatePerpendicularityWeight(float cosTheta)
    {
        // sin^2(t) = 1 - cos^2(t)
        float sinSq = 1.0f - cosTheta * cosTheta;
        // Efficiently calculate (sin^2)^16 = sin^32
        float p2 = sinSq * sinSq;
        float p4 = p2 * p2;
        float p8 = p4 * p4;
        float p16 = p8 * p8;
        return p16;
    }


    /**
     * @brief Tries to match a normal to an existing axis in the accumulator cache. If a match is found,
     * the axis is "nudged" towards the normal, and its vote count is incremented.
     * @return True if a match was found and the axis was nudged, false otherwise.
     */
    __device__ bool tryMatchAndNudge(const Vec3f& normal, RdvAxisAccumulator* d_accumulators)
    {
        float maxWeight = getConfig().m_rdvVoter.m_minNudgeThreshold;
        int bestSlot = -1;

        for (u32 i = 0; i < getConfig().m_rdvVoter.m_cacheSize; ++i)
        {
            if (d_accumulators[i].m_voteCount == 0) continue;

            Vec3f axisDirection = normalize(d_accumulators[i].m_vectorSum);
            float cosTheta = fabsf(dot(normal, axisDirection));

            // We want normals perpendicular to the axis, so the weight should be high when cosTheta is near 0.
            if (cosTheta < getConfig().m_rdvVoter.m_cosCutoff)
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
            // Atomically increment the vote count
            u32 oldCount = atomicAdd(&d_accumulators[bestSlot].m_voteCount, 1);

            // Nudge factor decreases as more votes are cast
            float alpha = 1.0f / (oldCount + getConfig().m_rdvVoter.m_initialVoteCount);

            // Nudge the vector sum. This needs to be atomic.
            // A simple atomicAdd on floats is not sufficient for a Vec3f.
            // We need to use atomicCAS loops for each component.
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_x, normal.m_x * alpha * maxWeight);
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_y, normal.m_y * alpha * maxWeight);
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_z, normal.m_z * alpha * maxWeight);

            return true;
        }

        return false;
    }


    __global__ void detail::k_adaptiveRdvVoting(
        const Vec3f* __restrict__ d_normals,
        const u32* __restrict__ d_normalsCount,
        RdvAxisAccumulator* __restrict__ d_axisAccumulators,
        Vec3f* __restrict__ d_ringBuffer,
        u32* __restrict__ d_ringBufferPosition
    )
    {
        const u32 normalsCount = *d_normalsCount;
        const u32 normalIndex = blockIdx.x * blockDim.x + threadIdx.x;
        if (normalIndex >= normalsCount) return;

        Vec3f inputNormal = d_normals[normalIndex];

        // If the normal can't be matched to an existing axis, add it to the ring buffer.
        if (!tryMatchAndNudge(inputNormal, d_axisAccumulators))
        {
            u32 pos = atomicAdd(d_ringBufferPosition, 1) % getConfig().m_rdvVoter.m_ringBufferSize;
            d_ringBuffer[pos] = inputNormal;
        }
    }

} // namespace Bocari
