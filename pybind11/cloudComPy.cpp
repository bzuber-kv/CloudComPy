//##########################################################################
//#                                                                        #
//#                              CloudComPy                                #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; either version 3 of the License, or     #
//#  any later version.                                                    #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#  You should have received a copy of the GNU General Public License     #
//#  along with this program. If not, see <https://www.gnu.org/licenses/>. #
//#                                                                        #
//#          Copyright 2020-2021 Paul RASCLE www.openfields.fr             #
//#                                                                        #
//##########################################################################

#include "cloudComPy.hpp"

#include "initCC.h"
#include "pyCC.h"
#include "PyScalarType.h"
#include <ccGLMatrix.h>
#include <ccHObject.h>
#include <ccPointCloud.h>
#include <ccOctree.h>
#include <ccMesh.h>
#include <ScalarField.h>
#include <ccNormalVectors.h>
#include <ccHObjectCaster.h>
#include <ccRasterGrid.h>
#include <ccClipBox.h>
#include <ccCommon.h>
#include <AutoSegmentationTools.h>
#include <ParallelSort.h>
#include <ccPointCloudInterpolator.h>
#include <ReferenceCloud.h>

//#include <ccMainAppInterface.h>

#include <QString>
#include <QSharedPointer>
#include <vector>
#include <tuple>

#include "optdefines.h"

#include "pyccTrace.h"
#include "cloudComPy_DocStrings.hpp"

QString greet()
{
   return QString("Hello, World, this is CloudCompare Python Interface: 'CloudComPy'");
}

void initCC_py()
{
#ifdef _PYTHONAPI_DEBUG_
    ccLogTrace::settrace();
#endif
    PyObject *cc_module;
    cc_module = PyImport_ImportModule("cloudComPy");
    const char* modulePath = PyUnicode_AsUTF8(PyModule_GetFilenameObject(cc_module));
    CCTRACE("modulePath: " <<  modulePath);
    initCC::init(modulePath);
}

void setTraces_py(bool isActive)
{
    if (isActive)
    {
        ccLogTrace::settrace(1);
    }
    else
    {
        ccLogTrace::settrace(0);
    }
}

 void initCloudCompare_py()
{
    pyCC* pyCCInternals = initCloudCompare();
}

const char* getScalarType()
{
    //Get the scalar type used in cloudCompare under the form defined in Numpy: 'float32' or 'float64'
    return CC_NPY_FLOAT_STRING;
}

struct ICPres
{
    ccPointCloud* aligned;
    ccGLMatrix transMat;
    double finalScale;
    double finalRMS;
    unsigned finalPointCount;
};

ICPres ICP_py(  ccHObject* data,
                ccHObject* model,
                double minRMSDecrease,
                unsigned maxIterationCount,
                unsigned randomSamplingLimit,
                bool removeFarthestPoints,
                CCCoreLib::ICPRegistrationTools::CONVERGENCE_TYPE method,
                bool adjustScale,
                double finalOverlapRatio = 1.0,
                bool useDataSFAsWeights = false,
                bool useModelSFAsWeights = false,
                int transformationFilters = CCCoreLib::RegistrationTools::SKIP_NONE,
                int maxThreadCount = 0)
{
    ICPres a;
    ICP(data,
        model,
        a.transMat,
        a.finalScale,
        a.finalRMS,
        a.finalPointCount,
        minRMSDecrease,
        maxIterationCount,
        randomSamplingLimit,
        removeFarthestPoints,
        method,
        adjustScale,
        finalOverlapRatio,
        useDataSFAsWeights,
        useModelSFAsWeights,
        transformationFilters,
        maxThreadCount);
    a.aligned = dynamic_cast<ccPointCloud*>(data);
    return a;
}

py::tuple importFilePy(const char* filename,
    CC_SHIFT_MODE mode = AUTO,
    double x = 0,
    double y = 0,
    double z = 0,
    QString extraData = QString())
{
    std::vector<ccMesh*> meshes;
    std::vector<ccPointCloud*> clouds;
    std::vector<QString> structure;
    std::vector<ccHObject*> entities = importFile(filename, mode, x, y, z, extraData, &structure);
    for( auto entity : entities)
    {
        ccMesh* mesh = ccHObjectCaster::ToMesh(entity);
        if (mesh)
        {
            meshes.push_back(mesh);
        }
        else
        {
            ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(entity);
            if (cloud)
            {
                clouds.push_back(cloud);
            }
        }
    }
    py::tuple res = py::make_tuple(meshes, clouds, structure);
    return res;
}

ccPointCloud* loadPointCloudPy(
    const char* filename,
    CC_SHIFT_MODE mode = AUTO,
    int skip = 0,
    double x = 0,
    double y = 0,
    double z = 0,
    QString extraData = QString())
{
    std::vector<ccMesh*> meshes;
    std::vector<ccPointCloud*> clouds;
    std::vector<ccHObject*> entities = importFile(filename, mode, x, y, z, extraData);
    for( auto entity : entities)
    {
        ccMesh* mesh = ccHObjectCaster::ToMesh(entity);
        if (mesh)
        {
            meshes.push_back(mesh);
        }
        else
        {
            ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(entity);
            if (cloud)
            {
                clouds.push_back(cloud);
            }
        }
    }
    if (clouds.size())
        return clouds.back();
    else
        return nullptr;
}


ccMesh* loadMeshPy(
    const char* filename,
    CC_SHIFT_MODE mode = AUTO,
    int skip = 0,
    double x = 0,
    double y = 0,
    double z = 0,
    QString extraData = QString())
{
    std::vector<ccMesh*> meshes;
    std::vector<ccPointCloud*> clouds;
    std::vector<ccHObject*> entities = importFile(filename, mode, x, y, z, extraData);
    for( auto entity : entities)
    {
        ccMesh* mesh = ccHObjectCaster::ToMesh(entity);
        if (mesh)
        {
            meshes.push_back(mesh);
        }
        else
        {
            ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(entity);
            if (cloud)
            {
                clouds.push_back(cloud);
            }
        }
    }
    if (meshes.size())
        return meshes.back();
    else
        return nullptr;
}

void deleteEntity(ccHObject* entity)
{
    delete entity;
}

