#include "voxelprocessor.h"
#include <random>

namespace Bocari
{
    namespace Voxelprocessing
    {
        inline u32 computeIndexComponent(double coord, double inv)
        {
            constexpr double eps  = 1e-4;
            constexpr u32    mask = maskFor(Voxelprocessing::cRadix);
            double           v    = coord * inv;
            assert(v > -eps);
            assert(v - Voxelprocessing::cRadix < eps); // Edge case could land in bin 0, but OK.
            if(v < 0.0) v = 0.0;
            return U32(v) & mask;
        }

        VoxelManager* VoxelManager::spInstance = nullptr;

        // --- Voxel Implementation ---
        Voxel::Voxel(u32 index1d, const VoxelManager* manager, Voxelprocessing::GridUnion<cSubVoxel>& subVoxelStarts)
            : m_index1d(index1d), m_manager(const_cast<VoxelManager*>(manager)), m_subVoxelStarts(&subVoxelStarts)
        {
            Voxelprocessing::unflatten3<cVoxel>(m_index1d, m_z, m_y, m_x);
        }

        u32 Voxel::start() const
        {
            assert(m_manager != nullptr);
            return m_manager->m_voxelStarts.1d[m_index1d];
        }

        u32 Voxel::count() const
        {
            assert(m_manager != nullptr);
            assert(m_manager->m_voxelStarts.1d[m_index1d + 1] >= m_manager->m_voxelStarts.1d[m_index1d]);
            return m_manager->m_voxelStarts.1d[m_index1d + 1] - m_manager->m_voxelStarts.1d[m_index1d];
        }

        void Voxel::getIndices(u32& x, u32& y, u32& z) const
        {
            x = m_x;
            y = m_y;
            z = m_z;
        }

        void Voxel::ensureSubVoxelStartsComputed() const
        {
            if(m_subVoxelStartsComputed) return;

            u32 b = start();
            u32 e = b + count();
            if(b >= e)
            {
                m_subVoxelStarts->initialize(e);
                m_subVoxelStartsComputed = true;
                return;
            }

            const Voxelprocessing::RangeData& sliceRangeData = m_manager->m_sliceRange;

            auto getSubVoxelIndex = [&](const Pointcloud::Point& p) {
                u32 x = computeIndexComponent(p.m_point.x, sliceRangeData.invRange.x);
                u32 y = computeIndexComponent(p.m_point.y, sliceRangeData.invRange.y);
                u32 z = computeIndexComponent(p.m_point.z, sliceRangeData.invRange.z);

                // Shift out LSB bits for subVoxels, so we keep MSB bits for voxel
                constexpr u32 shift = Voxelprocessing::log2(cSubVoxel);
                assert((x >> shift) == m_x);
                assert((y >> shift) == m_y);
                assert((z >> shift) == m_z);

                // LSB bits give subVoxel coordinate within parent voxel
                constexpr u32 mask = maskFor(cSubVoxel);
                return flatten3<cSubVoxel>(z & mask, y & mask, x & mask);
            };

            constexpr u32         Bins = Math::pow3(cSubVoxel);
            std::array<u32, Bins> counts{};
            countIndices<Bins>(m_manager->groupedPoints, b, e, counts, getSubVoxelIndex);
            buildStartIndices<cSubVoxel>(counts, *m_subVoxelStarts, b);

            VOXEL_DBG({
                for(u32 i = b; i < e; ++i)
                {
                    u32 g = getSubVoxelIndex(m_manager->groupedPoints[i]);
                    if(!(m_subVoxelStarts->1d[g] <= i && i < m_subVoxelStarts->1d[g + 1]))
                    {
                        std::cerr << "Assertion failed at i=" << i << " g=" << g << " range=["
                                  << m_subVoxelStarts->1d[g] << "," << m_subVoxelStarts->1d[g + 1]
                                  << "),  total=" << count() << std::endl;
                    }
                    assert(m_subVoxelStarts->1d[g] <= i && i < m_subVoxelStarts->1d[g + 1]);
                }
            });

            m_subVoxelStartsComputed = true;
        }

