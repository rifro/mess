#include "rdv/include/rdv_voter.cuh"
#include "rdv/include/config.h"
#include "rdv/include/rdv_types.h"
#include "rdv/include/cuda_utils.cuh"
#include <cuda_runtime.h>

// --- Forward Declarations ---
namespace Bocari
{
    __global__ void k_adaptiveRdvVoterKernel(
        const Vec3f* __restrict__ d_normals,
        u32 normalsCount,
        RdvAxisAccumulator* __restrict__ d_axisAccumulators,
        Vec3f* __restrict__ d_ringBuffer,
        u32* __restrict__ d_ringBufferPosition
    );

    __device__ bool tryMatchAndNudge(const Vec3f& normal, RdvAxisAccumulator* accumulators);
}


// --- Host Function Implementation ---
void Bocari::adaptiveRdvVoting(
    const DeviceBuffer<Vec3f>& d_normals,
    const DeviceBuffer<u32>& d_normalsCount,
    DeviceBuffer<RdvAxisAccumulator>& d_axisAccumulators,
    DeviceBuffer<Vec3f>& d_ringBuffer,
    DeviceBuffer<u32>& d_ringBufferPosition
)
{
    // Get the actual number of normals from the device buffer where the count was stored.
    u32 hostNormalsCount = 0;
    cudaError_t err = cudaMemcpy(&hostNormalsCount, d_normalsCount.data(), sizeof(u32), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess)
    {
        // In a real application, you'd handle this error more gracefully.
        return;
    }

    if (hostNormalsCount == 0) return;

    const u32 blockSize = 256;
    const u32 gridSize = (hostNormalsCount + blockSize - 1) / blockSize;

    k_adaptiveRdvVoterKernel<<<gridSize, blockSize>>>(
        d_normals.data(),
        hostNormalsCount,
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
    __device__ bool tryMatchAndNudge(const Vec3f& normal, RdvAxisAccumulator* accumulators)
    {
        float maxWeight = getConfig().m_rdvVoter.m_minNudgeThreshold;
        int bestSlot = -1;

        for (u32 i = 0; i < getConfig().m_rdvVoter.m_cacheSize; ++i)
        {
            if (accumulators[i].m_voteCount == 0) continue;

            Vec3f axisDirection = normalize(accumulators[i].m_vectorSum);
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
            u32 oldCount = atomicAdd(&accumulators[bestSlot].m_voteCount, 1);

            // Nudge factor decreases as more votes are cast
            float alpha = 1.0f / (oldCount + getConfig().m_rdvVoter.m_initialVoteCount);

            // Nudge the vector sum. This needs to be atomic.
            // A simple atomicAdd on floats is not sufficient for a Vec3f.
            // We need to use atomicCAS loops for each component.
            atomicAdd(&accumulators[bestSlot].m_vectorSum.x, normal.x * alpha * maxWeight);
            atomicAdd(&accumulators[bestSlot].m_vectorSum.y, normal.y * alpha * maxWeight);
            atomicAdd(&accumulators[bestSlot].m_vectorSum.z, normal.z * alpha * maxWeight);

            return true;
        }

        return false;
    }


    __global__ void k_adaptiveRdvVoterKernel(
        const Vec3f* __restrict__ d_normals,
        u32 normalsCount,
        RdvAxisAccumulator* __restrict__ d_axisAccumulators,
        Vec3f* __restrict__ d_ringBuffer,
        u32* __restrict__ d_ringBufferPosition
    )
    {
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
