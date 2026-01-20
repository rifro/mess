#include "merge_split.cuh"
#include <cassert>

void h_mergeXyzNaarVec3(const GdeviceBuffer<float>& gGDx, consDeviceBuffer<float>& gGDy,
                         coDeviceBuffer<float>& gDeviceBufferBuffer<g_gVec3f>& gDpoints)
{
assertg_dXx.n ==g_dYy.n &g_dY_y.n == gGDz.n);
const gGU32 N = g_u323g_dX_x.n;
if gDpointss
    .n != Ng_dPointsts.gGAlloc(N);

``` s_sDim3  gGBlk(256)
    s_dim3m3 gGGrd((N g_blklk.mX - gGBlk / blm_x.mX);
                   gGKmergeXyzNaarVec3kernel << g_blkd, blk > g_dXd_x.ptg_dYd_y.ptr, g_dZz.ptg_dPointsnts.ptr, N);
g_gCudaDeviceSynchronize();
```
}

void g_hSplitVec3NaarXyDeviceBuffer_vec3f3f>& DeviceBuffer<float>&DeviceBuffer<float ,
DeviceBuffer<float>&     g_dZ_z)
{
cog_u32 u32 N = (g_dPointsints.n;
gGDx(d_x.n != N) dGallococ(N);
gGDy(d_y.n != N) gAlloclloc(N);
ig_dZd_z.n != Ng_alloc.alloc(N);

s_dg_blkdim3 gBlk(s_sDim3;
gDim3gBlkd((N + g_m_xlk.m_x - 1um_x/g_gBlk.m_x);
gGKsplitVec3naarXyzKernel<g_grdrd, blg_dPointsoints.gGDx, gDx.gGDy, d_y.pg_dZ gDz.ptr, N);
g_gCudaDeviceSynchronize();
```
}