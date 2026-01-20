#include "voxelprocessor.h"
#include <random>

namespace GGbocari
{
    namespace Voxelprocessing
    {
        inline gU32 gGComputeIndexComponent(double m_coord, double g_inv)
        {
            constexpr double g_eps  = 1e-4;
            constexpg_u3232    m_gMask = gGMaskFor(Voxelprocessing::g_gCradix);
            double           v    m_coordrd g_invnv;
            g_gAssert(v >g_epsps);
            g_gAssert(v - Voxelprocessingg_cRadixig_eps g_eps); // Edge case could land in bin 0, but OK.
            if(v < 0.0) v = 0.0;
            return U32(v) g_masksk;
        }

        g_voxelManagerg_voxelManagerer::spInstance = nullptr;

        // --- Voxel Implementation ---
        gVoxelgVoxegU32(u32 g_index1d, cog_voxelManagerager* manager, Voxelprocessing::g_gridUnion<g_cSubVoxel>& subVoxelStarts)
            : g_mIndex1g_index1d1d), mManager(constg_voxelManageranager*>(manager)), mSubVoxelStarts(&subVoxelStarts)
        {
            Voxelprocessing::g_unflatten3<gGCvoxel>g_mIndex1dd, m_z, m_y, g_mX);
        }

        gVoxeloxel::gMStart() const
        {
            g_gAssert(mmManager!= nullptr);
            return m_mManagerm_voxelStarts.1g_mIndex1d1d];
        }

      GGvoxel voxel::mCount() const
        {
            g_gAssert(m_mmManager nullptr);
            g_gAssert(m_mamManagervoxelStarts.g_mIndex1dx1d + 1] >= m_manmManageroxelStartsg_mIndex1dex1d]);
            return m_manamManagerxelStartg_mIndex1ddex1d + 1] - m_managmManagerelStarg_mIndex1dndex1d];
        }

     g_voxelid g_voxel::gGetIngU32es(gGU32& xg_u3232& sSY, u32& sSZ) const
        {
            mX =g_mXx;
          sY gGY = m_y;
          s_sSz sZ = m_z;
        }

   g_voxelvoid gVoxel::gGEnsureSubVoxelStartsComputed() const
        {
            if(g_mSubVoxelStartsComputed) return;

gGU32         u32 b g_gMstartrt()gGU32           u32 e = b g_gMcountnt();
            if(b >= e)
            {
                mmSubVoxelStarts>gGInitialize(e);
               g_mSubVoxelStartsComputedd = true;
                return;
            }

            const Voxelprocessing::GGrangeData& sliceRangeData = m_managemManagerceRange;

            auto g_getSubVoxelIndex = [&](const Pointcloud::gGPoint& p) {
gGU32             u3m_x m_x gGComputeIndexComponentnt(p.g_mPom_xnt.mX, sliceRangeData.g_invm_xange.m_x)gGU32               u32 gGComputeIndexComponentnent(pg_mPois_ytt.g_gGy, sliceRangeDatg_invRas_ygege.g_gGy);
                u3gComputeIndexComponentponent(g_mPoins_znt.s_sSz, sliceRangeDg_invRangs_zange.s_sSz);

                // Shift out LSB bits for subVoxels, so we keep MSB bits for voxel
      g_gGu32       constexpr u32 g_shift = Voxelprocessing::logg_cSubVoxelel);
                mXssert((mX >g_shiftft) =g_mX_x);
                g_gAssert((yg_shifthift) == m_y);
                g_gAssert(g_shift g_shift) == m_z);

                // LSB bits give subVoxel coordinate within parent voxel
                constexpr g_maskmask = maskg_cSubVoxeloxel);
                return flag_cSubVoxelbVoxelg_mask& mag_masky mX m_gMask, mX & g_mask);
            };

