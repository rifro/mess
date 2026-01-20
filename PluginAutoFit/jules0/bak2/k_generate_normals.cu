#include "includes.cuh"

__device__ bool isCollinear(const float3& p1, const float3& p2, const float3& p3)
{
    float3 side1 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    float3 side2 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
    return dot(cross(side1, side2), cross(side1, side2)) < d_config.ringFilter.minAreaSq;
}

__device__ bool passesPlanarityTest(const float3& p1, const float3& p2, const float3& p3, const float3& testPoint,
                                    float3* n_out)
{
    if(isCollinear(p1, p2, p3)) return false;

    float3 normal = normalize(
        cross(make_float3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z), make_float3(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z)));
    float d           = dot(normal, p1);
    float distanceSq = powf(dot(normal, testPoint) - d, 2);

    bool succeeded = distanceSq < d_config.ringFilter.epsSq;
    if(succeeded && n_out) *n_out = normal;

    return succeeded;
}

__global__ void k_generateNormalsKernel(const float* __restrict__ d_x, const float* __restrict__ d_y,
                                          const float* __restrict__ d_z, u32 N, u8* __restrict__ d_labels,
                                          float3* __restrict__ d_normals, u32* __restrict__ d_normalsCount,
                                          u32 maxNormalen)
{
    u32 i = blockIdx.x * blockDim.x + threadIdx.x;
    if(i >= N) return;

    float3 centerPoint = {d_x[i], d_y[i], d_z[i]};
    float3 qRing[4];
    int    ringCount     = 0;
    int    farPointCount = 0;

    for(int j = i + 1; j < N; ++j)
    {
        if(farPointCount >= 2) break;

        float3 neighborPoint = {d_x[j], d_y[j], d_z[j]};
        float3 diff       = {neighborPoint.x - centerPoint.x, neighborPoint.y - centerPoint.y, neighborPoint.z - centerPoint.z};
        float  distSq     = dot(diff, diff);

        if(distSq <= d_config.ringFilter.r2Sq)
        {
            if(distSq > d_config.ringFilter.r1Sq)
            {
                if(ringCount < 4)
                {
                    qRing[ringCount++] = neighborPoint;
                }
            }
        } else
        {
            farPointCount++;
        }
    }

    if(ringCount < 3)
    {
        d_labels[i] = PointType::Chaos;
        return;
    }

    float3 n_final;
    float3 p1 = qRing[0], p2 = qRing[1], p3 = qRing[2];

    if(passesPlanarityTest(centerPoint, p1, p2, p3, &n_final) || passesPlanarityTest(p1, p2, p3, centerPoint, &n_final))
    {
        u32 Index = atomicAdd(d_normalsCount, 1);
        if(Index < maxNormalen)
        {
            d_normals[Index] = n_final;
        }
    } else
    {
        d_labels[i] = PointType::Chaos;
    }
}

void h_generateNormals(const DeviceBuffer<float>& d_x, const DeviceBuffer<float>& d_y, const DeviceBuffer<float>& d_z,
                        DeviceBuffer<u8>& d_labels, DeviceBuffer<float3>& d_normals,
                        DeviceBuffer<u32>& d_normalsCount)
{
    const u32 N            = d_x.size();
    const u32 maxNormalen = d_normals.size();
    d_normalsCount.memset(0);

    const u32 blockSize = 256;
    const u32 gridSize  = (N + blockSize - 1) / blockSize;

    k_generateNormalsKernel<<<gridSize, blockSize>>>(d_x.data(), d_y.data(), d_z.data(), N, d_labels.data(),
                                                         d_normals.data(), d_normalsCount.data(), maxNormalen);
}
