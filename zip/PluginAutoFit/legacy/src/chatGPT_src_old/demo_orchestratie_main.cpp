#include <iostream>
#include <vector>
#include <cudaRuntime.h>
#include "orchestratie.h"
#include "types.h"

template <typename T>
static T* gpuMallocCopy(const std::vector<T>& h){
T* d=nullptr;
cudaMalloc(&d, h.size()*sizeof(T));
if(!h.empty()) cudaMemcpy(d, h.data(), h.size()*sizeof(T), cudaMemcpyHostToDevice);
return d;
}

int main()
{
// Demo: lege stub — in de echte plugin geef jij d_points/d_mask/d_labels door.
std::vector<Vec3f> h_points; // vul met jouw cloud
u32 N = (u32)h_points.size();


Vec3f* d_points = gpuMallocCopy(h_points);
u8* d_mask=nullptr;  // alles actief
PuntLabel* d_labels=nullptr;
cudaMalloc(&d_labels, N*sizeof(PuntLabel));
cudaMemset(d_labels, 0, N*sizeof(PuntLabel));

OrkestratieParams p;
FrameStatus frame;
DiscoverLog log;

bool ok = hVindIdeaalFrame(d_points, N, d_mask, d_labels, p, frame, log);
if(!ok){
    std::cout << "[demo] geen ideaal frame gevonden: " << log.verslag << "\n";
    return 0;
}
hPrintFrame(frame);

hRotatieNaarIdeaal(d_points, N, frame);

hFineLabeling(d_points, N, d_labels, frame, p);

cudaFree(d_points);
cudaFree(d_labels);
std::cout << "[demo] klaar.\n";
return 0;


}