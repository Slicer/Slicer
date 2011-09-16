#include "vtkObjectFactory.h"

#include "vtkEMSegmentMRMLManager.h"
#include "vtkEMSegment.h"
#include <time.h>

#include "vtkMRMLScene.h"

#include "vtkMRMLEMSTemplateNode.h"
#include "vtkMRMLEMSTreeNode.h"
#include "vtkMRMLEMSTreeParametersLeafNode.h"
#include "vtkMRMLEMSTreeParametersParentNode.h"
#include "vtkMRMLEMSTreeParametersNode.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkImageEMGeneral.h"
#include "vtkMRMLEMSAtlasNode.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include "vtkMRMLEMSTargetNode.h"
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkMath.h"

#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
// needed to translate between enums
#include "EMLocalInterface.h"
#include <math.h>
#include <exception>

// #include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>


// For legacy 
#include "vtkMRMLEMSNode.h"
#include "vtkMRMLEMSSegmenterNode.h"


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
    std::cout << "Not the right scene" << std::endl;
    return;
    }

  vtkMRMLNode *node = reinterpret_cast<vtkMRMLNode*> (callData);
  if (node == NULL)
    {
    //std::cout << "Node is NULL: " << caller << " event: " << event << std::endl;
    return;
    }

  //std::cout << "adding node:" << node << " event: " << event << std::endl;

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
void vtkEMSegmentMRMLManager::SetNode(vtkMRMLEMSTemplateNode *n)
{
  vtkSetObjectBodyMacro(Node, vtkMRMLEMSTemplateNode, n);
  this->UpdateMapsFromMRML();  
}

//----------------------------------------------------------------------------
int vtkEMSegmentMRMLManager::SetNodeWithCheck(vtkMRMLEMSTemplateNode *n)
{
  if (n &&  !this->CheckTemplateMRMLStructure(n,1)) 
    {
        vtkErrorMacro("Incomplete or invalid MRML node structure.");
        return 1 ;  
    }
  this->SetNode(n);
  return 0;
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "NextVTKNodeID :" << this->NextVTKNodeID << "\n";
  os << indent << "MRMLScene: " <<  (this->MRMLScene ? this->MRMLScene->GetURL() : "(none)") << "\n";

  os << indent << "VTKNodeIDToMRMLNodeIDMap: " << "\n";
  for (VTKToMRMLMapType::iterator i =   VTKNodeIDToMRMLNodeIDMap.begin(); i != this->  VTKNodeIDToMRMLNodeIDMap.end(); ++i)
    {
      vtkIdType   VTKNodeID  = (*i).first;
      std::string MRMLNodeID = (*i).second;
      os << indent << "  VTKNode " <<  VTKNodeID << " MRMLNodeID: " << MRMLNodeID.c_str() << "\n";
    }
 os << indent << "MRMLNodeIDToVTKNodeIDMap: " << "\n";
  for (MRMLToVTKMapType::iterator i =   MRMLNodeIDToVTKNodeIDMap.begin(); i != this->  MRMLNodeIDToVTKNodeIDMap.end(); ++i)
    {
      vtkIdType   VTKNodeID  = (*i).second;
      std::string MRMLNodeID = (*i).first;
      os << indent << "   MRMLNodeID: " << MRMLNodeID.c_str() << "  VTKNode " <<  VTKNodeID << "\n";
    }
 
  os << indent << "Node: " 
     << (this->Node ? 
         this->Node->GetID() : 
         "(none)") 
     << "\n";
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::PrintInfo(ostream& os)
{
  vtkIndent indent(4);
  os << "\n===== Print Manger Info ==== \n";
  os << "MRMLScene: " <<  (this->MRMLScene ? this->MRMLScene->GetURL() : "(none)") << "\n";
  os << "===== Node \n" ;
  if (!Node) 
    {
      os << indent << "Warning: No Template Node Defined " << endl;
      return;
    }
  this->Node->PrintSelf(os,indent);
  os << "===== GlobalParametersNode \n" ;
  if (!this->GetGlobalParametersNode())
    {
      os << indent << "Warning: No Global Parameters Node Defined " << endl;
      return;
    }
  this->GetGlobalParametersNode()->PrintSelf(os,indent);

  os << "===== WorkingDataNode \n" ;
  if (!this->GetWorkingDataNode())
    {
      os << indent << "No Node Defined " << endl;
    } 
  else 
    {
      this->GetWorkingDataNode()->PrintSelf(os,indent);
    }

  os << "===== Spatial Atlas Node \n" ;
  if (!this->GetAtlasInputNode())
    {
      os << indent << "No Node Defined " << endl;
    } 
  else 
    {
      this->GetAtlasInputNode()->PrintSelf(os,indent);
    }
}

void vtkEMSegmentMRMLManager::PrintInfo() {
    this->PrintInfo(cout); 
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
  if (childNodeID == -1)
    {
    return NULL;
    }

  vtkMRMLEMSTreeNode* childNode = this->GetTreeNode(childNodeID);
  if (childNode == NULL)
    {
    vtkErrorMacro("Child tree node is null for nodeID: " << childNodeID);
    return ERROR_NODE_VTKID;
    }

  vtkMRMLEMSTreeNode* parentNode = childNode->GetParentNode();
  if (parentNode == NULL)
    {
      vtkErrorMacro("Child's parent node is null for nodeID: " << childNodeID << " and Name: " << childNode->GetName());
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
    int childIndex = oldParentNode->GetChildIndexByMRMLID(childNode->GetID());
    if (childIndex < 0)
      {
      vtkErrorMacro("ERROR: can't find child's index in old parent node.");
      }

      oldParentNode->RemoveNthChildNode(childIndex);

       // Delete ParameterParentNode and create ParameterLeafNode
      this->TurnFromParentToLeafNode(oldParentNode) ;
    } 

  // point the child to the new parent
  childNode->SetParentNodeID(parentNode->GetID());

  // point parent to this child node
  parentNode->AddChildNode(childNode->GetID());
  
  // Create Parent Parameter node if needed
  vtkMRMLEMSTreeParametersParentNode *parParentNode = parentNode->GetParentParametersNode();
  if ( !parParentNode)
    {
           parParentNode  =vtkMRMLEMSTreeParametersParentNode::New();
           parParentNode->SetHideFromEditors(this->HideNodesFromEditors);
           this->MRMLScene->AddNode(parParentNode);
           parentNode->SetParentParametersNodeID(parParentNode->GetID()); 
           parParentNode->Delete(); 
    } 

    //   Delete leaf parameter node  if needed
    vtkMRMLEMSTreeParametersLeafNode* parLeafNode = parentNode->GetLeafParametersNode();
    parentNode->SetLeafParametersNodeID(NULL);
 
    if ( parLeafNode )
       {
            this->MRMLScene->RemoveNode(parLeafNode);
       }
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
       this->TurnFromParentToLeafNode(parentNode);
      }
    }
 
  // remove node from scene  
  this->GetMRMLScene()->RemoveNode(node);
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
  n->GetColorRGB(rgb);
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
  n->SetColorRGB(rgb);
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
vtkEMSegmentMRMLManager::GetTreeNodeDistributionLogMeanCorrection(vtkIdType nodeID, int volumeNumber)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }
  return this->GetTreeParametersLeafNode(nodeID)->GetLogMeanCorrection(volumeNumber);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeDistributionLogMeanCorrection(vtkIdType nodeID, 
                               int volumeNumber, 
                               double value)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }
  this->GetTreeParametersLeafNode(nodeID)->SetLogMeanCorrection(volumeNumber, value);
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
vtkstd::vector<vtkstd::vector<double> >
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionLogCovariance(vtkIdType nodeID)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    vtkstd::vector<vtkstd::vector<double> > blub;
    return blub;
    }
  return this->GetTreeParametersLeafNode(nodeID)->GetLogCovariance();
}

//----------------------------------------------------------------------------   
vtkstd::vector<vtkstd::vector<double> >
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionLogCovarianceCorrection(vtkIdType nodeID)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    vtkstd::vector<vtkstd::vector<double> > blub;
    return blub;
    }
  return this->GetTreeParametersLeafNode(nodeID)->GetLogCovarianceCorrection();
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
double   
vtkEMSegmentMRMLManager::
GetTreeNodeDistributionLogCovarianceCorrection(vtkIdType nodeID, 
                                     int rowIndex,
                                     int columnIndex)
{
  if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }
  return this->GetTreeParametersLeafNode(nodeID)->GetLogCovarianceCorrection(rowIndex, columnIndex);
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
      if (this->GetTreeParametersLeafNode(*i) &&  (this->GetTreeParametersLeafNode(*i)->GetDistributionSpecificationMethod() == vtkMRMLEMSTreeParametersLeafNode::DistributionSpecificationManuallySample)) 
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
      if (this->GetTreeParametersLeafNode(*i) && (this->GetTreeParametersLeafNode(*i)->GetDistributionSpecificationMethod() == vtkMRMLEMSTreeParametersLeafNode::DistributionSpecificationManuallySample))
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
  vtkMRMLEMSVolumeCollectionNode* workingTarget = 
    this->GetWorkingDataNode()->GetInputTargetNode();
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
    GetTreeNode(nodeID)->GetLeafParametersNode();
  if (!leafNode) 
    { 
      vtkErrorMacro("Tree node  " << nodeID << " or not a leaf node!");
      return ;
    }
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

  return n->GetPrintWeights();  
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
  n->SetPrintWeights(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintQuality(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL  || !n->GetLeafParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a leaf node!");
      return 0;
    }

  return n->GetLeafParametersNode()->GetPrintQuality();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintQuality(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetLeafParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a leaf node!");
      return;
    }
  n->GetLeafParametersNode()->
    SetPrintQuality(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeIntensityLabel(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetLeafParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a leaf node!");
      return 0;
    }

  return n->GetLeafParametersNode()->
    GetIntensityLabel();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeIntensityLabel(vtkIdType nodeID, int label)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL  || !n->GetLeafParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a leaf node!");
      return;
    }
  n->GetLeafParametersNode()->SetIntensityLabel(label);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintFrequency(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }

  return n->GetParentParametersNode()->
    GetPrintFrequency();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintFrequency(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetPrintFrequency(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintLabelMap(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }

  return n->GetParentParametersNode()->
    GetPrintLabelMap();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintLabelMap(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetPrintLabelMap(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintEMLabelMapConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }

  return n->GetParentParametersNode()->
    GetPrintEMLabelMapConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintEMLabelMapConvergence(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetPrintEMLabelMapConvergence(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintEMWeightsConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }

  return n->GetParentParametersNode()->
    GetPrintEMWeightsConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintEMWeightsConvergence(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetPrintEMWeightsConvergence(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintMFALabelMapConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }

  return n->GetParentParametersNode()->
    GetPrintMFALabelMapConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintMFALabelMapConvergence(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetPrintMFALabelMapConvergence(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintMFAWeightsConvergence(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }

  return n->GetParentParametersNode()->
    GetPrintMFAWeightsConvergence();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintMFAWeightsConvergence(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetPrintMFAWeightsConvergence(shouldPrint);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeGenerateBackgroundProbability(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return 0;
    }

  return n->GetParentParametersNode()->
    GetGenerateBackgroundProbability();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeGenerateBackgroundProbability(vtkIdType nodeID, int value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetGenerateBackgroundProbability(value);  
}


//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeExcludeFromIncompleteEStep(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL  )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID  );
      return 0;
    }

  return n->GetExcludeFromIncompleteEStep();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeExcludeFromIncompleteEStep(vtkIdType nodeID, int shouldExclude)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return;
    }
  n->SetExcludeFromIncompleteEStep(shouldExclude);  
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeAlpha(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return 0;
    }
  return n->GetParentParametersNode()->GetAlpha();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeAlpha(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return; 
    }

  n->GetParentParametersNode()->SetAlpha(value);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodePrintBias(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return 0;
    }
  return n->GetParentParametersNode()->GetPrintBias();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodePrintBias(vtkIdType nodeID, int shouldPrint)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID <<  " or not a parent node");
      return;
    }

  n->GetParentParametersNode()->SetPrintBias(shouldPrint);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeBiasCalculationMaxIterations(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }
  return n->GetParentParametersNode()->
    GetBiasCalculationMaxIterations();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeBiasCalculationMaxIterations(vtkIdType nodeID, int value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return;
    }

  n->GetParentParametersNode()->
    SetBiasCalculationMaxIterations(value);  
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeSmoothingKernelWidth(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return 0;
    }
  return n->GetParentParametersNode()->
    GetSmoothingKernelWidth();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeSmoothingKernelWidth(vtkIdType nodeID, int value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return;
    }

  n->GetParentParametersNode()->
    SetSmoothingKernelWidth(value);  
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeSmoothingKernelSigma(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }
  return n->GetParentParametersNode()->
    GetSmoothingKernelSigma();  
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeSmoothingKernelSigma(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return;
    }

  n->GetParentParametersNode()->
    SetSmoothingKernelSigma(value);  
}

//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetBiasCorrectionType(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }
  return n->GetParentParametersNode()->
    GetBiasCorrectionType();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetBiasCorrectionType(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return;
    }

  n->GetParentParametersNode()->
    SetBiasCorrectionType(value);
}


//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeClassProbability(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL)
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID );
      return 0;
    }
  return n->GetClassProbability();  
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
  n->SetClassProbability(value);  
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
      (fabs(1- this->GetTreeNodeChildrenSumClassProbability(*i) ) > 0.0005))
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
  return n->GetSpatialPriorWeight();
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
  n->SetSpatialPriorWeight(value);
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
  return n->GetInputChannelWeight(volumeNumber);
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
  n->SetInputChannelWeight(volumeNumber, value);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionEMType(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return -1;
    }
  return n->GetParentParametersNode()->GetStopEMType();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionEMType(vtkIdType nodeID, 
                                   int conditionType)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetStopEMType(conditionType);
}
  
