#pragma once
#include "config.h"

namespace Bocari
{
    /**
     * @brief Copies the host-side configuration to the device's __constant__ memory.
     * @details This function should be called once before any kernels that rely on the
     * global configuration are launched. It uses cudaMemcpyToSymbol to transfer the
     * configuration object to the `d_config` variable in constant memory.
     * @param hostConfig The host-side configuration object to copy from.
     */
    void h_updateConstantConfig(const RdvConfig& hostConfig);

} // namespace Bocari
