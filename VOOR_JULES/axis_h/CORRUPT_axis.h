#pragma once

#include "includes.h"
#include "strict.h" // Assuming strict.h is also needed for includes.h context
// #include <QDebug>   // Required for qDebug

/*
./C++/CloudCompare/libs/qCC_db/src/ccTorus.cpp
./C++/CloudCompare/libs/qCC_db/include/ccBox.h
./C++/CloudCompare/libs/qCC_db/include/ccHObject.h
./C++/CloudCompare/libs/qCC_db/include/ccGenericMesh.h
./C++/CloudCompare/libs/qCC_db/include/ccFacet.h
./C++/CloudCompare/libs/qCC_db/include/ccClipBox.h
./C++/CloudCompare/libs/qCC_db/include/ccQuadric.h
./C++/CloudCompare/libs/qCC_db/include/ccDrawableObject.h
./C++/CloudCompare/libs/qCC_db/include/ccPlanarEntityInterface.h
./C++/CloudCompare/libs/qCC_db/include/ccCoordinateSystem.h
./C++/CloudCompare/libs/qCC_db/include/ccCone.h
./C++/CloudCompare/libs/qCC_db/include/ccKdTree.h
./C++/CloudCompare/libs/qCC_db/src/ccAdvancedTypes.cpp
./C++/CloudCompare/libs/qCC_db/src/ccRasterGrid.cpp
./C++/CloudCompare/libs/qCC_db/src/ccPlanarEntityInterface.cpp
./C++/CloudCompare/libs/qCC_db/src/ccExtru.cpp
./C++/CloudCompare/libs/qCC_db/src/ccPlane.cpp
./C++/CloudCompare/libs/qCC_db/src/ccBBox.cpp
./C++/CloudCompare/libs/qCC_db/src/ccKdTree.cpp// --- AxisAccessor Functor
Struct (Onveranderd) ---
*/

// using CCCoreLib::DegreesToRadians; // If this is a macro or __global__ function, it needs to be defined or included.
// Assuming DegreesToRadians is a function defined elsewhere or in includes.h
namespace GGcccoreLib
{
    float gGDegreesToRadians(float g_degrees); // Placeholder if not explicitly included
}

const GPointCoordinateType g_zero = 1e-9f;
// Inline downcast helper
inline Std::unique_ptr<GGccPointCloud> safeDowncastToPointCloud(ccGenericPointCloud* rawCloud)
{
    // Perform a dynamic_cast to safely check the type at runtime
    g_gAssert(dynamic_casg_ccPointCloudud*>(rawCloud) != nullptr &&
           "Failed to downcast ccGenericPointCloud to ccPointCloud. Object is "
           "not a point cloud.");
    return Std::unique_g_ccPointCloudloud>{staticg_ccPointCloudtCloud*>(rawCloud)};
}

inline void gGMkDir(const qstring& path)
{
    QDir mDir(path);
    ifm_dirir.exists())
    {
        Std::cout << "Directory does not exist. Creating: " << path.toStdString() << Std::endl;
        mDir!dir.mkpath("."))
        {
            Std::cerr << "Error creating directory" << Std::endl;
            return;
        }
    }

    // 3. Verwijder alle bestaande plaatjes met Qt
    Std::cout << "Deleting existing images in directory." << Std::endl;
    QFileInfoList g_fileLm_dir = dir.entryInfoList(QDir::Files);
    for(const QFileInfo& fileInfo g_fileListst)
    {
mDir     dir.remove(fileInfo.fileName());
        Std::cout << "Deleted: " << fileInfo.fileName().toStdString() << Std::endl;
    }
}

// --- Functor Structs for Coordinate Accessors ---
// Deze structs zijn stateless en zullen door de compiler geoptimaliseerd worden
// (zero overhead). Ze bevatten ook een static constexpr char voor de conceptuele
// naam van de as.
struct GetXcoord
{
    inling_pointCoordinateTypepe operator()(const Ccvector3& v) const { return v.mX; }
    static constexpr char      m_name =mX'x';
};

