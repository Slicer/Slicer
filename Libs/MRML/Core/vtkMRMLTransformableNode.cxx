/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include "vtkMRMLTransformableNode.h"

// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

const char* vtkMRMLTransformableNode::TransformNodeReferenceRole = "transform";
const char* vtkMRMLTransformableNode::TransformNodeReferenceMRMLAttributeName = "transformNodeRef";

//----------------------------------------------------------------------------
vtkMRMLTransformableNode::vtkMRMLTransformableNode()
{
  this->TransformNodeIDInternal = nullptr;

  this->HideFromEditors = 0;

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  this->AddNodeReferenceRole(this->GetTransformNodeReferenceRole(),
                             this->GetTransformNodeReferenceMRMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLTransformableNode::~vtkMRMLTransformableNode() = default;

//----------------------------------------------------------------------------
const char* vtkMRMLTransformableNode::GetTransformNodeReferenceRole()
{
  return vtkMRMLTransformableNode::TransformNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformableNode::GetTransformNodeReferenceMRMLAttributeName()
{
  return vtkMRMLTransformableNode::TransformNodeReferenceMRMLAttributeName;
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
  return vtkMRMLTransformNode::SafeDownCast(
        this->GetNodeReference(this->GetTransformNodeReferenceRole()));
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformableNode::SetAndObserveTransformNodeID(const char *transformNodeID)
{
  // Prevent circular reference in transform tree
  vtkMRMLTransformNode* newParentTransformNode = vtkMRMLTransformNode::SafeDownCast(
    this->GetScene() != nullptr ? this->GetScene()->GetNodeByID(transformNodeID) : nullptr);
  if (newParentTransformNode)
    {
    vtkMRMLTransformNode* thisTransform = vtkMRMLTransformNode::SafeDownCast(this);
    if (thisTransform)
      {
      if (newParentTransformNode == thisTransform || thisTransform->IsTransformNodeMyChild(newParentTransformNode))
        {
        vtkErrorMacro("vtkMRMLTransformableNode::SetAndObserveTransformNodeID failed: parent transform cannot be self or a child transform");
        return false;
        }
      }
    }

  this->SetAndObserveNodeReferenceID(this->GetTransformNodeReferenceRole(), transformNodeID);
  return true;
}


//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event,
                                                  void *vtkNotUsed(callData) )
{
  // as retrieving the parent transform node can be costly (browse the scene)
  // do some checks here to prevent retrieving the node for nothing.
  if (caller == nullptr ||
      (event != vtkCommand::ModifiedEvent &&
      event != vtkMRMLTransformableNode::TransformModifiedEvent))
    {
    return;
    }
  vtkMRMLTransformNode *tnode = this->GetParentTransformNode();
  if (tnode == caller)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLTransformableNode::TransformModifiedEvent, nullptr);
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
  vtkNew<vtkTransform> transform;
  transform->SetMatrix(transformMatrix);
  this->ApplyTransform(transform.GetPointer());
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform))
{
  vtkErrorMacro("ApplyTransform is not implemented for node type "<<this->GetClassName());
}

//-----------------------------------------------------------
bool vtkMRMLTransformableNode::HardenTransform()
{
  vtkMRMLTransformNode* transformNode = this->GetParentTransformNode();
  if (!transformNode)
    {
    // already in the world coordinate system
    return true;
    }
  if (transformNode->IsTransformToWorldLinear())
    {
    vtkNew<vtkMatrix4x4> hardeningMatrix;
    transformNode->GetMatrixTransformToWorld(hardeningMatrix.GetPointer());
    this->ApplyTransformMatrix(hardeningMatrix.GetPointer());
    }
  else
    {
    vtkNew<vtkGeneralTransform> hardeningTransform;
    transformNode->GetTransformToWorld(hardeningTransform.GetPointer());
    this->ApplyTransform(hardeningTransform.GetPointer());
    }

  this->SetAndObserveTransformNodeID(nullptr);
  return true;
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::TransformPointToWorld(const double inLocal[3], double outWorld[3])
{
  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode == nullptr)
    {
    // not transformed
    outWorld[0] = inLocal[0];
    outWorld[1] = inLocal[1];
    outWorld[2] = inLocal[2];
    return;
    }

  // Get transform
  vtkNew<vtkGeneralTransform> transformToWorld;
  tnode->GetTransformToWorld(transformToWorld.GetPointer());

  // Convert coordinates
  transformToWorld->TransformPoint(inLocal, outWorld);
}

//-----------------------------------------------------------
void vtkMRMLTransformableNode::TransformPointFromWorld(const double inWorld[3], double outLocal[3])
{
  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode == nullptr)
    {
    // not transformed
    outLocal[0] = inWorld[0];
    outLocal[1] = inWorld[1];
    outLocal[2] = inWorld[2];
    return;
    }

  // Get transform
  vtkNew<vtkGeneralTransform> transformFromWorld;
  tnode->GetTransformFromWorld(transformFromWorld.GetPointer());

  // Convert coordinates
  transformFromWorld->TransformPoint(inWorld, outLocal);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::TransformPointToWorld(const vtkVector3d &inLocal, vtkVector3d &outWorld)
{
  this->TransformPointToWorld(inLocal.GetData(),outWorld.GetData());
}

//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::TransformPointFromWorld(const vtkVector3d &inWorld, vtkVector3d &outLocal)
{
  this->TransformPointFromWorld(inWorld.GetData(),outLocal.GetData());
}

//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
{
  Superclass::OnNodeReferenceAdded(reference);
  if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
    {
    this->OnTransformNodeReferenceChanged(vtkMRMLTransformNode::SafeDownCast(reference->GetReferencedNode()));
    }
}

//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::OnNodeReferenceModified(vtkMRMLNodeReference *reference)
{
  Superclass::OnNodeReferenceModified(reference);
  if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
    {
    this->OnTransformNodeReferenceChanged(vtkMRMLTransformNode::SafeDownCast(reference->GetReferencedNode()));
    }
}

//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::OnNodeReferenceRemoved(vtkMRMLNodeReference *reference)
{
  Superclass::OnNodeReferenceRemoved(reference);
  if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
    {
    this->OnTransformNodeReferenceChanged(vtkMRMLTransformNode::SafeDownCast(reference->GetReferencedNode()));
    }
}

//---------------------------------------------------------------------------
void vtkMRMLTransformableNode::OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode)
{
  this->InvokeCustomModifiedEvent(vtkMRMLTransformableNode::TransformModifiedEvent, transformNode);
}
