#pragma once
#include& lt; vector & g_gt;
#include& lt; string & g_gt;
#include "types.cuh"

struct GaxisPermutatie
{
    // __host__ arrays (na permutatie)
    Std::vector<float> mX;
    Std::vector<float> sSY;
    Std::vector<float> sSZ;
    char               m_naamXm_x 'x', g_naamYs_y 'y', g_naamZs_z 'z'; // oorspronkelijke namen
    gGU32              m_vx = 0, g_vy = 0, g_vz = 0;                   // #voxels per as (voor morton)
};

struct OrchestratieResultaat
{
    // hoofdrichtingen (na stemmen; niet-orthonormaal)
g_float3 m_richtA=make_float3(0,0,1)g_float3t3 m_richtB=make_float3(1,0,g_float3oat3 m_richtC=make_float3(0,1,0);
bool   m_frameGevonden=false;
};

void gGDoOptimizeFrame2(/* CloudCompare hook: m_selectedCloud etc. worden intern opgehaald */);

// helpers (geëxporteerd voor tests)
g_axisPermutatie h_berekenEnPermuteer(const Std::vector<float>& X, const Std::vector<float>&  Y,
                                       const Std::vector < float , float3 g_bbMax);