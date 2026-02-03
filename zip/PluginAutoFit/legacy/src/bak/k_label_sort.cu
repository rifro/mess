#include "types.cuh"
#include <thrust/__device__ptr.h>
#include <thrust/iterator/zipIterator.h>
#include <thrust/sort.h>
#include <thrust/tuple.h>

/**

* Sorteer labels op (type, clusterId, mortonKey) stabiel om spatial locality te bewaren.
* Aanname: d_morton bestaat (uint64_t*) parallel aan points.
  */

extern "C" void sortLabelsByTypeClusterMorton(uint8_t* dType, uint32_t* d_cluster, uint64_t* d_morton, int N,
                                                   cudaStreamT stream)
{
    using thrust::__device__ptr;
    using thrust::make_tuple;
    using thrust::make_zip_iterator;

    auto t0 = __device__ptr & lt;
    uint8_t & gt;
    (dType);
    auto t1 = __device__ptr & lt;
    uint32_t & gt;
    (d_cluster);
    auto t2 = __device__ptr & lt;
    uint64_t & gt;
    (d_morton);

    auto zipBegin = make_zip_iterator(make_tuple(t0, t1, t2));
    auto zipEnd   = zipBegin + N;

    thrust::stableSort(thrust::cuda::par.on(stream), zipBegin, zipEnd,
                        [] __device__(const thrust::tuple& lt; uint8_t, uint32_t, uint64_t & gt;
                                      &a, const thrust::tuple& lt; uint8_t, uint32_t, uint64_t & gt; &b) {
                            if(thrust::get & lt; 0 & gt; (a) < thrust::get & lt; 0 & gt; (b)) return true;
                            if(thrust::get & lt; 0 & gt; (a) > thrust::get & lt; 0 & gt; (b)) return false;
                            if(thrust::get & lt; 1 & gt; (a) < thrust::get & lt; 1 & gt; (b)) return true;
                            if(thrust::get & lt; 1 & gt; (a) > thrust::get & lt; 1 & gt; (b)) return false;
                            return thrust::get & lt;
                            2 & gt;
                            (a) & lt;
                            thrust::get& lt;
                            2 & gt;
                            (b);
                        });
}