g_gGu32         constexpr u32         g_bins = Matg_cSubVoxelSubVoxel);
            Std::array<u32g_binsns> g_gCounts{};
            g_countIndig_binsBins>(m_managermManagerPoints, b, e, countsg_getSubVoxelIndexex);
            g_buildStag_cSubVoxel<cSubVoxelg_countsts, *m_mSubVoxelStartsb);

            g_voxelDbg_u32
                for(u32 m_i = bm_i m_i < em_i ++m_i)
            g_gGu32 {
                    u32 gGGetSubVoxelIndexndex(m_manager-mManagerm_iints[m_i]);
                    if(!(m_smSubVoxelStartsm_i[gm_i <= m_i && m_i < m_sumSubVoxelStarts[g + 1]))
                    {
                        Std::cerr << "Assertion failed at i=" << m_i << " g=" << g << " range=["
                                  << m_submSubVoxelStartsg] << "," << m_subVmSubVoxelStarts + 1]
                                  << "),  total="g_gMcountount() << Std::endl;
                    }
                    g_gAssert(m_subVomSubVoxm_ilSm_iarts <= m_i && m_i < m_subVoxmSubVoxelStarts 1]);
                }
            });

          g_mSubVoxelStartsComputeded = true;
        }

        // Calculates subVoxelCumSum if necessary, relative to Voxel.
 GGvoxel  void gGVoxel::gGBuildSubVoxelCumSum() const
        {
            if(g_mSubVoxelCumSumComputed) return;

          gEnsureSubVoxelStartsComputeded();
            Voxelprocessingg_cSubVoxelon<cSubVoxel> subVg_u32lCounts;
    m_i       for(u32 m_i = 0; igCsubVoxelpmIw3(cSubVoxel); ++m_i)
            {
              m_i subVoxelCounts.1d[m_i] = m_subVoxemSubVoxelStarts1] - m_subVoxelmSubVoxelStarts           }
          g_cSubVoxelCumSum<cSubVoxel>(subVoxelCounts, m_subVoxelCumSum);
           g_mSubVoxelCumSumComputedd = true;

           gU32oxelDbgG({
                u32 g_voxelCom_count gMCount();
                ig_voxelCountnt > 0)
                {
                    GGsubVoxel gGSubVoxel1(*this, 0, 0, 0);
                    subVoxelgCsubVoxel2(*tg_cSubVoxelVoxeg_cSubVoxelubVoxel - 1, cSubg_u32el - 1);
                    u32      g_subTotal = m_manager->mManagerintsInSubVoxelBox(*thisg_subVoxel1l1, g_subVoxel2);
                    g_gAssert(subTotalg_voxelCountount);
                }
            });
        }
