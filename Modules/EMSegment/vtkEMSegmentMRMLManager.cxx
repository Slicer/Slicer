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
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSIntensityNormalizationParametersNode.h"
#include "vtkMRMLEMSClassInteractionMatrixNode.h"

#include "vtkMatrix4x4.h"
#include "vtkMath.h"

#include "vtkSlicerVolumesLogic.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

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
  this->MRMLScene = NULL;
  this->Node = NULL;
  this->NextVTKNodeID = 1000;
  this->HideNodesFromEditors = false;
  //this->DebugOn();
}

//----------------------------------------------------------------------------
vtkEMSegmentMRMLManager::~vtkEMSegmentMRMLManager()
{
  this->SetNode(NULL);
  this->SetMRMLScene(NULL);
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
      this->IDMapRemovePair(node->GetID());
      }
    else if (node->IsA("vtkMRMLVolumeNode"))
      {
      this->IDMapRemovePair(node->GetID());
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::SetNode(vtkMRMLEMSNode *n)
{
  vtkSetObjectBodyMacro(Node, vtkMRMLEMSNode, n);
  this->UpdateMapsFromMRML();  
  
  if (n != NULL)
  {
    int ok = this->CheckMRMLNodeStructure();
    if (!ok)
    {
      vtkErrorMacro("Incomplete or invalid MRML node structure.");
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

  // remove the reference from the old parent
  vtkMRMLEMSTreeNode* oldParentNode = childNode->GetParentNode();
  if (oldParentNode)
    {
    vtkIdType oldParentID = 
      this->MapMRMLNodeIDToVTKNodeID(oldParentNode->GetID());
    if (oldParentID == ERROR_NODE_VTKID)
      {
      vtkErrorMacro("Can't get old parent vtk id for node: " 
                    << newParentNodeID);
      return;    
      }

    int childIndex = oldParentNode->GetChildIndexByMRMLID(childNode->GetID());
    if (childIndex < 0)
      {
      vtkErrorMacro("ERROR: can't find child's index in old parent node.");
      }

    oldParentNode->RemoveNthChildNode(childIndex);
    }  

  // point the child to the new parent
  childNode->SetParentNodeID(parentNode->GetID());

  // point parent to this child node
  parentNode->AddChildNode(childNode->GetID());
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

  // remove reference to this node from it's parent node
  vtkMRMLEMSTreeNode* parentNode = node->GetParentNode();
  if (parentNode)
    {
    vtkIdType parentID = this->MapMRMLNodeIDToVTKNodeID(parentNode->GetID());
    if (parentID != ERROR_NODE_VTKID)
      {
      int childIndex = parentNode->GetChildIndexByMRMLID(node->GetID());

      if (childIndex < 0)
        {
        vtkErrorMacro("ERROR: can't find child's index in old parent node.");
        }

      parentNode->RemoveNthChildNode(childIndex);
      }
    }
 
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
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return -1;
    }
  return this->GetTreeParametersLeafNode(nodeID)->
    GetDistributionSpecificationMethod();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeDistributionSpecificationMethod(vtkIdType nodeID, 
                                           int method)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }
  this->GetTreeParametersLeafNode(nodeID)->
    SetDistributionSpecificationMethod(method);

  if (this->GetTreeParametersLeafNode(nodeID)->
      GetDistributionSpecificationMethod() == 
      vtkMRMLEMSTreeParametersLeafNode::
      DistributionSpecificationManuallySample)
    {
    this->UpdateIntensityDistributionFromSample(nodeID);
    }
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::   
GetTreeNodeDistributionLogMean(vtkIdType nodeID, 
                               int volumeNumber)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }
  return this->GetTreeParametersLeafNode(nodeID)->GetLogMean(volumeNumber);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeDistributionLogMean(vtkIdType nodeID, 
                               int volumeNumber, 
                               double value)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }
  this->GetTreeParametersLeafNode(nodeID)->SetLogMean(volumeNumber, value);
}

//----------------------------------------------------------------------------
double   
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionLogCovariance(vtkIdType nodeID, 
                                     int rowIndex,
                                     int columnIndex)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }
  return this->GetTreeParametersLeafNode(nodeID)->
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
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }
  this->GetTreeParametersLeafNode(nodeID)->
    SetLogCovariance(rowIndex, columnIndex, value);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionNumberOfSamples(vtkIdType nodeID)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }
  return
    this->GetTreeParametersLeafNode(nodeID)->GetNumberOfSamplePoints();
}