        // Calculates subVoxelCumSum if necessary, relative to Voxel.
        void Voxel::buildSubVoxelCumSum() const
        {
            if(m_subVoxelCumSumComputed) return;

            ensureSubVoxelStartsComputed();
            Voxelprocessing::GridUnion<cSubVoxel> subVoxelCounts;
            for(u32 i = 0; i < Math::pow3(cSubVoxel); ++i)
            {
                subVoxelCounts.1d[i] = m_subVoxelStarts->1d[i + 1] - m_subVoxelStarts->1d[i];
            }
            buildCumSum<cSubVoxel>(subVoxelCounts, m_subVoxelCumSum);
            m_subVoxelCumSumComputed = true;

            VOXEL_DBG({
                u32 voxelCount = count();
                if(voxelCount > 0)
                {
                    SubVoxel subVoxel1(*this, 0, 0, 0);
                    SubVoxel subVoxel2(*this, cSubVoxel - 1, cSubVoxel - 1, cSubVoxel - 1);
                    u32      subTotal = m_manager->countPointsInSubVoxelBox(*this, subVoxel1, subVoxel2);
                    assert(subTotal == voxelCount);
                }
            });
        }

        bool Voxel::detectPlane()
        {
            u32 cnt = count();
            if(cnt < 100) return false;
            u32                             tries   = 5;
            u32                             count40 = (cnt * 13) >> 5; // ca cnt*0.4
            u32                             count60 = (cnt * 77) >> 7; // ca cnt*0.6
            u32                             step    = (count60 - count40) / tries;
            u32                             s       = start();
            std::vector<Pointcloud::Point>& points  = m_manager->groupedPoints;

            u32       confirmation = 0;
            float     eps          = 0.01f;
            const u32 threshold    = 60; // 900 dots/m^2 -> 81 points per 30x30 cm.
            u32       e           = s + cnt - 1;

            for(u32 b = s; b < s + tries; ++b)
            {
                for(u32 e = e; e > e - tries; --e)
                {
                    for(u32 m = s + count40; m < count60; m += step)
                    {
                        bool ok;
                        auto n = Math::Vec3f::normalFrom(points[b].m_point, points[e].m_point, points[m].m_point, ok);
                        if(!ok) continue;

                        float d = n.dot(points[b].m_point);

                        for(u32 x = s; x < s + cnt; ++x)
                        {
                            if(fabsf(n.dot(points[x].m_point) - d) < eps)
                            {
                                ++confirmation;
                                if(confirmation > threshold)
                                {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }

            return false;
        }

        SubVoxel Voxel::getSubVoxel(u32 x, u32 y, u32 z) const
        {
            ensureSubVoxelStartsComputed();
            return SubVoxel(*this, x, y, z);
        }

        // --- SubVoxel Implementation ---
        SubVoxel::SubVoxel(const Voxel& voxel, u32 x, u32 y, u32 z)
        {
            voxel.ensureSubVoxelStartsComputed();
            m_x        = x;
            m_y        = y;
            m_z        = z;
            m_index1d = flatten3<cSubVoxel>(z, y, x);
            m_start    = voxel.m_subVoxelStarts->1d[m_index1d];
            m_count    = voxel.m_subVoxelStarts->1d[m_index1d + 1] - m_start;
        }

        SubVoxel::SubVoxel(u32 index1d, u32 start, u32 count, const Math::Vec3d& bbMin)
            : m_index1d(index1d), m_start(start), m_count(count), m_bbMin(bbMin)
        {
            Voxelprocessing::unflatten3<cSubVoxel>(m_index1d, m_z, m_y, m_x);
        }

        // --- OnlinePCA Implementation ---
        void OnlinePca::reset()
        {
            n    = 0;
            mean = Math::Vec3d(0.0, 0.0, 0.0);
            varSumSq.zero();
        }

        void OnlinePca::update(const Math::Vec3d& v)
        {
            n++;
            Math::Vec3d delta = v - mean;
            mean += delta / static_cast<double>(n);
            Math::Vec3d delta2 = v - mean;
            varSumSq += outerProduct(delta, delta2);
        }

        Math::Matrix3d OnlinePca::covariance() const
        {
            if(n < 2) return Math::Matrix3d::zeroMatrix();
            return varSumSq / static_cast<double>(n - 1);
        }

        // --- VoxelManager Implementation ---
        VoxelManager::VoxelManager()
        {
            spInstance = this;
            m_voxelStarts.initialize(0);
            m_voxelCumSum.initialize(0);
            groupedPoints.clear();
        }

        void VoxelManager::getVoxel(u32 x, u32 y, u32 z, Voxel& out,
                                    Voxelprocessing::GridUnion<cSubVoxel>& subVoxelStarts) const
        {
            u32 index1d = flatten3<cVoxel>(z, y, x);
            out          = Voxel(index1d, const_cast<VoxelManager*>(this), subVoxelStarts);
        }

        void VoxelManager::getVoxelById(u32 flatIndex, Voxel& out,
                                        Voxelprocessing::GridUnion<cSubVoxel>& subVoxelStarts) const
        {
            out = Voxel(flatIndex, const_cast<VoxelManager*>(this), subVoxelStarts);
        }

        void VoxelManager::voxelize(ccPointCloud* cloud)
        {
            assert(cloud != nullptr && cloud->size() > 0);

            // Bereken normalisatie parameters
            m_normResult = computeNormalizationParameters(cloud);

            // De cloud wordt opgesplitst in c_radix (1024) slices, voor 10 bits sortering in elke richting.
            m_sliceRange.computeVoxelSizeData(m_normResult.bbMaxTranslatedScaled, cRadix);

            m_voxelStarts.1d[0] = 0;

            // Subsample en normaliseer in grouped_points
            u32 totalPoints = cloud->size();
            groupedPoints.clear();
            groupedPoints.reserve(totalPoints);

            const double eps = 1e-4;
            for(u32 i = 0; i < totalPoints; ++i)
            {
                const CCVector3* p = cloud->getPoint(i);
                assert(p != nullptr);
                Math::Vec3d point(*p);

                // Translatie (altijd)
                point -= m_normResult.bbMinOrg;
                assert(point.x < maxAllowedSize + eps);
                assert(point.y < maxAllowedSize + eps);
                assert(point.z < maxAllowedSize + eps);

                // Schaling (alleen als nodig - gebruik de flag!)
                if(m_normResult.needsScaling)
                {
                    point *= m_normResult.scale;
                    assert(point.x < maxAllowedScaledSize + eps);
                    assert(point.y < maxAllowedScaledSize + eps);
                    assert(point.z < maxAllowedScaledSize + eps);
                }

                // grouped_points.emplace_back(Pointcloud::Point(i32(i), Math::Vec3f(point)));
                groupedPoints.emplaceBack(i32(i), Math::Vec3f(point));
            }

            auto getX = [&](const Pointcloud::Point& p) {
                return computeIndexComponent(static_cast<double>(p.m_point.x), m_sliceRange.invRange.x);
            };
            auto getY = [&](const Pointcloud::Point& p) {
                return computeIndexComponent(static_cast<double>(p.m_point.y), m_sliceRange.invRange.y);
            };
            auto getZ = [&](const Pointcloud::Point& p) {
                return computeIndexComponent(static_cast<double>(p.m_point.z), m_sliceRange.invRange.z);
            };

            auto getVoxelIndex = [&](const Pointcloud::Point& p) {
                u32 x = getX(p);
                u32 y = getY(p);
                u32 z = getZ(p);
                // Shift out LSB bits for subVoxels, so we keep MSB bits for voxel
                constexpr u32 shift = Voxelprocessing::log2(cSubVoxel);
                return flatten3<cVoxel>(z >> shift, y >> shift, x >> shift);
            };

            auto getSubVoxelIndex = [&](const Pointcloud::Point& p) {
                u32 x = getX(p);
                u32 y = getY(p);
                u32 z = getZ(p);

                // LSB bits give subVoxel coordinate within parent voxel
                constexpr u32 mask = maskFor(cSubVoxel);
                return flatten3<cSubVoxel>(z & mask, y & mask, x & mask);
            };

            GridUnion<cSubVoxel> startSubVoxel;
            countingSort<cSubVoxel>(groupedPoints, getSubVoxelIndex, startSubVoxel);
            countingSort<cVoxel>(groupedPoints, getVoxelIndex, m_voxelStarts);

            // Herbereken voxelCounts uit startVoxel
            u32 Bins = Math::pow3(cVoxel);
            for(u32 i = 0; i < Bins; ++i)
            {
                m_voxelCounts[i] = m_voxelStarts.1d[i + 1] - m_voxelStarts.1d[i];
            }
            m_voxelCounts[Bins] = 0;
            computeVoxelMediumRange();

#ifdef _DEBUG
            u32 sum = 0;
            for(u32 i = 0; i < Bins; ++i) sum += m_voxelCounts.1d[i];
            assert(sum == groupedPoints.size());
#endif
            VOXEL_DBG({
                for(u32 i = 0; i < groupedPoints.size(); ++i)
                {
                    u32 g = getVoxelIndex(groupedPoints[i]);
                    if(!(m_voxelStarts.1d[g] <= i && i < m_voxelStarts.1d[g + 1]))
                    {
                        std::cerr << "Assertion failed at i=" << i << " g=" << g << " range=[" << m_voxelStarts.1d[g]
                                  << "," << m_voxelStarts.1d[g + 1] << "), total=" << groupedPoints.size()
                                  << std::endl;
                    }
                    assert(m_voxelStarts.1d[g] <= i && i < m_voxelStarts.1d[g + 1]);
                }
                std::cout << "Voxelization complete. Total points: " << groupedPoints.size() << std::endl;
                std::cout << "Scale: " << m_normResult.scale << std::endl;
            });
        }

        void VoxelManager::computeVoxelMediumRange()
        {
            constexpr u32         Bins = Math::pow3(cVoxel);
            std::array<u32, Bins> sortedCounts;
            std::copy(m_voxelCounts.1d,        // Bron startadres
                      m_voxelCounts.1d + Bins, // Bron eindadres (start + Bins elementen)
                      sortedCounts.begin()      // Bestemming startadres
            );

            std::sort(sortedCounts.begin(), sortedCounts.end());

            // Find first index j where count > 0 (exclude zero bins)
            u32       low            = 0;
            const u32 noiseThreshold = 10;
            while(low < Bins && sortedCounts[low] < noiseThreshold)
                ++low; // Less than 10 points per slice is considered noise.

            if(low >= Bins)
            {
                // All bins are zero
                m_mediumVoxelRange.minCount  = 0;
                m_mediumVoxelRange.maxCount  = 0;
                m_mediumVoxelRange.p25Value = 0.0;
                m_mediumVoxelRange.p50Value = 0.0;
                m_mediumVoxelRange.p75Value = 0.0;
                return;
            }

            // Q1, Q2, Q3 quartile indices within j..Bins
            u32 p50Index = (Bins + low) / 2;
            u32 p25Index = (low + p50Index) / 2;
            u32 p75Index = (Bins + p50Index) / 2;

            // Store percentile values in m_madRange
            m_mediumVoxelRange.p25Value = static_cast<double>(sortedCounts[p25Index]);
            m_mediumVoxelRange.p50Value = static_cast<double>(sortedCounts[p50Index]);
            m_mediumVoxelRange.p75Value = static_cast<double>(sortedCounts[p75Index]);

            const double alpha   = 0.2;
            double       lowerD = m_mediumVoxelRange.p25Value * (1 - alpha);
            double       upperD = m_mediumVoxelRange.p75Value * (1 + alpha);

            // Store results (use integers for min/max counts)
            m_mediumVoxelRange.minCount = U32(std::floor(lowerD));
            m_mediumVoxelRange.maxCount = U32(std::ceil(upperD));

            VOXEL_DBG({
                std::cout << "VoxelRange: j=" << low << " p25_index=" << p25Index << " p50_index=" << p50Index
                          << " p75_index=" << p75Index << " p25=" << m_mediumVoxelRange.p25Value
                          << " p50=" << m_mediumVoxelRange.p50Value << " p75=" << m_mediumVoxelRange.p75Value
                          << " => [" << m_mediumVoxelRange.minCount << ", " << m_mediumVoxelRange.maxCount << "]"
                          << std::endl;
            });
        }

        // Fast check using actual voxel count
        bool VoxelManager::isMediumDensityVoxel(u32 index) const
        {
            u32 count = m_voxelCounts.1d[index];
            return (count >= m_mediumVoxelRange.minCount) && (count <= m_mediumVoxelRange.maxCount);
            // return count > 4 && count < 25;
        }

        void VoxelManager::buildVoxelCumSum() const
        {
            if(m_voxelCumSumComputed) return;
            // Reconstruct counts from starts
            Voxelprocessing::GridUnion<cVoxel> voxelCounts;
            voxelCounts.initialize(0);
            for(u32 i = 0; i < Math::pow3(cVoxel); ++i)
            {
                voxelCounts.1d[i] = m_voxelStarts.1d[i + 1] - m_voxelStarts.1d[i];
            }
            buildCumSum<cVoxel>(voxelCounts, m_voxelCumSum);
            m_voxelCumSumComputed = true;

            VOXEL_DBG({
                Voxelprocessing::GridUnion<cSubVoxel> subVoxelStarts;
                Voxel                                  v1(0, this, subVoxelStarts);
                Voxel                                  v2(Math::pow3(cVoxel) - 1, this, subVoxelStarts);
                u32                                    totalPoints = countPointsInVoxelBox(v1, v2);
                assert(totalPoints == U32(groupedPoints.size()));
            });
        }

        u32 VoxelManager::countPointsInVoxelBox(const Voxel& voxel1, const Voxel& voxel2) const
        {
            return countPointsInBox<cVoxel>(m_voxelCumSum, voxel1.m_x, voxel1.m_y, voxel1.m_z, voxel2.m_x, voxel2.m_y,
                                             voxel2.m_z);
        }

        u32 VoxelManager::countPointsInSubVoxelBox(const Voxel& voxel, const SubVoxel& subVoxel1,
                                                   const SubVoxel& subVoxel2) const
        {
            if(!voxel.hasSubVoxelCumSum())
            {
                voxel.buildSubVoxelCumSum();
            }
            u32 sx1, sy1, sz1, sx2, sy2, sz2;
            subVoxel1.getIndices(sx1, sy1, sz1);
            subVoxel2.getIndices(sx2, sy2, sz2);
            return countPointsInBox<cSubVoxel>(voxel.m_subVoxelCumSum, sx1, sy1, sz1, sx2, sy2, sz2);
        }

        NormalizationResult VoxelManager::computeNormalizationParameters(ccPointCloud* cloud)
        {
            assert(cloud != nullptr && cloud->size() > 0);

            CCVector3 minCC, maxCC;
            cloud->getBoundingBox(minCC, maxCC);
            NormalizationResult result{1.0, false, Math::Vec3d(minCC), Math::Vec3d(maxCC)};
            result.bbMaxTranslatedScaled -= result.bbMinOrg;

            Math::Vec3d& bboxSize        = result.bbMaxTranslatedScaled;
            double       maxBoxDimension = std::max({bboxSize.x, bboxSize.y, bboxSize.z});
            result.needsScaling          = (maxBoxDimension > maxAllowedSize);

            if(result.needsScaling)
            {
                result.scale = maxAllowedScaledSize /
                               maxBoxDimension; // if scaling, coordinates will be 100.0 max, else 1000.0 max
                result.bbMaxTranslatedScaled = result.bbMaxTranslatedScaled * result.scale;
            }

            return result;
        }
    } // namespace Voxelprocessing

} // namespace Bocari
