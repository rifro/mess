#include "includes.cuh"
#pragma once

// ====== Doel ======
// __host__ levert drie afzonderlijke arrays (x[], y[], z[]) in "nieuwe as-volgorde"
// (na AxisPermutatie op de __host__).
// GPU merge't naar één Vec3f-buffer voor kernels die coalesced 3D willen.
// Later kan GPU weer splitten naar (x[],y[],z[]) zodat __host__ makkelijk terug-permuteert.

// Merge: (x,y,z) → Vec3f
void hMergeXyzNaarVec3(const DeviceBuffer<float>& d_x, const DeviceBuffer<float>& d_y, const DeviceBuffer<float>& d_z,
                       DeviceBuffer<Vec3f>& d_points);

// Split: Vec3f → (x,y,z)
void hSplitVec3NaarXyz(const DeviceBuffer<Vec3f>& d_points, DeviceBuffer<float>& d_x, DeviceBuffer<float>& d_y,
                       DeviceBuffer<float>& d_z);

// ====== Helpers voor AxisPermutatie ======
// Bouw __host__zijde AxisPermutatie-array vanuit drie kolommen (x,y,z) die al
// hernoemd/gepermutateerd zijn.
inline void hAxisPermutatieVul(AxisPermutatie (&ap)[3], const float* nieuwX, const float* nieuwY, const float* nieuwZ,
                               char naamX, char naamY, char naamZ, u32 Vx, u32 Vy, u32 Vz)
{
    ap[0] = AxisPermutatie{nieuwX, naamX, Vx};
    ap[1] = AxisPermutatie{nieuwY, naamY, Vy};
    ap[2] = AxisPermutatie{nieuwZ, naamZ, Vz};
    // Volgorde ap[0]=X, ap[1]=Y, ap[2]=Z is wat GPU verwacht.
}