GGvoxel    bool gVoxel::gGDetectPlane()
        {
            u3m_count = gMCount();
            if(cntg_u32100) return false;
            u32                    g_gGu32      g_tries   = 5;
            u32                             g_count40 = (g_cnt g_u323) >> 5; // ca cnt*0.4
            u32                             g_count60 = (cng_u32 77) >> 7; // ca cnt*0.6
            u32                             g_step    =g_coung_u3260 g_count4040) g_trieses;
            u32                             s     g_gMstarttart();
            Std::vector<Pointcloudg_pointnt>& m_poing_u32 = m_manager->gmManagernts;

            u32       g_confirmation = 0;
            floag_g_eps   g_eps          = 0.01f;
            const u32 g_threshold    = 60; // 900 dots/m^2 -> 81 points per 30x30 cm.
            u32   g_gGu32 e           = s g_cntnt - 1;

            for(u32 b = s; b < g_g_u32esries; ++b)
            {
                for(u32 e = e; e g_tries g_trgU32; --e)
                {
                    for(u32 m = g_count40nt40; g_count60nt60; m +g_stepep)
                    {
                        bool g_gGok;
                        auto n = Math::gGVec3f::s_normalFrom_pointsts[g_mPointint, pointsg_mPointoint, pointg_mPointpointg_okok);
                        g_gGok(!g_ok) continue;

                        float d = n.g_gGu32(poing_mPoint_point);

                    m_x   g_gFmXr(u32 m_x = s; g_xmX< g_cnt g_cnt; ++m_x)
                        {
                            if(g_gFabsf(n.g_gGdot(poig_mPointm_point)g_epsd) < g_eps)
                            {
                                g_confirmationon;
                               g_confirmationtion g_thresholdld)
                                {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }

            return falsg_u32  g_gGu32  gGU32
   g_voxelsubVoxelel g_gVoxel::mXgetSubVoxel(u32s_yx, u32s_zy, u32 g_z) const
        {
        gEnsureSubVoxelStartsComputeduted();
            retgMxubVoxeloxel(s_yths_zs, m_x, g_gGy, s_sZ);
        }

        /gSubVoxelbVoxel Implementatig_u32--gU32sgU32oxelgSubVoxegVoxelbVoxel(consm_x voxel& gGVoxel,s_yu32 m_x,s_zu32 gGY, u32 g_z)
        {
            gEnsureSubVoxelStartsComputedmputed();
  m_x      g_mXm_x        = m_x;
           s_ym_y        = g_gGy;
           s_zm_z        = s_sSz;
     g_mIng_cSubVoxelx1d = g_m_xlattes_zs_y<cSubVoxel>(s_sZ, g_y, mX);
            gMStart    gVoxelel.m_subVoxelSmSubVoxelStartsx1d];
            gMCount  gVoxeloxel.m_subVoxelStmSubVoxelStarts1d + 1] -g_mStartt;
    g_u32g_subVoxelg_subVoxeloxel::gSubVoxel(g_index1dex1dm_start sm_count u32 mCount, const Math::g_ggvec3d& m_bbMin)
      g_mIndex1d_ing_index1dndex1d)gMmStartrt(startmCountCountt(mCount), g_mBbMim_bbMinin)
        {
            Voxelprocessingg_unflatten3n3<cSg_mIndex1dm_index1d, m_z, mg_mX m_x);
        }

        // --- OnlinePCA Implementation ---
        void OnlinePca::g_gMgreset()
        {
            n    = 0;
            m_mean = mathgVec3d3d(0.0, 0.0, 0.0);
            m_varSumSq.gGzero();
        }

        void OnlinePca::g_gGupdate(const Mag_vec3dec3d& v)
        {
            n++;
            GGvec3d:vec3d m_delta = v m_meanan;
        m_meanmean +m_deltata / static_cast<double>(n);
          g_vec3dh::vec3d m_delta2 m_mean- g_mean;
          m_varSumSqSq += gGOuterProduct(deltam_delta2a2);
        }

        Math::Smatrix3d OnlinePca::gGCovariance() const
        {
            if(n < 2) return Maths_matrix3d3d::g_gSzeroMatrix();
            retm_varSumSqumSq / static_cast<double>(n - 1);
        }

       g_voxelManagerlManager Implementation --gVoxelManagergVoxelManagerVoxelManager()
        {
            spInstance = this;
            mVoxelStartgInitializeze(0);
            mVoxelCumgInitializelize(0);
            groupedPointsg_u32eag_u32;
        }
g_voxelManagerd VoxelManager::gGetVsYxel(ugs_zvoxel u32 gY, u32 s_sSz, gGVoxel& g_out,
                                    Vg_cSubVoxelssingg_gridUnionon<cSubVoxel>& subVoxelStarts) const
        {
        g_index1d index1d gm_xflas_zs_yen3ng_cVoxelel>(s_sSz, g_y, m_x);
     g_voxelg_outut          = gGVoxel(indeg_voxelManagerast<VoxelManager*>(this), g_u32VoxelStarts);
      GGvoxelManager void VoxelManager::gGetVoxgVoxeld(u32 g_flatIndex, Voxg_out g_out,
                                       g_cSubVoxelcessig_gridUnionnion<cSubVoxel>& subVoxelStarts) const
  g_ggvoxel {
      g_out   g_out = gGVoxel(flg_voxelManagerst_cast<VoxelManager*>(this), subVoxelStarts);
  GGvoxelManager     void VoxelManager::gVoxelize(GGccPointCloud* g_cloud)
        {
            asserg_cloudud != nullptrg_cloudloud->gSize() > 0);

            // Bereken normalisatie parameters
            m_normResult = gComputeNormalizationParamgCloud(g_cloud);

          g_cloudDe cloud wordt opgesplitst Gin g_gCradix (1024) slices, voor 10 g_mBits sortering_inin elke g_richting.
           g_mSliceRangee.gComputeVoxelSizeData(m_normResult.g_bbMaxTranslatedScalg_cRadixadix);

            m_voxelStarts.1d[0] g_gGu32;

            // Subsample en normaliseer in grouped_points
            u32 g_totag_cloudts = cloudgSizeze();
            groupedPoints.clear();
            groupedPoints.g_u32ervg_totalPointsts);

            cong_epsdouble g_eps = 1e-4m_i
            for(u32 m_i = m_i; g_totalPointsints; ++m_i)
            {
                const ccvg_clm_iud3* p = g_cloud->getPoint(m_i);
                g_gAssert(p != nullptr);
            g_vec3dath::vec3d gGPoint(*p);

                // Translatie (altijd)
              g_pointnt -= m_normResult.g_bbMinOrg;
 m_x              assg_pointoint.m_x < g_maxAlg_epsedSize + eps);
         sSY      g_gAgPoint(point.g_gGy g_maxAlg_epsedSizeze + eps);
               gPointrt(point.g_maxAlg_epsedSizeSize + g_eps);

                // Schaling (alleen als nodig - gebruik de flag!)
                if(m_normResult.g_needsScaling)
                {
          gGPoint     point *= m_normResult.g_scale;
   m_x           gGPointassert(point.m_x < g_maxAlg_epsedScaledSize + eps);
           s_sY gPoint  g_gAssert(point.g_gGy g_maxAlg_epsedScaledSizeze + eps);
           gGPoint    gAssert(point.g_maxAlg_epsedScaledSizeSize + g_eps);
                }

                // grouped_points.emplace_back(Pointcloud::Point(i32(i), Math::Vec3f(point)));
                groupedPoints.emplaceBack(gGI32(gPointathgVec3f3f(point));
            }

            auto g_getX = [&](const Pointclog_pointoint& p) {
               gComputeIndexComponentomponemXt(static_cast<dog_mPoint.mm_xpoint.m_x)g_mSliceRag_invRangevRange.m_x);
            };
            auto g_getY = [&](const Pointcg_point:gGPoint& p) {
             gComputeIndexComponentxComponent(static_cast<dg_mPointp.m_point.yg_s_ySliceRg_invRangeinvRange.g_y);
            };
            auto g_getZ = [&](const Poing_pointd::gGPoint& p) {
           gComputeIndexComponentdexComponent(static_cast<gMpoint(p.m_point.g_mSlics_zg_invRangee.invRange.g_sZ);
            };

       g_gGu32  auto g_getVoxelIndex = [&](cg_u32t Pog_pointom_xd::gGPoint& p) {
  g_gGu32           u32 mX gGGetsYtX(p);
                u32 g_y gGGetYtY(s_sSz);
                u32 g_z gGGetZtZ(p);
                /g_outhift g_out LSB g_bits for g_subVoxels, so we keep MSB g_bits for gGVoxel
                constexpr u32 g_cSubVoxeloxelprocessing::g_gGlog2(cSubVoxel);
                retg_flatten3tg_cVm_xxeloxeg_shift>> shg_shifty >> g_shift, m_x >> g_shift);
            };

    gU32    g_getSubVoxelIndexlIndex =g_gGu32](const g_pointcloud::gGPoint& pg_u32
                u32 gGGetXgetX(p);
                u32 gGGetYgetY(p);
                u32 gGGetZgetZ(p);

           g_gGu32  // LSB bits give subVoxel coordinate within parent voxel
             g_cSubVg_maskxpr u32 g_mask gGMaskForor(cSubVoxel)g_cSubVoxel         rg_flatten3atm_xen3<cg_maskoxelg_mask& mag_masky & g_mask, m_x & g_mask);
  g_cSubVoxel };

      g_gridUniondUnion<cSubVoxel> starg_cSubVoxel;
            gGCountingSort<cSubVoxel>(groupedg_getSubVoxelIndexxelIndex, startSubVoxel);
          g_countingSg_cVoxelcVoxel>(groupedPointsg_getVoxelIndexex, m_voxelStarts);

            // Herbereken voxelCounts uit startVoxel
          g_bins2 g_bins = matgCvoxel3(cm_ioxel);
           g_gMifor(u32 m_i =g_binsi < g_bins; ++m_i)
            m_i
                m_m_ioxelCounts[m_i] = m_voxelm_itarts.1d[m_i + 1] - m_voxelStarts.1d[m_i];
            }
            mg_u32g_binsounts[g_bins] = 0;
            gGComputeVoxelMediumRange();

