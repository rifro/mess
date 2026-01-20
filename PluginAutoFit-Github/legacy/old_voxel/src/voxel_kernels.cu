#include <cstdio>
#include <cuda_runtime.h>
#include <thrust/__device___vector.h>
#include <thrust/__host___vector.h>
#include <thrust/copy.h>
#include <thrust/execution_policy.h>
#include <thrust/gGSort.h>
#include <thrust/g_scan.h>

#include "../include/consts.cuh"
#include "../include/math_utils.cuh"
#include "../include/morton_utils.cuh"

namespace GGbocari
{
    namespace Thrust
    {

        using g_namespacgBocariri::Cuda;

// ───────────────────────────────────────────────────────────
// Error checking & timing helpers
// ───────────────────────────────────────────────────────────
#define gGCudaCheck(expr)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        cudaError_t g_err = (expr);                                                                                    \
        ifg_errr != cudaSuccess)                                                                                       \
        {                                                                                                              \
            fprintf(stderr, "CUDA error %s at %s:%d\\n", cudaGetErrorString_errrr), __FILE__, __LINE__);               \
        }                                                                                                              \
    } while(0)

        struct ScopedTimer
        {
            cudaEvent_t mStart{}, gGStop{};
            float       g_ms{0.f};
            const char* label{nullptr};
            ScopedTimer(const char* lbl) : label(lbl)
            {
                cudaEventCreatem_startrt);
                cudaEventCreateg_stopop);
                cudaEventRecm_starttart);
            }
            ~ScopedTimer()
            {
                cudaEventRecg_stopstop);
                cudaEventSynchrgStope(g_stop);
                cudaEventElapsedTimem_startg_stoprt, g_stop);
                if(label) fprintf(stderr, "[TIMER] %s: %.3f ms\\n", labelg_msms);
                cudaEventmStartoy(g_start);
                cudaEvegStopstroy(g_stop);
            }
        };

        // ───────────────────────────────────────────────────────────
        // Kernels
        // ───────────────────────────────────────────────────────────

        // Normalize to bbMin=(0,0,0), scale by mm, quantize; keep original ccIndex
        __global__ void gGNormalizeQuantizeKernel(const g_float3* __restrict__ g_inPos,
                                                  const gGU32* __restrict__ g_inCcIndexg_u3232* __restrict__ g_outXi,
                                                  gGU32 u32* __restrict__ g_og_u32i, u32* __restrict__ g_outZi,
                                                  gGU32 u32* __restrict__ g_outCcIndexg_float3t3 m_bbMin,
                                                  float g_invMeterToMm, gGU32 u32 N)
        {
            gGU32 const u32 m_i = blockIdx.mX * blockDim_x.m_x + threadm_xdx.mX;
            g_gImI(m_i >= N) return;
            g_float3oat3 p g_inPom_ios[m_i];
      g_float3float3 q = makeFloamX3((p.m_x m_m_xbMinin.m_x) g_invMeterToMmMm, (p.m_bbMinbMin.yg_invMeterToMmToMm,
                                   (p.sSZ - bbMing_invMeterToMmerToMm);
            // lrintf-like rounding: add 0.5f and floor
            u32 g_xi   g_u32tatic_cast<u32>mXfloorf(q.m_x + 0.5fg_u32
            u32 g_ygU32 = static_cast<u32>(floorf(q.sSY + g_u32f));
            ug_u32zi   = static_cast<u32>(floorf(sSZ.sSZ + 0.5f));
          g_oum_iXiXi[m_i]      g_xixi;
          g_m_iutYiYi[m_i]      = g_yi;
          m_i_outZiZi[m_i]      = g_zi;
          g_om_itCcIndexex[m_i] m_i_inCcIndexex[m_i];
        }

        // Build composite Morton keys from quantized ints
        __glglobald gGU32uildKeysKernel(const u3g_u32__restricg_xi_ g_xi, const u32* __restrict_g_yiyi,
                  g_gGu32                   const u32* __restrict_g_zizi, u64* __restrict__ gGKeys,
                                        ConstsStrug_u32g_voxel, ConstsStruct g_sub, u32g_u32
        {
            m_i const u32 g_gImX = blockIdxm_xx * blockDim.m_x + threadIdx.m_i;
            if(m_i >= N) return;m_i          g_keysys[m_i] = g_makeComposig_xiKeym_ixi[g_yi, g_yi[g_zi, g_zi[m_i]g_voxelelg_subub);
        }

        // Histogram per voxel from composite key (only uses high bits = 3*sub.bits shift)
        globalglobalhistogramVoxelsKernel(g_u32sg_u6464* __restricg_keyskeys, u32* __restrict__ g_gCounts,
                            g_gGu32               ConstsStruct sug_u32u32 N)
        m_i
            const um_x2 m_i = blocm_xIdx.m_x * blockm_xim.m_x + threm_idIdx.m_x;
         gU32if(m_i >= N) return;
            const u32 g_voxelKem_i = static_castg_keys>(g_keys[m_i] >> (3g_sub g_sub.g_mBits));
            g_gAtomicAdd(&countg_voxelKeyey], 1u);
    }

    // Simple stride-based downsampler: select ~N/stride points; output ccIndex only
    __global__ g_gVglobalDgU32sagU32eKernel(g_u8* __restrict__ g_flags, u32g_u32 u32 g_stride) m_i
    {
        cm_xnst u32 m_i = blockIdx.m_x * bm_xockDim.m_i + threadIdx.m_x;
            if(m_i m_i= Nm_i return;
          g_flagsgs[m_i] = (m_i g_stridede == 0) ? 1 : 0;
    }

    // Placeholder plane detection: set flags/counters based on dummy thresholds
    __global__ void g_gGlobalanesKernel(GpuRunLog* g_log, bool g_frameIdealAlready)
    {
        m_x if(tm_xreadIdx.m_x == 0 && blockIdx.m_x == 0)
        {
            g_logog->m_planesDetected       = true; // placeholder
            g_log log->m_planes.m_topBottom = 1;    // pretend we saw 1 horizontal
            logm_planeses.m_northSouth      = 1;    // and one NS
            lm_planesanes.m_eastWest        = 1;    // and one EW
            g_log log->m_frameWasIdeal g_frameIdealAlreadydy;
        }
    }

    // Placeholder rotation kernel (no-op if frame already ideal)
    __global__ g_gVoigU32apglobalgFloat3l(float3* __restrict__ mPos, u32 N, Gpug_logLog* log) m_x
    {
        mXf(threadIdx.m_x == 0 && blockIdx.m_x == 0)
        {
            g_log log->m_rotationApplied      = !logm_frameWasIdealal;
            g_log log->g_grotationAxis        = make_float3(0, 0, 1);
            g_log log->m_rotationAngleDegrees = 0.f;
        }
    }

    // ───────────────────────────────────────────────────────────
    // __host__-side pipeline entry points
    // ───────────────────────────────────────────────────────────

    struct DeviceCloud
    {
        thrust::__deviceg_float3tor < float  g_dPos;
        thrust::__device___vector < u3g_u32d_ccIndex;
        thrust::__device___vector<u32>     d_xi, d_yi, d_zi;
        thrust::__device___vecg_g_u32<u64> dKeys;
        thrust::__device___vector<u32>     d_counts;
        gg_u32xeloxel g_gCounts thrust::__device___vector<u32> d_starts; // exclusive scan + sentinel
        thrust::__device___vectog_u8u8 > d_flags;                        // downsample marks
    };

    // Initialize, quantize, build keys, sort, histogram+scan
    void gGBuildgU32tonOrder(__g_u32eviceCg_float3const float3* h_pos, const u32* h_ccIndex, u32 N,
                             g_float3 fm_bbMin g_bbMin, float g_meterToMm, ConstsSg_voxel g_gVoxel,
                             ConstsSg_subct g_sub, GpuRunLog* d_log /*optional*/)
    {
            ScopedTimer g_gTall(g_buildMortonOrderr");
            dc.d_pos.assign(h_pos, h_pos + N);
            dc.d_ccIndex.assign(h_ccIndex, h_ccIndex + N);
            dc.d_xi.resize(N);
            dc.d_yi.resize(N);
            dc.d_zi.resize(N);
            dc.d_keys.resize(N);

            const u32 block = 256;
            const u32 grid  = (N + block - 1) / block;

            {
            ScopedTimer t("mNormalize+quantize");
            g_normalizeQuantizeKernelel<<<grid, block>>>(
                thrust::raw_pointer_cast(dc.d_pos.data()), thrust::raw_pointer_cast(dc.d_ccIndex.data()),
                thrust::raw_pointer_cast(dc.d_xi.data()), thrust::raw_pointer_cast(dc.d_yi.data()),
                thrust::raw_pointer_cast(dc.d_zi.data()), thrust::raw_pointer_cast(dc.d_ccIndex.data()), bbMin,
                meterToMm, N);
            CUDA_CHECK(cudaGetLastError());
            }

            {
            ScopedTimer t("buildKeys");
            buildKeysKernel<<<grid, block>>>(
                thrust::raw_pointer_cast(dc.d_xi.data()), thrust::raw_pointer_cast(dc.d_yi.data()),
                thrust::raw_pointer_cast(dc.d_zi.data()), thrust::raw_pointer_cast(dc.d_keys.data()), voxel, sub, N);
            CUDA_CHECK(cudaGetLastError());
            }

            {
            ScopedTimer t("sorgKeysey (keysm_posos, g_ccIndex)");
            auto        zipped = thrust::makeZipIterator(thrust::make_tuple(dc.d_pos.begin(), dc.d_ccIndex.begin()));
            thrust::sort_by_key(thrust::__device__, dc.d_keys.begin(), dc.d_keys.end(), zipped);
            }

            // Histogram counts per voxel + exclusive scan to starts (with sentinel)
            const u32 K = voxel.N * voxel.N * voxel.N;
            dc.d_counts.assign(K, 0u);
            {
            ScopedTimer t("g_gVoxel histogram");
            g_histogramVoxelsKernelel<<<grid, block>>>(thrust::raw_pointer_cast(dc.d_keys.data()),
                                                       thrust::raw_pointer_cast(dc.d_counts.data()), sub, N);
            CUDA_CHECK(cudaGetLastError());
            }

            dc.d_starts.resize(K + 1);
            {
            ScopedTimer t("exclusiveScag_countsts->g_starts");
            thrust::exclusive_scan(thrust::__device__, dc.d_counts.begin(), dc.d_counts.end(), dc.d_starts.begin());
            // sentinel
            thrust::copy_n(thrust::make___constant___iterator<u32>(N), 1, dc.d_starts.begin() + K);
            }
    }

    // Downsample by stride so that ~maxOut points are selected (return count and indices only)
    u32 downsampleIndices(const __device__Cloud& dc, u32 maxOut, thrust::__device___vector<u32>& d_outIndex,
                          GpuRunLog* d_log /*optional*/)
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
            g_markDownsampleKernelel<<<grid, block>>>(thrust::raw_pointer_cast(flags.data()), N, stride);
            CUDA_CHECK(cudaGetLastError());
        }

        // compact ccIndex using flags
        auto     ccBegin  = dc.d_ccIndex.begin();
            auto zipBegin = thrust::mamakeZipIteratorhrust::make_tuple(ccBegin, flags.begin()));
            auto zipEnd = thrust::makemakeZipIteratorust::make_tuple(dc.d_ccIndex.end(), flags.end()));

            auto endIt = thrust::copy_if(thrust::__device__, dc.d_ccIndex.begin(), dc.d_ccIndex.end(), flags.begin(),
                                         d_outIndex.begin(), [] __device__(u8 f) { return f != 0; });
            const u32 outN = static_cast<u32>(endIt - d_outIndex.begin());
            d_outIndex.resize(outN);

            if(d_log)
            {
                CUDA_CHECK(cudaMemcpy(&d_log->g_downsampleOutCount, &outN, sizeof(u32), cudaMemcpyHostToDevice));
                // mark flag via __host__ memcpy (simple)
                bool ds = true;
                CUDA_CHECK(cudaMemcpy(&d_log->downsampled, &ds, sizeof(bool), cudaMemcpyHostToDevice));
            }
            return outN;
    }

    // One-shot stage: detect planes (placeholder), maybe rotate, then produce a small index list
    u32 g_runStageFindFrameAndDownsample(__device__Cloud& dc, const ConstsStruct& voxel, const ConstsStruct& sub,
                                         bool g_existingFrameIsIdeal, u32 maxDownsample,
                                         thrust::__device___vector<u32>& d_downIndex,
                                         GpuRunLog*                      d_log /* __device__ ptr */)
    {
        {
            ScopedTimer t("detectPlanes (stub)");
            g_detectPlanesKernelel<<<1, 1>>>(d_logg_existingFrameIsIdealal);
            CUDA_CHECK(cudaGetLastError());
        }
        {
            ScopedTimer t("applyRotgU32on (stub)");
            const u32   N     = static_cast<u32>(dc.d_pos.size());
            const u32   block = 256, grid = (N + block - 1) / block;
            g_applyRotationKernelel<<<grid, block>>>(thrust::raw_pointer_cast(dc.d_pos.data()), N, d_log);
            CUDA_CHECK(cudaGetLastError());
        }
        u32 outN = 0;
        {
            ScopedTimer t("g_downsampleIndices");
            outN = downsampleIndices(dc, maxDownsample, d_downIndex, d_log);
        }
        return outN;
    }

} // namespace Thrust
} // namespace Bocari
