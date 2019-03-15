/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDisplayableHierarchyNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayableHierarchyNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkMRMLDisplayableHierarchyNode, DisplayNodeID);

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLDisplayableHierarchyNode);

//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode::vtkMRMLDisplayableHierarchyNode()
{
  this->DisplayNodeID = nullptr;
  this->DisplayNode = nullptr;
  this->HideFromEditors = 1;
  this->Expanded = 1;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode::~vtkMRMLDisplayableHierarchyNode()
{
  this->SetAndObserveDisplayNodeID( nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  if (this->DisplayNodeID != nullptr)
    {
    of << " displayNodeID=\"" << this->DisplayNodeID << "\"";
    }

  of << " expanded=\"" << (this->Expanded ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->DisplayNodeID, this);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->DisplayNodeID == nullptr || !strcmp(oldID, this->DisplayNodeID))
    {
    this->SetDisplayNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "displayableNodeID"))
      {
      this->SetDisplayableNodeID(attValue);
      }
    else if (!strcmp(attName, "displayNodeRef") ||
             !strcmp(attName, "displayNodeID"))
      {
      this->SetDisplayNodeID(attValue);
      }
    else if (!strcmp(attName, "expanded"))
        {
        if (!strcmp(attValue,"true"))
          {
          this->Expanded = 1;
          }
        else
          {
          this->Expanded = 0;
          }
        }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDisplayableHierarchyNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLDisplayableHierarchyNode *node = (vtkMRMLDisplayableHierarchyNode *) anode;

  this->SetDisplayNodeID(node->DisplayNodeID);
  this->SetExpanded(node->Expanded);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "DisplayNodeID: " <<
    (this->DisplayNodeID ? this->DisplayNodeID : "(none)") << "\n";
  os << indent << "Expanded:        " << this->Expanded << "\n";

  vtkNew<vtkCollection> col;
  this->GetChildrenDisplayableNodes(col.GetPointer());
  unsigned int numChildren = col->GetNumberOfItems();
  os << indent << "Number of children displayable nodes = " << numChildren << "\n";
  for (unsigned int i = 0; i < numChildren; i++)
    {
    vtkMRMLDisplayableNode *child = vtkMRMLDisplayableNode::SafeDownCast(col->GetItemAsObject(i));
    if (child)
      {
      os << indent.GetNextIndent() << i << "th child id = " << (child->GetID() ? child->GetID() : "NULL") << "\n";
      }
    }
}

//-----------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  this->SetAndObserveDisplayNodeID(this->GetDisplayNodeID());

}

//-----------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->Scene == nullptr)
    {
    return;
    }
  if (this->DisplayNodeID != nullptr && this->Scene->GetNodeByID(this->DisplayNodeID) == nullptr)
    {
    this->SetAndObserveDisplayNodeID(nullptr);
    }
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode* vtkMRMLDisplayableHierarchyNode::GetDisplayableNode()
{
  vtkMRMLDisplayableNode* node = vtkMRMLDisplayableNode::SafeDownCast(this->GetAssociatedNode());
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLDisplayableHierarchyNode::GetDisplayNode()
{
  vtkMRMLDisplayNode* node = nullptr;
  if (this->GetScene() && this->GetDisplayNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
    node = vtkMRMLDisplayNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->DisplayNode, nullptr);

  this->SetDisplayNodeID(displayNodeID);

  vtkMRMLDisplayNode *dnode = this->GetDisplayNode();

  vtkSetAndObserveMRMLObjectMacro(this->DisplayNode, dnode);

  if (this->Scene)
    {
    this->Scene->AddReferencedNodeID(displayNodeID, this);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != nullptr && dnode == vtkMRMLDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent, nullptr);
    }
  return;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode* vtkMRMLDisplayableHierarchyNode::GetCollapsedParentNode()
{
  // initialize the return node to null, if there are no collapsed hierarchy
  // nodes, returns null
  vtkMRMLDisplayableHierarchyNode *node = nullptr;

  // build up a vector of collapsed parents
  std::vector< vtkMRMLDisplayableHierarchyNode * > collapsedParents;
  if (!this->GetExpanded())
    {
    collapsedParents.push_back(this);
    }
  vtkMRMLDisplayableHierarchyNode *parent = vtkMRMLDisplayableHierarchyNode::SafeDownCast(this->GetParentNode());
  while (parent)
    {
    if (!parent->GetExpanded())
      {
      collapsedParents.push_back(parent);
      }
    parent = vtkMRMLDisplayableHierarchyNode::SafeDownCast(parent->GetParentNode());
    }
  // return the last collapsed parent
  if (collapsedParents.size() != 0)
    {
    node = collapsedParents.back();
    }
  return node;
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::GetChildrenDisplayableNodes(vtkCollection *children)
{
  this->GetAssociatedChildrenNodes(children, "vtkMRMLDisplayableNode");
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::RemoveChildrenNodes()
{
  if (this->GetScene() == nullptr)
    {
    return;
    }

  std::vector< vtkMRMLHierarchyNode *> children = this->GetChildrenNodes();
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkMRMLDisplayableHierarchyNode *child = vtkMRMLDisplayableHierarchyNode::SafeDownCast(children[i]);
    if (child)
      {
      std::vector< vtkMRMLHierarchyNode *> childChildern = child->GetChildrenNodes();
      vtkMRMLDisplayableNode *dnode = child->GetDisplayableNode();
      if (dnode)
        {
        this->GetScene()->RemoveNode(dnode);
        }
      vtkMRMLDisplayNode *disnode = child->GetDisplayNode();
      if (disnode)
        {
        this->GetScene()->RemoveNode(disnode);
        }
      }
    }
  this->RemoveHierarchyChildrenNodes();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::RemoveAllChildrenNodes()
{
  if (this->GetScene() == nullptr)
    {
    return;
    }

  std::vector< vtkMRMLHierarchyNode *> children = this->GetChildrenNodes();
  for (unsigned int i=0; i<children.size(); i++)
    {
    vtkMRMLDisplayableHierarchyNode *child = vtkMRMLDisplayableHierarchyNode::SafeDownCast(children[i]);
    if (child)
      {
      child->RemoveAllChildrenNodes();

      std::vector< vtkMRMLHierarchyNode *> childChildern = child->GetChildrenNodes();
      vtkMRMLDisplayableNode *dnode = child->GetDisplayableNode();
      if (dnode)
        {
        this->GetScene()->RemoveNode(dnode);
        }
      vtkMRMLDisplayNode *disnode = child->GetDisplayNode();
      if (disnode)
        {
        this->GetScene()->RemoveNode(disnode);
        }
      }
    }
  this->RemoveAllHierarchyChildrenNodes();

}

//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode*
vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(vtkMRMLScene *scene,
                                                             const char *displayableNodeID)
{
  return vtkMRMLDisplayableHierarchyNode::SafeDownCast(
    vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene,displayableNodeID));
}