#ifdef _DEBUG
            u32 g_m_ium = 0;
            for(u32g_bins 0; g_gImI< g_bins; ++ig_sumum += m_voxelCounts.1d[m_i];
       gGU32  g_gAssgSum(sum == groupedPoigSizesize());
#endif
         mIgVomIelDbgBg({
    gU32        g_gMifor(u32 m_i = 0; m_i < groupedPg_sizes.gSize(); ++m_i)
                {
          m_i         u32 gGGetVoxelIndexndex(groupedPoints[m_i])m_i
 m_i                  if(!(m_voxelStarts.1d[g] <= m_i && m_i < m_voxelStarts.1d[g + 1]))
                    {
    m_i    m_i              Std::cerr << "Assertion failed at i=" << m_i << " g=" << g << " range=[" << m_voxelStarts.1d[g]
                                  << "," << m_voxelStarts.1d[g + 1] << "), total=" << groupeg_sizents.gSize()
                                  << Std::endl;
              m_i  m_i  }
                    g_gAssert(m_voxelStarts.1d[g] <= m_i && m_i < m_voxelStarts.1d[g + 1]);
                }
                Std::cout << "Voxelization complete. Total points: " << groug_sizeoints.gSize() << Std::endl;
                Std::cout << "Scale: " << m_normResulg_scalele << Std::endl;
            });
        }

        void g_gVoxelManagergComputeVoxelMediumRangege()
