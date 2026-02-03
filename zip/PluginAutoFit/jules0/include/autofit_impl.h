#pragma once
#include "includes.h"
#include <vector>
#include <span>

class ccPointCloud;
class ccMainAppInterface;

struct AxisChunk {
    std::span<float> coords;
    char originalAxisName;
};

class AutofitImpl
{
public:
    explicit AutofitImpl();

    void performFit(ccMainAppInterface* app, ccPointCloud* selectedCloud);

private:
    void doProcessing();
    
    bool getSafeVramLimit(size_t& safeVramLimit);
    void recursiveSplit(std::vector<AxisChunk>& chunks, int depth);

    ccMainAppInterface* m_app           = nullptr;
    ccPointCloud*       m_selectedCloud = nullptr;
};