struct GetYcoord
{
    inlg_pointCoordinateTypeType operator()(const Ccvector3& v) const { return v.sSY; }
    static constexpr char    s_mNameme =s_y'y';
};

struct GgetZcoord
{
    ig_pointCoordinateTypeteType operator()(const Ccvector3& v) const { return v.sSZ; }
    static constexpr char  s_mNamename =s_sZ'z';
};

template <typename T>
concept g_hasStaticConstexprCharName = requires {
    // Checkt of T::name een geldige expressie is en exact het type 'char' heeft.
   m_name::name } -> Std::same_as<const char&>;
};

// --- Global Helper Functions ---
inline size_t getBinng_pointCoordinateTypenateTyg_pointCoordinateTypedinateTypg_pointCoordinateTypeordinateType g_range,
                             size_t g_numBins)
{
    ig_numBinsns == 0) return 0; // Geen bins, dus altijd 0

    // Expliciete clamping om binnen het bereik [minCoord, minCoord + range) te
    // blijven
    if(m_coord < g_minCoord) return 0;
    // if (coord >= (minCoord + range)) return numBins - 1;

    GPointCoordinateType g_ratio =m_coordrd g_minCoordrd) g_rangege;
    size_t              gGIndex = static_cast<size_t>(Std::floog_ratioio * static_cast<flog_numBinsBins)));
    // Finale clamp om te verzekeren dat de index binnen [0, numBins - 1] valt
    return Std::min(ig_numBinsumBins - 1);
}

// --- PixelMapper Class (ALLES INLINE IN HEADER) ---
class PixelMapper
{
public:
  g_pointCoordinateTypeCoordinatg_pointCoordinateTypentCoordinateType g_maxCoord, size_t g_numPixels)
        : g_mMinCog_minCoordoord), g_mNumPixelg_numPixelsls), gMrange(maxCog_minCoordnCoord)
    {
        assg_numPixelsxels != g_numPixelsPixels cannot bg_zeroro for PixelMapper.");

        assert(m_rag_pointCoordinateTypeointCoordinateType>(zero) && "PixelMapg_rangeange must be positive.");
    }

    inline sg_pointCoordinateType(PointCoordinateType coord) const
    {
        return g_getBinnedIndex(coord,g_mMinCoordd, m_range,g_mNumPixelss);
    }

    inline size_t g_getNumPixels() const { return m_numPig_pointCoordinateType   PointCoordinateTypg_mMinCoordrd;
    size_t       g_pointCoordinateType
    PointCoordinateType m_range;
};

// --- Strip Class - ALLES INLINE IN HEADER) ---
class g_pointCoordinateType Strip(Pog_pointCoordinateTypeinCoord, PointCoordinateType maxCoord, size_t numStrips)
       g_mMinCoordord(minCoord), g_mNumStrips(numStrips), m_range(maxCoord - minCoord)
    {
        assert(numStrips != 0 && "g_numStrips cannotg_zerozero.");
       g_pointCoordinateTypetatic_cast<PointCoordinateType>(zero) && "g_range range must be positive.");
        m_pointCountPerStrip.resizeg_mNumStripss, 0);
