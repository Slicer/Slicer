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
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <string>
#include <iostream>
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode* vtkMRMLDisplayableHierarchyNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDisplayableHierarchyNode");
  if(ret)
    {
    return (vtkMRMLDisplayableHierarchyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDisplayableHierarchyNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDisplayableHierarchyNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDisplayableHierarchyNode");
  if(ret)
    {
    return (vtkMRMLDisplayableHierarchyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDisplayableHierarchyNode;
}


//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode::vtkMRMLDisplayableHierarchyNode()
{
  this->DisplayNodeID = NULL;
  this->DisplayNode = NULL;
  this->HideFromEditors = 1;
  this->Expanded = 1;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode::~vtkMRMLDisplayableHierarchyNode()
{
  this->SetAndObserveDisplayNodeID( NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->DisplayNodeID != NULL) 
    {
    of << indent << " displayNodeID=\"" << this->DisplayNodeID << "\"";
    }

  of << indent << " expanded=\"" << (this->Expanded ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->DisplayNodeID == NULL || !strcmp(oldID, this->DisplayNodeID))
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
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "displayableNodeID")) 
      {
      this->SetDisplayableNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->DisplayableNodeIDReference, this);
      }
    else if (!strcmp(attName, "displayNodeRef") ||
             !strcmp(attName, "displayNodeID")) 
      {
      this->SetDisplayNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->DisplayNodeID, this);
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

  vtkSmartPointer<vtkCollection> col =  vtkSmartPointer<vtkCollection>::New();
  this->GetChildrenDisplayableNodes(col);
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

  if (this->Scene == NULL)
    {
    return;
    }
  if (this->DisplayNodeID != NULL && this->Scene->GetNodeByID(this->DisplayNodeID) == NULL)
    {
    this->SetAndObserveDisplayNodeID(NULL);
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
  vtkMRMLDisplayNode* node = NULL;
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
  vtkSetAndObserveMRMLObjectMacro(this->DisplayNode, NULL);

  this->SetDisplayNodeID(displayNodeID);

  vtkMRMLDisplayNode *dnode = this->GetDisplayNode();

  vtkSetAndObserveMRMLObjectMacro(this->DisplayNode, dnode);

}


//---------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  return;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableHierarchyNode* vtkMRMLDisplayableHierarchyNode::GetUnExpandedParentNode()
{
  vtkMRMLDisplayableHierarchyNode *node = NULL;
  if (!this->GetExpanded()) 
    {
    node = this;
    }
  else 
    {
    vtkMRMLDisplayableHierarchyNode *parent = vtkMRMLDisplayableHierarchyNode::SafeDownCast(this->GetParentNode());
    if (parent)
      {
      node =  parent->GetUnExpandedParentNode();
      }
    else
      {
      node =  NULL;
      }
    }
  return node;
}


  
//---------------------------------------------------------------------------
void vtkMRMLDisplayableHierarchyNode::GetChildrenDisplayableNodes(vtkCollection *children)
{
  this->GetAssociateChildrendNodes(children, "vtkMRMLDisplayableNode");
}

//----------------------------------------------------------------------------

void vtkMRMLDisplayableHierarchyNode::RemoveChildrenNodes()
{
  if (this->GetScene() == NULL)
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
  if (this->GetScene() == NULL)
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

vtkMRMLDisplayableHierarchyNode* 
vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(vtkMRMLScene *scene,
                                                             const char *displayableNodeID)
{
  return vtkMRMLDisplayableHierarchyNode::SafeDownCast(
    vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene,displayableNodeID));
}
