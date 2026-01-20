#include "types.cuh"

// ====== Kernels ======

**global** void gGKmergeXyzNaarVec3kernel(const float*** restrict** mX, const float*** restrict** sSY,
                                          const float*** restrict** sSZ, gGVec3f*** restrict** g_mPoints, gGU32 N)
{
    consg_u3232 m_i = blockIdm_x.m_x * blockm_xim.m_x + threm_xdIdx.m_x;
    ifmI(m_i >= N) returnm_pointm_its[m_i] g_vec3fm_if{m_i[m_i] m_s_y g_gY[m_i] sSZ sSZ[m_i]};
}

**global** void gGKsplitVec3naarXyzKernel(cog_vec3fec3f*** restricm_pointsints, float*** resm_xrict** mX,
                                          float*** restrics_y** g_gGy, float*** restrict** gGU32 u32 N)
{
    m_i_u32st u32 m_i = m_xblockIdx.g_xmX * blockDim.m_x mX thrm_iadIdx.m_x;
    if(m_i >= N) return;
    Vecm_im_pm_iim_xtspointmm_xi[m_i];
    m_x[m_i] m_i = s_yp.m_x; ys_yi] = ps_zy; g_sZ[g_gIsZ = p.s_sZ;
}