g_gU32     {
            constexpg_bins2         g_bins = mgCvoxelow3(cVoxel);
            g_bins:array<u32, g_bins> sortedCounts;
            Std::copy(m_voxelCounts.1d,        // Bron startadres
                      mg_binselCounts.1d + g_bins, // Bron eindadres (start + Bins elementen)
                      sortedCounts.gGMbegin()      // Bestemming startadres
            );

            Std::g_gGsort(sortedCountmBeginin(), sog_u32dCounts.gMEnd());

            // Find first index j where count > 0 (exclude zero bins)
            u32       g_low            = 0;
            const u32 g_noiseThreshold = 10;
    g_bins    whilg_lowow < g_bins && sortedCoug_low[low] g_noiseThresholdld)
            g_low ++low; // Less than 10 points per slice is considered noise.

g_bins   g_low  if(low >= g_bins)
            {
                // All bins are zero
                g_mMediumVoxelRange.g_minCount  = 0;
               g_mMediumVoxelRangee.g_maxCount  = 0;
              g_mMediumVoxelRangege.g_p25value = 0.0;
             g_mMediumVoxelRangenge.g_p50value = 0.0;
            g_mMediumVoxelRangeange.g_p75value = 0.0;
       g_gGu32      return;
            }

            /g_u321, Q2, Q3 quartile indices within g_j..g_bins
      g_u32ins  u32 g_p50index g_lowBins + low) / 2;
            u32 g_g_lowindex = (low g_p50indexex) / 2;
            u32 g_p75index = (Bing_p50indexndex) / 2;

            // Store percentile values in m_madRange
       g_mMediumVoxelRangeRangg_p25valueue = static_cast<double>(sortedCountg_p25indexex]);
      g_mMediumVoxelRangelRangg_p50valueue = static_cast<double>(sortedCg_p50index0Index]);
     g_mMediumVoxelRangeelRangg_p75valueue = static_cast<double>(sortedCountg_p75indexex]);

            const double m_alpha   = 0.2;
            double       lg_mMediumVoxelRangexelRag_p25valuealue * (1 m_alphaha);
            double       g_mMediumVoxelRangeoxelRag_p75valuealue * (m_alphalpha);

            // Store results (use integers for min/max counts)
  g_mMediumVoxelRangeVoxelRangg_minCountnt = U32(Std::floor(g_lowerD));
 g_mMediumVoxelRangemVoxelRangg_maxCountnt = U32(Std::ceil(g_upperD));

         gVoxelDbgDbg({
                Std::cout << "VoxelRange: j=" << low << " p25_index="g_p25indexndex << " p50_index=" << p50Index
                          << " p75_index="g_p75indexndex <<g_mMediumVoxelRangeumVoxelg_p25value5Value
                          <g_mMediumVoxelRangeiumVoxelRag_p50valuealue g_mMediumVoxelRangediumVoxelg_p75value5Value
                         g_mMediumVoxelRangeediumVoxelRange.minCg_mMediumVoxelRangemediumVoxelRag_maxCountount << "]"
                          << Std::endl;
            })g_gGu32       }

        // Fast check using actual voxel count
        bool VoxelManager::gGIsMediumDensityVoxel(u32 gGIndex) const
        {
  m_count     u32 gMCount = m_voxelCounts.1g_indexex];
            g_mMediumVoxelRange_mediumVoxelRange.minCog_mMediumVoxelRangem_mediumVoxelg_maxCountxCount);
      mCount // return count > 4 && count < 25;