py::tuple ExtractSlicesAndContours_py
    (
    std::vector<ccHObject*> entities,
    ccBBox bbox,
    ccGLMatrix bboxTrans = ccGLMatrix(),
    bool singleSliceMode = true,
    bool processRepeatX =false,
    bool processRepeatY =false,
    bool processRepeatZ =true,
    bool extractEnvelopes = false,
    PointCoordinateType maxEdgeLength = 0,
    int envelopeType = 0,
    bool extractLevelSet = false,
    double levelSetGridStep = 0,
    int levelSetMinVertCount= 0,
    PointCoordinateType gap = 0,
    bool multiPass = false,
    bool splitEnvelopes = false,
    bool projectOnBestFitPlane = false,
    bool generateRandomColors = false)
{
    std::vector<ccGenericPointCloud*> clouds;
    std::vector<ccGenericMesh*> meshes;
    for(auto obj: entities)
    {
        if (obj->isKindOf(CC_TYPES::MESH))
        {
            ccMesh* mesh = ccHObjectCaster::ToMesh(obj);
            meshes.push_back(mesh);
        }
        else if(obj->isKindOf(CC_TYPES::POINT_CLOUD))
        {
            ccGenericPointCloud* cloud = ccHObjectCaster::ToGenericPointCloud(obj);
            clouds.push_back(cloud);
        }
    }
    CCTRACE("clouds: " << clouds.size() << " meshes: " << meshes.size());
    ccClipBox clipBox;
    clipBox.set(bbox, bboxTrans);
    clipBox.enableGLTransformation(true);
    bool processDimensions[3] = {processRepeatX, processRepeatY, processRepeatZ};

    Envelope_Type val[3] = {LOWER, UPPER, FULL};
    if (envelopeType <0) envelopeType = 0;
    if (envelopeType >2) envelopeType = 2;
    Envelope_Type envelType = val[envelopeType];

    std::vector<ccHObject*> outputSlices;
    std::vector<ccPolyline*> outputEnvelopes;
    std::vector<ccPolyline*> levelSet;
    ExtractSlicesAndContoursClone(clouds, meshes, clipBox, singleSliceMode, processDimensions, outputSlices,
                             extractEnvelopes, maxEdgeLength, envelType, outputEnvelopes,
                             extractLevelSet, levelSetGridStep, levelSetMinVertCount, levelSet,
                             gap, multiPass, splitEnvelopes, projectOnBestFitPlane, false, generateRandomColors, nullptr);
    py::tuple res = py::make_tuple(outputSlices, outputEnvelopes, levelSet);
    return res;
}

struct ComponentIndexAndSize
{
    unsigned index;
    unsigned size;

    ComponentIndexAndSize(unsigned i, unsigned s) : index(i), size(s) {}

    static bool DescendingCompOperator(const ComponentIndexAndSize& a, const ComponentIndexAndSize& b)
    {
        return a.size > b.size;
    }
};

//! from MainWindow::createComponentsClouds
std::tuple <std::vector<ccPointCloud*>, std::vector<ccPointCloud*>>
createComponentsClouds_(ccGenericPointCloud* cloud,
                        CCCoreLib::ReferenceCloudContainer& components,
                        unsigned minPointsPerComponent,
                        bool randomColors,
                        bool sortBysize=true)
{
    CCTRACE("createComponentsClouds_ " << randomColors);
    std::vector<ccPointCloud*> resultClouds;
    std::vector<ccPointCloud*> residualClouds;
    if (!cloud || components.empty())
        return {resultClouds, residualClouds};

    std::vector<ComponentIndexAndSize> sortedIndexes;
    std::vector<ComponentIndexAndSize>* _sortedIndexes = nullptr;
    if (sortBysize)
    {
        try
        {
            sortedIndexes.reserve(components.size());
        }
        catch (const std::bad_alloc&)
        {
            CCTRACE("[CreateComponentsClouds] Not enough memory to sort components by size!");
            sortBysize = false;
        }

        if (sortBysize) //still ok?
        {
            unsigned compCount = static_cast<unsigned>(components.size());
            for (unsigned i = 0; i < compCount; ++i)
            {
                sortedIndexes.emplace_back(i, components[i]->size());
            }

            ParallelSort(sortedIndexes.begin(), sortedIndexes.end(), ComponentIndexAndSize::DescendingCompOperator);

            _sortedIndexes = &sortedIndexes;
        }
    }

    //we create "real" point clouds for all input components
    {
        ccPointCloud* pc = cloud->isA(CC_TYPES::POINT_CLOUD) ? static_cast<ccPointCloud*>(cloud) : nullptr;
        CCCoreLib::ReferenceCloud* refCloud = new CCCoreLib::ReferenceCloud(cloud);
        ccPointCloud* residualCloud = nullptr;

        //for each component
        int nbComp =0;
        for (size_t i = 0; i < components.size(); ++i)
        {
            CCCoreLib::ReferenceCloud* compIndexes = _sortedIndexes ? components[_sortedIndexes->at(i).index] : components[i];

            //if it has enough points
            if (compIndexes->size() >= minPointsPerComponent)
            {
                //we create a new entity
                ccPointCloud* compCloud = (pc ? pc->partialClone(compIndexes) : ccPointCloud::From(compIndexes));
                if (compCloud)
                {
                    //shall we colorize it with random color?
                    if (randomColors)
                    {
                        ccColor::Rgb col = ccColor::Generator::Random();
                        compCloud->setColor(col);
                    }

                    //'shift on load' information
                    if (pc)
                    {
                        compCloud->copyGlobalShiftAndScale(*pc);
                    }
                    compCloud->setName(QString("CC#%1").arg(nbComp));

                    //we add new CC to group
                    resultClouds.push_back(compCloud);
                    nbComp++;
                }
                else
                {
                    CCTRACE("[CreateComponentsClouds] Failed to create component " << nbComp << "(not enough memory)");
                }
            }
            else
            {
                // regroup all small chunks in one entity
                unsigned numberOfPoints = compIndexes->size();
                for (unsigned i = 0; i < numberOfPoints; ++i)
                {
                    //add the point to the current component
                    if (!refCloud->addPointIndex(i))
                    {
                        //not enough memory
                        CCTRACE("not enough memory!");
                        delete refCloud;
                        refCloud = nullptr;
                        break;
                    }
                }
            }

            delete compIndexes;
            compIndexes = nullptr;
        }

        if (refCloud)
        {
            residualCloud = (pc ? pc->partialClone(refCloud) : ccPointCloud::From(refCloud));
            residualClouds.push_back(residualCloud);
            delete refCloud;
            refCloud = nullptr;
        }
        components.clear();

        if (nbComp == 0)
        {
            CCTRACE("No component was created! Check the minimum size...");
        }
        else
        {
            CCTRACE("[CreateComponentsClouds] " << nbComp << " component(s) were created from cloud " << cloud->getName().toStdString());
        }
    }
    return {resultClouds, residualClouds};
}

