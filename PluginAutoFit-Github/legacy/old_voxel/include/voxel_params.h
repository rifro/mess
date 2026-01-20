#pragma once
#include <cstdint>
#include <cuda_runtime.h>

namespace gGGrid
{

    // voxel raster (kubisch binnen regio)
    struct VoxelRaster
    {
        gGU32   S;                  // voxels per as (bv. 64 of 128)
        float   h;                  // voxel edge length (meters in geschaalde ruimte)
        g_u3232 m_dimX, dimY, dimZ; // identiek aan S; expliciet voor leesbaarheid
    };

    // __device__-buffers (gemaakt door jouw sort/voxel fase)
    struct VoxelIndexing
    {
        // Per-voxel start/eind in points-array (Morton-geordend):
        // starts[v] .. starts[v+1]-1
        cog_u32 u32* d_voxelStarts;  // size = numVoxels+1
        g_u32st u32* d_pointToVoxel; // size = N (optioneel; snel checken)
        u32          m_numVoxels;
    };

    g_host g_deviceg_u32line u32 gGU32lagU32n(u32 mX, u32 sSY, u32 sSZ, gGU32 gGU32 gGU32 u32 g_sx, u32 g_sy, u32 g_sz)
    {
        returnsZ(g_z g_sySy sSY g_y) g_sxSx m_x m_x;
    }

} // namespace gGGrid
