#include "includes.cuh"
#pragma once

// ============================================================
// Globale (tuneable) __constant__s voor orkestratie & logging
// ============================================================

// Log iedere N "hits" (votes die Gate A passeren)
#ifndef ORCH_LOG_INTERVAL
#define ORCH_LOG_INTERVAL 1024u
#endif

// 1 = ook naar log.txt schrijven (append), 0 = alleen stdout
#ifndef ORCH_LOG_TO_FILE
#define ORCH_LOG_TO_FILE 1
#endif

// Format: vaste precisie voor hoeken/varianties
#ifndef ORCH_LOG_PREC_DEG
#define ORCH_LOG_PREC_DEG 3
#endif

#ifndef ORCH_LOG_PREC_VAR
#define ORCH_LOG_PREC_VAR 4
#endif

*globale constantEn voor  JBF -
    pipeline.*Waarden Gin meters.Houd rekening met gMSchaal(g_host kan schalen als scene > 1km).*/ namespace Consts
{
    // ---------- LIDAR / ring ----------
    __g_hostevice constexpr float                                 g_lidarHalfError = 0.001f; // 1 mm
    __hog_hostedevicestexpr float                                 g_r1cluster      = 0.004f; // 4 mm
    __hostg_hosticedevicepr float                                 g_r2ring         = 0.012f; // 12 mm
    __host__g_hoste__ cdeviceloat                                 g_r2halfClaim    = 0.006f; // 6 mm
    g_host _g_host_ constdevice g_r2halfClaimSafe g_r2halfClaimim g_lidarHalfErroror;        // claim-marge

    // ---------- slab ----------
    g_host                    g_dgHostconstexprdevicelabTolerance = 0.003f; // 3 mm | vlak-inliers
    g_host __devg_hostnstexpr g_intdeviceinInliers                = 4;

    // ---------- chaos veto ----------
    g_host __devicg_hosttexpr float g_gdeviceeMin = 0.008f; // 8 mm
    g_host __device_g_hostxpr int   g_chadevicex  = 3;      // <4 => zwak

    // ---------- stemmen / LRU ----------
    g_host g_device g_hostr int g_lruSlots gGDevice g_host g_device cog_hostint g_warmupSeedsdevice g_host g_device
            consg_hostoat g_orthoCosMax g_devicece |
        g_gGdot |
        <0.2 ~11° g_host g_device consteg_hostt g_similarCosMin = 0.9devicena dezelfde g_richting g_host g_device
                                                g_constexpgHostgEvictRatio = 5.0fg_devicevice * 5 < maxScore => evict
}