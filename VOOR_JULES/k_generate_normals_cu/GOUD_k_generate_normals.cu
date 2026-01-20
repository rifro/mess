#include "Config.h"
#include "k_generatenormals.cuh"
#include "math_utils.cuh"
#include "pointTypes.h"
#include <cuda_runtime.h>

__device__ bool iscollinear(const float3& p1, const float3& p2, const float3& p3)
{
    float3 side1 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    float3 side2 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
    return dot(cross(side1, side2), cross(side1, side2)) < d_config.ringfilter.minAreaSq;
}

__device__ bool passesplanaritytest(const float3& p1, const float3& p2, const float3& p3, const float3& p_test,
                                    float3* n_out)
{
    if(iscollinear(p1, p2, p3)) return false;

    float3 normal = normalize(
        cross(makeFloat3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z), makeFloat3(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z)));
    float d          = dot(normal, p1);
    float distancesq = powf(dot(normal, p_test) - d, 2);

    bool succeeded = distancesq < d_config.ringfilter.epsSq;
    if(succeeded && n_out) *n_out = normal;

    return succeeded;
}

global void k_generatenormalskernel(const float* __restrict__ d_x, const float* __restrict__ d_y,
                                       const float* __restrict__ d_z, uint32_t n, uint8_t* __restrict__ d_labels,
                                       float3* __restrict__ d_normalen, uint32_t* __restrict__ d_normalsCount,
                                       uint32_t maxnormalen)
{
    uint32_t i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= n) return;

    float3 p_center = {d_x[i], d_y[i], d_z[i]};
    float3 qring[4];
    int    ringcount     = 0;
    int    farpointcount = 0;

    for(int j = i + 1; j < n; ++j)
    {
        if(farpointcount >= 2) break;

        float3 p_neighbor = {d_x[j], d_y[j], d_z[j]};
        float3 diff      = {p_neighbor.x - p_center.x, p_neighbor.y - p_center.y, p_neighbor.z - p_center.z};
        float  distsq    = dot(diff, diff);

        if(distsq <= d_config.ringfilter.r2Sq)
        {
            if(distsq > d_config.ringfilter.r1Sq)
            {
                if(ringcount < 4)
                {
                    qring[ringcount++] = p_neighbor;
                }
            }
        } else
        {
            farpointcount++;
        }
    }

    if(ringcount < 3)
    {
        d_labels[i] = pointtype::chaos;
        return;
    }

    float3 n_final;
    float3 p1 = qring[0], p2 = qring[1], p3 = qring[2];

    if(passesplanaritytest(p_center, p1, p2, p3, &n_final) || passesplanaritytest(p1, p2, p3, p_center, &n_final))
    {
        uint32_t Index = atomicAdd(d_normalsCount, 1);
        if(Index < maxnormalen)
        {
            d_normalen[Index] = n_final;
        }
    } else
    {
        d_labels[i] = pointtype::chaos;
    }
}

void h_generatenormals(const DeviceBuffer<float>& d_x, const DeviceBuffer<float>& d_y, const DeviceBuffer<float>& d_z,
                      DeviceBuffer<uint8_t>& d_labels, DeviceBuffer<float3>& d_normalen,
                      DeviceBuffer<uint32_t>& d_normalsCount)
{
    const uint32_t n           = d_x.size();
    const uint32_t maxnormalen = d_normalen.size();
    d_normalsCount.memset(0);

    const uint32_t blockSize = 256;
    const uint32_t gridsize  = (n + blockSize - 1) / blockSize;

    k_generatenormalskernel<<<gridsize, blockSize>>>(d_x.data(), d_y.data(), d_z.data(), n, d_labels.data(),
                                                    d_normalen.data(), d_normalsCount.data(), maxnormalen);
}
