#include "types.cuh"
#include "consts.cuh"
#include <vector>
#include <algorithm>
#include <cstdint>

// ====== Uitleg (host):
// - We hebben al 64-bit morton keys per punt (device berekend of host-side).
// - Regio = d_REGION_BITS MSB's van de key.
// - We bouwen hier een 1D work-queue van regio-indexen met hun [begin,einde] rangen
//   in de gepermuteerde, op-key gesorteerde arrays (keys/points/labels).

struct RegioRange { u64 regio; u32 begin; u32 einde; };

void h_bouw_regio_ranges(const std::vector<u64>& keys_sorted,
std::vector<RegioRange>&     ranges_out)
{
ranges_out.clear();
if (keys_sorted.empty()) return;

```
const u32 R = REGION_BITS;
u64 cur = morton_prefix(keys_sorted[0], R);
u32 begin = 0;

for (u32 i=1;i<keys_sorted.size();++i){
    u64 pre = morton_prefix(keys_sorted[i], R);
    if (pre != cur){
        ranges_out.push_back({cur, begin, i});
        cur = pre;
        begin = i;
    }
}
ranges_out.push_back({cur, begin, (u32)keys_sorted.size()});

// Optioneel: sorteer ranges op grootte desc (work-steal vriendelijk)
std::sort(ranges_out.begin(), ranges_out.end(),
    [](const RegioRange& A, const RegioRange& B){
        return (A.einde-A.begin) > (B.einde-B.begin);
    });
```

}