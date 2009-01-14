
/*=========================================================================

  Module:    $RCSfile: vtkFiniteElementMeshList.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkFiniteElementMeshList.h"
#include "vtkMRMLFiniteElementMeshNode.h"
#include "vtkMRMLScene.h"
#include "vtkDebugLeaks.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLFiniteElementMeshDisplayNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkMimxMeshActor.h"
#include "vtkMRMLColorTableNode.h"

//vtkCxxRevisionMacro(vtkFiniteElementMeshList, "$Revision: 1.3 $");

vtkStandardNewMacro(vtkFiniteElementMeshList);

vtkFiniteElementMeshList::vtkFiniteElementMeshList() 
{ 
    this->SetMRMLSceneForStorage(vtkMRMLScene::GetActiveScene());

}

vtkFiniteElementMeshList::~vtkFiniteElementMeshList() 
{
  // need to delete all actors in the list, so there are no dangling references to 
  // the renderwindow.  Delete off of the front of the list
  int NumberOfItemsInList = this->GetNumberOfItems();
  for (int i=0; i<NumberOfItemsInList;i++)
    {
    this->RemoveItem(0);
    vtkDebugMacro("deleting Mesh Actor");
    }
  
}

// save reference to the scene to be used for storage 
void vtkFiniteElementMeshList::SetMRMLSceneForStorage(vtkMRMLScene* scene) 
{
    this->savedMRMLScene = scene;
    // each MRML class type needs to be registeredv
    vtkMRMLFiniteElementMeshNode* meshListNode = vtkMRMLFiniteElementMeshNode::New();
    this->savedMRMLScene->RegisterNodeClass(meshListNode);
    meshListNode->Delete();
}


int vtkFiniteElementMeshList::AppendItem(vtkMimxMeshActor* actor)
{
   if (this->savedMRMLScene)
   {
     // allocate a new MRML node for this item and add it to the scene
     vtkMRMLFiniteElementMeshNode* newMRMLNode = vtkMRMLFiniteElementMeshNode::New();
     newMRMLNode->SetMimxMeshActor(actor);
     newMRMLNode->SetAndObserveUnstructuredGrid(actor->GetDataSet());
     
     // now add the display and storage nodes
      vtkMRMLFiniteElementMeshDisplayNode* dispNode = vtkMRMLFiniteElementMeshDisplayNode::New();
      vtkMRMLUnstructuredGridStorageNode* storeNode = vtkMRMLUnstructuredGridStorageNode::New();
      vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::New();

      // for this version of the meshing module, we are using the Mimx
      // actors to render, so turn off the default MRML display of the geometry
      dispNode->SetVisibility(0);
 
      // this sets the default display to be colored based on selectable attribute value
      colorNode->SetTypeToRainbow();
       
      // Establish linkage between the surface
      // node and its display and storage nodes, so the viewer will be updated when data
      // or attributes change
      dispNode->SetScene(this->savedMRMLScene);
      storeNode->SetScene(this->savedMRMLScene);
      colorNode->SetScene(this->savedMRMLScene);
      this->savedMRMLScene->AddNodeNoNotify(dispNode);
      this->savedMRMLScene->AddNodeNoNotify(storeNode);
      this->savedMRMLScene->AddNodeNoNotify(colorNode);
      this->savedMRMLScene->AddNode(newMRMLNode);
      
      // ** commented out display node to avoid crash on mac after 11/17 update
      // point the display node to the proper grid
      //dispNode->SetUnstructuredGrid(newMRMLNode->GetUnstructuredGrid());
      // set the color node to specify the color table associated with the grid
      //dispNode->SetAndObserveColorNodeID(colorNode->GetID());
      // need to turn this on so the scalars are used to color the grid
      //dispNode->SetScalarVisibility(1);
      //newMRMLNode->AddAndObserveDisplayNodeID(dispNode->GetID());
      
      newMRMLNode->SetAndObserveStorageNodeID(storeNode->GetID());      
     //cout << "copied data to MRML mesh node " << endl;
   } else 
   {
       vtkErrorMacro("MeshingWorkflow: Adding FEMesh to uninitialized MRML Scene");
   }
  return VTK_OK;
}


vtkMimxMeshActor* vtkFiniteElementMeshList::GetItem(vtkIdType id)
{
    //return this->InternalMimxObjectList->GetItem(id);
     
  // first fetch the MRML node that has been requested
  vtkMRMLFiniteElementMeshNode* requestedMrmlNode = 
      (vtkMRMLFiniteElementMeshNode*)(this->savedMRMLScene->GetNthNodeByClass(id,"vtkMRMLFiniteElementMeshNode"));
  return requestedMrmlNode->GetMimxMeshActor();

}


int vtkFiniteElementMeshList::GetNumberOfItems()
{
  //return this->InternalMimxObjectList->GetNumberOfItems();
  return this->savedMRMLScene->GetNumberOfNodesByClass("vtkMRMLFiniteElementMeshNode");
}

int vtkFiniteElementMeshList::RemoveItem(int Num)
{

  this->savedMRMLScene->RemoveNode(this->savedMRMLScene->GetNthNodeByClass(Num,"vtkMRMLFiniteElementMeshNode"));
  return VTK_OK;
}
