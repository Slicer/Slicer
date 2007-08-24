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
#include "vtkIntArray.h"

#include "vtkMRMLHierarchyNode.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLScene.h"


//----------------------------------------------------------------------------
vtkMRMLHierarchyNode::vtkMRMLHierarchyNode()
{
  this->HideFromEditors = 0;

  this->ParentNodeID = NULL;

}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode::~vtkMRMLHierarchyNode()
{
  if (this->ParentNodeID) 
    {
    delete [] this->ParentNodeID;
    this->ParentNodeID = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->ParentNodeID != NULL) 
    {
    of << indent << " parentNodeRef=\"" << this->ParentNodeID << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->ParentNodeID && !strcmp(oldID, this->ParentNodeID))
    {
    this->SetParentNodeID(ParentNodeID);
    }
}
//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "parentNodeRef")) 
      {
      this->SetParentNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ParentNodeID, this);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLHierarchyNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLHierarchyNode *node = (vtkMRMLHierarchyNode *) anode;
  this->SetParentNodeID(node->ParentNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "ParentNodeID: " <<
    (this->ParentNodeID ? this->ParentNodeID : "(none)") << "\n";
}

//----------------------------------------------------------------------------
vtkMRMLHierarchyNode* vtkMRMLHierarchyNode::GetParentNode()
{
  vtkMRMLHierarchyNode* node = NULL;
  if (this->GetScene() && this->ParentNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ParentNodeID);
    node = vtkMRMLHierarchyNode::SafeDownCast(snode);
    }
  return node;
}

//-----------------------------------------------------------
void vtkMRMLHierarchyNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkMRMLHierarchyNode::UpdateReferences()
{
  Superclass::UpdateReferences();
  
  if (this->ParentNodeID != NULL && this->Scene->GetNodeByID(this->ParentNodeID) == NULL)
    {
    this->SetParentNodeID(NULL);
    }
}
// End
