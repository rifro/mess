#include "includes.cuh"
#include "strict.h"

namespace Bocari
{
    void h_updateConstantConfig(const Config& hostConfig)
    {
        cudaMemcpyToSymbol(d_config, &hostConfig, sizeof(Config));
    }

} // namespace Bocari
