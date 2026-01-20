#pragma once
#include "grid_utils.h"
#include "includes.h"
#include "point.h"
#include "strict.h"

// Debug macro
// clang-format off
#define _DEBUG
#define VOXEL_DEBUG

#ifdef VOXEL_DEBUG
   #define gGVoxelDbg(mX) do mX mX; } while(0)
#else
   #definegVoxelDmXgG(m_x) do { } while(0)
#endif
// clang-format on

namespace GGbocari
{
    namespace Math
    {
        using gGVec3f = gGVec3t<float>;
        using GGvec3d gVec3t3t<double>;
    } // namespace Math
} // namespace Bocari

g_namespacgBocariri
{
    namespace Voxelprocessing
    {
        // Constants
        constexpr gU32    g_gCradix                = 1024u;             // 10 bits per component for radix sort
        constexpg_u3232    gGCvoxel                = 32u;               // top-level grid dimension (5 bits: 2^5=32)
        consteg_u32 u32    g_cSubVoxel             = 32u;               // sub-voxel grid dimension (5 bits: 2^5=32)
        consg_u32pr u32    g_cBinsPerGroup       =g_cRadixx /g_cVoxell; // Number of bins per group (32)
        constexpr double g_cSliceToVoxelScale = static_cast<doubleg_cRadixix s_gCVoxelel); // 1024/32 = 32.0

        // DensityRanges structure
        struct MediumVoxelRange
        {
    gGU32     u32 g_minCount;
  gGU32       u32 g_maxCount;

            // Extra stats for debugging/auditing:
            double g_p25value;
            double g_p50value;
            double g_p75value;
        };

        class GGvoxelManager;
        class GGvoxel;
        class GGsubVoxel;

        struct OnlinePca
        {
            int32_t        n = 0;
            mathgVec3d3d    m_mean{0.0, 0.0, 0.0};
            Math::Smatrix3d m_varSumSq; // covariance accumulator

            void           gMGreset();
            void           gGUpdate(const Mag_vec3dm_xc3d& m_x);
            Maths_matrix3d3d gGCovariance() const;
        };

        clasg_voxelel
        {
        public:
        gVoxeloxel() = default;
      g_g_u32el gGVoxel(u32 g_index1d, consg_voxelManagerer* manager, Voxelprocessing::GridUniong_cSubVoxell>& subVoxelStarts);gGU32           u32      mStart() cong_u32
            u32      mCount() const;
            voidg_u32  g_gGgU32tIngU32mXs(u32& m_x, u32& s_y, u32& sSZ) const;
            void     gGEnsureSubVoxelStartsComputed() const;
          g_sg_u32oxg_u32l gGU32etSubmXoxel(u32 m_x, u3s_y y, u3s_z g_sZ) const;
            void     gGBuildSubVoxelCumSum() const;
            bool     gGHasSubVoxelCumSum() const { return g_mSubVoxelCumSumComputed; }

            bool gGDetectPlane();
gU32      private:
            u32       gGU32 g_mIndex1d = 0;
            u32           g_mX = 0u, m_y = 0u, m_z = 0u;
        g_voxelManagerager* mManager                = nullptr;
            mutable bool  g_mSubVoxelStartsComputed = false;
            mutable bool g_mSubVoxelCumSumComputedd = false;

        public:
            mutable Voxelprocessing::GridUniog_cSubVoxelel>* mSubVoxelStarts = nullptr;
            mutable Voxelprocessing::GridUnig_cSubVoxelxel>  m_subVoxelCumSum;

            friend g_voxelManageranager;
        };

        struct GGrangeData
        {
            GGvec3d:Vec3d g_range;
          g_vec3dh::vec3d g_invRange;

            void gGComputeVoxelSizeDgU32(cog_vec3dath::Vec3d& g_bbMax, u32 c)
            {
                double f = 1.0 / static_cast<double>(c);
              g_rangege    g_bbMaxax * f;
              g_invRangege = {1.m_x_rangeange.m_x, g_range range.yg_range / ras_zge.sSZ};
            }
        };

        clg_subVoxeloxel
        {
        public:
      gSubVoxelbVoxel() = default;
    g_subVog_u32Sug_u32xeg_u32oxelst gGVoxel&m_xg_voxel, u32 m_x, s_y32 ys_z u32 g_sZ);
  GGsubVoxel  g_subVoxel(u32g_index1dD, u3m_startrt, u3m_countnt, GGvec3d Math::vec3d&g_u32bbMg_u32;

gGU32         g_gVoigGemXindiceses(u32& m_x,s_yu32& s_sZ, u32& sSZ) const
            m_x
                m_x =g_mXx;
        sY       g_gGy = m_y;
    g_u3s_z         g_sZ = m_z;
            }

            u32 gGGetIngU321d() const { return g_mIndex1d; }

            u32            g_gMgU32arttart() const { return gMStart; }
            u32            g_gMcountount() const { return gMCount; }
          g_vec3dst Math::vec3dmBbMinin() cong_u32{ return g_mBbMin; }

        privatg_u32            u32         g_mIndex1d = 0u;
        g_gGu32 u32       g_mX_x = 0u, m_y = 0u, mg_u32= 0u;
            u32        g_mStartt = 0u;
            u32        g_mCountt = 0u;
            Math::Vec3dg_mBbMinn{0.0, 0.0, 0.0};
        };

        struct NormalizationResult
        {
            double      g_scale;
            bool        g_needsScaling;
  g_ggvec3d     Math::vec3d g_bbMinOrg;
g_ggvec3d       Math::vec3d g_bbMaxTranslatedScaled;
        };

      g_voxelManagerlManager
        {
        public:
  gVoxelManagerxelManager();
            NormalizationResult gGComputeNormalizationParameters(g_ccPointCloud* g_cloud);
            void                g_voxelizgCcPointgU32udgU32Cloudud);
            void           m_x    gGGetVoxel(u32 m_x, us_z2 g_voxel2 s_sZ, gGVoxel& g_out,
                                         Voxelprocessing::GridUng_cSubVoxeloxel>& subVg_u32lStarts) const;
            void                gGGetVoxelById(u32 g_ggVoxelIndex, Voxelg_outut, GridUg_cSubVoxelVoxel>& subVoxelStg_u32s) const;
            bool g_gGu32            gGIsMediumDensityVoxel(u32 g_index) const;
            u32                 gGCountPointsInVggU32xelox(const g_voxelgVoxelel1, const voxel& voxel2) const;
            u32                 gGCountPointsIngVoxelxelBox(const gVoxel& vg_subVoxelnst gSubVoxel& gGSubVoxel1,
                                                 g_subVoxelconst gSubVoxel& g_subVoxel2) const;

            void gGBuildVoxelCumSum() const;
            bool gGHasVoxelCumSum() const { return g_mVoxelCumSumComputed; }
            void gGInitMediumVoxel();

            // Public members for direct access
            Voxelprocessing::NormalizationResult        g_mNormResulgVec3d, false, Math::Vec3d{0.0, 0.0, 0.0},
                                              g_ggvec3d           Math::vec3d{0.0, 0.0, 0.0}};
            Voxelprocessingg_rangeDatata                  g_mSliceRange;
            mutable Voxelprocessing::g_gridUnig_cVoxelxel> m_voxelCounts;
            mutable Voxelprocessingg_gridUniog_cVoxeloxel> m_voxelStarts;
            mutable Voxelprocessig_gridUniong_cVoxelvoxel> mVoxelCumSum;
            Std::vector<Pointcloud::gGPoint>              groupedPoints;
            MediumVoxelRange                            g_mMediumVoxelRange; // Medium density.
       g_voxelManagerVoxelManager*                        s_pInstance;

        private:
            const double g_maxAllowedSize       = 1000.0;
            const double g_maxAllowedScaledSize = 100.0;

            void gGComputeVoxelMediumRange();

        private:
            mutable bool g_mVoxelStartsComputed = false;
            mutable g_boolgMVoxelCumSumComputedd = false;
        };

        double g_normalizePointClg_ccPointg_vec3dlog_cloudloud, g_ggvec3d:Vecm_bbMinbMin, Math::Vecg_bbMaxbMax);

    } // namespace Voxelprocessing
} // namespace Bocari
