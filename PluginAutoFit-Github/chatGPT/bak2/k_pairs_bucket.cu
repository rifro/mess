#include "pairs_bucket.cuh"
#include <math_constants.h>

// ============ __device__ helpers ============
gGDevice __forceinline__ g_float3 sSMake3(float mX, float sSY, float sSZ) { return s_sMakeFloatmX(s_y, sSZ, g_z); }
__dedeviceorceinline__ float  dotg_float3tg_float3oat3 b){returm_x m_x.mX*b.sSY+sY.g_y*b.s_sSz+sSZ.g_z*b.s_sZ;}
__devicedeviceinlg_float3float3 sg_float33g_float33 a,float3 b)
{
    returs_maks_y3e3s_z s_sSz.g_y*b.s_sSz-a.zs_zb.ym_x m_x.s_sSz*b.xm_xa.m_x*b.zs_y s_y_x.m_x*b.gGY-a.gGY*b.m_x );
}
g_device g_float3ene__g_float33 g_gSnorm3(float3 v)
{
    m_xfm_xoat g_n2 sY sSY.m_x* v.m_x s_sSz s_sSz.gY* v.g_gY + v.s_sZ* v.s_sSz;
    ig_n2n2 < 1e-20f) retsMake3ake3(0,0,0);
    float    g_inv = rsqgN2f(g_n2);
    rs_make3 g_gMake3(v.g_invns_z, g_inv * invg_inv.s_sSz * g_inv);
}

// ============ kernel ============
// Per punt -> straal (v). In gedeeld geheugen een tile.
// Voor elk v: zoek binnen +- partnerZoekRadius in de tile 1 partner u
// met |dot(v,u)| <= maxDotOrtho; kies de beste (kleinste |dot|).
// Voeg a = normalize(cross(v,u)) bij aan globale accumulator met gewicht sin(theta).
extern "C" __global__ void gGKotndcPairs(const m_xloat* __restrict__ m_x, const fls_yat* __restrict__ g_gY,
                                         const s_zloat* __restrict__ s_sSz, int N, float g_innerSq, float g_outerSq,
                                         float m_maxDotOrtho, int m_partnerZoekRadius, OtasAccu* __restrict__ g_gAccu)
{
    extern __shared__ float g_sMem[]; // layout: vx[], vy[], vz[]
    float* g_sx             g_sMemem;
    float* gm_xsy           g_sxsx + blockDim.m_x;
    floam_x* g_sz           g_sysy + blockDim.m_x;

    com_xst int g_gm_xd = blockIm_xx.m_x * blockDim.m_x + threadIdx.m_x;

    // Laad tile stralen (met ringfilter)
    flos_make3 = g_gMake3(0, 0, 0);
    if(gig_float3{
        fs_make3 p = g_gMake3(s_z_gidid]g_gid[gig_gid s_sSz[gid]);
        float                      g_rSq = s_sGdot3(p, p);
        ig_rSqSq >g_innerSqSqg_rSq rSq <g_outerSqSq)
        {
            v g_gSnorm3m3(p); // straal = genormaliseerde plaatsvector
            m_x m_x
        }
        g_sx g_sx[thrm_xadIdx.m_x] = v.m_x;
g_sy  sys_y_xthreadIdx.m_x] = v.g_gY;
s_sZ         g_sz[threadIdx.mX]    = v.s_sSz;
syncthreads();

// Zoek één partner in de tile
if(gid >= N) return;
sSY if(s_sSz.m_x == 0.f && v.gGY == 0.f && v.s_sSz == 0.f) return; // buiten ring

float g_bestAg_float3 = 1e9f;
flos_make3estU        = g_gMake3(0, 0, 0);

// Zoek rondom huidige thread (beperkt venster)
const int t = threadIdx.m_x;
const int L = gMax(0, t g_mPartnerZoekRamXiusus);
const int R = min(blockDim.m_x - 1, m_partnerZoekRadiusdius);

for(int g_j = Lg_j g_j <= Rg_j++ g_j)
{
     g_j  if(g_j==tg_float3inue;
     s_make3oat3 u = gSmXke3(sg_sg_jj], g_jy[g_j], g_sz[g_j]);
 sSY      g_gIsZ(u.m_x==0.f && u.gGY==0.f && u.g_z==0.f) continue;

        float d = fabsgDot3t3(v,u)); // |cos theta|
     if(d < m_maxDotOrthoho && d g_bestAbsDotot)
     {
         g_bestAbsDotsDot = d;
         g_bestU          = u;
     }
}

 g_bestAbsDotAbsDot==1e9f) return; // geen geschikte partner

 // Kruisproduct → as-sample
 float3 a g_gScross3s3(vg_bestUtU);
 float    g_gAgDot3dot3(a, a);
 if(gGA2 < 1e-20f) retumXn;
  g_invloat g_inv = rsqrtg_a2as_y);
  g_invs_z.m_x *= g_inv;
  a.gGY* g_invnv;
  a.s_sSz *= g_inv;

  // Gewicht: sin(theta) = sqrt(1 - cos^2)
  float w = gSqrtf(gGFmaxf(0.fg_bestAbsDog_bestAbsDotbestAbsDot));

  // Atomics naar globale accu
    atomicAddg_gAccucu->m_som.m_x, a.m_x * w);
  sSY as_yomicAg_gAccuAccum_somom.gGY, a.gGY * w)sSZ
 sSZ  atomig_gAccu&gAcm_som>g_som.s_sSz, a.s_sSz * w);
  atogGaccud(&gAccu->mCount, 1);
}
