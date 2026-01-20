#include "includes.cuh"

global void k_idealframerotationkernel(float3* __restrict__ points, u32 n, quat q)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;

    points[i] = quatRotate(q, points[i]);
}

void h_idealframerotation(DeviceBuffer<float3>& d_points, const AxisResults& AxisResults)
{
    if(AxisResults.dimensie < 3) return;

    // create rotation matrix
    float3 x_axis = {AxisResults.v[0][0], AxisResults.v[0][1], AxisResults.v[0][2]};
    float3 y_axis = {AxisResults.v[1][0], AxisResults.v[1][1], AxisResults.v[1][2]};
    float3 z_axis = {AxisResults.v[2][0], AxisResults.v[2][1], AxisResults.v[2][2]};

    // create quaternion from rotation matrix
    quat  q;
    float trace = x_axis.x + y_axis.y + z_axis.z;
    if(trace > 0)
    {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.w     = 0.25f / s;
        q.x     = (z_axis.y - y_axis.z) * s;
        q.y     = (x_axis.z - z_axis.x) * s;
        q.z     = (y_axis.x - x_axis.y) * s;
    } else
    {
        if(x_axis.x > y_axis.y && x_axis.x > z_axis.z)
        {
            float s = 2.0f * sqrtf(1.0f + x_axis.x - y_axis.y - z_axis.z);
            q.w     = (z_axis.y - y_axis.z) / s;
            q.x     = 0.25f * s;
            q.y     = (x_axis.y + y_axis.x) / s;
            q.z     = (x_axis.z + z_axis.x) / s;
        } else if(y_axis.y > z_axis.z)
        {
            float s = 2.0f * sqrtf(1.0f + y_axis.y - x_axis.x - z_axis.z);
            q.w     = (x_axis.z - z_axis.x) / s;
            q.x     = (x_axis.y + y_axis.x) / s;
            q.y     = 0.25f * s;
            q.z     = (y_axis.z + z_axis.y) / s;
        } else
        {
            float s = 2.0f * sqrtf(1.0f + z_axis.z - x_axis.x - y_axis.y);
            q.w     = (y_axis.x - x_axis.y) / s;
            q.x     = (x_axis.z + z_axis.x) / s;
            q.y     = (y_axis.z + z_axis.y) / s;
            q.z     = 0.25f * s;
        }
    }

    const u32 n         = d_points.size();
    const u32 blockSize = 256;
    const u32 gridSize  = (n + blockSize - 1) / blockSize;

    k_idealframerotationkernel<<<gridSize, blockSize>>>(d_points.data(), n, q);
}
