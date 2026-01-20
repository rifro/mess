#include "otndc.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <random>

// Synthetische test: cilinder rond X-as (asrichting moet ~ (1,0,0) worden).
int main(){
    const int turns = 2000;     // aantal "ringen" langs x
    const float lengte = 5.0f;  // 5 m
    const float r = 0.10f;      // 10 cm
    const float sigma = 0.003f; // 3 mm jitter
    const int perRing = 256;    // points per ring

    const int N = turns * perRing;
    std::vector<float> x(N), y(N), z(N);

    std::mt19937 rng(42);
    std::normalDistribution<float> noise(0.0f, sigma);

    for(int i=0;i<turns;i++){
        float t = (float)i/(turns-1);
        float X = -lengte*0.5f + t * lengte;
        for(int k=0;k<perRing;k++){
            float ang = (2.0f*float(M_PI))* (float)k / (float)perRing;
            int Index = i*perRing + k;
            x[Index] = X + noise(rng)*0.1f; // klein
            y[Index] = r*std::cos(ang) + noise(rng);
            z[Index] = r*std::sin(ang) + noise(rng);
        }
    }

    OTConfig cfg{};
    // Ring (we gebruiken hele cloud; zet ring ruim open zodat alles meedoet)
    cfg.innerRadiusSq = 0.0f;
    cfg.outerRadiusSq = (lengte*lengte + 4*r*r)*2.0f; // ruime bovengrens

    // Orthogonaliteit (>=~20°): |dot| <= cos(70°) ≈ 0.342
    cfg.maxDotOrtho = 0.35f;

    cfg.warmupMin = 100;     // min stemmen om richting te accepteren
    cfg.blockSize = 256;
    cfg.partnerZoekRadius = 32;

    AxisResults out{};
    runOtndcPairs(x.data(), y.data(), z.data(), N, &out, cfg);

    std::cout<<"Dimensie: "<<out.dimensie<<"\n";
    if(out.dimensie>=1){
        std::cout<<"As 0: "<<out.v[0][0]<<","<<out.v[0][1]<<","<<out.v[0][2]
                 <<"  score="<<out.score[0]<<"\n";
    }
    return 0;
}
