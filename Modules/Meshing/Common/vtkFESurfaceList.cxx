
/*=========================================================================

  Module:    $RCSfile: vtkFESurfaceList.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/




#include "vtkMRMLScene.h"
#include "vtkDebugLeaks.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelStorageNode.h"

#include "vtkMRMLFESurfaceNode.h"
#include "vtkFESurfaceList.h"

//vtkCxxRevisionMacro(vtkFESurfaceList, "$Revision: 1.3 $");

vtkStandardNewMacro(vtkFESurfaceList);

vtkFESurfaceList::vtkFESurfaceList() 
{ 
  this->savedMRMLScene = vtkMRMLScene::GetActiveScene();
}

vtkFESurfaceList::~vtkFESurfaceList() 
{
    // need to delete all actors in the list, so there are no dangling references to 
    // the renderwindow.  Delete off of the front of the list
    int NumberOfItemsInList = this->GetNumberOfItems();
    for (int i=0; i<NumberOfItemsInList; i++)
    {
        this->RemoveItem(0);
        vtkDebugMacro("deleting Surface Actor");
    }
}

// save reference to the scene to be used for storage 
void vtkFESurfaceList::SetMRMLSceneForStorage(vtkMRMLScene* scene) 
{
    this->savedMRMLScene = scene;
    // each MRML node class type has to be registered with the scene

    vtkMRMLFESurfaceNode* newMRMLNode = vtkMRMLFESurfaceNode::New();
    // this->savedMRMLScene->RegisterNodeClass( newMRMLNode );
    vtkMRMLScene::GetActiveScene()->RegisterNodeClass(newMRMLNode);
    this->savedMRMLScene = vtkMRMLScene::GetActiveScene();
     newMRMLNode->Delete();
}


int vtkFESurfaceList::AppendItem(vtkMimxSurfacePolyDataActor* actor)
{
    
    static int registered=0;
 
  // allocate a new MRML node for this item and add it to the scene
   if (this->savedMRMLScene)
   {
     // create a node to contain the geometry 
     vtkMRMLFESurfaceNode* newMRMLNode = vtkMRMLFESurfaceNode::New();
     
     // if this is the first entry, then initialize the MRML scene
     if (!registered)
     {
       this->savedMRMLScene->RegisterNodeClass( newMRMLNode );
       registered=1;
     }
     
     // share the actor with the local list so all the values are populated correctly 
     newMRMLNode->SetMimxSurfacePolyDataActor(actor);
     newMRMLNode->SetAndObservePolyData(actor->GetDataSet());
     
     // create node to use for display and storage in slicer; use standard model
      // node initially to learn how display nodes work. Use our own 
      // subclasses later, possibly.  
      vtkMRMLModelDisplayNode* dispNode = vtkMRMLModelDisplayNode::New();
      vtkMRMLModelStorageNode* storeNode = vtkMRMLModelStorageNode::New();
      
      // for this version of the meshing module, we are using the Mimx
      // actors to render, so turn off the default MRML display of the geometry
      dispNode->SetVisibility(0);
      
      // *** this broke the mrml reload, why?
      //storeNode->SetFileName(actor->GetFileName());
 
      this->savedMRMLScene->AddNode(newMRMLNode);
      
      // Establish linkage between the surface
      // node and its display and storage nodes, so the viewer will be updated when data
      // or attributes change
      this->savedMRMLScene->AddNode(dispNode);
      this->savedMRMLScene->AddNode(storeNode);
      newMRMLNode->AddAndObserveDisplayNodeID(dispNode->GetID());
      newMRMLNode->SetAndObserveStorageNodeID(storeNode->GetID());
   }
   else {
       vtkErrorMacro("Attempted save to MRML, but scene not initialized");
       return VTK_ERROR;
   }
  return VTK_OK;
}


vtkMimxSurfacePolyDataActor* vtkFESurfaceList::GetItem(vtkIdType id)
{ 
   //return this->InternalMimxObjectList->GetItem(id);
       
   //  fetch the MRML node that has been requested
   vtkMRMLFESurfaceNode* requestedMrmlNode = 
       (vtkMRMLFESurfaceNode*)(this->savedMRMLScene->GetNthNodeByClass(id,"vtkMRMLFESurfaceNode"));
   // then get the actor from the MRML node and return the actor
   vtkMimxSurfacePolyDataActor* returnNode = requestedMrmlNode->GetMimxSurfacePolyDataActor();
   return returnNode;

}

int vtkFESurfaceList::GetNumberOfItems()
{
  //return this->InternalMimxObjectList->GetNumberOfItems();
  return this->savedMRMLScene->GetNumberOfNodesByClass("vtkMRMLFESurfaceNode");
}

int vtkFESurfaceList::RemoveItem(int Num)
{
  this->savedMRMLScene->RemoveNode(this->savedMRMLScene->GetNthNodeByClass(Num,"vtkMRMLFESurfaceNode"));
  return VTK_OK;
}
