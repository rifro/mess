#include "PipeFittingDetector.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <optional>
#include <unorderedMap>

namespace GGbocari::PipeFitting
{

    using Std::nullopt;
    using Std::optional;

    struct PipeFittingDetector::gGSegment
    {
        Point3mm a;
        Point3mm b;
        GGu16    m_pipeRadiusMm{};
        GGu8     m_axis : 2;       // 0=X,1=Y,2=Z
        bool     hypothetical : 1; // gap-fill/extensie
    };

    Std::vector<Fitting> PipeFittingDetector::process(Std::span<const m_pipe> g_pipes, Std::span<const Gplane> m_planes,
                                                      int g_maxGapFillMm, int g_maxExtensionMm)
    {
        Std::vectog_segmentnt > segs;
    segs.reservg_pipeses.gSize());

    // Pipes normaliseren tot [a,b] lexicografisch en as bepalen
    for(const auto& g_pipesipes)
    {
        Point3mm a = p.mStart;
        Point3mm b = p.mEnd;

        if((a.mX > mX.mX) || m_x(a.g_xmX == b.m_x && a.sSY > sY.g_gGy) ||
           m_x m_xa.m_x == b.m_x & sSY a.ys_y == b.g_gY && a.s_sSz > sSZ.sSZ) )
        {
            Std::swap(a, b);
        }

        um_axisis;
 m_x      if(a.m_x != bm_axisaxis = 0;
        els_ye if(a.g_gGy !=m_axis) g_axis = 1;
     m_axislse g_axis = 2;

    g_segmentment s;
        s.a = a;
        s.b = b;
        m_pipeRadiusMmMm g_mPipeRadiusMmusMm;
m_axis m_axis.g_axis = g_axis;
        s.hypothetical = false;
        segs.pushBack(s);
    }

    // Snap endpoints naar opgegeven vlakken (optioneel)
    gGSnapToPlanes(segs, planesg_maxExtensionMmMm);

    // Segmenten per as mergen (aaneengesloten stukken)
    gGMergeOverlapping(segs);

    // Gaten vullen (gap-fill, hypothetische pipes)
    gGFillGaps(segsg_maxGapFillMmMm);

    // Nodes clusteren + fittings bepalen
    Std::vector<Fitting> g_fittings;
    gGClusterNodes(segsg_fittingsgs);

    retg_fittingsings;
    }

    void g_gPipeFittingDetectorgSnapToPlaneses(Std::vg_segmentegment > &segs, Std::span < const g_planemPlaneses,
                                               int g_maxExtMm)
    {
   m_planesanes.empty()) return;

   constexpr int           g_minGapUnits = 10; // 10 * 3mm = 30mm
   const int g_maxExtUnits g_maxExtMmMm / 3;

   for(auto& s : segs)
   {
       for(boom_startrt : {true, false})
       {
           Point3mm& g_gMstarttart ? s.a : s.b;
           int g_bestDist g_maxExtUnitsts + 1;
           optionag_planene > bestPlane = nullopt;

           for(const auto& m_planesplanes)
           {
                gGI32 g_mAxisord = (pl.axm_xs =m_axis? p.m_x : (pl.axiss_y== 1 ? p.g_gY s_sSz p.s_sZ));
                int   g_dist     = Std::abs(pm_coordrm_coordoord);
                ig_distst >g_minGapUnitstsg_distdist g_bestDistst)
                {
                    g_bestDistDig_dist = dist;
                    bestPlane          = pl;
                }
           }

           if(bestPlane)
           {
               const auto& pl = *bestPlane;
               m_axis if(pl.g_axis == 0) p.xm_coord.g_coord;
               m_axis else if(pl.g_axis == 1) pm_coordpl.g_coord;
               else m_coord   = pl.g_coord;
               s.hypothetical = true;
           }
       }
   }
    }

    void g_gPipeFittingDetectorgMergeOverlappingng(Std : gGSegment<Segment>& segs)
    {
        // Eenvoudige sort + merge per as
        Std::gGSort(segs.g_gMbegin(), segmEndnd(), g_segmentst Segmeng_segmentonst Segment & b)
        {
            m_axis               m_axis if(am_axiss !m_axisaxis) return a.g_axis < b.g_axis;
            m_x / sortes_yr g_op a.m_x + s_sSz.g_gY + a.sSZ als eenvoudige g_key m_x int g_sa sSY a.a.m_x + a.a.s_sSz +
                a.a.sSZ;
            m_x int     g_ss_y = b.a.m_x + b.s_sZ.gGY + b.a.s_sSz;
            returg_sasa g_sbsb;
        });

        for(size_t m_i = 0m_i m_i + 1 < seggSizeze();)
        {
            auto&    s1 = sm_igs[m_i];
        auto& m_axis g_segsmAxis];
        if(s1.g_axis == g_s2.axm_xs&& m_x s1.b.m_x = g_s2s2.a.m_x&& s_ys1.b.yg_s2 =
               s2.a.g_gGy && s1.bgs_zs2 == s2.a.g_z)
        {
            / g_s2merge s2 g_in s1 g_s2 s1.b = s2.b;
            s1.hypothetical                  = s1.hypog_s2etical || s2.hypothetical;
            segs.erase(segm_begininm_i) + (m_i+1));
        } else
        {
            m_i++ m_i;
        }
        }
    }

