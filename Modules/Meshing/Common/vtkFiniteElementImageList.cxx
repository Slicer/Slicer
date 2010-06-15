
/*=========================================================================

  Module:    $RCSfile: vtkFiniteElementImageList.cxx,v $

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


#include "vtkMRMLFiniteElementImageNode.h"
#include "vtkFiniteElementImageList.h"

#include "vtkMRMLVolumeArchetypeStorageNode.h"


vtkStandardNewMacro(vtkFiniteElementImageList);

vtkFiniteElementImageList::vtkFiniteElementImageList()
{ 
  this->savedMRMLScene = vtkMRMLScene::GetActiveScene();
}

vtkFiniteElementImageList::~vtkFiniteElementImageList()
{
    // need to delete all actors in the list, so there are no dangling references to 
    // the renderwindow.  Delete off of the front of the list
    int NumberOfItemsInList = this->GetNumberOfItems();
    for (int i=0; i<NumberOfItemsInList; i++)
    {
        this->RemoveItem(0);
        vtkDebugMacro("deleting Image Actor");
    }
}

// save reference to the scene to be used for storage 
void vtkFiniteElementImageList::SetMRMLSceneForStorage(vtkMRMLScene* scene)
{
    this->savedMRMLScene = scene;
    // each MRML node class type has to be registered with the scene

    vtkMRMLFiniteElementImageNode* newMRMLNode = vtkMRMLFiniteElementImageNode::New();
    vtkMRMLScene::GetActiveScene()->RegisterNodeClass(newMRMLNode);
    this->savedMRMLScene = vtkMRMLScene::GetActiveScene();
    newMRMLNode->Delete();
}


int vtkFiniteElementImageList::AppendItem(vtkMimxImageActor* actor)
{
    
    static int registered=0;
 
  // allocate a new MRML node for this item and add it to the scene
   if (this->savedMRMLScene)
   {
     // create a node to contain the geometry 
     vtkMRMLFiniteElementImageNode* newMRMLNode = vtkMRMLFiniteElementImageNode::New();
     
     // if this is the first entry, then initialize the MRML scene
     if (!registered)
     {
       this->savedMRMLScene->RegisterNodeClass( newMRMLNode );
       registered=1;
     }
     
     // share the actor with the local list so all the values are populated correctly 
     newMRMLNode->SetMimxImageActor(actor);
     newMRMLNode->SetAndObserveImageData(actor->GetDataSet());
     newMRMLNode->SetRASToIJKMatrix(actor->GetRASToIJKMatrix());
     newMRMLNode->SetOrigin(actor->Origin);
     newMRMLNode->SetSpacing(actor->Spacing);

     // create node to use for display and storage in slicer; use standard
     // node initially to learn how display nodes work. Use our own
     // subclasses later.

     // for this version of the meshing module, we are using the Mimx
     // actors to render, so turn off the default MRML display of the image
     
     vtkMRMLScalarVolumeDisplayNode* dispNode = vtkMRMLScalarVolumeDisplayNode::New();
     dispNode->SetDefaultColorMap();
     dispNode->SetScene(this->savedMRMLScene);
     dispNode->SetVisibility(1);

      vtkMRMLVolumeArchetypeStorageNode* storeNode = vtkMRMLVolumeArchetypeStorageNode::New();
      storeNode->SetScene(this->savedMRMLScene);

      // Establish linkage between the image node and display and storage nodes is added
      // here.  There is no display node currently because the image actor is directly
      // handling the rendering in this version.
      
      this->savedMRMLScene->AddNode(newMRMLNode);
      this->savedMRMLScene->AddNode(storeNode);
      this->savedMRMLScene->AddNode(dispNode);
      this->savedMRMLScene->AddNode(dispNode);
      newMRMLNode->AddAndObserveDisplayNodeID(dispNode->GetID());
      newMRMLNode->AddAndObserveStorageNodeID(storeNode->GetID());

      dispNode->Delete();
      storeNode->Delete();
      newMRMLNode->Delete();
   }
   else {
       vtkErrorMacro("Attempted save to MRML, but scene not initialized");
       return VTK_ERROR;
   }
  return VTK_OK;
}


vtkMimxImageActor* vtkFiniteElementImageList::GetItem(vtkIdType id)
{ 
   //return this->InternalMimxObjectList->GetItem(id);
       
   //  fetch the MRML node that has been requested
   vtkMRMLFiniteElementImageNode* requestedMrmlNode =
       (vtkMRMLFiniteElementImageNode*)(this->savedMRMLScene->GetNthNodeByClass(id,"vtkMRMLFiniteElementImageNode"));
   // then get the actor from the MRML node and return the actor
   vtkMimxImageActor* returnNode = requestedMrmlNode->GetMimxImageActor();
   return returnNode;

}

int vtkFiniteElementImageList::GetNumberOfItems()
{
  //return this->InternalMimxObjectList->GetNumberOfItems();
  return this->savedMRMLScene->GetNumberOfNodesByClass("vtkMRMLFiniteElementImageNode");
}

int vtkFiniteElementImageList::RemoveItem(int Num)
{
  this->savedMRMLScene->RemoveNode(this->savedMRMLScene->GetNthNodeByClass(Num,"vtkMRMLFiniteElementImageNode"));
  return VTK_OK;
}