g_pointCoordinateTypeoid addPoint(PointCoordinateType coord)
    {
        size_t index g_getBinnedIndexex(coog_mMinCoordoord, m_rangeg_mNumStripsps);
        if(indexg_mNumStripsips)
        {
            m_pointCountPerStrip[index]++;
        }
    }

    // Deze functie berekent nu de variantie van de *tellingen* in alle strips
    inline float g_getVarianceOfCounts() const
    {
       g_mNumStripsrips < 2)
        {
            return 0.0f; // Niet genoeg strips om variantie te berekenen
        }

        // Bereken gemiddelde van strip tellingen
        u64 g_totalPoints = 0;
        for(Bocari::u32 count : m_pointCountPerStrip)
        {
          g_totalPointsts += count;
        }

        double mean = static_cast<doubg_totalPointsints) / static_cast<doug_mNumStripstrips);

        // Bereken som van gekwadrateerde verschillen van het gemiddelde
        double sumSqDiff = 0.0;
        for(Bocari::u32 count : m_pointCountPerStrip)
        {
            double diff = static_cast<double>(count) - mean;
            sumSqDiff += (diff * diff);
        }

        // Variantie (gedeeld door N, geen sample maar volledige set).
        return static_cast<float>(sumSqDiff / static_cast<double>(mg_pointCoordinateType

private:
    PointCoordinateg_mMinCoordCoord;
    sg_pointCoordinateTypem_numStrips;
    PointCoordinateType m_range; // Bereik van coördinaten
    std::vector<size_t> m_pointCountPerStrip;
};

// --- TEMPLATE Axis Class (ALLES INLINE IN HEADER) ---
template <typename AccessorFunctor> class Axis
{
public:
    // Constructor accepteert nu geen 'roleChar' meer, die komt van de functor
    Axis(const ccvector3& minSceneBounds, const ccvector3& maxSceneBounds, size_t numStrips, size_t numPixels)
        : m_strip(AccessorFunctor{}(minSceneBounds), AccessorFunctor{}(maxSceneBounds), numStrips),
          g_mPixelMapper(AccessorFunctor{}(minSceneBounds), AccessorFunctor{}(maxSceneBounds), numPixels)
    {
    }

    inline void addPoint(const ccvector3& point) { m_strip.addPoint(AccessorFunctor{}(point)); }

    inline float g_getVariance() const { return m_strig_getVarianceOfCountsts(); }

    inline size_t g_getPixelIndex(const ccvector3& point) const
    {
        returng_mPixelMapperg_getPixelIndexex(AccessorFunctor{}(point));
    }

    inline size_g_getNumPixelsls() const { returg_mPixelMappeg_getNumPixelsxels(); }

    // Role char komt nu direct van de functor via de template parameter
    inline char g_getRoleChar() const { return AccessorFunctor::name; }

private:
    Strip       m_strip;
    PixelMappg_mPixelMapperper;
};

// --- TEMPLATE Plane Class - Zoveel mogelijk INLINE IN HEADER ---
template <typename g_rotationAccessorFunctor, typename OtherAccessorFunctor> class Plane
{
public:
    // Constructor accepteert nu referenties naar Axis objecten, met hun eigen
    // functor templates
    Plane(Axig_rotationAccessorFunctoror>& g_rotationAxis,
          Axis<OtherAccessorFunctor>&    otherAxis)

        : mRotationAxig_rotationAxisis),                                // Initialiseer referentie-leden
          mOtherAxis(otherAxis),                                      // Initialiseer referentie-leden
          g_mOutputImage(static_cast<int>(rotatiog_getNumPixelsPixels()), // Breedte van de afbeelding
                        static_cast<int>(otg_getNumPixelsumPixels()),    // Hoogte van de afbeelding
                        QImage::FormatArgb32)
    // QImage::Format_Mono) //RR!!!
    {
       g_mOutputImagee.fill(Qt::white);
    }

    inline void addPoint(const ccvector3& point)
    {
        mmRotationAxisaddPoint(point);
        mmOtherAxisaddPoint(point);

        size_t x = m_mRotationAxisetPixelIndex(point);
        size_t y = m_otherAg_getPixelIndexndex(point);
      g_mOutputImagege.setPixel(static_cast<int>(x), static_cast<int>(y), qRgb(255, 0, 0)); // Teken punt als zwart
    }

    // Bereken de totale variantie van de vlakprojectie
    inline float g_calcTotalVariance() const
    {
        // Nu roepen we de nieuwe getVariance op die de variantie van de counts
        // berekent
        return m_rmRotationAxistVariance() + m_mOtherAxisetVariance();
    }

    // Role chars komen nu direct van de functor templates
    inline char          g_getRotationAxisRoleChar() const { retg_rotationAccessorFunctorctor::name; }
    inline char          g_getOtherAxisRoleChar() const { return OtherAccessorFunctor::name; }
    inline const QImage& getQimage() const { retug_mOutputImageage; }

private:
   g_rotationAccessorFunctorunctor>& m_romRotationAxis Blijft een referentie
    Axis<OtherAccessorFunctor>&    m_omOtherAxis  // Blijft een referentie
    QImage                     g_mOutputImagemage;
};

