#pragma once

// --- c++ Standard Library ---
#include <algorithm> // for std::round, std::max, std::min
#include <array>
#include <cassert> // for assert
#include <cmath>   // for std::floor, std::round, std::sqrt, std::abs
#include <cstddef> // for size_t
#include <cstdint>
#include <format>
#include <functional>  // for std::function
#include <csignal>  // for signal()
#include <iostream>    // for std::cout, std::cerr
#include <limits>      //
#include <memory>      // for std::unique_ptr, std::shared_ptr
#include <numeric>     //
#include <sstream>     // for std::stringstream
#include <stdexcept>   // for std::invalid_argument, std::runtime_error
#include <string>      //
#include <tuple>       //
#include <type_traits> // Vereist for std::is_base_of_v
#include <vector>      //

// --- CloudCompare Core ---
// #include <Pointcloud.h> // Let op: kan conflicteren met ccPointCloud indien beide gebruikt worden
// #include <ccColorScaleEditorDialog.h>
#include <ccPointCloud.h>
#include <ccColorScalesManager.h>
#include <ccGLMatrix.h>
#include <ccHObject.h>          // for ccHObject::container
#include <ccMainAppInterface.h> // for ccMainAppInterface
#include <ccOctree.h>
#include <ccScalarField.h>
#include <ccStdPluginInterface.h> // for CloudCompare plugin interface

// --- CCCoreLib ---
#include <CCConst.h>
#include <CCGeom.h>  //
#include <CCMath.h>  // for CCCoreLib::DegreesToRadians
#include <CCTypes.h> // for PointCoordinateType = float!
#include <DgmOctree.h>
#include <CCCoreLib.h> // for CCCoreLib::computeOptimalLevel
#include <GenericIndexedCloudPersist.h>
#include <CloudSamplingTools.h>
#include <ReferenceCloud.h>

// --- Qt ---
#include <QAction>
#include <QColor> // for qRgb
// #include <qDebug> // for qDebug
#include <QDir> // for QDir::mkpath
#include <QFile>
#include <QImage>
#include <QList>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QtGlobal> // for Qt::white etc.

// --- Eigen Headers ---
#include "rdv/kalman_filter.h"
#include "rdv/rdv_types.h"
#include "vec3.h"
