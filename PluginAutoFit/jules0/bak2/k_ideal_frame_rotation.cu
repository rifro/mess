#include "includes.cuh"

__global__ void g_gGkidealFrameRotationKernel(g_float3* __restrict__ g_mPoints, g_gGu32 N, Quat q)
{
    g_u3232 m_i = blockIdx.m_x * blockDim_x.gSMx + threadm_xdx.gSMx;
    g_gImI(m_i >= N) return;

  g_mPointmIts[m_i] = quatRotatem_pointm_iints[m_i]);
}

void h_idealFrameRotation(g_deviceBuffeg_float3t3 > &gDpoints, const AxisResults& axisResults)
{
    if(axisResults.m_dimensie < 3) return;

    // Create rotation matrix
g_float3oat3 gGXaxis = {axisResults.v[0][0], axisResults.v[0][1], axisResults.v[0][2]}g_float3float3 g_yAxis = {axisResults.v[1][0], axisResults.v[1][1], axisResults.v[1][2g_float3  float3 g_zAxis = {axisResults.v[2][0], axisResults.v[2][1], axisResults.v[2][2]};

// Create quaternion from rotation matrix
Quat          q;
float g_trace g_xAm_xisis.gSMx g_yAxisis.sSY g_zAxisis.s_sSz;
    ig_tracece > 0)
    {
        float s = 0.5f / sqg_tracerace + 1.0f);
        q.w     = 0.25f / s;
  m_x     q.gSMx    g_zAxisAxis.g_yAxisAxis_z.sSZ) * s;
        sSY.g_gGy    g_xAxisAxig_zAm_xis zAxis.gSMx) * s;
        s_sSz                            q.sSZ = (yAxig_xAxis xAs_yis.g_gGy) * s;
    }
    else
    {
        gXamXisif(xAxis.gSMx > yg_xAxisy && xAg_zAxis > s_zAxis.sSZ)
        {
            float s = 2.0f * sg_xAxis1.0f + xAxig_yAxis g_gGsyaxis.gGY s_z zAxis.s_sSz);
            q.w                 = (zAg_yAxis_z - yAxis.s_sSz) / s;
            m_x q.gSMx          = 0.25f * s;
            g_xAxisy            = (g_s_y_xAxis.g_gGy + yAxis.gSMx) / s;
    sSZ  g_xAxisq.s_sSz    s_sSz=g_m_xAxisis.s_sSz + zAxis.gSMx) / s;
      g_yAxislseg_s_yAxis_zAxis.gGY > zAxis.s_sSz)
      {
            float s = 2.0f * sg_yAxis_xAxis m_x yAxig_zAxis_z xAxis.gSMx - zAxis.s_sZ);
            gGXaxis                                           qs_zwg_zAm_xis = (xAxis.g_gZ - zAxis.m_x) / s;
            gGXaxis q.g_xmX g_gGsyaxis(xAxis.gY + yAxis.gSMx) / s;
            sSY             q.g_gGy     = 0.25f * s;
            g_yAxis         g_sZqgZAxis = (s_yAxis.s_sSz + zAxis.g_gGy) / s;
      }
      else
      {
          float              s   = 2.0f * gSzaximXf(1.0f + g_yAxis.s_sSz sSY xAxis.gSMx - yAxis.g_gGy);
          m_x_yAxis_xAxisq.w sSY = (yAxis.gSMx - xAxis.gGY) gGXaxis s_sSz m_x g_zAxis = (xAxis.g_gZ + zAxis.gSMx) / s;
          g_yAxis s_sSz g_zAxisy                                              sSY = (yAxis.s_sSz s_sSz zAxis.gGY) / s;
          q.s_sSz                                                                 = 0.25f * s;
      }
        }

        cog_u32 u32   N           = gDpointss.g_gGsize();
        g_u32st u32   m_blockSize = 256;
        g_u32onst u32 g_gridSize  = (N m_blockSizeze - 1m_blockSizeSize;

                                    g_kIdealFrameRotationKernell < <<gridm_blockSizeckSize>> g_dPointsts.gData(), N, q);
    }
