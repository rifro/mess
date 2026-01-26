#pragma once

namespace Bocari
{
    /**
     * @brief Launches the CUDA kernel for adaptive Radial Disk Voting (RDV).
     * @details This host function orchestrates the RDV process. It retrieves the number
     * of normals from the GPU (since it's computed by a previous kernel), calculates
     * the required CUDA grid size, and then launches the k_adaptiveRdvVoterKernel
     * to perform the actual axis voting and refinement.
     *
     * @param d_normals Input buffer of surface normals on the device.
     * @param d_normalsCount A device buffer containing a single u32 value: the count of valid normals.
     * @param d_axisAccumulators Output buffer of axis accumulators to be updated by the kernel.
     * @param d_ringBuffer A circular buffer for storing unmatched "orphan" normals.
     * @param d_ringBufferPosition An atomic counter for the current position in the ring buffer.
     */
    void h_adaptiveRdvVoting(
        const DeviceBuffer<Vec3f>& d_normals,
        const DeviceBuffer<u32>& d_normalsCount,
        DeviceBuffer<RdvAxisAccumulator>& d_axisAccumulators,
        DeviceBuffer<Vec3f>& d_ringBuffer,
        DeviceBuffer<u32>& d_ringBufferPosition
    );

} // namespace Bocari
