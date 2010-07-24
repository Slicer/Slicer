/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkCollection.h"

#include "vtkMRMLSceneSnapshotNode.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

//------------------------------------------------------------------------------
vtkMRMLSceneSnapshotNode* vtkMRMLSceneSnapshotNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSceneSnapshotNode");
  if(ret)
    {
    return (vtkMRMLSceneSnapshotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSceneSnapshotNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSceneSnapshotNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSceneSnapshotNode");
  if(ret)
    {
    return (vtkMRMLSceneSnapshotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSceneSnapshotNode;
}

//----------------------------------------------------------------------------
vtkMRMLSceneSnapshotNode::vtkMRMLSceneSnapshotNode()
{
  this->HideFromEditors = 1;

  this->Nodes = NULL;

}

//----------------------------------------------------------------------------
vtkMRMLSceneSnapshotNode::~vtkMRMLSceneSnapshotNode()
{
  if (this->Nodes) 
    {
    this->Nodes->GetCurrentScene()->RemoveAllItems();
    this->Nodes->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::WriteNodeBodyXML(ostream& of, int nIndent)
{
  this->SetAbsentStorageFileNames();

  vtkMRMLNode * node = NULL;
  int n;
  for (n=0; n < this->Nodes->GetCurrentScene()->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->Nodes->GetCurrentScene()->GetItemAsObject(n);
    if (node && !node->IsA("vtkMRMLSceneSnapshotNode") && node->GetSaveWithScene())
      {
      vtkIndent vindent(nIndent+1);
      of << vindent << "<" << node->GetNodeTagName() << "\n";

      node->WriteXML(of, nIndent + 2);

      of << vindent << ">";
      node->WriteNodeBodyXML(of, nIndent+1);
      of << "</" << node->GetNodeTagName() << ">\n";
      }
    }
    
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::ProcessChildNode(vtkMRMLNode *node)
{
  int disabledModify = node->StartModify();

  Superclass::ProcessChildNode(node);
  node->SetAddToSceneNoModify(0);

  node->EndModify(disabledModify);

  if (this->Nodes == NULL)
    {
    this->Nodes = vtkMRMLScene::New();
    }  
  node->SetScene(this->Nodes);
  this->Nodes->GetCurrentScene()->vtkCollection::AddItem((vtkObject *)node);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLSceneSnapshotNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSceneSnapshotNode *snode = (vtkMRMLSceneSnapshotNode *) anode;

  if (this->Nodes == NULL)
    {
    this->Nodes = vtkMRMLScene::New();
    }
  else
    {
    this->Nodes->GetCurrentScene()->RemoveAllItems();
    }
  vtkMRMLNode *node = NULL;
  if ( snode->Nodes != NULL )
    {
    int n;
    for (n=0; n < snode->Nodes->GetCurrentScene()->GetNumberOfItems(); n++) 
      {
      node = (vtkMRMLNode*)snode->Nodes->GetCurrentScene()->GetItemAsObject(n);
      if (node)
        {
        this->Nodes->GetCurrentScene()->vtkCollection::AddItem((vtkObject *)node);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::UpdateScene(vtkMRMLScene *scene)
{
  this->Nodes->CopyNodeReferences(scene);
  this->Nodes->UpdateNodeChangedIDs();
  this->Nodes->UpdateNodeReferences();
  this->UpdateSnapshotScene(this->Nodes);
}
//----------------------------------------------------------------------------

void vtkMRMLSceneSnapshotNode::UpdateSnapshotScene(vtkMRMLScene *)
{
  if (this->Scene == NULL)
    {
    return;
    }

  if (this->Nodes == NULL)
    {
    return;
    }

  unsigned int nnodesSanpshot = this->Nodes->GetCurrentScene()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = NULL;

  // prevent data read in UpdateScene
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetCurrentScene()->GetItemAsObject(n));
    if (node) 
      {
      node->SetAddToSceneNoModify(0);
      }
    }

  // update nodes in the snapshot
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetCurrentScene()->GetItemAsObject(n));
    if (node) 
      {
      node->UpdateScene(this->Nodes);
      }
    }

  /**
  // update nodes in the snapshot
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetCurrentScene()->GetItemAsObject(n));
    if (node) 
      {
      node->SetAddToSceneNoModify(1);
      }
    }
    ***/
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::StoreScene()
{
  if (this->Scene == NULL)
    {
    return;
    }

  if (this->Nodes == NULL)
    {
    this->Nodes = vtkMRMLScene::New();
    }
  else
    {
    this->Nodes->GetCurrentScene()->RemoveAllItems();
    }

  if (this->GetScene())
    {
    this->Nodes->SetRootDirectory(this->GetScene()->GetRootDirectory());
    }

  vtkMRMLNode *node = NULL;
  int n;
  for (n=0; n < this->Scene->GetNumberOfNodes(); n++) 
    {
    node = this->Scene->GetNthNode(n);
    if (node && !node->IsA("vtkMRMLSceneSnapshotNode") && !node->IsA("vtkMRMLSnapshotClipNode")  && node->GetSaveWithScene() )
      {
      vtkMRMLNode *newNode = node->CreateNodeInstance();
      newNode->CopyWithoutModifiedEvent(node);
      newNode->SetScene(this->Nodes);
      newNode->SetAddToSceneNoModify(0);
      newNode->CopyID(node);

      this->Nodes->GetCurrentScene()->vtkCollection::AddItem((vtkObject *)newNode);
      //--- Try deleting copy after collection has a reference to it,
      //--- in order to eliminate debug leaks..
      newNode->Delete();
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::RestoreScene()
{
  if (this->Scene == NULL)
    {
    return;
    }

  if (this->Nodes == NULL)
    {
    return;
    }

  unsigned int nnodesSanpshot = this->Nodes->GetCurrentScene()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = NULL;

  //this->Scene->SetIsClosing(1);
  this->Scene->IsRestoring++;
  this->Scene->InvokeEvent(vtkMRMLScene::SceneAboutToBeRestoredEvent, NULL);

  // remove nodes in the scene which are not stored in the snapshot
  std::map<std::string, vtkMRMLNode*> snapshotMap;
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetCurrentScene()->GetItemAsObject(n));
    if (node) 
      {
      /***
      const char *newID = this->Scene->GetChangedID(node->GetID());
      if (newID)
        {
        snapshotMap[newID] = node;
        }
      else
        {
        snapshotMap[node->GetID()] = node;
        }
      ***/
      if (node->GetID()) 
        {
        snapshotMap[node->GetID()] = node;
        }
      }
    }
  std::vector<vtkMRMLNode*> removedNodes;
  unsigned int nnodesScene = this->Scene->GetNumberOfNodes();
  for (n=0; n<nnodesScene; n++)
    {
    node = this->Scene->GetNthNode(n);
    if (node)
      {
      std::map<std::string, vtkMRMLNode*>::iterator iter = snapshotMap.find(std::string(node->GetID()));
      if (iter == snapshotMap.end() && !node->IsA("vtkMRMLSceneSnapshotNode") && !node->IsA("vtkMRMLSnapshotClipNode") && node->GetSaveWithScene())
        {
        removedNodes.push_back(node);
        }
      }
    }
  for(n=0; n<removedNodes.size(); n++)
    {
    this->Scene->RemoveNode(removedNodes[n]);
    }

  std::vector<vtkMRMLNode *> addedNodes;
  for (n=0; n < nnodesSanpshot; n++) 
    {
    node = (vtkMRMLNode*)this->Nodes->GetCurrentScene()->GetItemAsObject(n);
    if (node)
      {
      /***
      const char *newID = this->Scene->GetChangedID(node->GetID());
      if (newID == NULL)
        {
        newID = node->GetID();
        }
      vtkMRMLNode *snode = this->Scene->GetNodeByID(newID);
      ***/
      
      vtkMRMLNode *snode = this->Scene->GetNodeByID(node->GetID());

      if (snode)
        {
        snode->SetScene(this->Scene);
        // to prevent copying of default info if not stored in sanpshot
        snode->CopyWithSingleModifiedEvent(node);
        // to prevent reading data on UpdateScene()
        snode->SetAddToSceneNoModify(0);
        }
      else 
        {
        addedNodes.push_back(node);
        node->SetAddToSceneNoModify(1);
        this->Scene->AddNodeNoNotify(node);
        // to prevent reading data on UpdateScene()
        // but new nodes should read their data
        //node->SetAddToSceneNoModify(0);
        }
      }
    }

  // update all nodes in the scene

  //this->Scene->UpdateNodeReferences(this->Nodes);

  nnodesScene = this->Scene->GetNumberOfNodes();
  for (n=0; n<nnodesScene; n++) 
    {
    node = this->Scene->GetNthNode(n);
    if(!node->IsA("vtkMRMLSceneSnapshotNode") && !node->IsA("vtkMRMLSnapshotClipNode") && node->GetSaveWithScene())
      {
      node->UpdateScene(this->Scene);
      }
    }

  // reset AddToScene
  for (n=0; n < nnodesSanpshot; n++) 
    {
    node = (vtkMRMLNode*)this->Nodes->GetCurrentScene()->GetItemAsObject(n);
    if (node)
      {
      node->SetAddToSceneNoModify(1);
      }
    }

  //this->Scene->SetIsClosing(0);
  for(n=0; n<addedNodes.size(); n++)
    {
    //addedNodes[n]->UpdateScene(this->Scene);
    this->Scene->InvokeEvent(vtkMRMLScene::NodeAddedEvent, addedNodes[n] );
    }

  this->Scene->IsRestoring--;

  this->Scene->InvokeEvent(vtkMRMLScene::SceneRestoredEvent, this);
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::SetAbsentStorageFileNames()
{
  if (this->Scene == NULL)
    {
    return;
    }

  if (this->Nodes == NULL)
    {
    return;
    }

  unsigned int nnodesSanpshot = this->Nodes->GetCurrentScene()->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = NULL;

  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetCurrentScene()->GetItemAsObject(n));
    if (node) 
      {
      // for storage nodes replace full path with relative
      vtkMRMLStorageNode *snode = vtkMRMLStorageNode::SafeDownCast(node);
      if (snode && (snode->GetFileName() == NULL || std::string(snode->GetFileName()) == "") )
        {
        vtkMRMLNode *node1 = this->Scene->GetNodeByID(snode->GetID());
        if (node1)
          {
          vtkMRMLStorageNode *snode1 = vtkMRMLStorageNode::SafeDownCast(node1);
          if (snode1)
            {
            snode->SetFileName(snode1->GetFileName());
            }
          }
        }
      } //if (node) 
    } //for (n=0; n<nnodesSanpshot; n++) 
}
