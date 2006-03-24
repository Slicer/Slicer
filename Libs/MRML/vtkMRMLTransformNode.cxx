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
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLScene.h"


//----------------------------------------------------------------------------
vtkMRMLTransformNode::vtkMRMLTransformNode()
{
  this->ParentTransformNodeID = NULL;
  this->ParentTransformNode = NULL;
  this->TransformToParent = vtkGeneralTransform::New();
  this->TransformToParent->Identity();
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode::~vtkMRMLTransformNode()
{
  if (this->ParentTransformNodeID) {
    delete [] this->ParentTransformNodeID;
    this->ParentTransformNodeID = NULL;
  }
  if (this->ParentTransformNode) {
    this->ParentTransformNode->Delete();
  }
  if (this->TransformToParent) {
    this->TransformToParent->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->ParentTransformNodeID != NULL) {
    of << indent << "ParentTransformNodeID='" << this->ParentTransformNodeID << "' ";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "ParentTransformNodeID")) {
      this->SetParentTransformNodeID(attValue);
    }
  }  
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTransformNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTransformNode *node = (vtkMRMLTransformNode *) anode;
  this->SetParentTransformNodeID(node->ParentTransformNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "ParentTransformNodeID: " <<
    (this->ParentTransformNodeID ? this->ParentTransformNodeID : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::UpdateScene(vtkMRMLScene *scene)
{
  if (this->GetParentTransformNodeID() == NULL) {
    return;
  }
  vtkMRMLNode* mnode = scene->GetNodeByID(this->ParentTransformNodeID);
  if (mnode) {
    vtkMRMLTransformNode *node  = dynamic_cast < vtkMRMLTransformNode *>(mnode);
    this->SetParentTransformNode(node);
  }
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::IsTransformToWorldLinear()
{
  if (this->IsLinear() == 0) {
    return 0;
  }
  else {
    vtkMRMLTransformNode *parent = this->GetParentTransformNode();
    if (parent != NULL) {
      return parent->IsTransformToWorldLinear();
    }
    else {
      return 1;
    }
  }

}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformToWorld(vtkGeneralTransform* transformToWorld)
{
  if (transformToWorld->GetNumberOfConcatenatedTransforms() == 0) {
    transformToWorld->Identity();
  }

  transformToWorld->Concatenate(this->TransformToParent);

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL) {
    parent->GetTransformToWorld(transformToWorld);
  }
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::IsTransformToNodeLinear(vtkMRMLTransformNode* node)
{
  if (this->IsTransformToWorldLinear() == 1 && 
      node->IsTransformToWorldLinear() == 1) {
    return 1;
  }
  else {
    return 0;
  }
}

//----------------------------------------------------------------------------
void  vtkMRMLTransformNode::GetTransformToNode(vtkMRMLTransformNode* node, 
                                               vtkGeneralTransform* transformToNode)
{

  this->GetTransformToWorld(transformToNode);
  vtkGeneralTransform* transformToWorld2 = vtkGeneralTransform::New();
  transformToWorld2->Identity();
  
  node->GetTransformToWorld(transformToWorld2);
  transformToWorld2->Inverse();

  transformToNode->Concatenate(transformToWorld2);
}

// End
