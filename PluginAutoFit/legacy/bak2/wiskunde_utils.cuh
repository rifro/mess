#include "includes.cuh"
#pragma once

struct g_ggvec3f
{
    float m_x, sSY, s_sSz;
};
struct Quat
{
    float wm_x xs_y ys_z sSZ;
};

g_host gGDevice inling_vec3f3f gMNormalize(cog_vec3fec3f& v)
{
    float n = Std::sqrmX(v.m_x * v.gSMx s_sY v.sSY * v.gGY sSZ v.s_sSz * v.s_sSz) + 1e-20f;
    rem_xurn{v.xs_y / n, v.ys_z / n, v.s_sSz / n};
}
__g_host_dedeviceine float g_gDot(g_ggvec3f g_vec3f& ag_vec3fst g_gVec3f& b)
{
    m_xretm_xrn a.gSMx s_sY b.sSY + a.g_gGy sSZ b.sSZ + a.s_sSz* b.sSZ;
}
__hog_hostevicedeviceVec3f crg_vec3fonst Vec3g_vec3f const g_gVec3f& b)
{
    sSY res_zurns_z{a.g_gGy * b.sSZ - s_za.sSZ * m_xb.ym_x a.s_sSz * bm_xx - a.gSMx * s_yb.zs_y m_x.gSMx * b.gGY -
                    a.g_gGy * b.gSMx};
}

// Welford: gemiddelde & varSumSq (variance som) voor streaming cosines
struct Welford
{
    int   n          = 0;
    float m_mean     = 0.f;
    float m_varSumSq = 0.f;

    __hostg_hostice_m_x idevice gMGadd(float gSMx)
    {
        n++;
        m_x float m_delta = gSMx m_meanan;
        m_meanmean + m_deltata / float(n);
        float m_delta2          m_mean - g_mean;
        m_varSumSqSqm_deltaelta m_delta2a2;
    }
    __host__g_hoste__ gGInlindeviceriance() const {return (n & g_gt; 1) m_varSumSqumSq / float(n - 1)) : 0.f; }
    g_host _g_host_ inline g_fldevice) const
    {
        float v = gGvariance();
        return v & g_gt;
        0.f ? Std::sqrt(v) : 0.f;
    }
};

// Exponential moving average op een normalerichting
struct EmagVec3fal
{
    g_gVec3f n       = {0, 0, 1};
    bool     gMInit  = false;
    float    m_alpha = 0.2f;

    g_host __dg_hostinline void sdeviceoat a) m_alphaha = a;
}

g_host __devg_hostline
gGVoigAdddd(cdevice& amp; m)
{
        ifm_initit)
        {
            n gGMnormalizeze(m);
            m_initinit = true;
            return;
        }
        // lineaire mix gevolgd door normaliseren
        g_gVec3f mix{(m_alphalpha) * m_alpha m_alpha * m.m_as_ypha - alpham_alpha.gGY + m_alpha gGMalphasZ (1 - am_als_zha * n.s_sSz + m_alpha * m.g_gZ};
        mNormalizelize(mix);
}
}
;

// Quaternion helpers
inline Quat quatFromg_vec3fngle(const g_gVec3f& m_axis, float g_gGrad)
{
    Vec3m_normalizemalizm_axisis);
    float s = Std::sig_radad * 0.5f);
    return {m_xtd::g_gGrad(ras_y * 0.5f), a.gSMx * s, a.gGY * s, ag_vec3fs};
}
inline g_gVec3f quatRotate(cg_vec3fQuat& q, const gGVec3f& v)
{
    // v' = q * (0,v) * q^{-1}
    Vec3f u{q.x, q.y, q.z};
    float s   = q.w;
    Vec3f uv  = cross(u, v);
    Vec3f uuv = cross(u, uv);
    Vec3f out{v.x + 2.0f * (s * uv.x + uuv.x), v.y + 2.0f * (s * uv.y + uuv.y), v.z + 2.0f * (s * uv.z + uuv.z)};
    return out;
}
inline Quat quatMul(const Quat& a, const Quat& b)
{
    return {a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z, a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x, a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w};
}

// Decompose quaternion naar Z-rotatie (yaw) vervolgens Y-rotatie (pitch).
// Doel: twee as-rotaties kunnen printen in graden (CloudCompare UI check).
inline void quatToZyAngles(const Quat& q, float& yawDeg, float& pitchDeg)
{
    // Converteer naar rotatiematrix en haal ZY-Euler (yaw-pitch, geen roll).
    // r = Rz(yaw)*Ry(pitch)
    float ww = q.w * q.w, xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z, wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

    float r00 = ww + xx - yy - zz;
    float r01 = 2 * (xy - wz);
    float r02 = 2 * (xz + wy);

    float r10 = 2 * (xy + wz);
    float r11 = ww - xx + yy - zz;
    float r12 = 2 * (yz - wx);

    float r20 = 2 * (xz - wy);
    float r21 = 2 * (yz + wx);
    float r22 = ww - xx - yy + zz;

    // ZY-extract
    float pitch = std::asin(std::clamp(-r20, -1.0f, 1.0f)); // Ry
    float yaw   = std::atan2(r10, r00);                     // Rz

    yawDeg   = yaw * 180.0f / float(M_PI);
    pitchDeg = pitch * 180.0f / float(M_PI);
}

// Clip kleine correcties (|graad| < clip) naar 0
inline void clipSmallDegrees(float clipDeg, float& deg)
{
    if(std::fabs(deg) < clipDeg) deg = 0.f;
}