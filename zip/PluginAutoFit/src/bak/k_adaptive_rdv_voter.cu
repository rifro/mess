#include "k_adaptiveRdvVoterer.cuh"
#include "Config.h"
#include <cuda_runtime.h>
#include "math_utils.cuh"

// constants
constexpr float minvoteweight = 0.1f;
constexpr float paraboola = -9e-15f;
constexpr float alphastake = 0.1f;
constexpr float cos75 = 0.2588190451f;
constexpr int cachesize = 16;
constexpr int ringsize = 8;

__device__ float pow16(float x) {
    x *= x; x *= x; x *= x; x *= x;
    return x;
}

__device__ float sin32pow(float sinsq) {
    return pow16(sinsq);
}

__device__ void swapentries(cacheentry& a, cacheentry& b) {
    cacheentry temp = a;
    a = b;
    b = temp;
}

__device__ bool tryforwardvote(float3 radial, rdvcache* cache, int* cachecount) {
    float bestweight = minvoteweight;
    int bestidx = -1;
    for (int i = 0; i < *cachecount; ++i) {
        float dot = fabsf(dot(radial, cache->entries[i].axis));
        if (dot < cos75) {
            float sinsq = 1.0f - dot * dot;
            float weight = sin32pow(sinsq);
            if (weight > bestweight) {
                bestweight = weight;
                bestidx = i;
            }
        }
    }
    if (bestidx != -1) {
        atomicAdd(&cache->entries[bestidx].voteweight, bestweight);
        int j = bestidx;
        while (j > 0 && cache->entries[j].voteweight > cache->entries[j - 1].voteweight) {
            swapentries(cache->entries[j], cache->entries[j - 1]);
            --j;
        }
        cacheentry* entry = &cache->entries[bestidx];
        atomicAdd(&entry->votecount, 1u);
        u32 count = entry->votecount;
        float countf = (float)count;
        float alpha = 1.0f + paraboola * countf * countf;
        if (alpha > alphastake) {
            float3 nudge = (1.0f - alpha) * entry->axis + alpha * normalize(radial) * bestweight;
            entry->axis = normalize(nudge);
        }
        return true;
    }
    return false;
}

__device__ bool trycrosswithbestbuffermatch(float3 normal, rdvringbuffer* ring, rdvcache* cache, int* cachecount) {
    if (!ring->full && ring->count == 0) return false;
    float bestDot = cos75;
    int bestidx = -1;
    int limit = ring->full ? ringsize : ring->count;
    for (int i = 0; i < limit; ++i) {
        float dot = fabsf(dot(normal, ring->entries[i]));
        if (dot < bestDot) {
            bestDot = dot;
            bestidx = i;
        }
    }
    if (bestidx != -1 && bestDot < cos75) {
        float3 cross = normalize(cross(normal, ring->entries[bestidx]));
        pushbacknewaxis(cross, 1.0f, cache, cachecount);
        return true;
    }
    return false;
}

__device__ void addtoringbuffer(float3 normal, rdvringbuffer* ring, rdvcache* cache, int* cachecount) {
    if (ring->full) {
        tryreversevote(ring->entries[ring->count], cache, cachecount);
        ring->entries[ring->count] = normal;
        ring->count = (ring->count + 1) & (ringsize - 1);
    } else {
        ring->entries[ring->count] = normal;
        ring->count = (ring->count + 1) & (ringsize - 1);
        if (ring->count == 0) ring->full = true;
    }
}

__device__ bool tryreversevote(float3 radial, rdvcache* cache, int* cachecount) {
    float bestweight = minvoteweight;
    int bestidx = -1;
    for (int i = *cachecount - 1; i >= 0; --i) {
        if (cache->entries[i].voteweight > g_config.rdv.recentaxiscutoff) break;
        float dot = fabsf(dot(radial, cache->entries[i].axis));
        if (dot < cos75) {
            float sinsq = 1.0f - dot * dot;
            float weight = sin32pow(sinsq);
            if (weight > bestweight) {
                bestweight = weight;
                bestidx = i;
            }
        }
    }
    if (bestidx != -1) {
        atomicAdd(&cache->entries[bestidx].voteweight, bestweight);
        int j = bestidx;
        while (j > 0 && cache->entries[j].voteweight > cache->entries[j - 1].voteweight) {
            swapentries(cache->entries[j], cache->entries[j - 1]);
            --j;
        }
        cacheentry* entry = &cache->entries[bestidx];
        atomicAdd(&entry->votecount, 1u);
        u32 count = entry->votecount;
        float countf = (float)count;
        float alpha = 1.0f + paraboola * countf * countf;
        if (alpha > alphastake) {
            float3 nudge = (1.0f - alpha) * entry->axis + alpha * normalize(radial) * bestweight;
            entry->axis = normalize(nudge);
        }
        return true;
    }
    return false;
}

__device__ void processnormal(float3 normal, rdvringbuffer* ring, rdvcache* cache, int* cachecount) {
    if (lengthsq(normal) < 1e-12f) return;
    if (tryforwardvote(normal, cache, cachecount)) return;
    if (trycrosswithbestbuffermatch(normal, ring, cache, cachecount)) return;
    addtoringbuffer(normal, ring, cache, cachecount);
}

__device__ void pushbacknewaxis(float3 newaxis, float initweight, rdvcache* cache, int* cachecount) {
    newaxis = normalize(newaxis);
    int idx = atomicAdd(cachecount, 1);
    if (idx < cachesize) {
        cache->entries[idx].axis = newaxis;
        cache->entries[idx].voteweight = initweight;
        cache->entries[idx].votecount = 1u;
    } else {
        int evictidx = cachesize - 1;
        cache->entries[evictidx].axis = newaxis;
        cache->entries[evictidx].voteweight = initweight;
        cache->entries[evictidx].votecount = 1u;
    }
}

global void k_adaptiveRdvVotererkernel(
    const float3* normals,
    uint32_t normalscount,
    rdvcache* cache,
    int* cachecount,
    rdvringbuffer* rings  // shared per-thread rings
) {
    uint32_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= normalscount) return;

    rdvringbuffer* myring = &rings[threadIdx.x];
    if (threadIdx.x == 0) *cachecount = 3;  // init (primaries in __host__)

    float3 normal = normals[i];
    processnormal(normal, myring, cache, cachecount);
}

void h_adaptiveRdvVotering(
    const DeviceBuffer<float3>& d_normals,
    const DeviceBuffer<uint32_t>& d_normalscount,
    DeviceBuffer<rdvcache>& d_cache,
    DeviceBuffer<rdvringbuffer>& d_rings,
    DeviceBuffer<int>& d_cachecount
) {
    uint32_t h_normalscount;
    d_normalscount.download(&h_normalscount, 1);
    if (h_normalscount == 0) return;

    const uint32_t blocksize = 256;
    const uint32_t gridsize = (h_normalscount + blocksize - 1) / blocksize;
    const size_t sharedsize = blocksize * sizeof(rdvringbuffer);

    k_adaptiveRdvVotererkernel<<<gridsize, blocksize, sharedsize>>>(
        d_normals.data(), h_normalscount, d_cache.data(),
        d_cachecount.data(), d_rings.data()
    );
}
