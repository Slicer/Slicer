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
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"

// VTK includes
#include <vtkGeneralTransform.h>

//----------------------------------------------------------------------------
vtkMRMLTransformNode::vtkMRMLTransformNode()
{
  this->TransformToParent = vtkGeneralTransform::New();
  this->TransformToParent->Identity();
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode::~vtkMRMLTransformNode()
{
  if (this->TransformToParent) 
    {
    this->TransformToParent->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTransformNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::IsTransformToWorldLinear()
{
  if (this->IsLinear() == 0) 
    {
    return 0;
    }
  else 
    {
    vtkMRMLTransformNode *parent = this->GetParentTransformNode();
    if (parent != NULL) 
      {
      return parent->IsTransformToWorldLinear();
      }
    else 
      {
      return 1;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformToWorld(vtkGeneralTransform* transformToWorld)
{
  if (transformToWorld->GetNumberOfConcatenatedTransforms() == 0) 
    {
    transformToWorld->Identity();
    }

  transformToWorld->Concatenate(this->GetTransformToParent());

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL) 
    {
    parent->GetTransformToWorld(transformToWorld);
    }
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::IsTransformToNodeLinear(vtkMRMLTransformNode* node)
{
  if (this->IsTransformNodeMyParent(node)) 
    {
    vtkMRMLTransformNode *parent = this->GetParentTransformNode();
    if (parent != NULL) 
      {
      if (!strcmp(parent->GetID(), node->GetID()) ) 
        {
        return this->IsLinear();
        }
      else 
        {
        return this->IsLinear() * parent->IsTransformToNodeLinear(node);
        }
      }
    else return this->IsLinear();
    }
  else if (this->IsTransformNodeMyChild(node)) 
    {
    vtkMRMLTransformNode *parent = node->GetParentTransformNode();
    if (parent != NULL) 
      {
      if (!strcmp(parent->GetID(), this->GetID()) ) 
        {
        return node->IsLinear();
        }
      else 
        {
        return node->IsLinear() * parent->IsTransformToNodeLinear(this);
        }
      }
    else return node->IsLinear();
    }
  else if (this->IsTransformToWorldLinear() == 1 && 
           node->IsTransformToWorldLinear() == 1) 
    {
    return 1;
    }
  else 
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void  vtkMRMLTransformNode::GetTransformToNode(vtkMRMLTransformNode* node, 
                                               vtkGeneralTransform* transformToNode)
{
  if (this->IsTransformNodeMyParent(node)) 
    {
    vtkMRMLTransformNode *parent = this->GetParentTransformNode();
    if (parent != NULL) 
      {
      transformToNode->Concatenate(this->GetTransformToParent());
      if (strcmp(parent->GetID(), node->GetID()) ) 
        {
        this->GetTransformToNode(node, transformToNode);
        }
      }
    else if (this->GetTransformToParent()) 
      {
      transformToNode->Concatenate(this->GetTransformToParent());
      }
    }
  else if (this->IsTransformNodeMyChild(node)) 
    {
    vtkMRMLTransformNode *parent = node->GetParentTransformNode();
    if (parent != NULL) 
      {
      transformToNode->Concatenate(node->GetTransformToParent());
      if (strcmp(parent->GetID(), this->GetID()) ) 
        {
        node->GetTransformToNode(this, transformToNode);
        }
      }
    else if (node->GetTransformToParent()) 
      {
      transformToNode->Concatenate(node->GetTransformToParent());
      }
    }
  else 
    {
    this->GetTransformToWorld(transformToNode);
    vtkGeneralTransform* transformToWorld2 = vtkGeneralTransform::New();
    transformToWorld2->Identity();
    
    node->GetTransformToWorld(transformToWorld2);
    transformToWorld2->Inverse();
    
    transformToNode->Concatenate(transformToWorld2);
    transformToWorld2->Delete();
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsTransformNodeMyParent(vtkMRMLTransformNode* node)
{
  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL) 
    {
    if (!strcmp(parent->GetID(), node->GetID()) ) 
      {
      return 1;
      }
    else 
      {
      return parent->IsTransformNodeMyParent(node);
      }
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsTransformNodeMyChild(vtkMRMLTransformNode* node)
{
  return node->IsTransformNodeMyParent(this);
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ApplyTransform(vtkAbstractTransform* transform)
{
  this->TransformToParent->Concatenate(transform); 
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLTransformNode::CreateDefaultStorageNode()
{
  return vtkMRMLTransformStorageNode::New();
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->TransformToParent &&
     this->TransformToParent->GetMTime() > this->GetStoredTime());
}
