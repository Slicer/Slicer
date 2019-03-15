/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLSnapshotClipNode.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSnapshotClipNode);

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

  vtkMRMLSceneViewNode * node = nullptr;
  std::stringstream ss;
  int n;
  for (n=0; n < this->SceneSnapshotNodes->GetNumberOfItems(); n++)
    {
    node = vtkMRMLSceneViewNode::SafeDownCast(this->SceneSnapshotNodes->GetItemAsObject(n));
    ss << node->GetID();
    if (n < this->SceneSnapshotNodes->GetNumberOfItems()-1)
      {
      ss << " ";
      }
    }
    of << " sceneSnapshotIDs=\"" << ss.str().c_str() << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSnapshotClipNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  this->SceneSnapshotNodeIDs.clear();

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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
  //vtkMRMLSnapshotClipNode *snode = (vtkMRMLSnapshotClipNode *) anode;

  if (this->SceneSnapshotNodes == nullptr)
    {
    this->SceneSnapshotNodes = vtkCollection::New();
    }
  else
    {
    this->SceneSnapshotNodes->RemoveAllItems();
    }
  vtkMRMLNode *node = nullptr;
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
    vtkMRMLSceneViewNode *node = vtkMRMLSceneViewNode::SafeDownCast(scene->GetNodeByID(this->SceneSnapshotNodeIDs[n]));
    this->SceneSnapshotNodes->AddItem(node);
    }
}

void vtkMRMLSnapshotClipNode::AddSceneSnapshotNode(vtkMRMLSceneViewNode * node)
{
  this->SceneSnapshotNodes->AddItem(node);
}

///
/// Get Numbre of SceneSnapshot nodes
int vtkMRMLSnapshotClipNode::GetNumberOfSceneSnapshotNodes()
{
  return this->SceneSnapshotNodes->GetNumberOfItems();
}

///
/// Get SceneSnapshot node
vtkMRMLSceneViewNode* vtkMRMLSnapshotClipNode::GetSceneSnapshotNode(int index)
{
  return vtkMRMLSceneViewNode::SafeDownCast(this->SceneSnapshotNodes->GetItemAsObject(index));
}
