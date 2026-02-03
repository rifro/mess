#pragma once

// --- C++ Standard Library ---
#include <algorithm> // Voor std::round, std::max, std::min
#include <array>
#include <cassert> // Voor assert
#include <cmath>   // Voor std::floor, std::round, std::sqrt, std::abs
#include <cstddef> // Voor size_t
#include <cstdint>
#include <format>
#include <functional>  // Voor std::function
#include <iostream>    // Voor std::cout, std::cerr
#include <limits>      //
#include <memory>      // Voor std::unique_ptr, std::shared_ptr
#include <numeric>     //
#include <signal.h>    // Voor signal()
#include <sstream>     // Voor std::stringstream (toegevoegd voor eerdere discussie)
#include <stdexcept>   // Voor std::invalid_argument, std::runtime_error
#include <string>      //
#include <tuple>       //
#include <type_traits> // Vereist voor std::is_base_of_v
#include <vector>      //

// --- CloudCompare Core ---
// #include <PointCloud.h> // Let op: kan conflicteren met ccPointCloud indien
// beide gebruikt worden
// #include <ccColorScaleEditorDialog.h>
#include <ccColorScalesManager.h>
#include <ccGLMatrix.h>
#include <ccHObject.h>          // For ccHObject::Container
#include <ccMainAppInterface.h> // For ccMainAppInterface
#include <ccOctree.h>
#include <ccPointCloud.h>
#include <ccScalarField.h>
#include <ccStdPluginInterface.h> // Voor CloudCompare plugin interface

// --- CCCoreLib ---
#include <CCConst.h>
#include <CCCoreLib.h> // Voor CCCoreLib::computeOptimalLevel
#include <CCGeom.h>    //
#include <CCMath.h>    // Voor CCCoreLib::DegreesToRadians
#include <CCTypes.h>   // Voor PointCoordinateType = float!
#include <CloudSamplingTools.h>
#include <DgmOctree.h>
#include <GenericIndexedCloudPersist.h>
#include <ReferenceCloud.h>

// --- Qt ---
#include <QAction>
#include <QColor> // For qRgb
// #include <QDebug> // Voor qDebug
#include <QDir> // Voor QDir::mkpath
#include <QFile>
#include <QImage>
#include <QList>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QtGlobal> // For Qt::white etc.

#include "types.h"

// Nu kun je hier eventueel nog forward declarations zetten 
// voor zaken die GEEN basis-types zijn:
namespace Bocari {
    class AutofitImpl; 
}

inline void dbg(const char* message) { std::cout << message << std::endl; }
inline void dbg(const QString& s) { dbg(s.toStdString().c_str()); }