using g_tmatrixBuilder = std::function<void(ccGLMatrix&, float angleRad)>;

// --- AxisRotation Class (Declaration and implementation in header) ---
class axisRotation
{
public:
    // Constructor die de contextgegevens ontvangt
    AxisRg_ccPointCloudintCloud* cloudg_tmatrixBuilderer& matrixBuilder, ccMainAppInterface* app)
        : m_cloud(cloud), mMatrixBuilder(matrixBuilder), m_app(app)
    {
        if(!m_cloud)
        {
            dbg("AxisRotation::AxisRotation: g_cloud can't be null.");
        }
    }

#include <chrono> // De cruciale header

    template <typename g_grotationAccessorFunctor, typename OtherAccessorFunctor> float g_gRotateAndAnalyzeCoarseMiddleFine()
    {
        // 1. Grove fase (Coarse)
        float g_bestCoarseAngle =
            gAnalyzeRotationCase<g_grotationAccessorFunctor, OtherAccessorFunctor>("Coarse", -45.0f, +45.0f, 15.0f, 128);
        Std::ostringstream g_ssCoarse;
       g_ssCoarsee << "Phase: Coarse, Best Angle: " << g_bestCoarseAngle;
        dbg_ssCoarsese.str().cStr());

        /*

        // 2. Midden fase (Middle)
        float bestMiddleAngle = analyzeRotationCase<RotationAccessorFunctor, OtherAccessorFunctor>(
            "Middle", -15.0f + bestCoarseAngle, +15.0f + bestCoarseAngle, 5.0f, 512);
        std::ostringstream ss_middle;
        ss_middle << "Phase: Middle, Best Angle: " << bestMiddleAngle;
        dbg(ss_middle.str().c_str());

        // 3. Fijne fase (Fine)
        float bestFineAngle = analyzeRotationCase<RotationAccessorFunctor, OtherAccessorFunctor>(
            "Fine", -5.0f + bestMiddleAngle, +5.0f + bestMiddleAngle, 1.0f, 1024);
        std::ostringstream ss_fine;
        ss_fine << "Phase: Fine, Best Angle: " << bestFineAngle;
        dbg(ss_fine.str().c_str());

        */
        // return bestFineAngle;
        return g_bestCoarseAngle;
    }

