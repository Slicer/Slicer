/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerModelsLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkSlicerModelsLogic.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelDisplayNode.h"

vtkCxxRevisionMacro(vtkSlicerModelsLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerModelsLogic);

//----------------------------------------------------------------------------
vtkSlicerModelsLogic::vtkSlicerModelsLogic()
{
  this->ActiveModelNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerModelsLogic::~vtkSlicerModelsLogic()
{
  if (this->ActiveModelNode != NULL)
    {
        this->ActiveModelNode->Delete();
        this->ActiveModelNode = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::ProcessMRMLEvents()
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::SetActiveModelNode(vtkMRMLModelNode *activeNode)
{
  this->SetMRML( vtkObjectPointer(&this->ActiveModelNode), activeNode );
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkSlicerModelsLogic::AddModel (char* filename)
{
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
  vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
  vtkMRMLModelStorageNode *storageNode = vtkMRMLModelStorageNode::New();

  storageNode->SetFileName(filename);
  if (storageNode->ReadData(modelNode) != 0)
    {
    const vtksys_stl::string fname(filename);
    vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
    modelNode->SetName(name.c_str());

    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);
    modelNode->SetStorageNodeID(storageNode->GetID());
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  

    this->GetMRMLScene()->AddNode(modelNode);  


        
    this->Modified();  
    }

  modelNode->Delete();
  storageNode->Delete();
  displayNode->Delete();

  return modelNode;  
}

//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerModelsLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveModelNode: " <<
    (this->ActiveModelNode ? this->ActiveModelNode->GetName() : "(none)") << "\n";
}

