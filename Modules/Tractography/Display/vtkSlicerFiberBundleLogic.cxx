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
void vtkSlicerFiberBundleLogic::ProcessMRMLEvents(vtkObject * caller, 
                                            unsigned long event, 
                                            void * callData)
{

  if (this->MRMLScene == NULL)
    {
    vtkErrorMacro("Can't process MRML events, no MRMLScene set.");
    return;
    }

  // If new scene with fiber bundle nodes, set up display logic properly.
  // Make sure to only handle new bundle nodes (not already displayed).
  // GetNthNodeByClass is how to loop.
  if (vtkMRMLScene::SafeDownCast(caller) != NULL
      && (event == vtkMRMLScene::NewSceneEvent))
    {

    vtkErrorMacro("New scene event");

    // Loop through all of the fiberBundleNodes.
    // If the node does not have a display logic node yet, then make one for it.
    vtkMRMLFiberBundleNode *node= NULL;
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiberBundleNode");
    for (int n=0; n<nnodes; n++)
      {
      node = 
        vtkMRMLFiberBundleNode::
        SafeDownCast (this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLFiberBundleNode"));

      if (node == NULL)
        {
        vtkErrorMacro("Got null node.");
        }
      else
        {
        // Set up display logic and any other logic classes in future
        this->InitializeLogicForFiberBundleNode(node);
        }
      
      }
    }
}

void vtkSlicerFiberBundleLogic::InitializeLogicForFiberBundleNode(vtkMRMLFiberBundleNode *node)
{
  vtkErrorMacro("Adding display logic");
  // Lauren here test if we already HAVE a display logic!
  // Lauren put this logic element on a collection and DELETE it.
  // If we have a new fiber bundle node set up its display logic
  vtkSlicerFiberBundleDisplayLogic *displayLogic = vtkSlicerFiberBundleDisplayLogic::New();
  //displayLogic->DebugOn();
  // set the MRML scene so display logic can add its extra model nodes
  displayLogic->SetMRMLScene(this->GetMRMLScene());
  // Observe the bundle node which observes the display node, in order to update display
  // when display or polydata change
  displayLogic->SetAndObserveFiberBundleNode(node);
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
  vtkErrorMacro("Adding fiber bundle from filename " << filename);

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

    // Set up display logic and any other logic classes in future
    this->InitializeLogicForFiberBundleNode(fiberBundleNode);

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