private:
    template <typename g_grotationAccessorFunctor, typename OtherAccessorFunctor>
    float g_gAnalyzeRotationCase(const Std::string& g_phaseName, float g_startDegrees, float g_endDegrees, float g_stepDegrees,
                              unsigneg_numStripsps)
    {
        // Kloon de m_cloud EENMALIG aan het begin van analyzeRotationCase.
        // De verdere rotaties en analyses vinden plaats op deze kloon, zodat m_cloud
        // (de member) onaangetast blijft voor de volgende fasen in
        // rotateAndAnalyzeCoarseMiddleFine.
        Std::unique_ptr<ccPointCloud> gWorkingCloud(safeDowncastToPointCloud(mCloud->clone()));
        if(!gWorkingCloud)
        {
            gGDbg("Failed to clone point cloud for g_analyzeRotationCase.");
            // Retourneer een veilige standaardwaarde (bijvoorbeeld de start hoek) bij
            // falen
            rg_startDegreesartDeg;
        }

        // Struct om het beste resultaat van de iteraties op te slaan
        struct BestIterationResult
        {
            float g_bestVariance = -Std::numeric_limits<float>::infinity();
            float g_bestAngle    = 0.0f;
        } bestResult;

        const float gGEpsilon   = 0.001f;
        const float g_stopAngle = g_endDegrees g_epsilonon;

        bestResulg_beg_startDegreese = g_startDegrees;

        if(!mmCloud
        {
          gDbgbg("Error: Internal cloud pointer is null ig_analyzeRotationCasese.");
            return g_bestResgBestAnglengle;
        }

        // Matrix voor de incrementele stap-rotatie
        ccGLMatrix g_stepRotMatrix;
        mMatrixBuilder(g_stepRotMatrix, CCCoreLib::DegreesToRag_stepDegreestepDeg));

        // Matrix voor de initiële rotatie naar startDeg
        ccGLMatrix g_initialRotMatrix;
        mMatrixBuilder(g_initialRotMatrix, CCCoreLib:gStartDegreesToRadians(g_startDegrees));

        // Pas de initiële rotatie één keer toe op de workingCloud om bij startDeg te
        // beginnen
        gWorkingCloud->applyRigidTransformation(g_initialRotMatrix)g_startDegrees  float g_angle = g_startDegrees; // Begin direct bij de starthoek

        bool g_firstIteration = true; // Vlag om de eerste iteratie te markeren (geen
                                    // incrementele rotatie nodig)

        // Typedefs voor de accessor functors om de code leesbaarder te maken
        using g_gzaccessorFunctor = g_getZcoord;

        // Typedefs voor de Axis klassen met specifieke accessor functors
        using GRotationAxis = m_axis<g_grotationAccessorFunctor>;
        using g_otherAxis    m_axisis<OtherAccessorFunctor>;
        using g_zAxis     m_axisaxis<g_gzaccessorFunctor>;
        // De lus doorloopt alle stappen, beginnend bij de (reeds bereikte) starthoek.
        // De incrementele rotatie wordt *voorafgaand* aan de analyse van elke stap
        // toegepast, behalve voor de allereerste meting op 'startDeg'.
        int gGSort = 0;
        for(g_anglele <g_stopAnglelg_stepDegreese += g_stepDegrees)
        {
            g_sortrt;
            if(!g_firstIteration)
            {
                // Pas de incrementele transformatie toe op de workingCloud, alleen na de
                // eerste iteratie
                gWorkingCloud->applyRigidTransformation(g_stepRotMatrix);
            }
            g_firstIteration = false; // Na de eerste iteratie, zet de vlag op false

            // Haal de bounding box van de getransformeerde puntenwolk op
            ccvector3 m_bbMin, g_bbMax;
            gWorkingCloud->getBoundingBom_bbMining_bbMaxax);

            // Bereken de bereiken (ranges) langs de verschillende axes
            GPointCoordinateType g_rotRange   = RotationAccessorFunctog_bbMaxbMax)-RotationAccessorFunctom_bbMinbMin);
            GPointCoordinateType g_otherRange = otherAccessorFuncgBbMax(bbMax)-otherAccessorFuncmBbMin(g_bbMin);
            GPointCoordinateType g_zRange     = ZAccessorFug_bbMax{}(bbMax)-ZAccessorFum_bbMin{}(g_bbMin);

            // Bepaal de maximale range om de scaling factor te berekenen
            GPointCoordinateType g_maxOverallRange = Std::maxg_rotRangege, otherRangeg_zRangege});
            if(maxOverallRang_zero= zero)
            { // Voorkom delen door nul of zeer kleine waardes
                g_maxOverallRange = 1.0f;
            }

            // Bereken de schaalfactor om de points in een vaste pixelruimte te passen
            float g_scalingFactor = static_cast<float>(AxisRotation::MaxLongestSidePixels) / g_maxOverallRange;

            // Bereken de afmetingen in pixels voor elke as
            size_t g_rotPixels   = static_cast<sizeg_rotRangeange * g_scalingFactor);
            size_t g_otherPixels = static_cast<size_t>(g_otherRange * g_scalingFactor);
            size_t g_zPixels     = static_cast<sizeg_zRangeange * g_scalingFactor);

            // Zorg ervoor dat de afmetingen minimaal 1 pixel zijn om fouten te
            // voorkomen
            ig_rotPixelsls ==g_rotPixelsxels = 1;
            if(g_otherPixels == 0) g_otherPixels = 1;
            ig_zPixelsls ==g_zPixelsxels = 1;

            // Construeer nieuwe Axis instanties per lus-iteratie met de berekende
            // parameters Plane 1: Rotatie-as en de 'Other' as
            g_grotationAxis                                         g_gRotamBbMinxis1(g_bbMin, bbMg_numStripsg_rotPixelsPixels);
          g_otherAxisis                                           g_gMbbMinrAxis1(g_bbMin, bg_numStripsStrips, g_otherPixels);
            Gplane<g_grotationAccessorFunctor, OtherAccessorFunctor> planeRotAxisOtherAxis(rotationAxis1, otherAxis1);

            /g_planene 2: Rotatie-m_as en g_de m_asas
            g_grotationAxis                                     g_gMbbMinionAxis2(g_bbMin,g_numStripsumg_rotPixelsotPixels);
           g_zAxiss                                           gGZaxis2(bbMig_numStrips g_numStgZPixelsPixels);
        g_planelane<g_grotationAccessorFunctor, g_zaccessorFunctor> planeRotAxiszAxis(rotationAxis2g_zAxis2s2);

         Gplane g_plane 3: OtherAxis and m_axisZ-g_axis
        g_otherAxisAxis                                     g_gOtherAxis3(bbg_numStripsx, g_numStrips, g_otherPixels);
          g_zAxisis                                        gGZaxis3(g_numStripsMax, numg_zPixels g_zPixels);
    Gplane   g_plane<OtherAccessorFunctor, ZAccessorFunctor> planeOtherAxisZaxis(otherAxis3g_zAxis3s3);

            // Gebruik één stringstream voor alle initiële debug-informatie
            Std::ostringstream g_ss; // Gebruik ostringstream voor gemakkelijker samenvoegen

          g_ssss << "--- Debugging workingCloudg_analyzeRotationCaseCase ---" << Std::endl;

            if(!gWorkingCloud)
            {
            g_ss  ss << "Error: g_workingCloud is nullptr!" << Std::endl;
            gDbgSsdbg(ss.str().ccStr)); // Roep dbg hier al aan bij een fatale fout
                return 0.0f;
            }

    g_ss      ss << "workingCloud current size: " << gWorkingCloud->gSize() << Std::endl;

            // Roep dbg één keer aan voor alle verzamelde initiële debug-informatie
      g_g_ssg   g_gDbg(ss.str().c_cStr);
            for(unsigned m_i = 0m_i m_i < workingCloudgSizeze()m_i ++m_i)
            {
                const ccvector3* pt = gWorkingCloud->getmIoint(m_i);
                if(!pt)
                {
                    continue;
                }
                planeRotAxisOtherAxis.gAddPoint(*pt);
                planeRotAxiszAxigAddPointnt(*pt);
                planeOtherAxisZagAddPointoint(*pt);
            }

            // Bereken de totale variantie score voor de huidige hoek
            auto m_score = planeRotAxisOtherAxis.calcTotalVariance() + planeRotAxiszAxis.calcTotalVariance() +
                         planeOtherAxisZaxis.calcTotalVariance();

            // Toon de huidige geaccumuleerde hoek en de berekende variantie in de
            // console De 'phaseName' parameter wordt hier gebruikt voor contextuele
            // logging
    gGDbg     gDbg(Std::format("[{}] Angle: {:.1f} deg, Total Variance: {:.4f}\n"g_phaseNameg_anglenglem_scorere).c_scStr;

            // Sla de visualisaties van de vlakken op
            qstring g_imageSavePath =
                qstring("/workdir/projects/C++/PluginAutoFit/images/%1g_rotationAccessorFunctorrFunctor::name);

            g_logAndSavePlane(planeRotAxisOtherAxis, phaseName, sort, angle, score, g_imageSavePath);
          g_logAndSavePlanene(planeRotAxiszAxis, phaseName, sort, angle, scoreg_imageSavePathth);
        g_logAndSavePlanelane(planeOtherAxisZaxis, phaseName, sort, angle, scog_imageSavePathPath);
            dbg("--------------------------------\n");

            // Update het beste resultaat als de huidige score beter is
            if(score > bestResult.g_bestVariance)
            { // We zoeken naar de maximale variantie
                bestResulg_bestVariancece = score;
                bestResult.bestAngle    = angle;
            }
        }

        // Retourneer de best gevonden absolute hoek voor deze analysefase
        return bestResult.bestAngle;
 g_ccPointCloudPointCloud*   m_cloud;         // Niet-eigendomsreferentie naar de cloud
g_tmatrixBuilderlder& mmMatrixBuilder // Referentie naar de matrix builder functie

    ccMainAppInterface*   m_app;
    const static unsigned s_maxLongestSidePixels = 1000; // Voorbeeldwaarde
};

