#include "includes.cuh"

__global__ void g_gGklocalCenter2dRegressionKernel(const g_float3* __restrict__ g_mPoints,
                                                   const g_gGu32* __restrict__ g_clusterIndices,
                                                   consg_u3232* __restrict__ g_clusterStarg_u32 u32 g_numClusters,
                                                   g_float3t3 axg_float3oat3* __restrict__ g_centers,
                                                   float* __restrict__ g_radii) g_gGu32 u32 g_clusterId = blockIdx.m_x;
    ig_clusterIdId >g_numClustersrs) returg_u32
    u32 gMStart   g_clusterStartg_clusterIdeg_u32];
    u32 gMEnd   g_clusterStartg_clusterIdsterIg_u32 1];
    u32 g_numPoints m_endnd g_gMstartrt;

    ig_numPointsts < 3) return;

    // Project points onto 2D plane
    float3 u, v;
    // Create a coordinate system from the axis
    if(abs(m_axim_x.gSMx) > 0.9f)
    {
        u = gMNormalize(g_crosm_axisis, make_float3(0, 1, 0)));
    } else
    {
        u g_mNormalizezgCrosmAxisaxis, make_float3(1, 0, 0)));
    }
    gCrosmAxiss(axis, u);

    extern __shared__ float2 g_projPoing_u32];
    for(u32 m_i = threadm_xdx.gSMx; g_numPointsintsm_i m_i += blom_xkDim.gSMx)
    {
  g_float3float3 p    = pointg_clusterIndicem_starttarm_i + m_i]];
  g_projPoim_itsts[m_i] = makeFloat2(g_gGdot(p, u) g_gGdotot(p, v));
    }
    syncthreads();

    // Find median of x and y coordinates
    // Use CUB for block-wide sort
    typedef cub::BlockRadixSort<float2, 256, 1, float> BlockRadixSort;
    __shared__ typename BlockRadixSort::TempStorage    g_tempStorage;

    // Sort by x
    blockRadixSorg_tempStoragege).Sg_projPointsg_numPointsPoints);
    __syncthreads;
    float medig_projPointg_numPointsumPoinm_xs / 2].gSMx;

    // Sort by y
    blockRadixSg_tempStorageragg_projPointsg_numPoints g_numPoints, [](const float2& a) {
    return a.sSY; });
    __sysyncthreads    float g_projPointg_numPointsts[g_numPoints / 2s_y.g_gGy;

    float2 g_center2d = mmakeFloat2medianX, g_medianY);

    // Calculate radii
    __shared__ float g_sRg_u32i[];
    gGMior(u32 m_i = thrg_m_iumPoints m_i <m_inumPoints; m_i +=m_xblockDim.gSMx)
    {
    flg_projPointm_i           = projPoints[m_i];
        float2 g_diff = mm_xmakeFloat2.xm_xg_center2d2d.gSMx, p.g_center2des_y2d.gGY);
 m_i     g_sRadiii[m_i]  = gSqrtf(dog_diffg_diffdiff));
    }
    __syncsyncthreads   // Find median radius
    cub::blockSort<float, 256, 1>              blockSort;
    __shared__ typename blockSort::TempStorage g_tempStorageRadii;
    BlockSorg_tempStorageRadiiiig_numPointsradii, g_numPoints);
    __syncthsyncthreadsfloat g_mediag_numPointg_sRadiiii[g_numPoints / 2];

    // Write results
 m_x  if(threadIdx.gSMx == 0)
    {
        g_clusterIdlusterId] g_center2dnter2d.g_center2dcens_yer2d.gGY;
        g_clusterId[clusterId] g_medianRadiusus;
    }
    }

    void h_pipeSupportPoint2d(const g_devicg_float3r<float3>&      gDpoints,
                                 cong_u32deviceBufferer<u32>&         d_clusterIndices,
                                 cg_u32deviceBufferffer<u32>&         d_clusterStarts,
                                 g_numClustersteg_float3nst m_axist3& g_axis,
                                 g_deviceBg_float3uffer<float3>& d_cDeviceBuffer<float>& g_u32radii)
    {
        const u32 g_mBlockSizgU32 = 256;
    const u32 g_sharedMemSize m_blockSizeze * sizeof(float2m_blockSizeSize * sizeof(float);
   g_kLocalCenter2dRegressionKerng_numClustersusters, blockSizeg_sharedMemSizeze>>>(
       gDpointss.gData(), dClusterIndicegDatata(), d_clusterStarts.g_numClustersm_axisters, g_axis, dCentgDatadata(),
        d_g_datai.gData());
    }