//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionEMValue(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }
  return n->GetParentParametersNode()->GetStopEMValue();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionEMValue(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->SetStopEMValue(value);
}
  
//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionEMIterations(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return 0;
    }
  return n->GetParentParametersNode()->
    GetStopEMMaxIterations();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionEMIterations(vtkIdType nodeID,
                                         int iterations)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetStopEMMaxIterations(iterations);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionMFAType(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode()) 
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return -1;
    }
  return n->GetParentParametersNode()->GetStopMFAType();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionMFAType(vtkIdType nodeID, 
                                    int conditionType)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode()) 
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node" );
      return;
    }
  n->GetParentParametersNode()->
    SetStopMFAType(conditionType);
}
  
//----------------------------------------------------------------------------
double
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionMFAValue(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode()) 
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return 0;
    }
  return n->GetParentParametersNode()->GetStopMFAValue();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionMFAValue(vtkIdType nodeID, double value)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode()) 
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return;
    }
  n->GetParentParametersNode()->SetStopMFAValue(value);
}
  
//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetTreeNodeStoppingConditionMFAIterations(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode()) 
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return 0;
    }
  return n->GetParentParametersNode()->
    GetStopMFAMaxIterations();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeStoppingConditionMFAIterations(vtkIdType nodeID,
                                          int iterations)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
      return;
    }
  n->GetParentParametersNode()->
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
  char* atlasVolumeName = n->GetSpatialPriorVolumeName();
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
      if (n->GetSpatialPriorVolumeName())
    {
          n->SetSpatialPriorVolumeName(NULL);
    }
      else 
    {
      // Did not change anything
      return;
    }
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
      int modifiedFlag = this->GetAtlasInputNode()->AddVolume(priorVolumeName.c_str(), volumeMRMLID);

      if (!n->GetSpatialPriorVolumeName() || strcmp(n->GetSpatialPriorVolumeName(),priorVolumeName.c_str()))
    {
      // set name of atlas volume in tree node
      n->SetSpatialPriorVolumeName(priorVolumeName.c_str());
      modifiedFlag = 1;
    }
      // Nothing has changed so do not change status of ValidFlag 
      if (!modifiedFlag) {return;} 
    }

  // aligned atlas is no longer valid
  // 
  this->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);
}


//----------------------------------------------------------------------------
vtkIdType
vtkEMSegmentMRMLManager::
GetTreeNodeSubParcellationVolumeID(vtkIdType nodeID)
{
  vtkMRMLEMSTreeParametersLeafNode* n = this->GetTreeParametersLeafNode(nodeID) ;
  if (n == NULL)
    {
      vtkErrorMacro("Leaf node  is null for nodeID: " << nodeID);
      return ERROR_NODE_VTKID;
    }

  // get name of subParcellation volume from tree node
  char* subParcellationVolumeName = n->GetSubParcellationVolumeName();
  if (subParcellationVolumeName == NULL || strlen(subParcellationVolumeName) == 0)
    {
      return ERROR_NODE_VTKID;
    }

  // get MRML volume ID from atas node
  const char* mrmlVolumeNodeID = 
    this->GetSubParcellationInputNode()->GetVolumeNodeIDByKey(subParcellationVolumeName);
  
  if (mrmlVolumeNodeID == NULL || strlen(subParcellationVolumeName) == 0)
    {
      vtkErrorMacro("MRMLID for subparcellation volume is null; nodeID=" << nodeID);
      return ERROR_NODE_VTKID;
    }
  else if (this->IDMapContainsMRMLNodeID(mrmlVolumeNodeID))
    {
      // convert mrml id to vtk id
      return this->MapMRMLNodeIDToVTKNodeID(mrmlVolumeNodeID);
    }
  else
    {
      vtkErrorMacro("Volume MRML ID was not in map! subParcellationVolumeName = " 
            << subParcellationVolumeName << " mrmlID = " << mrmlVolumeNodeID);
      return ERROR_NODE_VTKID;
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTreeNodeSubParcellationVolumeID(vtkIdType nodeID, 
                   vtkIdType volumeID)
{
  vtkMRMLEMSTreeParametersLeafNode* n = this->GetTreeParametersLeafNode(nodeID) ;
  if (n == NULL)
    {
      vtkErrorMacro("Parameter Leaf node is null for nodeID: " << nodeID);
      return;
    }

  if (volumeID == -1)
    {
      if (n->GetSubParcellationVolumeName())
    {
      n->SetSubParcellationVolumeName(NULL);
    }
      else 
    {
      // Did not change anything
      return;
    }
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
    
      // add key value pair to subParcellation
      int modifiedFlag = this->GetSubParcellationInputNode()->AddVolume(priorVolumeName.c_str(), volumeMRMLID);

      if (!n->GetSubParcellationVolumeName() || strcmp(n->GetSubParcellationVolumeName(),priorVolumeName.c_str()))
    {
      // set name of subParcellation volume in tree node
      n->SetSubParcellationVolumeName(priorVolumeName.c_str());
      modifiedFlag = 1;
    }
      // Nothing has changed so do not change status of ValidFlag 
      if (!modifiedFlag) {return;} 
    }

  // aligned subParcellation is no longer valid
  // 
  this->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode*  vtkEMSegmentMRMLManager::GetAlignedSubParcellationFromTreeNodeID(vtkIdType nodeID)
{
  if (! this->GetTreeNodeIsLeaf(nodeID) || !this->GetTreeParametersLeafNode(nodeID)) 
    {
      return NULL;
    }

  vtkMRMLEMSVolumeCollectionNode* workingParcellation = this->GetWorkingDataNode()->GetAlignedSubParcellationNode();
  if (!workingParcellation)
    {
      return NULL;
    }

  std::string parcellationVolumeKey = this->GetTreeParametersLeafNode(nodeID)->GetSubParcellationVolumeName() ? this->GetTreeParametersLeafNode(nodeID)->GetSubParcellationVolumeName() : "";
  int parcellationVolumeIndex       = workingParcellation->GetIndexByKey(parcellationVolumeKey.c_str());
  if (parcellationVolumeIndex >= 0 )
    {
      return workingParcellation->GetNthVolumeNode(parcellationVolumeIndex);   
    }
  return NULL;
}


//----------------------------------------------------------------------------  
int vtkEMSegmentMRMLManager::GetEnableSubParcellation()
{
  return (this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->GetEnableSubParcellation() : 0);
}

//----------------------------------------------------------------------------  
void
vtkEMSegmentMRMLManager::SetEnableSubParcellation(int enable)
{
  if (this->GetGlobalParametersNode())
    {
      this->GetGlobalParametersNode()->SetEnableSubParcellation(enable);
    }
}


//----------------------------------------------------------------------------  
int vtkEMSegmentMRMLManager::GetMinimumIslandSize()
{
  return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->GetMinimumIslandSize() : 0;
}

//----------------------------------------------------------------------------  
void
vtkEMSegmentMRMLManager::SetMinimumIslandSize(int value)
{
  if (this->GetGlobalParametersNode())
    {
      this->GetGlobalParametersNode()->SetMinimumIslandSize(value);
    }
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
              "Template Node is nonnull");
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

  std::cout << "Old number of images: " << targetOldNumImages << std::endl;
  std::cout << "New number of images: " << targetNewNumImages << std::endl;

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
      std::cout << "removing an image: " << targetOldNumImages-1-i 
            << std::endl;
      this->PropogateRemovalOfSelectedTargetImage(targetOldNumImages-1-i);
    }
    }

  // aligned targets are no longer valid
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
  if (!this->GetTargetInputNode()) 
    {
      vtkErrorMacro("No TargetInputNode defined "); 
      return; 
    }
  this->GetTargetInputNode()->AddVolume(name.c_str(), mrmlID);

  // aligned targets are no longer valid
  if (!this->GetWorkingDataNode()) 
    {
      vtkErrorMacro("No WorkingDataNode defined "); 
      return; 
    }
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
  this->RemoveTargetSelectedVolumeIndex(vtkIdType(imageIndex));
}

