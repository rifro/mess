// include/AutofitImpl.h
#pragma once
#include "autofit.h"
#include "includes.h"
#include "strict.h"
class ccPointCloud;
class ccMainAppInterface;

struct OptimizationParams
{
    float   startAngle;
    float   endAngle;
    float   step;
    int     strips;
    QString phaseName;
};

struct OptimizationResult
{
    float                         bestAngle    = 0.0f;
    float                         bestVariance = -1.0f;
    std::unique_ptr<ccPointCloud> bestTransformedCloud;
};

struct AxisAccess {
    char name;  // 'x', 'y', 'z'
    std::vector<float> data;  // Owned coords
    u32 V;  // Subdivisions
    std::function<float(const CCVector3*)> getter;  // Initial fill only
};

class AutofitImpl
{
public:
    explicit AutofitImpl();

    void performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud); // Implementatie zonder Qt-signalen/slots
    void doOptimizeFrame1();
    std::tuple<float, float> doOptimizeFrame2();

private:
    void doProcessing();
    
    bool getSafeVramLimit(size_t& safeVramLimit);
    void recursiveSplit(std::vector<AxisChunk>& chunks, int depth);

    ccMainAppInterface* m_app           = nullptr;
    ccPointCloud*       m_selectedCloud = nullptr;
    // Subsampling methodes
    std::unique_ptr<ccPointCloud> voxelSubsample(ccPointCloud* inputCloud, int32_t pointCount);
    std::unique_ptr<ccPointCloud> spatialSubsample(ccPointCloud* inputCloud, PointCoordinateType minDistance);
    std::unique_ptr<ccPointCloud> cloneReferenceCloud(CCCoreLib::ReferenceCloud* ref);


private:
    // helpers (inline or separate .cpp)
    void makeSquareMethod(const ccPointCloud& cloud, array<AxisAccess, 3>& axes);
    void h_mergeSoaToFloat3(const array<vector<float>*, 3>& soa, vector<float3>& out);
    void h_splitFloat3ToSoa(DeviceBuffer<float3>& d_in, DeviceBuffer<float>& d_outX, DeviceBuffer<float>& d_outY, DeviceBuffer<float>& d_outZ, size_t n);
    void h_rotatieQuatApply(DeviceBuffer<float3>& d_points, const quat& q, size_t n);
    // Stubs (expand with kernels)
    void h_voxeliseerMorton(DeviceBuffer<float3>& d_points, DeviceBuffer<uint64_t>& d_keys, DeviceBuffer<uint32_t>& d_index, DeviceBuffer<float3>& d_pointsSorted, DeviceBuffer<u32>& d_voxelBereiken, size_t n, const mortonCfg& cfg);
    void h_noiseBoeren(DeviceBuffer<float3>& d_pointsSorted, DeviceBuffer<u8>& d_labels);
    void h_planeVoteCurrentFrame(DeviceBuffer<float3>& d_pointsSorted, DeviceBuffer<u8>& d_labels, CandidateAxes& axes);
    void h_planeVoteBruteforce(DeviceBuffer<float3>& d_pointsSorted, DeviceBuffer<u8>& d_labels, CandidateAxes& axes);
    void h_pipeVoteBruteforce(DeviceBuffer<float3>& d_pointsSorted, DeviceBuffer<u8>& d_labels, CandidateAxes& axes);
    void h_fineSlabsCylLabel(DeviceBuffer<float3>& d_pointsSorted, DeviceBuffer<u8>& d_labels);
    Quat maakQuatNaarIdeaalFrame(const CandidateAxes& axes);  // Bocari func stub
    float3 quatToEuler(const quat& q);  // debug helper
};
