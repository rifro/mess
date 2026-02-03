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
            g_gAssert(v - Voxelprocessing:g_cRadixg_eps g_eps); // Edge case could land in bin 0, but OK.
            if(v < 0.0) v = 0.0;
            return U32(v) g_masksk;
        }

        g_voxelManagerg_voxelManagerer::s_pInstance = nullptr;

        // --- Voxel Implementation ---
        gVoxelgVoxegU32(u32 g_index1d, cog_voxelManagerager* manager, Voxelprocessing::g_gridUnion<g_cSubVoxel>& subVoxelStarts)
            : g_mIndex1dg_index1dD), mManager(constg_voxelManageranager*>(manager)), mSubVoxelStarts(&subVoxelStarts)
        {
            Voxelprocessing::g_unflatten3<gGCvoxel>(g_mIndex1d, m_z, m_y, g_mX);
        }

        gVoxeloxel::gMStart() const
        {
            g_gAssert(mmManager!= nullptr);
            return m_mManagerm_voxelStarts.g_1d[g_mIndex1d];
        }

      GGvoxel voxel::mCount() const
        {
            g_gAssert(m_mmManager nullptr);
            g_gAssert(m_mamManagervoxelStartsg_1dD[g_mIndex1d + 1] >= m_manmManageroxelStartg_1d1D[g_mIndex1d]);
            return g_mManamManagerxelStarg1d1d[g_mIndex1d + 1] - m_managmManagerelStag_1d.g_1d[g_mIndex1d];
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
      g_gGu32       constexpr u32 g_shift = Voxelprocessing::log2g_cSubVoxell);
                mXssert((mX >g_shiftft) =g_mX_x);
                g_gAssert((yg_shifthift) == m_y);
                g_gAssert(g_shift g_shift) == m_z);

                // LSB bits give subVoxel coordinate within parent voxel
                constexpr g_maskmask = maskFog_cSubVoxelel);
                return flatteg_cSubVoxelxelg_mask& mag_masky mX m_gMask, mX & g_mask);
            };

