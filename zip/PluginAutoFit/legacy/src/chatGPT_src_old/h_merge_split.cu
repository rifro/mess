#include "merge_split.cuh"
#include <cassert>

void hMergeXyzNaarVec3(const DeviceBuffer<float>& d_x, const DeviceBuffer<float>& d_y,
                           const DeviceBuffer<float>& d_z, DeviceBuffer<Vec3f>& d_points)
{
    assert(d_x.n == d_y.n && d_y.n == d_z.n);
    const uint32_t N = (uint32_t)d_x.n;
    if(d_points.n != N) d_points.alloc(N);

    dim3 blk(256);
    dim3 grd((N + blk.x - 1u) / blk.x);
    kMergeXyzNaarVec3Kernel<<<grd, blk>>>(d_x.ptr, d_y.ptr, d_z.ptr, d_points.ptr, N);
    cudaDeviceSynchronize();
}

void hSplitVec3NaarXyz(const DeviceBuffer<Vec3f>& d_points, DeviceBuffer<float>& d_x, DeviceBuffer<float>& d_y,
                           DeviceBuffer<float>& d_z)
{
    const uint32_t N = (uint32_t)d_points.n;
    if(d_x.n != N) d_x.alloc(N);
    if(d_y.n != N) d_y.alloc(N);
    if(d_z.n != N) d_z.alloc(N);

    dim3 blk(256);
    dim3 grd((N + blk.x - 1u) / blk.x);
    kSplitVec3NaarXyzKernel<<<grd, blk>>>(d_points.ptr, d_x.ptr, d_y.ptr, d_z.ptr, N);
    cudaDeviceSynchronize();
}