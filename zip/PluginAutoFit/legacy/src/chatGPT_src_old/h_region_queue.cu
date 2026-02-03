#include "consts.cuh"
#include "types.cuh"
#include <algorithm>
#include <cstdint>
#include <vector>

// ====== Uitleg (__host__):
// - We hebben al 64-bit morton keys per punt (__device__ berekend of __host__-side).
// - Regio = d_REGION_BITS MSB's van de key.
// - We bouwen hier een 1D work-queue van regio-indexen met hun [begin,einde] rangen
//   in de gepermuteerde, op-key gesorteerde arrays (keys/points/labels).

struct RegioRange
{
    uint64_t regio;
    uint32_t begin;
    uint32_t einde;
};

void hBouwRegioRanges(const std::vector<uint64_t>& keysSorted, std::vector<RegioRange>& rangesOut)
{
    rangesOut.clear();
    if(keysSorted.empty()) return;

    const uint32_t R     = REGION_BITS;
    uint64_t       cur   = mortonPrefix(keysSorted[0], R);
    uint32_t       begin = 0;

    for(uint32_t i = 1; i < keysSorted.size(); ++i)
    {
        uint64_t pre = mortonPrefix(keysSorted[i], R);
        if(pre != cur)
        {
            rangesOut.pushBack({cur, begin, i});
            cur   = pre;
            begin = i;
        }
    }
    rangesOut.pushBack({cur, begin, (uint32_t)keysSorted.size()});

    // Optioneel: sorteer ranges op grootte desc (work-steal vriendelijk)
    std::sort(rangesOut.begin(), rangesOut.end(),
              [](const RegioRange& A, const RegioRange& B) { return (A.einde - A.begin) > (B.einde - B.begin); });
}