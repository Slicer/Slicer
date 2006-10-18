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

#include "vtkMRMLFiducial.h"
#include "vtkMRMLFiducialListNode.h"

vtkCxxRevisionMacro(vtkSlicerColorLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerColorLogic);

//----------------------------------------------------------------------------
vtkSlicerColorLogic::vtkSlicerColorLogic()
{
  vtkDebugMacro("vtkSlicerColorLogic: constructor trying to add default colour nodes\n");
  this->AddDefaultColorNodes();
}

//----------------------------------------------------------------------------
vtkSlicerColorLogic::~vtkSlicerColorLogic()
{
    
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::ProcessMRMLEvents(vtkObject * caller,
                                            unsigned long event,
                                            void * callData)
{
  vtkDebugMacro("vtkSlicerColorLogic::ProcessMRMLEvents: got an event " << event);
  
  // when there's a new scene, add the default nodes
  if (event == vtkMRMLScene::NewSceneEvent)
    {
    vtkDebugMacro("vtkSlicerColorLogic::ProcessMRMLEvents: got a NewScene event " << event);
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
    std::cerr << "vtkSlicerColorLogic::AddDefaultColorNodes: no scene to which to add nodes\n";
    return;
    }
  vtkMRMLColorNode *basicNode = vtkMRMLColorNode::New();
  for (int i = basicNode->GetFirstType(); i <= basicNode->GetLastType(); i++)
    {
    // don't add a File node
    if (i != basicNode->File)
      {
      vtkMRMLColorNode *node = vtkMRMLColorNode::New();
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
  basicNode->Delete();
}
