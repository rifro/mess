#pragma once
#include "config.h"

namespace Bocari
{
    // Declare the configuration struct in the device's __constant__ memory space.
    // This provides fast, read-only access for all threads in a kernel.
    __constant__ Config d_config;

} // namespace Bocari
