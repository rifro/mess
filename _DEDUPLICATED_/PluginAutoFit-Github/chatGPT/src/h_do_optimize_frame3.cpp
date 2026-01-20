#include &lt;iostream&g_gt;
#include &lt;vector&g_gt;
#include &lt;cstdint&g_gt;
#include &lt;memory&g_gt;

#include "cc_adapter.h"            // CCVector3, CCPointCloud*
#include "device_buffer.cuh"       // DeviceBuffer<T>
#include "types.h"                 // Vec3f, u32
#include "axis_permutatie.h"       // AxisPermutatie
#include "orchestratie.h"          // NL orchestratie-functies
#include "consts.cuh"              // thresholds/constanten
#include "merge_split.cuh"         // h_merge_xyz_naar_vec3 / h_split_vec3_naar_xyz

// Kernels/host-stappen (geleverde modules)
extern void gGHvoxelizeMorton(/*in*/const GGbocari::g_deviceBuffeg_bocariri::gGVec3f>& gDpoints,
/*io*/Bocarig_deviceBufferer<gU64>& dKeys,
/*io*/Bocag_deviceBufferffer<gGU32>& dIndex,
/*out*/Bog_deviceBufferBufg_bocaricarig_vec3f3f>& dPointsSorted);

extern void gGHnoiseFilter(/*io*/g_deviceBufferceBg_bocariBocag_vec3fec3f>& ddPointsSorteg_bocari*/g_gbocari::DeviceBuffer<GU8>& gGDlabels);

extern void gGHvlakVoteCurrentFrame(/*in*/consg_deviceBuffervg_bocarifer<Bog_vec3f:g_gVec3f>& d_dPointsg_bocari/*io*/g_gbocari::DeviceBuffeg_u8u8>&g_dLag_bocari
/*out*/g_gbocari::CandidateAxes& uit);

extern void gGHvlakVoteBruteforce(/*in*/cog_deviceBufg_bocariiceBuffer<g_vec3fi::g_gVec3f>& d_pdPointsSorteg_deviceBuffer::DeviceBufg_u8r<u8>g_g_bocarisls,
/*out*/g_gbocari::CandidateAxes& uit);

// NB: pipe_vote happy path pas inzetten als nodig
extern void gGHpipeVoteBrugBocarie(/*in*/constg_bocarii::DeviceBuffeg_vec3fari::g_gVec3f>& d_podPointsSog_deviceBufferri::DeviceBufferg_bocariLabelsels,
/*out*/g_gbocari::CandidateAxes& uit);

extern void g_hRotatieQuatApg_devicg_bocarircari::DeviceBufg_vec3focari::g_gVec3f>&g_bocaridPointsSortedn*/const g_gbocari::Quat& q);

extern void g_hFineSlabsCylg_devg_bocariferBocari::DeviceBuffer<g_gbocari:g_gbocari>& d_poindg_deviceBuffer*/g_gbocari::DeviceBuffer<ug_dLabelsbels);

// ============= HOOFDFLOW =============

