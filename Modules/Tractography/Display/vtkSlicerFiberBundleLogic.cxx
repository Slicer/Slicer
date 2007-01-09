/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiberBundleLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

#include "vtkSlicerFiberBundleLogic.h"
#include "vtkSlicerFiberBundleDisplayLogic.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"

vtkCxxRevisionMacro(vtkSlicerFiberBundleLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerFiberBundleLogic);

//----------------------------------------------------------------------------
vtkSlicerFiberBundleLogic::vtkSlicerFiberBundleLogic()
{
  this->ActiveFiberBundleNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerFiberBundleLogic::~vtkSlicerFiberBundleLogic()
{
  if (this->ActiveFiberBundleNode != NULL)
    {
        this->ActiveFiberBundleNode->Delete();
        this->ActiveFiberBundleNode = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleLogic::ProcessMRMLEvents(vtkObject * /*caller*/, 
                                            unsigned long /*event*/, 
                                            void * /*callData*/)
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleLogic::SetActiveFiberBundleNode(vtkMRMLFiberBundleNode *activeNode)
{
  vtkSetMRMLNodeMacro(this->ActiveFiberBundleNode, activeNode );
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkSlicerFiberBundleLogic::AddFiberBundles (const char* dirname, const char* suffix )
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
        if (this->AddFiberBundle((char *)fullPath.c_str()) == NULL) 
          {
          res = 0;
          }
        }
      }
  }
  return res;
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleNode* vtkSlicerFiberBundleLogic::AddFiberBundle (char* filename)
{
  vtkDebugWithObjectMacro(this,"Adding fiber bundle from filename " << filename);

  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::New();
  vtkMRMLFiberBundleDisplayNode *displayNode = vtkMRMLFiberBundleDisplayNode::New();
  vtkMRMLFiberBundleStorageNode *storageNode = vtkMRMLFiberBundleStorageNode::New();

  storageNode->SetFileName(filename);
  if (storageNode->ReadData(fiberBundleNode) != 0)
    {
    const itksys_stl::string fname(filename);
    itksys_stl::string name = itksys::SystemTools::GetFilenameName(fname);
    fiberBundleNode->SetName(name.c_str());

    this->GetMRMLScene()->SaveStateForUndo();

    fiberBundleNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);
    fiberBundleNode->SetStorageNodeID(storageNode->GetID());
    fiberBundleNode->SetAndObserveDisplayNodeID(displayNode->GetID());  

    this->GetMRMLScene()->AddNode(fiberBundleNode);  

    vtkSlicerFiberBundleDisplayLogic *displayLogic = vtkSlicerFiberBundleDisplayLogic::New();
    displayLogic->DebugOn();
    // observe the bundle node which observes the display node.
    displayLogic->SetAndObserveFiberBundleNode(fiberBundleNode);
    // TO DO: work in progress, need to observe the displayNode to insert
    // "fake" models into MRML scene for tract display
    displayLogic->SetAndObserveMRMLScene(this->GetMRMLScene());
    //displayLogic->SetMRMLScene(this->GetMRMLScene());
    // Lauren put this logic element on a collection and DELETE it.

    this->Modified();  

    fiberBundleNode->Delete();
    }
  else
    {
    vtkDebugMacro("Couldn't read file, returning null fiberBundle node: " << filename);
    fiberBundleNode->Delete();
    fiberBundleNode = NULL;
    }
  storageNode->Delete();
  displayNode->Delete();
  //displayLogic->Delete();

  return fiberBundleNode;  
}
//----------------------------------------------------------------------------
int vtkSlicerFiberBundleLogic::SaveFiberBundle (char* filename, vtkMRMLFiberBundleNode *fiberBundleNode)
{
   if (fiberBundleNode == NULL || filename == NULL)
    {
    return 0;
    }
  
  vtkMRMLFiberBundleStorageNode *storageNode = NULL;
  vtkMRMLStorageNode *snode = fiberBundleNode->GetStorageNode();
  if (snode != NULL)
    {
    storageNode = vtkMRMLFiberBundleStorageNode::SafeDownCast(snode);
    }
  if (storageNode == NULL)
    {
    storageNode = vtkMRMLFiberBundleStorageNode::New();
    storageNode->SetScene(this->GetMRMLScene());
    this->GetMRMLScene()->AddNode(storageNode);  
    fiberBundleNode->SetStorageNodeID(storageNode->GetID());
    storageNode->Delete();
    }

  //storageNode->SetAbsoluteFileName(true);
  storageNode->SetFileName(filename);

  int res = storageNode->WriteData(fiberBundleNode);

  
  return res;

}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiberBundleLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveFiberBundleNode: " <<
    (this->ActiveFiberBundleNode ? this->ActiveFiberBundleNode->GetName() : "(none)") << "\n";
}


