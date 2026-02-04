#pragma once
#include "includes.cuh"

namespace Bocari
{
    /**
     * @brief Computes Morton codes for a page and sorts indices using CUB.
     * * @param d_pageBuffer    The 15-byte packed buffer (u32 x, u32 y, u32 z, u8 t, u16 id)
     * @param d_mortonKeys    Primary buffer for 64-bit Morton codes
     * @param d_mortonKeysAlt Alternative buffer for CUB's double-buffering
     * @param d_indices       Primary buffer for point indices (the result)
     * @param d_indicesAlt    Alternative buffer for CUB's double-buffering
     * @param d_tempStorage   Scratchpad memory for CUB Radix Sort
     * @param tempStorageBytes Size of the scratchpad memory
     * @param pointCount      Number of points in the current page
     */
    void launchMortonSort(
        u8* d_pageBuffer, 
        u64* d_mortonKeys, 
        u64* d_mortonKeysAlt,
        u32* d_indices, 
        u32* d_indicesAlt,
        void* d_tempStorage,
        size_t  tempStorageBytes,
        u32     pointCount);
}