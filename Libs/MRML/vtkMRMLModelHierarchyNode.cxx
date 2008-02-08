/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLModelHierarchyNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkMRMLModelHierarchyNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelHierarchyNode");
  if(ret)
    {
    return (vtkMRMLModelHierarchyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelHierarchyNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLModelHierarchyNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelHierarchyNode");
  if(ret)
    {
    return (vtkMRMLModelHierarchyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelHierarchyNode;
}


//----------------------------------------------------------------------------
vtkMRMLModelHierarchyNode::vtkMRMLModelHierarchyNode()
{
  this->ModelNodeID = NULL;
  this->DisplayNodeID = NULL;
  this->ModelDisplayNode = NULL;
  this->HideFromEditors = 1;
  this->Expanded = 1;
}

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyNode::~vtkMRMLModelHierarchyNode()
{
  if (this->ModelNodeID) 
    {
    delete [] this->ModelNodeID;
    this->ModelNodeID = NULL;
    }
  this->SetAndObserveDisplayNodeID( NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

   if (this->ModelNodeID != NULL) 
    {
    of << indent << " modelNodeRef=\"" << this->ModelNodeID << "\"";
    }
  if (this->DisplayNodeID != NULL) 
    {
    of << indent << " displayNodeRef=\"" << this->DisplayNodeID << "\"";
    }

  of << indent << " expanded=\"" << (this->Expanded ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->ModelNodeID == NULL || !strcmp(oldID, this->ModelNodeID))
    {
    this->SetModelNodeID(newID);
    }
  if (this->DisplayNodeID == NULL || !strcmp(oldID, this->DisplayNodeID))
    {
    this->SetDisplayNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "modelNodeRef")) 
      {
      this->SetModelNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ModelNodeID, this);
      }
    else if (!strcmp(attName, "displayNodeRef")) 
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
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLModelHierarchyNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLModelHierarchyNode *node = (vtkMRMLModelHierarchyNode *) anode;

  this->SetModelNodeID(node->ModelNodeID);
  this->SetDisplayNodeID(node->DisplayNodeID);
  this->SetExpanded(node->Expanded);

}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "ModelNodeID: " <<
    (this->ModelNodeID ? this->ModelNodeID : "(none)") << "\n";

  os << indent << "DisplayNodeID: " <<
    (this->DisplayNodeID ? this->DisplayNodeID : "(none)") << "\n";
  os << indent << "Expanded:        " << this->Expanded << "\n";
}

//-----------------------------------------------------------
void vtkMRMLModelHierarchyNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  this->SetAndObserveDisplayNodeID(this->GetDisplayNodeID());

}

//-----------------------------------------------------------
void vtkMRMLModelHierarchyNode::UpdateReferences()
{
  Superclass::UpdateReferences();
  
  if (this->DisplayNodeID != NULL && this->Scene->GetNodeByID(this->DisplayNodeID) == NULL)
    {
    this->SetAndObserveDisplayNodeID(NULL);
    }
  if (this->ModelNodeID != NULL && this->Scene->GetNodeByID(this->ModelNodeID) == NULL)
    {
    this->SetModelNodeID(NULL);
    }
}

vtkMRMLModelNode* vtkMRMLModelHierarchyNode::GetModelNode()
{
  vtkMRMLModelNode* node = NULL;
  if (this->GetScene() && this->GetModelNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ModelNodeID);
    node = vtkMRMLModelNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLModelDisplayNode* vtkMRMLModelHierarchyNode::GetDisplayNode()
{
  vtkMRMLModelDisplayNode* node = NULL;
  if (this->GetScene() && this->GetDisplayNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->DisplayNodeID);
    node = vtkMRMLModelDisplayNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::SetAndObserveDisplayNodeID(const char *displayNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->ModelDisplayNode, NULL);

  this->SetDisplayNodeID(displayNodeID);

  vtkMRMLModelDisplayNode *dnode = this->GetDisplayNode();

  vtkSetAndObserveMRMLObjectMacro(this->ModelDisplayNode, dnode);

}


//---------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLModelDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLModelDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  return;
}

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkMRMLModelHierarchyNode::GetUnExpandedParentNode()
{
  vtkMRMLModelHierarchyNode *node = NULL;
  if (!this->GetExpanded()) 
    {
    node = this;
    }
  else 
    {
    vtkMRMLModelHierarchyNode *parent = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetParentNode());
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

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkMRMLModelHierarchyNode::GetTopParentNode()
{
  vtkMRMLModelHierarchyNode *node = NULL;
  vtkMRMLModelHierarchyNode *parent = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetParentNode());
  if (parent == NULL) 
    {
    node = this;
    }
  else 
    {
    node =  parent->GetTopParentNode();
    }
  return node;
}


  
//---------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode:: GetChildrenModelNodes(vtkCollection *models)
{
  if (models == NULL)
    {
    return;
    }
  vtkMRMLScene *scene = this->GetScene();
  vtkMRMLNode *mnode = NULL;
  vtkMRMLModelHierarchyNode *hnode = NULL;
  for (int n=0; n < scene->GetNumberOfNodes(); n++) 
    {
    mnode = scene->GetNthNode(n);
    if (mnode->IsA("vtkMRMLModelNode"))
      {
      hnode = vtkMRMLModelHierarchyNode::GetModelHierarchyNode(scene, mnode->GetID());
      while (hnode)
        {
        if (hnode == this) 
          {
          models->AddItem(mnode);
          break;
          }
          hnode = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetParentNode());
        }// end while
      }// end if
    }// end for
}


//---------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkMRMLModelHierarchyNode::GetModelHierarchyNode(vtkMRMLScene *scene,
                                                                            const char *modelNodeID)
{
  if (modelNodeID == NULL)
    {
    return NULL;
    }

  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");
  for (int i=0; i<nnodes; i++)
    {
    vtkMRMLModelHierarchyNode *node =  vtkMRMLModelHierarchyNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLModelHierarchyNode"));
    if (node)
      {
      vtkMRMLModelNode *mnode = node->GetModelNode();
      if (mnode && !strcmp(mnode->GetID(), modelNodeID))
        {
        return node;
        }
      }
    }
  return NULL;
}
