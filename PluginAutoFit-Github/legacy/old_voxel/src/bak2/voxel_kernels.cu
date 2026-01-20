#include <cstdio>
#include <cuda_runtime.h>
#include <thrust/__device___vector.h>
#include <thrust/__host___vector.h>
#include <thrust/copy.h>
#include <thrust/execution_policy.h>
#include <thrust/scan.h>
#include <thrust/sort.h>

#include "../include/consts.cuh"
#include "../include/math_utils.cuh"
#include "../include/morton_utils.cuh"

namespace Bocari
{
    namespace Thrust
    {

        using namespace Bocari::Cuda;

// ───────────────────────────────────────────────────────────
// Error checking & timing helpers
// ───────────────────────────────────────────────────────────
#define CUDA_CHECK(expr)                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        cudaError_t _err = (expr);                                                                                     \
        if(_err != cudaSuccess)                                                                                        \
        {                                                                                                              \
            fprintf(stderr, "CUDA error %s at %s:%d\\n", cudaGetErrorString(_err), __FILE__, __LINE__);                \
        }                                                                                                              \
    } while(0)

        struct ScopedTimer
        {
            cudaEvent_t start{}, stop{};
            float       ms{0.f};
            const char* label{nullptr};
            ScopedTimer(const char* lbl) : label(lbl)
            {
                cudaEventCreate(&start);
                cudaEventCreate(&stop);
                cudaEventRecord(start);
            }
            ~ScopedTimer()
            {
                cudaEventRecord(stop);
                cudaEventSynchronize(stop);
                cudaEventElapsedTime(&ms, start, stop);
                if(label) fprintf(stderr, "[TIMER] %s: %.3f ms\\n", label, ms);
                cudaEventDestroy(start);
                cudaEventDestroy(stop);
            }
        };

        // ───────────────────────────────────────────────────────────
        // Kernels
        // ───────────────────────────────────────────────────────────

        // Normalize to bbMin=(0,0,0), scale by mm, quantize; keep original ccIndex
        __global__ void normalizeQuantizeKernel(const float3* __restrict__ inPos,
                                                const u32* __restrict__ inCcIndex, u32* __restrict__ outXi,
                                                u32* __restrict__ outYi, u32* __restrict__ outZi,
                                                u32* __restrict__ outCcIndex, float3 bbMin, float invMeterToMM,
                                                u32 N)
        {
            const u32 i = blockIdx.x * blockDim.x + threadIdx.x;
            if(i >= N) return;
            float3 p = inPos[i];
            float3 q = make_float3((p.x - bbMin.x) * invMeterToMM, (p.y - bbMin.y) * invMeterToMM,
                                   (p.z - bbMin.z) * invMeterToMM);
            // lrintf-like rounding: add 0.5f and floor
            u32 xi   = static_cast<u32>(floorf(q.x + 0.5f));
            u32 yi   = static_cast<u32>(floorf(q.y + 0.5f));
            u32 zi   = static_cast<u32>(floorf(q.z + 0.5f));
            outXi[i]      = xi;
            outYi[i]      = yi;
            outZi[i]      = zi;
            outCcIndex[i] = inCcIndex[i];
        }

        // Build composite Morton keys from quantized ints
        __global__ void buildKeysKernel(const u32* __restrict__ xi, const u32* __restrict__ yi,
                                        const u32* __restrict__ zi, u64* __restrict__ keys,
                                        ConstsStruct voxel, ConstsStruct sub, u32 N)
        {
            const u32 i = blockIdx.x * blockDim.x + threadIdx.x;
            if(i >= N) return;
            keys[i] = makeCompositeKey(xi[i], yi[i], zi[i], voxel, sub);
        }

        // Histogram per voxel from composite key (only uses high bits = 3*sub.bits shift)
        __global__ void histogramVoxelsKernel(const u64* __restrict__ keys, u32* __restrict__ counts,
                                              ConstsStruct sub, u32 N)
        {
            const u32 i = blockIdx.x * blockDim.x + threadIdx.x;
            if(i >= N) return;
            const u32 voxelKey = static_cast<u32>(keys[i] >> (3u * sub.bits));
            atomicAdd(&counts[voxelKey], 1u);
        }

