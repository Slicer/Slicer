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

#include "vtkMRMLSnapshotClipNode.h"

#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLSnapshotClipNode* vtkMRMLSnapshotClipNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSnapshotClipNode");
  if(ret)
    {
    return (vtkMRMLSnapshotClipNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSnapshotClipNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSnapshotClipNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSnapshotClipNode");
  if(ret)
    {
    return (vtkMRMLSnapshotClipNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSnapshotClipNode;
}

//----------------------------------------------------------------------------
vtkMRMLSnapshotClipNode::vtkMRMLSnapshotClipNode()
{
  this->HideFromEditors = 1;

  this->SceneSnapshotNodes = vtkCollection::New();

}

//----------------------------------------------------------------------------
vtkMRMLSnapshotClipNode::~vtkMRMLSnapshotClipNode()
{
  if (this->SceneSnapshotNodes) 
    {
    this->SceneSnapshotNodes->RemoveAllItems();
    this->SceneSnapshotNodes->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSnapshotClipNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  vtkMRMLSceneSnapshotNode * node = NULL;
  std::stringstream ss;
  int n;
  for (n=0; n < this->SceneSnapshotNodes->GetNumberOfItems(); n++) 
    {
    node = vtkMRMLSceneSnapshotNode::SafeDownCast(this->SceneSnapshotNodes->GetItemAsObject(n));
    ss << node->GetID();
    if (n < this->SceneSnapshotNodes->GetNumberOfItems()-1)
      {
      ss << " ";
      }
    }
    of << indent << " sceneSnapshotIDs=\"" << ss.str().c_str() << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLSnapshotClipNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  this->SceneSnapshotNodeIDs.clear();

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "sceneSnapshotIDs")) 
      {
      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        this->SceneSnapshotNodeIDs.push_back(id);
        }
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLSnapshotClipNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSnapshotClipNode *snode = (vtkMRMLSnapshotClipNode *) anode;

  if (this->SceneSnapshotNodes == NULL)
    {
    this->SceneSnapshotNodes = vtkCollection::New();
    }
  else
    {
    this->SceneSnapshotNodes->RemoveAllItems();
    }
  vtkMRMLNode *node = NULL;
  int n;
  for (n=0; n < this->SceneSnapshotNodes->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->SceneSnapshotNodes->GetItemAsObject(n);
    if (node)
      {
      this->SceneSnapshotNodes->vtkCollection::AddItem((vtkObject *)node);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSnapshotClipNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//-----------------------------------------------------------
void vtkMRMLSnapshotClipNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateReferences();
  this->SceneSnapshotNodes->RemoveAllItems();

  for (unsigned int n=0; n<this->SceneSnapshotNodeIDs.size(); n++)
    {
    vtkMRMLSceneSnapshotNode *node = vtkMRMLSceneSnapshotNode::SafeDownCast(scene->GetNodeByID(this->SceneSnapshotNodeIDs[n]));
    this->SceneSnapshotNodes->AddItem(node);
    }
}
