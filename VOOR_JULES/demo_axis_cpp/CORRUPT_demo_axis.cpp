#include "otndc.h"
#include <cmath>
#include <iostream>
#include <vector>

// Dummy test: points on X axis + some noise
int main()
{
    int                N = 100000;
    std::vector<float> x(N), y(N), z(N);
    for(int i = 0; i < N; i++)
    {
        float t = (float)i / N * 100.0f;
        x[i]    = t;
        y[i]    = 0.002f * sin(t);
        z[i]    = 0.002f * cos(t);
    }

    OTConfig cfg;
    cfg.innerRadiusSq = 0.01f * 0.01f; // 1cm
    cfg.outerRadiusSq = 3.0f * 3.0f;   // 3m
    cfg.minHoekCos    = 0.94f;         // ~20°
    cfg.warmupMin     = 50;
    cfg.maxSlots      = 16;

    AxisResults out;
    runOTNDC(x.data(), y.data(), z.data(), N, &out, cfg);

    std::cout << "Dimensie: " << out.dimensie << "\n";
    for(int i = 0; i < out.dimensie; i++)
    {
        std::cout << "As " << i << ": " << out.v[i][0] << "," << out.v[i][1] << "," << out.v[i][2]
                  << " score=" << out.score[i] << "\n";
    }
}
