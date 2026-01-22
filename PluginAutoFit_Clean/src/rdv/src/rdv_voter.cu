#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    // Forward declaration
    __global__ void k_adaptiveRdvVotingKernel(
        const Vec3f* d_normals,
        const u32* d_normalsCount,
        RdvAxisAccumulator* d_axisAccumulators,
        Vec3f* d_ringBuffer,
        u32* d_ringBufferPosition
    );

    // Helper functions
    __device__ float calculatePerpendicularityWeight(float cosTheta)
    {
        float sinSq = 1.0f - cosTheta * cosTheta;
        float p2 = sinSq * sinSq;
        float p4 = p2 * p2;
        float p8 = p4 * p4;
        return p8 * p8;
    }

    __device__ bool tryMatchAndNudge(const Vec3f& normal, RdvAxisAccumulator* d_accumulators)
    {
        float maxWeight = getConfig().m_rdvVoter.m_minNudgeThreshold;
        int bestSlot = -1;

        for (u32 i = 0; i < getConfig().m_rdvVoter.m_cacheSize; ++i)
        {
            if (d_accumulators[i].m_voteCount == 0) continue;

            Vec3f axisDirection = normalize(d_accumulators[i].m_vectorSum);
            float cosTheta = fabsf(dot(normal, axisDirection));

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
            u32 oldCount = atomicAdd(&d_accumulators[bestSlot].m_voteCount, 1);
            float alpha = 1.0f / (oldCount + getConfig().m_rdvVoter.m_initialVoteCount);
            Vec3f nudge = normal * (alpha * maxWeight);
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_x, nudge.m_x);
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_y, nudge.m_y);
            atomicAdd(&d_accumulators[bestSlot].m_vectorSum.m_z, nudge.m_z);
            return true;
        }

        return false;
    }

    // Host function
    void h_adaptiveRdvVoting(
        const DeviceBuffer<Vec3f>& d_normals,
        const DeviceBuffer<u32>& d_normalsCount,
        DeviceBuffer<RdvAxisAccumulator>& d_axisAccumulators,
        DeviceBuffer<Vec3f>& d_ringBuffer,
        DeviceBuffer<u32>& d_ringBufferPosition
    )
    {
        const u32 maxNormals = d_normals.size();
        if (maxNormals == 0) return;

        const u32 blockSize = 256;
        const u32 gridSize = (maxNormals + blockSize - 1) / blockSize;

        k_adaptiveRdvVotingKernel<<<gridSize, blockSize>>>(
            d_normals.data(),
            d_normalsCount.data(),
            d_axisAccumulators.data(),
            d_ringBuffer.data(),
            d_ringBufferPosition.data()
        );
    }

    // Kernel function
    __global__ void k_adaptiveRdvVotingKernel(
        const Vec3f* d_normals,
        const u32* d_normalsCount,
        RdvAxisAccumulator* d_axisAccumulators,
        Vec3f* d_ringBuffer,
        u32* d_ringBufferPosition
    )
    {
        const u32 normalsCount = *d_normalsCount;
        const u32 normalIndex = blockIdx.x * blockDim.x + threadIdx.x;
        if (normalIndex >= normalsCount) return;

        Vec3f inputNormal = d_normals[normalIndex];

        if (!tryMatchAndNudge(inputNormal, d_axisAccumulators))
        {
            u32 pos = atomicAdd(d_ringBufferPosition, 1) % getConfig().m_rdvVoter.m_ringBufferSize;
            d_ringBuffer[pos] = inputNormal;
        }
    }

} // namespace Bocari