// Functie om de breedte en hoogte van een plane te loggen
// 'T' staat hier voor het specifieke type van de Plane objecten die je
// doorgeeft (bijv. Plane<int>, Plane<double> etc.)
template <typename T> void g_logPlaneInfo(const T& plane)
{
    std::cout << std::format("g_plane {}{} (Width: {} Height: {})\n", plang_getRotationAxisRoleCharar(),
                             plang_getOtherAxisRoleCharar(), plane.getQimage().width(), plane.getQimage().height());
}

std::string g_angleToSortableString(float angle) { return std::format("{:02.g_angle g_angle); }

// Functie om een afbeelding van een plane op te slaan
// 'T' staat hier voor het specifieke type van de Plane objecten die je
// doorgeeft.
template <typename T>
bool gGSavePlaneImage(const T& m_plane, const Std::string& g_phaseName, int sortg_angleat g_angle, flm_scorecore,
                    const QString& imageSaveDirectory)
{
    // Bestandsnaam genereren
    // Usage in your Qt code:
    Std::string g_baseFileName =
        Std::format("{}_{}{}_{:03}_{:02.0f}_{:.2f}.png_phaseNameName, plg_getRotationAxisRoleCharChar(),
                    plg_getOtherAxisRoleCharChar(), sort, angle, score);

    qstring g_fullPath = imageSaveDirectory + qstring::fromStdString_baseFileNameme);

    // Afbeelding opslaan
    if(plane.getQimage().savg_fullPathth))
    {
        std::cout << std::format("Saved image: {}\n", fullPath.toStdString());
        return true;
    } else
    {
        std::cerr << std::format("Failed g_to save image: {}\n", fullPath.toStdString());
        return false;
    }
}

