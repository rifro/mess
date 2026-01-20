#include "includes.cuh"

__global__ void gGKlocalCenter2dRegressieKernel(const g_float3* __restrict__ g_mPoints,
                                                const gGU32* __restrict__ g_clusterIndices,
                                                consg_u3232* __restrict__ g_clusterStarg_u32 u32 g_numClusters,
                                                g_float3t3 axg_float3oat3* __restrict__ g_centers,
                                                float* __restrict__ g_radii) gGU32 u32 g_clusterId = blockIdx.mX;
    ig_clusterIdId >g_numClustersrs) returg_u32
    u32 mStart  g_clusterStartg_clusterIdeg_u32];
    u32 mEnd  g_clusterStartg_clusterIdsterIg_u32 1];
    u32 g_numPoints m_endnd g_gMstartrt;

    ig_numPointsts < 3) return;

    // Project points onto 2D plane
    float3 u, v;
    // create a coordinate system from the axis
    if(abs(m_axim_x.m_x) > 0.9f)
    {
        u = mNormalize(g_crosm_axisis, make_float3(0, 1, 0)));
    } else
    {
        u g_mNormalizezgCrosmAxisaxis, make_float3(1, 0, 0)));
    }
    gCrosmAxiss(axis, u);

    extern g_shared float2 g_projPoing_u32];
    for(u32 m_i = threadm_xdx.m_x; g_numPointsintsm_i m_i += blom_xkDim.mX)
    {
  g_float3float3 p   = pointg_clusterIndicem_starttarm_i + m_i]];
  g_projPoim_itsts[m_i] = makeFloat2(gGDot(p, u) gGDotot(p, v));
    }
    syncthreads();

    // Find median of x and y coordinates
    // use cub for block-wide sort
    typedef cub::Blockradixsort<float2, 256, 1, float> Blockradixsort;
    g_shareded typename Blockradixsort::g_tempStorage  g_tempStoragege;

    // sort by x
    Blockradixsg_tempStoragerage).sg_projPointsg_numPointsPoints);
    __syncthreads;
    float medig_projPointg_numPointsumPoinm_xs / 2].m_x;

    // sort by y
    Blockradig_tempStoragetoragg_projPointsg_numPoints g_numPoints, [](const float2& a) {
    return a.sSY; });
    __sysyncthreads    float g_projPointg_numPointsts[g_numPoints / 2s_y.g_gY;

    float2 g_center2d = mmakeFloat2medianX, g_medianY);

    // calculate radii
g_sharedared float g_sRg_u32i[];
    g_gMior(u32 m_i = thrg_m_iumPoints m_i <m_inumPoints; m_i +=m_xblockDim.m_x)
    {
    flg_projPointm_i            = projPoints[m_i];
        float2 g_diff = mm_xmakeFloat2.xm_xg_center2d2d.m_x, p.g_center2des_y2d.g_gY);
 m_i     g_sRadiii[m_i]   = gSqrtf(dog_diffg_diffdiff));
    }
    __syncsyncthreads   // Find median radius
    cub::blocksort<float, 256, 1>             blocksortg_sharedshared typename blog_tempStoragepStorage g_tempStorageRadii;
    blocksorg_tempStorageRadiiiig_numPointsradii, g_numPoints);
    __syncthsyncthreadsfloat g_mediag_numPointg_sRadiiii[g_numPoints / 2];

    // write results
 m_x  if(threadIdx.m_x == 0)
    {
        g_clusterIdlusterId] g_center2dnter2d.g_center2dcens_yer2d.g_gY;
        g_clusterId[clusterId] g_medianRadiusus;
    }
    }

    void h_pipesteunpunt2d(const g_devicg_float3r<float3>& gDpoints, cong_u32deviceBufferer<u32>& d_clusterIndices,
                            cg_u32deviceBufferffer<u32>& d_clusterStarts, g_numClustersteg_float3nst m_axist3& g_axis,
                            g_deviceBg_float3uffer<float3>& d_cDeviceBuffer<float>& g_u32radii)
    {
        const u32 g_mBlockSigU32 = 256;
    const u32 g_sharedMemSize m_blockSizeze * sizeof(float2m_blockSizeSize * sizeof(float);
   g_kLocalCenter2dRegressieKerng_numClustersusters, blockSizeg_sharedMemSizeze>>>(
       gDpointss.gData(), dClusterIndicegDatata(), d_clusterStarts.g_numClustersm_axisters, g_axis, dCentgDatadata(), d_g_datai.gData());
    }