//----------------------------------------------------------------------------
// send RAS coordinates, returns non-zero if point is indeed added
int
vtkEMSegmentMRMLManager::
AddTreeNodeDistributionSamplePoint(vtkIdType nodeID, double xyz[3])
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }

  this->GetTreeParametersLeafNode(nodeID)->AddSamplePoint(xyz);

  if (this->GetTreeParametersLeafNode(nodeID)->
      GetDistributionSpecificationMethod() == 
      vtkMRMLEMSTreeParametersLeafNode::
      DistributionSpecificationManuallySample)
    {
    this->UpdateIntensityDistributionFromSample(nodeID);
    }

  return 1;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RemoveTreeNodeDistributionSamplePoint(vtkIdType nodeID, int sampleNumber)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }
  this->GetTreeParametersLeafNode(nodeID)->RemoveNthSamplePoint(sampleNumber); 

  if (this->GetTreeParametersLeafNode(nodeID)->
      GetDistributionSpecificationMethod() == 
      vtkMRMLEMSTreeParametersLeafNode::
      DistributionSpecificationManuallySample)
    {
    this->UpdateIntensityDistributionFromSample(nodeID);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RemoveAllTreeNodeDistributionSamplePoints(vtkIdType nodeID)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }
  this->GetTreeParametersLeafNode(nodeID)->ClearSamplePoints();  

  if (this->GetTreeParametersLeafNode(nodeID)->
      GetDistributionSpecificationMethod() == 
      vtkMRMLEMSTreeParametersLeafNode::
      DistributionSpecificationManuallySample)
    {
    this->UpdateIntensityDistributionFromSample(nodeID);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionSamplePoint(vtkIdType nodeID, int sampleNumber,
                                   double xyz[3])
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }
  this->GetTreeParametersLeafNode(nodeID)->GetNthSamplePoint(sampleNumber,xyz);
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionSampleIntensityValue(vtkIdType nodeID, 
                                            int sampleNumber, 
                                            vtkIdType imageID)
{
  // get sample point
  double xyz[3];
  this->GetTreeNodeDistributionSamplePoint(nodeID, sampleNumber, xyz);
  
  // get volume
  vtkMRMLVolumeNode* volumeNode = this->GetVolumeNode(imageID);
  if (volumeNode == NULL)
    {
    vtkErrorMacro("Volume node is null for id: " << imageID);
    return 0;
    }

  // convert from RAS to IJK coordinates
  double rasPoint[4] = { xyz[0], xyz[1], xyz[2], 1.0 };
  double ijkPoint[4];
  vtkMatrix4x4* rasToijk = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(rasToijk);
  rasToijk->MultiplyPoint(rasPoint, ijkPoint);
  rasToijk->Delete();

  // get intensity value
  vtkImageData* imageData = volumeNode->GetImageData();
  double intensityValue = imageData->
    GetScalarComponentAsDouble(static_cast<int>(vtkMath::Round(ijkPoint[0])), 
                               static_cast<int>(vtkMath::Round(ijkPoint[1])), 
                               static_cast<int>(vtkMath::Round(ijkPoint[2])),
                               0);
  return intensityValue;
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
UpdateIntensityDistributions()
{
  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    if (this->GetTreeParametersLeafNode(*i)->
        GetDistributionSpecificationMethod() == 
        vtkMRMLEMSTreeParametersLeafNode::
        DistributionSpecificationManuallySample)
      {
      this->UpdateIntensityDistributionFromSample(*i);
      }
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
ChangeTreeNodeDistributionsFromManualSamplingToManual()
{
  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    if (this->GetTreeParametersLeafNode(*i)->
        GetDistributionSpecificationMethod() == 
        vtkMRMLEMSTreeParametersLeafNode::
        DistributionSpecificationManuallySample)
      {
      this->SetTreeNodeDistributionSpecificationMethod
        (*i, DistributionSpecificationManual);
      }
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
UpdateIntensityDistributionFromSample(vtkIdType nodeID)
{
  // get working node @@@
  vtkMRMLEMSTargetNode* workingTarget = 
    this->GetWorkingDataNode()->GetInputTargetNode();
  if (this->GetWorkingDataNode()->GetNormalizedTargetNode() &&
      this->GetWorkingDataNode()->GetNormalizedTargetNodeIsValid())
    {
    workingTarget = this->GetWorkingDataNode()->GetNormalizedTargetNode();
    }
  if (this->GetWorkingDataNode()->GetAlignedTargetNode() &&
      this->GetWorkingDataNode()->GetAlignedTargetNodeIsValid())
    {
    workingTarget = this->GetWorkingDataNode()->GetAlignedTargetNode();
    }

  unsigned int numTargetImages = workingTarget->GetNumberOfVolumes();

  unsigned int numPoints  = 
    this->GetTreeNodeDistributionNumberOfSamples(nodeID);
  unsigned int r, c, p;

  //
  // the default is mean 0, zero covariance
  //
  vtkstd::vector<double> logMean(numTargetImages, 0.0);
  vtkstd::vector<vtkstd::vector<double> > 
    logCov(numTargetImages, vtkstd::vector<double>(numTargetImages, 0.0));

  if (numPoints > 0)
    {
    //
    // get all the intensities and compute the means
    //
    vtkstd::vector<vtkstd::vector<double> > 
      logSamples(numTargetImages, vtkstd::vector<double>(numPoints, 0));
    
    for (unsigned int imageIndex = 0; imageIndex < numTargetImages; 
         ++imageIndex)
      {
      vtkstd::string mrmlID = workingTarget->GetNthVolumeNodeID(imageIndex);
      vtkIdType volumeID = this->MapMRMLNodeIDToVTKNodeID(mrmlID.c_str());
      
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
    for (r = 0; r < numTargetImages; ++r)
      {
      for (c = 0; c < numTargetImages; ++c)
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
    this->
    GetTreeNode(nodeID)->GetParametersNode()->GetLeafParametersNode();

  for (r = 0; r < numTargetImages; ++r)
    {
    leafNode->SetLogMean(r, logMean[r]);
    
    for (c = 0; c < numTargetImages; ++c)
      {
      leafNode->SetLogCovariance(r, c, logCov[r][c]);
      }
    }
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
GetTreeNodeClassInteraction(vtkIdType nodeID, 
                            int direction,
                            int rowIndex,
                            int columnIndex)
{
  vtkMRMLEMSClassInteractionMatrixNode* node = 
    this->GetTreeClassInteractionNode(nodeID);
  if (node == NULL)
    {
    vtkErrorMacro("Class interaction node is null for nodeID: " << nodeID);
    return 0;
    }
  return node->GetClassInteraction(direction, rowIndex, columnIndex);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeClassInteraction(vtkIdType nodeID, 
                            int direction,
                            int rowIndex, 
                            int columnIndex,
                            double value)
{
  vtkMRMLEMSClassInteractionMatrixNode* node = 
    this->GetTreeClassInteractionNode(nodeID);
  if (node == NULL)
    {
    vtkErrorMacro("Class interaction node is null for nodeID: " << nodeID);
    return;
    }
  node->SetClassInteraction(direction, rowIndex, columnIndex, value);
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
GetTreeNodeChildrenSumClassProbability(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID);
    return 0;
    }
  double sumOfProbabilities = 0;
  int numChildren = this->GetTreeNodeNumberOfChildren(nodeID);
  for (int childIndex = 0; childIndex < numChildren; ++childIndex)
    {
    vtkIdType childID = this->GetTreeNodeChildNodeID(nodeID, childIndex);
    sumOfProbabilities += this->GetTreeNodeClassProbability(childID);
    }
  return sumOfProbabilities;
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetTreeNodeFirstIDWithChildProbabilityError()
{
  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    if (!this->GetTreeNodeIsLeaf(*i) && 
        (this->GetTreeNodeChildrenSumClassProbability(*i) != 1.0))
      {
      return *i;
      }
    }
  return -1;
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
  return n->GetParametersNode()->GetParentParametersNode()->GetStopEMType();
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
  n->GetParametersNode()->GetParentParametersNode()->
    SetStopEMType(conditionType);
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
  return n->GetParametersNode()->GetParentParametersNode()->GetStopMFAType();
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
  n->GetParametersNode()->GetParentParametersNode()->
    SetStopMFAType(conditionType);
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
    this->GetAtlasInputNode()->GetVolumeNodeIDByKey(atlasVolumeName);
  
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

  if (volumeID == -1)
    {
    n->GetParametersNode()->SetSpatialPriorVolumeName(NULL);
    }
  else
    {
    // map volume id to MRML ID
    const char* volumeMRMLID = MapVTKNodeIDToMRMLNodeID(volumeID);
    if (volumeMRMLID == NULL || strlen(volumeMRMLID) == 0)
      {
      vtkErrorMacro("Could not map volume ID: " << volumeID);
      return;
      }
    
    // use tree node label (or mrml id if label is not specified)
    vtksys_stl::string priorVolumeName;
    priorVolumeName = n->GetID();
    
    // add key value pair to atlas
    this->GetAtlasInputNode()->AddVolume(priorVolumeName.c_str(), volumeMRMLID);

    // set name of atlas volume in tree node
    n->GetParametersNode()->SetSpatialPriorVolumeName(priorVolumeName.c_str());
    }

  // aligned atlas is no longer valid
  this->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTargetNumberOfSelectedVolumes()
{
  if (this->GetTargetInputNode())
    {
    return this->GetTargetInputNode()->GetNumberOfVolumes();
    }
  else
    {
    if (this->Node != NULL)
      {
      vtkWarningMacro("Can't get number of target volumes but " \
                      "EMSNode is nonnull");
      }
    return 0;
    }
}

//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetTargetSelectedVolumeNthID(int n)
{
  const char* mrmlID = 
    this->GetTargetInputNode()->GetNthVolumeNodeID(n);
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
  if (!this->GetTargetInputNode())
  {
    vtkWarningMacro("Can't access target node.");
    return NULL;
  }
  return
    this->GetTargetInputNode()->GetNthVolumeNodeID(n);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
ResetTargetSelectedVolumes(const std::vector<vtkIdType>& volumeIDs)
{
  int targetOldNumImages = this->GetTargetInputNode()->GetNumberOfVolumes();
  
  //
  // remove the old volumes from the target node and add the new volumes
  this->GetTargetInputNode()->RemoveAllVolumes();
  for (unsigned int i = 0; i < volumeIDs.size(); ++i)
    {
    vtkMRMLVolumeNode* volumeNode = this->GetVolumeNode(volumeIDs[i]);
    if (volumeNode == NULL)
      {
      vtkErrorMacro("Invalid volume ID: " << volumeIDs[i]);
      return;
      }

    vtkstd::string name = volumeNode->GetName();
    if (name.empty())
      {
      name = volumeNode->GetID();
      }
    this->GetTargetInputNode()->AddVolume(name.c_str(), volumeNode->GetID());
    }

  //
  // propogate change if the number of channels is different
  int targetNewNumImages = this->GetTargetInputNode()->GetNumberOfVolumes();

  std::cerr << "Old number of images: " << targetOldNumImages << std::endl;
  std::cerr << "New number of images: " << targetNewNumImages << std::endl;

  if (targetNewNumImages > targetOldNumImages)
    {
    int numAddedImages = targetNewNumImages - targetOldNumImages;
    for (int i = 0; i < numAddedImages; ++i)
      {
      this->PropogateAdditionOfSelectedTargetImage();
      }
    }
  else if (targetNewNumImages < targetOldNumImages)
    {
    int numRemovedImages = targetOldNumImages - targetNewNumImages;
    for (int i = 0; i < numRemovedImages; ++i)
      {
      std::cerr << "removing an image: " << targetOldNumImages-1-i 
                << std::endl;
      this->PropogateRemovalOfSelectedTargetImage(targetOldNumImages-1-i);
      }
    }

  // normalized and aligned targets are no longer valid
  this->GetWorkingDataNode()->SetNormalizedTargetNodeIsValid(0);
  this->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);

  // if someting was added or removed, or even if the order may have
  // changed, need to update distros
  this->UpdateIntensityDistributions();
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
  vtkstd::string name = volumeNode->GetName() ? volumeNode->GetName() : "";
  if (name.empty())
    {
    name = volumeNode->GetID();
    }

  // set volume name and ID in map
  this->GetTargetInputNode()->AddVolume(name.c_str(), mrmlID);

  // normalized and aligned targets are no longer valid
  this->GetWorkingDataNode()->SetNormalizedTargetNodeIsValid(0);
  this->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);

  // propogate change to parameters nodes
  this->PropogateAdditionOfSelectedTargetImage();
  this->UpdateIntensityDistributions();
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
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // remove from target
  this->GetTargetInputNode()->RemoveNthVolume(imageIndex);

  // normalized and aligned targets are no longer valid
  this->GetWorkingDataNode()->SetNormalizedTargetNodeIsValid(0);
  this->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);

  // propogate change to parameters nodes
  this->PropogateRemovalOfSelectedTargetImage(imageIndex);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
MoveNthTargetSelectedVolume(int fromIndex, int toIndex)
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
  this->GetTargetInputNode()->MoveNthVolume(fromIndex, toIndex);

  // normalized and aligned targets are no longer valid
  this->GetWorkingDataNode()->SetNormalizedTargetNodeIsValid(0);
  this->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);

  // propogate change to parameters nodes
  this->PropogateMovementOfSelectedTargetImage(fromIndex, toIndex);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
MoveTargetSelectedVolume(vtkIdType volumeID, int toIndex)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }
  this->MoveNthTargetSelectedVolume(imageIndex, toIndex);
}

//----------------------------------------------------------------------------
bool
vtkEMSegmentMRMLManager::
DoTargetAndAtlasDataTypesMatch()
{
  vtkMRMLEMSTargetNode* targetNode = 
    this->GetWorkingDataNode()->GetInputTargetNode();
  vtkMRMLEMSAtlasNode* atlasNode = 
    this->GetWorkingDataNode()->GetInputAtlasNode();  

  if (targetNode == NULL || atlasNode == NULL)
    {
    std::cerr << "Target or atlas node is null!" << std::endl;
    return false;
    }

  if (targetNode->GetNumberOfVolumes() == 0)
    {
    std::cerr << "Target node is empty!" << std::endl;
    return (atlasNode->GetNumberOfVolumes() == 0);
    }

  int standardScalarDataType = 
    targetNode->GetNthVolumeNode(0)->GetImageData()->GetScalarType();

  for (int i = 1; i < targetNode->GetNumberOfVolumes(); ++i)
    {
    int currentScalarDataType =
      targetNode->GetNthVolumeNode(i)->GetImageData()->GetScalarType();      
    if (currentScalarDataType != standardScalarDataType)
      {
      std::cerr << "Target volume " << i << ": scalar type does not match!" 
                << std::endl;
      return false;
      }
    }

  for (int i = 0; i < atlasNode->GetNumberOfVolumes(); ++i)
    {
    int currentScalarDataType =
      atlasNode->GetNthVolumeNode(i)->GetImageData()->GetScalarType();      
    if (currentScalarDataType != standardScalarDataType)
      {
      std::cerr << "Atlas volume " << i << ": scalar type does not match!" 
                << std::endl;
      return false;
      }
    }

  return true;
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
SetTargetVolumeIntensityNormalizationToDefaultT1SPGR(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationToDefaultT1SPGR(imageIndex);
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
SetTargetVolumeIntensityNormalizationToDefaultT2(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationToDefaultT2(imageIndex);
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
SetTargetVolumeIntensityNormalizationToDefaultT2_2(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationToDefaultT2_2(imageIndex);
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
void
vtkEMSegmentMRMLManager::
SetTargetVolumeIntensityNormalizationNormValue(vtkIdType volumeID, double d)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationNormValue(imageIndex, d);
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
double
vtkEMSegmentMRMLManager::
GetTargetVolumeIntensityNormalizationNormValue(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return 0;
    }

  // get parameters
  return this->GetNthTargetVolumeIntensityNormalizationNormValue(imageIndex);
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
void
vtkEMSegmentMRMLManager::
SetTargetVolumeIntensityNormalizationNormType(vtkIdType volumeID, int t)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationNormType(imageIndex, t);
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
int
vtkEMSegmentMRMLManager::
GetTargetVolumeIntensityNormalizationNormType(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return 0;
    }

  // get parameters
  return this->GetNthTargetVolumeIntensityNormalizationNormType(imageIndex);
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
void
vtkEMSegmentMRMLManager::
SetTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(vtkIdType volumeID, int t)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(imageIndex, t);
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
int
vtkEMSegmentMRMLManager::
GetTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return 0;
    }

  // get parameters
  return this->GetNthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(imageIndex);
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
void
vtkEMSegmentMRMLManager::
SetTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(vtkIdType volumeID, int t)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(imageIndex, t);
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

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return 0;
    }

  // get parameters
  return this->GetNthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(imageIndex);
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
void
vtkEMSegmentMRMLManager::
SetTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(vtkIdType volumeID, float f)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(imageIndex,
                                                                    f);
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
float
vtkEMSegmentMRMLManager::
GetTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return 0;
    }

  // get parameters
  return this->GetNthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(imageIndex);
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
void
vtkEMSegmentMRMLManager::
SetTargetVolumeIntensityNormalizationPrintInfo(vtkIdType volumeID, int t)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationPrintInfo(imageIndex, t);
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
int
vtkEMSegmentMRMLManager::
GetTargetVolumeIntensityNormalizationPrintInfo(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return 0;
    }

  // get parameters
  return this->GetNthTargetVolumeIntensityNormalizationPrintInfo(imageIndex);
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
void
vtkEMSegmentMRMLManager::
SetTargetVolumeIntensityNormalizationEnabled(vtkIdType volumeID, int t)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return;
    }

  // set parameters
  this->SetNthTargetVolumeIntensityNormalizationEnabled(imageIndex, t);
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
int
vtkEMSegmentMRMLManager::
GetTargetVolumeIntensityNormalizationEnabled(vtkIdType volumeID)
{
  // get this image's index in the target list
  int imageIndex = this->GetTargetVolumeIndex(volumeID);
  if (imageIndex < 0)
    {
    vtkErrorMacro("Volume not present in target: " << volumeID);
    return 0;
    }

  // get parameters
  return this->GetNthTargetVolumeIntensityNormalizationEnabled(imageIndex);
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
  const char* mrmlID = this->GetAtlasInputNode()->
    GetVolumeNodeIDByKey(volumeName);
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
  this->GetAtlasInputNode()->AddVolume(registrationVolumeName.c_str(), mrmlID);

  this->GetGlobalParametersNode()->
    SetRegistrationAtlasVolumeKey(registrationVolumeName.c_str());
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
  return this->GetGlobalParametersNode() ? 
    this->GetGlobalParametersNode()->GetRegistrationInterpolationType() : 0;  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetRegistrationInterpolationType(int interpolationType)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetRegistrationInterpolationType(interpolationType);  
    }
}

//----------------------------------------------------------------------------  
int
vtkEMSegmentMRMLManager::
GetEnableTargetToTargetRegistration()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->
    GetEnableTargetToTargetRegistration() : 0;
}

//----------------------------------------------------------------------------  
void
vtkEMSegmentMRMLManager::
SetEnableTargetToTargetRegistration(int enable)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->
      SetEnableTargetToTargetRegistration(enable);
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

// !!!bcdchecktoken^-!!!

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSaveTemplateFilename(const char* file)
{
  if (this->Node)
    {
    this->Node->SetTemplateFilename(file);
    }
  else
    {
    vtkErrorMacro("Attempt to access null EM node.");
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetSaveTemplateAfterSegmentation()
{
  if (this->Node)
    {
    return this->Node->GetSaveTemplateAfterSegmentation();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSaveTemplateAfterSegmentation(int shouldSave)
{
  if (this->Node)
    {
    this->Node->SetSaveTemplateAfterSegmentation(shouldSave);
    }
  else
    {
    vtkErrorMacro("Attempt to access null EM node.");
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetSaveIntermediateResults()
{
  if (this->GetGlobalParametersNode())
    {
    return this->GetGlobalParametersNode()->GetSaveIntermediateResults();
    }
  else
    {
    return 0;
    }
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
  else
  {
    vtkErrorMacro("Attempt to access null global parameter node.");
  }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetSaveSurfaceModels()
{
  if (this->GetGlobalParametersNode())
    {
    return this->GetGlobalParametersNode()->GetSaveSurfaceModels();  
    }
  else
    {
    return 0;
    }
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
  else
    {
    vtkErrorMacro("Attempt to access null global parameter node.");
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
    return;
    }
  else
    {
    this->GetSegmenterNode()->SetOutputVolumeNodeID(mrmlID);
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
    this->GetGlobalParametersNode()->SetMultithreadingEnabled(isEnabled);
    }
  else
    {
    vtkErrorMacro("Attempt to access null global parameter node.");
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetUpdateIntermediateData()
{
  if (this->GetGlobalParametersNode())
    {
    return this->GetGlobalParametersNode()->GetUpdateIntermediateData();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetUpdateIntermediateData(int shouldUpdate)
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetUpdateIntermediateData(shouldUpdate);
    }
  else
    {
    vtkErrorMacro("Attempt to access null global parameter node.");
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
GetSegmentationBoundaryMin(int minPoint[3])
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->GetSegmentationBoundaryMin(minPoint);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSegmentationBoundaryMin(int minPoint[3])
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetSegmentationBoundaryMin(minPoint);
    }
  else
    {
    vtkErrorMacro("Attempt to access null global parameter node.");
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
GetSegmentationBoundaryMax(int maxPoint[3])
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->GetSegmentationBoundaryMax(maxPoint);
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSegmentationBoundaryMax(int maxPoint[3])
{
  if (this->GetGlobalParametersNode())
    {
    this->GetGlobalParametersNode()->SetSegmentationBoundaryMax(maxPoint);
    }
  else
    {
    vtkErrorMacro("Attempt to access null global parameter node.");
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetAtlasNumberOfTrainingSamples()
{
  if (this->GetAtlasInputNode())
    {
    return this->GetAtlasInputNode()->GetNumberOfTrainingSamples();
    }
  else
    {
    return 0;
    }
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
vtkMRMLEMSTreeParametersNode*
vtkEMSegmentMRMLManager::
GetTreeParametersNode(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* node = this->GetTreeNode(nodeID);
  if (node == NULL)
    {
    vtkWarningMacro("Tree node is null for node id: " << nodeID);
    return NULL;
    }
  return node->GetParametersNode();
}

//----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersLeafNode*
vtkEMSegmentMRMLManager::
GetTreeParametersLeafNode(vtkIdType nodeID)
{
  vtkMRMLEMSTreeParametersNode* node = this->GetTreeParametersNode(nodeID);
  if (node == NULL)
    {
    vtkWarningMacro("Tree parameters node is null for node id: " << nodeID);
    return NULL;
    }
  return node->GetLeafParametersNode();
}

//----------------------------------------------------------------------------
vtkMRMLEMSTreeParametersParentNode*
vtkEMSegmentMRMLManager::
GetTreeParametersParentNode(vtkIdType nodeID)
{
  vtkMRMLEMSTreeParametersNode* node = this->GetTreeParametersNode(nodeID);
  if (node == NULL)
    {
    vtkWarningMacro("Tree parameters node is null for node id: " << nodeID);
    return NULL;
    }
  return node->GetParentParametersNode();
}

//----------------------------------------------------------------------------
vtkMRMLEMSClassInteractionMatrixNode*
vtkEMSegmentMRMLManager::
GetTreeClassInteractionNode(vtkIdType nodeID)
{
  vtkMRMLEMSTreeParametersParentNode* node = 
    this->GetTreeParametersParentNode(nodeID);
  if (node == NULL)
    {
    vtkWarningMacro("Tree parameters parent node is null for node id: " 
                    << nodeID);
    return NULL;
    }
  return node->GetClassInteractionMatrixNode();
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
GetTargetInputNode()
{
  vtkMRMLEMSWorkingDataNode* workingDataNode = this->GetWorkingDataNode();
  if (workingDataNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null WorkingDataNode with nonnull EMSNode.");
      }
    return NULL;
    }
  else
    {
    return workingDataNode->GetInputTargetNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkEMSegmentMRMLManager::
GetAtlasInputNode()
{
  vtkMRMLEMSWorkingDataNode* workingDataNode = this->GetWorkingDataNode();
  if (workingDataNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null WorkingDataNode with nonnull EMSNode.");
      }
    return NULL;
    }
  else
    {
    return workingDataNode->GetInputAtlasNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode*
vtkEMSegmentMRMLManager::
GetWorkingDataNode()
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
    return segmenterNode->GetWorkingDataNode();
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
vtkMRMLEMSNode*
vtkEMSegmentMRMLManager::
GetEMSNode()
{
  return this->Node;
}

//----------------------------------------------------------------------------
vtkMRMLEMSTargetNode*
vtkEMSegmentMRMLManager::
CloneTargetNode(vtkMRMLEMSTargetNode* targetNode, const char* name)
{
  if (targetNode == NULL)
    {
    return NULL;
    }

  // clone the target node
  vtkMRMLEMSTargetNode* clonedTarget = vtkMRMLEMSTargetNode::New();
  clonedTarget->CopyWithScene(targetNode);
  clonedTarget->SetName(name);
  clonedTarget->CloneVolumes(targetNode);

  // replace image names
  for (int i = 0; i < clonedTarget->GetNumberOfVolumes(); ++i)
  {    
    vtksys_stl::stringstream volumeName;
    volumeName << targetNode->GetNthVolumeNode(i)->GetName()
               << " (" << name << ")";
    clonedTarget->GetNthVolumeNode(i)->SetName(volumeName.str().c_str());
  }

  // add the target node to the scene
  this->MRMLScene->AddNode(clonedTarget);

  // clean up
  clonedTarget->Delete();

  return clonedTarget;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SynchronizeTargetNode(const vtkMRMLEMSTargetNode* templateNode, 
                      vtkMRMLEMSTargetNode* changingNode,
                      const char* name)
{
  if (templateNode == NULL || changingNode == NULL)
    {
      vtkWarningMacro("Attempt to synchronize target with null node!");
      return;
    }

  int numActualImages  = changingNode->GetNumberOfVolumes();
  
  // delete images from the current node
  for (int i = 0; i < numActualImages; ++i)
    {
    vtkMRMLVolumeNode* volumeNode = changingNode->GetNthVolumeNode(0);
    // NB: this will notify this node to remove the volume from the list
    this->GetMRMLScene()->RemoveNode(volumeNode);
    }

  // replace each image with a cloned image
  changingNode->SetName(name);
  changingNode->CloneVolumes(templateNode);

  // replace image names
  for (int i = 0; i < changingNode->GetNumberOfVolumes(); ++i)
  {    
    vtksys_stl::stringstream volumeName;
    volumeName << templateNode->GetNthVolumeNode(i)->GetName()
               << " (" << name << ")";
    changingNode->GetNthVolumeNode(i)->SetName(volumeName.str().c_str());
  }
}

//----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode*
vtkEMSegmentMRMLManager::
CloneAtlasNode(vtkMRMLEMSAtlasNode* atlasNode, const char* name)
{
  if (atlasNode == NULL)
    {
    return NULL;
    }

  // clone the atlas node
  vtkMRMLEMSAtlasNode* clonedAtlas = vtkMRMLEMSAtlasNode::New();
  clonedAtlas->CopyWithScene(atlasNode);
  clonedAtlas->SetName(name);
  clonedAtlas->CloneVolumes(atlasNode);

  // replace names
  for (int i = 0; i < clonedAtlas->GetNumberOfVolumes(); ++i)
  {
    vtksys_stl::stringstream volumeName;
    volumeName << atlasNode->GetNthVolumeNode(i)->GetName()
               << " (" << name << ")";
    clonedAtlas->GetNthVolumeNode(i)->SetName(volumeName.str().c_str());
  }

  // add the atlas node to the scene
  this->MRMLScene->AddNode(clonedAtlas);

  // clean up
  clonedAtlas->Delete();

  return clonedAtlas;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SynchronizeAtlasNode(const vtkMRMLEMSAtlasNode* templateNode, 
                     vtkMRMLEMSAtlasNode* changingNode,
                     const char* name)
{
  if (templateNode == NULL || changingNode == NULL)
    {
      vtkWarningMacro("Attempt to synchronize atlas with null node!");
      return;
    }

  int numActualImages  = changingNode->GetNumberOfVolumes();
  
  // delete images from the current node
  for (int i = 0; i < numActualImages; ++i)
    {
    vtkMRMLVolumeNode* volumeNode = changingNode->GetNthVolumeNode(0);
    // NB: this will notify this node to remove the volume from the list
    this->GetMRMLScene()->RemoveNode(volumeNode);
    }

  // replace each image with a cloned image
  changingNode->SetName(name);
  changingNode->CloneVolumes(templateNode);

  // replace image names
  for (int i = 0; i < changingNode->GetNumberOfVolumes(); ++i)
  {    
    vtksys_stl::stringstream volumeName;
    volumeName << templateNode->GetNthVolumeNode(i)->GetName()
               << " (" << name << ")";
    changingNode->GetNthVolumeNode(i)->SetName(volumeName.str().c_str());
  }

  // copy over number of training samples
  changingNode->SetNumberOfTrainingSamples
    (const_cast<vtkMRMLEMSAtlasNode*>(templateNode)->
     GetNumberOfTrainingSamples());
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

  vtkMRMLNode* node = 
    this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLEMSNode");
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
  
  this->SetNode(templateBuilderNode);
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

  // create working data node
  vtkMRMLEMSWorkingDataNode* workingNode = vtkMRMLEMSWorkingDataNode::New();
  workingNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(workingNode);
  
  // make connections
  workingNode->SetInputTargetNodeID(targetNode->GetID());
  workingNode->SetInputAtlasNodeID(atlasNode->GetID());

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

  vtkMRMLEMSClassInteractionMatrixNode* classInteractionMatrixNode = 
    vtkMRMLEMSClassInteractionMatrixNode::New();
  classInteractionMatrixNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(classInteractionMatrixNode);

  vtkMRMLEMSTreeParametersNode* treeParametersNode = 
    vtkMRMLEMSTreeParametersNode::New();
  treeParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  treeParametersNode->SetClassProbability(1.0);
  this->GetMRMLScene()->AddNode(treeParametersNode);
  
  parentParametersNode->
    SetClassInteractionMatrixNodeID(classInteractionMatrixNode->GetID());
  treeParametersNode->SetLeafParametersNodeID(leafParametersNode->GetID());
  treeParametersNode->SetParentParametersNodeID(parentParametersNode->GetID());

  // create root tree node
  vtkMRMLEMSTreeNode* treeNode = vtkMRMLEMSTreeNode::New();
  treeNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(treeNode); // this adds id map

  // add connections
  treeNode->SetTreeParametersNodeID(treeParametersNode->GetID());

  // create template node
  vtkMRMLEMSTemplateNode* templateNode = vtkMRMLEMSTemplateNode::New();
  templateNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(templateNode);
  
  // add connections
  templateNode->SetTreeNodeID(treeNode->GetID());
  templateNode->SetGlobalParametersNodeID(globalParametersNode->GetID());

  // create segmenter node
  vtkMRMLEMSSegmenterNode* segmenterNode = vtkMRMLEMSSegmenterNode::New();
  segmenterNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(segmenterNode);
  
  // add connections
  segmenterNode->SetTemplateNodeID(templateNode->GetID());
  segmenterNode->SetWorkingDataNodeID(workingNode->GetID());
  
  // create template builder node
  vtkMRMLEMSNode* templateBuilderNode = vtkMRMLEMSNode::New();
  templateBuilderNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(templateBuilderNode);
  
  // add connections
  templateBuilderNode->SetSegmenterNodeID(segmenterNode->GetID());

  this->SetNode(templateBuilderNode);

  // add basic information for root node
  vtkIdType rootID = this->GetTreeRootNodeID();
  this->SetTreeNodeLabel(rootID, "Root");
  this->SetTreeNodeName(rootID, "Root");
  this->SetTreeNodeIntensityLabel(rootID, rootID);

  // delete nodes
  atlasNode->Delete();
  targetNode->Delete();
  workingNode->Delete();
  globalParametersNode->Delete();
  classInteractionMatrixNode->Delete();
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

  vtkMRMLEMSClassInteractionMatrixNode* classInteractionMatrixNode = 
    vtkMRMLEMSClassInteractionMatrixNode::New();
  classInteractionMatrixNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(classInteractionMatrixNode);

  vtkMRMLEMSTreeParametersNode* treeParametersNode = 
    vtkMRMLEMSTreeParametersNode::New();
  treeParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(treeParametersNode);

  // add connections  
  parentParametersNode->
    SetClassInteractionMatrixNodeID(classInteractionMatrixNode->GetID());
  treeParametersNode->SetLeafParametersNodeID(leafParametersNode->GetID());
  treeParametersNode->SetParentParametersNodeID(parentParametersNode->GetID());

  // update memory
  treeParametersNode->
    SetNumberOfTargetInputChannels(this->GetTargetInputNode()->
                                   GetNumberOfVolumes());

  // create tree node and add it to the scene
  vtkMRMLEMSTreeNode* treeNode = vtkMRMLEMSTreeNode::New();
  treeNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(treeNode); // this adds id pair to map

  // add connections
  treeNode->SetTreeParametersNodeID(treeParametersNode->GetID());

  // set the intensity label (in resulting segmentation) to the ID
  vtkIdType treeNodeID = this->MapMRMLNodeIDToVTKNodeID(treeNode->GetID());
  this->SetTreeNodeIntensityLabel(treeNodeID, treeNodeID);
  
  // delete nodes
  classInteractionMatrixNode->Delete();
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
    return;
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

    // remove class interaction matrix node
    vtkMRMLNode* classInteractionMatrixNode = parametersNode->
      GetParentParametersNode()->GetClassInteractionMatrixNode();
    if (classInteractionMatrixNode != NULL)
      {
      this->GetMRMLScene()->RemoveNode(classInteractionMatrixNode);
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
    vtkErrorMacro("vtk ID mapped to null mrml ID: " << nodeID);
    }
  
  // NB: being careful not to return point to temporary
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
int
vtkEMSegmentMRMLManager::
GetTargetVolumeIndex(vtkIdType volumeID)
{
  // map to MRML ID
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not map volume ID: " << volumeID);
    return -1;
    }

  // get this image's index in the target list
  return this->GetTargetInputNode()->GetIndexByVolumeNodeID(mrmlID);
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PropogateAdditionOfSelectedTargetImage()
{
  this->GetGlobalParametersNode()->AddTargetInputChannel();

  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    this->GetTreeParametersNode(*i)->
      AddTargetInputChannel();
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PropogateRemovalOfSelectedTargetImage(int imageIndex)
{
  this->GetGlobalParametersNode()->RemoveNthTargetInputChannel(imageIndex);

  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    this->GetTreeParametersNode(*i)->
      RemoveNthTargetInputChannel(imageIndex);
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PropogateMovementOfSelectedTargetImage(int fromIndex, int toIndex)
{
  this->GetGlobalParametersNode()->
    MoveNthTargetInputChannel(fromIndex, toIndex);

  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
    this->GetTreeParametersNode(*i)->
      MoveNthTargetInputChannel(fromIndex, toIndex);
    }
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

  vtkMRMLEMSClassInteractionMatrixNode* emsClassInteractionMatrixNode = 
    vtkMRMLEMSClassInteractionMatrixNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsClassInteractionMatrixNode);
  emsClassInteractionMatrixNode->Delete();

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

  vtkMRMLEMSWorkingDataNode* emsWorkingDataNode = 
    vtkMRMLEMSWorkingDataNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsWorkingDataNode);
  emsWorkingDataNode->Delete();  
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
UpdateMapsFromMRML()
{
  // create coppies of old maps
  VTKToMRMLMapType oldVTKToMRMLMap = this->VTKNodeIDToMRMLNodeIDMap;
  MRMLToVTKMapType oldMRMLtoVTKMap = this->MRMLNodeIDToVTKNodeIDMap;

  // clear maps
  this->VTKNodeIDToMRMLNodeIDMap.clear();
  this->MRMLNodeIDToVTKNodeIDMap.clear();
  
  if (this->GetMRMLScene() == NULL)
  {
    return;
  }

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
      vtkstd::string mrmlID = node->GetID();
      
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
      vtkstd::string mrmlID = node->GetID();
      
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

  // check template builder node
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
  vtkMRMLEMSTargetNode *targetNode = this->GetTargetInputNode();
  if (targetNode == NULL)
    {
    vtkErrorMacro("Target node is NULL.");
    return 0;
    }

  // check atlas node
  vtkMRMLEMSAtlasNode *atlasNode = this->GetAtlasInputNode();
  if (atlasNode == NULL)
    {
    vtkErrorMacro("Atlas node is NULL.");
    return 0;
    }

  // check working data node
  vtkMRMLEMSWorkingDataNode *workingNode = this->GetWorkingDataNode();
  if (workingNode == NULL)
    {
    vtkErrorMacro("Working data node is NULL.");
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
  int numTargetInputChannels = this->GetTargetInputNode()->GetNumberOfVolumes();
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
void
vtkEMSegmentMRMLManager::
PrintTree()
{
  this->PrintTree(this->GetTreeRootNodeID(), static_cast<vtkIndent>(0));
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PrintTree(vtkIdType rootID, vtkIndent indent)
{
  vtkstd::string mrmlID = this->MapVTKNodeIDToMRMLNodeID(rootID);
  vtkMRMLEMSTreeNode* rnode = this->GetTreeNode(rootID);
  const char* label = this->GetTreeNodeLabel(rootID);
  const char* name = this->GetTreeNodeName(rootID);

  if (rnode == NULL)
    {
    vtkstd::cerr << indent << "Node is null for id=" << rootID << std::endl;
    }
  else
    {
    vtkstd::cerr << indent << "Label: " << (label ? label : "(null)") 
                 << vtkstd::endl;
    vtkstd::cerr << indent << "Name: " << (name ? name : "(null)") 
                 << vtkstd::endl;
    vtkstd::cerr << indent << "ID: "    << rootID 
                 << " MRML ID: " << rnode->GetID()
                 << " From Map: " << mrmlID << vtkstd::endl;
    vtkstd::cerr << indent << "Is Leaf: " << this->GetTreeNodeIsLeaf(rootID) 
                 << vtkstd::endl;
    int numChildren = this->GetTreeNodeNumberOfChildren(rootID); 
    vtkstd::cerr << indent << "Num. Children: " << numChildren << vtkstd::endl;
    vtkstd::cerr << indent << "Child IDs from parent: ";
    for (int i = 0; i < numChildren; ++i)
      {
      vtkstd::cerr << rnode->GetNthChildNodeID(i) << " ";
      }
    vtkstd::cerr << vtkstd::endl;
    vtkstd::cerr << indent << "Child IDs from children: ";
    for (int i = 0; i < numChildren; ++i)
      {
      vtkstd::cerr << rnode->GetNthChildNode(i)->GetID() << " ";
      }
    vtkstd::cerr << vtkstd::endl;

    indent = indent.GetNextIndent();
    for (int i = 0; i < numChildren; ++i)
      {
      vtkIdType childID = this->GetTreeNodeChildNodeID(rootID, i);
      vtkstd::cerr << indent << "Child " << i << " (" << childID 
                   << ") of node " << rootID << vtkstd::endl;
      this->PrintTree(childID, indent);
      }
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
PrintVolumeInfo()
{
  // for every volume node
  int numVolumes = 
    this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
  for (int i = 0; i < numVolumes; ++i)
    {
    vtkMRMLNode* node = this->GetMRMLScene()->
      GetNthNodeByClass(i, "vtkMRMLVolumeNode");
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);

    // print volume node ID and name
    vtkstd::cerr << "Volume Node ID / Name: " << volumeNode->GetID()
              << " / " << volumeNode->GetName() << vtkstd::endl;
    // print display node id
    vtkstd::cerr << " Display Node ID: " 
              << (volumeNode->GetDisplayNode() ?
                  volumeNode->GetDisplayNode()->GetID() : "NULL")
              << vtkstd::endl;

    // print storage node id and filename
    vtkstd::cerr << " Storage Node ID / Filename: " 
              << (volumeNode->GetStorageNode() ?
                  volumeNode->GetStorageNode()->GetID() : "NULL")
              << " / "
              << (volumeNode->GetStorageNode() ?
                  volumeNode->GetStorageNode()->GetFileName() : "NULL")
              << vtkstd::endl;
    }
}

//-----------------------------------------------------------------------------
bool
vtkEMSegmentMRMLManager::
PackageAndWriteData(const char* packageDirectory)
{
  //
  // create a scene and copy the EMSeg related nodes to it
  //
  vtkMRMLScene* newScene = vtkMRMLScene::New();
  std::string outputDirectory(packageDirectory);

  if (!outputDirectory.empty() && 
      outputDirectory[outputDirectory.size()-1] != '/')
    {
    // make sure directory ends in seperator
    outputDirectory = outputDirectory + "/";
    }

  std::string mrmlURL(outputDirectory + "EMSegmenterScene.mrml");
  newScene->SetRootDirectory(outputDirectory.c_str());
  newScene->SetURL(mrmlURL.c_str());
  this->CopyEMRelatedNodesToMRMLScene(newScene);
  
  //
  // update filenames to match standardized package structure
  this->CreatePackageFilenames(newScene, packageDirectory);

  //
  // create directory structure on disk
  bool createdDirectories = this->CreatePackageDirectories(packageDirectory);
  if (!createdDirectories)
    {
    vtkErrorMacro("PackageAndWriteData: failed to create directories");
    newScene->Delete();
    return false;
    }

  //
  // write the scene out to disk
  bool wroteScene = this->WritePackagedScene(newScene);

  // clean up
  newScene->Delete();
  
  if (!wroteScene)
    {
    vtkErrorMacro("PackageAndWrite: failed to write scene");
    return false;
    }  

  return true;
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyEMRelatedNodesToMRMLScene(vtkMRMLScene* newScene)
{
  //
  // clear scene
  //
  newScene->Clear(1);

  //
  // copy over nodes from the current scene to the new scene
  //

  vtkMRMLScene*   currentScene = this->GetMRMLScene();
  vtkMRMLEMSNode* emNode       = this->GetEMSNode();
  if (currentScene == NULL || emNode == NULL)
    {
    return;
    }

  // get all nodes associated with this EMSeg parameter set
  vtkCollection* emNodeCollection = this->GetMRMLScene()->
    GetReferencedNodes(emNode);  

  // add the nodes to the scene
  emNodeCollection->InitTraversal();
  vtkObject* currentObject = NULL;
  while ((currentObject = emNodeCollection->GetNextItemAsObject()) &&
         (currentObject != NULL))
    {
    vtkMRMLNode* n = dynamic_cast<vtkMRMLNode*>(currentObject);
    if (n == NULL)
      {
      continue;
      }

    vtkMRMLNode* node = n->CreateNodeInstance();

    node->CopyWithScene(n);
    newScene->AddNodeNoNotify(node);
    node->Delete();
    }

  // clean up
  emNodeCollection->Delete();
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CreatePackageFilenames(vtkMRMLScene* scene, 
                       const char* packageDirectoryName)
{
  //
  // set up mrml manager for this new scene
  vtkEMSegmentMRMLManager* newSceneManager = vtkEMSegmentMRMLManager::New();
  newSceneManager->SetMRMLScene(scene);
  vtkMRMLEMSNode* newEMSNode = dynamic_cast<vtkMRMLEMSNode*>
    (scene->GetNthNodeByClass(0, "vtkMRMLEMSNode"));
  if (newEMSNode == NULL)
    {
    vtkWarningMacro("CreatePackageFilenames: no EMS node!");
    newSceneManager->Delete();
    return;
    }
  else
    {
    newSceneManager->SetNode(newEMSNode);
    }
  vtkMRMLEMSWorkingDataNode* workingDataNode = 
    newSceneManager->GetWorkingDataNode();

  //
  // We might be creating volume storage nodes.  We must decide if the
  // images should be automatically centered when they are read.  Look
  // at the original input target node zero to decide if we will use
  // centering.
  bool centerImages = false;
  if (workingDataNode && workingDataNode->GetInputTargetNode())
    {
    if (workingDataNode->GetInputTargetNode()->GetNumberOfVolumes() > 0)
      {
      vtkMRMLStorageNode* firstTargetStorageNode =
        workingDataNode->GetInputTargetNode()->GetNthVolumeNode(0)->
        GetStorageNode();
      vtkMRMLVolumeArchetypeStorageNode* firstTargetVolumeStorageNode =
        dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*>
        (firstTargetStorageNode);
      if (firstTargetVolumeStorageNode != NULL)
        {
        centerImages = firstTargetVolumeStorageNode->GetCenterImage();
        }
      }
    }

  //
  // change the storage file for the segmentation result
    {
    vtkMRMLVolumeNode* volumeNode = newSceneManager->GetOutputVolumeNode();
    if (volumeNode != NULL)
      {
      vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
      vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode = 
        dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*>(storageNode);
      if (volumeStorageNode == NULL)
      {
      // create a new storage node for this volume
      volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
      scene->AddNodeNoNotify(volumeStorageNode);
      volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
      std::cerr << "Added storage node : " << volumeStorageNode->GetID() 
                << std::endl;
      volumeStorageNode->Delete();
      storageNode = volumeStorageNode;
      }
      volumeStorageNode->SetCenterImage(centerImages);
    
      // create new filename
      std::string oldFilename       = 
        (storageNode->GetFileName() ? storageNode->GetFileName() :
         "SegmentationResult.mhd");
      std::string oldFlienameNoPath = 
        vtksys::SystemTools::GetFilenameName(oldFilename);
      std::vector<std::string> pathComponents;
      pathComponents.push_back("");
      pathComponents.push_back("Segmentation");
      pathComponents.push_back(oldFlienameNoPath);
      std::string newFilename = 
        vtksys::SystemTools::JoinPath(pathComponents);
      storageNode->SetFileName(newFilename.c_str());
      }
    }

  //
  // change the storage file for the targets
  int numTargets = newSceneManager->GetTargetNumberOfSelectedVolumes();

  // input target volumes
  if (workingDataNode->GetInputTargetNode())
    {
    for (int i = 0; i < numTargets; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
        workingDataNode->GetInputTargetNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode = 
          dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*>(storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cerr << "Added storage node : " << storageNode->GetID() 
                    << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
      
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Target" << i << "_Input.mhd";
        std::string oldFilename       = 
          (storageNode->GetFileName() ? storageNode->GetFileName() :
           defaultFilename.str().c_str());
        std::string oldFlienameNoPath = 
          vtksys::SystemTools::GetFilenameName(oldFilename);
        std::vector<std::string> pathComponents;
        pathComponents.push_back("");
        pathComponents.push_back("Target");
        pathComponents.push_back("Input");
        pathComponents.push_back(oldFlienameNoPath);
        std::string newFilename = 
          vtksys::SystemTools::JoinPath(pathComponents);
        
        storageNode->SetFileName(newFilename.c_str());
        }
      }  
    }

  // normalized target volumes
  if (workingDataNode->GetNormalizedTargetNode())
    {
    for (int i = 0; i < numTargets; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
        workingDataNode->GetNormalizedTargetNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode = 
          dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*>(storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cerr << "Added storage node : " << volumeStorageNode->GetID() 
                    << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
          
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Target" << i << "_Normalized.mhd";
        std::string oldFilename       = 
          (storageNode->GetFileName() ? storageNode->GetFileName() :
           defaultFilename.str().c_str());
        std::string oldFlienameNoPath = 
          vtksys::SystemTools::GetFilenameName(oldFilename);
        std::vector<std::string> pathComponents;
        pathComponents.push_back("");
        pathComponents.push_back("Target");
        pathComponents.push_back("Normalized");
        pathComponents.push_back(oldFlienameNoPath);
        std::string newFilename = 
          vtksys::SystemTools::JoinPath(pathComponents);
        
        storageNode->SetFileName(newFilename.c_str());
        }
      }  
    }

  // aligned target volumes
  if (workingDataNode->GetAlignedTargetNode())
    {
    for (int i = 0; i < numTargets; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
        workingDataNode->GetAlignedTargetNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode = 
          dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*>(storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cerr << "Added storage node : " << volumeStorageNode->GetID() 
                    << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
          
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Target" << i << "_Aligned.mhd";
        std::string oldFilename       = 
          (storageNode->GetFileName() ? storageNode->GetFileName() :
           defaultFilename.str().c_str());
        std::string oldFlienameNoPath = 
          vtksys::SystemTools::GetFilenameName(oldFilename);
        std::vector<std::string> pathComponents;
        pathComponents.push_back("");
        pathComponents.push_back("Target");
        pathComponents.push_back("Aligned");
        pathComponents.push_back(oldFlienameNoPath);
        std::string newFilename = 
          vtksys::SystemTools::JoinPath(pathComponents);
        
        storageNode->SetFileName(newFilename.c_str());
        }
      }  
    }

  //
  // change the storage file for the atlas
  int numAtlasVolumes = newSceneManager->GetAtlasInputNode()->
    GetNumberOfVolumes();

  // input atlas volumes
  if (workingDataNode->GetInputAtlasNode())
    {
    for (int i = 0; i < numAtlasVolumes; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
        workingDataNode->GetInputAtlasNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode = 
          dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*>(storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cerr << "Added storage node : " << volumeStorageNode->GetID() 
                    << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
      
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Atlas" << i << "_Input.mhd";
        std::string oldFilename       = 
          (storageNode->GetFileName() ? storageNode->GetFileName() :
           defaultFilename.str().c_str());
        std::string oldFlienameNoPath = 
          vtksys::SystemTools::GetFilenameName(oldFilename);
        std::vector<std::string> pathComponents;
        pathComponents.push_back("");
        pathComponents.push_back("Atlas");
        pathComponents.push_back("Input");
        pathComponents.push_back(oldFlienameNoPath);
        std::string newFilename = 
          vtksys::SystemTools::JoinPath(pathComponents);
        
        storageNode->SetFileName(newFilename.c_str());
        }
      }  
    }

  // aligned atlas volumes
  if (workingDataNode->GetAlignedAtlasNode())
    {
    for (int i = 0; i < numAtlasVolumes; ++i)
      {
      vtkMRMLVolumeNode* volumeNode =
        workingDataNode->GetAlignedAtlasNode()->GetNthVolumeNode(i);
      if (volumeNode != NULL)
        {
        vtkMRMLStorageNode* storageNode = volumeNode->GetStorageNode();
        vtkMRMLVolumeArchetypeStorageNode* volumeStorageNode = 
          dynamic_cast<vtkMRMLVolumeArchetypeStorageNode*>(storageNode);
        if (volumeStorageNode == NULL)
          {
          // create a new storage node for this volume
          volumeStorageNode = vtkMRMLVolumeArchetypeStorageNode::New();
          scene->AddNodeNoNotify(volumeStorageNode);
          volumeNode->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
          std::cerr << "Added storage node : " << volumeStorageNode->GetID() 
                    << std::endl;
          volumeStorageNode->Delete();
          storageNode = volumeStorageNode;
          }
        volumeStorageNode->SetCenterImage(centerImages);
        
        // create new filename
        vtkstd::stringstream defaultFilename;
        defaultFilename << "Atlas" << i << "_Aligned.mhd";
        std::string oldFilename       = 
          (storageNode->GetFileName() ? storageNode->GetFileName() :
           defaultFilename.str().c_str());
        std::string oldFlienameNoPath = 
          vtksys::SystemTools::GetFilenameName(oldFilename);
        std::vector<std::string> pathComponents;
        pathComponents.push_back("");
        pathComponents.push_back("Atlas");
        pathComponents.push_back("Aligned");
        pathComponents.push_back(oldFlienameNoPath);
        std::string newFilename = 
          vtksys::SystemTools::JoinPath(pathComponents);
        
        storageNode->SetFileName(newFilename.c_str());
        }
      }  
    }

  // clean up
  newSceneManager->Delete();
}

//-----------------------------------------------------------------------------
bool
vtkEMSegmentMRMLManager::
CreatePackageDirectories(const char* packageDirectoryName)
{
  vtkstd::string packageDirectory(packageDirectoryName);
  
  // check that parent directory exists
  std::string parentDirectory = 
    vtksys::SystemTools::GetParentDirectory(packageDirectory.c_str());
  if (!vtksys::SystemTools::FileExists(parentDirectory.c_str()))
    {
    vtkWarningMacro
      ("CreatePackageDirectories: Parent directory does not exist!");
    return false;
    }
  
  // create package directories
  bool createdOK = true;
  std::string newDir = packageDirectory + "/Atlas/Input";
  createdOK = createdOK &&
    vtksys::SystemTools::MakeDirectory(newDir.c_str());  
  newDir = packageDirectory + "/Atlas/Aligned";
  createdOK = createdOK &&
    vtksys::SystemTools::MakeDirectory(newDir.c_str());  
  newDir = packageDirectory + "/Target/Input";
  createdOK = createdOK &&
    vtksys::SystemTools::MakeDirectory(newDir.c_str());  
  newDir = packageDirectory + "/Target/Normalized";
  createdOK = createdOK &&
    vtksys::SystemTools::MakeDirectory(newDir.c_str());  
  newDir = packageDirectory + "/Target/Aligned";
  createdOK = createdOK &&
    vtksys::SystemTools::MakeDirectory(newDir.c_str());  
  newDir = packageDirectory + "/Segmentation";
  createdOK = createdOK &&
    vtksys::SystemTools::MakeDirectory(newDir.c_str());  

  if (!createdOK)
    {
    vtkWarningMacro("CreatePackageDirectories: Could not create directories!");
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool
vtkEMSegmentMRMLManager::
WritePackagedScene(vtkMRMLScene* scene)
{
  //
  // write the volumes
  scene->InitTraversal();
  vtkMRMLNode* currentNode;
  bool allOK = true;
  while ((currentNode = scene->GetNextNodeByClass("vtkMRMLVolumeNode")) &&
         (currentNode != NULL))
    {
    vtkMRMLVolumeNode* volumeNode = 
      dynamic_cast<vtkMRMLVolumeNode*>(currentNode);

    if (volumeNode == NULL)
      {
      vtkWarningMacro("Volume node is null for node: " 
                    << currentNode->GetID());
      scene->RemoveNode(currentNode);
      allOK = false;
      continue;
      }
    if (volumeNode->GetImageData() == NULL)
      {
      vtkWarningMacro("Volume data is null for volume node: " 
                    << currentNode->GetID());
      scene->RemoveNode(currentNode);
      allOK = false;
      continue;
      }
    if (volumeNode->GetStorageNode() == NULL)
      {
      vtkWarningMacro("Volume storage node is null for volume node: " 
                    << currentNode->GetID());
      scene->RemoveNode(currentNode);
      allOK = false;
      continue;
      }

    try
      {
      std::cerr << "Writing volume: " << volumeNode->GetName() 
                << ": " << volumeNode->GetStorageNode()->GetFileName() << "...";
      volumeNode->GetStorageNode()->SetUseCompression(0);
      volumeNode->GetStorageNode()->WriteData(volumeNode);
      std::cerr << "DONE" << std::endl;
      }
    catch (...)
      {
      vtkErrorMacro("Problem writing volume: " << volumeNode->GetID());
      allOK = false;
      }
    }
  
  //
  // write the MRML scene file
  try 
    {
    scene->Commit();
    }
  catch (...)
    {
    vtkErrorMacro("Problem writing scene.");
    allOK = false;
    }  

  return allOK;
}