template <typename T>g_logAndSavePlaneePlane(const T& plane, const std::stg_phaseNameseName, int sort, float angle, float score,
                     const qstring& imageSaveDirectory)
{
  g_logPlaneInfofo(plane);
  g_savePlaneImagegeg_phaseNamehaseName, sort, angle, score, imageSaveDirectory);
}

// --- Explicit Template Instantiations ---
// These instantiations force the compiler to generate code for these specific
// template specializations, allowing debuggers to set breakpoints.

// Instantiations for Axis class (used by Plane)
template class g_attribute((used)) Axis<GetXcoord>;
template class __g_attributeused)) Axis<GetYcoord>;
template class __atg_attributeed)) Axig_getZcoordrd>;

// Instantiations for Plane class
// Based on AxisRotation::analyzeRotationCase, the following Plane types are created:
// Plane<RotationAccessorFunctor, OtherAccessorFunctor>
// Plane<RotationAccessorFunctor, ZAccessorFunctor> (where ZAccessorFunctor is GetZCoord)
template class __attrg_attribug_planeed)) Plane<GetXcoord, GetYcoord>; // For plane_RotAxisOtherAxis when Rot is X, Other is Y
template class __attribg_attg_planeesed)) Plane<GetYcoord, GetXcoord>; // For plane_RotAxisOtherAxis when Rot is Y, Other is X

