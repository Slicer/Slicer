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

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkCollection.h"

#include "vtkMRMLSceneSnapshotNode.h"

#include "vtkMRMLScene.h"

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
    this->Nodes->RemoveAllItems();
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
  vtkMRMLNode * node = NULL;
  int n;
  for (n=0; n < this->Nodes->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->Nodes->GetItemAsObject(n);
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
  Superclass::ProcessChildNode(node);
  node->SetAddToScene(0);
  if (this->Nodes == NULL)
    {
    this->Nodes = vtkCollection::New();
    }  
  this->Nodes->vtkCollection::AddItem((vtkObject *)node);
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
    this->Nodes = vtkCollection::New();
    }
  else
    {
    this->Nodes->RemoveAllItems();
    }
  vtkMRMLNode *node = NULL;
  int n;
  for (n=0; n < snode->Nodes->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)snode->Nodes->GetItemAsObject(n);
    if (node)
      {
      this->Nodes->vtkCollection::AddItem((vtkObject *)node);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSceneSnapshotNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
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
    this->Nodes = vtkCollection::New();
    }
  else
    {
    this->Nodes->RemoveAllItems();
    }

  vtkMRMLNode *node = NULL;
  int n;
  for (n=0; n < this->Scene->GetNumberOfNodes(); n++) 
    {
    node = this->Scene->GetNthNode(n);
    if (node && !node->IsA("vtkMRMLSceneSnapshotNode") && !node->IsA("vtkMRMLSnapshotClipNode")  && node->GetSaveWithScene() )
      {
      vtkMRMLNode *newNode = node->CreateNodeInstance();
      newNode->CopyWithScene(node);
      this->Nodes->vtkCollection::AddItem((vtkObject *)newNode);
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

  unsigned int nnodesSanpshot = this->Nodes->GetNumberOfItems();
  unsigned int n;
  vtkMRMLNode *node = NULL;

  // remove nodes in the scene which are not stored in the snapshot
  std::map<std::string, vtkMRMLNode*> snapshotMap;
  for (n=0; n<nnodesSanpshot; n++) 
    {
    node  = dynamic_cast < vtkMRMLNode *>(this->Nodes->GetItemAsObject(n));
    if (node) 
      {
      snapshotMap[node->GetID()] = node;
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
    node = (vtkMRMLNode*)this->Nodes->GetItemAsObject(n);
    if (node)
      {
      vtkMRMLNode *snode = this->Scene->GetNodeByID(node->GetID());
      if (snode)
        {
        snode->CopyWithSceneWithSingleModifiedEvent(node);
        }
      else 
        {
        node->SetAddToScene(1);
        this->Scene->AddNodeNoNotify(node);
        addedNodes.push_back(node);
        }
      }
    }
  for(n=0; n<addedNodes.size(); n++)
    {
    addedNodes[n]->UpdateScene(this->Scene);
    this->Scene->InvokeEvent(vtkMRMLScene::NodeAddedEvent, addedNodes[n] );
    }

}
// End
