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
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

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
void vtkSlicerModelsLogic::ProcessMRMLEvents(vtkObject * /*caller*/, 
                                            unsigned long /*event*/, 
                                            void * /*callData*/)
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
int vtkSlicerModelsLogic::AddModels (const char* dirname, const char* suffix )
{
  std::string ssuf = suffix;
  itksys::Directory dir;
  dir.Load(dirname);
 
  int nfiles = dir.GetNumberOfFiles();
  int res = 1;
  for (int i=0; i<nfiles; i++) {
    const char* filename = dir.GetFile(i);
    std::string sname = filename;
    if (!itksys::SystemTools::FileIsDirectory(filename))
      {
      if ( sname.find(ssuf) != std::string::npos )
        {
        std::string fullPath = std::string(dir.GetPath())
            + "/" + filename;
        if (this->AddModel((char *)fullPath.c_str()) == NULL) 
          {
          res = 0;
          }
        }
      }
  }
  return res;
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
    const itksys_stl::string fname(filename);
    itksys_stl::string name = itksys::SystemTools::GetFilenameName(fname);
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
int vtkSlicerModelsLogic::SaveModel (char* filename, vtkMRMLModelNode *modelNode)
{
   if (modelNode == NULL || filename == NULL)
    {
    return 0;
    }
  
  vtkMRMLModelStorageNode *storageNode = NULL;
  vtkMRMLStorageNode *snode = modelNode->GetStorageNode();
  if (snode != NULL)
    {
    storageNode = vtkMRMLModelStorageNode::SafeDownCast(snode);
    }
  if (storageNode == NULL)
    {
    storageNode = vtkMRMLModelStorageNode::New();
    storageNode->SetScene(this->GetMRMLScene());
    this->GetMRMLScene()->AddNode(storageNode);  
    modelNode->SetStorageNodeID(storageNode->GetID());
    storageNode->Delete();
    }

  //storageNode->SetAbsoluteFileName(true);
  storageNode->SetFileName(filename);

  int res = storageNode->WriteData(modelNode);

  
  return res;

}


//----------------------------------------------------------------------------
void vtkSlicerModelsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerModelsLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveModelNode: " <<
    (this->ActiveModelNode ? this->ActiveModelNode->GetName() : "(none)") << "\n";
}