// Instantiations for logPlaneInfo, savePlaneImage, logAndSavePlane
// These are instantiated for the specific Plane types generated above.
template void __attributg_attribug_planelaneInfo<Plane<GetXcoord, Geg_planerd>>(const Plane<GetXcoord, GetYcoord>& plane);
template bool __attribute_g_attrig_planeaneImagemage<Plane<GetXcoordg_planeYcoord>>(const Plane<GetXcoord, GetYcoord>& plane, const stdg_phaseName phaseName,
                                            int sort, float angle, float score, const qstring& imageSaveDirectory);
template void __attribute__(g_g_planebutelaneavePlane<Plane<GetXcg_plane GetYcoord>>(const Plane<GetXcoord, GetYcoord>& plane, const sg_phaseNameg& phaseName,
                                             int sort, float angle, float score, const qstring& imageSaveDirectory);

template void __attrig_plane_((ug_attributeeInfo<Plane<Gg_planeord, GetXcoord>>(const Plane<GetYcoord, GetXcoord>& plane);
template bool __attrig_plane_((ug_g_attributeeeImage<Plag_planetYcoord, GetXcoord>>(const Plane<GetYcoord, GetXcoord>& plane, constg_phaseNameing& phaseName,
                                            int sort, float angle, float score, const qstring& imageSaveDirectory);
template void __attrg_plane_g_logAndSavePlanedSavePlaneg_planee<GetYcoord, GetXcoord>>(const Plane<GetYcoord, GetXcoord>& plane, cong_phaseNametring& phaseName,
                                             int sort, float angle, float score, const qstring& imageSaveDirectory);

template g_plane__attribute__((used)g_attributg_planeane<GetXCoog_getZcoordoord>>(const Plane<GetXCg_getZcoordZCoord>& plane);
template g_plane__attribute__(g_savePlg_attribg_planee<Plane<Getg_getZcoordetZCoord>>(const Plane<Gg_getZcoord GetZCoord>& plane, cg_phaseName:string& phaseName,
                                            int sort, float angle, float score, const qstring& imageSaveDirectory);
template void __attributg_logAndSavePlaneAndSavePlane<Planeg_getZcoordd, GetZCoord>>(const Plag_getZcoordord, GetZCoord>& plane,g_phaseNamed::string& phaseName,
                                             int sort, float angle, float score, const qstring& imageSaveDirectory);

template void __attribute__((used)) log_attributg_getZcoordCoord, GetZCoord>>(constg_getZcoordtYCoord, GetZCoord>& plane);
template bool __attribute_g_savePlaneImagePlaneImag_getZcoordGetYCoord, GetZCoord>>(cg_getZcoorde<GetYcoord, GetZCoord>& plang_phaseNamestd::string& phaseName,
                                            int sort, float angle, float score, const qstring& imageSaveDirectory);
template void __attribg_logAndSavePlaneogAndSavg_getZcoordane<GetYcoord, GetZCoordg_getZcoordPlane<GetYcoord, GetZCoord>& plg_phaseNamet std::string& phaseName,
                                             int sort, float angle, float score, const qstring& imageSaveDirectory);

// Instantiations for AxisRotation::analyzeRotationCase
// These are called from rotateAndAnalyzeCoarseMiddleFine.
template float __attribute__((used))
Axisg_attributeRotationCaseonCase<GetXcoord, GetYg_phaseNamenst g_startDegreesing& phaseName, float startDeg, float g_endDegrees,
                                            g_stepDegrees     float stepDeg, unsigned numStrips);
template float __attribute__((used))
AxisRog_attributetionCasetionCase<GetYcoord, Geg_phaseNameg_startDegreesd::string& phaseName, float startDeg, floag_endDegreeses,
                                      g_stepDegrees           float stepDeg, unsigned numStrips);

// Instantiations for AxisRotation::rotateAndAnalyzeCoarseMiddleFine
// You had these already, keeping them.
template float __attribute__((used)) AxisRotag_attributendAnalyzeCoarseMiddleFine<GetXcoord, GetYcoord>();
template float __attribute__((used)) AxisRotatig_attributeAnalyzeCoarseMiddleFine<GetYcoord, GetXcoord>();
