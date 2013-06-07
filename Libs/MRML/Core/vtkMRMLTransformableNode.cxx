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
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>


//----------------------------------------------------------------------------
vtkMRMLTransformableNode::vtkMRMLTransformableNode()
{
  this->TransformNodeIDInternal = 0;
  this->TransformNodeReferenceRole = 0;
  this->TransformNodeReferenceRererenceMRMLAttributeName = 0;

  this->SetTransformNodeReferenceRole("transform");
  this->SetTransformNodeReferenceRererenceMRMLAttributeName("transformNodeRef");


  this->HideFromEditors = 0;
  this->AddNodeReferenceRole(this->GetTransformNodeReferenceRole(),
                             this->GetTransformNodeReferenceRererenceMRMLAttributeName());
}

//----------------------------------------------------------------------------
vtkMRMLTransformableNode::~vtkMRMLTransformableNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLTransformableNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
void vtkMRMLTransformableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  const char* transformNodeID = this->GetNodeReferenceID(this->GetTransformNodeReferenceRole());

  os << indent << "TransformNodeID: " <<
    (transformNodeID ? transformNodeID : "(none)") << "\n";
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformableNode::GetTransformNodeID()
{
  this->SetTransformNodeIDInternal(
    this->GetNodeReferenceID(this->GetTransformNodeReferenceRole()));

  return this->GetTransformNodeIDInternal();
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLTransformableNode::GetParentTransformNode()
{
  vtkMRMLTransformNode* node = NULL;
  const char* transformNodeID = this->GetNodeReferenceID(this->GetTransformNodeReferenceRole());

  if (this->GetScene() && transformNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(transformNodeID);
    node = vtkMRMLTransformNode::SafeDownCast(snode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformableNode::SetAndObserveTransformNodeID(const char *transformNodeID)
{
  if (transformNodeID == 0 && this->GetNodeReferenceID(this->GetTransformNodeReferenceRole()) == 0)
    {
    // was NULL and still NULL, nothing to do
    return;
    }

  if (transformNodeID != 0 && this->GetNodeReferenceID(this->GetTransformNodeReferenceRole()) != 0 &&
      !strcmp(transformNodeID, this->GetNodeReferenceID(this->GetTransformNodeReferenceRole())))
    {
    //the same ID, nothing to do
    return;
    }

  vtkMRMLTransformNode* tnode = vtkMRMLTransformNode::SafeDownCast(
    this->GetScene() != 0 ?this->GetScene()->GetNodeByID(transformNodeID) : 0);

  // Prevent transform cycles
  if (tnode && tnode->GetParentTransformNode() == this)
    {
    tnode->SetAndObserveTransformNodeID(0);
    }

  // use vtkMRMLNode call to set and observe reference
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  this->SetAndObserveNodeReferenceID(this->GetTransformNodeReferenceRole(), transformNodeID, events);
  events->Delete();

  this->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent, NULL);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *vtkNotUsed(callData) )
{
  // as retrieving the parent transform node can be costly (browse the scene)
  // do some checks here to prevent retrieving the node for nothing.
  if (caller == NULL ||
      (event != vtkCommand::ModifiedEvent && 
      event != vtkMRMLTransformableNode::TransformModifiedEvent))
    {
    return;
    }
  vtkMRMLTransformNode *tnode = this->GetParentTransformNode();
  if (tnode == caller)
    {
    //TODO don't send even on the scene but rather have vtkMRMLSliceLayerLogic listen to
    // TransformModifiedEvent
    //this->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    this->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent, NULL);
    }
}


//-----------------------------------------------------------
bool vtkMRMLTransformableNode::CanApplyNonLinearTransforms()const
{
  return false;
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  vtkMatrixToLinearTransform* transform = vtkMatrixToLinearTransform::New();
  transform->SetInput(transformMatrix);
  this->ApplyTransform(transform);
  transform->Delete();
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::ApplyTransform(vtkAbstractTransform* transform)
{
  vtkHomogeneousTransform* linearTransform = vtkHomogeneousTransform::SafeDownCast(transform);
  if (linearTransform)
    {
    this->ApplyTransformMatrix(linearTransform->GetMatrix());
    return;
    }
  if (!this->CanApplyNonLinearTransforms())
    {
    vtkErrorMacro("Can't apply a non-linear transform");
    return;
    }
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::TransformPointToWorld(const double in[4], double out[4])
{
  // get the nodes's transform node
  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
    transformToWorld->Identity();
    lnode->GetMatrixTransformToWorld(transformToWorld);
    transformToWorld->MultiplyPoint(in, out);
    transformToWorld->Delete();
    }
  else if (tnode == NULL)
    {
    for (int i=0; i<4; i++)
      {
      out[i] = in[i];
      }
    }
  else 
    {
    vtkErrorMacro("TransformPointToWorld: not a linear transform");
    }
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::TransformPointFromWorld(const double in[4], double out[4])
{
  // get the nodes's transform node
  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
    transformToWorld->Identity();
    lnode->GetMatrixTransformToWorld(transformToWorld);
    transformToWorld->Invert();
    transformToWorld->MultiplyPoint(in, out);
    transformToWorld->Delete();
    }
  else if (tnode == NULL)
    {
    for (int i=0; i<4; i++)
      {
      out[i] = in[i];
      }
    }
  else 
    {
    vtkErrorMacro("TransformPointToWorld: not a linear transform");
    }
}