py::tuple ExtractConnectedComponents_py(std::vector<ccHObject*> entities,
                                        int octreeLevel=8,
                                        int minComponentSize=100,
                                        int maxNumberComponents=100,
                                        bool randomColors=false)
{
    CCTRACE("ExtractConnectedComponents_py");
    int realComponentCount = 0;
    int nbCloudDone = 0;

    std::vector<ccHObject*> resultComponents;
    std::vector<ccHObject*> residualComponents;
    py::tuple res = py::make_tuple(nbCloudDone, resultComponents, residualComponents);

    std::vector<ccGenericPointCloud*> clouds;
    {
        for ( ccHObject *entity : entities )
        {
            if (entity->isKindOf(CC_TYPES::POINT_CLOUD))
                clouds.push_back(ccHObjectCaster::ToGenericPointCloud(entity));
        }
    }

    size_t count = clouds.size();
    if (count == 0)
        return res;

    bool randColors = randomColors;

    for ( ccGenericPointCloud *cloud : clouds )
    {
        if (cloud && cloud->isA(CC_TYPES::POINT_CLOUD))
        {
            CCTRACE("cloud");
            ccPointCloud* pc = static_cast<ccPointCloud*>(cloud);

            ccOctree::Shared theOctree = cloud->getOctree();
            if (!theOctree)
            {
                theOctree = cloud->computeOctree(nullptr);
                if (!theOctree)
                {
                    CCTRACE("Couldn't compute octree for cloud " <<cloud->getName().toStdString());
                    break;
                }
            }

            //we create/activate CCs label scalar field
            int sfIdx = pc->getScalarFieldIndexByName(CC_CONNECTED_COMPONENTS_DEFAULT_LABEL_NAME);
            if (sfIdx < 0)
            {
                sfIdx = pc->addScalarField(CC_CONNECTED_COMPONENTS_DEFAULT_LABEL_NAME);
            }
            if (sfIdx < 0)
            {
                CCTRACE("Couldn't allocate a new scalar field for computing CC labels! Try to free some memory ...");
                break;
            }
            pc->setCurrentScalarField(sfIdx);

            //we try to label all CCs
            CCTRACE("---");
            CCCoreLib::ReferenceCloudContainer components;
            int componentCount = CCCoreLib::AutoSegmentationTools::labelConnectedComponents(cloud,
                                                                                        static_cast<unsigned char>(octreeLevel),
                                                                                        false,
                                                                                        nullptr,
                                                                                        theOctree.data());

            CCTRACE("---");
            if (componentCount >= 0)
            {
                //if successful, we extract each CC (stored in "components")

                pc->getCurrentInScalarField()->computeMinAndMax();
                if (!CCCoreLib::AutoSegmentationTools::extractConnectedComponents(cloud, components))
                {
                    CCTRACE("[ExtractConnectedComponents] Something went wrong while extracting CCs from cloud " << cloud->getName().toStdString());
                }
                CCTRACE("---");

                //safety test
                {
                    for (size_t i = 0; i < components.size(); ++i)
                    {
                        if (components[i]->size() >= minComponentSize)
                        {
                            ++realComponentCount;
                        }
                    }
                }
                CCTRACE("total components: " << componentCount << " with " << realComponentCount << " components of size > " << minComponentSize);

                if (realComponentCount > maxNumberComponents)
                {
                    //too many components
                    CCTRACE("Too many components: " << realComponentCount << " for a maximum of: " << maxNumberComponents);
                    CCTRACE("Extraction incomplete, modify some parameters and retry");
                    pc->deleteScalarField(sfIdx);
                    res = py::make_tuple(nbCloudDone, resultComponents);
                    return res;
                }
            }
            else
            {
                CCTRACE("[ExtractConnectedComponents] Something went wrong while extracting CCs from cloud " << cloud->getName().toStdString());
            }

            //we delete the CCs label scalar field (we don't need it anymore)
            pc->deleteScalarField(sfIdx);
            sfIdx = -1;

            //we create "real" point clouds for all CCs
            if (!components.empty())
            {
                std::vector<ccPointCloud*> resultClouds;
                std::vector<ccPointCloud*> residualClouds;
                std::tie(resultClouds, residualClouds) = createComponentsClouds_(cloud, components, minComponentSize, randColors, true);
                for (ccPointCloud* cloud : resultClouds)
                    resultComponents.push_back(cloud);
                for (ccPointCloud* cloud : residualClouds)
                    residualComponents.push_back(cloud);
            }
            nbCloudDone++;
            CCTRACE("nbCloudDone: " << nbCloudDone);
        }
    }
    res = py::make_tuple(nbCloudDone, resultComponents, residualComponents);
    return res;
}

//struct interpolatorParameters: public ccPointCloudInterpolator::Parameters
//{
//}

//{
//    ccPointCloudInterpolator::Parameters::Method method = ccPointCloudInterpolator::Parameters::Method::NEAREST_NEIGHBOR;
//    ccPointCloudInterpolator::Parameters::Algo algo = ccPointCloudInterpolator::Parameters::Algo::AVERAGE;
//    unsigned knn = 0;
//    float radius = 0;
//    double sigma = 0;
//};

bool InterpolateScalarFieldsFrom_py(ccPointCloud* destCloud,
                                    ccPointCloud* srcCloud,
                                    std::vector<int> sfIndexes,
                                    const ccPointCloudInterpolator::Parameters& params,
                                    unsigned char octreeLevel = 0)
{
    CCTRACE("InterpolateScalarFieldsFrom_py");
    return ccPointCloudInterpolator::InterpolateScalarFieldsFrom(destCloud, srcCloud, sfIndexes, params, nullptr, octreeLevel);
}

