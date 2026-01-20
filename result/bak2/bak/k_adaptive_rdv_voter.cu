#include "k_adaptiveRdvVoterer.cuh"
#include "Config.h"
#include <cuda_runtime.h>
#include "math_utils.cuh"

// constants
constexpr float minVoteWeight = 0.1f;
constexpr float paraboolA = -9e-15f;
constexpr float alphaStake = 0.1f;
constexpr float cos75 = 0.2588190451f;
constexpr int cacheSize = 16;
constexpr int ringSize = 8;

__device__ float pow16(float x) {
    x *= x; x *= x; x *= x; x *= x;
    return x;
}

__device__ float sin32pow(float sinSq) {
    return pow16(sinSq);
}

__device__ void swapEntries(cacheEntry& a, cacheEntry& b) {
    cacheEntry temp = a;
    a = b;
    b = temp;
}

__device__ bool tryForward_vote(float3 radial, rdvCache* cache, int* cacheCount) {
    float bestWeight = minVoteWeight;
    int bestIndex = -1;
    for (int i = 0; i < *cacheCount; ++i) {
        float dot = fabsf(dot(radial, cache->entries[i].axis));
        if (dot < cos75) {
            float sinSq = 1.0f - dot * dot;
            float weight = sin32pow(sinSq);
            if (weight > bestWeight) {
                bestWeight = weight;
                bestIndex = i;
            }
        }
    }
    if (bestIndex != -1) { //RR!!! voteWeight, voteCount
        atomicAdd(&cache->entries[bestIndex].voteWeight, bestWeight);
        int j = bestIndex;
        while (j > 0 && cache->entries[j].voteWeight > cache->entries[j - 1].voteWeight) {
            swapEntries(cache->entries[j], cache->entries[j - 1]);
            --j;
        }
        cacheEntry* entry = &cache->entries[bestIndex];
        atomicAdd(&entry->voteCount, 1u);
        u32 count = entry->voteCount;
        float countf = (float)count;
        float alpha = 1.0f + paraboolA * countf * countf;
        if (alpha > alphaStake) {
            float3 nudge = (1.0f - alpha) * entry->axis + alpha * normalize(radial) * bestWeight;
            entry->axis = normalize(nudge);
        }
        return true;
    }
    return false;
}

__device__ bool tryCrossWithBestBufferMatch(float3 normal, rdvRingBuffer* ring, rdvCache* cache, int* cacheCount) {
    if (!ring->full && ring->count == 0) return false;
    float bestDot = cos75;
    int bestIndex = -1;
    int limit = ring->full ? ringSize : ring->count;
    for (int i = 0; i < limit; ++i) {
        float dot = fabsf(dot(normal, ring->entries[i]));
        if (dot < bestDot) {
            bestDot = dot;
            bestIndex = i;
        }
    }
    if (bestIndex != -1 && bestDot < cos75) {
        float3 cross = normalize(cross(normal, ring->entries[bestIndex]));
        pushbackNewAxis(cross, 1.0f, cache, cacheCount);
        return true;
    }
    return false;
}

__device__ void addToRingBuffer(float3 normal, rdvRingBuffer* ring, rdvCache* cache, int* cacheCount) {
    if (ring->full) {
        tryReverseVote(ring->entries[ring->count], cache, cacheCount);
        ring->entries[ring->count] = normal;
        ring->count = (ring->count + 1) & (ringSize - 1);
    } else {
        ring->entries[ring->count] = normal;
        ring->count = (ring->count + 1) & (ringSize - 1);
        if (ring->count == 0) ring->full = true;
    }
}

__device__ bool tryReverseVote(float3 radial, rdvCache* cache, int* cacheCount) {
    float bestWeight = minVoteWeight;
    int bestIndex = -1;
    for (int i = *cacheCount - 1; i >= 0; --i) {
        if (cache->entries[i].voteWeight > g_config.rdv.recentAxisCutoff) break;
        float dot = fabsf(dot(radial, cache->entries[i].axis));
        if (dot < cos75) {
            float sinSq = 1.0f - dot * dot;
            float weight = sin32pow(sinSq);
            if (weight > bestWeight) {
                bestWeight = weight;
                bestIndex = i;
            }
        }
    }
    if (bestIndex != -1) {
        atomicAdd(&cache->entries[bestIndex].voteWeight, bestWeight);
        int j = bestIndex;
        while (j > 0 && cache->entries[j].voteWeight > cache->entries[j - 1].voteWeight) {
            swapEntries(cache->entries[j], cache->entries[j - 1]);
            --j;
        }
        cacheEntry* entry = &cache->entries[bestIndex];
        atomicAdd(&entry->voteCount, 1u);
        u32 count = entry->voteCount;
        float countf = (float)count;
        float alpha = 1.0f + paraboolA * countf * countf;
        if (alpha > alphaStake) {
            float3 nudge = (1.0f - alpha) * entry->axis + alpha * normalize(radial) * bestWeight;
            entry->axis = normalize(nudge);
        }
        return true;
    }
    return false;
}

__device__ void processNormal(float3 normal, rdvRingBuffer* ring, rdvCache* cache, int* cacheCount) {
    if (lengthsq(normal) < 1e-12f) return;
    if (tryForward_vote(normal, cache, cacheCount)) return;
    if (tryCrossWithBestBufferMatch(normal, ring, cache, cacheCount)) return;
    addToRingBuffer(normal, ring, cache, cacheCount);
}

__device__ void pushbackNewAxis(float3 newaxis, float initweight, rdvCache* cache, int* cacheCount) {
    newaxis = normalize(newaxis);
    int Index = atomicAdd(cacheCount, 1);
    if (Index < cacheSize) {
        cache->entries[Index].axis = newaxis;
        cache->entries[Index].voteWeight = initweight;
        cache->entries[Index].voteCount = 1u;
    } else {
        int evictIndex = cacheSize - 1;
        cache->entries[evictIndex].axis = newaxis;
        cache->entries[evictIndex].voteWeight = initweight;
        cache->entries[evictIndex].voteCount = 1u;
    }
}

global void k_adaptiveRdvVoterKernel(
    const float3* normals,
    u32 normalsCount,
    rdvCache* cache,
    int* cacheCount,
    rdvRingBuffer* rings  // shared per-thread rings
) {
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= normalsCount) return;

    rdvRingBuffer* myring = &rings[threadIdx.x];
    if (threadIdx.x == 0) *cacheCount = 3;  // init (primaries in __host__)

    float3 normal = normals[i];
    processNormal(normal, myring, cache, cacheCount);
}

void h_adaptiveRdvVoting(
    const DeviceBuffer<float3>& d_normals,
    const DeviceBuffer<u32>& d_normalsCount,
    DeviceBuffer<rdvCache>& d_cache,
    DeviceBuffer<rdvRingBuffer>& d_rings,
    DeviceBuffer<int>& d_cacheCount
) {
    u32 h_normalsCount;
    d_normalsCount.download(&h_normalsCount, 1);
    if (h_normalsCount == 0) return;

    const u32 blockSize = 256;
    const u32 gridSize = (h_normalsCount + blockSize - 1) / blockSize;
    const Size_t sharedSize = blockSize * Sizeof(rdvRingBuffer);

    k_adaptiveRdvVoterKernel<<<gridSize, blockSize, sharedSize>>>(
        d_normals.data(), h_normalsCount, d_cache.data(),
        d_cacheCount.data(), d_rings.data()
    );
}