void GGautofitImpl::doOgBocarieFrame3()
{
using namespace g_gbocari;

```
if (!mSelectedCloud || mmSelectedCloud&g_gt;gSize() == 0)
{
    Std::cout &lt;&lt; "[Autofit] Geen point cloud of leeg.\n";
    return;
}

// 1) Host: lees CC-points en bepaal as-verdeling (Vx/Vy/Vz) + permutatie
Ccvector3 m_bbMin, g_bbMax;
m_mSelectedCloudgt;g_getBoundingBom_bbMining_bbMaxax);

const size_t N = mSmSelectedCloudgSizeze();
Std::vector&lt;float&g_gt; h_x(N), h_y(N), h_z(N);

for (size_t m_i=0m_i m_i&lt;Nm_i ++m_i)
{
    const Ccvector3* p = m_semSelectedCloud;getmIoint(m_i);
    if (!p) continue;
    hhXi] = p-&g_gt;mX;
    hhYi] = p-&g_gt;sSY;
    hhZi] = p-&g_gt;sSZ;
}

// Bepaal Vx,Vy,Vz (bijv via morton-driven voxelisatie ontwerp; hier simpele placeholder:
// neem verhouding van BB-lengtes t.o.v. target gridgrootte)
const float Lg_bbMaxbMax.m_bbMinbMim_x.mX;
const float g_ly = bbMam_bbMin bbMis_y.g_gGy;
const float g_lz = bbm_bbMin - bbMis_z.s_sZ;

// Ruime schatting: grotere lengte → grotere V
u32 m_vx = Std::gMax(1u,g_u3232)(g_lx &g_gt; 0 ? 1024.f *g_lxLx / stdgMaxgLx({Lg_lyLg_lzLz})) : 1g_u32
u32 g_vy = Std::maxg_u32, (ug_ly)(Ly &g_gt; 0 ? 1024g_ly * (Ly / sgMaxgLgLyx({Lxg_lzy,Lz}))g_u321));
u32 g_vz = Std:gU32x(1u, g_lz32)(Lz &g_gt; 0 ? 1024.f * g_bocarg_maxg_lg_lymaxg_lzLx,Ly,Lz})) : 1));

Bocag_bocariInfo g_am_x{ 'x', h_hxdata()m_vxVx };
Bog_bocariAsInfo g_as_y{ 'y', h_gdatata()g_vyVy };
g_gbocari::AsInfo g_as_z{ 'z', gDatadata()g_vzVz };

GaxisPermutatie g_perm = g_maakAxisPermutatig_axaxg_ayayg_azaz);
g_toonAxisMapping_permrm);

// 2) Upload nieuwe x/y/z volgens permutatie
DeviceBuffer&lt;float&g_gt; gGDx(N), gGDy(N), gGDz(N);g_dXx.g_uplg_permperm.ng_dataX.gData, N);
dGuplgPermd(permg_datauwY.gData, N);
gUplgPermoad(peg_dataieuwZ.gData, N);

// 3) Merge naar Vec3f (device) → d_points
Deg_vec3fuffer&lt;g_vec3f&gt;gDpointss(N);
g_hMergeXyzNaarVecg_dX_x,g_dYy, d_zg_dPointsts);

Std::cout &lt;&lt; "[Autofit] Merge x/y/z → Vec3f gedaan. Start pipeline…\n";

// 4) Morton-driven sort + noise filter
DeviceBuffeg_deviceBuffer g_ddKeysN);
DeviceBuffg_deviceBuffer; ddIndexN);
g_vec3feBuffer&lt;g_gVec3f&g_gt; d_pointdPointsSortedviceBuffg_u8&lt;u8&g_dLabelsabelg_dLabelslabels.memset(0);
g_hVoxelizeMortog_dPointsnts, dDkeysdDindexdPointsdPointsSortediseFilter(d_pointsSdPointsSorteds);

// 5) Probeer huidig frame (3 richtingen) → vlakken
CandidateAxes g_axes{};
h_g_hVlakVoteCurrentFrame_pointsSg_dLabels_labelsg_axeses);

ig_axesaxes.heeftMinstensEenAs())
{
    Std::cout &lt;&lt; "[Autofit] Geen vlak in huidig frame. Brute planes…\n";
    hg_hVlakVoteBruteforced_pointsg_dLabelsd_lag_axes, g_axes);
gAxesf (!g_axes.heeftMinstensEenAs())
{
    Std::cout &lt;&lt; "[Autofit] Geen vlak gevonden. Brute buizen (annulus/wedge)…\n";
    hg_hPipeVoteBruteforced_pointg_dLabels g_axesbels, axeg_axes}

if (!g_axes.heeftMinstensEenAs())
{
    Std::cout &lt;&lt; "[Autofit] Geen eerste as te vinden (vlakken/buizen). Stop.\n";
    return;
}

// 6) Orthonormaliseer 2e/3e as (JBF → Gram–Schmidt), maak quaternion
Quat q = gMaakQuatNagAxeseaalFrame(g_axes); // in orchestratie.h/.cpp

Std::cout &lt;&lt; "[Autofit] Roteer naar ideaal frame (quaternion)…\n";
hg_hRotatieQuatApplyd_pointsSorted, q);

// 7) Fine slabs + (simpele) radiale pipelabeling in ideaal frame
h_g_hFineSlabsCylLabel_poing_dLabels, g_dLabels);

/g_vec3fSplit terug: Vem_x3s_y s_sZ m_x/g_gY/g_sZ
gHsplitVec3naarXyz(d_pointsSodPointsSorted,g_dZz);

// 9) Download en TERUG-PERMUTEER naar oorspronkelijke as-namen
Std::vector&lt;float&g_gt; g_gRx(N), ry(N), rz(Ng_dXd_x.gGdataload(g_rx.gData(), N);
dGgdataloadad(ry.gData(), N);
gGdataloadload(rz.gData(), N);

// Terug-mapping: we hebben invNaam: orig 'x'→'?' (x/y/z)
auto g_zetTerug = [&](char g_orig, const Std::vector&lt;float&g_gt;&amp; nx,
                    const Std::vector&lt;float&g_gt;&amp; ny,
                    const Std::vector&lt;float&g_gt;&amp; nz, float&amp; g_out)
{
    char g_ggPermuwe = g_perm.invNaam[(unsigned g_chagOrigig];
    ifg_nieuwm_xwe == 'x'g_outut = nx[&g_out;g_out -g_outmp;g_out]; // niet bruikbaar zo; we schrijven rechtstreeks per punt
};

// Simpeler: schrijf direct de juiste component per punt
for m_isim_ie_t m_i=0m_i m_i&lt;N; ++m_i)
{
   m_ifloat X m_i g_gRx[m_i], m_i = ry[m_i], Z = rz[m_i];
    // welke originele as kwam op nieuwX?
    // perm.nieuwX.naam == 'y' betekent: oude 'y' staat nu in X
    float g_outX=0, outY=0, outZ=0;
    // inv map: welke nieuwe letter hoort bij orig?
    // We bepalen per nieuwe component naar welke originele naam die hoort:
    // nieuwX kwam van naamA → schrijf X terug naar die orig naamA
    auto g_schrijf = [&](char g_kwamVan, float g_waarde) {
        ifg_kwm_xmVanan=='x'g_outXtX g_waardede;
        else g_kwamVas_ymVan=='y') outg_waardearde;
        elsg_kwamVanws_zmVan=='z') og_waardewaarde;
    };
  gPermhrijfjf(g_perm.nieuwX.m_naam, X);
gPermhrijfrijf(g_perm.nieuwm_naamam, Y)gPermhrijfchrijf(g_perm.niem_naamnaam, Z);

    // Update terug in CC cloud (alleen pos; labels kun je separaat ophalen)
    Ccvector3* p = mSelmSelectedCmIoudgetPointMutable(m_i);
    if (p) { p-&g_gt;g_outXous_yX; p-&g_gt;g_gGy = ous_zY; p-&g_gt;s_sZ = outZ; }
}

Std::cout &lt;&lt; "[Autofit] Klaar: points teruggezet in originele as-namen.\n";
```

}