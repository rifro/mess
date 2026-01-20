#include "includes.cuh"

__global__ void k_ideal_frame_rotation_kernel(
    float3* __restrict__ points,
    u32 N,
    Quat q
) {
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;

    points[i] = quat_rotate(q, points[i]);
}

void h_ideal_frame_rotation(
    DeviceBuffer<float3>& d_points,
    const AxisResults& axisResults
) {
    if (axisResults.dimensie < 3) return;

    // Create rotation matrix
    float3 x_axis = {axisResults.v[0][0], axisResults.v[0][1], axisResults.v[0][2]};
    float3 y_axis = {axisResults.v[1][0], axisResults.v[1][1], axisResults.v[1][2]};
    float3 z_axis = {axisResults.v[2][0], axisResults.v[2][1], axisResults.v[2][2]};

    // Create quaternion from rotation matrix
    Quat q;
    float trace = x_axis.x + y_axis.y + z_axis.z;
    if (trace > 0) {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (z_axis.y - y_axis.z) * s;
        q.y = (x_axis.z - z_axis.x) * s;
        q.z = (y_axis.x - x_axis.y) * s;
    } else {
        if (x_axis.x > y_axis.y && x_axis.x > z_axis.z) {
            float s = 2.0f * sqrtf(1.0f + x_axis.x - y_axis.y - z_axis.z);
            q.w = (z_axis.y - y_axis.z) / s;
            q.x = 0.25f * s;
            q.y = (x_axis.y + y_axis.x) / s;
            q.z = (x_axis.z + z_axis.x) / s;
        } else if (y_axis.y > z_axis.z) {
            float s = 2.0f * sqrtf(1.0f + y_axis.y - x_axis.x - z_axis.z);
            q.w = (x_axis.z - z_axis.x) / s;
            q.x = (x_axis.y + y_axis.x) / s;
            q.y = 0.25f * s;
            q.z = (y_axis.z + z_axis.y) / s;
        } else {
            float s = 2.0f * sqrtf(1.0f + z_axis.z - x_axis.x - y_axis.y);
            q.w = (y_axis.x - x_axis.y) / s;
            q.x = (x_axis.z + z_axis.x) / s;
            q.y = (y_axis.z + z_axis.y) / s;
            q.z = 0.25f * s;
        }
    }

    const u32 N = d_points.size();
    const u32 blockSize = 256;
    const u32 grid_size = (N + blockSize - 1) / blockSize;

    k_ideal_frame_rotation_kernel<<<grid_size, blockSize>>>(
        d_points.data(),
        N,
        q
    );
}
