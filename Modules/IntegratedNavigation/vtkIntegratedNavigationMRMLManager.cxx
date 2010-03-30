#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkIntegratedNavigationMRMLManager.h"
#include "vtkIntegratedNavigationLogic.h"
#include "vtkIntegratedNavigation.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLModelNode.h"

#include <math.h>
#include <exception>

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>

#define ERROR_NODE_VTKID 0

//----------------------------------------------------------------------------
vtkIntegratedNavigationMRMLManager* vtkIntegratedNavigationMRMLManager::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkIntegratedNavigationMRMLManager");
  if(ret)
    {
    return (vtkIntegratedNavigationMRMLManager*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIntegratedNavigationMRMLManager;
}


//----------------------------------------------------------------------------
vtkIntegratedNavigationMRMLManager::vtkIntegratedNavigationMRMLManager()
{
  this->MRMLScene = NULL;
}

//----------------------------------------------------------------------------
vtkIntegratedNavigationMRMLManager::~vtkIntegratedNavigationMRMLManager()
{
  this->SetMRMLScene(NULL);
}

//----------------------------------------------------------------------------
void 
vtkIntegratedNavigationMRMLManager::
ProcessMRMLEvents(vtkObject* caller,
                  unsigned long event,
                  void* callData)
{
  vtkDebugMacro("vtkIntegratedNavigationMRMLManager::ProcessMRMLEvents: got an event " 
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
    if (node->IsA("vtkMRMLVolumeNode"))
      {
      vtkIdType newID = this->GetNewVTKNodeID();
      this->IDMapInsertPair(newID, node->GetID());
      }
    if (node->IsA("vtkMRMLFiducialListNode"))
      {
      vtkIdType newID = this->GetNewVTKNodeID();
      this->IDMapInsertPair(newID, node->GetID());
      }
    if (node->IsA("vtkMRMLModelNode"))
      {
      vtkIdType newID = this->GetNewVTKNodeID();
      this->IDMapInsertPair(newID, node->GetID());
      }
    }
  else if (event == vtkMRMLScene::NodeRemovedEvent)
    {
    if (node->IsA("vtkMRMLVolumeNode"))
      {
      this->IDMapRemovePair(node->GetID());
      }
    if (node->IsA("vtkMRMLFiducialListNode"))
      {
      this->IDMapRemovePair(node->GetID());
      }
    if (node->IsA("vtkMRMLModelNode"))
      {
      this->IDMapRemovePair(node->GetID());
      }
    }
}


//----------------------------------------------------------------------------
int
vtkIntegratedNavigationMRMLManager::
GetVolumeNumberOfChoices()
{
  //
  // Get number of volumes loaded into the
  // slicer scene
  //
  return this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
}

//----------------------------------------------------------------------------
int
vtkIntegratedNavigationMRMLManager::
GetFiducialNumberOfChoices()
{
  //
  // Get number of fiducials loaded into the
  // slicer scene
  //
  return this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
}

//----------------------------------------------------------------------------
int
vtkIntegratedNavigationMRMLManager::
GetModelNumberOfChoices()
{
  //
  // Get number of models loaded into the
  // slicer scene
  //
  return this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
}

//----------------------------------------------------------------------------
vtkIdType
vtkIntegratedNavigationMRMLManager::
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
vtkIdType
vtkIntegratedNavigationMRMLManager::
GetFiducialNthID(int n)
{
  vtkMRMLNode* node = this->GetMRMLScene()->
    GetNthNodeByClass(n, "vtkMRMLFiducialListNode");

  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth fiducial list in scene: " << n);
    return ERROR_NODE_VTKID;
    }

  if (this->IDMapContainsMRMLNodeID(node->GetID()))
    {
    return this->MapMRMLNodeIDToVTKNodeID(node->GetID());
    }
  else
    {
    vtkErrorMacro("Fiducial MRML ID was not in map!" << node->GetID());
    return ERROR_NODE_VTKID;
    }
}

