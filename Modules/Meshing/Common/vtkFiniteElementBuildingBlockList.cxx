
/*=========================================================================

  Module:    $RCSfile: vtkFiniteElementBuildingBlockList.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkFiniteElementBuildingBlockList.h"
#include "vtkMRMLFiniteElementBuildingBlockNode.h"
#include "vtkMRMLFiniteElementBuildingBlockDisplayNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkMRMLScene.h"
#include "vtkDebugLeaks.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkDataSetWriter.h"

//vtkCxxRevisionMacro(vtkFiniteElementBuildingBlockList, "$Revision: 1.3 $");

vtkStandardNewMacro(vtkFiniteElementBuildingBlockList);

vtkFiniteElementBuildingBlockList::vtkFiniteElementBuildingBlockList() 
{ 
    SetMRMLSceneForStorage(NULL);
}

vtkFiniteElementBuildingBlockList::~vtkFiniteElementBuildingBlockList() 
{
    // need to delete all actors in the list, so there are no dangling references to 
    // the renderwindow.  Delete off of the front of the list
    int NumberOfItemsInList = this->GetNumberOfItems();
    for (int i=0; i<NumberOfItemsInList; i++)
    {
        this->RemoveItem(0);
        vtkDebugMacro("deleting BBox Actor");
    }
}

// save reference to the scene to be used for storage 
void vtkFiniteElementBuildingBlockList::SetMRMLSceneForStorage(vtkMRMLScene* scene) 
{
    // the value passed from the module was NULL, so use the Scene class to return it
    //this->savedMRMLScene = scene;
    this->savedMRMLScene = vtkMRMLScene::GetActiveScene();
    // each node type should be registered once in the MRML scene, so we do it here when the 
    // MRML scene is set, which is called only once per slicer session. 
    vtkMRMLFiniteElementBuildingBlockNode* feBBNode = vtkMRMLFiniteElementBuildingBlockNode::New();
    vtkMRMLFiniteElementBuildingBlockDisplayNode* feBBDispNode = vtkMRMLFiniteElementBuildingBlockDisplayNode::New();
    //vtkMRMLFiniteElementBuildingBlockStorageNode* feBBStoreNode = vtkMRMLFiniteElementBuildingBlockStorageNode::New();
    this->savedMRMLScene->RegisterNodeClass(feBBNode);
    this->savedMRMLScene->RegisterNodeClass(feBBDispNode);
    feBBNode->Delete();
    feBBDispNode->Delete();
}


int vtkFiniteElementBuildingBlockList::AppendItem(vtkMimxUnstructuredGridActor* actor)
{ 
   if (this->savedMRMLScene)
   {
     // allocate a new MRML node for this item and add it to the scene
     vtkMRMLFiniteElementBuildingBlockNode* newMRMLNode = vtkMRMLFiniteElementBuildingBlockNode::New();

     // copy the state variables to the MRML node
     newMRMLNode->SetMimxUnstructuredGridActor(actor);
     newMRMLNode->SetAndObserveUnstructuredGrid(actor->GetDataSet());
     
     // now add the display, storage, and displayable nodes
     vtkMRMLFiniteElementBuildingBlockDisplayNode* dispNode = vtkMRMLFiniteElementBuildingBlockDisplayNode::New();
     vtkMRMLUnstructuredGridStorageNode* storeNode = vtkMRMLUnstructuredGridStorageNode::New();

     // for this version of the meshing module, we are using the Mimx
     // actors to render, so turn off the default MRML display of the geometry
     dispNode->SetVisibility(0);
   
     
     dispNode->SetScene(this->savedMRMLScene);
     storeNode->SetScene(this->savedMRMLScene);
     this->savedMRMLScene->AddNode(newMRMLNode);
     this->savedMRMLScene->AddNodeNoNotify(dispNode);
     this->savedMRMLScene->AddNodeNoNotify(storeNode);
     
     //cout << "added mrml bblock node " << endl;

     // Establish linkage between the bounding box
     // node and its display and storage nodes, so the viewer will be updated when data
     // or attributes change
     // *** commented out next two lines since we are using Mimx Actors for this release.
     // set Ugrid was causing a crash on Mac
     //dispNode->SetUnstructuredGrid(newMRMLNode->GetUnstructuredGrid());
     //newMRMLNode->AddAndObserveDisplayNodeID(dispNode->GetID());
     newMRMLNode->SetAndObserveStorageNodeID(storeNode->GetID());   
     //vtkDebugMacro("copied data to MRML bbox node ");
     newMRMLNode->Modified();
     
   } else 
   {
     vtkErrorMacro("MeshingWorkflow: Adding to uninitialized MRML Scene");
     return VTK_ERROR;
   }
  return VTK_OK;
}



vtkMimxUnstructuredGridActor* vtkFiniteElementBuildingBlockList::GetItem(vtkIdType id)
{
    
  // first fetch the MRML node that has been requested
  vtkMRMLFiniteElementBuildingBlockNode* requestedMrmlNode = 
      (vtkMRMLFiniteElementBuildingBlockNode*)(this->savedMRMLScene->GetNthNodeByClass(id,"vtkMRMLFiniteElementBuildingBlockNode"));
  return requestedMrmlNode->GetMimxUnstructuredGridActor();
}


int vtkFiniteElementBuildingBlockList::GetNumberOfItems()
{
  return this->savedMRMLScene->GetNumberOfNodesByClass("vtkMRMLFiniteElementBuildingBlockNode");
}

int vtkFiniteElementBuildingBlockList::RemoveItem(int Num)
{
  this->savedMRMLScene->RemoveNode(this->savedMRMLScene->GetNthNodeByClass(Num,"vtkMRMLFiniteElementBuildingBlockNode"));
  return VTK_OK;
}
