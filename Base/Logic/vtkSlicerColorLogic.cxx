/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerColorLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkSlicerColorLogic.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"

#include "vtkMRMLFiducial.h"
#include "vtkMRMLFiducialListNode.h"

vtkCxxRevisionMacro(vtkSlicerColorLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerColorLogic);

//----------------------------------------------------------------------------
vtkSlicerColorLogic::vtkSlicerColorLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerColorLogic::~vtkSlicerColorLogic()
{
  // remove the default color nodes
  this->RemoveDefaultColorNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::ProcessMRMLEvents(vtkObject * caller,
                                            unsigned long event,
                                            void * callData)
{
  vtkDebugMacro("vtkSlicerColorLogic::ProcessMRMLEvents: got an event " << event);
  
  // when there's a new scene, add the default nodes
  //if (event == vtkMRMLScene::NewSceneEvent || event == vtkMRMLScene::SceneCloseEvent)
  if (event == vtkMRMLScene::NewSceneEvent)
    {
    vtkDebugMacro("vtkSlicerColorLogic::ProcessMRMLEvents: got a NewScene event " << event);
    //this->RemoveDefaultColorNodesFromScene();
    this->AddDefaultColorNodes();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerColorLogic:             " << this->GetClassName() << "\n";

}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::AddDefaultColorNodes()
{
    // create the default color nodes, they don't get saved with the scenes as
    // they'll be created on start up, and when a new
    // scene is opened
  if (this->GetMRMLScene() == NULL)
    {
    vtkWarningMacro("vtkSlicerColorLogic::AddDefaultColorNodes: no scene to which to add nodes\n");
    return;
    }
  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't add a File node
    if (i != basicNode->File)
      {
      vtkMRMLColorTableNode *node = vtkMRMLColorTableNode::New();
      node->SetType(i);
      node->SaveWithSceneOff();
      node->SetName(node->GetTypeAsString());      
      std::string id = std::string(this->GetDefaultColorTableNodeID(i));
      node->SetSingletonTag(id.c_str());
      if (this->GetMRMLScene()->GetNodeByID(id.c_str()) == NULL)
        {
        vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: requesting id " << id.c_str() << endl);
        this->GetMRMLScene()->RequestNodeID(node, id.c_str());
        this->GetMRMLScene()->AddNode(node);
        vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: added node " << node->GetID() << ", requested id was " << id.c_str() << ", type = " << node->GetTypeAsString() << endl);
        }
      else
        {
        vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: didn't add node " << node->GetID() << " as it was already in the scene.\n");
        }
      node->Delete();
      }
    }
  basicNode->Delete();

  // add freesurfer nodes
  vtkDebugMacro("Adding Freesurfer nodes");
  vtkMRMLFreeSurferProceduralColorNode *basicFSNode = vtkMRMLFreeSurferProceduralColorNode::New();
  for (int i = basicFSNode->GetFirstType(); i <= basicFSNode->GetLastType(); i++)
    {
    vtkMRMLFreeSurferProceduralColorNode *node = vtkMRMLFreeSurferProceduralColorNode::New();
    node->SetType(i);
    node->SaveWithSceneOff();
    if (node->GetTypeAsString() == NULL)
      {
      vtkWarningMacro("Node type as string is null");      
      node->SetName("NoName");
      }
    else
      {
      vtkDebugMacro("Got node type as string " << node->GetTypeAsString());
      node->SetName(node->GetTypeAsString());
      }
    if (this->GetDefaultFreeSurferColorNodeID(i) == NULL)
      {
      vtkDebugMacro("Error getting default node id for freesurfer node " << i);
      }
    std::string id = std::string(this->GetDefaultFreeSurferColorNodeID(i));
    node->SetSingletonTag(id.c_str());
    vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: requesting id " << id.c_str() << endl);
    if (this->GetMRMLScene()->GetNodeByID(id.c_str()) == NULL)
      {
      this->GetMRMLScene()->RequestNodeID(node, id.c_str());        
      this->GetMRMLScene()->AddNode(node);
      vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: added node " << node->GetID() << ", requested id was " << id.c_str() << ", type = " << node->GetTypeAsString() << endl);
      }
    else
      {
      vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: didn't add node " << node->GetID() << " as it was already in the scene.\n");
      }
    node->Delete();
    }
  
  // add a regular colour tables holding the freesurfer volume file colours and
  // surface colours
  vtkMRMLColorTableNode *node = vtkMRMLColorTableNode::New();
  node->SetTypeToFile();
  node->SaveWithSceneOff();

  vtkDebugMacro("Adding FreeSurfer Labels file node");
  std::string colorFileName;
  std::string id;
  // volume labels
  node->SetName("FSLabels");
  if (basicFSNode->GetLabelsFileName() == NULL)
    {
    vtkErrorMacro("Unable to get the labels file name, not adding");
    }
  else
    {
    colorFileName = std::string(basicFSNode->GetLabelsFileName());
    vtkDebugMacro("Trying to read colour file " << colorFileName.c_str());
  
    node->SetFileName(colorFileName.c_str());
    node->ReadFile();
    id = std::string(this->GetDefaultFreeSurferLabelMapColorNodeID());
    node->SetSingletonTag(id.c_str());
    if (this->GetMRMLScene()->GetNodeByID(id.c_str()) == NULL)
      {
      this->GetMRMLScene()->RequestNodeID(node, id.c_str());        
      this->GetMRMLScene()->AddNode(node);
      }
    else
      {
      vtkWarningMacro("Unable to add a new colour node " << id.c_str() << " with freesurfer colours, from file: " << node->GetFileName());
      }
    }
  node->Delete();

  // surface labels
  node = vtkMRMLColorTableNode::New();
  node->SetTypeToFile();
  node->SaveWithSceneOff();
  node->SetName("FSSurfaceLabels");
  if (basicFSNode->GetSurfaceLabelsFileName() == NULL)
    {
    vtkErrorMacro("Unable to get the freesurfer surface labels file name, not adding a colour node");
    }
  else
    {
    colorFileName = std::string(basicFSNode->GetSurfaceLabelsFileName());
    vtkDebugMacro("Trying to read colour file " << colorFileName.c_str());
    node->SetFileName(colorFileName.c_str());
    node->ReadFile();
    id = std::string(this->GetDefaultFreeSurferSurfaceLabelsColorNodeID());
    node->SetSingletonTag(id.c_str());
    if (this->GetMRMLScene()->GetNodeByID(id.c_str()) == NULL)
      {
      this->GetMRMLScene()->RequestNodeID(node, id.c_str());        
      this->GetMRMLScene()->AddNode(node);
      }
    else
      {
      vtkWarningMacro("Unable to add a new colour node " << id.c_str() << " with freesurfer colours, from file: " << node->GetFileName());
      }
    }
  basicFSNode->Delete();
  node->Delete();
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::RemoveDefaultColorNodes()
{
  // try to find any of the default colour nodes that are still in the scene
  if (this->GetMRMLScene() == NULL)
    {
    // nothing can do, it's gone
    return;
    }
  
  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();
  vtkMRMLColorTableNode *node;
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't have a File node
    if (i != basicNode->File)
      {
      basicNode->SetType(i);
      std::string id = std::string(this->GetDefaultColorTableNodeID(i));
      vtkDebugMacro("vtkSlicerColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id.c_str() << endl);
      node = vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id.c_str()));
      if (node != NULL)
        {
        node->Delete();
        node = NULL;
        }
      }
    }

  // remove freesurfer nodes
  vtkMRMLFreeSurferProceduralColorNode *basicFSNode = vtkMRMLFreeSurferProceduralColorNode::New();
  vtkMRMLFreeSurferProceduralColorNode *fsnode;
  for (int i = basicFSNode->GetFirstType(); i <= basicFSNode->GetLastType(); i++)
    {
    basicFSNode->SetType(i);
    std::string id = std::string(this->GetDefaultFreeSurferColorNodeID(i));
    vtkDebugMacro("vtkSlicerColorLogic::RemoveDefaultColorNodes: trying to find node with id " << id.c_str() << endl);
    fsnode =  vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id.c_str()));
    if (fsnode != NULL)
      {       
      fsnode->Delete();
      fsnode = NULL;
      }
    }
  basicFSNode->Delete();
  
  // remove the fs lookup table node
  node = vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetDefaultFreeSurferLabelMapColorNodeID()));
  if (node != NULL)
    {       
    node->Delete();
    node = NULL;
    }
  // remove the fs surface labels node
  node = vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GetDefaultFreeSurferSurfaceLabelsColorNodeID()));
  if (node != NULL)
    {       
    node->Delete();
    node = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkSlicerColorLogic::RemoveDefaultColorNodesFromScene()
{
  // try to find any of the default colour nodes that are still in the scene
  if (this->GetMRMLScene() == NULL)
    {
    // nothing can do, it's gone
    return;
    }
  
  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();
  vtkMRMLColorTableNode *node;
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't have a File node
    if (i != basicNode->File)
      {
      basicNode->SetType(i);
      std::string id = std::string(this->GetDefaultColorTableNodeID(i));
      node = vtkMRMLColorTableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id.c_str()));
      if (node != NULL)
        {
        this->GetMRMLScene()->RemoveNode(node);
        }
      }
    }
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultColorTableNodeID(int type)
{
  const char *id;
  vtkMRMLColorTableNode *basicNode = vtkMRMLColorTableNode::New();
  basicNode->SetType(type);

  //std::string id = std::string(basicNode->GetClassName()) + std::string(basicNode->GetTypeAsString());
  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();
  //basicNode = NULL;
  //return id.c_str();

  return (id);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultFreeSurferColorNodeID(int type)
{
  const char *id;
  vtkMRMLFreeSurferProceduralColorNode *basicNode = vtkMRMLFreeSurferProceduralColorNode::New();
  basicNode->SetType(type);

  id = basicNode->GetTypeAsIDString();
  basicNode->Delete();

  return (id);
}
//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultVolumeColorNodeID()
{
  return this->GetDefaultColorTableNodeID(vtkMRMLColorTableNode::Grey);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultLabelMapColorNodeID()
{
  return this->GetDefaultColorTableNodeID(vtkMRMLColorTableNode::Labels);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultModelColorNodeID()
{
  return this->GetDefaultFreeSurferColorNodeID(vtkMRMLFreeSurferProceduralColorNode::Heat);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultFreeSurferLabelMapColorNodeID()
{
  return this->GetDefaultFreeSurferColorNodeID(vtkMRMLFreeSurferProceduralColorNode::Labels);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultFreeSurferSurfaceLabelsColorNodeID()
{
  return this->GetDefaultFreeSurferColorNodeID(vtkMRMLFreeSurferProceduralColorNode::SurfaceLabels);
}