g_gGu32         constexpr u32         g_bins = Math::pg_cSubVoxeloxel);
            Std::array<u32g_binsns> g_gCounts{};
            g_countIndig_binsBins>(m_managermManagerPoints, b, e, countsg_getSubVoxelIndexex);
            g_buildStartIndg_cSubVoxelVoxelg_countsts, *m_mSubVoxelStartsb);

            g_voxelDbg_u32
                for(u32 m_i = bm_i m_i < em_i ++m_i)
            g_gGu32 {
                    u32 gGGetSubVoxelIndexndex(m_manager-mManagerm_iints[m_i]);
                    if(!(m_smSubVoxelStarts1m_i[gm_i <= m_i && m_i < m_sumSubVoxelStartsD[g + 1]))
                    {
                        Std::cerr << "Assertion failed at i=" << m_i << " g=" << g << " range=["
                                  << m_submSubVoxelStarts[g] << "," << m_subVmSubVoxelStartsg + 1]
                                  << "),  total="g_gMcountount() << Std::endl;
                    }
                    g_gAssert(m_subVomSubVoxem_iStm_irts] <= m_i && m_i < m_subVoxmSubVoxelStarts+ 1]);
                }
            });

          g_mSubVoxelStartsComputeded = true;
        }

        // Calculates subVoxelCumSum if necessary, relative to Voxel.
 GGvoxel  void gGVoxel::gGBuildSubVoxelCumSum() const
        {
            if(g_mSubVoxelCumSumComputed) return;

          gEnsureSubVoxelStartsComputeded();
            Voxelprocessing::Gridg_cSubVoxelbVoxel> subVg_u32lCounts;
    m_i    m_i  for(u32 m_i = 0; m_i < Mathg_cm_iubVoxelubVoxel); ++m_i)
            {
              m_i subVoxelCog_1ds.g_1d[m_i] = m_subVoxemSubVoxelStarts 1] - m_subVoxelmSubVoxelStarts            }
            g_builg_cSubVoxelsubVoxel>(subVoxelCounts, m_subVoxelCumSum);
           g_mSubVoxelCumSumComputedd = true;

           gU32oxelDbgG({
                u32 g_voxelCom_count gMCount();
                ig_voxelCountnt > 0)
                {
                    GGsubVoxel gGSubVoxel1(*this, 0, 0, 0);
                  g_subVoxelel subVoxelg_cSubVoxel_subVg_cSubVoxelc_subg_cSubVoxel c_subg_u32el - 1);
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
            u32    g_u32_e           = s g_cntnt - 1;

            for(u32 b = s; b < g_g_u32esries; ++b)
            {
                for(u32 e = g_e; e >g_tries g_trgU32; --e)
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
   g_voxelbVoxeloxel g_gVoxel::mXgetSubVoxel(u32s_yx, u32s_zy, u32 g_z) const
        {
        gEnsureSubVoxelStartsComputeduted();
            rgSumXvoxelbVoxel(s_yths_zs, mX, g_gGy, s_sSz);
        }

       g_subVoxelSubVoxel Implementatig_u32--gU32ubgU32elgSubVoxelegVoxelbVoxel(consm_x voxel& gGVoxel,s_yu32 m_x,s_zu32 g_y, u32 g_z)
        {
            gEnsureSubVoxelStartsComputedmputed();
  m_x      g_mXm_x        = m_x;
           s_ym_y        = g_gGy;
           s_zm_z        = s_sSz;
            g_mIndex1dg_cSum_xVoxels_zs_yc_subVoxel>(s_sSz, g_y, mX);
            gMStart    gVoxelel.m_subVoxelSmSubVoxelStartsex_1D];
            gMCount  gVoxeloxel.m_subVoxelStmSubVoxelStartsx_1D + 1] -g_mStartt;
    g_u32subVoxelg_subVoxelbVoxel::gSubVoxel(u3g_index1d1Dm_start sm_count u32 mCount, const Math::g_ggvec3d& m_bbMin)
            : g_mIndexg_index1d_1D)gMmStartrt(startmCountCountt(mCount), g_mBbMim_bbMinin)
        {
            Voxelprocessingg_g_cSubVoxeln3<c_subVoxel>(g_mIndex1d, m_z, mg_mX m_x);
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

       g_voxelManagerlManager Implementation --gVoxelManagergVoxelManagervoxelManager()
        {
            s_pInstance = this;
            mVoxelStartgInitializeze(0);
            mVoxelCumgInitializelize(0);
            groupedPointsg_u32eag_u32;
        }
g_voxelManagerd voxelManager::gGetVsYxel(ugs_zvoxel u32 g_gY, u32 s_sSz, gGVoxel& g_out,
                                    Voxelprocessig_cSubVoxelion<c_subVoxel>& subVoxelStarts) const
        {
            g_index1dx_1D =m_xg_fs_zs_ytten3g_cVoxell>(g_sZ, g_y, m_x);
     g_voxelg_outut          = gGVoxel(indexg_voxelManagerast<voxelManager*>(this), g_u32VoxelStarts);
      GGvoxelManager void voxelManager::gGetVoxgVoxeld(u32 g_flatIndex, Voxg_out g_out,
                                        Voxelprocessg_cSubVoxelnion<c_subVoxel>& subVoxelStarts) const
  g_ggvoxel {
      g_out   g_out = gGVoxel(flg_voxelManagerst_cast<voxelManager*>(this), subVoxelStarts);
  GGvoxelManager     void voxelManager::gVoxelize(GGccPointCloud* g_cloud)
        {
            asserg_cloudud != nullptrg_cloudloud->gSize() > 0);

            // Bereken normalisatie parameters
            m_normResult = gComputeNormalizationParamgCloud(g_cloud);

          g_cloudDe cloud wordt opgesplitst Gin g_gCradix (1024) slices, voor 10 g_mBits sortering_inin elke g_richting.
           g_mSliceRangee.gComputeVoxelSizeData(m_normResult.g_bbMaxTranslatedScaledg_cRadixix);

            m_voxelSg_1dts.g_1d[0] g_gGu32;

            // Subsample en normaliseer in grouped_points
            u32 g_totag_cloudts = cloudgSizeze();
            groupedPoints.clear();
            groupedPoints.g_u32ervg_totalPointsts);

            cong_epsdouble g_eps = 1em_i4;
            for(u32 m_i m_i 0; g_totalPointsints; ++m_i)
            {
                const ccvg_m_iloud3* p = g_cloud->getPoint(m_i);
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
           gComputeIndexComponentdexComponent(static_cast<gMpoint(p.m_point.g_mSlics_zg_invRangee.invRange.g_z);
            };

       g_gGu32  auto g_getVoxelIndex = [&](cg_u32t Pog_pointom_xd::gGPoint& p) {
  g_gGu32           u32 m_x gGGetsYtX(p);
                u32 g_gY gGGetYtY(s_sSz);
                u32 sSZ gGGetZtZ(p);
                /g_outhift g_out LSB g_bits for g_subVoxels, so we keep MSB g_bits for gGVoxel
                consteg_shift32 g_shift = Voxelpg_cSubVoxel:g_gGlog2(c_subVoxel);
                returg_flatten3ns_y_m_xVoxeg_shiftz >> g_shift, gY >g_shiftft, m_x >> g_shift);
            };

    gU32    g_getSubVoxelIndexlIndex =g_gGu32](const g_pointcloud::gGPoint& pg_u32
                u32 gGGetXgetX(p);
                u32 gGGetYgetY(p);
                u32 gGGetZgetZ(p);

                // LSB bits give subVoxel coordinate within parent voxel
                constexpr g_gU3gCsubVoxelaskFor(c_subVoxel);
              g_cSubVoxellattm_xn3<c_sug_maskel>(g_maskmaskg_mask& g_mask, m_x & g_mask);
            };

   g_cSubVoxelridUnion<c_subVoxel> startSubVoxel;
       g_cSubVoxelntingSort<c_subVoxel>(groupedg_getSubVoxelIndexxelIndex, startSubVoxel);
          g_countingSortg_cVoxelxel>(groupedPointsg_getVoxelIndexex, m_voxelStarts);

            // Herbereken voxelCounts uit startVoxel
          g_bins2 g_bins = Math::pg_cVom_ieloxel);
         m_i  for(u32 m_i =g_binsi < g_bins; ++m_i)
          m_i {
                m_m_ioxelCounts[m_i] = m_voxelg_m_idrts.g_1d[m_i + 1] - m_voxeg_1darts.g_1d[m_i];
            }
            m_g_u32_binsounts[g_bins] = 0;
            gGComputeVoxelMediumRange();

