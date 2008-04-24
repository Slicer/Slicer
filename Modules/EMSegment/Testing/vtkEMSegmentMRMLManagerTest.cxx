#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSTargetNode.h"
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

int main(int argc, char** argv)
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

#ifdef WIN32
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
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();
  vtkIntArray *emsEvents                 = vtkIntArray::New();
  emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
  emsEvents->Delete();

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

    vtkIdType treeNodeID = m->GetTreeNodeChildNodeID(rootID, 0);

    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeIntensityLabel,
                            MAGIC_INT, treeNodeID);

    vtkTestSetGetStringMacroIndex(pass, m,
                                  TreeNodeLabel, MAGIC_STRING.c_str(), treeNodeID);
    vtkTestSetGetStringMacroIndex(pass, m,
                                  TreeNodeName, MAGIC_STRING.c_str(), treeNodeID);
    double color[3] = { MAGIC_DOUBLE, MAGIC_DOUBLE2, MAGIC_DOUBLE3 };
    vtkTestSetGetPoint3DMacroIndex(pass, m,
                                   TreeNodeColor, double, color, treeNodeID);
    
    // intensity distribution
    vtkTestSetGetMacroIndex(pass, m, 
                            TreeNodeDistributionSpecificationMethod, 
                            vtkEMSegmentMRMLManager::
                            DistributionSpecificationManual, 
                            treeNodeID);
    vtkTestSetGetMacroIndex2(pass, m, 
                             TreeNodeDistributionLogMean, 
                             MAGIC_DOUBLE, treeNodeID, 0);
    vtkTestSetGetMacroIndex3(pass, m, 
                             TreeNodeDistributionLogCovariance, 
                             MAGIC_DOUBLE, treeNodeID, 0, 0);

    std::cerr << "Testing sample point interface...";
    double p1[3] = {MAGIC_DOUBLE, MAGIC_DOUBLE2, MAGIC_DOUBLE3};
    double p2[3] = {MAGIC_DOUBLE2, MAGIC_DOUBLE3, MAGIC_DOUBLE};
    double p3[3] = {MAGIC_DOUBLE3, MAGIC_DOUBLE, MAGIC_DOUBLE2};
    m->RemoveAllTreeNodeDistributionSamplePoints(treeNodeID);
    m->AddTreeNodeDistributionSamplePoint(treeNodeID, p1);
    m->AddTreeNodeDistributionSamplePoint(treeNodeID, p2);
    m->AddTreeNodeDistributionSamplePoint(treeNodeID, p3);
    m->RemoveTreeNodeDistributionSamplePoint(treeNodeID, 1);
    bool localPass = true;
    if (m->GetTreeNodeDistributionNumberOfSamples(treeNodeID) != 2)
      {
      std::cerr << "Error: wrong number of sample points!" << std::endl;
      pass = false;
      localPass = false;
      }
    m->GetTreeNodeDistributionSamplePoint(treeNodeID, 1, p2);
    if (! (p2[0]==p3[0] && p2[1]==p3[1] && p2[2]==p3[2]))
      {
      std::cerr << "Error: wrong order of sample points!" << std::endl;
      pass = false;
      localPass = false;
      }
    m->RemoveAllTreeNodeDistributionSamplePoints(treeNodeID);
    if (m->GetTreeNodeDistributionNumberOfSamples(treeNodeID) != 0)
      {
      std::cerr << "Error: wrong number of sample points!" << std::endl;
      pass = false;
      localPass = false;
      }
    std::cerr << (localPass ? "OK" : "FAILED") << std::endl;

    // node-specific parameters
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintWeight,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintQuality,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintFrequency,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintLabelMap,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintEMLabelMapConvergence,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintEMWeightsConvergence,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintMFALabelMapConvergence,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintMFAWeightsConvergence,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeGenerateBackgroundProbability,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeExcludeFromIncompleteEStep,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeAlpha,
                            MAGIC_DOUBLE, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodePrintBias,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeBiasCalculationMaxIterations,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeSmoothingKernelWidth,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeSmoothingKernelSigma,
                            MAGIC_DOUBLE, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeClassProbability,
                            MAGIC_DOUBLE, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeSpatialPriorWeight,
                            MAGIC_DOUBLE, treeNodeID);
    vtkTestSetGetMacroIndex2(pass, m,
                             TreeNodeInputChannelWeight,
                             MAGIC_DOUBLE, treeNodeID, 1);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionEMType,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionEMValue,
                            MAGIC_DOUBLE, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionEMIterations,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionMFAType,
                            MAGIC_INT, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionMFAValue,
                            MAGIC_DOUBLE, treeNodeID);
    vtkTestSetGetMacroIndex(pass, m,
                            TreeNodeStoppingConditionMFAIterations,
                            MAGIC_INT, treeNodeID);
    
    // normalization parameters
    int numTargets = m->GetTargetNumberOfSelectedVolumes();
    vtkIdType targetID = m->GetTargetSelectedVolumeNthID(numTargets-1);
    
    vtkTestSetGetMacroIndex(pass, m, 
                            NthTargetVolumeIntensityNormalizationNormValue, 
                            MAGIC_DOUBLE, numTargets-1);
    vtkTestSetGetMacroIndex(pass, m, 
                            TargetVolumeIntensityNormalizationNormValue,
                            MAGIC_DOUBLE, targetID);
      
      vtkTestSetGetMacroIndex(pass, m,
                              NthTargetVolumeIntensityNormalizationNormType,
                              MAGIC_INT, numTargets-1);
      vtkTestSetGetMacroIndex(pass, m,
                              TargetVolumeIntensityNormalizationNormType,
                              MAGIC_INT, targetID);

      vtkTestSetGetMacroIndex(pass, m,
                              NthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth,
                              MAGIC_INT, numTargets-1);
      vtkTestSetGetMacroIndex(pass, m,
                              TargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth,
                              MAGIC_INT, targetID);

      vtkTestSetGetMacroIndex(pass, m,
                              NthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth,
                              MAGIC_INT, numTargets-1);
      vtkTestSetGetMacroIndex(pass, m,
                              TargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth,
                              MAGIC_INT, targetID);

      vtkTestSetGetMacroIndex(pass, m,
                              NthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum,
                              MAGIC_INT, numTargets-1);
      vtkTestSetGetMacroIndex(pass, m,
                              TargetVolumeIntensityNormalizationRelativeMaxVoxelNum,
                              MAGIC_INT, targetID);

      vtkTestSetGetMacroIndex(pass, m,
                              NthTargetVolumeIntensityNormalizationPrintInfo,
                              MAGIC_INT, numTargets-1);
      vtkTestSetGetMacroIndex(pass, m,
                              TargetVolumeIntensityNormalizationPrintInfo,
                              MAGIC_INT, targetID);

      vtkTestSetGetMacroIndex(pass, m,
                              NthTargetVolumeIntensityNormalizationEnabled,
                              MAGIC_INT, numTargets-1);
      vtkTestSetGetMacroIndex(pass, m,
                              TargetVolumeIntensityNormalizationEnabled,
                              MAGIC_INT, targetID);

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
      std::cerr << "Adding B&C...";
      vtkIdType idB = m->AddTreeNode(idA);
      m->SetTreeNodeIntensityLabel(idB, int('B'));
      vtkIdType idC = m->AddTreeNode(idA);
      m->SetTreeNodeIntensityLabel(idC, int('C'));

      // add node D under B
      std::cerr << "Adding D...";
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
      
      // check CIM sizes
      if (m->GetTreeClassInteractionNode(rootID)->GetNumberOfClasses() != 3 ||
          m->GetTreeClassInteractionNode(idA)->GetNumberOfClasses() != 2 ||
          m->GetTreeClassInteractionNode(idB)->GetNumberOfClasses() != 1 ||
          m->GetTreeClassInteractionNode(idC)->GetNumberOfClasses() != 0 ||
          m->GetTreeClassInteractionNode(idD)->GetNumberOfClasses() != 0)
        {
        std::cerr << "Error adding child nodes: CIM size error" << std::endl;

        std::cerr 
          << "Root:" 
          << m->GetTreeClassInteractionNode(rootID)->GetNumberOfClasses()
          << " A:" 
          << m->GetTreeClassInteractionNode(idA)->GetNumberOfClasses()
          << " B:" 
          << m->GetTreeClassInteractionNode(idB)->GetNumberOfClasses()
          << " C:" 
          << m->GetTreeClassInteractionNode(idC)->GetNumberOfClasses()
          << " D:" 
          << m->GetTreeClassInteractionNode(idD)->GetNumberOfClasses()
          << std::endl;

        pass = false;
        localPass = false;
        }
      
      // check CIM entries
      for (unsigned int direction = 0; direction < 6; ++direction)
        {
        if (m->GetTreeNodeClassInteraction(idA, direction, 0, 0) != 1 ||
            m->GetTreeNodeClassInteraction(idA, direction, 1, 1) != 1 ||
            m->GetTreeNodeClassInteraction(idA, direction, 0, 1) != 0 ||
            m->GetTreeNodeClassInteraction(idA, direction, 1, 0) != 0 ||
            m->GetTreeNodeClassInteraction(idB, direction, 0, 0) != 1)
          {
          std::cerr << "Error adding child nodes: CIM size error" << std::endl;
          pass = false;
          localPass = false;
          }
        }

      // move node D to node C
      std::cerr << "Moving D to C...";
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

      if (m->GetTreeNodeIntensityLabel(idA) != int('A') ||
          m->GetTreeNodeIntensityLabel(idC) != int('C') ||
          m->GetTreeNodeIntensityLabel(idD) != int('D'))
        {
        std::cerr << "Error manipulating tree nodes" << std::endl;
        pass = false;
        localPass = false;
        }
      
      // check CIM sizes
      if (m->GetTreeClassInteractionNode(rootID)->GetNumberOfClasses() != 3 ||
          m->GetTreeClassInteractionNode(idA)->GetNumberOfClasses() != 1 ||
          m->GetTreeClassInteractionNode(idC)->GetNumberOfClasses() != 1 ||
          m->GetTreeClassInteractionNode(idD)->GetNumberOfClasses() != 0)
        {
        std::cerr << "Error moving child nodes: CIM size error" << std::endl;

        std::cerr 
          << "Root:" 
          << m->GetTreeClassInteractionNode(rootID)->GetNumberOfClasses()
          << " A:" 
          << m->GetTreeClassInteractionNode(idA)->GetNumberOfClasses()
          << " C:" 
          << m->GetTreeClassInteractionNode(idC)->GetNumberOfClasses()
          << " D:" 
          << m->GetTreeClassInteractionNode(idD)->GetNumberOfClasses()
          << std::endl;

        pass = false;
        localPass = false;
        }
      
      // check CIM entries
      for (unsigned int direction = 0; direction < 6; ++direction)
        {
        if (m->GetTreeNodeClassInteraction(rootID, direction, 0, 0) != 1 ||
            m->GetTreeNodeClassInteraction(idC, direction, 0, 0) != 1 ||
            m->GetTreeNodeClassInteraction(idA, direction, 0, 0) != 1)
          {
          std::cerr << "Error moving child nodes: CIM size error" << std::endl;
          pass = false;
          localPass = false;
          }
        }

      // remove node A
      std::cerr << "Removing A...";
      m->RemoveTreeNode(idA);
      if (numChildren != m->GetTreeNodeNumberOfChildren(rootID) ||
          m->GetTreeNode(idA) != NULL ||
          m->GetTreeNode(idB) != NULL ||
          m->GetTreeNode(idC) != NULL ||
          m->GetTreeNode(idD) != NULL)
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
      m->SetTreeNodeDistributionLogMean(treeNodeID, 0, MAGIC_DOUBLE);
      m->SetTreeNodeDistributionLogMean(treeNodeID, 1, MAGIC_DOUBLE2);
      m->SetTreeNodeDistributionLogMean(treeNodeID, 2, MAGIC_DOUBLE3);
      m->SetTreeNodeDistributionLogMean(treeNodeID, 3, MAGIC_DOUBLE4);
      m->SetTreeNodeDistributionLogMean(treeNodeID, 4, MAGIC_DOUBLE5);

      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 0, 0, 1);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 0, 1, 2);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 0, 2, 3);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 0, 3, 4);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 0, 4, 5);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 1, 0, 6);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 1, 1, 7);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 1, 2, 8);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 1, 3, 9);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 1, 4, 20);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 2, 0, 11);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 2, 1, 12);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 2, 2, 13);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 2, 3, 14);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 2, 4, 15);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 3, 0, 16);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 3, 1, 17);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 3, 2, 18);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 3, 3, 19);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 3, 4, 20);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 4, 0, 21);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 4, 1, 22);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 4, 2, 23);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 4, 3, 24);
      m->SetTreeNodeDistributionLogCovariance(treeNodeID, 4, 4, 25);

      m->SetTreeNodeInputChannelWeight(treeNodeID, 0, MAGIC_DOUBLE);
      m->SetTreeNodeInputChannelWeight(treeNodeID, 1, MAGIC_DOUBLE2);
      m->SetTreeNodeInputChannelWeight(treeNodeID, 2, MAGIC_DOUBLE3);
      m->SetTreeNodeInputChannelWeight(treeNodeID, 3, MAGIC_DOUBLE4);
      m->SetTreeNodeInputChannelWeight(treeNodeID, 4, MAGIC_DOUBLE5);

      m->SetNthTargetVolumeIntensityNormalizationNormValue(0, MAGIC_DOUBLE);
      m->SetNthTargetVolumeIntensityNormalizationNormValue(1, MAGIC_DOUBLE2);
      m->SetNthTargetVolumeIntensityNormalizationNormValue(2, MAGIC_DOUBLE3);
      m->SetNthTargetVolumeIntensityNormalizationNormValue(3, MAGIC_DOUBLE4);
      m->SetNthTargetVolumeIntensityNormalizationNormValue(4, MAGIC_DOUBLE5);

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
      if (m->GetTreeNodeDistributionLogMean(treeNodeID, 0) != MAGIC_DOUBLE5 ||
          m->GetTreeNodeDistributionLogMean(treeNodeID, 1) != MAGIC_DOUBLE ||
          m->GetTreeNodeDistributionLogMean(treeNodeID, 2) != MAGIC_DOUBLE4 ||
          m->GetTreeNodeDistributionLogMean(treeNodeID, 3) != MAGIC_DOUBLE2)
        {
        std::cerr << "Error moving log mean" << std::endl;
        std::cerr << "M1 " << MAGIC_DOUBLE << std::endl;
        std::cerr << "M2 " << MAGIC_DOUBLE2 << std::endl;
        std::cerr << "M3 " << MAGIC_DOUBLE3 << std::endl;
        std::cerr << "M4 " << MAGIC_DOUBLE4 << std::endl;
        std::cerr << "M5 " << MAGIC_DOUBLE5 << std::endl;

        std::cerr << "Expected order: M5 M1 M4 M2" << std::endl;

        std::cerr << "Found order: " 
                  << m->GetTreeNodeDistributionLogMean(treeNodeID, 0) << " "
                  << m->GetTreeNodeDistributionLogMean(treeNodeID, 1) << " "
                  << m->GetTreeNodeDistributionLogMean(treeNodeID, 2) << " "
                  << m->GetTreeNodeDistributionLogMean(treeNodeID, 3) 
                  << std::endl;
        pass = false;
        localPass = false;
        }

      if (
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 0, 0) != 25 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 0, 1) != 21 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 0, 2) != 24 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 0, 3) != 22 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 1, 0) != 5 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 1, 1) != 1 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 1, 2) != 4 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 1, 3) != 2 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 2, 0) != 20 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 2, 1) != 16 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 2, 2) != 19 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 2, 3) != 17 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 3, 0) != 20 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 3, 1) != 6 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 3, 2) != 9 ||
          m->GetTreeNodeDistributionLogCovariance(treeNodeID, 3, 3) != 7 
          )
        {
        std::cerr << "Error moving log covariance" << std::endl;

        for (int r = 0; r < 4; ++r)
          {
          for (int c = 0; c < 4; ++c)
            {
            std::cerr << m->GetTreeNodeDistributionLogCovariance(treeNodeID, r, c)
                      << " ";
            }
          std::cerr << std::endl;
          }
        pass = false;
        localPass = false;
        }

      if (m->GetTreeNodeInputChannelWeight(treeNodeID, 0) != MAGIC_DOUBLE5 ||
          m->GetTreeNodeInputChannelWeight(treeNodeID, 1) != MAGIC_DOUBLE ||
          m->GetTreeNodeInputChannelWeight(treeNodeID, 2) != MAGIC_DOUBLE4 ||
          m->GetTreeNodeInputChannelWeight(treeNodeID, 3) != MAGIC_DOUBLE2)
        {
        std::cerr << "Error moving input channel weight" << std::endl;
        pass = false;
        localPass = false;
        }

      if (m->GetNthTargetVolumeIntensityNormalizationNormValue(0) != MAGIC_DOUBLE5 ||
          m->GetNthTargetVolumeIntensityNormalizationNormValue(1) != MAGIC_DOUBLE ||
          m->GetNthTargetVolumeIntensityNormalizationNormValue(2) != MAGIC_DOUBLE4 ||
          m->GetNthTargetVolumeIntensityNormalizationNormValue(3) != MAGIC_DOUBLE2)
        {
        std::cerr << "Error moving normalization parameters" << std::endl;
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
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  vtkEventBroker::GetInstance()->Delete(); 
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
  
  return (pass ? returnValue : EXIT_FAILURE);
}

