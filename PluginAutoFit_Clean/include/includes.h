#pragma once

// --- C++ Standard Library ---
#include <cstdint>
#include <memory>
#include <vector>

// --- Qt ---
#include <QAction>
#include <QIcon>
#include <QObject>
#include <QtPlugin>
#include <QString>

// --- CloudCompare ---
#include "ccHObject.h"
#include "ccMainAppInterface.h"
#include "ccPointCloud.h"
#include "ccStdPluginInterface.h"

// --- Eigen Headers ---
#include "rdv/kalman_filter.h"
#include "rdv/rdv_types.h"
#include "vec3.h"
