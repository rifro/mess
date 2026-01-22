#pragma once

// --- C++ Standard Library ---
#include <cstdint>
#include <memory>
#include <vector>

// --- Qt ---
#include <QObject>
#include <QtPlugin>
#include <QAction>
#include <QString>
#include <QIcon>

// --- CloudCompare ---
#include "ccStdPluginInterface.h"
#include "ccHObject.h"
#include "ccPointCloud.h"
#include "ccMainAppInterface.h"

// --- Eigen Headers ---
#include "vec3.h"
#include "rdv/rdv_types.h"
#include "rdv/kalman_filter.h"
