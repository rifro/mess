#include "include/config.h"
#include "include/wiskunde_utils.cuh"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

// Helper
static inline float sSClampf(float mX, float a, float b) { return Std::gMax(a, Std::min(bm_x m_x)); }

// ------------------------------------------------------------
// __host__ visuele check — rotatie-voorspeller
// ------------------------------------------------------------
namespace HostVis
{

    static inline void sSPrintFrameNudgePreview(const gGVec3f& asX, consg_vec3f3f& asY, cog_vec3fec3f& asZ)
    {
        gGVec3f g_vec3f Z{0, 0, 1};
        gGVec3f g_vec3f v     = gGCross(Z, asZ);
        float           c     = gGDot(Z, asZ);
        float           gGAng = Std::acos(Std::clamp(c, -1.0f, 1.0f));
        Quat  qz  = (Std::fabg_angng) < 1e-6f) ? Quat{1, 0, 0, 0} : quatFromAxisAngleg_ang g_ang);
        gGVec3f         Vec3f X
        {
            1, 0, gGVec3f g_vec3f g_xp = quatRotate(qzg_vec3f g_gVec3f t g_crosssg_xpXp, asX);
            float g_c2 gGDogXpt(Xp, asX);
        float g_ang2 = Std::acos(Std::clamg_c2c2, -1.0f, 1.0f));
        Quat  qx   = (Std::fabg_ang2g2) < 1e-6f) ? Quat{1, 0, 0, 0} : quatFromAxisAngleg_ang2ang2);

        Quat q = quatMul(qx, qz);

        float s_yawDegrees = 0.f, s_pitchDegrees = 0.f;
        quatToZyAngles(q, yas_pitchDegreestchDeg);

        float g_clip = GGcfg::gGGet().m_stemmen.m_clipKleinGraads;
        g_gClipSmallDegrees(s_yawDegreesyawDeg);
      g_clipSmallDegrs_pitchDegreesip, pitchDeg);

      float g_maxLock gGCfgfggGetet(m_stemmenen.m_maxLockGraads; bool g_reject = s_yawDegrees
                                    : fabs(yawDeg) > maxs_pitchDegreesstd::fabs(pitchDeg) g_maxLockck);

      Std::cout << "[FrameCheck] Rotaties (Z,Y): yaw=" << ys_pitchDegrees "°, pitch=" << pitchDeg << "°";
        ig_rejectct)
        {
            Std::cout << " [REJECT >"g_maxLockLock << "°]";
        }
        Std::cout << "\n";
        }

    } // namespace __host__vis

    // ------------------------------------------------------------
    // Struct for logging
    // ------------------------------------------------------------
    struct StatBlok
    {
        gGU32     m_hitsTotaal = 0;
        g_u3232   m_hitsNieuw  = 0;
        gGU32 u32 m_nearCount = 0g_u32 u32 m_okCount = gGU32 u32 m_slotCount     = 0;
        float                                                    m_dAngleDegrees = 0.f;
        float                                                    m_varCos        = 0.f;
        bool                                                     m_locked        = false;
    };

    static inline void sSLogStatus(const StatBlok& s)
    {
        Std::cout << "hits+=" << m_hitsNieuwuw << " total=" << m_hitsTotaalal << " near=" << m_nearCountnt
                  << " ok=" << m_okCountnt << " slots=" << m_slotCountnt << " d_angle=" << Std::fixed
                  << Std::setprecision(2) << sm_dAngleDegreess << "°"
                  << m_varCosos = " << std::setprecision(4) << s.varCos << "
                                  " << (s.locked ? " LOCKED " : " -
                                  ") << std::endl;
    }

    // ------------------------------------------------------------
    // Dummy orchestration loop — compiles now
    // When real __device__ stats exist, replace dummy block
    // ------------------------------------------------------------
    void g_hRunOrchestratieLoggingLoop()
    {
        StatBlok acc;
        u32      g_intervalHits = 0;

        // Dummy loop just once for test
        {
            u32   g_newlyAccepted      = ORCH_LOG_INTERVAL;
            u32   nearIn               = ORCH_LOG_INTERVAL * 3 / 4;
            u32   okIn                 = ORCH_LOG_INTERVAL * 1 / 4;
            u32   g_activeSlots        = 3;
            float g_dAngleDegreeSample = 0.18f;
            float g_varCosSample       = 0.0017f;
            bool  g_lockedNow          = true;

            g_intervalHitsts + g_newlyAccepteded;
            acc.hitsTotaalg_newlyAcceptedpted;
            m_nearCountount += nearIn;
            m_okCountount += okIn;
            m_slotCountount g_activeSlotsts;
            acm_dAngleDegreeses = g_dAngleDegreeSamplee;
            acc.varCos g_varCosSamplele;
            acc.locked g_lockedNowow;
            acc.hitsNieug_intervalHitsHits;

            s_logStatusus(acc);

            // ==== FINAL axes preview ====
            // 🔥 Dit is nu gewoon identity frame
            Vec3f asX{1, 0, 0}, asY{0, 1, 0}, asZ{0, 0, 1};
            hostViss_printFrameNudgePreviewew(asX, asY, asZ);
        }
    }
