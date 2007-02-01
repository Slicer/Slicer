#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

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

#include "vtkImageEMLocalSegmenter.h"
#include "vtkImageEMLocalSuperClass.h"

#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkMatrix4x4.h"
#include "vtkMath.h"

// needed to translate between enums
#include "EMLocalInterface.h"

#include <math.h>
#include <exception>


//----------------------------------------------------------------------------
vtkEMSegmentLogic* vtkEMSegmentLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkEMSegmentLogic");
  if(ret)
    {
    return (vtkEMSegmentLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkEMSegmentLogic;
}


//----------------------------------------------------------------------------
vtkEMSegmentLogic::vtkEMSegmentLogic()
{
  this->Node = NULL;
  this->ModuleName = NULL;

  this->ProgressCurrentAction = NULL;
  this->ProgressGlobalFractionCompleted = 0.0;
  this->ProgressCurrentFractionCompleted = 0.0;
  this->NextVTKNodeID = 1000;

  this->HideNodesFromEditors = false;
  //this->DebugOn();
}

//----------------------------------------------------------------------------
vtkEMSegmentLogic::~vtkEMSegmentLogic()
{
  vtkSetMRMLNodeMacro(this->Node, NULL);
  this->SetProgressCurrentAction(NULL);
  this->SetModuleName(NULL);
}

//----------------------------------------------------------------------------
void 
vtkEMSegmentLogic::
ProcessMRMLEvents(vtkObject* caller,
                  unsigned long event,
                  void* callData)
{
  vtkDebugMacro("vtkEMSegmentLogic::ProcessMRMLEvents: got an event " 
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
void vtkEMSegmentLogic::SetAndObserveNode(vtkMRMLEMSNode *n)
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
void vtkEMSegmentLogic::PrintSelf(ostream& os, vtkIndent indent)
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
vtkEMSegmentLogic::
GetTreeRootNodeID()
{
  vtkMRMLEMSTreeNode* rootNode = this->GetTreeRootNode();
  if (rootNode == NULL)
    {
    return 0;
    }

  return this->MapMRMLNodeIDToVTKNodeID(rootNode->GetID());
}

//----------------------------------------------------------------------------
int 
vtkEMSegmentLogic::
GetTreeNodeIsLeaf(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetNumberOfChildNodes() == 0;
}

//----------------------------------------------------------------------------
int 
vtkEMSegmentLogic::
GetTreeNodeNumberOfChildren(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    if (nodeID != 0)
      {
      vtkWarningMacro("Tree node is null for nodeID: " << nodeID);
      }
    return 0;
    }

  return n->GetNumberOfChildNodes();
}

//----------------------------------------------------------------------------
vtkIdType 
vtkEMSegmentLogic::
GetTreeNodeChildNodeID(vtkIdType parentNodeID, int childIndex)
{
  vtkMRMLEMSTreeNode* parentNode = this->GetTreeNode(parentNodeID);
  if (parentNode == NULL)
    {
    vtkErrorMacro("Parent tree node is null for nodeID: " << parentNodeID);
    return 0;
    }

  vtkMRMLEMSTreeNode* childNode = parentNode->GetNthChildNode(childIndex);
  if (childNode == NULL)
    {
    vtkErrorMacro("Child tree node is null for parent id / child number=" 
                  << parentNodeID << "/" << childIndex);
    return 0;
    }

  return this->MapMRMLNodeIDToVTKNodeID(childNode->GetID());
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentLogic::
GetTreeNodeParentNodeID(vtkIdType childNodeID)
{
  vtkMRMLEMSTreeNode* childNode = this->GetTreeNode(childNodeID);
  if (childNode == NULL)
    {
    vtkErrorMacro("Child tree node is null for nodeID: " << childNodeID);
    return 0;
    }

  vtkMRMLEMSTreeNode* parentNode = childNode->GetParentNode();
  if (parentNode == NULL)
    {
    return 0;
    }
  else
    {
    return this->MapMRMLNodeIDToVTKNodeID(parentNode->GetID());
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodeLabel(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    if (nodeID != 0)
      {
      vtkWarningMacro("Tree node is null for nodeID: " << nodeID);
      }
    return NULL;
    }
  return n->GetLabel();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
  if (method == vtkEMSegmentLogic::DistributionSpecificationManuallySample)
    {
    this->UpdateMeanAndCovarianceFromSample(nodeID);
    }
}

//----------------------------------------------------------------------------
double
vtkEMSegmentLogic::   
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodeDistributionNumberOfSamples(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("SampledPointsList not found for node: " << nodeID);
    return -1;
    }
  
  return this->TreeInfoMap[nodeID].SamplePoints.size();
}

//----------------------------------------------------------------------------
// send RAS coordinates, returns non-zero if point is indeed added
int
vtkEMSegmentLogic::
AddTreeNodeDistributionSamplePoint(vtkIdType nodeID, double xyz[3])
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  if (this->TreeInfoMap.count(nodeID) == 0)
    {
    vtkErrorMacro("SamplePointsList not found for node: " << nodeID);
    return -1;
    }

  vtksys_stl::vector<double> point(3);
  point[0] = xyz[0];
  point[1] = xyz[1];
  point[2] = xyz[2];

  this->TreeInfoMap[nodeID].SamplePoints.push_back(point);

  if (this->TreeInfoMap[nodeID].DistributionSpecificationMethod == 
      vtkEMSegmentLogic::DistributionSpecificationManuallySample)
    {
    this->UpdateMeanAndCovarianceFromSample(nodeID);
    }

  // for now: all points are added
  return 1;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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

  if (this->TreeInfoMap[nodeID].DistributionSpecificationMethod ==
      vtkEMSegmentLogic::DistributionSpecificationManuallySample)
    {
    this->UpdateMeanAndCovarianceFromSample(nodeID);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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

  if (this->TreeInfoMap[nodeID].DistributionSpecificationMethod ==
      vtkEMSegmentLogic::DistributionSpecificationManuallySample)
    {
    this->UpdateMeanAndCovarianceFromSample(nodeID);
    }
}


//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodePrintWeight(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetPrintWeights();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodePrintQuality(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetLeafParametersNode()->GetPrintQuality();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodeIntensityLabel(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetLeafParametersNode()->
    GetIntensityLabel();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodePrintFrequency(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintFrequency();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodePrintLabelMap(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintLabelMap();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodePrintEMLabelMapConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintEMLabelMapConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodePrintEMWeightsConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintEMWeightsConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodePrintMFALabelMapConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintMFALabelMapConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodePrintMFAWeightsConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetPrintMFAWeightsConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodeGenerateBackgroundProbability(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetParentParametersNode()->
    GetGenerateBackgroundProbability();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodeExcludeFromIncompleteEStep(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }

  return n->GetParametersNode()->GetExcludeFromIncompleteEStep();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodeStoppingConditionEMIterations(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }
  return n->GetParametersNode()->GetParentParametersNode()->
    GetStopEMMaxIterations();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodeStoppingConditionMFAIterations(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return -1;
    }
  return n->GetParametersNode()->GetParentParametersNode()->
    GetStopMFAMaxIterations();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetVolumeNthID(int n)
{
  vtkMRMLNode* node = this->GetMRMLScene()->
    GetNthNodeByClass(n, "vtkMRMLVolumeNode");

  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth volume in scene: " << n);
    return 0;
    }

  if (this->IDMapContainsMRMLNodeID(node->GetID()))
    {
    return this->MapMRMLNodeIDToVTKNodeID(node->GetID());
    }
  else
    {
    vtkErrorMacro("Volume MRML ID was not in map!" << node->GetID());
    return 0;
    }
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetTreeNodeSpatialPriorVolumeID(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }

  // get name of atlas volume from tree node
  char* atlasVolumeName = 
    n->GetParametersNode()->
    GetSpatialPriorVolumeName();

  if (atlasVolumeName == NULL || strlen(atlasVolumeName) == 0)
    {
    return 0;
    }

  // get MRML volume ID from atas node
  const char* mrmlVolumeNodeID = 
    this->GetGlobalParametersNode()->GetAtlasNode()->
    GetVolumeNodeIDByKey(atlasVolumeName);
  
  if (mrmlVolumeNodeID == NULL || strlen(atlasVolumeName) == 0)
    {
    vtkErrorMacro("MRMLID for prior volume is null; nodeID=" << nodeID);
    return 0;
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
    return 0;
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
  this->GetGlobalParametersNode()->GetAtlasNode()->
    AddVolume(priorVolumeName.c_str(), volumeMRMLID);

  // set name of atlas volume in tree node
  n->GetParametersNode()->SetSpatialPriorVolumeName(priorVolumeName.c_str());
}

//----------------------------------------------------------------------------
int
vtkEMSegmentLogic::
GetTargetNumberOfSelectedVolumes()
{
  
  if (this->GetGlobalParametersNode() &&
      this->GetGlobalParametersNode()->GetTargetNode())
    {
    return this->GetGlobalParametersNode()->GetTargetNode()->
      GetNumberOfVolumes();
    }
  else
    {
    if (this->Node != NULL)
      {
      vtkWarningMacro("Can't get number of target volumes while " \
                      "EMSNode is nonnull");
      }
    return 0;
    }
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentLogic::
GetTargetSelectedVolumeNthID(int n)
{
  const char* mrmlID = 
    this->GetGlobalParametersNode()->GetTargetNode()->GetNthVolumeNodeID(n);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Did not find nth target volume; n = " << n);
    return 0;
    }
  else if (!this->IDMapContainsMRMLNodeID(mrmlID))
    {
    vtkErrorMacro("Volume MRML ID was not in map!" << mrmlID);
    return 0;
    }
  else
    {
    // convert mrml id to vtk id
    return this->MapMRMLNodeIDToVTKNodeID(mrmlID);
    }
}
 
//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
  this->GetGlobalParametersNode()->GetTargetNode()->
    AddVolume(name.c_str(), mrmlID);
  
  this->PropogateChangeInNumberOfSelectedTargetImages();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
RemoveTargetSelectedVolume(vtkIdType volumeID)
{
  // map to MRML ID
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not map volume ID: " << volumeID);
    return;
    }

  // remove 
  this->GetGlobalParametersNode()->GetTargetNode()->
    RemoveVolumeByNodeID(mrmlID);

  this->PropogateChangeInNumberOfSelectedTargetImages();
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentLogic::
GetRegistrationAtlasVolumeID()
{
  if (!this->GetGlobalParametersNode())
    {
    return 0;
    }

  // the the name of the atlas image from the global parameters
  char* volumeName = this->GetGlobalParametersNode()->
    GetRegistrationAtlasVolumeKey();

  if (volumeName == NULL || strlen(volumeName) == 0)
    {
    return 0;
    }

  // get MRML ID of atlas from it's name
  const char* mrmlID = this->GetGlobalParametersNode()->GetAtlasNode()->
    GetVolumeNodeIDByKey(volumeName);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not find mrml ID for registration atlas volume.");
    return 0;
    }
  
  // convert mrml id to vtk id
  return this->MapMRMLNodeIDToVTKNodeID(mrmlID);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
  this->GetGlobalParametersNode()->GetAtlasNode()->
    AddVolume(registrationVolumeName.c_str(), mrmlID);

  this->GetGlobalParametersNode()->
    SetRegistrationAtlasVolumeKey(registrationVolumeName.c_str());
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentLogic::
GetRegistrationTargetVolumeID()
{
  if (this->GetGlobalParametersNode() == NULL)
    {
    return 0;
    }

  // the the name of the target image from the global parameters
  char* volumeName = this->GetGlobalParametersNode()->
    GetRegistrationTargetVolumeKey();
  if (volumeName == NULL || strlen(volumeName) == 0)
    {
    return 0;
    }

  // get MRML ID of target image from it's name
  const char* mrmlID = 
    this->GetGlobalParametersNode()->GetTargetNode()->
    GetVolumeNodeIDByKey(volumeName);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not find mrml ID for registration target volume.");
    return 0;
    }

  // convert mrml id to vtk id
  return this->MapMRMLNodeIDToVTKNodeID(mrmlID);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SetRegistrationTargetVolumeID(vtkIdType volumeID)
{
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not find mrml ID for volumeID: " << volumeID);
    }

  // get name for this image from target node
  const char* volumeKey = this->GetGlobalParametersNode()->GetTargetNode()->
    GetKeyByVolumeNodeID(mrmlID);
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
vtkEMSegmentLogic::
GetRegistrationAffineType()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetRegistrationAffineType() : 0;  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SetRegistrationAffineType(int affineType)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetRegistrationAffineType(affineType);
    }
}


//----------------------------------------------------------------------------
int
vtkEMSegmentLogic::
GetRegistrationDeformableType()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetRegistrationDeformableType() : 0;  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetRegistrationInterpolationType()
{
  int algType = this->GetGlobalParametersNode() ? 
    this->GetGlobalParametersNode()->GetRegistrationInterpolationType() : 0;  
  return this->ConvertAlgorithmEnumToGUIEnumInterpolationType(algType);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetSaveWorkingDirectory()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetWorkingDirectory() : NULL;  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SetSaveWorkingDirectory(const char* directory)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetWorkingDirectory(directory);  
    }
}


//----------------------------------------------------------------------------
const char*
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetSaveIntermediateResults()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetSaveIntermediateResults() : 0;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetSaveSurfaceModels()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetSaveSurfaceModels() : 0;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SetSaveSurfaceModels(int shouldSaveModels)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetSaveSurfaceModels(shouldSaveModels);  
    }
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
SaveTemplateNow()
{
  //
  // for now just save entire mrml scene, later it might be helpful to
  // decide which nodes are important and which are unimportant
  //
  
  // according to comments this should be "file://path/file.xml"
  // but that does not work

  if (this->Node == NULL)
    {
    vtkErrorMacro("Attempting to save scene but EMSNode is null");
    return;
    }
  vtksys_stl::string url = ""; 
  url += this->Node->GetTemplateFilename();
  this->GetMRMLScene()->Commit(url.c_str());
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
StartSegmentation()
{
  //
  // make sure we're ready to start
  //

  // find output volume
  if (!this->GetSegmenterNode())
    {
    vtkErrorMacro("Segmenter node is null---aborting segmentation.");
    return;
    }
  const char* outMRLMID = this->GetSegmenterNode()->GetOutputVolumeNodeID();
  vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(outMRLMID));
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found---aborting segmentation.");
    return;
    }

  //
  // Copy RASToIJK matrix, and other attributes from input to
  // output. Use first target volume as source for this data.
  //
  
  // get attributes from first target input volume
  const char* inMRLMID = this->GetGlobalParametersNode()->
    GetTargetNode()->GetNthVolumeNodeID(0);
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(inMRLMID));
  if (inVolume == NULL)
    {
    vtkErrorMacro("Can't get first target image.");
    return;
    }

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  outVolume->SetName("Segmentation_Result");

  //
  // create segmenter class
  //
  vtkImageEMLocalSegmenter* segmenter = vtkImageEMLocalSegmenter::New();
  if (segmenter == NULL)
    {
    vtkErrorMacro("Could not create vtkImageEMLocalSegmenter pointer");
    return;
    }

  //
  // copy mrml data to segmenter class
  //
  vtkstd::cerr << "[Start] Copying data to algorithm class..." << vtkstd::endl;
  this->CopyDataToSegmenter(segmenter);
  vtkstd::cerr << "[Done]  Copying data to algorithm class." << vtkstd::endl;
  vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
  vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
  vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
  segmenter->PrintSelf(vtkstd::cerr, 0);
  vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
  vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
  vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;

  //
  // start segmentation
  //
  try 
    {
    vtkstd::cerr << "[Start] Segmentation algorithm..." 
                 << vtkstd::endl;
    segmenter->Update();
    vtkstd::cerr << "[Done]  Segmentation algorithm." << vtkstd::endl;
    }
  catch (std::exception e)
    {
    vtkErrorMacro("Exception thrown during segmentation: " << e.what());
    }
  vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
  vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
  vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
  segmenter->PrintSelf(vtkstd::cerr, 0);
  vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
  vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
  vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;

  //
  // copy result to output volume
  //
  
  // set ouput of the filter to VolumeNode's ImageData
  // NB: this comment coppied from Gradient Anisotropic Dif. filter:
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  vtkImageData* image = vtkImageData::New(); 
  image->DeepCopy(segmenter->GetOutput());
  outVolume->SetAndObserveImageData(image);
  image->Delete();
  outVolume->SetModifiedSinceRead(1);

  //outVolume->
  //SetImageData(segmenter->GetOutput());

  //
  // clean up
  //
  segmenter->Delete();

  //
  // save template file if desired
  //
  if (this->GetSaveTemplateAfterSegmentation())
    {
    this->SaveTemplateNow();
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentLogic::
GetEnableMultithreading()
{
  return this->GetGlobalParametersNode() ?
    this->GetGlobalParametersNode()->GetMultithreadingEnabled() : 0;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SetEnableMultithreading(int isEnabled)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetMultithreadingEnabled(isEnabled);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetAtlasNumberOfTrainingSamples()
{
  if (this->GetGlobalParametersNode() &&
      this->GetGlobalParametersNode()->GetAtlasNode())
    {
    return this->GetGlobalParametersNode()->GetAtlasNode()->
      GetNumberOfTrainingSamples();
    }
  return 0;
}

// ????
//----------------------------------------------------------------------------
int
vtkEMSegmentLogic::
HasGlobalParametersNode()
{
  return this->GetGlobalParametersNode() ? 1 : 0;
}

//----------------------------------------------------------------------------
vtkMRMLEMSGlobalParametersNode*
vtkEMSegmentLogic::
GetGlobalParametersNode()
{
  vtkMRMLEMSTemplateNode* templateNode = this->GetTemplateNode();  
  if (templateNode == NULL)
    {
    if (this->Node != NULL)
      {
      vtkErrorMacro("Can't get global parametrs node but EMSNode is nonnull.");
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
vtkEMSegmentLogic::
GetTreeRootNode()
{
  vtkMRMLEMSTemplateNode* templateNode = this->GetTemplateNode();
  if (templateNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null Template node when EMSNode is nonnull.");
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
vtkEMSegmentLogic::
GetTreeNode(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* node = NULL;
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(nodeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    if (nodeID != 0)
      {
      vtkWarningMacro("Can't find tree node for id: " << nodeID);
      }
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
vtkEMSegmentLogic::
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
vtkMRMLEMSTemplateNode*
vtkEMSegmentLogic::
GetTemplateNode()
{
  vtkMRMLEMSSegmenterNode* segmenterNode = this->GetSegmenterNode();
  if (segmenterNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null Segmenter node when EMSNode is nonnull.");
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
GetNumberOfParameterSets()
{
  //
  // just the the get number of template bulider nodes loaded into the
  // slicer scene
  //
  return this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLEMSNode");
}

//----------------------------------------------------------------------------
const char*
vtkEMSegmentLogic::
GetNthParameterSetName(int n)
{
  vtkMRMLNode* node = this->GetMRMLScene()->
    GetNthNodeByClass(n, "vtkMRMLEMSNode");

  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth template builder node in scene: " << n);
    return 0;
    }
  return node->GetName();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SetLoadedParameterSetIndex(int n)
{
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
vtkEMSegmentLogic::
CreateAndObserveNewParameterSet()
{
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

  // add connections
  globalParametersNode->SetAtlasNodeID(atlasNode->GetID());
  globalParametersNode->SetTargetNodeID(targetNode->GetID());

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
  
  // add connections
  leafParametersNode->SetGlobalParametersNodeID(globalParametersNode->GetID());
  parentParametersNode->
    SetGlobalParametersNodeID(globalParametersNode->GetID());
  treeParametersNode->SetGlobalParametersNodeID(globalParametersNode->GetID());

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
  // this crashed the system during initial development
  templateBuilderNode->Delete();
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentLogic::
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
  leafParametersNode->
    SetGlobalParametersNodeID(this->GetGlobalParametersNode()->GetID());
  parentParametersNode->
    SetGlobalParametersNodeID(this->GetGlobalParametersNode()->GetID());
  treeParametersNode->
    SetGlobalParametersNodeID(this->GetGlobalParametersNode()->GetID());

  treeParametersNode->SetLeafParametersNodeID(leafParametersNode->GetID());
  treeParametersNode->SetParentParametersNodeID(parentParametersNode->GetID());

  // update memory
  leafParametersNode->SynchronizeNumberOfTargetInputChannels();
  treeParametersNode->SynchronizeNumberOfTargetInputChannels();

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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
MapMRMLNodeIDToVTKNodeID(const char* MRMLNodeID)
{
  bool contained = this->MRMLNodeIDToVTKNodeIDMap.count(MRMLNodeID);
  if (!contained)
    {
    vtkErrorMacro("mrml ID does not map to vtk ID: " << MRMLNodeID);
    return 0;
    }

  return this->MRMLNodeIDToVTKNodeIDMap[MRMLNodeID];
}

//-----------------------------------------------------------------------------
const char*
vtkEMSegmentLogic::
MapVTKNodeIDToMRMLNodeID(vtkIdType nodeID)
{
  bool contained = this->VTKNodeIDToMRMLNodeIDMap.count(nodeID);
  if (!contained)
    {
    if (nodeID != 0)
      {
      vtkWarningMacro("vtk ID does not map to mrml ID: " << nodeID);
      }
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
IDMapRemovePair(vtkIdType vtkID)
{
  this->MRMLNodeIDToVTKNodeIDMap.
    erase(this->VTKNodeIDToMRMLNodeIDMap[vtkID]);
  this->VTKNodeIDToMRMLNodeIDMap.erase(vtkID);
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
IDMapContainsVTKNodeID(vtkIdType id)
{
  return this->VTKNodeIDToMRMLNodeIDMap.count(id) > 0;
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
  this->SetProgressGlobalFractionCompleted(0.9);

  vtkDebugMacro("Done populating test data");
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SpecialTestingFunction()
{
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
PropogateChangeInNumberOfSelectedTargetImages()
{
  //
  // update (1) channel weights in tree parameters & (2) intensity
  // distributions
  //
  
  // iterate over nodes
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
    parametersNode->SynchronizeNumberOfTargetInputChannels();

    // copy from current input channels
    // !!!bcd!!!

    // update mean and covariance
    vtkMRMLEMSTreeParametersLeafNode* leafParametersNode = 
      parametersNode->GetLeafParametersNode();
    leafParametersNode->SynchronizeNumberOfTargetInputChannels();

    if (this->TreeInfoMap[*i].DistributionSpecificationMethod == 
        vtkEMSegmentLogic::DistributionSpecificationManuallySample)
      {
      this->UpdateMeanAndCovarianceFromSample(*i);
      }
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
UpdateMeanAndCovarianceFromSample(vtkIdType nodeID)
{
  unsigned int numVolumes = this->GetTargetNumberOfSelectedVolumes();
  unsigned int numPoints  = 
    this->GetTreeNodeDistributionNumberOfSamples(nodeID);
  unsigned int r, c, p;

  //
  // the default is mean 0, zero covariance
  //
  vtkstd::vector<double> logMean(numVolumes, 0.0);
  vtkstd::vector<vtkstd::vector<double> > 
    logCov(numVolumes, vtkstd::vector<double>(numVolumes, 0.0));

  if (numPoints > 0)
    {
    //
    // get all the intensities and compute the means
    //
    vtkstd::vector<vtkstd::vector<double> > 
      logSamples(numVolumes, vtkstd::vector<double>(numPoints, 0));
    
    for (unsigned int imageIndex = 0; imageIndex < numVolumes; ++imageIndex)
      {
      vtkIdType volumeID = this->GetTargetSelectedVolumeNthID(imageIndex);
      
      for (unsigned int sampleIndex = 0; sampleIndex < numPoints; 
           ++sampleIndex)
        {
        // we are interested in stats of log of intensities
        // copy Kilian, use log(i + 1)
        double logIntensity = 
          log(this->
              GetTreeNodeDistributionSampleIntensityValue(nodeID,
                                                          sampleIndex,
                                                          volumeID) + 1.0);

        logSamples[imageIndex][sampleIndex] = logIntensity;
        logMean[imageIndex] += logIntensity;
        }
      logMean[imageIndex] /= numPoints;
      }

    //
    // compute covariance
    //
    for (r = 0; r < numVolumes; ++r)
      {
      for (c = 0; c < numVolumes; ++c)
        {
        for (p = 0; p < numPoints; ++p)
          {
          // I don't want to change indentation for kwstyle---it is
          // easier to catch problems with this lined up vertically
          logCov[r][c] += 
            (logSamples[r][p] - logMean[r]) * 
            (logSamples[c][p] - logMean[c]);
          }
        // unbiased covariance
        logCov[r][c] /= numPoints - 1;
        }
      }
    }

  //
  // propogate data to mrml node
  //
  vtkMRMLEMSTreeParametersLeafNode* leafNode = 
    this->GetTreeNode(nodeID)->GetParametersNode()->GetLeafParametersNode();

  for (r = 0; r < numVolumes; ++r)
    {
    leafNode->SetLogMean(r, logMean[r]);
    
    for (c = 0; c < numVolumes; ++c)
      {
      leafNode->SetLogCovariance(r, c, logCov[r][c]);
      }
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  //
  // copy atlas related parameters to algorithm
  //
  this->CopyAtlasDataToSegmenter(segmenter);

  //
  // copy target related parameters to algorithm
  //
  this->CopyTargetDataToSegmenter(segmenter);

  //
  // copy global parameters to algorithm 
  //
  this->CopyGlobalDataToSegmenter(segmenter);

  //
  // copy tree base parameters to algorithm
  //
  vtkImageEMLocalSuperClass* rootNode = vtkImageEMLocalSuperClass::New();
  this->CopyTreeDataToSegmenter(rootNode, this->GetTreeRootNodeID());
  segmenter->SetHeadClass(rootNode);
  rootNode->Delete();
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyAtlasDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  segmenter->
    SetNumberOfTrainingSamples(this->GetAtlasNumberOfTrainingSamples());
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
    segmenter->SetImageInput(i, imageData);
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
  if (probVolumeID != 0)
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
vtkEMSegmentLogic::
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
  vtkMRMLEMSSegmenterNode* segmentorNode = 
    this->GetSegmenterNode();
  if (segmentorNode == NULL)
    {
    vtkErrorMacro("Segmenter node is NULL.");
    return 0;
    }

  // check output volume
  const char* outMRLMID = this->GetSegmenterNode()->GetOutputVolumeNodeID();
  if (outMRLMID == NULL)
    {
    vtkErrorMacro("No output volume found.");
    return 0;
    }
  vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(outMRLMID));
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id = " << outMRLMID);
    return 0;
    }

  // check template node
  vtkMRMLEMSTemplateNode* templateNode = 
    this->GetTemplateNode();
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

  // check that global parametrs has an associated target node
  vtkMRMLEMSTargetNode* globalParametersTargetNode = 
    globalParametersNode->GetTargetNode();
  if (globalParametersTargetNode == NULL)
    {
    vtkErrorMacro("Global parameters' target node is NULL.");
    return 0;
    } 
  // keep track of number of target input images
  vtkNotUsed(int numGlobalParametersTargetInputImages = 
             globalParametersTargetNode->GetNumberOfVolumes(););

  // check that global parametrs has an associated atlas node
  vtkMRMLEMSAtlasNode* globalParametersAtlasNode = 
    globalParametersNode->GetAtlasNode();
  if (globalParametersAtlasNode == NULL)
    {
    vtkErrorMacro("Global parameters' atlas node is NULL.");
    return 0;
    } 

  //
  // check tree
  //
  vtkMRMLEMSTreeNode* rootNode = GetTreeRootNode(); 
  if (rootNode == 0)
    {
    vtkErrorMacro("Root node of tree is NULL.");
    return 0;
    }

  // everything checks out
  return 1;
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentLogic::
ConvertGUIEnumToAlgorithmEnumStoppingConditionType(int guiEnumValue)
{
  switch (guiEnumValue)
    {
    case (vtkEMSegmentLogic::StoppingConditionIterations):
      return EMSEGMENT_STOP_FIXED;
    case (vtkEMSegmentLogic::StoppingConditionLabelMapMeasure):
      return EMSEGMENT_STOP_LABELMAP;
    case (vtkEMSegmentLogic::StoppingConditionWeightsMeasure):
      return EMSEGMENT_STOP_WEIGHTS;
    default:
      return -1;
    }
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentLogic::
ConvertAlgorithmEnumToGUIEnumStoppingConditionType(int algEnumValue)
{
  switch (algEnumValue)
    {
    case (EMSEGMENT_STOP_FIXED):
      return vtkEMSegmentLogic::StoppingConditionIterations;
    case (EMSEGMENT_STOP_LABELMAP):
      return vtkEMSegmentLogic::StoppingConditionLabelMapMeasure;
    case (EMSEGMENT_STOP_WEIGHTS):
      return vtkEMSegmentLogic::StoppingConditionWeightsMeasure;
    default:
      return -1;
    }
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentLogic::
ConvertGUIEnumToAlgorithmEnumInterpolationType(int guiEnumValue)
{
  switch (guiEnumValue)
    {
    case (vtkEMSegmentLogic::InterpolationLinear):
      return EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR;
    case (vtkEMSegmentLogic::InterpolationNearestNeighbor):
      return EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR;
    case (vtkEMSegmentLogic::InterpolationCubic):
      // !!! not implemented
      return -1;
    default:
      return -1;
    }
}

//-----------------------------------------------------------------------------
int 
vtkEMSegmentLogic::
ConvertAlgorithmEnumToGUIEnumInterpolationType(int algEnumValue)
{
  switch (algEnumValue)
    {
    case (EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR):
      return vtkEMSegmentLogic::InterpolationLinear;
    case (EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR):
      return vtkEMSegmentLogic::InterpolationNearestNeighbor;
    default:
      return -1;
    }
}

void
vtkEMSegmentLogic::
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
