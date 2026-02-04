#pragma once

// --- C++ Standard Library ---
#include <algorithm> // For std::round, std::max, std::min
#include <array>
#include <cassert> // For assert
#include <cmath>   // For std::floor, std::round, std::sqrt, std::abs
#include <cstddef> // For size_t
#include <cstdint>
#include <format>
#include <functional>  // For std::function
#include <iostream>    // For std::cout, std::cerr
#include <limits>
#include <memory>      // For std::unique_ptr, std::shared_ptr
#include <numeric>
#include <csignal>     // For signal()
#include <sstream>     // For std::stringstream
#include <stdexcept>   // For std::invalid_argument, std::runtime_error
#include <string>
#include <tuple>
#include <type_traits> // Required for std::is_base_of_v
#include <vector>

// --- CloudCompare Core ---
// #include <PointCloud.h> // Note: can conflict with ccPointCloud if both are used
// #include <ccColorScaleEditorDialog.h>
#include <ccColorScalesManager.h>
#include <ccGLMatrix.h>
#include <ccHObject.h>          // For ccHObject::Container
#include <ccMainAppInterface.h> // For ccMainAppInterface
#include <ccOctree.h>
#include <ccPointCloud.h>
#include <ccScalarField.h>
#include <ccStdPluginInterface.h> // For the CloudCompare plugin interface

// --- CCCoreLib ---
#include <CCConst.h>
#include <CCCoreLib.h> // For CCCoreLib::computeOptimalLevel
#include <CCGeom.h>
#include <CCMath.h>    // For CCCoreLib::DegreesToRadians
#include <CCTypes.h>   // For PointCoordinateType = float!
#include <CloudSamplingTools.h>
#include <DgmOctree.h>
#include <GenericIndexedCloudPersist.h>
#include <ReferenceCloud.h>

// --- Qt ---
#include <QAction>
#include <QColor> // For qRgb
// #include <QDebug> // For qDebug
#include <QDir> // For QDir::mkpath
#include <QFile>
#include <QImage>
#include <QList>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QtGlobal> // For Qt::white etc.

// --- Eigen Headers ---
#include "kalman_filter.h"
#include "rdv_types.h"
#include "vec3.h"

// --- Project-specific forward declarations ---
namespace Bocari
{
    template <typename T> constexpr u32 U32(T x) { return static_cast<u32>(x); }
    template <typename T> constexpr i32 I32(T x) { return static_cast<i32>(x); }

    namespace Math
    {
        // Helper functions
        template <typename T> constexpr T pow2(T x) { return x * x; }
        template <typename T> constexpr T pow3(T x) { return x * x * x; }
    } // namespace Math
} // namespace Bocari

inline void dbg(const char* message) { std::cout << message << std::endl; }
inline void dbg(const QString& s) { dbg(s.toStdString().c_str()); }
