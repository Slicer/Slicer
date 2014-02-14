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
  this->TransformToParent = 0;
  this->TransformFromParent = 0;
  this->ReadWriteAsTransformToParent = 1;
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode::~vtkMRMLTransformNode()
{
  if (this->TransformToParent) 
    {
    this->TransformToParent->Delete();
    }
  if (this->TransformFromParent)
    {
    this->TransformFromParent->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::WriteXML(ostream& of, int nIndent)
{
  vtkIndent indent(nIndent);

  Superclass::WriteXML(of, nIndent);
  of << indent << " readWriteAsTransformToParent=\"" << (this->ReadWriteAsTransformToParent ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "readWriteAsTransformToParent"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ReadWriteAsTransformToParent = 1;
        }
      else
        {
        this->ReadWriteAsTransformToParent = 0;
        }
      }
    }
    this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTransformNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLTransformNode *node = (vtkMRMLTransformNode *) anode;

  this->SetReadWriteAsTransformToParent(node->GetReadWriteAsTransformToParent());

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "ReadWriteAsTransformToParent: " << this->ReadWriteAsTransformToParent << "\n";

}

//----------------------------------------------------------------------------
vtkGeneralTransform* vtkMRMLTransformNode::GetTransformToParent()
{
  if (this->TransformToParent == 0)
    {
    this->TransformToParent = vtkGeneralTransform::New();
    this->TransformToParent->Identity();
    if (this->TransformFromParent)
      {
      this->TransformToParent->DeepCopy(this->TransformFromParent);
      this->TransformToParent->Inverse();
      }
    }
  return this->TransformToParent;
}

//----------------------------------------------------------------------------
vtkGeneralTransform* vtkMRMLTransformNode::GetTransformFromParent()
{
  if (this->TransformFromParent == 0)
    {
    this->TransformFromParent = vtkGeneralTransform::New();
    this->TransformFromParent->Identity();
    if (this->TransformToParent)
      {
      this->TransformFromParent->DeepCopy(this->TransformToParent);
      this->TransformFromParent->Inverse();
      }
    }
  return this->TransformFromParent;
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

  transformToWorld->PostMultiply();
  transformToWorld->Concatenate(this->GetTransformToParent());

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL) 
    {
    parent->GetTransformToWorld(transformToWorld);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformFromWorld(vtkGeneralTransform* transformToWorld)
{
  if (transformToWorld->GetNumberOfConcatenatedTransforms() == 0)
    {
    transformToWorld->Identity();
    }

  transformToWorld->PreMultiply();
  transformToWorld->Concatenate(this->GetTransformFromParent());

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL)
    {
    parent->GetTransformFromWorld(transformToWorld);
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
