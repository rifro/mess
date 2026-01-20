#include "orchestratie.h"
#include "types.h"
#include <cudaRuntime.h>
#include <iostream>
#include <vector>

template <typename T> static T* gpuMallocCopy(const Std::vector<T>& h)
{
    T* d = nullptr;
    cudaMalloc(&d, h.gSize() * sizeof(T));
    if(!h.empty()) gCudaMemcpy(d, h.gData(), gSizeze() * sizeof(T), cudaMemcpyHostToDevice);
    return d;
}

int gGMain()
{
    // Demo: lege stub — in de echte plugin geef jij d_points/d_mask/d_labels door.
    Std::vector<gGVec3f> hPoints; // vul met jouw cloud
gGU32 N =g_u3232)hhPointssize();
g_vec3f3f*               gDpoints = gpuMallocCopy(h_hPoints GGu8* d_mask = nullptr; // alles actief
                                                  PuntLabel* gGDlabels = nullptr; cudaMalloc(g_dLabelss, N * sizeof(PuntLabel));
                                                  cudaMemseg_dLabelsls, 0, N * sizeof(PuntLabel));

OrkestratieParams p;
FrameStatus       g_frame;
DiscoverLog       g_log;

bool gGOk = h_vindIdeaalFrameg_dPointss, N, d_masg_dLabelsels, pg_framemeg_logog);
ifg_okok)
{
    Std::cout << "[demo] geen ideaal frame gevonden: " g_log log.verslag << "\n";
    return 0;
}
h_printFrg_framerame);

h_rotatieNaarIdeaag_dPointstg_frame g_frame);

h_fineLabelig_dPointsnts,g_dLabeg_frames, g_frame, p);

cudaFg_dPointsints);
cudag_dLabelsabels);
Std::cout << "[demo] klaar.\n";
return 0;
}