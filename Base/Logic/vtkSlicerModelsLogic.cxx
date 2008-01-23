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
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"

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
  vtkSetMRMLNodeMacro(this->ActiveModelNode, activeNode );
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
vtkMRMLModelNode* vtkSlicerModelsLogic::AddModel (const char* filename)
{
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
  vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
  vtkMRMLModelStorageNode *mStorageNode = vtkMRMLModelStorageNode::New();
  vtkMRMLFreeSurferModelStorageNode *fsmStorageNode = vtkMRMLFreeSurferModelStorageNode::New();
  fsmStorageNode->SetUseStripper(0);  // turn off stripping by default (breaks some pickers)
  vtkMRMLStorageNode *storageNode = NULL;
  
  mStorageNode->SetFileName(filename);
  fsmStorageNode->SetFileName(filename);
  
  if (mStorageNode->ReadData(modelNode) != 0)
    {
    storageNode = mStorageNode;
    }
  else if (fsmStorageNode->ReadData(modelNode) != 0)
    {
    storageNode = fsmStorageNode;
    }
  if (storageNode != NULL)
    {
    const itksys_stl::string fname(filename);
    itksys_stl::string name = itksys::SystemTools::GetFilenameName(fname);
    modelNode->SetName(name.c_str());

    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene()); 

    this->GetMRMLScene()->AddNodeNoNotify(storageNode);  
    this->GetMRMLScene()->AddNodeNoNotify(displayNode);
    modelNode->SetStorageNodeID(storageNode->GetID());
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  
    displayNode->SetPolyData(modelNode->GetPolyData());
    
    this->GetMRMLScene()->AddNode(modelNode);  

    //this->Modified();  

    modelNode->Delete();
    }
  else
    {
    vtkDebugMacro("Couldn't read file, returning null model node: " << filename);
    modelNode->Delete();
    modelNode = NULL;
    }
  mStorageNode->Delete();
  fsmStorageNode->Delete();
  displayNode->Delete();

  return modelNode;  
}
//----------------------------------------------------------------------------
int vtkSlicerModelsLogic::SaveModel (const char* filename, vtkMRMLModelNode *modelNode)
{
   if (modelNode == NULL || filename == NULL)
    {
    vtkErrorMacro("SaveModel: unable to proceed, filename is " <<
                  (filename == NULL ? "null" : filename) <<
                  ", model node is " <<
                  (modelNode == NULL ? "null" : modelNode->GetID()));
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

//----------------------------------------------------------------------------
int vtkSlicerModelsLogic::AddScalar(const char* filename, vtkMRMLModelNode *modelNode)
{
  if (modelNode == NULL ||
      filename == NULL)
    {
    vtkErrorMacro("Model node or file name are null.");
    return 0;
    }  

   // get the storage node and use it to read the scalar file
  vtkMRMLFreeSurferModelStorageNode *storageNode = NULL;
  vtkMRMLStorageNode *snode = modelNode->GetStorageNode();
  if (snode != NULL)
    {
    storageNode = vtkMRMLFreeSurferModelStorageNode::SafeDownCast(snode);
    }
  if (storageNode == NULL)
    {
    vtkErrorMacro("Model "  << modelNode->GetName() << " does not have a freesurfer storage node associated with it, cannot load scalar overlay.");
    return 0;
    }
  storageNode->SetFileName(filename);
  storageNode->ReadData(modelNode);

  // check to see if the model display node has a colour node already
  vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
  if (displayNode == NULL)
    {
    vtkWarningMacro("Model " << modelNode->GetName() << "'s display node is null\n");
    }
  else
    {
    vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(displayNode->GetColorNode());
    if (colorNode == NULL)
      {
      vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
      displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultModelColorNodeID());
      colorLogic->Delete();
      }
    
    }
  return 1;
}
