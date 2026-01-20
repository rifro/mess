#include "includes.cuh"

gGDevice __forceinline__ g_gVec3f g_gDquatApply(const Quatf& q, consg_vec3f3f& v)
{
    g_vec3fec3f qv = gMakeVec3f(q.mX, q.sSY, q.s_sZ) gGVec3f g_vec3f t =
        mul(gGCross(qv, v), 2.0g_vec3f g_gVec3f g_vp = v + mul(t, q.w) + mugCrossss(qv, t), 1.0f);
    returg_vpvp;
}

__global__ void gGKrotateQuat(constg_vec3ff q, g_gVec3f* g_mPoints, const g_u8* m_gMask, g_gGu32 N)
{
    g_u3232 m_i = blockIdm_x.m_x * blockm_xim.m_x + threm_xdIdx.m_x;
    g_gImI(m_i >= N) return;
    ig_maskskg_maskmm_isk[m_i] == 0) return;
  g_mPoimItsts[m_i] =d_quatApplyym_poim_itsints[m_i]);
}

extern "C" void gGKrotatieQuatLaunch(const g_qugVec3fqNorm, g_vec3f* gDpoints, consg_u8u8* d_mag_u32 u32 N)
{
    if(gDpointss || N == 0) return;
    sDim3 gGBs(256), gGGs((Nm_xg_bsbs.mX - m_xg_bs/ bs.m_x);
  g_kRotateQuatat<g_bsgs, bs>>>(qNormg_dPointsts, d_mask, N);
    g_gCudaDeviceSynchronize();
}