void  vtkEMSegmentMRMLManager::RemoveTargetSelectedVolumeIndex(vtkIdType imageIndex)
{
  
  // remove from target
  this->GetTargetInputNode()->RemoveNthVolume(imageIndex);

  // aligned targets are no longer valid
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

  // aligned targets are no longer valid
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
DoTargetAndAtlasDataTypesMatch(vtkMRMLEMSVolumeCollectionNode* targetNode, vtkMRMLEMSAtlasNode* atlasNode)
{
  if (targetNode == NULL)
    {
      std::cout << "WARNING: Target node is null!" << std::endl;
      return false;
    }

  if (atlasNode == NULL)
    {
      std::cout << "WARNING: Atlas node is null!" << std::endl;
      return false;
    }

  if (targetNode->GetNumberOfVolumes() == 0)
    {
      std::cout << "Target node is empty!" << std::endl;
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
      std::cout << "Target volume " << i << ": scalar type does not match!" 
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
      std::cout << "Atlas volume " << i << ": scalar type does not match!" 
            << std::endl;
      return false;
    }
    }

  return true;
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
  const char* volumeName = this->GetGlobalParametersNode()->GetRegistrationAtlasVolumeKey();

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
  //  cout << "SetRegistrationAtlasVolumeID " << volumeID << endl;

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
GetRegistrationPackageType()
{
  return this->GetGlobalParametersNode() ?
    this->GetGlobalParametersNode()->GetRegistrationPackageType() : 0;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetRegistrationPackageType(int packageType)
{
  if (this->GetGlobalParametersNode())
    {
      this->GetGlobalParametersNode()->
    SetRegistrationPackageType(packageType);
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
GetColorNodeID()
{
  if (!this->GetGlobalParametersNode()) 
    {
      return NULL;
    }  

  const char* colorID = this->GetGlobalParametersNode()->GetColormap();
  if (colorID)
    {
      if (this->MRMLScene->GetNodeByID(colorID))
    { 
      return colorID;
    }
    }

  vtkSlicerColorLogic *colLogic = vtkSlicerColorLogic::New();
  // It is important that the color node exists otherwise we get wired errors - I learned the hard way!
  this->GetGlobalParametersNode()->SetColormap(colLogic->GetDefaultLabelMapColorNodeID());    
  colLogic->Delete();
  return this->GetGlobalParametersNode()->GetColormap();  
}
 
//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetColorNodeID(const char* colormap)
{
  if (this->GetGlobalParametersNode())
    {
      this->GetGlobalParametersNode()->SetColormap(colormap);  
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

  if (!this->Node || !this->GetGlobalParametersNode() )
    {
      return NULL;
    }
  return this->GetGlobalParametersNode()->GetTemplateFileName();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetSaveTemplateFilename(const char* file)
{
  if (this->Node && this->GetGlobalParametersNode())
    {
      this->GetGlobalParametersNode()->SetTemplateFileName(file);
    }
  else
    {
      vtkErrorMacro("Attempt to access null EM node or GlobalParametersNode.");
    }
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
GetSaveTemplateAfterSegmentation()
{
  if (this->Node  && this->GetGlobalParametersNode())
    {
      return this->GetGlobalParametersNode()->GetTemplateSaveAfterSegmentation();
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
  if (this->Node  && this->GetGlobalParametersNode())
    {
      this->GetGlobalParametersNode()->SetTemplateSaveAfterSegmentation(shouldSave);
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
  if (!this->GetWorkingDataNode())
    {
      if (this->Node)
    {
      vtkWarningMacro("Can't get WorkingData and EMSTemplateNode is nonnull.");
    }
      return NULL;
    }
  return this->GetWorkingDataNode()->GetOutputSegmentationNodeID();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetOutputVolumeMRMLID(const char* mrmlID)
{
  if (!this->GetWorkingDataNode())
    {
      if (this->Node)
    {
      vtkWarningMacro("Can't get WorkingData and EMSTemplateNode is nonnull.");
    }
      return;
    }
  else
    {
      this->GetWorkingDataNode()->SetOutputSegmentationNodeID(mrmlID);
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


//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
ComputeAtlasNumberOfTrainingSamples()
{
  cout << "vtkEMSegmentMRMLManager::ComputeAtlasNumberOfTrainingSamples: Start" << endl;

  vtkMRMLEMSAtlasNode *atlasInputNode = this->GetAtlasInputNode();
  if (atlasInputNode == NULL)
    {
      return;
    }

  if (!this->GetWorkingDataNode()->GetAlignedTargetNode() || !this->GetWorkingDataNode()->GetAlignedTargetNodeIsValid())
    {
      atlasInputNode->SetNumberOfTrainingSamples(0);
      return;
    }

  int maxNum = 0;
  int setFlag = 0;

  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;

  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
      if (this->GetTreeNodeIsLeaf(*i)) 
        {  
      std::string atlasVolumeKey =  this->GetTreeNode(*i)->GetSpatialPriorVolumeName() ? this->GetTreeNode(*i)->GetSpatialPriorVolumeName() : "";
      int atlasVolumeIndex       = atlasInputNode->GetIndexByKey(atlasVolumeKey.c_str());
      if (atlasVolumeIndex >= 0)
        {
          vtkImageData* imageData = atlasInputNode->GetNthVolumeNode(atlasVolumeIndex)->GetImageData();
          if (imageData)
        {
          double range[2];
          imageData->GetScalarRange(range);
          cout << "Max of " << atlasInputNode->GetNthVolumeNode(atlasVolumeIndex)->GetName() << ": " << range[1] << endl;
          if (!setFlag ||  int(range[1]) > maxNum) 
            {
              maxNum = int(range[1]);
              setFlag = 1;
            }
        }
        }
       }
    }
  if (!setFlag) 
    {
      // Just set it to 1 so that it does not create problems later when running the EMSegmenter
      maxNum =1 ;
    }
    atlasInputNode->SetNumberOfTrainingSamples(maxNum);
    cout << "New NumberOfTrainingSamples: " << maxNum << endl;
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
  if (!this->Node )
    {
      vtkWarningMacro("Null TemplateNode");
      return NULL;
    }
  else
    {
    return this->Node->GetGlobalParametersNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkEMSegmentMRMLManager::
GetTreeRootNode()
{
    if (!this->Node)
    {
      vtkWarningMacro("Null TemplateNode.");
      return NULL;
    }
  else
    {
    return this->Node->GetTreeNode();
    }
}

//----------------------------------------------------------------------------
bool vtkEMSegmentMRMLManager::TreeNodeExists(vtkIdType nodeID)
{
  if (nodeID == -1)
    {
    return false;
    }

  bool contained = this->VTKNodeIDToMRMLNodeIDMap.count(nodeID);
  if (!contained)
    {
      return false;
    }

  if (this->VTKNodeIDToMRMLNodeIDMap[nodeID].empty())
    {
      return false;
    }
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLEMSTreeNode*
vtkEMSegmentMRMLManager::
GetTreeNode(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* node = NULL;

  if (nodeID == -1)
    {
    return NULL;
    }

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
vtkMRMLEMSTreeParametersLeafNode*
vtkEMSegmentMRMLManager::
GetTreeParametersLeafNode(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* node = this->GetTreeNode(nodeID);
  if (node == NULL )
    {
    vtkWarningMacro("Tree parameters node is null for node id: " << nodeID);
    return NULL;
    }
  return node->GetLeafParametersNode();
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
vtkMRMLEMSVolumeCollectionNode*
vtkEMSegmentMRMLManager::
GetTargetInputNode()
{
  vtkMRMLEMSWorkingDataNode* workingDataNode = this->GetWorkingDataNode();
  if (workingDataNode == NULL)
    {
    if (this->Node)
      {
      vtkWarningMacro("Null WorkingDataNode with nonnull EMSTemplateNode.");
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
  if (!this->Node)
      {
      vtkWarningMacro("Null EMSTemplateNode.");
      return NULL;
    }
  else
    {
    return this->Node->GetSpatialAtlasNode();
    }
}


//----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode* vtkEMSegmentMRMLManager::GetAtlasAlignedNode()
{
  if (!this->GetWorkingDataNode() ) {
      vtkWarningMacro("Null AtlasAlignedNode.");
      return NULL;
    }
  
  return  this->GetWorkingDataNode()->GetAlignedAtlasNode();
}

//----------------------------------------------------------------------------
vtkMRMLEMSVolumeCollectionNode* vtkEMSegmentMRMLManager::GetSubParcellationInputNode()
{
  if (!this->Node)
    {
      vtkWarningMacro("Null EMSTemplateNode.");
      return NULL;
    }
  else
    {
    return this->Node->GetSubParcellationNode();
    }
}


//----------------------------------------------------------------------------
vtkMRMLEMSWorkingDataNode* vtkEMSegmentMRMLManager::GetWorkingDataNode()
{

  if (!this->Node)
    {
      return NULL;
    }
  else
    {
      return this->Node->GetEMSWorkingDataNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkEMSegmentMRMLManager::
GetOutputVolumeNode()
{

  vtkMRMLEMSWorkingDataNode* workingNode = this->GetWorkingDataNode();
  if (workingNode == NULL)
    {
      return NULL;
    }
  else
    {
    return  workingNode->GetOutputSegmentationNode();
    }
}

//----------------------------------------------------------------------------
vtkMRMLEMSAtlasNode* vtkEMSegmentMRMLManager::CloneAtlasNode(vtkMRMLEMSAtlasNode* atlasNode, const char* name)
{
  if (atlasNode == NULL)
    {
    return NULL;
    }

  // clone the atlas node
  vtkMRMLEMSAtlasNode* clonedAtlas = vtkMRMLEMSAtlasNode::New();
  clonedAtlas->CopyWithScene(atlasNode);
  vtksys_stl::string postFix = vtksys_stl::string("(") +  vtksys_stl::string(name) + vtksys_stl::string(")");
  vtksys_stl::string newName = vtksys_stl::string(atlasNode->GetName()) + postFix; 
  clonedAtlas->SetName(newName.c_str());
  clonedAtlas->CloneVolumes(atlasNode,postFix.c_str());

  // add the atlas node to the scene
  this->MRMLScene->AddNode(clonedAtlas);

  // clean up
  clonedAtlas->Delete();

  return clonedAtlas;
}

//----------------------------------------------------------------------------
vtkMRMLEMSVolumeCollectionNode* vtkEMSegmentMRMLManager::CloneVolumeCollectionNode(vtkMRMLEMSVolumeCollectionNode* collectionNode, const char* name)
{
  if (collectionNode == NULL)
    {
    return NULL;
    }

  // clone the atlas node
  vtkMRMLEMSVolumeCollectionNode* clonedCollectionNode = vtkMRMLEMSVolumeCollectionNode::New();
  clonedCollectionNode->CopyWithScene(collectionNode);
  vtksys_stl::string postFix = vtksys_stl::string("(") +  vtksys_stl::string(name) + vtksys_stl::string(")");
  vtksys_stl::string newName = vtksys_stl::string(collectionNode->GetName()) + postFix; 
  clonedCollectionNode->SetName(newName.c_str());
  clonedCollectionNode->CloneVolumes(collectionNode,postFix.c_str());

  // add the subParcellation node to the scene
  this->MRMLScene->AddNode(clonedCollectionNode);

  // clean up
  clonedCollectionNode->Delete();

  return clonedCollectionNode;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SynchronizeAtlasNode(vtkMRMLEMSAtlasNode* templateNode, 
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
   vtksys_stl::string postFix = vtksys_stl::string("(") +  vtksys_stl::string(name) + vtksys_stl::string(")");
   vtksys_stl::string newName = vtksys_stl::string(templateNode->GetName()) + postFix; 

   changingNode->SetName(newName.c_str());
   changingNode->CloneVolumes(templateNode,postFix.c_str());

  // copy over number of training samples
  changingNode->SetNumberOfTrainingSamples
    (const_cast<vtkMRMLEMSAtlasNode*>(templateNode)->
     GetNumberOfTrainingSamples());
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::SynchronizeVolumeCollectionNode(vtkMRMLEMSVolumeCollectionNode* templateNode,  vtkMRMLEMSVolumeCollectionNode* changingNode, const char* name)
{
  if (templateNode == NULL || changingNode == NULL)
    {
      vtkWarningMacro("Attempt to synchronize subParcellation with null node!");
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
  vtksys_stl::string postFix = vtksys_stl::string("(") +  vtksys_stl::string(name) + vtksys_stl::string(")");
  vtksys_stl::string newName = vtksys_stl::string(templateNode->GetName()) + postFix; 
  changingNode->SetName(newName.c_str());
  changingNode->CloneVolumes(templateNode,postFix.c_str());
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

  // Before returning number makesure that there is new legacy code 
  this->RemoveLegacyNodes();   
  // Now return number 
  return this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLEMSTemplateNode");
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
    this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLEMSTemplateNode");

  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth template builder node in scene: " << n);
    return NULL;
    }

  return node->GetName();
}

//----------------------------------------------------------------------------
void  vtkEMSegmentMRMLManager::SetNthParameterName(int n, const char* newName)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("MRML scene is NULL.");
    return;
    }

  vtkMRMLNode* node = 
    this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLEMSTemplateNode");

  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth template builder node in scene: " << n);
    return; 
    }

   node->SetName(newName);
}


//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
SetLoadedParameterSetIndex(int n)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("MRML scene is NULL.");
    return 1;
    }

  // this always has to be called before calling the function 
  vtkMRMLNode* tNode = this->GetMRMLScene()->GetNthNodeByClass(n, "vtkMRMLEMSTemplateNode");

  if (tNode == NULL)
    {
    vtkErrorMacro("Did not find nth template builder node in scene: " << n);
    return 1;
    }


  vtkMRMLEMSTemplateNode* templateBuilderNode = vtkMRMLEMSTemplateNode::SafeDownCast(tNode);
  return this->SetLoadedParameterSetIndex(templateBuilderNode);
}

//----------------------------------------------------------------------------
int
vtkEMSegmentMRMLManager::
SetLoadedParameterSetIndex(vtkMRMLEMSTemplateNode *templateBuilderNode)
{

  if (templateBuilderNode == NULL)
    {
      vtkErrorMacro("No template node defined"); 
      return 1;
    }

  this->CompleteTemplateMRMLStructureForGUI(templateBuilderNode);
   
  // Check if all the volume data in the EMSegmenter tree is non-null 
  if (this->CheckEMSTemplateVolumeNodes(templateBuilderNode)) 
    {
       vtkErrorMacro("EMSegment related volume nodes are corrupted for node: " << templateBuilderNode->GetID());
       return 1;
    }

  return this->SetNodeWithCheck(templateBuilderNode);
}

//----------------------------------------------------------------------------
int vtkEMSegmentMRMLManager::CheckEMSTemplateVolumeNodes(vtkMRMLEMSTemplateNode* emsTemplateNode)
{
    if (emsTemplateNode == NULL)
    {
       vtkErrorMacro("EMS node is NULL !"); 
       return 1;
    }

   vtkMRMLScene*   currentScene = this->GetMRMLScene();
   if (currentScene == NULL)
    {
        vtkErrorMacro("Current scene not set !"); 
        return 1 ;
    }

    vtkMRMLScene *emsScene =  vtkMRMLScene::New();
    currentScene->GetReferencedSubScene(emsTemplateNode, emsScene);

    emsScene->InitTraversal();
    vtkMRMLNode* currentNode;

    int errorFlag = 0;
    while ((currentNode = emsScene->GetNextNodeByClass("vtkMRMLVolumeNode")) && (currentNode != NULL) && !errorFlag )
    {
       vtkMRMLVolumeNode* volumeNode = dynamic_cast<vtkMRMLVolumeNode*>(currentNode);
       if (volumeNode == NULL)
       {
          vtkErrorMacro("Volume node is null for node: " << volumeNode->GetID());
          errorFlag = 1;
          continue;
      }

      vtkImageData *volumeData = volumeNode->GetImageData();
      if (volumeData == NULL)
      {
          vtkErrorMacro("Image data is null for volume node: " << volumeNode->GetID() << " Name : " <<  (volumeNode->GetName() ? volumeNode->GetName(): "(none)" ));
          errorFlag = 1;
          continue;
      }

      int* dim = volumeData->GetDimensions();
      if (!(dim[0]*dim[1]))
    {
          vtkErrorMacro("Image data is null for volume node: " << volumeNode->GetID() << " Name : " <<  (volumeNode->GetName() ? volumeNode->GetName(): "(none)" ));
          errorFlag = 1;
          continue;
    }
    }

    emsScene->Delete();
    return errorFlag;
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

  // create template node
  vtkMRMLEMSTemplateNode* templateNode = vtkMRMLEMSTemplateNode::New();
  templateNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(templateNode);

  //
  // Create Global and tree nodes 
  //
  {
  vtkMRMLEMSGlobalParametersNode* globalParametersNode = vtkMRMLEMSGlobalParametersNode::New();
    globalParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
    this->GetMRMLScene()->AddNode(globalParametersNode);
    templateNode->SetGlobalParametersNodeID(globalParametersNode->GetID());
    globalParametersNode->Delete();


  // create root tree node
  vtkMRMLEMSTreeNode* treeNode = vtkMRMLEMSTreeNode::New();
    treeNode->SetHideFromEditors(this->HideNodesFromEditors);
    this->GetMRMLScene()->AddNode(treeNode); 
    templateNode->SetTreeNodeID(treeNode->GetID());
  
    {

         vtkMRMLEMSTreeParametersLeafNode* leafParametersNode =  vtkMRMLEMSTreeParametersLeafNode::New();
          leafParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
          this->GetMRMLScene()->AddNode(leafParametersNode);
          treeNode->SetLeafParametersNodeID(leafParametersNode->GetID());
          leafParametersNode->Delete();

         vtkMRMLEMSTreeParametersParentNode* parentParametersNode = vtkMRMLEMSTreeParametersParentNode::New();
          parentParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
          this->GetMRMLScene()->AddNode(parentParametersNode);
          treeNode->SetParentParametersNodeID(parentParametersNode->GetID());
          parentParametersNode->Delete();

    }
    treeNode->Delete();
  }
  // Completes the other necessary nodes for GUI 
  this->CompleteTemplateMRMLStructureForGUI(templateNode); 

  // Link to MRML Manager
  this->SetNodeWithCheck(templateNode);
  templateNode->Delete();

  // add basic information for root node
  vtkIdType rootID = this->GetTreeRootNodeID();
  this->SetTreeNodeName(rootID, "Root");
  this->SetTreeNodeIntensityLabel(rootID, rootID);
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

  // New node is always a child so no parent node needed
//  vtkMRMLEMSTreeParametersParentNode* parentParametersNode = 
//    vtkMRMLEMSTreeParametersParentNode::New();
//  parentParametersNode->SetHideFromEditors(this->HideNodesFromEditors);
//  this->GetMRMLScene()->AddNode(parentParametersNode);

  // create tree node and add it to the scene
  vtkMRMLEMSTreeNode* treeNode = vtkMRMLEMSTreeNode::New();
  treeNode->SetHideFromEditors(this->HideNodesFromEditors);
  this->GetMRMLScene()->AddNode(treeNode); // this adds id pair to map

  // add connections  
  treeNode->SetLeafParametersNodeID(leafParametersNode->GetID());

  // update memory -leaf node must be defined before so that the following step is propagated 
  int numInputChannel = this->GetTargetInputNode()->GetNumberOfVolumes();
  treeNode->SetNumberOfTargetInputChannels(numInputChannel);


  // set the intensity label (in resulting segmentation) to the ID
  vtkIdType treeNodeID = this->MapMRMLNodeIDToVTKNodeID(treeNode->GetID());
  this->SetTreeNodeIntensityLabel(treeNodeID, treeNodeID);
  
  // delete nodes
  leafParametersNode->Delete();
  // parentParametersNode->Delete();
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

    // remove leaf node parameters
    vtkMRMLNode* leafParametersNode = n->GetLeafParametersNode();
    if (leafParametersNode != NULL)
      {
      this->GetMRMLScene()->RemoveNode(leafParametersNode);
      }

    // remove parent node parameters
    vtkMRMLNode* parentParametersNode = n->GetParentParametersNode();
    if (parentParametersNode != NULL)
      {
      this->GetMRMLScene()->RemoveNode(parentParametersNode);
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
  if (this->TreeNodeExists(nodeID)) 
    {
      return this->VTKNodeIDToMRMLNodeIDMap[nodeID].c_str();  
    }
   vtkErrorMacro("vtk ID ("<< nodeID << ") does not map to mrml ID or to empry mrlm ID" );
  return NULL;
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
  if (rootNodeID == 0)
  {
    vtkErrorMacro("rootNodeID is zero");
    return;
  }

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
  if ( this->GetGlobalParametersNode()->GetNumberOfTargetInputChannels() < this->GetTargetInputNode()->GetNumberOfVolumes() ) {
    this->GetGlobalParametersNode()->AddTargetInputChannel();
  }

  // iterate over tree nodes
  typedef vtkstd::vector<vtkIdType>  NodeIDList;
  typedef NodeIDList::const_iterator NodeIDListIterator;
  NodeIDList nodeIDList;
  this->GetListOfTreeNodeIDs(this->GetTreeRootNodeID(), nodeIDList);
  for (NodeIDListIterator i = nodeIDList.begin(); i != nodeIDList.end(); ++i)
    {
      if (int(this->GetTreeNode(*i)->GetNumberOfTargetInputChannels()) < int(this->GetTargetInputNode()->GetNumberOfVolumes()) ) {
    this->GetTreeNode(*i)->AddTargetInputChannel();
      }
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
    this->GetTreeNode(*i)->
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
    this->GetTreeNode(*i)->
      MoveNthTargetInputChannel(fromIndex, toIndex);
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
RegisterMRMLNodesWithScene()
{
  if(!this->GetMRMLScene())
    {
    return;
    } 

  // -----------------------
  // For legacy purpose 
  // -----------------------
  vtkMRMLEMSNode* emsNode = 
    vtkMRMLEMSNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsNode);
  emsNode->Delete();

  vtkMRMLEMSSegmenterNode* emsSegmenterNode = 
    vtkMRMLEMSSegmenterNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsSegmenterNode);
  emsSegmenterNode->Delete();

  vtkMRMLEMSTargetNode* emsTargetNode = vtkMRMLEMSTargetNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsTargetNode);
  emsTargetNode->Delete();

  // -----------------------
  // Updated Structure 
  // -----------------------

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

  vtkMRMLEMSVolumeCollectionNode* emsVolumeCollectionNode =  vtkMRMLEMSVolumeCollectionNode::New();
  this->GetMRMLScene()->RegisterNodeClass(emsVolumeCollectionNode);
  emsVolumeCollectionNode->Delete();  

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
    vtkMRMLNode* node = this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLEMSTreeNode");

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

void vtkEMSegmentMRMLManager::CompleteTemplateMRMLStructureForGUI(vtkMRMLEMSTemplateNode *emsTemp)
{
   if (!emsTemp)
    {
    vtkErrorMacro("Template builder node is NULL.");
    return;
    }

  if (!emsTemp->GetSpatialAtlasNode())
    {
      vtkMRMLEMSAtlasNode* aNode = vtkMRMLEMSAtlasNode::New();
      aNode->SetHideFromEditors(this->HideNodesFromEditors);
      this->GetMRMLScene()->AddNode(aNode);
      emsTemp->SetSpatialAtlasNodeID(aNode->GetID());
      aNode->Delete();
    }

  if (!emsTemp->GetSubParcellationNode())
   {
      vtkMRMLEMSVolumeCollectionNode* sNode = vtkMRMLEMSVolumeCollectionNode::New();
      sNode->SetHideFromEditors(this->HideNodesFromEditors);
      this->GetMRMLScene()->AddNode(sNode);
      emsTemp->SetSubParcellationNodeID(sNode->GetID());
      sNode->Delete();
   }

   // Jump over tree and global parameters node -> bc if those are not defined then it is not a valid structure 

   if (!emsTemp->GetEMSWorkingDataNode())
   {
      vtkMRMLEMSWorkingDataNode* wNode = vtkMRMLEMSWorkingDataNode::New();
      wNode->SetHideFromEditors(this->HideNodesFromEditors);
      this->GetMRMLScene()->AddNode(wNode);
      emsTemp->SetEMSWorkingDataNodeID(wNode->GetID());
      wNode->Delete();
   }

   if (!emsTemp->GetEMSWorkingDataNode()->GetInputTargetNode())
   {
      vtkMRMLEMSVolumeCollectionNode* tNode = vtkMRMLEMSVolumeCollectionNode::New();
      tNode->SetHideFromEditors(this->HideNodesFromEditors);
      this->GetMRMLScene()->AddNode(tNode);
      emsTemp->GetEMSWorkingDataNode()->SetInputTargetNodeID(tNode->GetID());
      tNode->Delete();
   }

}


int vtkEMSegmentMRMLManager::CheckTemplateMRMLStructure(vtkMRMLEMSTemplateNode *emsTemp, int guiFlag)
{
  //
  // check global attributes
  //

  // check template builder node
  if (emsTemp == NULL)
    {
    vtkErrorMacro("Template builder node is NULL.");
    return 0;
    }

  // 
  // check Template node
  //

  // check atlas node
  vtkMRMLEMSAtlasNode *atlasNode = emsTemp->GetSpatialAtlasNode(); 
  if (atlasNode == NULL)
    {
    vtkErrorMacro("Atlas node is NULL.");
    return 0;
    }

  vtkMRMLEMSVolumeCollectionNode *subParcellationNode = emsTemp->GetSubParcellationNode();
  if (subParcellationNode == NULL)
   {
      vtkErrorMacro("SubParcellation node is NULL.");
      return 0;
   }

  // check global parameters node
  vtkMRMLEMSGlobalParametersNode* globalParametersNode = emsTemp->GetGlobalParametersNode();
  if (globalParametersNode == NULL)
    {
    vtkErrorMacro("Global parameters node is NULL.");
    return 0;
    }

  // check tree
  vtkMRMLEMSTreeNode* rootNode = emsTemp->GetTreeNode(); 
  if (rootNode == NULL)
    {
    vtkErrorMacro("Root node of tree is NULL.");
    return 0;
    }
 
  // check working data node
  vtkMRMLEMSWorkingDataNode *workingNode = emsTemp->GetEMSWorkingDataNode();
  if (workingNode == NULL)
  {
        vtkErrorMacro("Working data node is NULL.");
        return 0;
  }

  // check target node
  vtkMRMLEMSVolumeCollectionNode *targetInputNode = workingNode->GetInputTargetNode() ; 
  if (targetInputNode == NULL)
  {
         vtkErrorMacro("Target node is NULL.");
         return 0;
  }

  //
  // Check nodes needed for processing template 
  //
  if (!guiFlag)
    {
       // check output volume
      vtkMRMLScalarVolumeNode *outVolume = workingNode->GetOutputSegmentationNode();
      if (outVolume == NULL)
      {
        vtkErrorMacro("Output Segmentation is NULL.");
        return 0;
      }

      // check that the number of target nodes is consistent
      if (globalParametersNode->GetNumberOfTargetInputChannels() != targetInputNode->GetNumberOfVolumes())
       {
          vtkErrorMacro("Inconsistent number of input channles. Target="
                  << targetInputNode->GetNumberOfVolumes()
                  << " Global Parameters="
                  << globalParametersNode->GetNumberOfTargetInputChannels());
        return 0;    
       }

      for (int i=0;  i < targetInputNode->GetNumberOfVolumes(); i++)
      {
        vtkMRMLVolumeNode* cNode = targetInputNode->GetNthVolumeNode(i);
        if (!cNode)
         {
           cout << "Input Channel Error: Please assign an volume to each input channel" << endl;
           return 0; 
         }
        if (!cNode->GetImageData()|| !cNode->GetImageData()->GetDimensions()[0]) 
         {
           cout << "Input Channel Error: Volume of " << i + 1 << "th Input channel is empty or of size zero !" << endl;
           return 0; 
         }
       if (cNode->GetImageData()->GetScalarRange()[0] < 0 )
         {
           cout << "WARNING: Input Channel warning: Volume " << i + 1 << " contains negative values - negative values will be set to 0 !" << endl;
         }
      }
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
  const char* name = this->GetTreeNodeName(rootID);

  if (rnode == NULL)
    {
    vtkstd::cout << indent << "Node is null for id=" << rootID << std::endl;
    return;
    }


    vtkstd::cout << indent << "Name: " << (name ? name : "(null)") 
                 << vtkstd::endl;
    vtkstd::cout << indent << "ID: "    << rootID 
                 << " MRML ID: " << rnode->GetID()
                 << " From Map: " << mrmlID << vtkstd::endl;
    vtkstd::cout << indent << "AlignedSpatialAtlas: "   ;
    vtkMRMLVolumeNode* alignedSpatial = this->GetAlignedSpatialPriorFromTreeNodeID(rootID);
    if (alignedSpatial && alignedSpatial->GetName()) {
      vtkstd::cout <<  alignedSpatial->GetName() << endl;
    } else {
      vtkstd::cout << "None" << endl;
    }

    vtkstd::cout << indent << "Is Leaf: " << this->GetTreeNodeIsLeaf(rootID)  << vtkstd::endl;
   
    if (!this->GetTreeNodeIsLeaf(rootID) )
      {  
      int numChildren = this->GetTreeNodeNumberOfChildren(rootID); 
  
      vtkstd::cout << indent << "Num. Children: " << numChildren << vtkstd::endl;
      vtkstd::cout << indent << "Child IDs from parent: ";
      for (int i = 0; i < numChildren; ++i)
        {
        vtkstd::cout << rnode->GetNthChildNodeID(i) << " ";
        }
      vtkstd::cout << vtkstd::endl;
      vtkstd::cout << indent << "Child IDs from children: ";
      for (int i = 0; i < numChildren; ++i)
        {
        vtkstd::cout << rnode->GetNthChildNode(i)->GetID() << " ";
        }
      vtkstd::cout << vtkstd::endl;
  
      indent = indent.GetNextIndent();
      for (int i = 0; i < numChildren; ++i)
        {
        vtkIdType childID = this->GetTreeNodeChildNodeID(rootID, i);
        vtkstd::cout << indent << "Child " << i << " (" << childID 
                     << ") of node " << rootID << vtkstd::endl;
        this->PrintTree(childID, indent);
        }
      }

}

//-----------------------------------------------------------------------------

void
vtkEMSegmentMRMLManager::PrintVolumeInfo( vtkMRMLScene* mrmlScene)
{
  // for every volume node
  int numVolumes = mrmlScene->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
  for (int i = 0; i < numVolumes; ++i)
    {
    vtkMRMLNode* node = mrmlScene->GetNthNodeByClass(i, "vtkMRMLVolumeNode");
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
    if (!volumeNode)
      {
    continue;
      } 
    // print volume node ID and name
    vtkstd::cout << "Volume Node ID / Name / ImageData : " << volumeNode->GetID()
         << " / " << volumeNode->GetName() << " / " << (volumeNode->GetImageData() ? "Defined" : "NULL")  << vtkstd::endl;
    // print display node id
    vtkstd::cout << " Display Node ID: " 
              << (volumeNode->GetDisplayNode() ?
                  volumeNode->GetDisplayNode()->GetID() : "NULL")
              << vtkstd::endl;

    // print storage node id and filename
    vtkstd::cout << " Storage Node ID / Filename: ";
    if (volumeNode->GetStorageNode()) 
      {
    vtkstd::cout <<   (volumeNode->GetStorageNode()->GetID() ? volumeNode->GetStorageNode()->GetID() : "NULL") 
                     << " / " << (volumeNode->GetStorageNode()->GetFileName() ? volumeNode->GetStorageNode()->GetFileName() : "NULL") 
                     << vtkstd::endl;
      } 
    else  
      {
        vtkstd::cout <<  "Not Defined" << vtkstd::endl;
      }
    }
}

//-----------------------------------------------------------------------------
// this returns only the em tree - only works when Import correctly works - however I am not sure if that is the case 
int vtkEMSegmentMRMLManager::CreateTemplateFile()
{
  cout << "vtkEMSegmentMRMLManager::CreateTemplateFile()" << endl;
  if (!this->GetSaveTemplateFilename())
    {
      vtkErrorMacro("No template file name defined!");
      return 1;
    }

  std::string fileName = this->GetSaveTemplateFilename();


  if (vtksys::SystemTools::GetParentDirectory(fileName.c_str()) != vtksys::SystemTools::GetParentDirectory(this->MRMLScene->GetURL())) 
     {
       vtkErrorMacro("Directory of template file has to be the same as that of the current scene, which is " << this->MRMLScene->GetURL());
       return 1;
     } 


  //
  // Reset all the data in the EMStree that is not template specific
  // 
  this->GetMRMLScene()->SaveStateForUndo();

  // Name of EMSTemplateNode depends on filename ! 
 
  std::string mrmlName = vtksys::SystemTools::GetFilenameName(fileName.c_str());
  std::string taskName  = TurnDefaultMRMLFileIntoTaskName(mrmlName.c_str());
  this->Node->SetName(taskName.c_str()); 

  // Removing WorkingNode 
  vtkMRMLEMSWorkingDataNode *workingNode = this->GetWorkingDataNode();
  if (workingNode)
    {
      this->Node->SetEMSWorkingDataNodeID("");
    }

  if (this->GetGlobalParametersNode()) 
    {
      // This is important otherwise when you load a volume with different dimensions it creates problems 
      int maxPoint[3] = {-1, -1, -1};
      this->SetSegmentationBoundaryMax(maxPoint);

      // Unset template file as this is user specific 
      this->SetSaveTemplateFilename("");
    }

  // 
  // Extract all EM Related Nodes and copy those in the new file 
  // 
  vtkMRMLScene *cScene =  vtkMRMLScene::New();
  std::string outputDirectory = vtksys::SystemTools::GetFilenamePath(fileName);
  cScene->SetRootDirectory(outputDirectory.c_str());
  cScene->SetURL(fileName.c_str());
  this->CopyEMRelatedNodesToMRMLScene(cScene);
  cout << "Write Template to  " << fileName.c_str() << " ..." << endl;
  cScene->Commit(fileName.c_str());
  cout << "... finished" << endl;
  cScene->Delete();

  //
  // Reset Scene
  // 
  this->MRMLScene->Undo();
  return 0 ;
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
CopyEMRelatedNodesToMRMLScene(vtkMRMLScene* newScene)
{

  //
  // copy over nodes from the current scene to the new scene
  //
  vtkMRMLScene*   currentScene = this->GetMRMLScene();
  if (currentScene == NULL || this->Node == NULL)
    {
    return;
    }

  currentScene->GetReferencedSubScene(this->Node, newScene);

  return;

}

//----------------------------------------------------------------------------
bool vtkEMSegmentMRMLManager::ExistRegistrationAtlasVolumeKey(vtkIdType inputID)
{
  if (!this->GetGlobalParametersNode())
    {
    vtkErrorMacro("GlobalParametersNode is NULL.");
    return false;
    }

   const char* volumeName = this->GetGlobalParametersNode()->GetRegistrationAtlasVolumeKey(inputID);
   if (volumeName == NULL || strlen(volumeName) == 0)
     {
       return false;
     }
   return true;
}
//----------------------------------------------------------------------------
vtkIdType vtkEMSegmentMRMLManager::GetRegistrationAtlasVolumeID(vtkIdType inputID)
{
  if (!this->GetGlobalParametersNode())
    {
    vtkErrorMacro("GlobalParametersNode is NULL.");
    return ERROR_NODE_VTKID;
    }

  // the the name of the atlas image from the global parameters
  const char* volumeName = this->GetGlobalParametersNode()->GetRegistrationAtlasVolumeKey(inputID);
  if (volumeName == NULL || strlen(volumeName) == 0)
    {
    vtkWarningMacro("AtlasVolumeName is NULL/blank.");
    return ERROR_NODE_VTKID;
    }

  // get MRML ID of atlas from it's name
  const char* mrmlID = this->GetAtlasInputNode()->GetVolumeNodeIDByKey(volumeName);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
      // This means it was not set 
      return ERROR_NODE_VTKID;
      //    vtkErrorMacro("Could not find mrml ID for registration atlas volume.");
    }
  
  // convert mrml id to vtk id
  return this->MapMRMLNodeIDToVTKNodeID(mrmlID);
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetRegistrationAtlasVolumeID(vtkIdType inputID, vtkIdType volumeID)
{
  // for now there can be only one atlas image for registration
  std::stringstream registrationVolumeName;
  registrationVolumeName << "atlas_registration_image" << inputID;

  // map to MRML ID
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
  // if it is null then this means that the channel is not set 
  if (mrmlID && strlen(mrmlID))
    {
      this->GetAtlasInputNode()->AddVolume(registrationVolumeName.str().c_str(), mrmlID);
    }

  // set volume name and ID in map
  this->GetGlobalParametersNode()->SetRegistrationAtlasVolumeKey(inputID,registrationVolumeName.str().c_str());
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::SetTargetSelectedVolumeNthID(int n, vtkIdType newVolumeID)
{  
  vtkIdType oldVolumeID  = this->GetTargetSelectedVolumeNthID(n);
  if (oldVolumeID ==  ERROR_NODE_VTKID) 
    {
      vtkErrorMacro("Did not find nth target volume; n = " << n);
      return;
    }

  if (oldVolumeID == newVolumeID) 
    {
      return ;
    }

 vtkMRMLVolumeNode* volumeNode = this->GetVolumeNode(newVolumeID);
  if (volumeNode == NULL)
    {
    vtkErrorMacro("Invalid volume ID: " << newVolumeID);
    return;
    }

  // map to MRML ID
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(newVolumeID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Could not map volume ID: " << newVolumeID);
    return;
    }
 
  // set volume name and ID in map
  this->GetTargetInputNode()->SetNthVolumeNodeID(n, mrmlID);

  // aligned targets are no longer valid
  this->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);

  // propogate change to parameters nodes
  this->PropogateAdditionOfSelectedTargetImage();
  this->UpdateIntensityDistributions();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentMRMLManager::
SetTargetSelectedVolumeNthMRMLID(int n, const char* mrmlID)
{
  vtkIdType volumeID = this->MapMRMLNodeIDToVTKNodeID(mrmlID);
  this->SetTargetSelectedVolumeNthID(n,volumeID);
}

//----------------------------------------------------------------------------
double LogToNormalIntensities(double val) 
{
  return exp(val) -1.0 ;
}

// make sure the value is greater -1 - we are not checking here 
double NormalToLogIntensities(double val) 
{
  return log(val + 1.0);
}

//----------------------------------------------------------------------------
// This is for GUI
double vtkEMSegmentMRMLManager:: GetTreeNodeDistributionMeanWithCorrection(vtkIdType nodeID, int volumeNumber) {  
  double value = LogToNormalIntensities(this->GetTreeNodeDistributionLogMeanWithCorrection(nodeID, volumeNumber));
  // cout << "GetTreeNodeDistributionMeanWithCorrection :" << value << endl; 
   return value;
}

//----------------------------------------------------------------------------
// This is for Graph and Logic
double vtkEMSegmentMRMLManager::GetTreeNodeDistributionLogMeanWithCorrection(vtkIdType nodeID, int volumeNumber) {  
   if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }
   double value = this->GetTreeNodeDistributionLogMean(nodeID, volumeNumber) - this->GetTreeNodeDistributionLogMeanCorrection(nodeID, volumeNumber); 
   // cout << "GetTreeNodeDistributionLogMeanWithCorrection :" << value << endl; 
   return value;
}


//----------------------------------------------------------------------------
// This is for GUI 
void vtkEMSegmentMRMLManager::SetTreeNodeDistributionMeanWithCorrection(vtkIdType nodeID, int volumeNumber,double value) {
  if (value <= -1) {
    vtkErrorMacro("Value of Mean has to be greater -1 !" ); 
    return;
  }

 if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }

  this->SetTreeNodeDistributionLogMeanCorrection(nodeID, volumeNumber, this->GetTreeNodeDistributionLogMean(nodeID, volumeNumber) - NormalToLogIntensities(value));
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::ResetTreeNodeDistributionLogMeanCorrection(vtkIdType nodeID) 
{
  vtkMRMLEMSTreeParametersLeafNode* node = this->GetTreeParametersLeafNode(nodeID) ;
   if (node == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }

   unsigned int n = node->GetNumberOfTargetInputChannels();
   for (unsigned int i = 0; i < n; ++i)
     {
       node->SetLogMeanCorrection(i,0.0);
     }
}

//----------------------------------------------------------------------------
int vtkEMSegmentMRMLManager::TreeNodeDistributionLogCovarianceCorrectionEnabled(vtkIdType nodeID)
{
   if (this->GetTreeParametersLeafNode(nodeID) == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }

  // if this is unequal 
  return (this->GetTreeNodeDistributionLogCovarianceCorrection(nodeID,0,0) !=0);
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::ResetTreeNodeDistributionLogCovarianceCorrection(vtkIdType nodeID) 
{
  vtkMRMLEMSTreeParametersLeafNode* node = this->GetTreeParametersLeafNode(nodeID) ;
   if (node == NULL)
    {
    vtkErrorMacro("Leaf node is null for nodeID: " << nodeID);
    return;
    }

    unsigned int n = node->GetNumberOfTargetInputChannels();
    for (unsigned int i = 0; i < n; ++i)
      {
      for (unsigned int j = 0; j < n; ++j)
    {
      node->SetLogCovarianceCorrection(i,j,0.0);
    }
      }
}

//----------------------------------------------------------------------------
double vtkEMSegmentMRMLManager::GetTreeNodeDistributionLogCovarianceWithCorrection(vtkIdType nodeID, int rowIndex, int columnIndex) {   
  vtkMRMLEMSTreeParametersLeafNode* node = this->GetTreeParametersLeafNode(nodeID);

  if (!node)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }

  int n = int(node->GetNumberOfTargetInputChannels());
 
  if (rowIndex < 0 || rowIndex >= n || columnIndex < 0 || columnIndex >= n)
    {
    vtkErrorMacro("Out of range for nodeID: " << nodeID);
    return 0;
    }

  double value;
  if (this->TreeNodeDistributionLogCovarianceCorrectionEnabled(nodeID))
    {      
      value = node->GetLogCovarianceCorrection(rowIndex,columnIndex);
    }
  else 
    {
     value =  node->GetLogCovariance(rowIndex,columnIndex);
    }
  // cout << "GetTreeNodeDistributionLogCovarianceWithCorrection " << value << endl;
  return value; 
}


//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::SetTreeNodeDistributionLogCovarianceWithCorrection(vtkIdType nodeID, int rowIndex, int columnIndex, double value ) {
   vtkMRMLEMSTreeParametersLeafNode* node = this->GetTreeParametersLeafNode(nodeID);
   if (node == NULL)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return;
    }


   int n = int(node->GetNumberOfTargetInputChannels());
   if (rowIndex < 0 || rowIndex >= n || columnIndex < 0 || columnIndex >= n)
    {
    vtkErrorMacro("Out of range for nodeID: " << nodeID);
    return;
    }

   if (value <= -1 ) 
     {
       vtkErrorMacro("Incorrect value for node id: " << nodeID);
       return ;
     }   

   if (!this->TreeNodeDistributionLogCovarianceCorrectionEnabled(nodeID))
     {
        vtkstd::vector<vtkstd::vector<double> > logCov =  node->GetLogCovariance();
       //make sure it is not because of a rounding error 
    if (fabs( value - logCov[rowIndex][columnIndex]) < 0.001)
     {
       return;
     }
        // Have to copy values over 
        for (int i = 0; i < n; ++i)
        {
          for (int j = 0; j < n; ++j)
          {
             node->SetLogCovarianceCorrection(i,j, logCov[i][j]);
          }
        }
     }
   node->SetLogCovarianceCorrection(rowIndex,columnIndex, value);
}

//----------------------------------------------------------------------------
int  vtkEMSegmentMRMLManager::GetPackageTypeFromString(const char* type)
{
  if (!strcmp(type,"CMTK"))
    {
      return vtkEMSegmentMRMLManager::CMTK;
    }

  if (!strcmp(type,"BRAINS"))
    {
      return vtkEMSegmentMRMLManager::BRAINS;
    }

  if (!strcmp(type,"PLASTIMATCH"))
    {
      return vtkEMSegmentMRMLManager::PLASTIMATCH;
    }

  if (!strcmp(type,"DEMONS"))
    {
      return vtkEMSegmentMRMLManager::DEMONS;
    }

  if (!strcmp(type,"DRAMMS"))
    {
      return vtkEMSegmentMRMLManager::DRAMMS;
    }

  if (!strcmp(type,"ANTS"))
    {
      return vtkEMSegmentMRMLManager::ANTS;
    }

  return -1;
}

//----------------------------------------------------------------------------
int  vtkEMSegmentMRMLManager::GetInterpolationTypeFromString(const char* type)
{
  if (!strcmp(type,"InterpolationLinear")) 
    {
      return vtkEMSegmentMRMLManager::InterpolationLinear; 
    }

  if (!strcmp(type,"InterpolationNearestNeighbor")) 
    {
      return vtkEMSegmentMRMLManager::InterpolationNearestNeighbor; 
    }

  if (!strcmp(type,"InterpolationCubic")) 
    {
      return vtkEMSegmentMRMLManager::InterpolationCubic; 
    }
  return -1;
}


//----------------------------------------------------------------------------
int  vtkEMSegmentMRMLManager::GetRegistrationTypeFromString(const char* type)
{
  if (!strcmp(type,"RegistrationOff")) 
    {
      return vtkEMSegmentMRMLManager::RegistrationOff;
    }

  if (!strcmp(type,"RegistrationFast")) 
    {
      return vtkEMSegmentMRMLManager::RegistrationFast;
    }
  if (!strcmp(type,"RegistrationSlow")) 
    {
      return vtkEMSegmentMRMLManager::RegistrationSlow;
    }
  return -1;
}


//----------------------------------------------------------------------------
vtkMRMLVolumeNode*  vtkEMSegmentMRMLManager::GetAlignedSpatialPriorFromTreeNodeID(vtkIdType nodeID)
{
   vtkMRMLEMSAtlasNode* workingAtlas = this->GetWorkingDataNode()->GetAlignedAtlasNode();
   if (workingAtlas == NULL)
     {
     vtkErrorMacro("Invalid ID: " << nodeID);
     return NULL;
     }
   std::string atlasVolumeKey = this->GetTreeNode(nodeID)->GetSpatialPriorVolumeName() ? this->GetTreeNode(nodeID)->GetSpatialPriorVolumeName() : "";
   int atlasVolumeIndex       = workingAtlas->GetIndexByKey(atlasVolumeKey.c_str());
    if (atlasVolumeIndex >= 0 )
    {
      return workingAtlas->GetNthVolumeNode(atlasVolumeIndex);   
    }
    return NULL;
}

//----------------------------------------------------------------------------
void  vtkEMSegmentMRMLManager::SetAlignedSpatialPrior(vtkIdType nodeID, vtkIdType volumeID)
{
   vtkMRMLEMSAtlasNode* workingAtlas = this->GetWorkingDataNode()->GetAlignedAtlasNode();
   if (workingAtlas == NULL)
     {
        vtkErrorMacro("No aligned atlas defined");
        return;
     }

   vtkMRMLEMSTreeNode* node = this->GetTreeNode(nodeID);
    if (node == NULL)
     {
         vtkErrorMacro("Invalid ID: " << nodeID);
         return;
     }

   if (!node->GetSpatialPriorVolumeName() )
     {
          vtkErrorMacro("No input spatial atlas defined for : " << nodeID);
           return;
     }
   std::string atlasVolumeKey = node->GetSpatialPriorVolumeName() ;

   vtkMRMLNode* volumeNode = this->GetVolumeNode(volumeID);
   if (volumeNode == NULL)
     {
          vtkErrorMacro("VolumeID " << volumeID << " does not exist" ); 
           return;
     } 
   workingAtlas->AddVolume(atlasVolumeKey.c_str(),  volumeNode->GetID() );
}


//----------------------------------------------------------------------------
const char*  vtkEMSegmentMRMLManager::GetTclTaskFilename()
{
  if (!this->Node || !this->Node->GetGlobalParametersNode()  )
      {
      vtkWarningMacro("Can't get tcl file name bc EMSTemplateNode or GlobalParametersNode is null");
      return NULL;
      }
    return this->Node->GetGlobalParametersNode()->GetTaskTclFileName();

}

//----------------------------------------------------------------------------
void  vtkEMSegmentMRMLManager::SetTclTaskFilename(const char* fileName)
{
    if (!this->Node || !this->Node->GetGlobalParametersNode()  )
      {
      vtkWarningMacro("Can't set tcl file name bc EMSTemplateNode or GlobalParametersNode is null");
      return;
      }
    this->Node->GetGlobalParametersNode()->SetTaskTclFileName(fileName);
}

//----------------------------------------------------------------------------
vtksys_stl::string vtkEMSegmentMRMLManager::TurnDefaultTclFileIntoPreprocessingName(const char* fileName)
{
  vtksys_stl::string taskName(fileName);
  taskName.resize(taskName.size() -  4);

  return TurnDefaultFileIntoName(taskName);
}

//----------------------------------------------------------------------------
vtksys_stl::string vtkEMSegmentMRMLManager::TurnDefaultMRMLFileIntoTaskName(const char* fileName)
{
  vtksys_stl::string taskName(fileName);
  taskName.resize(taskName.size() -  5);

  return TurnDefaultFileIntoName(taskName);
}

//----------------------------------------------------------------------------
vtksys_stl::string vtkEMSegmentMRMLManager::TurnDefaultFileIntoName(vtksys_stl::string taskName  )
{

  size_t found=taskName.find_first_of("-");
  while (found!=vtksys_stl::string::npos)
  {
    taskName[found]=' ';
    found=taskName.find_first_of("-",found+1);
  }

  return taskName;
}         

void  vtkEMSegmentMRMLManager::SetTreeNodeDistributionLogCovarianceOffDiagonal(vtkIdType nodeID, double value)
{
 vtkMRMLEMSTreeParametersLeafNode* node = this->GetTreeParametersLeafNode(nodeID);
  if (!node)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return ;
     }

  int correctFlag = this->TreeNodeDistributionLogCovarianceCorrectionEnabled(nodeID);
  int n = this->GetTargetNumberOfSelectedVolumes(); 

 
  for (int i = 0 ; i < n; i++)
    {
      for (int j = 0 ; j < n; j++)
    {
      if ( i != j) {
        if (correctFlag)
          {
        node->SetLogCovarianceCorrection(i,j,value);
          }
        else 
        {
          node->SetLogCovariance(i,j,value);
        }
      }
    }
    }
}

bool vtkEMSegmentMRMLManager::IsTreeNodeDistributionLogCovarianceWithCorrectionInvertableAndSemiDefinite(vtkIdType nodeID)
{
 vtkMRMLEMSTreeParametersLeafNode* node = this->GetTreeParametersLeafNode(nodeID);
  if (!node)
    {
    vtkErrorMacro("Leaf parameters node is null for nodeID: " << nodeID);
    return 0;
    }

  int correctFlag = this->TreeNodeDistributionLogCovarianceCorrectionEnabled(nodeID);
  int n = this->GetTargetNumberOfSelectedVolumes(); 

 
  double** matrix = new double*[n];
  double** invMatrix = new double*[n];
  for (int i = 0 ; i < n; i++)
    {
      invMatrix[i] = new double[n];
      matrix[i] = new double[n];
      for (int j = 0 ; j < n; j++)
    {
      if (correctFlag)
        {
          matrix[i][j] =  node->GetLogCovarianceCorrection(i,j);
        }
      else 
        {
          matrix[i][j] =  node->GetLogCovariance(i,j);
              // cout << nodeID << "  "<<  matrix[i][j] << endl;
        }
    }
    }
  int flag = vtkImageEMGeneral::InvertMatrix(matrix,invMatrix,n);
  // Check if semidefinite 
  if (flag) 
    {
      flag = (vtkImageEMGeneral::determinant(matrix,n) >0.00001);
    }
  for (int i = 0 ; i < n; i++)
    {
      delete[] invMatrix[i];
      delete[] matrix[i];
    }
  delete[] invMatrix;
  delete[] matrix;
  return flag;
}


void vtkEMSegmentMRMLManager::ResetLogCovarianceCorrectionOfAllNodes()
{
  this->ResetLogCovarianceCorrectionsOfAllNodes(this->GetTreeRootNodeID());
} 

void vtkEMSegmentMRMLManager::ResetLogCovarianceCorrectionsOfAllNodes(vtkIdType rootID)
{
  if (this->GetTreeParametersLeafNode(rootID))
    {
      this->ResetTreeNodeDistributionLogCovarianceCorrection(rootID);
    }

  int numChildren = this->GetTreeNodeNumberOfChildren(rootID); 
  for (int i = 0; i < numChildren; ++i)
      {
    vtkIdType childID = this->GetTreeNodeChildNodeID(rootID, i);
    this->ResetLogCovarianceCorrectionsOfAllNodes(childID);
      }
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::RemoveLegacyNodes() 
{
  /// cout << "vtkEMSegmentMRMLManager::RemoveLegacyNodes Start "  << endl;

  // PHASE 1
  // Removed vtkMRMLEMSNode,  vtkMRMLEMSSegmenterNode, vtkMRMLEMSTargetNode

   int n = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLEMSNode");
   // now go through all of them and transfer information 
   for (int i = 0 ; i < n ; i++) 
     {
       // cout << "Remove " << i << endl; 
       vtkMRMLNode* mrmlNode = this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLEMSNode");
       if (!mrmlNode)
       {
           continue;
       }
       vtkMRMLEMSNode* emsNode = vtkMRMLEMSNode::SafeDownCast(mrmlNode);
       if (!emsNode)
       { 
         continue;
       }
       vtkMRMLEMSSegmenterNode* segNode = emsNode->GetSegmenterNode(); 
       if (!segNode) 
       {
           continue;
       }
 
       // Nothing usefull defined in the node if not temNode is 
       vtkMRMLEMSTemplateNode* temNode = segNode->GetTemplateNode();
       if (!temNode)
       {
           continue;
       }

       // Update Nodes with info from SegNode and EMSNode 
       temNode->SetName(emsNode->GetName());
       temNode->SetEMSWorkingDataNodeID(segNode->GetWorkingDataNodeID());
        
        vtkMRMLEMSWorkingDataNode* worNode = segNode->GetWorkingDataNode();
        if (worNode && segNode->GetOutputVolumeNode())
        {
           worNode->SetOutputSegmentationNodeID(segNode->GetOutputVolumeNodeID());
        }

        vtkMRMLEMSGlobalParametersNode* parNode = temNode->GetGlobalParametersNode(); 
        if (parNode)
        {
            parNode->SetTemplateFileName(emsNode->GetTemplateFilename());
            parNode->SetTemplateSaveAfterSegmentation(emsNode->GetSaveTemplateAfterSegmentation());
            parNode->SetTaskTclFileName(emsNode->GetTclTaskFilename());
            parNode->SetTaskPreProcessingSetting(emsNode->GetTaskPreprocessingSetting());
        } 

        // Transfer Infromation from worNode to segNode 
        if (worNode)
        {
            temNode->SetSpatialAtlasNodeID(worNode->GetInputAtlasNodeID());
            worNode->RemoveInputAtlasNodeID();
            temNode->SetSubParcellationNodeID(worNode->GetInputSubParcellationNodeID());
            worNode->RemoveInputSubParcellationNodeID();

            // This is the scene vtkMRMLEMSTargetNode - so I have to copy it over to loose the tag
            // it looks a little bit strange but the virtual functions GetNodeTagName is otherwise not changed !
            vtkMRMLEMSVolumeCollectionNode* intNode = worNode->GetInputTargetNode();
            if (intNode)
            {
                vtkMRMLEMSVolumeCollectionNode* newINTNode = vtkMRMLEMSVolumeCollectionNode::New(); 
                newINTNode->Copy(intNode); 
                this->GetMRMLScene()->AddNode(newINTNode);
                worNode->SetInputTargetNodeID(newINTNode->GetID());
                newINTNode->Delete();
                this->GetMRMLScene()->RemoveNode(intNode);
            }
            vtkMRMLEMSVolumeCollectionNode* outNode = worNode->GetAlignedTargetNode();
            if (outNode)
            {
                vtkMRMLEMSVolumeCollectionNode* newOUTNode = vtkMRMLEMSVolumeCollectionNode::New(); 
                newOUTNode->Copy(outNode); 
                this->GetMRMLScene()->AddNode(newOUTNode);
                worNode->SetInputTargetNodeID(newOUTNode->GetID());
                newOUTNode->Delete();
                this->GetMRMLScene()->RemoveNode(outNode);
            }
        }
     }  
  
     // remove all EMSNodes and Segmenter nodes 
     for (int i = 0 ; i < n ; i++) 
     {
       vtkMRMLNode* mrmlNode = this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLEMSNode");
       if (!mrmlNode)
       {
           continue;
       }
       vtkMRMLEMSNode* emsNode = vtkMRMLEMSNode::SafeDownCast(mrmlNode);
       if (!emsNode)
       { 
           continue;
       }

       vtkMRMLEMSSegmenterNode* segNode = emsNode->GetSegmenterNode(); 
       if (segNode) 
       {
           this->GetMRMLScene()->RemoveNode(segNode);
       } 
       this->GetMRMLScene()->RemoveNode(emsNode);
     }

     n = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLEMSTargetNode");
     // Remove all remaining Target nodes 
     for (int i = 0 ; i < n ; i++) 
       {
           // cout << "Remove " << i << endl; 
           vtkMRMLNode* mrmlNode = this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLEMSTargetNode");
           vtkMRMLEMSTargetNode* tarNode = vtkMRMLEMSTargetNode::SafeDownCast(mrmlNode);
           if (tarNode)
           { 
              this->GetMRMLScene()->RemoveNode(tarNode);
           }
       }

      // PHASE 2
      // remove vtkMRMLEMSPParametersNode 
     if (this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLEMSTreeParametersNode")) {
       std::vector<vtkMRMLNode *> mrmlNodes;
       this->MRMLScene->GetNodesByClass("vtkMRMLEMSTreeNode", mrmlNodes);
     for (int m= 0 ; m < int(mrmlNodes.size()); m++) 
        {
          vtkMRMLEMSTreeNode* treeNode = vtkMRMLEMSTreeNode::SafeDownCast(mrmlNodes[m]);
          if (treeNode)
        {
            vtkMRMLEMSTreeParametersNode* parNode = treeNode->GetTreeParametersNode();
            if (parNode) 
              {
        // cout << "===> Removing  TreeParametersNode with ID: " << parNode->GetID() << endl;
                  if (treeNode->GetNumberOfChildNodes()) 
                  {
                           
                           treeNode->SetParentParametersNodeID(parNode->GetParentParametersNodeID());
                            // delete associated leaf node;
                            vtkMRMLEMSTreeParametersLeafNode* leafNode = parNode->GetLeafParametersNode();
                            if (leafNode)
                            {
                                 this->MRMLScene->RemoveNode(leafNode);
                            }
                            // This is now a parent node so it does not need any leaf node parameters 
                            treeNode->SetLeafParametersNodeID(NULL);
               // Currently GUI does not display spatial prior maps - so remove it here so it does not lurk around in the background 
                           treeNode->SetSpatialPriorVolumeName(NULL);
                     } else {
                           treeNode->SetLeafParametersNodeID(parNode->GetLeafParametersNodeID());
                           // delete associated parent node;
                           vtkMRMLEMSTreeParametersParentNode* parentNode = parNode->GetParentParametersNode();
                           if (parentNode)
                           {
                                    this->MRMLScene->RemoveNode(parentNode);
                           }
                           treeNode->SetParentParametersNodeID(NULL);
                      }
                      treeNode->SetColorRGB(parNode->GetColorRGB());
                      int numInput  = parNode->GetNumberOfTargetInputChannels();
                      treeNode->SetNumberOfTargetInputChannels(numInput);                        
                      for(int i = 0 ; i < numInput ; i++)
                     { 
                            treeNode->SetInputChannelWeight(i,parNode->GetInputChannelWeight(i));
                     }
 
                        treeNode->SetSpatialPriorVolumeName(parNode->GetSpatialPriorVolumeName());
                        treeNode->SetSpatialPriorWeight(parNode->GetSpatialPriorWeight());
                        treeNode->SetClassProbability(parNode->GetClassProbability());
                        treeNode->SetExcludeFromIncompleteEStep(parNode->GetExcludeFromIncompleteEStep());
                        treeNode->SetPrintWeights(parNode->GetPrintWeights());
                        
                        this->MRMLScene->RemoveNode(parNode);
              }
           }
        }
     }    
     
     n = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLEMSTreeParametersNode");
     for (int i = 0 ; i < n ; i++) 
      {
      vtkMRMLEMSTreeParametersNode* parNode = vtkMRMLEMSTreeParametersNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(i, "vtkMRMLEMSTreeParametersNode"));
           if (parNode)
        {
                this->MRMLScene->RemoveNode(parNode);
         }
       }
}
        
// Delete ParameterParentNode and create ParameterLeafNode if necessary
void vtkEMSegmentMRMLManager::TurnFromParentToLeafNode(vtkMRMLEMSTreeNode* treeNode) 
{
    // Nothing to do bc for the node to be a leaf node it has to have no children
    if (treeNode->GetNumberOfChildNodes())
    {
      return; 
    }

     // Delete ParameterParentNode and create ParameterLeafNode ->
     vtkMRMLEMSTreeParametersParentNode* parParentNode = treeNode->GetParentParametersNode();
     treeNode->SetParentParametersNodeID(NULL);
     if ( parParentNode )
        {
            this->MRMLScene->RemoveNode(parParentNode);
        }


      vtkMRMLEMSTreeParametersLeafNode* parLeafNode = treeNode->GetLeafParametersNode();
      // Nothing to do more bc leaf node already exists
      if ( parLeafNode )
      {
      return;
       }
     
        parLeafNode  =vtkMRMLEMSTreeParametersLeafNode::New();
        parLeafNode->SetHideFromEditors(this->HideNodesFromEditors);
        this->MRMLScene->AddNode(parLeafNode);
        treeNode->SetLeafParametersNodeID(parLeafNode->GetID()); 
    parLeafNode ->Delete();
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::ImportMRMLFile(const char *mrmlFile)
{

  vtksys_stl::string errMSG = "";
  this->ImportMRMLFile(mrmlFile,errMSG);

}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::ImportMRMLFile(const char *mrmlFile,  vtksys_stl::string errMSG)
{
 
  this->MRMLScene->SetURL(mrmlFile); 

  if (!this->MRMLScene->Import())
    {
      errMSG = vtksys_stl::string("Could not load mrml file ") + vtksys_stl::string( mrmlFile);
      vtkErrorMacro(<< errMSG.c_str()); 
      return;
    }

  if(this->MRMLScene->GetErrorCode())
    {
         std::stringstream strstream;
         strstream << "Corrupted MRML file:"  <<  mrmlFile << " was corrupted or could not be loaded. Error code: " 
           << this->MRMLScene->GetErrorCode()  << " Error message: " << this->MRMLScene->GetErrorMessage();
         strstream >> errMSG;
         vtkErrorMacro(<<errMSG.c_str()); 
         return;
    }
  cout << "==========================================================================" << endl;
  cout << "== Completed importing data " << this->MRMLScene->GetURL() << endl;
  cout << "==========================================================================" << endl;

}

//----------------------------------------------------------------------------
void  vtkEMSegmentMRMLManager::SetStorageNodeToNULL(vtkMRMLStorableNode* sNode) 
{
    sNode-> SetAndObserveStorageNodeID(NULL);
}

//----------------------------------------------------------------------------
int  vtkEMSegmentMRMLManager::GetIsland2DFlag() 
 {
   return this->GetGlobalParametersNode() ? this->GetGlobalParametersNode()->GetIsland2DFlag() : 0;
 }

//----------------------------------------------------------------------------  
void    vtkEMSegmentMRMLManager::SetIsland2DFlag(int flag) 
{
  if (!this->GetGlobalParametersNode())
    {
      vtkErrorMacro("GlobalParametersNode is not defined"); 
      return ;
    } 
   this->GetGlobalParametersNode()->SetIsland2DFlag(flag);
}

//----------------------------------------------------------------------------
int vtkEMSegmentMRMLManager::GetTreeNodeInteractionMatrices2DFlag(vtkIdType nodeID)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode() )
    {
    vtkErrorMacro("Tree node is null for nodeID: " << nodeID << " or not a parent node");
    return 0;
    }
  return n->GetParentParametersNode()->GetMFA2DFlag();  
}

//----------------------------------------------------------------------------

void
vtkEMSegmentMRMLManager::
SetTreeNodeInteractionMatrices2DFlag(vtkIdType nodeID, int flag)
{
  vtkMRMLEMSTreeNode* n = this->GetTreeNode(nodeID);
  if (n == NULL || !n->GetParentParametersNode())
    {
      vtkErrorMacro("Tree node is null for nodeID: " << nodeID <<  " or not a parent node");
    return;
    }

  n->GetParentParametersNode()->SetMFA2DFlag(flag);
}

//----------------------------------------------------------------------------
void vtkEMSegmentMRMLManager::PrintWeightOnForEntireTree() {
  this->PrintWeightOnForTree(this->GetTreeRootNodeID());
}

//----------------------------------------------------------------------------

void vtkEMSegmentMRMLManager::PrintWeightOnForTree(vtkIdType rootID) {  
  unsigned int numChildren =   this->GetTreeNodeNumberOfChildren(rootID);
    for (unsigned int i = 0; i < numChildren; ++i)
    {
      vtkIdType childID = this->GetTreeNodeChildNodeID(rootID, i);
      this->SetTreeNodePrintWeight(childID, 1);
      bool isLeaf = this->GetTreeNodeIsLeaf(childID);

      if (!isLeaf)
      {
        this->PrintWeightOnForTree(childID);
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*  vtkEMSegmentMRMLManager::CreateVolumeScalarNode(vtkMRMLScalarVolumeNode*  referenceNode , const char *name)
{
     vtkMRMLScalarVolumeNode *clonedVolumeNode = vtkMRMLScalarVolumeNode::New();
     clonedVolumeNode->CopyWithScene(referenceNode);
     clonedVolumeNode->SetAndObserveStorageNodeID(NULL);
     clonedVolumeNode->SetName(name);

     vtkMRMLScalarVolumeDisplayNode* clonedDisplayNode =  vtkMRMLScalarVolumeDisplayNode::New();
     clonedDisplayNode->CopyWithScene(referenceNode->GetDisplayNode());
     this->MRMLScene->AddNode(clonedDisplayNode);
     clonedVolumeNode->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());
     clonedDisplayNode->Delete();

     vtkImageData* volumeData =  vtkImageData::New();
     clonedVolumeNode->SetAndObserveImageData(volumeData);
     volumeData->Delete();

     // add the cloned volume to the scene
     this->MRMLScene->AddNode(clonedVolumeNode);

     const char* volID = clonedVolumeNode->GetID();
     clonedVolumeNode->Delete();
     return  vtkMRMLScalarVolumeNode::SafeDownCast(this->MRMLScene->GetNodeByID(volID));
}

vtkIdType vtkEMSegmentMRMLManager::CreateVolumeScalarNodeVolumeID(vtkMRMLScalarVolumeNode*  referenceNode , const char *name)
{
    vtkMRMLScalarVolumeNode* node = this->CreateVolumeScalarNode(referenceNode, name);
    return this->MapMRMLNodeIDToVTKNodeID(node->GetID());
}