// from MainWindow::AddToRemoveList helper for MergePy
void AddToRemoveListPy(ccHObject* toRemove, ccHObject::Container& toBeRemovedList)
{
    // is a parent or sibling already in the "toBeRemoved" list?
    size_t count = toBeRemovedList.size();
    for (size_t j = 0; j < count;)
    {
        if (toBeRemovedList[j]->isAncestorOf(toRemove))
        {
            // nothing to do, we already have an ancestor
            return;
        }
        else if (toRemove->isAncestorOf(toBeRemovedList[j]))
        {
            // we don't need to keep the children
            toBeRemovedList[j] = toBeRemovedList.back();
            toBeRemovedList.pop_back();
            count--;
        }
        else
        {
            // forward
            ++j;
        }
    }

    toBeRemovedList.push_back(toRemove);
}

//! from MainWindow::doActionMerge
ccHObject* MergeEntitiesPy(std::vector<ccHObject*> entities,
                           bool deleteOriginalClouds=false,
                           bool createSFcloudIndex=false,
                           bool createSubMeshes=false)
{
    CCTRACE("MergeEntitiesPy");
    //let's look for clouds or meshes (warning: we don't mix them)
    std::vector<ccPointCloud*> clouds;
    std::vector<ccMesh*> meshes;

    try
    {
        for ( ccHObject *entity : entities )
        {
            if (!entity)
                continue;

            if (entity->isA(CC_TYPES::POINT_CLOUD))
            {
                ccPointCloud* cloud = ccHObjectCaster::ToPointCloud(entity);
                clouds.push_back(cloud);

                // check whether this cloud is an ancestor of the first cloud in the selection
                if (clouds.size() > 1)
                {
                    if (clouds.back()->isAncestorOf(clouds.front()))
                    {
                        // this way we are sure that the first cloud is not below any other cloud
                        std::swap(clouds.front(), clouds.back());
                    }
                }
            }
            else if (entity->isKindOf(CC_TYPES::MESH))
            {
                ccMesh* mesh = ccHObjectCaster::ToMesh(entity);
                //this is a purely theoretical test for now!
                if (mesh && mesh->getAssociatedCloud() && mesh->getAssociatedCloud()->isA(CC_TYPES::POINT_CLOUD))
                {
                    meshes.push_back(mesh);
                }
                else
                {
                    CCTRACE("Only meshes with standard vertices are handled for now! Can't merge entity " << entity->getName().toStdString());
                }
            }
            else
            {
                CCTRACE("Entity " << entity->getName().toStdString() << " is neither a cloud nor a mesh, can't merge it!");
            }
        }
    }
    catch (const std::bad_alloc&)
    {
        CCTRACE("Not enough memory!");
        return nullptr;
    }

    if (clouds.empty() && meshes.empty())
    {
        CCTRACE("Select only clouds or meshes!");
        return nullptr;
    }
    if (!clouds.empty() && !meshes.empty())
    {
        CCTRACE("Can't mix point clouds and meshes!");
    }

    //merge clouds?
    if (!clouds.empty())
    {
        CCTRACE("clouds");

        //we will remove the useless clouds/meshes later
        ccHObject::Container toBeRemoved;

        ccPointCloud* firstCloud = nullptr;

        //whether to generate the 'original cloud index' scalar field or not
        CCCoreLib::ScalarField* ocIndexSF = nullptr;
        size_t cloudIndex = 0;
        int sfIdx = -1;

        for (size_t i = 0; i < clouds.size(); ++i)
        {
            CCTRACE("cloud: " << i);
            ccPointCloud* pc = clouds[i];
            if (!firstCloud)
            {
                if (deleteOriginalClouds)
                {
                    firstCloud = pc;
                }
                else
                {
                    firstCloud = pc->cloneThis();
                }

                if (createSFcloudIndex)
                {
                    sfIdx = firstCloud->getScalarFieldIndexByName(CC_ORIGINAL_CLOUD_INDEX_SF_NAME);
                    if (sfIdx < 0)
                    {
                        sfIdx = firstCloud->addScalarField(CC_ORIGINAL_CLOUD_INDEX_SF_NAME);
                    }
                    if (sfIdx < 0)
                    {
                        CCTRACE("Couldn't allocate a new scalar field for storing the original cloud index! Try to free some memory ...");
                        return nullptr;
                    }
                    else
                    {
                        ocIndexSF = firstCloud->getScalarField(sfIdx);
                        ocIndexSF->fill(0);
                        firstCloud->setCurrentDisplayedScalarField(sfIdx);
                        CCTRACE("NumberOfScalarFields: " << firstCloud->getNumberOfScalarFields());
                        CCTRACE("SF name: " << ocIndexSF->getName());
                    }
                }
            }
            else
            {
                unsigned countBefore = firstCloud->size();
                unsigned countAdded = pc->size();
                if (deleteOriginalClouds)
                    *firstCloud += pc;
                else
                    *firstCloud += pc->cloneThis();
                CCTRACE("  new size: " << firstCloud->size());
                //success?
                if (firstCloud->size() == countBefore + countAdded)
                {
                    ccHObject* toRemove = nullptr;
                    //if the entity to remove is inside a group with a unique child, we can remove the group as well
                    ccHObject* parent = pc->getParent();
                    if (parent && parent->isA(CC_TYPES::HIERARCHY_OBJECT) && parent->getChildrenNumber() == 1 ) //&& parent != firstCloudContext.parent)
                        toRemove = parent;
                    else
                        toRemove = pc;

                    if (deleteOriginalClouds)
                        AddToRemoveListPy(toRemove, toBeRemoved);

                    if (ocIndexSF)
                    {
                        CCTRACE("  ocIndexSF")
                        ocIndexSF->resizeSafe(firstCloud->size());
                        ScalarType index = static_cast<ScalarType>(++cloudIndex);
                        for (unsigned i = 0; i < countAdded; ++i)
                        {
                            ocIndexSF->setValue(countBefore + i, index);
                        }
                        CCTRACE("  ocIndexSF")
                    }
                }
                else
                {
                    CCTRACE("Fusion failed! (not enough memory?)");
                    break;
                }
                pc = nullptr;
            }
        }

        if (ocIndexSF)
        {
            CCTRACE("SF computeMinAndMax SF: " << sfIdx);
            CCTRACE("NumberOfScalarFields: " << firstCloud->getNumberOfScalarFields());
            ocIndexSF->computeMinAndMax();
            firstCloud->setCurrentDisplayedScalarField(sfIdx);
            firstCloud->showSF(true);
        }

        //something to remove?
        if (deleteOriginalClouds)
        {
        for (ccHObject* toRemove : toBeRemoved)
            {
                if (toRemove->getParent())
                    toRemove->getParent()->removeChild(toRemove);
                else
                    delete toRemove;
            }
            toBeRemoved.clear();
        }
        return firstCloud;
    }
    //merge meshes?
    else if (!meshes.empty())
    {
        //meshes are merged
        ccPointCloud* baseVertices = new ccPointCloud("vertices");
        ccMesh* baseMesh = new ccMesh(baseVertices);
        baseMesh->setName("Merged mesh");
        baseMesh->addChild(baseVertices);
        baseVertices->setEnabled(false);

        for (ccMesh *mesh : meshes)
        {
            if (!baseMesh->merge(mesh, createSubMeshes))
            {
                CCTRACE("Fusion failed! (not enough memory?)");
                break;
            }
        }
        baseMesh->setDisplay_recursive(meshes.front()->getDisplay());
        baseMesh->setVisible(true);
        return baseMesh;
    }
    return nullptr;
}

