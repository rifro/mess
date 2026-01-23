#pragma once

namespace Bocari
{
    /**
     * @brief Launches the CUDA kernel for adaptive Radial Disk Voting (RDV).
     *
     * This kernel takes a list of surface normals and "votes" for dominant axes.
     * Normals that match an existing axis "nudge" it, refining its direction.
     * Normals that don't match are placed in a ring buffer for potential future use.
     *
     * @param d_normals Input device buffer of surface normals.
     * @param d_normalsCount Device buffer containing the count of valid normals.
     * @param d_axisAccumulators Device buffer of axis accumulators to be updated.
     * @param d_ringBuffer Device buffer for storing unmatched normals.
     * @param d_ringBufferPosition Atomic counter for the current position in the ring buffer.
     */
    void h_adaptiveRdvVoting(
        const DeviceBuffer<Vec3f>& d_normals,
        const DeviceBuffer<u32>& d_normalsCount,
        DeviceBuffer<RdvAxisAccumulator>& d_axisAccumulators,
        DeviceBuffer<Vec3f>& d_ringBuffer,
        DeviceBuffer<u32>& d_ringBufferPosition
    );

} // namespace Bocari