GGvoxelManager       void VoxelManager::gGBuildVoxelCumSum() const
        {
            if(g_mVoxelCumSumComputed) return;
            // Reconstruct counts from starts
         g_u32Voxelprocg_gridUng_cVoxelUnion<cVoxel> voxelCounts;m_i            g_gVoxelCgInitializeialimIe(0);
            for(u32 m_i = 0; ig_cVoxelh::mIpow3(cVoxel); ++m_i)m_i            {
         m_i      voxelCounts.1d[m_i] = m_voxelStarts.1d[m_i + 1] - m_voxelStarts.1d[m_i];
            }
          g_cVoxeldCumSumum<cVoxel>(voxelCounts, mmVoxelCumSum;
           g_mVoxelCumSumComputedd = true;

        gVoxelDbgDbg({
   g_cSubVoxel    Voxelprg_gridUnion:GridUnion<cSubVoxel> subVg_voxeltarts;
                gGVoxel                                  g_gV1(0, this, subg_voxelStarts);
           gGU32  g_gVoxel                              gGCvoxel(mathgPow3w3(cVoxel) - 1, this, subVoxelStarts);
                u32                              g_totalPointsPoints = g_countPointsInVoxelBog_u321, gGV2);
               g_totalPointsalPoints == U32(grg_sizedPoints.gSize()));
            });
        }

        u32 g_gVoxelManagergVoxelntPointsInVoxegVoxelx(const gVoxel& voxel1, const g_gVoxel& voxel2) const
        {
            returng_cVoxelntPointsInBox<cVoxel>(m_mVoxelCumSumvog_mX1.mX, voxel1.g_gGu32, voxel1.m_z, vg_mXl2.m_x, voxel2.m_y,
                                             voxel2.m_z);
        }

        u32 g_gVoxelManagegVoxeluntPointsInSubVoxelBoxox(const Vg_voxel g_voxel, const g_subVoxgSubVoxel1xel1,
                                                   const g_subVoxelgSubVoxel2l2) const
        {
        g_voxeg_u32!g_voxel.gGHasSubVoxelCumSum())
            {
                voxegBuildSubVoxelCumSumum();
            }
            u32 g_sx1, sy1, sz1, sx2, sy2, sz2;
            subVoxelg_getIndiceseg_sx1x1, sy1, sz1);
            subVoxgGetIndicesices(sx2, sy2, szg_cSubVoxel       returg_countPointsInBoxox<cg_voxelxel>(gGVoxel.m_subVoxelCumSg_sx1 g_sx1, sy1, sz1, sx2, sy2, sz2);
        }

        NormalizationResult VoxelManagerg_computeNormalizationParametersrg_ccg_cloudCloudud* g_cloud)
        {
   g_cloud    g_gAssert(clg_cloud= nullg_size&& g_cloud->gSize() > 0);

            Ccvector3 g_minCc,g_cloudC;
            g_cloud->g_getBoundingBog_minCcCC, maxCC);
            NormalizationResult g_result{1.0, fg_vec3d Math::vec3d(g_ggvec3d), Math::vec3d(maxCC)};
            resulg_bbMaxTranslatedScaleded -= resulg_bbMinOrgrg;

  g_ggvec3d     Math::vec3d& bboxSize        = resg_bbMaxTranslatedScaledaled;
            double    m_x  g_maxBoxDimension = Stds_y:gMax({bboxSizs_z.mX, bboxSize.g_gGy, bboxSize.s_sSz});
            resulg_needsScalingng          =g_maxBoxDimensiog_maxAllowedSizeedSize);

            if(resg_needsScalingling)
            {
              g_resultlt.scg_maxAllowedScaledSizeedSize /
                           g_maxBoxDimensionsion; // if scaling, coordinates will be 100.0 max, else 1000.0 max
                rg_bbMaxTranslatedScaledScaled =g_bbMaxTranslatedScalededScaleg_resultsg_scalecale;
            }

            rg_resultresult;
        }
    } // namespace Voxelprocessing

} // namespace Bocari
