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
//#include "vtkMRMLColorProceduralFreeSurferNode.h"

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
    this->RemoveDefaultColorNodesFromScene();
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
      vtkMRMLColorNode *node = vtkMRMLColorTableNode::New();
      node->SetType(i);
      node->SaveWithSceneOff();
      node->SetName(node->GetTypeAsString());      
      std::string id = std::string(this->GetDefaultColorTableNodeID(i));
      vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: requesting id " << id.c_str() << endl);
      this->GetMRMLScene()->RequestNodeID(node, id.c_str());
      if (this->GetMRMLScene()->GetNodeByID(node->GetID()) == NULL)
        {
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
/*
  // add freesurfer nodes
  vtkMRMLColorProceduralFreeSurferNode *basicFSNode = vtkMRMLColorProceduralFreeSurferNode::New();
  for (int i = basicFSNode->GetFirstType(); i <= basicFSNode->GetLastType(); i++)
    {
    // don't add a File node
    if (i != basicFSNode->File)
      {
      vtkMRMLColorProceduralFreeSurferNode *node = vtkMRMLColorProceduralFreeSurferNode::New();
      node->SetType(i);
      node->SaveWithSceneOff();
      node->SetName(node->GetTypeAsString());      
      std::string id = std::string(node->GetClassName()) + std::string(node->GetTypeAsString());
      vtkDebugMacro("vtkSlicerColorLogic::AddDefaultColorNodes: requesting id " << id.c_str() << endl);
      this->GetMRMLScene()->RequestNodeID(node, id.c_str());
      if (this->GetMRMLScene()->GetNodeByID(node->GetID()) == NULL)
        {
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
  basicFSNode->Delete();
*/
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
/*
  // remove freesurfer nodes
  vtkMRMLColorProceduralFreeSurferNode *basicFSNode = vtkMRMLColorProceduralFreeSurferNode::New();
  vtkMRMLColorProceduralFreeSurferNode *fsnode;
  for (int i = basicFSNode->GetFirstType(); i <= basicFSNode->GetLastType(); i++)
    {
    // don't have a File node
    if (i != basicFSNode->File)
      {
      basicFSNode->SetType(i);
      std::string id = std::string(basicFSNode->GetClassName()) + std::string(basicFSNode->GetTypeAsString());
      fsnode =  vtkMRMLColorProceduralFreeSurferNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id.c_str()));
      if (fsnode != NULL)
        {
        std::cout << "Deleting FS color node " << fsnode->GetID() << endl;
        fsnode->Delete();
        fsnode = NULL;
        }
      }
    }
*/
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
  // TODO: return a freesurfer colour node ID
   return this->GetDefaultColorTableNodeID(vtkMRMLColorTableNode::Ocean);
}