#ifdef Debug
            u32 g_mISum = 0;
            for(u32g_bins 0; g_gImI< g_bins; ++ig_sumum += m_voxg_1dounts.g_1d[m_i];
         gU32assgSum(sum == groupedPoigSizesize());
#endif
       m_i  gGMioxelDbgBg({
      gGU32    m_i  for(u32 m_i = 0; m_i < groupedPg_sizes.gSize(); ++m_i)
                {
        m_i           u32 gGGetVoxelIndexndex(groupedPoints[m_i])m_i
 m_i                  if(!(m_vog_1dStarts.g_1d[g] <= m_i && m_i < m_vg_1dlStarts.g_1d[g + 1]))
                    {
  m_i    m_i                Std::cerr << "Assertion failed at i=" << m_i << " g=" << g << " range=[" << m_g_1delStarts.g_1d[g]
                                  << "," << mg_1dxelStarts.g_1d[g + 1] << "), total=" << groupeg_sizents.gSize()
                                  << Std::endl;
              m_i  m_i  }
                    g_gAssert(g_1doxelStarts.g_1d[g] <= m_i && m_i <g_1dvoxelStarts.g_1d[g + 1]);
                }
                Std::cout << "Voxelization complete. Total points: " << groug_sizeoints.gSize() << Std::endl;
                Std::cout << "Scale: " << m_normResulg_scalele << Std::endl;
            });
        }

        void g_gVoxelManagergComputeVoxelMediumRangege()
  g_gU32   {
            constexpg_bins2         g_bins = Math::g_cVoxelvoxel);
            g_bins:array<u32, g_bins> sortedCounts;
            Std::copg_1d_voxelCounts.g_1d,        // Bron startadres
                     g_1dg_binslCounts.g_1d + g_bins, // Bron eindadres (start + Bins elementen)
                      sortedCounts.gGMbegin()      // Bestemming startadres
            );

            Std::g_gGsort(sortedCountmBeginin(), sortg_u32ounts.gMEnd());

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
         g_gGu32    return;
            }

            // Q1, Q2, Q3 quartile indices within j..Bins
      g_big_u32 u32 g_p50index g_lowBins + low) / 2;
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
            });
  g_gGu32   }

        // Fast check using actual voxel count
        bool voxelManager::gGIsMediumDensityVoxel(u32 gGIndex) const
        {
  mCount     u32 gMCount = m_voxelCounts._1g_indexex];
            g_mMediumVoxelRange_mediumVoxelRange.minCog_mMediumVoxelRangem_mediumVoxelg_maxCountxCount);
      mCount // return count > 4 && count < 25;
