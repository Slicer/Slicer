#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkEMSegmentMRMLManager.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegment.h"

#include "vtkMRMLScene.h"

#include "vtkMRMLEMSNode.h"
#include "vtkMRMLEMSSegmenterNode.h"
#include "vtkMRMLEMSTemplateNode.h"
#include "vtkMRMLEMSTreeNode.h"
#include "vtkMRMLEMSTreeParametersLeafNode.h"
#include "vtkMRMLEMSTreeParametersParentNode.h"
#include "vtkMRMLEMSTreeParametersNode.h"
#include "vtkMRMLEMSIntensityNormalizationParametersNode.h"
#include "vtkImageEMLocalSegmenter.h"
#include "vtkImageEMLocalSuperClass.h"
#include "vtkImageMeanIntensityNormalization.h"

#include "vtkSlicerVolumesLogic.h"
#include "vtkMatrix4x4.h"
#include "vtkMath.h"

// needed to translate between enums
#include "EMLocalInterface.h"

#include <math.h>
#include <exception>

#define ERROR_NODE_VTKID 0

//----------------------------------------------------------------------------
vtkEMSegmentMRMLManager* vtkEMSegmentMRMLManager::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkEMSegmentMRMLManager");
  if(ret)
    {
    return (vtkEMSegmentMRMLManager*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkEMSegmentMRMLManager;
}


//----------------------------------------------------------------------------
vtkEMSegmentMRMLManager::vtkEMSegmentMRMLManager()
{
  this->Node = NULL;
  this->NextVTKNodeID = 1000;
  this->HideNodesFromEditors = false;
  //this->DebugOn();
}

//----------------------------------------------------------------------------
vtkEMSegmentMRMLManager::~vtkEMSegmentMRMLManager()
{
  vtkSetMRMLNodeMacro(this->Node, NULL);
}

//----------------------------------------------------------------------------
void 
vtkEMSegmentMRMLManager::
ProcessMRMLEvents(vtkObject* caller,
                  unsigned long event,
                  void* callData)
{
  vtkDebugMacro("vtkEMSegmentMRMLManager::ProcessMRMLEvents: got an event " 
                << event);

  if (vtkMRMLScene::SafeDownCast(caller) != this->MRMLScene)
    {
    return;
    }

  vtkMRMLNode *node = (vtkMRMLNode*)(callData);
  if (node == NULL)
    {
    return;
    }

  if (event == vtkMRMLScene::NodeAddedEvent)
    {
    if (node->IsA("vtkMRMLEMSTreeNode"))
      {
      vtkIdType newID = this->GetNewVTKNodeID();
      this->IDMapInsertPair(newID, node->GetID());
      this->TreeInfoMap[newID] = TreeInfo();
      }
    else if (node->IsA("vtkMRMLVolumeNode"))
      {
      vtkIdType newID = this->GetNewVTKNodeID();
      this->IDMapInsertPair(newID, node->GetID());
      }
    }
  else if (event == vtkMRMLScene::NodeRemovedEvent)
    {
    if (node->IsA("vtkMRMLEMSTreeNode"))
      {
      vtkIdType   vtkID  = this->MapMRMLNodeIDToVTKNodeID(node->GetID());
      this->IDMapRemovePair(node->GetID());
      this->TreeInfoMap.erase(vtkID);
      }
    else if (node->IsA("vtkMRMLVolumeNode"))
      {
      this->IDMapRemovePair(node->GetID());
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::SetAndObserveNode(vtkMRMLEMSNode *n)
{
  vtkMRMLEMSNode* oldNode = this->Node;

  vtkSetAndObserveMRMLNodeMacro(this->Node, n);
  this->UpdateMapsFromMRML();  

  int ok = this->CheckMRMLNodeStructure();
  if (!ok)
    {
    vtkErrorMacro("Incomplete or invalid MRML node structure.");
    if (oldNode != NULL)
      {
      vtkSetAndObserveMRMLNodeMacro(this->Node, oldNode);
      this->UpdateMapsFromMRML();
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent 
     << (this->Node ? 
         this->Node->GetID() : 
         "(none)") 
     << "\n";
}

//----------------------------------------------------------------------------
vtkIdType 
vtkEMSegmentMRMLManager::
GetTreeRootNodeID()
{
  vtkMRMLEMSTreeNode* rootNode = this->GetTreeRootNode();
  if (rootNode == NULL)
    {
    vtkErrorMacro("Tree root node is NULL");
    return ERROR_NODE_VTKID;
    }

  return this->MapMRMLNodeIDToVTKNodeID(rootNode->GetID());
}

//----------------------------------------------------------------------------
int 
vtkEMSegmentMRMLManager::
GetTreeNodeIsLeaf(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetNumberOfChildNodes() == 0;
}

//----------------------------------------------------------------------------
int 
vtkEMSegmentMRMLManager::
GetTreeNodeNumberOfChildren(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetNumberOfChildNodes();
}

//----------------------------------------------------------------------------
vtkIdType 
vtkEMSegmentMRMLManager::
GetTreeNodeChildNodeID(vtkIdType parentNodeID, int childIndex)
{
  vtkMRMLEMSTreeNode* parentNode = this->GetTreeNode(parentNodeID);
  if (parentNode == NULL)
    {
    vtkErrorMacro("Parent tree node is null for nodeID: " << parentNodeID);
    return ERROR_NODE_VTKID;
    }

  vtkMRMLEMSTreeNode* childNode = parentNode->GetNthChildNode(childIndex);
  if (childNode == NULL)
    {
    vtkErrorMacro("Child tree node is null for parent id / child number=" 
                  << parentNodeID << "/" << childIndex);
    return ERROR_NODE_VTKID;
    }

  return this->MapMRMLNodeIDToVTKNodeID(childNode->GetID());
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetTreeNodeParentNodeID(vtkIdType childNodeID)
{
  vtkMRMLEMSTreeNode* childNode = this->GetTreeNode(childNodeID);
  if (childNode == NULL)
    {
    vtkErrorMacro("Child tree node is null for nodeID: " << childNodeID);
    return ERROR_NODE_VTKID;
    }

  vtkMRMLEMSTreeNode* parentNode = childNode->GetParentNode();
  if (parentNode == NULL)
    {
    vtkErrorMacro("Child's parent node is null for nodeID: " << childNodeID);
    return ERROR_NODE_VTKID;
    }
  else
    {
    return this->MapMRMLNodeIDToVTKNodeID(parentNode->GetID());
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeParentNodeID(vtkIdType childNodeID, vtkIdType newParentNodeID)
{
  vtkMRMLEMSTreeNode* childNode  = this->GetTreeNode(childNodeID);
  if (childNode == NULL)
    {
    vtkErrorMacro("Child tree node is null for nodeID: " << childNodeID);
    return;
    }

  vtkMRMLEMSTreeNode* parentNode = this->GetTreeNode(newParentNodeID);
  if (parentNode == NULL)
    {
    vtkErrorMacro("Parent tree node is null for nodeID: " << newParentNodeID);
    return;
    }

  childNode->SetParentNodeID(parentNode->GetID());
}

//----------------------------------------------------------------------------
vtkIdType 
vtkEMSegmentMRMLManager::
AddTreeNode(vtkIdType parentNodeID)
{
  //
  // creates the node (and associated parameters nodes), adds the node
  // to the mrml scene, adds an ID mapping entry
  //
  vtkIdType childNodeID = this->AddNewTreeNode();

  this->SetTreeNodeParentNodeID(childNodeID, parentNodeID);
  return childNodeID;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RemoveTreeNode(vtkIdType removedNodeID)
{
  vtkMRMLEMSTreeNode* node = this->GetTreeNode(removedNodeID);
  if (node == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << removedNodeID);
    return;
    }

  // remove child nodes recursively 

  //  NB: get a list of child ids first
  // because GetTreeNodeChildNodeID is invalidated if a child is
  // removed
  int numChildren = this->GetTreeNodeNumberOfChildren(removedNodeID);
  vtkstd::vector<vtkIdType> childIDs(numChildren);
  int i;
  for (i = 0; i < numChildren; ++i)
    {
    childIDs[i] = this->GetTreeNodeChildNodeID(removedNodeID, i);
    }
  for (i = 0; i < numChildren; ++i)
    {
    this->RemoveTreeNode(childIDs[i]);
    }

  // remove parameters nodes associated with this node
  this->RemoveTreeNodeParametersNodes(removedNodeID);

  // remove node from scene  
  this->GetMRMLScene()->RemoveNode(node);
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
GetTreeNodeLabel(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    if (nodeID != ERROR_NODE_VTKID)
      {
      vtkWarningMacro("Tree node is null for nodeID: " << nodeID);
      }
    return NULL;
    }
  return n->GetLabel();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeLabel(vtkIdType nodeID, const char* label)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->SetLabel(label);  
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
GetTreeNodeName(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return NULL;
    }
  return n->GetName();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeName(vtkIdType nodeID, const char* name)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->SetName(name);  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
GetTreeNodeColor(vtkIdType nodeID, double rgb[3])
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetColorRGB(rgb);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeColor(vtkIdType nodeID, double rgb[3])
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->SetColorRGB(rgb);
}

//----------------------------------------------------------------------------
// Manual, Manually Sample, Auto-Sample
int
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionSpecificationMethod(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("Distribution specification type not found for nodeID: "
                  << nodeID);
    return -1;
    }

  return this->TreeInfoMap[nodeID].DistributionSpecificationMethod;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeDistributionSpecificationMethod(vtkIdType nodeID, 
                                           int method)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("Distribution specification type not found for nodeID: "
                  << nodeID);
    return;
    }

  this->TreeInfoMap[nodeID].DistributionSpecificationMethod = method;

  // update mean and covariance
  // !!!this should be done in logic
  //if (method == vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample)
  //    {
  //this->UpdateMeanAndCovarianceFromSample(nodeID);
  //}
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::   
GetTreeNodeDistributionLogMean(vtkIdType nodeID, 
                               int volumeNumber)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetLeafParametersNode()->
    GetLogMean(volumeNumber);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeDistributionLogMean(vtkIdType nodeID, 
                               int volumeNumber, 
                               double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  n->GetParametersNode()->GetLeafParametersNode()->
    SetLogMean(volumeNumber, value);  
}

//----------------------------------------------------------------------------
double   
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionLogCovariance(vtkIdType nodeID, 
                                     int rowIndex,
                                     int columnIndex)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetLeafParametersNode()->
    GetLogCovariance(rowIndex, columnIndex);  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeDistributionLogCovariance(vtkIdType nodeID, 
                                     int rowIndex, 
                                     int columnIndex,
                                     double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  n->GetParametersNode()->GetLeafParametersNode()->
    SetLogCovariance(rowIndex, columnIndex, value);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionNumberOfSamples(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("SampledPointsList not found for node: " << nodeID);
    return 0;
    }
  
  return this->TreeInfoMap[nodeID].SamplePoints.size();
}

//----------------------------------------------------------------------------
// send RAS coordinates, returns non-zero if point is indeed added
int
vtkEMSegmentMRMLManager::
AddTreeNodeDistributionSamplePoint(vtkIdType nodeID, double xyz[3])
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("SamplePointsList not found for node: " << nodeID);
    return 0;
    }

  vtksys_stl::vector<double> point(3);
  point[0] = xyz[0];
  point[1] = xyz[1];
  point[2] = xyz[2];

  this->TreeInfoMap[nodeID].SamplePoints.push_back(point);
  
  //!!! in logic
  //if (this->TreeInfoMap[nodeID].DistributionSpecificationMethod == 
  //    vtkEMSegmentLogic::DistributionSpecificationManuallySample)
  //  {
  //  this->UpdateMeanAndCovarianceFromSample(nodeID);
  //  }

  // for now: all points are added
  return 1;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RemoveTreeNodeDistributionSamplePoint(vtkIdType nodeID, int sampleNumber)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("SamplePointsList not found for node: " << nodeID);
    return;
    }
  
  this->TreeInfoMap[nodeID].SamplePoints.
    erase(this->TreeInfoMap[nodeID].SamplePoints.begin() + sampleNumber);

  // !!! in logic
//   if (this->TreeInfoMap[nodeID].DistributionSpecificationMethod ==
//       vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample)
//     {
//     this->UpdateMeanAndCovarianceFromSample(nodeID);
//     }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RemoveAllTreeNodeDistributionSamplePoints(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("SamplePointsList not found for node: " << nodeID);
    return;
    }
  
  this->TreeInfoMap[nodeID].SamplePoints.clear();

  // !!! in logic
//   if (this->TreeInfoMap[nodeID].DistributionSpecificationMethod ==
//       vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample)
//     {
//     this->UpdateMeanAndCovarianceFromSample(nodeID);
//     }
}


//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionSamplePoint(vtkIdType nodeID, int sampleNumber,
                                   double xyz[3])
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("SamplePointsList not found for node: " << nodeID);
    return;
    }

  xyz[0] = this->TreeInfoMap[nodeID].SamplePoints[sampleNumber][0];
  xyz[1] = this->TreeInfoMap[nodeID].SamplePoints[sampleNumber][1];
  xyz[2] = this->TreeInfoMap[nodeID].SamplePoints[sampleNumber][2];
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionSampleIntensityValue(vtkIdType nodeID, 
                                            int sampleNumber, 
                                            vtkIdType imageID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("SamplePointsList not found for node: " << nodeID);
    return 0;
    }

  vtkMRMLVolumeNode* volumeNode = this->GetVolumeNode(imageID);
  if (volumeNode == NULL)
    {
    vtkErrorMacro("Volume node is null for id: " << imageID);
    return 0;
    }

  // get point and convert from RAS to IJK coordinates
  vtksys_stl::vector<double> point = 
    this->TreeInfoMap[nodeID].SamplePoints[sampleNumber];
  double rasPoint[4] = { point[0], point[1], point[2], 1.0 };
  double ijkPoint[4];
  vtkMatrix4x4* rasToijk = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(rasToijk);
  rasToijk->MultiplyPoint(rasPoint, ijkPoint);
  rasToijk->Delete();

  vtkImageData* imageData = volumeNode->GetImageData();
  double intensityValue = imageData->
    GetScalarComponentAsDouble(static_cast<int>(vtkMath::Round(ijkPoint[0])), 
                               static_cast<int>(vtkMath::Round(ijkPoint[1])), 
                               static_cast<int>(vtkMath::Round(ijkPoint[2])),
                               0);
  return intensityValue;
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintWeight(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetPrintWeights();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintWeight(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->SetPrintWeights(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintQuality(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetLeafParametersNode()->GetPrintQuality();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintQuality(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetLeafParametersNode()->
    SetPrintQuality(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeIntensityLabel(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetLeafParametersNode()->
    GetIntensityLabel();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeIntensityLabel(vtkIdType nodeID, int label)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetLeafParametersNode()->SetIntensityLabel(label);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintFrequency(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintFrequency();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintFrequency(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetPrintFrequency(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintLabelMap(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintLabelMap();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintLabelMap(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetPrintLabelMap(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintEMLabelMapConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintEMLabelMapConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintEMLabelMapConvergence(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetPrintEMLabelMapConvergence(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintEMWeightsConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintEMWeightsConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintEMWeightsConvergence(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetPrintEMWeightsConvergence(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintMFALabelMapConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintMFALabelMapConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintMFALabelMapConvergence(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetPrintMFALabelMapConvergence(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintMFAWeightsConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintMFAWeightsConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintMFAWeightsConvergence(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetPrintMFAWeightsConvergence(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeGenerateBackgroundProbability(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetGenerateBackgroundProbability();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeGenerateBackgroundProbability(vtkIdType nodeID, int value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetGenerateBackgroundProbability(value);  
}


//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeExcludeFromIncompleteEStep(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  return n->GetParametersNode()->GetExcludeFromIncompleteEStep();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeExcludeFromIncompleteEStep(vtkIdType nodeID, int shouldExclude)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->SetExcludeFromIncompleteEStep(shouldExclude);  
}


//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeAlpha(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->GetAlpha();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeAlpha(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  n->GetParametersNode()->GetParentParametersNode()->SetAlpha(value);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintBias(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->GetPrintBias();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintBias(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  n->GetParametersNode()->GetParentParametersNode()->SetPrintBias(shouldPrint);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeBiasCalculationMaxIterations(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->
    GetBiasCalculationMaxIterations();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeBiasCalculationMaxIterations(vtkIdType nodeID, int value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  n->GetParametersNode()->GetParentParametersNode()->
    SetBiasCalculationMaxIterations(value);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeSmoothingKernelWidth(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->
    GetSmoothingKernelWidth();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeSmoothingKernelWidth(vtkIdType nodeID, int value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  n->GetParametersNode()->GetParentParametersNode()->
    SetSmoothingKernelWidth(value);  
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeSmoothingKernelSigma(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->
    GetSmoothingKernelSigma();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeSmoothingKernelSigma(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  n->GetParametersNode()->GetParentParametersNode()->
    SetSmoothingKernelSigma(value);  
}


//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeClassProbability(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetClassProbability();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeClassProbability(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->SetClassProbability(value);  
}


//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeSpatialPriorWeight(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetSpatialPriorWeight();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeSpatialPriorWeight(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->SetSpatialPriorWeight(value);
}


//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeInputChannelWeight(vtkIdType nodeID, int volumeNumber)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetInputChannelWeight(volumeNumber);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeInputChannelWeight(vtkIdType nodeID, int volumeNumber, 
                              double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->SetInputChannelWeight(volumeNumber, value);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionEMType(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }
  int stopType = n->GetParametersNode()->
    GetParentParametersNode()->GetStopEMType();
  return this->ConvertAlgorithmEnumToGUIEnumStoppingConditionType(stopType);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionEMType(vtkIdType nodeID, 
                                   int conditionType)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  int algConditionType = this->
    ConvertGUIEnumToAlgorithmEnumStoppingConditionType(conditionType);
  n->GetParametersNode()->
    GetParentParametersNode()->SetStopEMType(algConditionType);
}
  
//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionEMValue(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->GetStopEMValue();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionEMValue(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->SetStopEMValue(value);
}
  
//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionEMIterations(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->
    GetStopEMMaxIterations();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionEMIterations(vtkIdType nodeID,
                                         int iterations)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetStopEMMaxIterations(iterations);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionMFAType(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  int stopType = n->GetParametersNode()->GetParentParametersNode()->
    GetStopMFAType();
  return this->ConvertAlgorithmEnumToGUIEnumStoppingConditionType(stopType);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionMFAType(vtkIdType nodeID, 
                                    int conditionType)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  int algConditionType = this->
    ConvertGUIEnumToAlgorithmEnumStoppingConditionType(conditionType);
  n->GetParametersNode()->GetParentParametersNode()->
    SetStopMFAType(algConditionType);
}
  
//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionMFAValue(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->GetStopMFAValue();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionMFAValue(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->SetStopMFAValue(value);
}
  
//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionMFAIterations(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  return n->GetParametersNode()->GetParentParametersNode()->
    GetStopMFAMaxIterations();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionMFAIterations(vtkIdType nodeID,
                                          int iterations)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }
  n->GetParametersNode()->GetParentParametersNode()->
    SetStopMFAMaxIterations(iterations);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetVolumeNumberOfChoices()
{
  //
  // for now, just the the get number of volumes loaded into the
  // slicer scene
  //
  return this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetVolumeNthID(int n)
{
  vtkMRMLNode* node = this->GetMRMLScene()->
    GetNthNodeByClass(n, "vtkMRMLVolumeNode");

  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth volume in scene: " << n);
    return ERROR_NODE_VTKID;
    }

  if (this->IDMapContainsMRMLNodeID(node->GetID()))
    {
    return this->MapMRMLNodeIDToVTKNodeID(node->GetID());
    }
  else
    {
    vtkErrorMacro("Volume MRML ID was not in map!" << node->GetID());
    return ERROR_NODE_VTKID;
    }
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
GetVolumeName(vtkIdType volumeID)
{
  vtkMRMLVolumeNode* volumeNode = this->GetVolumeNode(volumeID);
  if (volumeNode == NULL)
    {
    return NULL;
    }
  return volumeNode->GetName();
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return ERROR_NODE_VTKID;
    }

  // get name of atlas volume from tree node
  char* atlasVolumeName = n->GetParametersNode()->GetSpatialPriorVolumeName();
  if (atlasVolumeName == NULL || strlen(atlasVolumeName) == 0)
    {
    return ERROR_NODE_VTKID;
    }

  // get MRML volume ID from atas node
  const char* mrmlVolumeNodeID = 
    this->GetAtlasNode()->GetVolumeNodeIDByKey(atlasVolumeName);
  
  if (mrmlVolumeNodeID == NULL || strlen(atlasVolumeName) == 0)
    {
    vtkErrorMacro("MRMLID for prior volume is null; nodeID=" << nodeID);
    return ERROR_NODE_VTKID;
    }
  else if (this->IDMapContainsMRMLNodeID(mrmlVolumeNodeID))
    {
    // convert mrml id to vtk id
    return this->MapMRMLNodeIDToVTKNodeID(mrmlVolumeNodeID);
    }
  else
    {
    vtkErrorMacro("Volume MRML ID was not in map! atlasVolumeName = " 
                  << atlasVolumeName << " mrmlID = " << mrmlVolumeNodeID);
    return ERROR_NODE_VTKID;
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID, 
                                vtkIdType volumeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return;
    }

  // map volume id to MRML ID
  const char* volumeMRMLID = MapVTKNodeIDToMRMLNodeID(volumeID);
  if (volumeMRMLID == NULL || strlen(volumeMRMLID) == 0)
    {
    vtkErrorMacro("Could not map volume ID: " << volumeID);
    return;
    }

  // use tree node label (or mrml id if label is not specified)
  vtksys_stl::string priorVolumeName;
  if (n->GetLabel() == NULL || strlen(n->GetLabel()) == 0)
    {
    priorVolumeName = n->GetID();
    }
  else
    {
    priorVolumeName = n->GetLabel();
    }

  // add key value pair to atlas
  this->GetAtlasNode()->AddVolume(priorVolumeName.c_str(), volumeMRMLID);

  // set name of atlas volume in tree node
  n->GetParametersNode()->SetSpatialPriorVolumeName(priorVolumeName.c_str());
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTargetNumberOfSelectedVolumes()
{
  
  if (this->GetTargetNode())
    {
    return this->GetTargetNode()->GetNumberOfVolumes();
    }
  else
    {
    if (this->Node != NULL)
      {
      vtkWarningMacro("Can't get number of target volumes while " \
                      "EMSNode is nonnull");
      }
    vtkErrorMacro("Target node is NULL.");
    return 0;
    }
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetTargetSelectedVolumeNthID(int n)
{
  const char* mrmlID = 
    this->GetTargetNode()->GetNthVolumeNodeID(n);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Did not find nth target volume; n = " << n);
    return ERROR_NODE_VTKID;
    }
  else if (!this->IDMapContainsMRMLNodeID(mrmlID))
    {
    vtkErrorMacro("Volume MRML ID was not in map!" << mrmlID);
    return ERROR_NODE_VTKID;
    }
  else
    {
    // convert mrml id to vtk id
    return this->MapMRMLNodeIDToVTKNodeID(mrmlID);
    }
}
 
//----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
GetTargetSelectedVolumeNthMRMLID(int n)
{
  if (!this->GetTargetNode())
  {
    vtkWarningMacro("Can't access target node.");
    return NULL;
  }
  return
    this->GetTargetNode()->GetNthVolumeNodeID(n);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
AddTargetSelectedVolume(vtkIdType volumeID)
{
  vtkMRMLVolumeNode* volumeNode = this->GetVolumeNode(volumeID);
  if (volumeNode == NULL)
    {
    vtkErrorMacro("Invalid volume ID: " << volumeID);
    return;
    }

  // map to MRML ID
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not map volume ID: " << volumeID);
    return;
    }

  // get volume name
  std::string name = volumeNode->GetName();
  if (name.empty())
    {
    name = volumeNode->GetID();
    }

  // set volume name and ID in map
  this->GetTargetNode()->AddVolume(name.c_str(), mrmlID);
  
  // propogate change to parameters nodes
  this->PropogateAdditionOfSelectedTargetImage();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
AddTargetSelectedVolumeByMRMLID(char* mrmlID)
{
  vtkIdType volumeID = this->MapMRMLNodeIDToVTKNodeID(mrmlID);
  this->AddTargetSelectedVolume(volumeID);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RemoveTargetSelectedVolume(vtkIdType volumeID)
{
  // map to MRML ID
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not map volume ID: " << volumeID);
    return;
    }

  // get this image's index in the target list
  int imageIndex = this->GetTargetNode()->GetIndexByVolumeNodeID(mrmlID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // remove from target
  this->GetTargetNode()->RemoveVolumeByNodeID(mrmlID);

  // propogate change to parameters nodes
  this->PropogateRemovalOfSelectedTargetImage(imageIndex);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
MoveTargetSelectedVolume(int fromIndex, int toIndex)
{
  // make sure the indices make sense
  if (fromIndex < 0 || fromIndex >= this->GetTargetNumberOfSelectedVolumes())
    {
    vtkErrorMacro("invalid target from index " << fromIndex);
    return;
    }
  if (toIndex < 0 || toIndex >= this->GetTargetNumberOfSelectedVolumes())
    {
    vtkErrorMacro("invalid target to index " << toIndex);
    return;
    }

  // move inside target node
  this->GetTargetNode()->MoveNthVolume(fromIndex, toIndex);

  // propogate change to parameters nodes
  this->PropogateMovementOfSelectedTargetImage(fromIndex, toIndex);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationToDefaultT1SPGR(int n)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetToDefaultT1SPGR();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationToDefaultT2(int n)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetToDefaultT2();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationToDefaultT2_2(int n)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetToDefaultT2_2();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationNormValue(int n, double d)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetNormValue(d);  
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetNthTargetVolumeIntensityNormalizationNormValue(int n)
{
  return this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    GetNormValue();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationNormType(int n, int t)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetNormType(t);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetNthTargetVolumeIntensityNormalizationNormType(int n)
{
  return this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    GetNormType();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth
(int n, int t)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetInitialHistogramSmoothingWidth(t);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetNthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth
(int n)
{
  return this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    GetInitialHistogramSmoothingWidth();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(int n, 
                                                                   int t)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetMaxHistogramSmoothingWidth(t);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetNthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(int n)
{
  return this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    GetMaxHistogramSmoothingWidth();  
}

void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(int n, 
                                                            float f)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetRelativeMaxVoxelNum(f);  
}

//----------------------------------------------------------------------------
float
vtkEMSegmentMRMLManager::
GetNthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(int n)
{
  return this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    GetRelativeMaxVoxelNum();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationPrintInfo(int n, int t)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetPrintInfo(t);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetNthTargetVolumeIntensityNormalizationPrintInfo(int n)
{
  return this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    GetPrintInfo();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetNthTargetVolumeIntensityNormalizationEnabled(int n, int t)
{
  this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    SetEnabled(t);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetNthTargetVolumeIntensityNormalizationEnabled(int n)
{
  if (!this->GetGlobalParametersNode()->
      GetNthIntensityNormalizationParametersNode(n))
    {
      return 0;
    }
  return this->GetGlobalParametersNode()->
    GetNthIntensityNormalizationParametersNode(n)->
    GetEnabled();  
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetRegistrationAtlasVolumeID()
{
  if (!this->GetGlobalParametersNode())
    {
    vtkErrorMacro("GlobalParametersNode is NULL.");
    return ERROR_NODE_VTKID;
    }

  // the the name of the atlas image from the global parameters
  char* volumeName = this->GetGlobalParametersNode()->
    GetRegistrationAtlasVolumeKey();

  if (volumeName == NULL || strlen(volumeName) == 0)
    {
    vtkWarningMacro("AtlasVolumeName is NULL/blank.");
    return ERROR_NODE_VTKID;
    }

  // get MRML ID of atlas from it's name
  const char* mrmlID = this->GetAtlasNode()->GetVolumeNodeIDByKey(volumeName);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not find mrml ID for registration atlas volume.");
    return ERROR_NODE_VTKID;
    }
  
  // convert mrml id to vtk id
  return this->MapMRMLNodeIDToVTKNodeID(mrmlID);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetRegistrationAtlasVolumeID(vtkIdType volumeID)
{
  // for now there can be only one atlas image for registration
  vtksys_stl::string registrationVolumeName = "atlas_registration_image";

  // map to MRML ID
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not map volume ID: " << volumeID);
    return;
    }

  // set volume name and ID in map
  this->GetTargetNode()->AddVolume(registrationVolumeName.c_str(), mrmlID);

  this->GetGlobalParametersNode()->
    SetRegistrationAtlasVolumeKey(registrationVolumeName.c_str());
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetRegistrationTargetVolumeID()
{
  if (this->GetGlobalParametersNode() == NULL)
    {
    vtkErrorMacro("GlobalParametersNode is NULL.");
    return ERROR_NODE_VTKID;
    }

  // the the name of the target image from the global parameters
  char* volumeName = this->GetGlobalParametersNode()->
    GetRegistrationTargetVolumeKey();
  if (volumeName == NULL || strlen(volumeName) == 0)
    {
    vtkWarningMacro("TargetVolumeName is NULL/blank.");
    return ERROR_NODE_VTKID;
    }

  // get MRML ID of target image from it's name
  const char* mrmlID = 
    this->GetTargetNode()->GetVolumeNodeIDByKey(volumeName);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not find mrml ID for registration target volume.");
    return ERROR_NODE_VTKID;
    }

  // convert mrml id to vtk id
  return this->MapMRMLNodeIDToVTKNodeID(mrmlID);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetRegistrationTargetVolumeID(vtkIdType volumeID)
{
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not find mrml ID for volumeID: " << volumeID);
    }

  // get name for this image from target node
  const char* volumeKey = this->GetTargetNode()->GetKeyByVolumeNodeID(mrmlID);
  if (volumeKey == NULL || strlen(volumeKey) == 0)
    {
    vtkErrorMacro("Volume with id " << volumeID 
                  << " is not contained in target");
    }

  // set name in global parameters
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetRegistrationTargetVolumeKey(volumeKey);
    }
  else
    {
    vtkErrorMacro("Can't set registration target volume in " \
                  "null GlobalParameters");
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetRegistrationAffineType()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetRegistrationAffineType() : 0;  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetRegistrationAffineType(int affineType)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetRegistrationAffineType(affineType);
    }
}


//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetRegistrationDeformableType()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetRegistrationDeformableType() : 0;  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetRegistrationDeformableType(int deformableType)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetRegistrationDeformableType(deformableType);
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetRegistrationInterpolationType()
{
  int algType = this->GetGlobalParametersNode() ? 
    this->GetGlobalParametersNode()->GetRegistrationInterpolationType() : 0;  
  return this->ConvertAlgorithmEnumToGUIEnumInterpolationType(algType);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetRegistrationInterpolationType(int interpolationType)
{
  int algType = this->
    ConvertGUIEnumToAlgorithmEnumStoppingConditionType(interpolationType);
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetRegistrationInterpolationType(algType);  
    }
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
GetSaveWorkingDirectory()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetWorkingDirectory() : NULL;  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSaveWorkingDirectory(const char* directory)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetWorkingDirectory(directory);  
    }
}


//----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
GetSaveTemplateFilename()
{
  if (this->Node == NULL)
    {
    return NULL;
    }
  return this->Node->GetTemplateFilename();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSaveTemplateFilename(const char* file)
{
  if (this->Node == NULL)
    {
    vtkErrorMacro("Attempt to set TemplateFilename in  NULL EMSNode");
    return;
    }

  this->Node->SetTemplateFilename(file);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetSaveTemplateAfterSegmentation()
{
  if (this->Node == NULL)
    {
    return 0;
    }
  return this->Node->GetSaveTemplateAfterSegmentation();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSaveTemplateAfterSegmentation(int shouldSave)
{
  if (this->Node == NULL)
    {
    vtkErrorMacro("Attempt to set SaveTemplateAfter in  NULL EMSNode");
    return;
    }
  this->Node->SetSaveTemplateAfterSegmentation(shouldSave);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetSaveIntermediateResults()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetSaveIntermediateResults() : 0;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSaveIntermediateResults(int shouldSaveResults)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetSaveIntermediateResults(shouldSaveResults);
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetSaveSurfaceModels()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetSaveSurfaceModels() : 0;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSaveSurfaceModels(int shouldSaveModels)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetSaveSurfaceModels(shouldSaveModels);  
    }
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
GetOutputVolumeMRMLID()
{
  if (!this->GetSegmenterNode())
    {
    if (this->Node)
      {
      vtkWarningMacro("Can't get Segmenter and EMSNode is nonnull.");
      }
    return NULL;
    }
  return this->GetSegmenterNode()->GetOutputVolumeNodeID();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetOutputVolumeMRMLID(const char* mrmlID)
{
  if (!this->GetSegmenterNode())
    {
    if (this->Node)
      {
      vtkWarningMacro("Can't get Segmenter and EMSNode is nonnull.");
      }
    }
  else
    {
    this->GetSegmenterNode()->SetOutputVolumeNodeID(mrmlID);

    //
    // the output volume is a segmentation: make sure it is a labelmap
    //
    vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::
      SafeDownCast(this->GetMRMLScene()->GetNodeByID(mrmlID));
    if (outVolume == NULL)
      {
      vtkErrorMacro("Output volume is null");
      }

    bool isLabelMap = outVolume->GetLabelMap();
    if (!isLabelMap)
      {
      vtkWarningMacro("Changing output image to labelmap");
      outVolume->LabelMapOn();
      }
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetOutputVolumeID(vtkIdType volumeID)
{
  vtkMRMLVolumeNode* volumeNode = this->GetVolumeNode(volumeID);
  if (volumeNode == NULL)
    {
    vtkErrorMacro("Invalid volume ID: " << volumeID);
    return;
    }

  // map to MRML ID
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not map volume ID: " << volumeID);
    return;
    }

  this->SetOutputVolumeMRMLID(mrmlID);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetEnableMultithreading()
{
  if (this->GetGlobalParametersNode())
  {
    return this->GetGlobalParametersNode()->GetMultithreadingEnabled();
  }
  else
  {
    vtkErrorMacro("Attempt to read from non-existing global parameter node.");
    return 0;
  }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetEnableMultithreading(int isEnabled)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetMultithreadingEnabled(isEnabled);
    }
  else
  {
    vtkErrorMacro("Attempt to modify non-existing global parameter node.");
  }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
GetSegmentationBoundaryMin(int minPoint[3])
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      GetSegmentationBoundaryMin(minPoint);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSegmentationBoundaryMin(int minPoint[3])
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetSegmentationBoundaryMin(minPoint);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
GetSegmentationBoundaryMax(int maxPoint[3])
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      GetSegmentationBoundaryMax(maxPoint);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSegmentationBoundaryMax(int maxPoint[3])
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetSegmentationBoundaryMax(maxPoint);
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetAtlasNumberOfTrainingSamples()
{
  if (!this->GetAtlasNode())
    {
    vtkErrorMacro("Atlas node is null.");
    return 0;
    }
  return this->GetAtlasNode()->GetNumberOfTrainingSamples();
}

// ????
//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
HasGlobalParametersNode()
{
  return this->GetGlobalParametersNode() ? 1 : 0;
}

//----------------------------------------------------------------------------
vtkMRMLEMSGlobalParametersNode*
vtkEMSegmentMRMLManager::
GetGlobalParametersNode()
{
  vtkMRMLEMSTemplateNode* templateNode = this->GetTemplateNode();  
  if (templateNode == NULL)
    {
    if (this->Node != NULL)
      {
      vtkWarningMacro("Null TemplateNode with nonnull EMSNode.");
      }
    return NULL;
    }
  else
    {
    return templateNode->GetGlobalParametersNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkEMSegmentMRMLManager::
GetTreeRootNode()
{
  vtkMRMLEMSTemplateNode* templateNode = this->GetTemplateNode();
  if (templateNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null TemplateNode with nonnull EMSNode.");
      }
    return NULL;
    }
  else
    {
    return templateNode->GetTreeNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkEMSegmentMRMLManager::
GetTreeNode(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* node = NULL;
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(nodeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkWarningMacro("Can't find tree node for id: " << nodeID);
    return NULL;
    }

  if (this->GetMRMLScene())
    {
    vtkMRMLNode* snode = this->GetMRMLScene()->GetNodeByID(mrmlID);
    node = vtkMRMLEMSTreeNode::SafeDownCast(snode);

    if (node == NULL)
      {
      vtkErrorMacro("Attempt to cast to tree node from non-tree mrml id: " << 
                    mrmlID);
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode*
vtkEMSegmentMRMLManager::
GetVolumeNode(vtkIdType volumeID)
{
  vtkMRMLVolumeNode* node = NULL;
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Unknown volumeID: " << volumeID);
    return NULL;
    }

  if (this->GetMRMLScene())
    {
    vtkMRMLNode* snode = this->GetMRMLScene()->GetNodeByID(mrmlID);
    node = vtkMRMLVolumeNode::SafeDownCast(snode);
   
    if (node == NULL)
      {
      vtkErrorMacro("Attempt to cast to volume node from non-volume mrml id: " 
                    << mrmlID);
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkEMSegmentMRMLManager::
GetTargetNode()
{
  vtkMRMLEMSSegmenterNode* segmenterNode = this->GetSegmenterNode();
  if (segmenterNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null SegmenterNode with nonnull EMSNode.");
      }
    return NULL;
    }
  else
    {
    return segmenterNode->GetTargetNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkEMSegmentMRMLManager::
GetAtlasNode()
{
  vtkMRMLEMSSegmenterNode* segmenterNode = this->GetSegmenterNode();
  if (segmenterNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null SegmenterNode with nonnull EMSNode.");
      }
    return NULL;
    }
  else
    {
    return segmenterNode->GetAtlasNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkEMSegmentMRMLManager::
GetOutputVolumeNode()
{
  vtkMRMLEMSSegmenterNode* segmenterNode = this->GetSegmenterNode();
  if (segmenterNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null SegmenterNode with nonnull EMSNode.");
      }
    return NULL;
    }
  else
    {
    return segmenterNode->GetOutputVolumeNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSTemplateNode*
vtkEMSegmentMRMLManager::
GetTemplateNode()
{
  vtkMRMLEMSSegmenterNode* segmenterNode = this->GetSegmenterNode();
  if (segmenterNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null SegmenterNode with nonnull EMSNode.");
      }
    return NULL;
    }
  else
    {
    return segmenterNode->GetTemplateNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSSegmenterNode*
vtkEMSegmentMRMLManager::
GetSegmenterNode()
{
  if (this->Node == NULL)
    {
    return NULL;
    }
  else
    {
    return this->Node->GetSegmenterNode();
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetNumberOfParameterSets()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("MRML scene is NULL.");
    return 0;
    }
  return this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLEMSNode");
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
GetNthParameterSetName(int n)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("MRML scene is NULL.");
    return NULL;
    }

  vtkMRMLNode* node = 
    this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLEMSNode");

  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth template builder node in scene: " << n);
    return NULL;
    }

  return node->GetName();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetLoadedParameterSetIndex(int n)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("MRML scene is NULL.");
    return;
    }

  vtkMRMLNode* node = this->GetMRMLScene()->
    GetNthNodeByClass(n, "vtkMRMLEMSNode");
  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth template builder node in scene: " << n);
    return;
    }

  vtkMRMLEMSNode* templateBuilderNode = vtkMRMLEMSNode::SafeDownCast(node);
  if (templateBuilderNode == NULL)
    {
    vtkErrorMacro("Failed to cast node to template builder node: " << 
                  node->GetID());
    return;
    }
  
  this->SetAndObserveNode(templateBuilderNode);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CreateAndObserveNewParameterSet()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("MRML scene is NULL.");
    return;
    }

  // create atlas node
  vtkMRMLEMSAtlasNode* atlasNode = vtkMRMLEMSAtlasNode::New();
  atlasNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(atlasNode);

  // create target node
  vtkMRMLEMSTargetNode* targetNode = vtkMRMLEMSTargetNode::New();
  targetNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(targetNode);
  
  // create global parameters node
  vtkMRMLEMSGlobalParametersNode* globalParametersNode = 
    vtkMRMLEMSGlobalParametersNode::New();
  globalParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(globalParametersNode);

  // create root tree parameters nodes
  vtkMRMLEMSTreeParametersLeafNode* leafParametersNode = 
    vtkMRMLEMSTreeParametersLeafNode::New();
  leafParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(leafParametersNode);

  vtkMRMLEMSTreeParametersParentNode* parentParametersNode = 
    vtkMRMLEMSTreeParametersParentNode::New();
  parentParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(parentParametersNode);

  vtkMRMLEMSTreeParametersNode* treeParametersNode = 
    vtkMRMLEMSTreeParametersNode::New();
  treeParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(treeParametersNode);
  
  treeParametersNode->SetLeafParametersNodeID(leafParametersNode->GetID());
  treeParametersNode->SetParentParametersNodeID(parentParametersNode->GetID());

  // create root tree node
  vtkMRMLEMSTreeNode* treeNode = 
    vtkMRMLEMSTreeNode::New();
  treeNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(treeNode);

  // add connections
  treeNode->SetTreeParametersNodeID(treeParametersNode->GetID());

  // create template node
  vtkMRMLEMSTemplateNode* templateNode = 
    vtkMRMLEMSTemplateNode::New();
  templateNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(templateNode);
  
  // add connections
  templateNode->SetTreeNodeID(treeNode->GetID());
  templateNode->SetGlobalParametersNodeID(globalParametersNode->GetID());

  // create segmenter node
  vtkMRMLEMSSegmenterNode* segmenterNode = 
    vtkMRMLEMSSegmenterNode::New();
  segmenterNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(segmenterNode);
  
  // add connections
  segmenterNode->SetTemplateNodeID(templateNode->GetID());
  segmenterNode->SetAtlasNodeID(atlasNode->GetID());
  segmenterNode->SetTargetNodeID(targetNode->GetID());

  // create template builder node
  vtkMRMLEMSNode* templateBuilderNode = 
    vtkMRMLEMSNode::New();
  templateBuilderNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(templateBuilderNode);
  
  // add connections
  templateBuilderNode->SetSegmenterNodeID(segmenterNode->GetID());

  this->SetAndObserveNode(templateBuilderNode);

  // add basic information for root node
  vtkIdType rootID = this->GetTreeRootNodeID();
  this->SetTreeNodeLabel(rootID, "Root");
  this->SetTreeNodeName(rootID, "Root");
  this->SetTreeNodeIntensityLabel(rootID, rootID);

  // delete nodes
  atlasNode->Delete();
  targetNode->Delete();
  globalParametersNode->Delete();
  leafParametersNode->Delete();
  parentParametersNode->Delete();
  treeParametersNode->Delete();
  treeNode->Delete();
  templateNode->Delete();
  segmenterNode->Delete();
  templateBuilderNode->Delete();
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
AddNewTreeNode()
{
  // create parameters nodes and add them to the scene
  vtkMRMLEMSTreeParametersLeafNode* leafParametersNode = 
    vtkMRMLEMSTreeParametersLeafNode::New();
  leafParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(leafParametersNode);

  vtkMRMLEMSTreeParametersParentNode* parentParametersNode = 
    vtkMRMLEMSTreeParametersParentNode::New();
  parentParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(parentParametersNode);

  vtkMRMLEMSTreeParametersNode* treeParametersNode = 
    vtkMRMLEMSTreeParametersNode::New();
  treeParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(treeParametersNode);

  // add connections  
  treeParametersNode->SetLeafParametersNodeID(leafParametersNode->GetID());
  treeParametersNode->SetParentParametersNodeID(parentParametersNode->GetID());

  // update memory
  leafParametersNode->
    SetNumberOfTargetInputChannels(this->GetTargetNode()->
                                   GetNumberOfVolumes());
  treeParametersNode->
    SetNumberOfTargetInputChannels(this->GetTargetNode()->
                                   GetNumberOfVolumes());

  // create tree node and add it to the scene
  vtkMRMLEMSTreeNode* treeNode = 
    vtkMRMLEMSTreeNode::New();
  treeNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(treeNode);

  // add connections
  treeNode->SetTreeParametersNodeID(treeParametersNode->GetID());

  // set the intensity label (in resulting segmentation) to the ID
  vtkIdType treeNodeID = this->MapMRMLNodeIDToVTKNodeID(treeNode->GetID());
  this->SetTreeNodeIntensityLabel(treeNodeID, treeNodeID);
  
  // delete nodes
  leafParametersNode->Delete();
  parentParametersNode->Delete();
  treeParametersNode->Delete();
  treeNode->Delete();

  return treeNodeID;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RemoveTreeNodeParametersNodes(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    }
  vtkMRMLEMSTreeParametersNode* parametersNode = n->GetParametersNode();
  if (parametersNode != NULL)
    {
    // remove leaf node parameters
    vtkMRMLNode* leafParametersNode = parametersNode->GetLeafParametersNode();
    if (leafParametersNode != NULL)
      {
      this->GetMRMLScene()->RemoveNode(leafParametersNode);
      }

    // remove parent node parameters
    vtkMRMLNode* parentParametersNode = parametersNode->
      GetParentParametersNode();
    if (parentParametersNode != NULL)
      {
      this->GetMRMLScene()->RemoveNode(parentParametersNode);
      }

    // remove parameters node
    this->GetMRMLScene()->RemoveNode(parametersNode);
    }
}

//-----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
MapMRMLNodeIDToVTKNodeID(const char* MRMLNodeID)
{
  bool contained = this->MRMLNodeIDToVTKNodeIDMap.count(MRMLNodeID);
  if (!contained)
    {
    vtkErrorMacro("mrml ID does not map to vtk ID: " << MRMLNodeID);
    return ERROR_NODE_VTKID;
    }

  return this->MRMLNodeIDToVTKNodeIDMap[MRMLNodeID];
}

//-----------------------------------------------------------------------------
const char*
vtkEMSegmentMRMLManager::
MapVTKNodeIDToMRMLNodeID(vtkIdType nodeID)
{
  bool contained = this->VTKNodeIDToMRMLNodeIDMap.count(nodeID);
  if (!contained)
    {
    vtkErrorMacro("vtk ID does not map to mrml ID: " << nodeID);
    return NULL;
    }

  vtksys_stl::string mrmlID = this->VTKNodeIDToMRMLNodeIDMap[nodeID];  
  if (mrmlID.empty())
    {
    vtkWarningMacro("vtk ID mapped to null mrml ID: " << nodeID);
    }

  return this->VTKNodeIDToMRMLNodeIDMap[nodeID].c_str();
}

//-----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetNewVTKNodeID()
{
  vtkIdType nextID = this->NextVTKNodeID++;
  if (this->VTKNodeIDToMRMLNodeIDMap.count(nextID) != 0)
    {
    vtkErrorMacro("Duplicate vtk node id issued : " << nextID << "!");
    }
  return nextID;
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
IDMapInsertPair(vtkIdType vtkID, 
                const char* MRMLNodeID)
{
  if (MRMLNodeID == NULL || strlen(MRMLNodeID) == 0)
    {
    vtkErrorMacro("Attempt to insert null or blank mrml id into map; vtkID = " 
                  << vtkID);
    return;
    }
  this->MRMLNodeIDToVTKNodeIDMap[MRMLNodeID] = vtkID;
  this->VTKNodeIDToMRMLNodeIDMap[vtkID] = MRMLNodeID;
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
IDMapRemovePair(vtkIdType vtkID)
{
  this->MRMLNodeIDToVTKNodeIDMap.
    erase(this->VTKNodeIDToMRMLNodeIDMap[vtkID]);
  this->VTKNodeIDToMRMLNodeIDMap.erase(vtkID);
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
IDMapRemovePair(const char* MRMLNodeID)
{
  if (MRMLNodeID == NULL || strlen(MRMLNodeID) == 0)
    {
    vtkErrorMacro("Attempt to remove null or blank mrml id from map");
    return;
    }
  this->VTKNodeIDToMRMLNodeIDMap.
    erase(this->MRMLNodeIDToVTKNodeIDMap[MRMLNodeID]);  
  this->MRMLNodeIDToVTKNodeIDMap.erase(MRMLNodeID);  
}

//-----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
IDMapContainsMRMLNodeID(const char* MRMLNodeID)
{
  if (MRMLNodeID == NULL || strlen(MRMLNodeID) == 0)
    {
    vtkErrorMacro("Attempt to check null or blank mrml id in map");
    return 0;
    }
  return this->MRMLNodeIDToVTKNodeIDMap.count(MRMLNodeID) > 0;
}

//-----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
IDMapContainsVTKNodeID(vtkIdType id)
{
  return this->VTKNodeIDToMRMLNodeIDMap.count(id) > 0;
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PopulateTestingData()
{
  vtkDebugMacro("Begin populating test data");

  //
  // add some nodes to the hierarchy
  //
  vtkDebugMacro("Setting parameters for root node");
  double color[3];
  vtkIdType rootNodeID         = this->GetTreeRootNodeID();
  this->SetTreeNodeLabel(rootNodeID, "Root");
  this->SetTreeNodeName(rootNodeID, "Root");
  color[0] = 1.0; color[1] = 0.0; color[2] = 0.0;
  this->SetTreeNodeColor(rootNodeID, color);
  this->SetTreeNodeSpatialPriorWeight(rootNodeID, 0.5);
  this->SetTreeNodeClassProbability(rootNodeID, 0.5);
  this->SetTreeNodeAlpha(rootNodeID, 0.5);
  this->SetTreeNodePrintWeight(rootNodeID, 1);
  this->SetTreeNodeStoppingConditionEMType(rootNodeID, 1);
  this->SetTreeNodeStoppingConditionEMIterations(rootNodeID, 15);
  this->SetTreeNodeStoppingConditionEMValue(rootNodeID, 0.5);
  this->SetTreeNodeStoppingConditionMFAType(rootNodeID, 2);
  this->SetTreeNodeStoppingConditionMFAIterations(rootNodeID, 16);
  this->SetTreeNodeStoppingConditionMFAValue(rootNodeID, 0.6);

  vtkDebugMacro("Setting parameters for background node");
  vtkIdType backgroundNodeID   = this->AddTreeNode(rootNodeID);
  this->SetTreeNodeLabel(backgroundNodeID, "Background");
  this->SetTreeNodeName(backgroundNodeID, "Background");
  color[0] = 0.0; color[1] = 0.0; color[2] = 0.0;
  this->SetTreeNodeColor(backgroundNodeID, color);
  this->SetTreeNodeSpatialPriorWeight(backgroundNodeID, 0.4);
  this->SetTreeNodeClassProbability(backgroundNodeID, 0.4);
  this->SetTreeNodePrintWeight(backgroundNodeID, 1);

  vtkDebugMacro("Setting parameters for icc node");
  vtkIdType iccNodeID          = this->AddTreeNode(rootNodeID);
  this->SetTreeNodeLabel(iccNodeID, "ICC");
  this->SetTreeNodeName(iccNodeID, "ICC");
  color[0] = 0.0; color[1] = 1.0; color[2] = 0.0;
  this->SetTreeNodeColor(iccNodeID, color);
  this->SetTreeNodeSpatialPriorWeight(iccNodeID, 0.3);
  this->SetTreeNodeClassProbability(iccNodeID, 0.3);
  this->SetTreeNodeAlpha(iccNodeID, 0.3);
  this->SetTreeNodePrintWeight(iccNodeID, 1);
  this->SetTreeNodeStoppingConditionEMType(iccNodeID, 0);
  this->SetTreeNodeStoppingConditionEMIterations(iccNodeID, 13);
  this->SetTreeNodeStoppingConditionEMValue(iccNodeID, 0.3);
  this->SetTreeNodeStoppingConditionMFAType(iccNodeID, 1);
  this->SetTreeNodeStoppingConditionMFAIterations(iccNodeID, 14);
  this->SetTreeNodeStoppingConditionMFAValue(iccNodeID, 0.4);

  vtkDebugMacro("Setting parameters for grey matter node");
  vtkIdType greyMatterNodeID   = this->AddTreeNode(iccNodeID);
  this->SetTreeNodeLabel(greyMatterNodeID, "Grey Matter");
  this->SetTreeNodeName(greyMatterNodeID, "Grey Matter");
  color[0] = 0.0; color[1] = 1.0; color[2] = 1.0;
  this->SetTreeNodeColor(greyMatterNodeID, color);
  this->SetTreeNodeSpatialPriorWeight(greyMatterNodeID, 0.2);
  this->SetTreeNodeClassProbability(greyMatterNodeID, 0.2);
  this->SetTreeNodePrintWeight(greyMatterNodeID, 1);

  vtkDebugMacro("Setting parameters for white matter node");
  vtkIdType whiteMatterNodeID  = this->AddTreeNode(iccNodeID);
  this->SetTreeNodeLabel(whiteMatterNodeID, "White Matter");
  this->SetTreeNodeName(whiteMatterNodeID, "White Matter");
  color[0] = 1.0; color[1] = 1.0; color[2] = 0.0;
  this->SetTreeNodeColor(whiteMatterNodeID, color);
  this->SetTreeNodeSpatialPriorWeight(whiteMatterNodeID, 0.1);
  this->SetTreeNodeClassProbability(whiteMatterNodeID, 0.1);
  this->SetTreeNodePrintWeight(whiteMatterNodeID, 1);

  vtkDebugMacro("Setting parameters for csf node");
  vtkIdType csfNodeID  = this->AddTreeNode(iccNodeID);
  this->SetTreeNodeLabel(csfNodeID, "CSF");
  this->SetTreeNodeName(csfNodeID, "CSF");

  //
  // set registration parameters
  //
  vtkDebugMacro("Setting registration parameters");
  this->SetRegistrationAffineType(0);
  this->SetRegistrationDeformableType(0);
  this->SetRegistrationInterpolationType(1);

  //
  // set save parameters
  //
  vtkDebugMacro("Setting save parameters");
  this->SetSaveWorkingDirectory("/tmp");
  this->SetSaveTemplateFilename("/tmp/EMSTemplate.mrml");
  this->SetSaveTemplateAfterSegmentation(1);
  this->SetSaveIntermediateResults(1);
  this->SetSaveSurfaceModels(1);
  
  this->SetEnableMultithreading(1);

  vtkDebugMacro("Done populating test data");
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
GetListOfTreeNodeIDs(vtkIdType rootNodeID, vtkstd::vector<vtkIdType>& idList)
{
  // add this node
  idList.push_back(rootNodeID);

  // recursively add all child nodes
  unsigned int numberOfChildNodes = 
    this->GetTreeNodeNumberOfChildren(rootNodeID);
  for (unsigned int i = 0; i < numberOfChildNodes; ++i)
    {
    this->GetListOfTreeNodeIDs(this->GetTreeNodeChildNodeID(rootNodeID, i), 
                               idList);
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PropogateAdditionOfSelectedTargetImage()
{
  //
  // update (1) global parameters node, (2) channel weights in tree
  // parameters & (3) intensity distributions
  //
  int numberOfTargetChannels = 
    this->GetTargetNode()->GetNumberOfVolumes();

  // update global parameter node
  this->GetGlobalParametersNode()->
    AddTargetInputChannel();

  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    vtkMRMLEMSTreeNode* treeNode = this->GetTreeNode(*i);
    
    // update channel weights
    vtkMRMLEMSTreeParametersNode* parametersNode = 
      treeNode->GetParametersNode();
    parametersNode->
      AddTargetInputChannel();

    // update mean and covariance
    vtkMRMLEMSTreeParametersLeafNode* leafParametersNode = 
      parametersNode->GetLeafParametersNode();
    leafParametersNode->
      AddTargetInputChannel();

    // move to logic !!!
//     if (this->TreeInfoMap[*i].DistributionSpecificationMethod == 
//         vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample)
//       {
//       this->UpdateMeanAndCovarianceFromSample(*i);
//       }
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PropogateRemovalOfSelectedTargetImage(int imageIndex)
{
  std::cerr << "Removing target image: " << imageIndex << std::endl;

  //
  // update (1) global parameters node, (2) channel weights in tree
  // parameters & (3) intensity distributions
  //
  int numberOfTargetChannels = 
    this->GetTargetNode()->GetNumberOfVolumes();

  // update global parameter node
  this->GetGlobalParametersNode()->RemoveNthTargetInputChannel(imageIndex);

  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    vtkMRMLEMSTreeNode* treeNode = this->GetTreeNode(*i);
    
    // update channel weights
    vtkMRMLEMSTreeParametersNode* parametersNode = 
      treeNode->GetParametersNode();
    parametersNode->
      RemoveTargetInputChannel(imageIndex);

    // update mean and covariance
    vtkMRMLEMSTreeParametersLeafNode* leafParametersNode = 
      parametersNode->GetLeafParametersNode();
    leafParametersNode->
      RemoveTargetInputChannel(imageIndex);
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PropogateMovementOfSelectedTargetImage(int fromIndex, int toIndex)
{
  //
  // update (1) global parameters, (2) channel weights in tree
  // parameters & (3) intensity distributions in tree parameters
  //

  this->GetGlobalParametersNode()->
    MoveNthTargetInputChannel(fromIndex, toIndex);

  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    vtkMRMLEMSTreeNode* treeNode = this->GetTreeNode(*i);
    
    // update channel weights
    vtkMRMLEMSTreeParametersNode* parametersNode = 
      treeNode->GetParametersNode();
    parametersNode->
      MoveTargetInputChannel(fromIndex, toIndex);

    // update mean and covariance
    vtkMRMLEMSTreeParametersLeafNode* leafParametersNode = 
      parametersNode->GetLeafParametersNode();
    leafParametersNode->
      MoveTargetInputChannel(fromIndex, toIndex);
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  //
  // copy atlas related parameters to algorithm
  //
  vtkstd::cerr << "  atlas data..." << vtkstd::endl;
  this->CopyAtlasDataToSegmenter(segmenter);

  //
  // copy target related parameters to algorithm
  //
  vtkstd::cerr << "  target data..." << vtkstd::endl;
  this->CopyTargetDataToSegmenter(segmenter);

  //
  // copy global parameters to algorithm 
  //
  vtkstd::cerr << "  global data..." << vtkstd::endl;
  this->CopyGlobalDataToSegmenter(segmenter);

  //
  // copy tree base parameters to algorithm
  //
  vtkstd::cerr << "  tree data..." << vtkstd::endl;
  vtkImageEMLocalSuperClass* rootNode = vtkImageEMLocalSuperClass::New();
  this->CopyTreeDataToSegmenter(rootNode, this->GetTreeRootNodeID());
  segmenter->SetHeadClass(rootNode);
  rootNode->Delete();
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyAtlasDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  segmenter->
    SetNumberOfTrainingSamples(this->GetAtlasNumberOfTrainingSamples());
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyTargetDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  segmenter->SetNumInputImages(this->GetTargetNumberOfSelectedVolumes());

  // target images
  unsigned int numTargetImages = this->GetTargetNumberOfSelectedVolumes();
  for (unsigned int i = 0; i < numTargetImages; ++i)
    {
    vtkIdType volumeNodeID = this->GetTargetSelectedVolumeNthID(i);
    std::string mrmlID     = this->MapVTKNodeIDToMRMLNodeID(volumeNodeID);

    vtkDebugMacro("Setting target image " << i << " mrmlID=" 
                  << mrmlID.c_str());

    vtkImageData* imageData = 
      this->GetVolumeNode(this->GetTargetSelectedVolumeNthID(i))->
      GetImageData();

#ifdef NOT_DEFINED
    //
    // add normalization filter if normalization is enabled
    vtkMRMLEMSIntensityNormalizationParametersNode* normNode =
      this->GetTargetNode()->GetNthIntensityNormalizationParametersNode(i);

    if (normNode && normNode->GetEnabled())
      {
        vtkImageMeanIntensityNormalization* normFilter =
          vtkImageMeanIntensityNormalization::New();
        normFilter->SetNormValue(normNode->GetNormValue());
        normFilter->SetNormType(normNode->GetNormType());
        normFilter->SetInitialHistogramSmoothingWidth
          (normNode->GetInitialHistogramSmoothingWidth());
        normFilter->SetMaxHistogramSmoothingWidth
          (normNode->GetMaxHistogramSmoothingWidth());
        normFilter->SetRelativeMaxVoxelNum(normNode->GetRelativeMaxVoxelNum());
        normFilter->SetPrintInfo(normNode->GetPrintInfo());
        normFilter->SetInput(imageData);
        vtkstd::cerr << "Normalizing image..." << vtkstd::endl;
        normFilter->Update();

        segmenter->SetImageInput(i, normFilter->GetOutput());

        normFilter->Delete();
      }
#endif
    segmenter->SetImageInput(i, imageData);
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyGlobalDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  segmenter->SetDisableMultiThreading(!this->GetEnableMultithreading());
  segmenter->SetPrintDir(this->GetSaveWorkingDirectory());
  
  //
  // NB: In the algorithm code alpha is defined globally.  In this
  // logic, it is defined for each parent node.  For now copy alpha
  // from the root tree node. !!!todo!!!
  //
  vtkIdType rootNodeID = this->GetTreeRootNodeID();
  segmenter->SetAlpha(this->GetTreeNodeAlpha(rootNodeID));
                      
  //
  // NB: In the algorithm code smoothing widht and sigma parameters
  // are defined globally.  In this logic, they are defined for each
  // parent node.  For now copy parameters from the root tree
  // node. !!!todo!!!
  //
  segmenter->
    SetSmoothingWidth(this->GetTreeNodeSmoothingKernelWidth(rootNodeID));

  // type mismatch between logic and algorithm !!!todo!!!
  int intSigma = 
    vtkMath::Round(this->GetTreeNodeSmoothingKernelSigma(rootNodeID));
  segmenter->SetSmoothingSigma(intSigma);

  //
  // registration parameters
  //
  segmenter->
    SetRegistrationInterpolationType(this->GetRegistrationInterpolationType());
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyTreeDataToSegmenter(vtkImageEMLocalSuperClass* node, vtkIdType nodeID)
{
  // need this here because the vtkImageEM* classes don't use
  // virtual functions and so failed initializations lead to
  // memory errors
  node->SetNumInputImages(this->GetTargetNumberOfSelectedVolumes());

  // copy generic tree node data to segmenter
  this->CopyTreeGenericDataToSegmenter(node, nodeID);
  
  // copy parent specific tree node data to segmenter
  this->CopyTreeParentDataToSegmenter(node, nodeID);

  // add children
  unsigned int numChildren = this->GetTreeNodeNumberOfChildren(nodeID);
  for (unsigned int i = 0; i < numChildren; ++i)
    {
    vtkIdType childID = this->GetTreeNodeChildNodeID(nodeID, i);
    bool isLeaf = this->GetTreeNodeIsLeaf(childID);

    if (isLeaf)
      {
      vtkImageEMLocalClass* childNode = vtkImageEMLocalClass::New();
      // need this here because the vtkImageEM* classes don't use
      // virtual functions and so failed initializations lead to
      // memory errors
      childNode->SetNumInputImages(this->GetTargetNumberOfSelectedVolumes());
      this->CopyTreeGenericDataToSegmenter(childNode, childID);
      this->CopyTreeLeafDataToSegmenter(childNode, childID);
      node->AddSubClass(childNode, i);
      childNode->Delete();
      }
    else
      {
      vtkImageEMLocalSuperClass* childNode = vtkImageEMLocalSuperClass::New();
      this->CopyTreeDataToSegmenter(childNode, childID);
      node->AddSubClass(childNode, i);
      childNode->Delete();
      }
    }

  // update Markov matrices
  //!!todo!!! for now these are set to the identity
  for (unsigned int r = 0; r < numChildren; ++r)
    {
    for (unsigned int c = 0; c < numChildren; ++c)
      {
      // matrices are up, down, left, right, north, south (not nesc
      // right order)
      for (unsigned int whichMatrix = 0; whichMatrix < 6; ++whichMatrix)
        {
        double val = (r == c ? 1.0 : 0.0);
        node->SetMarkovMatrix(val, whichMatrix, c, r);
        }
      }
    }

  node->Update();
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyTreeGenericDataToSegmenter(vtkImageEMLocalGenericClass* node, 
                               vtkIdType nodeID)
{
  unsigned int numTargetImages = this->GetTargetNumberOfSelectedVolumes();
  
  int boundMin[3];
  this->GetSegmentationBoundaryMin(boundMin);
  node->SetSegmentationBoundaryMin(boundMin[0], boundMin[1], boundMin[2]);

  int boundMax[3];
  this->GetSegmentationBoundaryMax(boundMax);
  node->SetSegmentationBoundaryMax(boundMax[0], boundMax[1], boundMax[2]);
  
  node->SetProbDataWeight(this->GetTreeNodeSpatialPriorWeight(nodeID));

  node->SetTissueProbability(this->GetTreeNodeClassProbability(nodeID));

  node->SetPrintWeights(this->GetTreeNodePrintWeight(nodeID));

  // set target input channel weights
  for (unsigned int i = 0; i < numTargetImages; ++i)
    {
    node->SetInputChannelWeights(this->GetTreeNodeInputChannelWeight(nodeID, 
                                                                     i), i);
    }

  //
  // registration related data
  //
  //!!!bcd!!!

  //
  // set probability data
  //
  vtkIdType probVolumeID = this->GetTreeNodeSpatialPriorVolumeID(nodeID);
  if (probVolumeID != ERROR_NODE_VTKID)
    {
    vtkDebugMacro("Setting spatial prior: node=" 
                  << this->GetTreeNodeLabel(nodeID));
    vtkImageData* imageData = this->GetVolumeNode(probVolumeID)->
      GetImageData();
    node->SetProbDataPtr(imageData);
    }

  int exclude = this->GetTreeNodeExcludeFromIncompleteEStep(nodeID);
  node->SetExcludeFromIncompleteEStepFlag(exclude);
}


//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyTreeParentDataToSegmenter(vtkImageEMLocalSuperClass* node, 
                              vtkIdType nodeID)
{
  node->SetPrintFrequency (this->GetTreeNodePrintFrequency(nodeID));
  node->SetPrintBias      (this->GetTreeNodePrintBias(nodeID));
  node->SetPrintLabelMap  (this->GetTreeNodePrintLabelMap(nodeID));

  node->SetPrintEMLabelMapConvergence
    (this->GetTreeNodePrintEMLabelMapConvergence(nodeID));
  node->SetPrintEMWeightsConvergence
    (this->GetTreeNodePrintEMWeightsConvergence(nodeID));
  node->SetStopEMType     (this->GetTreeNodeStoppingConditionEMType(nodeID));
  node->SetStopEMValue    (this->GetTreeNodeStoppingConditionEMValue(nodeID));
  node->SetStopEMMaxIter
    (this->GetTreeNodeStoppingConditionEMIterations(nodeID));

  node->SetPrintMFALabelMapConvergence
    (this->GetTreeNodePrintMFALabelMapConvergence(nodeID));
  node->SetPrintMFAWeightsConvergence
    (this->GetTreeNodePrintMFAWeightsConvergence(nodeID));
  node->SetStopMFAType    (this->GetTreeNodeStoppingConditionMFAType(nodeID));
  node->SetStopMFAValue   (this->GetTreeNodeStoppingConditionMFAValue(nodeID));
  node->SetStopMFAMaxIter
    (this->GetTreeNodeStoppingConditionMFAIterations(nodeID));

  node->SetStopBiasCalculation
    (this->GetTreeNodeBiasCalculationMaxIterations(nodeID));

  node->SetPrintShapeSimularityMeasure(0);         // !!!bcd!!!

  node->SetPCAShapeModelType(0);                   // !!!bcd!!!

  node->SetRegistrationIndependentSubClassFlag(0); // !!!bcd!!!
  node->SetRegistrationType(0);                    // !!!bcd!!!

  node->SetGenerateBackgroundProbability
    (this->GetTreeNodeGenerateBackgroundProbability(nodeID));
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyTreeLeafDataToSegmenter(vtkImageEMLocalClass* node, 
                            vtkIdType nodeID)
{
  unsigned int numTargetImages = this->GetTargetNumberOfSelectedVolumes();

  // this label describes the output intensity value for this class in
  // the segmentation result
  node->SetLabel(this->GetTreeNodeIntensityLabel(nodeID));

  // set log mean and log covariance
  for (unsigned int r = 0; r < numTargetImages; ++r)
    {
    node->SetLogMu(this->GetTreeNodeDistributionLogMean(nodeID, r), r);

    for (unsigned int c = 0; c < numTargetImages; ++c)
      {
      node->SetLogCovariance(this->GetTreeNodeDistributionLogCovariance(nodeID,
                                                                        r, c), 
                             r, c);
      }
    }

  node->SetPrintQuality(this->GetTreeNodePrintQuality(nodeID));
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RegisterMRMLNodesWithScene()
{
  vtkMRMLEMSNode* emsNode = 
    vtkMRMLEMSNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsNode);
  emsNode->Delete();

  vtkMRMLEMSSegmenterNode* emsSegmenterNode = 
    vtkMRMLEMSSegmenterNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsSegmenterNode);
  emsSegmenterNode->Delete();

  vtkMRMLEMSTemplateNode* emsTemplateNode = 
    vtkMRMLEMSTemplateNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsTemplateNode);
  emsTemplateNode->Delete();

  vtkMRMLEMSIntensityNormalizationParametersNode* emsNormalizationNode = 
    vtkMRMLEMSIntensityNormalizationParametersNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsNormalizationNode);
  emsNormalizationNode->Delete();

  vtkMRMLEMSGlobalParametersNode* emsGlobalParametersNode = 
    vtkMRMLEMSGlobalParametersNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsGlobalParametersNode);
  emsGlobalParametersNode->Delete();

  vtkMRMLEMSTreeNode* emsTreeNode = 
    vtkMRMLEMSTreeNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsTreeNode);
  emsTreeNode->Delete();

  vtkMRMLEMSTreeParametersNode* emsTreeParametersNode = 
    vtkMRMLEMSTreeParametersNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsTreeParametersNode);
  emsTreeParametersNode->Delete();

  vtkMRMLEMSTreeParametersParentNode* emsTreeParametersParentNode = 
    vtkMRMLEMSTreeParametersParentNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsTreeParametersParentNode);
  emsTreeParametersParentNode->Delete();

  vtkMRMLEMSTreeParametersLeafNode* emsTreeParametersLeafNode = 
    vtkMRMLEMSTreeParametersLeafNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsTreeParametersLeafNode);
  emsTreeParametersLeafNode->Delete();

  vtkMRMLEMSAtlasNode* emsAtlasNode = 
    vtkMRMLEMSAtlasNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsAtlasNode);
  emsAtlasNode->Delete();  

  vtkMRMLEMSTargetNode* emsTargetNode = 
    vtkMRMLEMSTargetNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsTargetNode);
  emsTargetNode->Delete();  
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
UpdateMapsFromMRML()
{
  // create coppies of old maps
  VTKToMRMLMapType oldVTKToMRMLMap = this->VTKNodeIDToMRMLNodeIDMap;
  MRMLToVTKMapType oldMRMLtoVTKMap = this->MRMLNodeIDToVTKNodeIDMap;
  TreeInfoMapType  oldTreeInfoMap  = this->TreeInfoMap;

  // clear maps
  this->VTKNodeIDToMRMLNodeIDMap.clear();
  this->MRMLNodeIDToVTKNodeIDMap.clear();
  this->TreeInfoMap.clear();
  
  //
  // add tree nodes
  //
  int numTreeNodes = this->GetMRMLScene()->
    GetNumberOfNodesByClass("vtkMRMLEMSTreeNode");
  for (int i = 0; i < numTreeNodes; ++i)
    {
    vtkMRMLNode* node = this->GetMRMLScene()->
      GetNthNodeByClass(i, "vtkMRMLEMSTreeNode");

    if (node != NULL)
      {
      std::string mrmlID = node->GetID();
      
      if (oldMRMLtoVTKMap.count(mrmlID) > 0)
        {
        // copy the mapping to the new maps
        vtkIdType oldVTKID = oldMRMLtoVTKMap[mrmlID];
        this->VTKNodeIDToMRMLNodeIDMap[oldVTKID] = mrmlID;
        this->MRMLNodeIDToVTKNodeIDMap[mrmlID]   = oldVTKID;
        this->TreeInfoMap[oldVTKID] = oldTreeInfoMap[oldVTKID];
        }
      else
        {
        // add a new mapping
        vtkIdType newVTKID = this->GetNewVTKNodeID();
        this->VTKNodeIDToMRMLNodeIDMap[newVTKID] = mrmlID;
        this->MRMLNodeIDToVTKNodeIDMap[mrmlID]   = newVTKID;
        this->TreeInfoMap[newVTKID] = TreeInfo();
        }
      }
    }

  //
  // add volume nodes
  //
  int numVolumeNodes = this->GetMRMLScene()->
    GetNumberOfNodesByClass("vtkMRMLVolumeNode");
  for (int i = 0; i < numVolumeNodes; ++i)
    {
    vtkMRMLNode* node = this->GetMRMLScene()->
      GetNthNodeByClass(i, "vtkMRMLVolumeNode");

    if (node != NULL)
      {
      std::string mrmlID = node->GetID();
      
      if (oldMRMLtoVTKMap.count(mrmlID) > 0)
        {
        // copy the mapping to the new maps
        vtkIdType oldVTKID = oldMRMLtoVTKMap[mrmlID];
        this->VTKNodeIDToMRMLNodeIDMap[oldVTKID] = mrmlID;
        this->MRMLNodeIDToVTKNodeIDMap[mrmlID]   = oldVTKID;
        }
      else
        {
        // add a new mapping
        vtkIdType newVTKID = this->GetNewVTKNodeID();
        this->VTKNodeIDToMRMLNodeIDMap[newVTKID] = mrmlID;
        this->MRMLNodeIDToVTKNodeIDMap[mrmlID]   = newVTKID;
        }
      }
    }
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentMRMLManager::
CheckMRMLNodeStructure()
{
  //
  // check global attributes
  //

  // must have valid template builder node
  if (this->Node == NULL)
    {
    vtkErrorMacro("Template builder node is NULL.");
    return 0;
    }

  // check segmentor node
  vtkMRMLEMSSegmenterNode* segmentorNode = this->GetSegmenterNode();
  if (segmentorNode == NULL)
    {
    vtkErrorMacro("Segmenter node is NULL.");
    return 0;
    }

  // check target node
  vtkMRMLEMSTargetNode *targetNode = this->GetTargetNode();
  if (targetNode == NULL)
    {
    vtkErrorMacro("Target node is NULL.");
    return 0;
    }

  // check atlas node
  vtkMRMLEMSAtlasNode *atlasNode = this->GetAtlasNode();
  if (atlasNode == NULL)
    {
    vtkErrorMacro("Atlas node is NULL.");
    return 0;
    }
  
  // check output volume
  vtkMRMLScalarVolumeNode *outVolume = this->GetOutputVolumeNode();
  if (outVolume == NULL)
    {
    vtkErrorMacro("Output volume is NULL.");
    return 0;
    }

  // check template node
  vtkMRMLEMSTemplateNode* templateNode = this->GetTemplateNode();
  if (templateNode == NULL)
    {
    vtkErrorMacro("Template node is NULL.");
    return 0;
    }

  // check global parameters node
  vtkMRMLEMSGlobalParametersNode* globalParametersNode = 
    this->GetGlobalParametersNode();
  if (globalParametersNode == NULL)
    {
    vtkErrorMacro("Global parameters node is NULL.");
    return 0;
    }

  // check tree
  vtkMRMLEMSTreeNode* rootNode = GetTreeRootNode(); 
  if (rootNode == NULL)
    {
    vtkErrorMacro("Root node of tree is NULL.");
    return 0;
    }

  // check the tree recursively!!!

  // check that the number of target nodes is consistent
  int numTargetInputChannels = this->GetTargetNode()->GetNumberOfVolumes();
  if (this->GetGlobalParametersNode()->GetNumberOfTargetInputChannels() !=
      numTargetInputChannels)
    {
    vtkErrorMacro("Inconsistent number of input channles. Target="
                  << numTargetInputChannels
                  << " Global Parameters="
                  << this->GetGlobalParametersNode()->
                  GetNumberOfTargetInputChannels());
    return 0;    
    }
  
  // check the tree recursively!!!

  // check that all of the referenced volume nodes exist!!!

  // everything checks out
  return 1;
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentMRMLManager::
ConvertGUIEnumToAlgorithmEnumStoppingConditionType(int guiEnumValue)
{
  switch (guiEnumValue)
    {
    case (vtkEMSegmentMRMLManager::StoppingConditionIterations):
      return EMSEGMENT_STOP_FIXED;
    case (vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure):
      return EMSEGMENT_STOP_LABELMAP;
    case (vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure):
      return EMSEGMENT_STOP_WEIGHTS;
    default:
      vtkErrorMacro("Unknown stopping condition type: " << guiEnumValue);
      return -1;
    }
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentMRMLManager::
ConvertAlgorithmEnumToGUIEnumStoppingConditionType(int algEnumValue)
{
  switch (algEnumValue)
    {
    case (EMSEGMENT_STOP_FIXED):
      return vtkEMSegmentMRMLManager::StoppingConditionIterations;
    case (EMSEGMENT_STOP_LABELMAP):
      return vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure;
    case (EMSEGMENT_STOP_WEIGHTS):
      return vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure;
    default:
      vtkErrorMacro("Unknown stopping condition type: " << algEnumValue);
      return -1;
    }
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentMRMLManager::
ConvertGUIEnumToAlgorithmEnumInterpolationType(int guiEnumValue)
{
  switch (guiEnumValue)
    {
    case (vtkEMSegmentMRMLManager::InterpolationLinear):
      return EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR;
    case (vtkEMSegmentMRMLManager::InterpolationNearestNeighbor):
      return EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR;
    case (vtkEMSegmentMRMLManager::InterpolationCubic):
      // !!! not implemented
      vtkErrorMacro("Cubic interpolation not implemented: " << guiEnumValue);
      return -1;
    default:
      vtkErrorMacro("Unknown interpolation type: " << guiEnumValue);
      return -1;
    }
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentMRMLManager::
ConvertAlgorithmEnumToGUIEnumInterpolationType(int algEnumValue)
{
  switch (algEnumValue)
    {
    case (EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR):
      return vtkEMSegmentMRMLManager::InterpolationLinear;
    case (EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR):
      return vtkEMSegmentMRMLManager::InterpolationNearestNeighbor;
    default:
      vtkErrorMacro("Unknown interpolation type: " << algEnumValue);
      return -1;
    }
}

void
vtkEMSegmentMRMLManager::
PrintTree(vtkIdType rootID, vtkIndent indent)
{
  vtkstd::string mrmlID = this->MapVTKNodeIDToMRMLNodeID(rootID);
  vtkMRMLEMSTreeNode* rnode = this->GetTreeNode(rootID);
  const char* label = this->GetTreeNodeLabel(rootID);
  const char* name = this->GetTreeNodeName(rootID);
  vtkstd::cerr << indent << "Label: " << (label ? label : "(null)") 
               << vtkstd::endl;
  vtkstd::cerr << indent << "Name: " << (name ? name : "(null)") 
               << vtkstd::endl;
  vtkstd::cerr << indent << "ID: "    << rootID << " MRML ID: " 
               << rnode->GetID() << vtkstd::endl;
  vtkstd::cerr << indent << "Is Leaf: " << this->GetTreeNodeIsLeaf(rootID) 
               << vtkstd::endl;
  int numChildren = this->GetTreeNodeNumberOfChildren(rootID); 
  vtkstd::cerr << indent << "Num. Children: " << numChildren << vtkstd::endl;

  indent = indent.GetNextIndent();
  for (int i = 0; i < numChildren; ++i)
    {
    vtkIdType childID = this->GetTreeNodeChildNodeID(rootID, i);
    vtkstd::cerr << indent << "Child " << i << " (" << childID 
                 << ") of node " << rootID << vtkstd::endl;
    this->PrintTree(childID, indent);
    }
}