//----------------------------------------------------------------------------
vtkIdType
vtkIntegratedNavigationMRMLManager::
GetModelNthID(int n)
{
  vtkMRMLNode* node = this->GetMRMLScene()->
    GetNthNodeByClass(n, "vtkMRMLModelNode");

  if (node == NULL)
    {
    vtkErrorMacro("Did not find nth  model in scene: " << n);
    return ERROR_NODE_VTKID;
    }

  if (this->IDMapContainsMRMLNodeID(node->GetID()))
    {
    return this->MapMRMLNodeIDToVTKNodeID(node->GetID());
    }
  else
    {
    vtkErrorMacro("Model MRML ID was not in map!" << node->GetID());
    return ERROR_NODE_VTKID;
    }
}


//----------------------------------------------------------------------------
const char*
vtkIntegratedNavigationMRMLManager::
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
const char*
vtkIntegratedNavigationMRMLManager::
GetFiducialName(vtkIdType fiducialID)
{
  vtkMRMLFiducialListNode* fiducialNode = this->GetFiducialNode(fiducialID);
  if (fiducialNode == NULL)
    {
    return NULL;
    }
  return fiducialNode->GetName();
}

//----------------------------------------------------------------------------
const char*
vtkIntegratedNavigationMRMLManager::
GetModelName(vtkIdType modelID)
{
  vtkMRMLModelNode* modelNode = this->GetModelNode(modelID);
  if (modelNode == NULL)
    {
    return NULL;
    }
  return modelNode->GetName();
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode*
vtkIntegratedNavigationMRMLManager::
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
vtkMRMLFiducialListNode*
vtkIntegratedNavigationMRMLManager::
GetFiducialNode(vtkIdType fiducialID)
{
  vtkMRMLFiducialListNode* node = NULL;
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(fiducialID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Unknown fiducialID: " << fiducialID);
    return NULL;
    }

  if (this->GetMRMLScene())
    {
    vtkMRMLNode* snode = this->GetMRMLScene()->GetNodeByID(mrmlID);
    node = vtkMRMLFiducialListNode::SafeDownCast(snode);
   
    if (node == NULL)
      {
      vtkErrorMacro("Attempt to cast to fiducial list node from non-volume mrml id: " 
                    << mrmlID);
      }
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLModelNode*
vtkIntegratedNavigationMRMLManager::
GetModelNode(vtkIdType modelID)
{
  vtkMRMLModelNode* node = NULL;
  const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(modelID);
  if (mrmlID == NULL || strlen(mrmlID) == 0)
    {
    vtkErrorMacro("Unknown modelID: " << modelID);
    return NULL;
    }

  if (this->GetMRMLScene())
    {
    vtkMRMLNode* snode = this->GetMRMLScene()->GetNodeByID(mrmlID);
    node = vtkMRMLModelNode::SafeDownCast(snode);
   
    if (node == NULL)
      {
      vtkErrorMacro("Attempt to cast to model list node from non-volume mrml id: " 
                    << mrmlID);
      }
    }
  return node;
}




//-----------------------------------------------------------------------------
vtkIdType
vtkIntegratedNavigationMRMLManager::
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
vtkIntegratedNavigationMRMLManager::
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
vtkIntegratedNavigationMRMLManager::
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
vtkIntegratedNavigationMRMLManager::
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
vtkIntegratedNavigationMRMLManager::
IDMapRemovePair(vtkIdType vtkID)
{
  this->MRMLNodeIDToVTKNodeIDMap.
    erase(this->VTKNodeIDToMRMLNodeIDMap[vtkID]);
  this->VTKNodeIDToMRMLNodeIDMap.erase(vtkID);
}

//-----------------------------------------------------------------------------
void
vtkIntegratedNavigationMRMLManager::
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
vtkIntegratedNavigationMRMLManager::
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
vtkIntegratedNavigationMRMLManager::
IDMapContainsVTKNodeID(vtkIdType id)
{
  return this->VTKNodeIDToMRMLNodeIDMap.count(id) > 0;
}

//-----------------------------------------------------------------------------
void
vtkIntegratedNavigationMRMLManager::
RegisterMRMLNodesWithScene()
{
  if(!this->GetMRMLScene())
    {
    return;
    } 

  //Register additional node classes
}
 
//----------------------------------------------------------------------------
void vtkIntegratedNavigationMRMLManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

