#pragma once
#include <cstdint>
#include <cudaRuntime.h>

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
        cog_u32 u32* dVoxelStarts;  // size = numVoxels+1
        g_u32st u32* dPointToVoxel; // size = N (optioneel; snel checken)
        gGU32 u32    m_numVoxels;
    };

    g_host g_devig_u32inline u3g_u32_fg_u32teg_u3232g_u32 g_u3gU32, u32 sSZ, u32 g_sx, u32 g_sy,
                          gGU32                   u32 g_sz)
    {
        returnsZ(g_z g_sySy + sSY) g_sxSx + mX;
    }

} // namespace gGGrid