        // Simple stride-based downsampler: select ~N/stride points; output ccIndex only
        __global__ void markDownsampleKernel(u8* __restrict__ flags, u32 N, u32 stride)
        {
            const u32 i = blockIdx.x * blockDim.x + threadIdx.x;
            if(i >= N) return;
            flags[i] = (i % stride == 0) ? 1 : 0;
        }

        // Placeholder plane detection: set flags/counters based on dummy thresholds
        __global__ void detectPlanesKernel(GpuRunLog* log, bool frameIdealAlready)
        {
            if(threadIdx.x == 0 && blockIdx.x == 0)
            {
                log->planesDetected    = true; // placeholder
                log->planes.topBottom  = 1;    // pretend we saw 1 horizontal
                log->planes.northSouth = 1;    // and one NS
                log->planes.eastWest   = 1;    // and one EW
                log->frameWasIdeal     = frameIdealAlready;
            }
        }

        // Placeholder rotation kernel (no-op if frame already ideal)
        __global__ void applyRotationKernel(float3* __restrict__ pos, u32 N, GpuRunLog* log)
        {
            if(threadIdx.x == 0 && blockIdx.x == 0)
            {
                log->rotationApplied  = !log->frameWasIdeal;
                log->rotationAxis     = make_float3(0, 0, 1);
                log->rotationAngleDeg = 0.f;
            }
        }

        // ───────────────────────────────────────────────────────────
        // __host__-side pipeline entry points
        // ───────────────────────────────────────────────────────────

        struct __device__Cloud
        {
            thrust::__device___vector<float3>   d_pos;
            thrust::__device___vector<u32> d_ccIndex;
            thrust::__device___vector<u32> d_xi, d_yi, d_zi;
            thrust::__device___vector<u64> d_keys;
            thrust::__device___vector<u32> d_counts; // voxel counts
            thrust::__device___vector<u32> d_starts; // exclusive scan + sentinel
            thrust::__device___vector<u8>  d_flags;  // downsample marks
        };

        // Initialize, quantize, build keys, sort, histogram+scan
        void build_mortonOrder(__device__Cloud& dc, const float3* h_pos, const u32* h_ccIndex, u32 N,
                              float3 bbMin, float meterToMM, ConstsStruct voxel, ConstsStruct sub,
                              GpuRunLog* d_log /*optional*/)
        {
            ScopedTimer tAll("build_mortonOrder");
            dc.d_pos.assign(h_pos, h_pos + N);
            dc.d_ccIndex.assign(h_ccIndex, h_ccIndex + N);
            dc.d_xi.resize(N);
            dc.d_yi.resize(N);
            dc.d_zi.resize(N);
            dc.d_keys.resize(N);

            const u32 block = 256;
            const u32 grid  = (N + block - 1) / block;

            {
                ScopedTimer t("normalize+quantize");
                normalizeQuantizeKernel<<<grid, block>>>(
                    thrust::raw_pointer_cast(dc.d_pos.data()), thrust::raw_pointer_cast(dc.d_ccIndex.data()),
                    thrust::raw_pointer_cast(dc.d_xi.data()), thrust::raw_pointer_cast(dc.d_yi.data()),
                    thrust::raw_pointer_cast(dc.d_zi.data()), thrust::raw_pointer_cast(dc.d_ccIndex.data()), bbMin,
                    meterToMM, N);
                CUDA_CHECK(cudaGetLastError());
            }

            {
                ScopedTimer t("buildKeys");
                buildKeysKernel<<<grid, block>>>(thrust::raw_pointer_cast(dc.d_xi.data()),
                                                 thrust::raw_pointer_cast(dc.d_yi.data()),
                                                 thrust::raw_pointer_cast(dc.d_zi.data()),
                                                 thrust::raw_pointer_cast(dc.d_keys.data()), voxel, sub, N);
                CUDA_CHECK(cudaGetLastError());
            }

            {
                ScopedTimer t("sort_by_key (keys, pos, ccIndex)");
                auto zipped = thrust::make_zip_iterator(thrust::make_tuple(dc.d_pos.begin(), dc.d_ccIndex.begin()));
                thrust::sort_by_key(thrust::__device__, dc.d_keys.begin(), dc.d_keys.end(), zipped);
            }

            // Histogram counts per voxel + exclusive scan to starts (with sentinel)
            const u32 K = voxel.N * voxel.N * voxel.N;
            dc.d_counts.assign(K, 0u);
            {
                ScopedTimer t("voxel histogram");
                histogramVoxelsKernel<<<grid, block>>>(thrust::raw_pointer_cast(dc.d_keys.data()),
                                                       thrust::raw_pointer_cast(dc.d_counts.data()), sub, N);
                CUDA_CHECK(cudaGetLastError());
            }

            dc.d_starts.resize(K + 1);
            {
                ScopedTimer t("exclusive_scan counts->starts");
                thrust::exclusive_scan(thrust::__device__, dc.d_counts.begin(), dc.d_counts.end(), dc.d_starts.begin());
                // sentinel
                thrust::copy_n(thrust::make___constant___iterator<u32>(N), 1, dc.d_starts.begin() + K);
            }
        }

