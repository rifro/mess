// updated __host__-flow with 0.5GB chunks and tile-parallelism
// ... (jules' recursivesplit + safevramlimit)
void AutofitImpl::doprocessing() {
    // ... upload h_x/h_y/hz to d_x/d_y/d_z, merge to d_points (morton-sorted) //RR!!!

    const size_t hostChunkSizeBytes = 0.5 * 1024 * 1024 * 1024;  // 0.5GB
    const size_t hostChunkSizePoints = hostChunkSizeBytes / sizeof(float3);  // ~42M points
    const size_t numHostChunks = (n + hostChunkSizePoints - 1) / hostChunkSizePoints;
    const size_t overlap = 128;  // boundary jitter fix

    DeviceBuffer<u8> d_labels(n); d_labels.memset(pointtype::nolabel);

    for (size_t h = 0; h < numHostChunks; ++h) {
        size_t start = h * (hostChunkSizePoints - overlap);
        size_t size = min(hostChunkSizePoints, n - start);
        if (h > 0) start += overlap;

        DeviceBuffer<float3> d_chunkpoints(size);
        cudaMemcpy(d_chunkpoints.data(), d_points.data() + start, size * sizeof(float3), cudamemcpydevicetodevice);

        // tile-parallelism: 128 tiles in kernel
        const u32 tilesize = 1024;  // k*32 for warp-alignment
        const u32 numtiles = 128;
        dim3 grid(numtiles);
        dim3 block(256);
        k_generatenormalskernel<<<grid, block>>>(d_chunkpoints.data(), size, tilesize, d_labels.data() + start, true /* directprocess */);
    }
    cudaDeviceSynchronize();

    // rdv voting on normals (chunked similarly)
    DeviceBuffer<float3> d_normalen(n);
    DeviceBuffer<u32> d_normalsCount(1);  // per chunk
    h_generatenormals(d_points, d_normalsCount, d_labels, d_normalen, true);

    DeviceBuffer<rdvcache> d_cache(1);  // global for simplicity
    DeviceBuffer<rdvringBuffer> d_rings(n);  // per-point if needed
    DeviceBuffer<int> d_cacheCount(1);
    h_adaptiveRdvVotering(d_normalen, d_normalsCount, d_cache, d_rings, d_cacheCount);

    // reduce & rotate
    std::vector<rdvcache> chunkCaches = {d_cache.download()};  // Single for demo
    rdvcache merged = combineresults(chunkCaches);
    h_idealframerotation(d_points, merged);  // to AxisResults if needed

    // download & update cc cloud
    // ... (jules' Back-permute) //RR!!!
}
