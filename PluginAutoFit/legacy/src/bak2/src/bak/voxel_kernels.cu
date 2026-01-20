#include "includes.cuh"


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
        cudaErrorT err = (expr);                                                                                     \
        if(err != cudaSuccess)                                                                                        \
        {                                                                                                              \
            fprintf(stderr, "CUDA error %s at %s:%d\\n", cudaGetErrorString(err), __FILE__, __LINE__);                \
        }                                                                                                              \
    } while(0)

        struct ScopedTimer
        {
            cudaEventT start{}, stop{};
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
                                                u32* __restrict__ outCcIndex, float3 bbMin, float invMeterToMm,
                                                u32 N)
        {
            const u32 i = blockIdx.x * blockDim.x + threadIdx.x;
            if(i >= N) return;
            float3 p = inPos[i];
            float3 q = make_float3((p.x - bbMin.x) * invMeterToMm, (p.y - bbMin.y) * invMeterToMm,
                                   (p.z - bbMin.z) * invMeterToMm);
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

        struct deviceCloud
        {
            thrust::deviceVector<float3>   dPos;
            thrust::deviceVector<u32> dCcIndex;
            thrust::deviceVector<u32> dXi, dYi, dZi;
            thrust::deviceVector<u64> d_keys;
            thrust::deviceVector<u32> dCounts; // voxel counts
            thrust::deviceVector<u32> dStarts; // exclusive scan + sentinel
            thrust::deviceVector<u8>  dFlags;  // downsample marks
        };

        // Initialize, quantize, build keys, sort, histogram+scan
        void build_mortonOrder(deviceCloud& dc, const float3* h_pos, const u32* h_ccIndex, u32 N,
                              float3 bbMin, float meterToMm, ConstsStruct voxel, ConstsStruct sub,
                              GpuRunLog* d_Log /*optional*/)
        {
            ScopedTimer tAll("build_mortonOrder");
            dc.dPos.assign(h_pos, h_pos + N);
            dc.dCcIndex.assign(h_ccIndex, h_ccIndex + N);
            dc.dXi.resize(N);
            dc.dYi.resize(N);
            dc.dZi.resize(N);
            dc.d_keys.resize(N);

            const u32 block = 256;
            const u32 grid  = (N + block - 1) / block;

            {
                ScopedTimer t("normalize+quantize");
                normalizeQuantizeKernel<<<grid, block>>>(
                    thrust::rawPointerCast(dc.dPos.data()), thrust::rawPointerCast(dc.dCcIndex.data()),
                    thrust::rawPointerCast(dc.dXi.data()), thrust::rawPointerCast(dc.dYi.data()),
                    thrust::rawPointerCast(dc.dZi.data()), thrust::rawPointerCast(dc.dCcIndex.data()), bbMin,
                    meterToMm, N);
                CUDA_CHECK(cudaGetLastError());
            }

            {
                ScopedTimer t("buildKeys");
                buildKeysKernel<<<grid, block>>>(thrust::rawPointerCast(dc.dXi.data()),
                                                 thrust::rawPointerCast(dc.dYi.data()),
                                                 thrust::rawPointerCast(dc.dZi.data()),
                                                 thrust::rawPointerCast(dc.d_keys.data()), voxel, sub, N);
                CUDA_CHECK(cudaGetLastError());
            }

            {
                ScopedTimer t("sort_by_key (keys, pos, ccIndex)");
                auto zipped = thrust::make_zip_iterator(thrust::make_tuple(dc.dPos.begin(), dc.dCcIndex.begin()));
                thrust::sortByKey(thrust::__device__, dc.d_keys.begin(), dc.d_keys.end(), zipped);
            }

            // Histogram counts per voxel + exclusive scan to starts (with sentinel)
            const u32 K = voxel.N * voxel.N * voxel.N;
            dc.dCounts.assign(K, 0u);
            {
                ScopedTimer t("voxel histogram");
                histogramVoxelsKernel<<<grid, block>>>(thrust::rawPointerCast(dc.d_keys.data()),
                                                       thrust::rawPointerCast(dc.dCounts.data()), sub, N);
                CUDA_CHECK(cudaGetLastError());
            }

            dc.dStarts.resize(K + 1);
            {
                ScopedTimer t("exclusive_scan counts->starts");
                thrust::exclusiveScan(thrust::__device__, dc.dCounts.begin(), dc.dCounts.end(), dc.dStarts.begin());
                // sentinel
                thrust::copyN(thrust::make__constant__iterator<u32>(N), 1, dc.dStarts.begin() + K);
            }
        }

        // Downsample by stride so that ~maxOut points are selected (return count and indices only)
        u32 downsampleIndices(const deviceCloud& dc, u32 maxOut,
                                   thrust::deviceVector<u32>& d_outIndex, GpuRunLog* d_Log /*optional*/)
        {
            const u32 N = static_cast<u32>(dc.dCcIndex.size());
            if(N == 0 || maxOut == 0) return 0;
            const u32 stride = (N + maxOut - 1) / maxOut;

            d_outIndex.resize(N); // temporary max
            thrust::deviceVector<u8> flags(N, 0);
            const u32                    block = 256;
            const u32                    grid  = (N + block - 1) / block;
            {
                ScopedTimer t("markDownsample");
                markDownsampleKernel<<<grid, block>>>(thrust::rawPointerCast(flags.data()), N, stride);
                CUDA_CHECK(cudaGetLastError());
            }

            // compact ccIndex using flags
            auto ccBegin  = dc.dCcIndex.begin();
            auto zipBegin = thrust::make_zip_iterator(thrust::make_tuple(ccBegin, flags.begin()));
            auto zipEnd = thrust::make_zip_iterator(thrust::make_tuple(dc.dCcIndex.end(), flags.end()));

            auto endIt = thrust::copyIf(thrust::__device__, dc.dCcIndex.begin(), dc.dCcIndex.end(), flags.begin(),
                                         d_outIndex.begin(), [] __device__(u8 f) { return f != 0; });
            const u32 outN = static_cast<u32>(endIt - d_outIndex.begin());
            d_outIndex.resize(outN);

            if(d_Log)
            {
                CUDA_CHECK(cudaMemcpy(&d_Log->downsampleOutCount, &outN, sizeof(u32), cudaMemcpyHostToDevice));
                // mark flag via __host__ memcpy (simple)
                bool ds = true;
                CUDA_CHECK(cudaMemcpy(&d_Log->downsampled, &ds, sizeof(bool), cudaMemcpyHostToDevice));
            }
            return outN;
        }

        // One-shot stage: detect planes (placeholder), maybe rotate, then produce a small index list
        u32 runStageFindFrameAndDownsample(deviceCloud& dc, const ConstsStruct& voxel,
                                                 const ConstsStruct& sub, bool existingFrameIsIdeal,
                                                 u32 maxDownsample, thrust::deviceVector<u32>& d_downIndex,
                                                 GpuRunLog* d_Log /* __device__ ptr */)
        {
            {
                ScopedTimer t("detectPlanes (stub)");
                detectPlanesKernel<<<1, 1>>>(d_Log, existingFrameIsIdeal);
                CUDA_CHECK(cudaGetLastError());
            }
            {
                ScopedTimer    t("applyRotation (stub)");
                const u32 N     = static_cast<u32>(dc.dPos.size());
                const u32 block = 256, grid = (N + block - 1) / block;
                applyRotationKernel<<<grid, block>>>(thrust::rawPointerCast(dc.dPos.data()), N, d_Log);
                CUDA_CHECK(cudaGetLastError());
            }
            u32 outN = 0;
            {
                ScopedTimer t("downsampleIndices");
                outN = downsampleIndices(dc, maxDownsample, d_downIndex, d_Log);
            }
            return outN;
        }

    } // namespace Thrust
} // namespace Bocari
