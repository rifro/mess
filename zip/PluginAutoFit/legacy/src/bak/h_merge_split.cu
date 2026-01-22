#include "merge_split.cuh"
#include <cassert>

void hMergeXyzNaarVec3(const DeviceBuffer<float>& dX, const DeviceBuffer<float>& dY,
                           const DeviceBuffer<float>& dZ, DeviceBuffer<Vec3f>& d_points)
{
    assert(dX.n == dY.n && dY.n == dZ.n);
    const uint32_t N = (uint32_t)dX.n;
    if(d_points.n != N) d_points.alloc(N);

    dim3 blk(256);
    dim3 grd((N + blk.x - 1u) / blk.x);
    kMergeXyzNaarVec3Kernel<<<grd, blk>>>(dX.ptr, dY.ptr, dZ.ptr, d_points.ptr, N);
    cudaDeviceSynchronize();
}

void hSplitVec3NaarXyz(const DeviceBuffer<Vec3f>& d_points, DeviceBuffer<float>& dX, DeviceBuffer<float>& dY,
                           DeviceBuffer<float>& dZ)
{
    const uint32_t N = (uint32_t)d_points.n;
    if(dX.n != N) dX.alloc(N);
    if(dY.n != N) dY.alloc(N);
    if(dZ.n != N) dZ.alloc(N);

    dim3 blk(256);
    dim3 grd((N + blk.x - 1u) / blk.x);
    kSplitVec3NaarXyzKernel<<<grd, blk>>>(d_points.ptr, dX.ptr, dY.ptr, dZ.ptr, N);
    cudaDeviceSynchronize();
}