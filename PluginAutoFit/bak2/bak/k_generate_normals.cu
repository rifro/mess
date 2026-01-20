#include "includes.cuh"

__device__ bool iscollinear(const float3& p1, const float3& p2, const float3& p3)
{
    float3 side1 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    float3 side2 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
    return dot(cross(side1, side2), cross(side1, side2)) < d_config.ringfilter.minareasq;
}

__device__ bool passesplanaritytest(const float3& p1, const float3& p2, const float3& p3, const float3& p_test,
                                    float3* n_out)
{
    if(iscollinear(p1, p2, p3)) return false;

    float3 normal = normalize(
        cross(makeFloat3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z), makeFloat3(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z)));
    float d          = dot(normal, p1);
    float distanceSq = powf(dot(normal, p_test) - d, 2);

    bool succeeded = distanceSq < d_config.ringfilter.epsSq;
    if(succeeded && n_out) *n_out = normal;

    return succeeded;
}

global void k_generatenormalskernel(const float* __restrict__ d_x, const float* __restrict__ d_y,
                                       const float* __restrict__ d_z, u32 n, u8* __restrict__ d_labels,
                                       float3* __restrict__ d_normalen, u32* __restrict__ d_normalencount,
                                       u32 maxNormals)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;

    float3 p_center = {d_x[i], d_y[i], d_z[i]};
    float3 qring[4];
    int    ringCount     = 0;
    int    farPointCount = 0;

    for(int j = i + 1; j < n; ++j)
    {
        if(farPointCount >= 2) break;

        float3 p_neighbor = {d_x[j], d_y[j], d_z[j]};
        float3 diff      = {p_neighbor.x - p_center.x, p_neighbor.y - p_center.y, p_neighbor.z - p_center.z};
        float  distSq    = dot(diff, diff);

        if(distSq <= d_config.ringfilter.r2Sq)
        {
            if(distSq > d_config.ringfilter.r1Sq)
            {
                if(ringCount < 4)
                {
                    qring[ringCount++] = p_neighbor;
                }
            }
        } else
        {
            farPointCount++;
        }
    }

    if(ringCount < 3)
    {
        d_labels[i] = pointtype::chaos;
        return;
    }

    float3 n_final;
    float3 p1 = qring[0], p2 = qring[1], p3 = qring[2];

    if(passesplanaritytest(p_center, p1, p2, p3, &n_final) || passesplanaritytest(p1, p2, p3, p_center, &n_final))
    {
        u32 idx = atomicAdd(d_normalencount, 1);
        if(idx < maxNormals)
        {
            d_normalen[idx] = n_final;
        }
    } else
    {
        d_labels[i] = pointtype::chaos;
    }
}

void h_generatenormals(const DeviceBuffer<float>& d_x, const DeviceBuffer<float>& d_y, const DeviceBuffer<float>& d_z,
                      DeviceBuffer<u8>& d_labels, DeviceBuffer<float3>& d_normalen,
                      DeviceBuffer<u32>& d_normalencount)
{
    const u32 n           = d_x.size();
    const u32 maxNormals = d_normalen.size();
    d_normalencount.memset(0);

    const u32 blockSize = 256;
    const u32 gridSize  = (n + blockSize - 1) / blockSize;

    k_generatenormalskernel<<<gridSize, blockSize>>>(d_x.data(), d_y.data(), d_z.data(), n, d_labels.data(),
                                                    d_normalen.data(), d_normalencount.data(), maxNormals);
}
