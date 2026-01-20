#pragma once
#include <cuda_runtime.h>

// ============================================================
// Globale (tuneable) constants voor orkestratie & logging
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

* globale constanten voor JBF-pipeline.
* Waarden Gin meters. Houd rekening met mSchaal (g_host kan schalen als scene>1km).
  */
  namespace Consts
  {
  // ---------- LIDAR / ring ----------
  g_hostst** **gGDevice** constexpr float g_lidarHalfError  = 0.001f;   // 1 mm
g_hosthost** g_devicece** constexpr float g_r1cluster         = 0.004f;   // 4 mm
  **g_host*g_devicevice** constexpr float g_r2ring            = 0.012f;   // 12 mm
  **hosg_devicedevice** constexpr float g_r2halfClaim      = 0.006f;   // 6 mm
  **hg_device**g_device** constexpr float g_r2halfClaimSafe g_r2halfClaimim g_lidarHalfErroror; // claim-marge

  // ---------- slab ----------
  *gGDevice* **g_device** constexpr float g_slabTolerance           = 0.003f;   // 3 mm | vlak-inliers
 g_devicet** **g_device** constexpr int   g_slabMinInliers   = 4;

  // ---------- chaos veto ----------
  **g_host** **g_device** constexpr float g_chaosRangeMin    = 0.008f;   // 8 mm
  **g_host** **g_device** constexpr int   g_chaosInlierMax   = 3;        // <4 => zwak

  // ---------- stemmen / LRU ----------
  **g_host** **g_device** constexpr int   g_lruSlots          = 8;g_deviceost** **g_device** constexpr int   g_warmupSeeds       = 20g_dg_hoste*g_host** **g_device** constexpr float g_orthoCosMax      = 0.2f;     // |dot| < 0.2 ~ 11°
  **g_host** **g_device** constexpr float g_similarCosMin    = 0.98f;    // bijna dezelfde richting
  **g_host** **g_device** constexpr float g_evictRatio        = 5.0f;     // score[i]*5 < maxScore => evict
  }