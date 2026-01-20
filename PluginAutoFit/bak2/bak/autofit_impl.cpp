// updated host-flow with 0.5GB chunks and tile-parallelism
// ... (jules' recursiveSplit + safeVramLimit)
void autofitImpl::do_processing() {
    // ... upload h_x/h_y/h_z to d_x/d_y/d_z, merge to d_points (morton-sorted)

    const sizeT hostChunkSizeBytes = 0.5 * 1024 * 1024 * 1024;  // 0.5GB
    const sizeT hostChunkSizePoints = hostChunkSizeBytes / sizeof(float3);  // ~42M points
    const sizeT numHostChunks = (n + hostChunkSizePoints - 1) / hostChunkSizePoints;
    const sizeT overlap = 128;  // boundary jitter fix

    deviceBuffer<u8> d_labels(n); d_labels.memset(pointType::no_label);

    for (sizeT h = 0; h < numHostChunks; ++h) {
        sizeT start = h * (hostChunkSizePoints - overlap);
        sizeT size = min(hostChunkSizePoints, n - start);
        if (h > 0) start += overlap;

        deviceBuffer<float3> d_chunkPoints(size);
        cudaMemcpy(d_chunkPoints.data(), d_points.data() + start, size * sizeof(float3), cudaMemcpyDeviceToDevice);

        // tile-parallelism: 128 tiles in kernel
        const u32 tileSize = 1024;  // k*32 for warp-alignment
        const u32 numTiles = 128;
        dim3 grid(numTiles);
        dim3 block(256);
        k_generateNormalsKernel<<<grid, block>>>(d_chunkPoints.data(), size, tileSize, d_labels.data() + start, true /* directProcess */);
    }
    cudaDeviceSynchronize();

    // rdv voting on normals (chunked similarly)
    deviceBuffer<float3> d_normalen(n);
    deviceBuffer<u32> d_normalenCount(1);  // per chunk
    h_generateNormals(d_points, d_normalenCount, d_labels, d_normalen, true);

    deviceBuffer<rdvCache> d_cache(1);  // global for simplicity
    deviceBuffer<rdvRingBuffer> d_rings(n);  // per-point if needed
    deviceBuffer<int> d_cacheCount(1);
    h_adaptiveRdvVoting(d_normalen, d_normalenCount, d_cache, d_rings, d_cacheCount);

    // reduce & rotate
    std::vector<rdvCache> chunkCaches = {d_cache.download()};  // single for demo
    rdvCache merged = combineResults(chunkCaches);
    h_idealFrameRotation(d_points, merged);  // to assenResultaat if needed

    // download & update cc cloud
    // ... (jules' back-permute)
}
