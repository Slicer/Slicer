#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"
#include "vtkMRMLEMSTreeNode.h"
#include <stdexcept>
#include <stdlib.h>

#define vtkTestSetGetMacro(pass, obj, var, val)                       \
{                                                                     \
std::cerr << "Testing Set/Get " #var "...";                           \
obj->Set##var (val);                                                  \
bool match = val == obj->Get##var();                                  \
std::cerr << (match ? "OK" : "FAILED") << std::endl;                  \
pass = pass && match;                                                 \
}                                                                     \

#define vtkTestSetGetStringMacro(pass, obj, var, val)                 \
{                                                                     \
std::cerr << "Testing Set/Get " #var "...";                           \
obj->Set##var (val);                                                  \
bool match = std::string(val) == std::string(obj->Get##var());        \
std::cerr << (match ? "OK" : "FAILED") << std::endl;                  \
pass = pass && match;                                                 \
}                                                                     \

#define vtkTestSetGetPoint3DMacro(pass, obj, var, arrayType, val)         \
{                                                                         \
std::cerr << "Testing Set/Get " #var "...";                               \
obj->Set##var (val);                                                      \
arrayType outval[3];                                                      \
obj->Get##var (outval);                                                   \
bool match = val[0]==outval[0] && val[1]==outval[1] && val[2]==outval[2]; \
std::cerr << (match ? "OK" : "FAILED") << std::endl;                      \
pass = pass && match;                                                     \
}                                                                         \

#define vtkTestSetGetMacroIndex(pass, obj, var, val, index)           \
{                                                                     \
std::cerr << "Testing Set/Get " #var "...";                           \
obj->Set##var (index, val);                                           \
bool match = val == obj->Get##var(index);                             \
std::cerr << (match ? "OK" : "FAILED") << std::endl;                  \
pass = pass && match;                                                 \
}                                                                     \

#define vtkTestSetGetMacroIndex2(pass, obj, var, val, index1, index2) \
{                                                                     \
std::cerr << "Testing Set/Get " #var "...";                           \
obj->Set##var (index1, index2, val);                                  \
bool match = val == obj->Get##var(index1, index2);                    \
std::cerr << (match ? "OK" : "FAILED") << std::endl;                  \
pass = pass && match;                                                 \
}                                                                     \

#define vtkTestSetGetMacroIndex3(pass, obj, var, val, i1, i2, i3)     \
{                                                                     \
std::cerr << "Testing Set/Get " #var "...";                           \
obj->Set##var (i1, i2, i3, val);                                      \
bool match = val == obj->Get##var(i1, i2, i3);                        \
std::cerr << (match ? "OK" : "FAILED") << std::endl;                  \
pass = pass && match;                                                 \
}                                                                     \


#define vtkTestSetGetStringMacroIndex(pass, obj, var, val, index)       \
{                                                                       \
std::cerr << "Testing Set/Get " #var "...";                             \
obj->Set##var (index, val);                                             \
bool match = std::string(val) == std::string(obj->Get##var(index));     \
std::cerr << (match ? "OK" : "FAILED") << std::endl;                    \
pass = pass && match;                                                   \
}                                                                       \

#define vtkTestSetGetPoint3DMacroIndex(pass, obj, var, arrayType, val, index)      \
{                                                                         \
std::cerr << "Testing Set/Get " #var "...";                               \
obj->Set##var (index, val);                                               \
arrayType outval[3];                                                      \
obj->Get##var (index, outval);                                            \
bool match = val[0]==outval[0] && val[1]==outval[1] && val[2]==outval[2]; \
std::cerr << (match ? "OK" : "FAILED") << std::endl;                    \
pass = pass && match;                                                   \
}                                                                       \

int main(int vtkNotUsed(argc), char** argv)
{
  std::cerr << "Starting EM mrml manager test..." << std::endl;

  std::string mrmlSceneFilename           = argv[1];
  std::string parametersNodeName          = argv[2];

  // generate some magic numbers to use for testing
  const int    MAGIC_INT     = rand();
  const double MAGIC_DOUBLE  = 3.14159 * rand();
  const double MAGIC_DOUBLE2 = 3.14159 * rand();
  const double MAGIC_DOUBLE3 = 3.14159 * rand();
  const double MAGIC_DOUBLE4 = 3.14159 * rand();
  const double MAGIC_DOUBLE5 = 3.14159 * rand();
  const std::string MAGIC_STRING("OU812,10SNE1");

#ifdef _WIN32
  //
  // strip backslashes from parameter node name (present if spaces were used)
  std::string tmpNodeName = parametersNodeName;
  parametersNodeName.clear();
  for (unsigned int i = 0; i < tmpNodeName.size(); ++i)
    {
      if (tmpNodeName[i] != '\\')
        {
        parametersNodeName.push_back(tmpNodeName[i]);
        }
      else if (i > 0 && tmpNodeName[i-1] == '\\')
        {
        parametersNodeName.push_back(tmpNodeName[i]);
        }
    }
#endif

  bool pass = true;
  int returnValue = EXIT_SUCCESS;

  //
  // create a mrml scene that will hold the data parameters
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetURL(mrmlSceneFilename.c_str());

  //
  // create an instance of vtkEMSegmentLogic and connect it with the
  // MRML scene
  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetModuleName("EMSegment");
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->SetMRMLScene(mrmlScene);
  emLogic->InitializeEventListeners();

  emLogic->RegisterMRMLNodesWithScene();

  try 
    {
    try 
      {
      mrmlScene->Import();
      std::cerr << "Imported: " << mrmlScene->GetNumberOfNodes()
                << " nodes." << std::endl;
      }
    catch (...)
      {
      std::cerr << "Error reading/setting mrml scene: " << std::endl;
      throw;
      }

    //
    // populate the logic class with testing data
    int numParameterSets = emLogic->GetMRMLManager()->
      GetNumberOfParameterSets();
    std::cerr << "Found " << numParameterSets << " EM top level nodes."
              << std::endl;
    bool foundParameters = false;
    std::cerr << "Searching for an EM parameter node named: " 
              << parametersNodeName << std::endl;

    for (int i = 0; i < numParameterSets; ++i)
      {
      std::string 
        currentNodeName(emLogic->GetMRMLManager()->GetNthParameterSetName(i)); 
      std::cerr << "Node " << i << " name: " << currentNodeName << std::endl;
      if (parametersNodeName == currentNodeName)
        {
        try
          {
          emLogic->GetMRMLManager()->SetLoadedParameterSetIndex(i);
          }
        catch (...)
          {
          std::cerr << "Error setting parameter set: " << std::endl;
          throw;
          }
        foundParameters = true;
        break;
        }
      else
        {
        std::cerr << "Found non-matching EM parameters node: " 
                  << currentNodeName << std::endl;
        }
      }

    if (!foundParameters)
      {
      std::cerr << "Error: parameters not found in scene" << std::endl;
      throw std::runtime_error("Parameters Not Found!");
      }

    vtkEMSegmentMRMLManager* m = emLogic->GetMRMLManager();
    vtkIdType rootID = m->GetTreeRootNodeID();
    
    mrmlScene->InitTraversal();
    vtkMRMLNode* nodeToPrint;
    while ((nodeToPrint = mrmlScene->GetNextNode()) && nodeToPrint)
      {
      std::cerr << "Node ID/Name: " << nodeToPrint->GetID() 
                << "/" << nodeToPrint->GetName() << std::endl;
      }
    m->PrintTree(rootID, static_cast<vtkIndent>(2));

    //////////////////////////////////////////////////////////////////////
    // test parameter modification (we'll test structure changes later)
    //////////////////////////////////////////////////////////////////////

    // tree node
    vtkIdType treeLeafNodeID = m->GetTreeNodeChildNodeID(rootID, 0);
    while  ((treeLeafNodeID != ERROR_NODE_VTKID) && (!m->GetTreeNodeIsLeaf(treeLeafNodeID) ))  {
         treeLeafNodeID = m->GetTreeNodeChildNodeID(treeLeafNodeID, 0);
    }

    int index = 0 ;
    vtkIdType treeParentNodeID = m->GetTreeNodeChildNodeID(rootID, index);
    while  ((treeParentNodeID != ERROR_NODE_VTKID) && (m->GetTreeNodeIsLeaf(treeParentNodeID) ))  {
      index++;
         treeParentNodeID = m->GetTreeNodeChildNodeID(rootID, index);
    }


   
    if ((treeLeafNodeID == ERROR_NODE_VTKID)  ||  (treeParentNodeID == ERROR_NODE_VTKID ))
      {
        std::cerr << "Error: No leaf or parent node found !" << std::endl;
        throw std::runtime_error("No leaf or parent node was found");
      }


    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeIntensityLabel,
                            MAGIC_INT, treeLeafNodeID);

    vtkTestSetGetStringMacroIndex(pass, m,
                                  TreeNodeName, MAGIC_STRING.c_str(), treeLeafNodeID);
    double color[3] = { MAGIC_DOUBLE, MAGIC_DOUBLE2, MAGIC_DOUBLE3 };
    vtkTestSetGetPoint3DMacroIndex(pass, m,
                                   TreeNodeColor, double, color, treeLeafNodeID);
    
    // intensity distribution
    vtkTestSetGetMacroIndex(pass, m, 
                            TreeNodeDistributionSpecificationMethod, 
                            vtkEMSegmentMRMLManager::
                            DistributionSpecificationManual, 
                            treeLeafNodeID);
    //vtkTestSetGetMacroIndex2(pass, m, 
    //                         TreeNodeDistributionLogMeanWithCorrection, 
    //                         MAGIC_DOUBLE, treeLeafNodeID, 0);
    //vtkTestSetGetMacroIndex3(pass, m, 
    //                         TreeNodeDistributionLogCovarianceWithCorrection, 
    //                         MAGIC_DOUBLE, treeLeafNodeID, 0, 0);


    std::cerr << "Testing sample point interface...";
    double p1[3] = {MAGIC_DOUBLE, MAGIC_DOUBLE2, MAGIC_DOUBLE3};
    double p2[3] = {MAGIC_DOUBLE2, MAGIC_DOUBLE3, MAGIC_DOUBLE};
    double p3[3] = {MAGIC_DOUBLE3, MAGIC_DOUBLE, MAGIC_DOUBLE2};
    m->RemoveAllTreeNodeDistributionSamplePoints(treeLeafNodeID);
    m->AddTreeNodeDistributionSamplePoint(treeLeafNodeID, p1);
    m->AddTreeNodeDistributionSamplePoint(treeLeafNodeID, p2);
    m->AddTreeNodeDistributionSamplePoint(treeLeafNodeID, p3);
    m->RemoveTreeNodeDistributionSamplePoint(treeLeafNodeID, 1);
    bool localPass = true;
    if (m->GetTreeNodeDistributionNumberOfSamples(treeLeafNodeID) != 2)
      {
      std::cerr << "Error: wrong number of sample points!" << std::endl;
      pass = false;
      localPass = false;
      }
    m->GetTreeNodeDistributionSamplePoint(treeLeafNodeID, 1, p2);
    if (! (p2[0]==p3[0] && p2[1]==p3[1] && p2[2]==p3[2]))
      {
      std::cerr << "Error: wrong order of sample points!" << std::endl;
      pass = false;
      localPass = false;
      }
    m->RemoveAllTreeNodeDistributionSamplePoints(treeLeafNodeID);
    if (m->GetTreeNodeDistributionNumberOfSamples(treeLeafNodeID) != 0)
      {
      std::cerr << "Error: wrong number of sample points!" << std::endl;
      pass = false;
      localPass = false;
      }
    std::cerr << (localPass ? "OK" : "FAILED") << std::endl;

    // node-specific parameters
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintWeight,
                            MAGIC_INT, treeLeafNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintQuality,
                            MAGIC_INT, treeLeafNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintFrequency,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintLabelMap,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintEMLabelMapConvergence,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintEMWeightsConvergence,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintMFALabelMapConvergence,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintMFAWeightsConvergence,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeGenerateBackgroundProbability,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeExcludeFromIncompleteEStep,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeAlpha,
                            MAGIC_DOUBLE, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintBias,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeBiasCalculationMaxIterations,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeSmoothingKernelWidth,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeSmoothingKernelSigma,
                            MAGIC_DOUBLE, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeClassProbability,
                            MAGIC_DOUBLE, treeLeafNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeSpatialPriorWeight,
                            MAGIC_DOUBLE, treeLeafNodeID);
    vtkTestSetGetMacroIndex2(pass, m,
                             TreeNodeInputChannelWeight,
                             MAGIC_DOUBLE, treeLeafNodeID, 1);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionEMType,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionEMValue,
                            MAGIC_DOUBLE, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionEMIterations,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionMFAType,
                            MAGIC_INT, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionMFAValue,
                            MAGIC_DOUBLE, treeParentNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionMFAIterations,
                            MAGIC_INT, treeParentNodeID);
     
      // registration parameters
      vtkTestSetGetMacro(pass, m,
                         RegistrationAffineType, MAGIC_INT);
      vtkTestSetGetMacro(pass, m,
                         RegistrationDeformableType, MAGIC_INT);
      vtkTestSetGetMacro(pass, m,
                         RegistrationInterpolationType, MAGIC_INT);
      vtkIdType atlasRegID  = m->GetVolumeNthID(1);
      vtkTestSetGetMacro(pass, m,
                         RegistrationAtlasVolumeID, atlasRegID);

      // save parameters
      // - you need a  vtkSlicerColorLogic to test ColorNodeID - which requires the corresponding .h file , which is not part of the build 
    // vtkSlicerColorLogic* colLogic = vtkSlicerColorLogic::New();
        //this->MRMLScene->AddNode(colLogic);
          //colLogic->Delete();
          //vtkTestSetGetStringMacro(pass, m, 
          //                  ColorNodeID, MAGIC_STRING.c_str());

      vtkTestSetGetStringMacro(pass, m, 
                               SaveWorkingDirectory, MAGIC_STRING.c_str());
      vtkTestSetGetStringMacro(pass, m,    
                               SaveTemplateFilename, MAGIC_STRING.c_str());
      vtkTestSetGetMacro(pass, m, 
                         SaveTemplateAfterSegmentation, MAGIC_INT);
      vtkTestSetGetMacro(pass, m, 
                         SaveIntermediateResults, MAGIC_INT);
      vtkTestSetGetMacro(pass, m, 
                         SaveSurfaceModels, MAGIC_INT);

      // miscellaneous
      vtkTestSetGetMacro(pass, m, 
                         EnableMultithreading, MAGIC_INT);      
      int bound[3] = { 5, 10, 20 };
      vtkTestSetGetPoint3DMacro(pass, m, 
                                SegmentationBoundaryMin, int, bound);      
      vtkTestSetGetPoint3DMacro(pass, m, 
                                SegmentationBoundaryMax, int, bound);      


      /////////////////////////////////////////////////////////////////
      // manipulate tree structure
      /////////////////////////////////////////////////////////////////
      localPass = true;
      std::cerr << "Adding/Removing/Moving tree nodes...";

      // add node A under root
      int numChildren = m->GetTreeNodeNumberOfChildren(rootID);
      
      std::cerr << "Adding A...";

      vtkIdType idA = m->AddTreeNode(rootID);
      m->SetTreeNodeIntensityLabel(idA, int('A'));
      
      if (m->GetTreeNodeNumberOfChildren(rootID) != numChildren+1 ||
          !m->GetTreeNodeIsLeaf(idA))
        {
        std::cerr << "Error adding child node" << std::endl;
        pass = false;
        localPass = false;
        }

      // add node B and C under A
      std::cerr << "Adding B&C under A...";
      vtkIdType idB = m->AddTreeNode(idA);
      m->SetTreeNodeIntensityLabel(idB, int('B'));
      vtkIdType idC = m->AddTreeNode(idA);
      m->SetTreeNodeIntensityLabel(idC, int('C'));

      // add node D under B
      std::cerr << "Adding D under B...";
      vtkIdType idD = m->AddTreeNode(idB);
      m->SetTreeNodeIntensityLabel(idD, int('D'));

      if (m->GetTreeNodeIsLeaf(idA) ||
          m->GetTreeNodeIsLeaf(idB) || 
          !m->GetTreeNodeIsLeaf(idC) ||
          !m->GetTreeNodeIsLeaf(idD))
        {
        std::cerr << "Error adding child nodes" << std::endl;
        pass = false;
        localPass = false;
        }
      
      // move node D to node C
      std::cerr << "Moving D from under B to under  C...";
      m->SetTreeNodeParentNodeID(idD, idC);
      if (m->GetTreeNodeIsLeaf(idA) ||
          !m->GetTreeNodeIsLeaf(idB) || 
          m->GetTreeNodeIsLeaf(idC) ||
          !m->GetTreeNodeIsLeaf(idD))
        {
        std::cerr << "Error moving child nodes" << std::endl;
        pass = false;
        localPass = false;
        }

      // remove node B
      std::cerr << "Removing B...";
      m->RemoveTreeNode(idB);
      if (m->GetTreeNodeNumberOfChildren(idA) != 1 ||
          m->GetTreeNodeIsLeaf(idC) ||
          !m->GetTreeNodeIsLeaf(idD))
        {
        std::cerr << "Error removing tree node" << std::endl;
        pass = false;
        localPass = false;
        }

      if ( m->GetTreeNodeIntensityLabel(idD) != int('D'))
        {
        std::cerr << "Error manipulating tree nodes" << std::endl;
        pass = false;
        localPass = false;
        }
      
      // remove node A
      std::cerr << "Removing A...";
      m->RemoveTreeNode(idA);
      if (numChildren != m->GetTreeNodeNumberOfChildren(rootID) ||
          m->TreeNodeExists(idA)  || m->TreeNodeExists(idB)  || m->TreeNodeExists(idC) || m->TreeNodeExists(idD) )
        {
        std::cerr << "Error removing tree node" << std::endl;
        pass = false;
        localPass = false;
        }
      std::cerr << (localPass ? "OK" : "FAILED") << std::endl;

       /////////////////////////////////////////////////////////////////
      // manipulate atlas
      /////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
      // manipulate target
      /////////////////////////////////////////////////////////////////
      std::cerr << "Adding/Removing/Moving target images...";
      localPass = true;
      // remove all targets
      std::cerr << "Removing all targets...";
      while (m->GetTargetNumberOfSelectedVolumes() > 0)
        {
        std::cerr << m->GetTargetNumberOfSelectedVolumes() << ",";
        m->RemoveTargetSelectedVolume(m->GetTargetSelectedVolumeNthID(0));
        }
      if (m->GetTargetNumberOfSelectedVolumes() != 0)
        {
        std::cerr << "Error removing target input channels." << std::endl;
        pass = false;
        localPass = false;
        }

      // add back some targets
      std::cerr << "Adding back some targets...";
      m->AddTargetSelectedVolume(m->GetVolumeNthID(0));
      m->AddTargetSelectedVolume(m->GetVolumeNthID(1));
      m->AddTargetSelectedVolume(m->GetVolumeNthID(2));
      m->AddTargetSelectedVolume(m->GetVolumeNthID(3));
      m->AddTargetSelectedVolume(m->GetVolumeNthID(4));
      if (m->GetTargetNumberOfSelectedVolumes() != 5)
        {
        std::cerr << "Error adding target input channels." << std::endl;
        pass = false;
        localPass = false;
        }

      // set some parameters that we can check later
      std::cerr << "Set some log mean parameters...";
      m->SetTreeNodeDistributionLogMean(treeLeafNodeID, 0, MAGIC_DOUBLE);
      m->SetTreeNodeDistributionLogMean(treeLeafNodeID, 1, MAGIC_DOUBLE2);
      m->SetTreeNodeDistributionLogMean(treeLeafNodeID, 2, MAGIC_DOUBLE3);
      m->SetTreeNodeDistributionLogMean(treeLeafNodeID, 3, MAGIC_DOUBLE4);
      m->SetTreeNodeDistributionLogMean(treeLeafNodeID, 4, MAGIC_DOUBLE5);

      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 0, 0, 1);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 0, 1, 2);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 0, 2, 3);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 0, 3, 4);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 0, 4, 5);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 1, 0, 6);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 1, 1, 7);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 1, 2, 8);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 1, 3, 9);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 1, 4, 20);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 2, 0, 11);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 2, 1, 12);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 2, 2, 13);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 2, 3, 14);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 2, 4, 15);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 3, 0, 16);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 3, 1, 17);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 3, 2, 18);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 3, 3, 19);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 3, 4, 20);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 4, 0, 21);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 4, 1, 22);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 4, 2, 23);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 4, 3, 24);
      m->SetTreeNodeDistributionLogCovariance(treeLeafNodeID, 4, 4, 25);

      m->SetTreeNodeInputChannelWeight(treeLeafNodeID, 0, MAGIC_DOUBLE);
      m->SetTreeNodeInputChannelWeight(treeLeafNodeID, 1, MAGIC_DOUBLE2);
      m->SetTreeNodeInputChannelWeight(treeLeafNodeID, 2, MAGIC_DOUBLE3);
      m->SetTreeNodeInputChannelWeight(treeLeafNodeID, 3, MAGIC_DOUBLE4);
      m->SetTreeNodeInputChannelWeight(treeLeafNodeID, 4, MAGIC_DOUBLE5);


      // remove a target
      std::cerr << "Removing a target...";
      m->RemoveTargetSelectedVolume(m->GetVolumeNthID(2));
      if (m->GetTargetNumberOfSelectedVolumes() != 4)
        {
        std::cerr << "Error removing 1 target input channel." << std::endl;
        pass = false;
        localPass = false;
        }

      // move some targets
      std::cerr << "Moving some targetsx...";
      m->MoveTargetSelectedVolume(m->GetVolumeNthID(1), 2);
      m->MoveTargetSelectedVolume(m->GetVolumeNthID(4), 0);
      if (m->GetTargetNumberOfSelectedVolumes() != 4)
        {
        std::cerr << "Error moving target input channels." << std::endl;
        pass = false;
        localPass = false;
        }

      // check that all is ok
      // we should have changed 0, 1, 2, 3, 4 to 4, 0, 3, 1
      std::cerr << "Checking that parameters moved ok...";
      if (m->GetTreeNodeDistributionLogMeanWithCorrection(treeLeafNodeID, 0) != MAGIC_DOUBLE5 ||
          m->GetTreeNodeDistributionLogMeanWithCorrection(treeLeafNodeID, 1) != MAGIC_DOUBLE ||
          m->GetTreeNodeDistributionLogMeanWithCorrection(treeLeafNodeID, 2) != MAGIC_DOUBLE4 ||
          m->GetTreeNodeDistributionLogMeanWithCorrection(treeLeafNodeID, 3) != MAGIC_DOUBLE2)
        {
        std::cerr << "Error moving log mean" << std::endl;
        std::cerr << "M1 " << MAGIC_DOUBLE << std::endl;
        std::cerr << "M2 " << MAGIC_DOUBLE2 << std::endl;
        std::cerr << "M3 " << MAGIC_DOUBLE3 << std::endl;
        std::cerr << "M4 " << MAGIC_DOUBLE4 << std::endl;
        std::cerr << "M5 " << MAGIC_DOUBLE5 << std::endl;

        std::cerr << "Expected order: M5 M1 M4 M2" << std::endl;

        std::cerr << "Found order: " 
                  << m->GetTreeNodeDistributionLogMeanWithCorrection(treeLeafNodeID, 0) << " "
                  << m->GetTreeNodeDistributionLogMeanWithCorrection(treeLeafNodeID, 1) << " "
                  << m->GetTreeNodeDistributionLogMeanWithCorrection(treeLeafNodeID, 2) << " "
                  << m->GetTreeNodeDistributionLogMeanWithCorrection(treeLeafNodeID, 3) 
                  << std::endl;
        pass = false;
        localPass = false;
        }

      if (
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 0, 0) != 25 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 0, 1) != 21 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 0, 2) != 24 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 0, 3) != 22 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 1, 0) != 5 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 1, 1) != 1 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 1, 2) != 4 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 1, 3) != 2 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 2, 0) != 20 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 2, 1) != 16 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 2, 2) != 19 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 2, 3) != 17 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 3, 0) != 20 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 3, 1) != 6 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 3, 2) != 9 ||
          m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, 3, 3) != 7 
          )
        {
        std::cerr << "Error moving log covariance" << std::endl;

        for (int r = 0; r < 4; ++r)
          {
          for (int c = 0; c < 4; ++c)
            {
            std::cerr << m->GetTreeNodeDistributionLogCovarianceWithCorrection(treeLeafNodeID, r, c)
                      << " ";
            }
          std::cerr << std::endl;
          }
        pass = false;
        localPass = false;
        }

      if (m->GetTreeNodeInputChannelWeight(treeLeafNodeID, 0) != MAGIC_DOUBLE5 ||
          m->GetTreeNodeInputChannelWeight(treeLeafNodeID, 1) != MAGIC_DOUBLE ||
          m->GetTreeNodeInputChannelWeight(treeLeafNodeID, 2) != MAGIC_DOUBLE4 ||
          m->GetTreeNodeInputChannelWeight(treeLeafNodeID, 3) != MAGIC_DOUBLE2)
        {
        std::cerr << "Error moving input channel weight" << std::endl;
        pass = false;
        localPass = false;
        }
      
      std::cerr << (localPass ? "OK" : "FAILED") << std::endl;
    }
  catch(...)
    {
    returnValue = EXIT_FAILURE;
    }

  // clean up
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();

 
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  
  return (pass ? returnValue : EXIT_FAILURE);
}