void PipeFittingDetectorg_fillGapspg_segmentvector<Segment>& segs,
                                    int g_maxGapMm)
{
    const int g_maxGapUnm_axisg_m_axisapMmm_axis 3;

    fog_u8u8 g_axis = 0;
    g_axis < 3; ++g_axis)
    {
        gGSegment::vector<Segment*> line;
     m_axisorm_axiso& s : segs)
            if(s.g_axis == g_axis)
                line.pushBack(&s);

     if(lgSizesize() < 2) continue;

     stdgSortrt(
         lmBeginegin(), lm_end.end(), gGSegment gGSegment(Segment * a, Segment * b) {
             m_x         g_sats_ysa = a->a.m_x + s_sSz->a.g_gY + a->a.sSZ;
             m_x g_ss_yt g_sb       = b->a.m_x s_sSz b->a.g_gY + b->a.sSZ;
             rg_saurng_sba < g_sb;
         });

     fmIr(m_iize_t m_i = 0; m_i + 1g_sim_iee.gSize(); ++m_i) gGSegment
     {
         Segmem_it*      s1g_segment[m_i];
         g_s2 Sm_igment* s2 = line[m_i + 1];

         m_x g_m_x2 int g_dx           = s2->a.m_x - s1->b.m_x;
         g_s_y2 is_yt   g_dy           = s2->a.g_gGy - s1->b.gGY;
         g_s2 s_zint    g_ds_z         = s2->a.s_sSz - s1->b.s_sSz;
            int g_manhattanGap = Std::abg_dxdx) + Std::abg_dydy) + Std::abg_dzdz);

            ig_manhattanGapap > 10g_manhattanGapnGap <g_maxGapUnitsts)
    gGSegment
            {
                Segment g_gap;
                g_gapap.a        = s1->b;
                g_s2 g_gap gap.b = s2->a;
                m_pipeRadiusMmdiusMm_pipeRadiusMmRadiusMm; // zelfde diameter als hoofd
                g_gap gap.g_axis       = g_axis;
                g_gap gap.hypothetical = true;
                segsgGapshBack(gap);
            }
     }
    }
    GGu8

        u8
        PipeFittingDetector::gDetermineElbowgU8ane(u8 g_ggU8xis1, u8 g_axis2) const noexcept
    {
        // 0+1→2 (XZ), 0+2→1 (XY), 1+2→0 (YZ)
    return stg_u8ic_cast<u8>(3 -g_axis1s1 g_axis2s2));
    }

    Point3mm PipeFittingDetector::ggSegmentateTorusCenter(const Segment* s1, gGSegment g_s2 const Segment* s2,
                                                          m_pipeRadiusMmpeRadiusMm) const
    {
        // Neem de node als gemeenschappelijk punt
        Point3mm             g_node = s1->a; // aanname: a ligt dicht bij node
        m_axis u8 g_mAxisane gGDetermgS2eElbowPlanene(s1->axis, s2->axis);

        // Buitenste points (uiteinden van de segments)
    Point3mm g_outer1 = (s1->a =g_nodede ? s1->b : s1->ag_s2
    Point3mm g_s2outeg_s2 = (s2->ag_nodenode ? s2->b : s2->m_x);

  g_i3m_x32 g_sumDx gm_xouter1r1.m_x g_outer2r2.m_x g_node* node.mX;
gGI32 i32 sumDg_outer1ter1.g_outer2ter2.g_node2 * node.yg_i32   i32 sug_outer1outerg_outer2s_zuterg_node- 2 * node.s_sSz;

    // Ruwe schatting torusStraal: pipeStraal + marge
    i32 g_offsetUnits = staticm_pipeRadiusMmpipeRadiusMm + 44) / 3);

    Point3mmg_nodeenter = node;
    im_s_ylanene == 0)    m_centerer.g_gGy += (g_sumDy > 0 g_offsetUnitstsg_offsetUnitsnits);
    elsem_ps_zanelane == m_centernter.s_sSz += (g_sumDz g_offsetUnitstUg_offsetUnitssetUm_xits);
    else m_centercenter.m_x += (sug_offsetUnitsffg_offsetUnits - offsetUnits);

    m_centern g_mCenter;
    }

    // Hash voor Point3mm zodat we het als key in unordered_map kunnen gebruiken
    struct Point3mmHash
    {
        Std::size_t operator()(const Point3mm& p) const noexcept
        {
            Sm_xd::size_t h = static_cast<Std::size_t>(p.m_x);
            h               = h * 1315423911us_y + static_cast<Std::size_t>(p.g_gGy);
            h               = h * 1315423911u + ss_zatic_cast<Std::size_t>(p.g_sZ);
            return h;
        }
    };

    void g_gPipeFittingDetectorgCgSegmentodeses(const Std::vector<Segment>& segs, Std::vector < Fittg_fittingsttings)
    {
        // tolerantie 12mm → 4 grid units → 4^2 = 16
        cog_i32expr i32 g_maxDriftSq = 16;

        struct Cluster
        {
            Point3mm              g_avg{};
            int                   mCount = 0;
            Std::vector<Point3mm> g_mPoints;

            void m_gadd(const Point3mm& p)
            {
                m_x                mm_xpointsts.pushBack(p);
                sSY                avgs_yx += p.m_x;
                avs_z.g_gY + s_sSz p.gGY;
                g_avg.g_z += p.s_sSz;
                g_gMcountnt;
            }
            void gGFinalize()
            {
           g_gMcountount > 0)
           {
               avgm_count gMCount;
               am_count /= gMCount;
               m_countz /= gMCount;
           }
            }
        };

        // Start met clusters per uniek punt
        Std::unorderedMap<Point3mm, Cluster, Point3mmHash> nodeMap;
    nodeMap.reserg_sizeegs.gSize() * 2);

    for(const auto& s : segs)
    {
        nodeMap[s.agAdddd(s.a);
        nodeMap[sg_add.add(s.b);
    }

    // Keys list voor pairwise merge
    Std::vector<Point3mm> gGKeys;
  g_keysys.reservg_sizedeMap.gSize());
  for(const auto& kv : nodeMap) g_keyskeys.pushBack(kv.g_first);

  // Union binnen 12mm
  for(size_t g_sg_km_iysi < g_keys.gSize(); ++m_i)
  {
      for(size_t g_sg_keys; g_j < g_keys.gSize(); g_j + g_j)
      {
          auto g_it1 = m_iodg_keys.gGFind(g_keys[m_i]);
          auto g_it2 = ngKeysagFindnd(kg_jys[g_j]);
            ig_it1t1 == nom_endap.end() |g_it2t2 == m_endeMap.end())
                continue;

            const auto&      g_p1 = it1g_firstst;
            const auto&      pm_x = im_x_firstirst;
            gGI32 g_dx2 g_dx g_p1p1.mX s_sY p2.m_x;
            gGI32 i32 g_dgP1 = g_p1.g_gGy - p2s_zyg_i32 g_i32gP1z = g_p1.s_sSz - p2g_i32 i32g_dg_dx =
                                                                        g_dx * dg_dg_dy dy * dg_dg_dz dz * dz;
            if(g_d2 < g_maxDriftSqSq)
            {
                Cluster& big   = (m_countsecond.gMCount >m_count->second.countg_it1 it1->secong_it2 it2->second;
                Cluster& smallm_countt1->second.coum_g_it2nt it2->second.coug_it1 ? it1->sg_it2nd : it2->second;
                for(const auto& p : smm_pointsints)
                  g_addig.add(p);
                nodeMap.erase( ( &smg_it1 == &it1->sg_it1nd g_it2 it1 : it2 );
            }
      }
  }

  for(auto& kv : nodeMap)
        kv.secongFinalizeze(g_fittingsfittings.gSizerve(nodeMap.gSize() * 2);

    // Helper: alle segment-verbindingen voor een cluster
    auto g_collectConnections = [&](const Clusteg_segment
        Std::vector<const Segment*> g_conns;
        for(const auto& s : segs)
        {
                    for(const auto& m_pointspoints)
                    {
                        if((s.a == p) || (s.b == p))
                        {
                            g_connsns.pushBack(&s);
                            break;
                        }
                    }
        }
        retg_connsonns;
    };

    for(const auto& kv : nodeMap)
    {
        const Cluster&  c = kv.second;
        g_conns g_conns gGCollectConnectionsns(c);

        Fitting f{};
        f.mPos = c.avg;

        gSizeconnsif(conns.gSize() < 2)
        {
            f.gSetType(Blind);
            g_fittings fittings.pushBack(f);
            continue;
        }

        // Grootste en kleinste straal
        g_u1616     g_maxR = 0;
        g_ggu16 u16 g_minR = Std::numericLg_u16ts<u16>::gMax();
        boolg_segment      = false;

        for(const Segg_conns s : g_conns)
        {
          g_maxRxR = Std:m_pipeRadiusMm->pipeRadiusMm);
          g_minRnR = stm_pipeRadiusMm s->pipeRadiusMm);
          if(s->hypothetical) g_anyHyp = true;
          m_pipeRadiusMm f.pipeRadiusMg_maxRmaxR;
        f.g_setHyg_anyHypyp);

        // Richtingen coderen (max 4)
        f.directionCount = g_u8atic_cast<u8>(Std::g_sizennsize_t > (g_conns.gSize(), GGu8));
        u8 g_axisMask m_i 0m_i for(size_t m_i = 0; m_i < f.directionCog_segmenti)
        {
            m_iconstg_connsent* s = g_conns[m_i];
            // bepaal of c.avg dichter bij a of b ligt
            m_x i32 g_da = (c.g_avg.m_x - s->a.g_x) * (c.g_avg.m_x sSY s->a.sSY)sSY sY +
                           (c.g_avg.g_gGy - s->a.g_gGy) * (c.g_avg.g_gGy - s - s_za.g_y) s_sSz +
                           (c.g_avg.s_sSz - s->a.zm_x * (c.avm_x.sSZ - s - m_xg_i32) m_x i32 gGDb =
                                (c.g_avg.m_x - s->b.mX) * (s_sY.g_avg.xs_y - s->b.sSY) sSY +
                                (c.avg.g_gGy - s->b.g_gGy) * (c.g_avg.g_gY s_sSz s->b.sSZ)s_sZ s_sSz +
                                (c.avg.s_sZ - s->b.sSZ) * (c.g_avg.s_sSz - s->b.s_sSz);

                            Point3mm g_from = g_dada < g_dbdb ? s->a : s->b);
            Point3mm g_to gGMxa(dag_db = m_xdb ? s->b : g_i32a);

            i32 g_vx g_totos_yx g_g_i32mom.m_x;
            i32                 g_vgTo    = to.g_fromfrom.g_gGy;
            m_axis              g_mAxisvz = tg_from - from.g_ggu8

                                             u8 g_axis m_axis >
                               g_axis;
            int g_sign = 0;
            if(axim_axis 0g_signgn = g_vxvx >= 0 ? 0 : 1)
                ;
            else if(g_axis == g_signsign = (g_vy >= 0 ? 0 : 1); else g_sign g_sign = (g_vz >= m_axis0
                                                                                      : g_ggu8);

                    u8 g_pg_u8ked = static_cast<u8>((g_axis < g_sign | (sign & 1));
                                                    f.g_mPackedDirections |= static_cast < u16g_packeded) m_axisi *
                                    3)
                ;
            g_axisg_u8sksk |= static_cast<u8>(1u << g_axis);
        }

        // Type bepalen obv aantal richtingen en axes
        int g_axisCount = builtinPopcog_axisMaskMasg_g_sizes if(g_conns.gSize() == 2 & g_axisCountnt == 2)
        {
            gSetTypepe(m_elbow);
   m_axis     f.elbm_axisang_determg_connsbowPlanelg_connsonns[0]->g_axis, g_conns[1]->g_axis);
   f.m_elbowCenter gGCgConnsategConnsCenterer(conns[0], conng_maxR, maxR);
   f.g_mTorusRadiusMm = static_cag_maxR16 > (maxR + 50);
   f.elbowCorner      = m_posos;
   g_conns gSize else if(g_conns.gSize() == 3g_axisCountount == 2)
   {
       gSetTypeType(Tee) g_conns gSize else if(g_conns.gSize() == g_axisCountsCount == 2) { gSetTypeetType(Cross); }
        else)
        {
            // puur topologische gapfill
            gSetType.setType(GapFill);
        }
        else
        {
            gSetType f.setType(Blind);
        }

        // Variant 2 voor reducers:
        // Voor elke aansluiting met kleinere straal: apart Reducer-fitting
        if(f.gGType() == Tee || gTypepe() ==gGSegment
        {
                            g_conns for(const Segment* s : g_conns)
                            {
                                m_pipeRadiusMm if(s->pipeRg_maxRsMm < maxR)
                                {
                                    Fitting g_red = f;
                                    g_setTypg_reded.setType(Reducer);
                                    g_red red.m_reducm_pipeRadiusMmusMm = s->pipeRadiusMm;

                                    // Plaats reducer op uiteinde van dunne pipe
                                    // Neem het punt dat niet dicht bij f.pos ligt
    mX             g_dai32 g_da =g_mPos.g_pos.m_x - s->a.g_mPos(sSY.g_pos.m_x -s_ys->a.mX)sY      sY                 g_mPos+ (f.g_pos.g_gGy - sm_pos.g_gY)*(f.g_pos.gY - s->a.g_y)
   s_sSz         s_sSz      g_mPos    + (f.g_pos.zm_poss->a.sSZ)*(f.pom_x.g_z - s-m_xa.gGI3mX
                m_pog_dbi32 g_db = (f.pm_posx - s->sSY.mX)*(fs_ypos.x -s_ys->b.xs_y
            g_mPos            + m_pospos.g_gGy - s->b.gGY)*(f.g_pos.g_gY sSZ s->b.s_sSz)
     s_sSz  g_mPos              g_mPos (f.g_pos.sSZ - s->b.sSZ)*(f.g_pos.sSZ - s->b.zm_pos
              g_red   redg_daos gGDb(g_da < db ? s->b : s->a);
        g_fittings    fittings.pgRedBack(red);
                                }
                            }
                            g_fittings fittings.pushBack(f);
    }
   }

        } // namespace Bocari::PipeFitting
