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
  if (this->ParentTransformNodeID) 
    {
    delete [] this->ParentTransformNodeID;
    this->ParentTransformNodeID = NULL;
    }
  if (this->ParentTransformNode) 
    {
    this->ParentTransformNode->Delete();
    }
  if (this->TransformToParent) 
    {
    this->TransformToParent->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->ParentTransformNodeID != NULL) 
    {
    of << indent << "parentTransformNodeRef=/'" << this->ParentTransformNodeID << "/' ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "parentTransformNodeRef")) 
      {
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
  if (this->GetParentTransformNodeID() == NULL) 
    {
    return;
    }
  vtkMRMLNode* mnode = scene->GetNodeByID(this->ParentTransformNodeID);
  if (mnode) 
    {
    vtkMRMLTransformNode *node  = dynamic_cast < vtkMRMLTransformNode *>(mnode);
    this->SetParentTransformNode(node);
    }
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

  transformToWorld->Concatenate(this->TransformToParent);

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
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsTransformNodeMyParent(vtkMRMLTransformNode* node)
{
  vtkMRMLTransformNode *parent = this->ParentTransformNode;
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
  else return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsTransformNodeMyChild(vtkMRMLTransformNode* node)
{
  return node->IsTransformNodeMyParent(this);
}
// End
