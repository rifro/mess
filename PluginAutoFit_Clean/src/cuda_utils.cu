#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    void h_updateConstantConfig(const RdvConfig& hostConfig)
    {
        cudaMemcpyToSymbol(d_config, &hostConfig, sizeof(RdvConfig));
    }

} // namespace Bocari