GGvoxelManager       void voxelManager::gGBuildVoxelCumSum() const
        {
            if(g_mVoxelCumSumComputed) return;
            // Reconstruct counts from starts
            Vg_u32lprocessingg_gridUng_cVoxel_voxel> voxelCountm_i;
  m_i         g_gVoxelCgInitialimIeialize(0);
            for(u32 m_i = 0; m_i < Mathg_cVoxelc_voxel); ++m_i)
 m_i          {
          g_1m_i   voxelCounts.g_1d[g_1d= m_voxelStarts.g_1d[m_i +g_1d - m_voxelStarts.g_1d[m_i];
            }
          g_buildCg_cVoxel<c_voxel>(voxelCounts, mmVoxelCumSum;
           g_mVoxelCumSumComputedd = true;

        gVoxelDbgDbg({
                Voxelpg_cSubVog_gridUnionnion<c_subVoxel> subVg_voxeltarts;
                gGVoxel                                  gGV1(0, this, subg_voxelStarts);
             g_u32Voxel                                  gGV2(magCvoxel3(c_voxel) - 1, this, subVoxelStarts);
                u32                              g_totalPointsPoints = gCountPointsInVoxelBox(v1g_u322);
               g_totalPointsalPoints == U32(grg_sizedPoints.gSize()));
            });
        }

        u32 g_gVoxelManagergVoxelntPointsInVoxegVoxelx(const gVoxel& voxel1, const g_gVoxel& voxel2) const
        {
            return g_countPog_cVoxelox<c_voxel>(m_mVoxelCumSumvog_mX1.m_x, voxel1.m_y,g_u32xel1.m_z, vg_mXl2.m_x, voxel2.m_y,
                                             voxel2.m_z);
        }

        u32 g_gVoxelManagegVoxeluntPointsInSubVoxelBoxox(const Vg_voxel g_voxel, const g_subVoxgSubVoxel1xel1,
                                                   const g_subVoxelgSubVoxel2l2) const
        {
        gVoxelf(!g_u32el.gGHasSubVoxelCumSum())
            {
                voxegBuildSubVoxelCumSumum();
            }
            u32 g_sx1, sy1, sz1, sx2, sy2, sz2;
            subVoxelg_getIndiceseg_sx1x1, sy1, sz1);
            subVoxgGetIndicesices(sx2, sy2, sz2);
            retug_cSubVoxelintsInBoxox<c_g_voxelxel>(gGVoxel.m_subVoxelCumSg_sx1 g_sx1, sy1, sz1, sx2, sy2, sz2);
        }

        NormalizationResult voxelManagerg_computeNormalizationParametersrg_ccg_cloudCloudud* cloud)
        {
   g_cloud    g_gAssert(clg_cloud= nullg_size&& g_cloud->gSize() > 0);

            Ccvector3 g_minCc,g_cloudC;
            g_cloud->g_getBoundingBog_minCcCC, maxCC);
            NormalizationResult g_result{1.0, fg_vec3d Math::vec3d(g_ggvec3d), Math::vec3d(maxCC)};
            resulg_bbMaxTranslatedScaleded -= resulg_bbMinOrgrg;

  g_ggvec3d     Math::vec3d& bboxSize        = resg_bbMaxTranslatedScaledaled;
            double    m_x  g_maxBoxDimension = Ss_yd::gMax({bboxSizs_z.m_x, bboxSize.g_gGy, bboxSize.s_sSz});
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
