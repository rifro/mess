#include "autofit_impl.h"
#include "includes.h"
#include "config.h"
#include "k_generate_normals.cuh"
#include "k_adaptive_rdv_voter.cuh"
#include "h_reduce_consensus.cuh"
#include "k_ideal_frame_rotation.cuh"
#include "device_buffer.cuh"
#include <vector>
#include <iostream>
#include <cuda_runtime.h>
#include <algorithm>
#include <map>

using namespace CCCoreLib;

AutofitImpl::AutofitImpl() {}

void AutofitImpl::performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud)
{
    m_app           = app;
    m_selectedCloud = selectedCloud;
    doProcessing();
}

bool AutofitImpl::getSafeVramLimit(size_t& safeVramLimit) {
    size_t free_mem, total_mem;
    cudaError_t status = cudaMemGetInfo(&free_mem, &total_mem);
    if (status != cudaSuccess) {
        std::cerr << "Error getting CUDA memory info: " << cudaGetErrorString(status) << std::endl;
        return false;
    }

    int device;
    cudaGetDevice(&device);
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, device);
    std::string gpu_name = prop.name;

    std::map<std::string, size_t> vramLookup = {
        {"NVIDIA GeForce RTX 2060", 3.5 * 1024 * 1024 * 1024},
        {"NVIDIA GeForce RTX 4070", 10.0 * 1024 * 1024 * 1024}
    };

    if (vramLookup.count(gpu_name)) {
        safeVramLimit = vramLookup[gpu_name];
    } else {
        safeVramLimit = free_mem - (2.5 * 1024 * 1024 * 1024); // Default reserve
    }

    return true;
}


void AutofitImpl::recursiveSplit(std::vector<AxisChunk>& chunks, int depth) {
    if (depth > 5) return;

    std::vector<AxisChunk> new_chunks;
    bool needs_split = false;

    for (size_t i = 0; i < chunks.size(); i += 3) {
        std::vector<AxisChunk> subChunks = {chunks[i], chunks[i+1], chunks[i+2]};
        
        if (subChunks[0].coords.size() > 100000) {
             needs_split = true;
             size_t mid = subChunks[0].coords.size() / 2;
             new_chunks.push_back({{subChunks[0].coords.subspan(0, mid)}, subChunks[0].originalAxisName});
             new_chunks.push_back({{subChunks[1].coords.subspan(0, mid)}, subChunks[1].originalAxisName});
             new_chunks.push_back({{subChunks[2].coords.subspan(0, mid)}, subChunks[2].originalAxisName});
             
             new_chunks.push_back({{subChunks[0].coords.subspan(mid)}, subChunks[0].originalAxisName});
             new_chunks.push_back({{subChunks[1].coords.subspan(mid)}, subChunks[1].originalAxisName});
             new_chunks.push_back({{subChunks[2].coords.subspan(mid)}, subChunks[2].originalAxisName});
        } else {
            new_chunks.insert(new_chunks.end(), subChunks.begin(), subChunks.end());
        }
    }

    chunks = new_chunks;

    if (needs_split) {
        recursiveSplit(chunks, depth + 1);
    }
}


void AutofitImpl::doProcessing()
{
    if (!m_selectedCloud || m_selectedCloud->size() == 0)
    {
        std::cout << "Geen point cloud of lege cloud" << std::endl;
        return;
    }

    RdvConfig h_config = loadConfig();
    cudaMemcpyToSymbol(d_config, &h_config, sizeof(RdvConfig));

    size_t safeVramLimit;
    if (!getSafeVramLimit(safeVramLimit)) return;

    const size_t N = m_selectedCloud->size();
    const size_t page_size_points = safeVramLimit / (sizeof(float) * 3);

    std::vector<float> h_x(N), h_y(N), h_z(N);
    for (size_t i = 0; i < N; ++i) {
        const CCVector3* p = m_selectedCloud->getPoint(i);
        h_x[i] = p->x; h_y[i] = p->y; h_z[i] = p->z;
    }

    std::vector<std::vector<OTAsAccu>> allPageSlots;
    AxisResults final_result;

    for (size_t page_offset = 0; page_offset < N; page_offset += page_size_points) {
        size_t current_page_size = std::min(page_size_points, N - page_offset);

        std::vector<AxisChunk> chunks = {
            {{h_x.data() + page_offset, current_page_size}, 'X'},
            {{h_y.data() + page_offset, current_page_size}, 'Y'},
            {{h_z.data() + page_offset, current_page_size}, 'Z'}
        };

        recursiveSplit(chunks, 0);

        for (size_t i = 0; i < chunks.size(); i+=3) {
            DeviceBuffer<float> d_x(chunks[i].coords.size());
            DeviceBuffer<float> d_y(chunks[i+1].coords.size());
            DeviceBuffer<float> d_z(chunks[i+2].coords.size());
            d_x.upload(chunks[i].coords.data(), chunks[i].coords.size());
            d_y.upload(chunks[i+1].coords.data(), chunks[i+1].coords.size());
            d_z.upload(chunks[i+2].coords.data(), chunks[i+2].coords.size());

            DeviceBuffer<u8> d_labels(d_x.size());
            DeviceBuffer<float3> d_normalen(d_x.size());
            DeviceBuffer<u32> d_normalsCount(1);
            h_generate_normals(d_x, d_y, d_z, d_labels, d_normalen, d_normalsCount);
            
            DeviceBuffer<OTAsAccu> d_slots(h_config.global.cache_size_k);
            DeviceBuffer<float3> d_ringBuffer(h_config.global.ringBuffer_n);
            DeviceBuffer<u32> d_ringBuffer_pos(1);
            h_adaptive_rdv_voting(d_normalen, d_normalsCount, d_slots, d_ringBuffer, d_ringBuffer_pos);
            
            std::vector<OTAsAccu> h_slots(h_config.global.cache_size_k);
            d_slots.download(h_slots.data(), h_config.global.cache_size_k);
            allPageSlots.push_back(h_slots);
        }
    }
    
    h_reduce_global_consensus(allPageSlots, final_result);
    
    DeviceBuffer<float3> d_allPoints(N);
    std::vector<float3> h_allPoints(N);
    for(size_t i=0; i<N; ++i) h_allPoints[i] = make_float3(h_x[i], h_y[i], h_z[i]);
    d_allPoints.upload(h_allPoints.data(), N);

    h_ideal_frame_rotation(d_allPoints, final_result);

    d_allPoints.download(h_allPoints.data(), N);
    for (size_t i = 0; i < N; ++i) {
        CCVector3* p = m_selectedCloud->getPointMutable(i);
        p->x = h_allPoints[i].x; p->y = h_allPoints[i].y; p->z = h_allPoints[i].z;
    }

    m_selectedCloud->refreshDisplay();
    m_app->refreshAll();

    std::cout << "Processing complete." << std::endl;
}
