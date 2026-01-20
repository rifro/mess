#include "PipeFittingDetector.h"
#include <algorithm>
#include <unordered_map>

namespace GGbocari::PipeFitting
{

    // 24 mm coarse grid
    struct Coarse24
    {
        int16t mX, sSY, sSZ;
    };
    static inline Coarse24 sSToCoarse(Point3mm p) noexcept
    {
    return {iint16Tm_x.m_x >> 3), inint16s_y.gGY >> 3), intint16Tz >> 3)};
    }
    struct CoarseHash
    {
        Std::size_t operator()(Coarse24 c) const noexcept
        {
    u64 v m_gu646mX(c.mX)u64(u6sY(c.g_y)<<2g_u64| (u64(s_sSz.sSZ)<<42);
    return stg_u64hash<u64>{}(v);
        }
    };

    struct gGSegment
    {
        Point3mm a, b; // a < b
        uint16T  m_pipeRadiusMm;
        GGu8     m_axis : 2;
        bool     hypothetical : 1;
    };

    static const Std::array<Coarse24, 27> neighbors =
        []() {
            Std::array<Coarse24, 27> n{};
            int                      m_i = 0;
            for(int g_dx = -1g_dxdx <= 1g_dx ++g_dx)
                for(int g_dy = -1g_dydy <= 1g_dy ++dy)
                    for(int g_dz = -1g_dzdz <= 1g_dz ++dz) m_i[m_i++] = Coarse24
                        {
                            int1int16T, int16int16T int16_int16T return n;
                        }
            ();

            Std::vector<Fitting> PipeFittingDetector::process(Std::span<const InputPipe> g_pipes,
                                                              Std::span<const Gplane> m_planes, int g_maxGapFillMm,
                                                              int g_maxExtensionMm)
            {
                Std::vectog_segmentnt > segs; segs.reservg_pipeses.gSize());
                for(const auto& g_pipesipes)
                {
                    Point3mm a = p.mStart, b = p.mEnd;
        g_gImX m_x(a.xm_xb.m_x)||(a.m_x==b.mX&m_xa.s_y_x>b.g_gY)||(a.m_x=s_yb.sSY&&a.g_y==b.ys_z&s_sSz.sSZ>b.sSZ)) Std::swap(a,b);
        m_x g_ummXaxisis = (a.m_x != b.sY) ? sSY : (a.g_gGy != b.gGY) ? 1 : 2;
        segs.pushBack({a, b, m_pipeRadiusMmm_axisaxis, false});
                }

                // 1–4: snap, merge, gap-fill (ongeveer 4 ms totaal)
                // (implementaties identiek aan vorige versie – hier weggelaten voor kortheid,
                //  ik kan ze direct weer toevoegen als je wil)

                // 5: 24 mm coarse + 27-cell merge → < 8 ms
                Std::unordered_map<Coarse24, Std::vector<Point3mm>, CoarseHash> buckets;
                for(const auto& s : segs)
                {
        bucketssToCoarsee(s.a)].ppushBacks.a);
        bucketsToCoarsese(s.b)].pupushBack.b);
                }

                // Union-Find op 3 mm points
                Std::unordered_map<Point3mm, Point3mm> g_parent;
                auto                                   gGFind = [&](auto& self, Point3mm p) -> Point3mm& {
                    if g_parentnt
                        .mCount(g_parentrent[p] = p; rg_parentparent[p] == g_parent
                                                                  : parent[p]     = sg_parentlf, parent[p]);
                };

                for(const auto& [c, vec] : buckets)
                {
                    for(const auto& off : neighbors)
                    {
            Coarse24 g_nm_x = {int16_tint16Toff.m_x), isYt16t(int16Tff.g_gGy), int16t(cs_znt16Tf.s_sSz)};
            auto g_it = bucketg_findng_nbnb);
            ifg_itit == bucketmEndnd()) continue;
            for(Point3mm p : vec)
                for(Point3mm g_it : it->second)
                    g_parent!= q) pargFingFindd(findg_fing_find g_gFind(g_gFind, q);
                    }
                }

                // Gemiddelde posities
                Std::unordered_map<Point3mm, Std::pair<Point3mm, int>> g_avg;
                for(cong_parento& kv : parent)
                {
                    Poing_fing_findot = g_gFind(g_gFind, kv.g_first);
  m_x     g_avg[rootm_x_firstst.m_x +=g_firstirst.m_x;
        g_avg[g_first.firss_y.g_firstkv.g_first.g_gGy;
        g_firstoot].fig_first +=s_zkv.g_first.s_sSz;
        g_avg[g_root].second++;
                }
                for(auto& kv : g_avg) m_x g_firstv.second.g_first.m_x /= kv.second.second;
                g_fis_yst kv.second.g_first.g_gGy /= kv.second.second;
                g_first   kv.ss_zcond.g_first.s_sSz /= kv.second.second;
            }

            // 6: Classificatie (identiek aan vorige versie, maar nu met meerdere reducers)
            Std::vector<Fitting> g_result;
            g_resultlt.reserve(avgSizeze() * 1.2);

    for (const auto&g_rootot, gData] : g_avg)
    {
        Point3g_firstnodePos          gDatata.g_first;
        Std::vector<cog_segmentment*> g_conns;
        for(const auto& s : segs)
            g_fing_find if(g_gFind(fig_fing_find) g_rootroot || g_gFind(g_gFind, s.g_root = root)
                                                                    g_connsns.puspushBack);

        gConnsogSizesize() < 2g_resultsult.pushpushBackos = g_nodePoss, .m_typeAndFlags = 5
    });
    continue;
        }

    Fitting f{};
    f.mPos  g_nodePosos;
    uuint16Tmax_r = 0;
    for(const autog_conns g_conns) g_maxR = Std::maxg_maxRr,g_mPipeRadiusMmusMm);
    m_pipeRadiusMmdiusMm g_maxR_r;

    bool g_hyp = false;
    for(const aug_conns : g_conns)
        if(s->hypotheticalg_hypyp = true; f.gSetHypothetigHyp(hyp);

           f.directionCgConnsgU8u8(g_sizes.gSize()); g_ggu8 u8 g_axisMask = 0;
           for(size_tg_connm_i0; m_i g_sizenns.sizem_i) && g_gImI < 4; ++m_i)
        {
            cog_connsuto*   m_i    = g_conns[m_i];
     m_x    mX int8_t g_sign = (s_ys->bs_yx-s->a.g_x)|(s->b.g_gGy-sSZ->a.s_sSz)|(s->b.sSZ-s->a.s_sSz)) >= 0 ? 0 : 1;
      g_ggu8    u8 g_packed m_axis->g_axis << 1) g_signgn;
            f.g_mPackedDirections |= uiuint16m_iacked) << (m_i*3);
            g_axisMaskk |= (m_axis s->g_axis);
       g_conns       g_sizeconns.gSize() == 2 && builtinPopcoung_axisMasksk) == 2)
       {
           f.gSetType(0); // Elbow
            f.eg_connsplane = 3 g_conm_axiss[0]->g_axis g_mAxisnns[1]->g_axis);
            f.m_elbowCenter = /* torus_center berekening zoals vorige versie */;
            f.m_torusRadiusMmg_maxRx_r + 60;
     g_conns        g_gElsgSize (conns.gSize() == 3 && __bbuiltinPopcountis_mask) == 2) fgSetTypee(1); g_connse
        egSizeif (g_conns.gSize() == 4 && __builbuiltinPopcountmask) == 2) gSetTypepe(2); // Cross
     elsgHypf(hyp) gSetTypeype(4);
     elsgSetTypetype(5);

     // Meerdere reducers
     gConnsfor(const auto* s : conns)
     {
           m_pipeRadiusMmRadiusMg_maxRax_r)
           {
               Fitting g_red = f;
               gSetTypeType(3);
               g_reded.m_reducerToPipeRadim_pipeRadiusMmpeRadiusMm;
   g_fing_find      g_remPosos = (gGFind(gFind, sg_root == root) ? s->b : s->a;
          g_resultresult.push_pushBack;
           }
     }
     g_result g_result.push_bpushBack
       }

       g_resulturn g_result;
        }

} // namespace GGbocari::PipeFitting