PYBIND11_MODULE(_cloudComPy, m0)
{
    export_colors(m0);
    export_ScalarField(m0);
    export_ccGenericCloud(m0);
    export_ccPolyline(m0);
    export_ccOctree(m0);
    export_ccPointCloud(m0);
    export_ccMesh(m0);
    export_ccPrimitives(m0);
    export_distanceComputationTools(m0);
    export_geometricalAnalysisTools(m0);
    export_registrationTools(m0);
    export_cloudSamplingTools(m0);
    export_ccFacet(m0);
    export_ccSensor(m0);
    export_Neighbourhood(m0);

    m0.doc() = cloudComPy_doc;

    m0.def("greet", &greet);

    py::enum_<CC_SHIFT_MODE>(m0, "CC_SHIFT_MODE")
        .value("AUTO", CC_SHIFT_MODE::AUTO)
        .value("XYZ", CC_SHIFT_MODE::XYZ)
        .value("FIRST_GLOBAL_SHIFT", CC_SHIFT_MODE::FIRST_GLOBAL_SHIFT)
        .value("NO_GLOBAL_SHIFT", CC_SHIFT_MODE::NO_GLOBAL_SHIFT)
        .export_values();

    py::enum_<CC_DIRECTION>(m0, "CC_DIRECTION")
		.value("X", CC_DIRECTION::X)
		.value("Y", CC_DIRECTION::Y)
		.value("Z", CC_DIRECTION::Z)
        .export_values();

    py::enum_<CC_FILE_ERROR>(m0, "CC_FILE_ERROR")
        .value("CC_FERR_NO_ERROR", CC_FERR_NO_ERROR)
        .value("CC_FERR_BAD_ARGUMENT", CC_FERR_BAD_ARGUMENT)
        .value("CC_FERR_UNKNOWN_FILE", CC_FERR_UNKNOWN_FILE)
        .value("CC_FERR_WRONG_FILE_TYPE", CC_FERR_WRONG_FILE_TYPE)
        .value("CC_FERR_WRITING", CC_FERR_WRITING)
        .value("CC_FERR_READING", CC_FERR_READING)
        .value("CC_FERR_NO_SAVE", CC_FERR_NO_SAVE)
        .value("CC_FERR_NO_LOAD", CC_FERR_NO_LOAD)
        .value("CC_FERR_BAD_ENTITY_TYPE", CC_FERR_BAD_ENTITY_TYPE)
        .value("CC_FERR_CANCELED_BY_USER", CC_FERR_CANCELED_BY_USER)
        .value("CC_FERR_NOT_ENOUGH_MEMORY", CC_FERR_NOT_ENOUGH_MEMORY)
        .value("CC_FERR_MALFORMED_FILE", CC_FERR_MALFORMED_FILE)
        .value("CC_FERR_CONSOLE_ERROR", CC_FERR_CONSOLE_ERROR)
        .value("CC_FERR_BROKEN_DEPENDENCY_ERROR", CC_FERR_BROKEN_DEPENDENCY_ERROR)
        .value("CC_FERR_FILE_WAS_WRITTEN_BY_UNKNOWN_PLUGIN", CC_FERR_FILE_WAS_WRITTEN_BY_UNKNOWN_PLUGIN)
        .value("CC_FERR_THIRD_PARTY_LIB_FAILURE", CC_FERR_THIRD_PARTY_LIB_FAILURE)
        .value("CC_FERR_THIRD_PARTY_LIB_EXCEPTION", CC_FERR_THIRD_PARTY_LIB_EXCEPTION)
        .value("CC_FERR_NOT_IMPLEMENTED", CC_FERR_NOT_IMPLEMENTED)
        .export_values();

    py::enum_<CurvatureType>(m0, "CurvatureType")
        .value("GAUSSIAN_CURV", GAUSSIAN_CURV)
        .value("MEAN_CURV", MEAN_CURV)
        .value("NORMAL_CHANGE_RATE", NORMAL_CHANGE_RATE)
        .export_values();

    py::enum_<CCCoreLib::LOCAL_MODEL_TYPES>(m0, "LOCAL_MODEL_TYPES")
        .value("NO_MODEL", CCCoreLib::NO_MODEL )
        .value("LS", CCCoreLib::LS )
        .value("TRI", CCCoreLib::TRI )
        .value("QUADRIC", CCCoreLib::QUADRIC )
        .export_values();

    py::enum_<ccNormalVectors::Orientation>(m0, "Orientation")
        .value("PLUS_X", ccNormalVectors::PLUS_X )
        .value("MINUS_X", ccNormalVectors::MINUS_X )
        .value("PLUS_Y", ccNormalVectors::PLUS_Y )
        .value("MINUS_Y", ccNormalVectors::MINUS_Y )
        .value("PLUS_Z", ccNormalVectors::PLUS_Z )
        .value("MINUS_Z", ccNormalVectors::MINUS_Z )
        .value("PLUS_BARYCENTER", ccNormalVectors::PLUS_BARYCENTER )
        .value("MINUS_BARYCENTER", ccNormalVectors::MINUS_BARYCENTER )
        .value("PLUS_ORIGIN", ccNormalVectors::PLUS_ORIGIN )
        .value("MINUS_ORIGIN", ccNormalVectors::MINUS_ORIGIN )
        .value("PREVIOUS", ccNormalVectors::PREVIOUS )
        .value("PLUS_SENSOR_ORIGIN", ccNormalVectors::PLUS_SENSOR_ORIGIN )
        .value("MINUS_SENSOR_ORIGIN", ccNormalVectors::MINUS_SENSOR_ORIGIN )
        .value("UNDEFINED", ccNormalVectors::UNDEFINED )
        .export_values();

    py::enum_<ccRasterGrid::ProjectionType>(m0, "ProjectionType")
		.value("PROJ_MINIMUM_VALUE", ccRasterGrid::PROJ_MINIMUM_VALUE)
		.value("PROJ_AVERAGE_VALUE", ccRasterGrid::PROJ_AVERAGE_VALUE)
		.value("PROJ_MAXIMUM_VALUE", ccRasterGrid::PROJ_MAXIMUM_VALUE)
		.value("INVALID_PROJECTION_TYPE", ccRasterGrid::INVALID_PROJECTION_TYPE)
        .export_values();

    py::enum_<ccRasterGrid::EmptyCellFillOption>(m0, "EmptyCellFillOption")
		.value("LEAVE_EMPTY", ccRasterGrid::LEAVE_EMPTY)
		.value("FILL_MINIMUM_HEIGHT", ccRasterGrid::FILL_MINIMUM_HEIGHT)
		.value("FILL_MAXIMUM_HEIGHT", ccRasterGrid::FILL_MAXIMUM_HEIGHT)
		.value("FILL_CUSTOM_HEIGHT", ccRasterGrid::FILL_CUSTOM_HEIGHT)
		.value("FILL_AVERAGE_HEIGHT", ccRasterGrid::FILL_AVERAGE_HEIGHT)
		.value("INTERPOLATE_DELAUNAY", ccRasterGrid::INTERPOLATE_DELAUNAY)
        .value("KRIGING", ccRasterGrid::KRIGING)
        .export_values();

    py::enum_<ccRasterGrid::ExportableFields>(m0, "ExportableFields")
        .value("PER_CELL_HEIGHT", ccRasterGrid::PER_CELL_VALUE)
        .value("PER_CELL_COUNT", ccRasterGrid::PER_CELL_COUNT)
        .value("PER_CELL_MIN_HEIGHT", ccRasterGrid::PER_CELL_MIN_VALUE)
        .value("PER_CELL_MAX_HEIGHT", ccRasterGrid::PER_CELL_MAX_VALUE)
        .value("PER_CELL_AVG_HEIGHT", ccRasterGrid::PER_CELL_AVG_VALUE)
        .value("PER_CELL_HEIGHT_STD_DEV", ccRasterGrid::PER_CELL_VALUE_STD_DEV)
        .value("PER_CELL_HEIGHT_RANGE", ccRasterGrid::PER_CELL_VALUE_RANGE)
        .value("PER_CELL_INVALID", ccRasterGrid::PER_CELL_INVALID)
        .export_values();

    py::enum_<ccPointCloudInterpolator::Parameters::Method>(m0, "INTERPOL_METHOD")
        .value("NEAREST_NEIGHBOR", ccPointCloudInterpolator::Parameters::Method::NEAREST_NEIGHBOR)
        .value("K_NEAREST_NEIGHBORS", ccPointCloudInterpolator::Parameters::Method::K_NEAREST_NEIGHBORS)
        .value("RADIUS", ccPointCloudInterpolator::Parameters::Method::RADIUS)
        .export_values();

    py::enum_<ccPointCloudInterpolator::Parameters::Algo>(m0, "INTERPOL_ALGO")
        .value("AVERAGE", ccPointCloudInterpolator::Parameters::Algo::AVERAGE)
        .value("MEDIAN", ccPointCloudInterpolator::Parameters::Algo::MEDIAN)
        .value("NORMAL_DIST", ccPointCloudInterpolator::Parameters::Algo::NORMAL_DIST)
        .export_values();

    m0.def("importFile", &importFilePy,
           py::arg("filename"), py::arg("mode")=AUTO, py::arg("x")=0, py::arg("y")=0, py::arg("z")=0, py::arg("extraData")="",
           cloudComPy_importFile_doc);

    py::class_<ccPointCloudInterpolator::Parameters>(m0, "interpolatorParameters", cloudComPy_interpolatorParameters_doc)
        .def(py::init<>())
        .def_readwrite("method", &ccPointCloudInterpolator::Parameters::method,
                       cloudComPy_interpolatorParameters_doc)
        .def_readwrite("algos", &ccPointCloudInterpolator::Parameters::algo,
                       cloudComPy_interpolatorParameters_doc)
        .def_readwrite("knn", &ccPointCloudInterpolator::Parameters::knn,
                       cloudComPy_interpolatorParameters_doc)
        .def_readwrite("radius", &ccPointCloudInterpolator::Parameters::radius,
                       cloudComPy_interpolatorParameters_doc)
        .def_readwrite("sigma", &ccPointCloudInterpolator::Parameters::sigma,
                       cloudComPy_interpolatorParameters_doc)
        ;

    m0.def("interpolateScalarFieldsFrom", &InterpolateScalarFieldsFrom_py,
           py::arg("destCloud"), py::arg("srcCloud"), py::arg("sfIndexes"), py::arg("params"), py::arg("octreeLevel")=0,
           cloudComPy_interpolateScalarFieldsFrom_doc);

    m0.def("loadPointCloud", &loadPointCloudPy,
           py::arg("filename"),
           py::arg("mode")=AUTO, py::arg("skip")=0, py::arg("x")=0, py::arg("y")=0, py::arg("z")=0, py::arg("extraData")="",
           cloudComPy_loadPointCloud_doc, py::return_value_policy::reference);

    m0.def("loadMesh", &loadMeshPy,
           py::arg("filename"),py::arg("mode")=AUTO, py::arg("skip")=0, py::arg("x")=0, py::arg("y")=0, py::arg("z")=0, py::arg("extraData")="",
           cloudComPy_loadMesh_doc, py::return_value_policy::reference);

    m0.def("loadPolyline", &loadPolyline,
           py::arg("filename"), py::arg("mode")=AUTO, py::arg("skip")=0, py::arg("x")=0, py::arg("y")=0, py::arg("z")=0,
           cloudComPy_loadPolyline_doc, py::return_value_policy::reference);

    m0.def("deleteEntity", &deleteEntity, cloudComPy_deleteEntity_doc);

    m0.def("SaveMesh", &SaveMesh, cloudComPy_SaveMesh_doc);

    m0.def("SavePointCloud", &SavePointCloud,
           py::arg("cloud"), py::arg("filename"), py::arg("version")=QString(""), py::arg("pointFormat")=-1,
           cloudComPy_SavePointCloud_doc);

    m0.def("SaveEntities", &SaveEntities, cloudComPy_SaveEntities_doc);

    m0.def("initCC", &initCC_py, cloudComPy_initCC_doc);

    m0.def("setTraces", &setTraces_py, cloudComPy_setTraces_doc);

    m0.def("initCloudCompare", &initCloudCompare_py, cloudComPy_initCloudCompare_doc);

    m0.def("isPluginDraco", &pyccPlugins::isPluginDraco, cloudComPy_isPluginDraco_doc);

    m0.def("isPluginFbx", &pyccPlugins::isPluginFbx, cloudComPy_isPluginFbx_doc);

    m0.def("isPluginHPR", &pyccPlugins::isPluginHPR, cloudComPy_isPluginHPR_doc);

    m0.def("isPluginM3C2", &pyccPlugins::isPluginM3C2, cloudComPy_isPluginM3C2_doc);

    m0.def("isPluginMeshBoolean", &pyccPlugins::isPluginMeshBoolean, cloudComPy_isPluginMeshBoolean_doc);

    m0.def("isPluginPCL", &pyccPlugins::isPluginPCL, cloudComPy_isPluginPCL_doc);

    m0.def("isPluginPCV", &pyccPlugins::isPluginPCV, cloudComPy_isPluginPCV_doc);

    m0.def("isPluginCSF", &pyccPlugins::isPluginCSF, cloudComPy_isPluginCSF_doc);

    m0.def("isPluginCanupo", &pyccPlugins::isPluginCanupo, cloudComPy_isPluginCanupo_doc);

    m0.def("isPluginSRA", &pyccPlugins::isPluginSRA, cloudComPy_isPluginSRA_doc);

    m0.def("isPluginRANSAC_SD", &pyccPlugins::isPluginRANSAC_SD, cloudComPy_isPluginRANSAC_SD_doc);

    m0.def("computeCurvature", &computeCurvature, cloudComPy_computeCurvature_doc);

    m0.def("computeFeature", &computeFeature, cloudComPy_computeFeature_doc);

    m0.def("computeLocalDensity", &computeLocalDensity, cloudComPy_computeLocalDensity_doc);

    m0.def("computeApproxLocalDensity", &computeApproxLocalDensity, cloudComPy_computeApproxLocalDensity_doc);

    m0.def("computeRoughness", &computeRoughnessPy,
           py::arg("radius"), py::arg("clouds"), py::arg("roughnessUpDir")=CCVector3(0,0,0),
           cloudComPy_computeRoughness_doc);

    m0.def("computeMomentOrder1", &computeMomentOrder1, cloudComPy_computeMomentOrder1_doc);

    m0.def("filterBySFValue", &filterBySFValue, py::return_value_policy::reference, cloudComPy_filterBySFValue_doc);

    m0.def("GetPointCloudRadius", &GetPointCloudRadius,
           py::arg("clouds"), py::arg("nodes")=12, cloudComPy_GetPointCloudRadius_doc);

    m0.def("getScalarType", &getScalarType, cloudComPy_getScalarType_doc);

    py::class_<ICPres>(m0, "ICPres", cloudComPy_ICPres_doc)
       .def(py::init<>())
       .def_readonly("aligned", &ICPres::aligned, py::return_value_policy::reference_internal)
       .def_readwrite("transMat", &ICPres::transMat,
                       cloudComPy_ICPres_doc)
       .def_readwrite("finalScale", &ICPres::finalScale,
                      cloudComPy_ICPres_doc)
       .def_readwrite("finalRMS", &ICPres::finalRMS,
                      cloudComPy_ICPres_doc)
       .def_readwrite("finalPointCount", &ICPres::finalPointCount,
                      cloudComPy_ICPres_doc)
    ;

    m0.def("ICP", &ICP_py,
           py::arg("data"), py::arg("model"), py::arg("minRMSDecrease"), py::arg("maxIterationCount"), py::arg("randomSamplingLimit"),
           py::arg("removeFarthestPoints"), py::arg("method"), py::arg("adjustScale"), py::arg("finalOverlapRatio")=1.0,
           py::arg("useDataSFAsWeights")=false, py::arg("useModelSFAsWeights")=false,
           py::arg("transformationFilters")=CCCoreLib::RegistrationTools::SKIP_NONE,
           py::arg("maxThreadCount")=0,
           cloudComPy_ICP_doc);

    m0.def("computeNormals", &computeNormals,
           py::arg("selectedEntities"), py::arg("model")=CCCoreLib::LS,
           py::arg("useScanGridsForComputation")=true, py::arg("defaultRadius")=0.0, py::arg("minGridAngle_deg")=1.0,
           py::arg("orientNormals")=true, py::arg("useScanGridsForOrientation")=true,
           py::arg("useSensorsForOrientation")=true, py::arg("preferredOrientation")=ccNormalVectors::UNDEFINED,
           py::arg("orientNormalsMST")=true, py::arg("mstNeighbors")=6, py::arg("computePerVertexNormals")=true,
           cloudComPy_computeNormals_doc);

    py::class_<ReportInfoVol>(m0, "ReportInfoVol", cloudComPy_ReportInfoVol_doc)
        .def(py::init<>())
		.def_readonly("volume", &ReportInfoVol::volume)
		.def_readonly("addedVolume", &ReportInfoVol::addedVolume)
		.def_readonly("removedVolume", &ReportInfoVol::removedVolume)
		.def_readonly("surface", &ReportInfoVol::surface)
		.def_readonly("matchingPercent", &ReportInfoVol::matchingPercent)
		.def_readonly("ceilNonMatchingPercent", &ReportInfoVol::ceilNonMatchingPercent)
		.def_readonly("groundNonMatchingPercent", &ReportInfoVol::groundNonMatchingPercent)
		.def_readonly("averageNeighborsPerCell", &ReportInfoVol::averageNeighborsPerCell)
		;

    m0.def("ComputeVolume25D", &ComputeVolume25D,
           py::arg("reportInfo"), py::arg("ground"), py::arg("ceil"), py::arg("vertDim"),
           py::arg("gridStep"), py::arg("groundHeight"), py::arg("ceilHeight"),
           cloudComPy_ComputeVolume25D_doc);

    m0.def("invertNormals", &invertNormals, cloudComPy_invertNormals_doc);

    m0.def("ExtractConnectedComponents", &ExtractConnectedComponents_py,
           py::arg("clouds"),
           py::arg("octreeLevel")=8,
           py::arg("minComponentSize")=100,
           py::arg("maxNumberComponents")=100,
           py::arg("randomColors")=false,
           cloudComPy_ExtractConnectedComponents_doc);

    m0.def("ExtractSlicesAndContours", &ExtractSlicesAndContours_py,
            py::arg("entities"),
            py::arg("bbox"),
            py::arg("bboxTrans")=ccGLMatrix(),
            py::arg("singleSliceMode")=true,
            py::arg("processRepeatX")=false,
            py::arg("processRepeatY")=false,
            py::arg("processRepeatZ")=true,
            py::arg("extractEnvelopes")=false,
            py::arg("maxEdgeLength")=0,
            py::arg("envelopeType")=0,
            py::arg("extractLevelSet")=false,
            py::arg("levelSetGridStep")=0,
            py::arg("levelSetMinVertCount")=0,
            py::arg("gap")=0,
            py::arg("multiPass")=false,
            py::arg("splitEnvelopes")=false,
            py::arg("projectOnBestFitPlane")=false,
            py::arg("generateRandomColors")=false,
            cloudComPy_ExtractSlicesAndContours_doc);

    m0.def("MergeEntities", &MergeEntitiesPy,
           py::arg("entities"),
           py::arg("deleteOriginalClouds")=false,
           py::arg("createSFcloudIndex")=false,
           py::arg("createSubMeshes")=false,
           cloudComPy_MergeEntities_doc);

    m0.def("RasterizeToCloud", &RasterizeToCloud,
           py::arg("cloud"),
           py::arg("gridStep"),
           py::arg("vertDir") = CC_DIRECTION::Z,
           py::arg("outputRasterZ")=false,
           py::arg("outputRasterSFs")=false,
           py::arg("outputRasterRGB")=false,
           py::arg("pathToImages")=".",
           py::arg("resample")=false,
           py::arg("projectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
           py::arg("sfProjectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
           py::arg("emptyCellFillStrategy")=ccRasterGrid::LEAVE_EMPTY,
           py::arg("DelaunayMaxEdgeLength")=1.0,
           py::arg("KrigingParamsKNN")=8,
           py::arg("customHeight")=std::numeric_limits<double>::quiet_NaN(),
           py::arg("gridBBox")=ccBBox(),
           py::arg("export_perCellCount")=false,
           py::arg("export_perCellMinHeight")=false,
           py::arg("export_perCellMaxHeight")=false,
           py::arg("export_perCellAvgHeight")=false,
           py::arg("export_perCellHeightStdDev")=false,
           py::arg("export_perCellHeightRange")=false,
           cloudComPy_RasterizeToCloud_doc,
           py::return_value_policy::reference);

    m0.def("RasterizeToMesh", &RasterizeToMesh,
           py::arg("cloud"),
           py::arg("gridStep"),
           py::arg("vertDir") = CC_DIRECTION::Z,
           py::arg("outputRasterZ")=false,
           py::arg("outputRasterSFs")=false,
           py::arg("outputRasterRGB")=false,
           py::arg("pathToImages")=".",
           py::arg("resample")=false,
           py::arg("projectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
           py::arg("sfProjectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
           py::arg("emptyCellFillStrategy")=ccRasterGrid::LEAVE_EMPTY,
           py::arg("DelaunayMaxEdgeLength")=1.0,
           py::arg("KrigingParamsKNN")=8,
           py::arg("customHeight")=std::numeric_limits<double>::quiet_NaN(),
           py::arg("gridBBox")=ccBBox(),
           py::arg("export_perCellCount")=false,
           py::arg("export_perCellMinHeight")=false,
           py::arg("export_perCellMaxHeight")=false,
           py::arg("export_perCellAvgHeight")=false,
           py::arg("export_perCellHeightStdDev")=false,
           py::arg("export_perCellHeightRange")=false,
           cloudComPy_RasterizeToMesh_doc,
           py::return_value_policy::reference);

    m0.def("RasterizeGeoTiffOnly", &RasterizeGeoTiffOnly,
           py::arg("cloud"),
           py::arg("gridStep"),
           py::arg("vertDir") = CC_DIRECTION::Z,
           py::arg("outputRasterZ")=false,
           py::arg("outputRasterSFs")=false,
           py::arg("outputRasterRGB")=false,
           py::arg("pathToImages")=".",
           py::arg("resample")=false,
           py::arg("projectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
           py::arg("sfProjectionType")=ccRasterGrid::PROJ_AVERAGE_VALUE,
           py::arg("emptyCellFillStrategy")=ccRasterGrid::LEAVE_EMPTY,
           py::arg("DelaunayMaxEdgeLength")=1.0,
           py::arg("KrigingParamsKNN")=8,
           py::arg("customHeight")=std::numeric_limits<double>::quiet_NaN(),
           py::arg("gridBBox")=ccBBox(),
           py::arg("export_perCellCount")=false,
           py::arg("export_perCellMinHeight")=false,
           py::arg("export_perCellMaxHeight")=false,
           py::arg("export_perCellAvgHeight")=false,
           py::arg("export_perCellHeightStdDev")=false,
           py::arg("export_perCellHeightRange")=false,
           cloudComPy_RasterizeGeoTiffOnly_doc,
           py::return_value_policy::reference);

}