        // Downsample by stride so that ~maxOut points are selected (return count and indices only)
        u32 downsampleIndices(const __device__Cloud& dc, u32 maxOut,
                                   thrust::__device___vector<u32>& d_outIndex, GpuRunLog* d_log /*optional*/)
        {
            const u32 N = static_cast<u32>(dc.d_ccIndex.size());
            if(N == 0 || maxOut == 0) return 0;
            const u32 stride = (N + maxOut - 1) / maxOut;

            d_outIndex.resize(N); // temporary max
            thrust::__device___vector<u8> flags(N, 0);
            const u32                     block = 256;
            const u32                     grid  = (N + block - 1) / block;
            {
                ScopedTimer t("markDownsample");
                markDownsampleKernel<<<grid, block>>>(thrust::raw_pointer_cast(flags.data()), N, stride);
                CUDA_CHECK(cudaGetLastError());
            }

            // compact ccIndex using flags
            auto ccBegin  = dc.d_ccIndex.begin();
            auto zipBegin = thrust::make_zip_iterator(thrust::make_tuple(ccBegin, flags.begin()));
            auto zipEnd = thrust::make_zip_iterator(thrust::make_tuple(dc.d_ccIndex.end(), flags.end()));

            auto endIt = thrust::copy_if(thrust::__device__, dc.d_ccIndex.begin(), dc.d_ccIndex.end(), flags.begin(),
                                         d_outIndex.begin(), [] __device__(u8 f) { return f != 0; });
            const u32 outN = static_cast<u32>(endIt - d_outIndex.begin());
            d_outIndex.resize(outN);

            if(d_log)
            {
                CUDA_CHECK(cudaMemcpy(&d_log->downsampleOutCount, &outN, sizeof(u32), cudaMemcpyHostToDevice));
                // mark flag via __host__ memcpy (simple)
                bool ds = true;
                CUDA_CHECK(cudaMemcpy(&d_log->downsampled, &ds, sizeof(bool), cudaMemcpyHostToDevice));
            }
            return outN;
        }

        // One-shot stage: detect planes (placeholder), maybe rotate, then produce a small index list
        u32 runStage_FindFrameAndDownsample(__device__Cloud& dc, const ConstsStruct& voxel,
                                                 const ConstsStruct& sub, bool existingFrameIsIdeal,
                                                 u32 maxDownsample, thrust::__device___vector<u32>& d_downIndex,
                                                 GpuRunLog* d_log /* __device__ ptr */)
        {
            {
                ScopedTimer t("detectPlanes (stub)");
                detectPlanesKernel<<<1, 1>>>(d_log, existingFrameIsIdeal);
                CUDA_CHECK(cudaGetLastError());
            }
            {
                ScopedTimer    t("applyRotation (stub)");
                const u32 N     = static_cast<u32>(dc.d_pos.size());
                const u32 block = 256, grid = (N + block - 1) / block;
                applyRotationKernel<<<grid, block>>>(thrust::raw_pointer_cast(dc.d_pos.data()), N, d_log);
                CUDA_CHECK(cudaGetLastError());
            }
            u32 outN = 0;
            {
                ScopedTimer t("downsampleIndices");
                outN = downsampleIndices(dc, maxDownsample, d_downIndex, d_log);
            }
            return outN;
        }

    } // namespace Thrust
} // namespace Bocari
