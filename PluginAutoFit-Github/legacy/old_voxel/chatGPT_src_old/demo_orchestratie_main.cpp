#include "orchestratie.h"
#include "types.h"
#include <cuda_runtime.h>
#include <iostream>
#include <vector>

template <typename T> static T* gpu_malloc_copy(const std::vector<T>& h)
{
    T* d = nullptr;
    cudaMalloc(&d, h.size() * sizeof(T));
    if(!h.empty()) cudaMemcpy(d, h.data(), h.size() * sizeof(T), cudaMemcpyHostToDevice);
    return d;
}

int main()
{
    // Demo: lege stub — in de echte plugin geef jij d_points/d_mask/d_labels door.
    std::vector<Vec3f> h_points; // vul met jouw cloud
    u32                N = (u32)h_points.size();

    Vec3f*     d_points = gpu_malloc_copy(h_points);
    u8*        d_mask   = nullptr; // alles actief
    PuntLabel* d_labels = nullptr;
    cudaMalloc(&d_labels, N * sizeof(PuntLabel));
    cudaMemset(d_labels, 0, N * sizeof(PuntLabel));

    OrkestratieParams p;
    FrameStatus       frame;
    DiscoverLog       log;

    bool ok = h_vind_ideaal_frame(d_points, N, d_mask, d_labels, p, frame, log);
    if(!ok)
    {
        std::cout << "[demo] geen ideaal frame gevonden: " << log.verslag << "\n";
        return 0;
    }
    h_print_frame(frame);

    h_rotatie_naar_ideaal(d_points, N, frame);

    h_fine_labeling(d_points, N, d_labels, frame, p);

    cudaFree(d_points);
    cudaFree(d_labels);
    std::cout << "[demo] klaar.\n";
    return 0;
}