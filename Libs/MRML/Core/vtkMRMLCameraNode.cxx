/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCameraNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>
#include <sstream>

vtkCxxSetObjectMacro(vtkMRMLCameraNode, Camera, vtkCamera);
vtkCxxSetObjectMacro(vtkMRMLCameraNode, AppliedTransform, vtkMatrix4x4);
vtkCxxSetReferenceStringMacro(vtkMRMLCameraNode, InternalActiveTag);

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCameraNode);

//----------------------------------------------------------------------------
vtkMRMLCameraNode::vtkMRMLCameraNode()
{
  this->HideFromEditors = 0;

  vtkNew<vtkCamera> camera;
  camera->SetPosition(0, 500, 0);
  camera->SetFocalPoint(0, 0, 0);
  camera->SetViewUp(0, 0, 1);
  this->SetAndObserveCamera(camera.GetPointer());

  this->AppliedTransform = vtkMatrix4x4::New();

  this->Interacting = 0;
  this->InteractionFlags = 0;
 }

//----------------------------------------------------------------------------
vtkMRMLCameraNode::~vtkMRMLCameraNode()
{
  this->SetAndObserveCamera(nullptr);
  delete [] this->InternalActiveTag;

  if (this->AppliedTransform)
    {
    this->AppliedTransform->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLVectorMacro(position, Position, double, 3);
  vtkMRMLWriteXMLVectorMacro(focalPoint, FocalPoint, double, 3);
  vtkMRMLWriteXMLVectorMacro(viewUp, ViewUp, double, 3);
  vtkMRMLWriteXMLBooleanMacro(parallelProjection, ParallelProjection);
  vtkMRMLWriteXMLFloatMacro(parallelScale, ParallelScale);
  vtkMRMLWriteXMLFloatMacro(viewAngle, ViewAngle);
  vtkMRMLWriteXMLStringMacro(activetag, ActiveTag);
  vtkMRMLWriteXMLEndMacro();

  if (this->GetAppliedTransform())
    {
    std::stringstream ss;
    for (int row=0; row<4; row++)
      {
      for (int col=0; col<4; col++)
        {
        ss << this->AppliedTransform->GetElement(row, col);
        if (!(row==3 && col==3))
          {
          ss << " ";
          }
        }
      if ( row != 3 )
        {
        ss << " ";
        }
      }
    of << " appliedTransform=\"" << ss.str() << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLVectorMacro(position, Position, double, 3);
  vtkMRMLReadXMLVectorMacro(focalPoint, FocalPoint, double, 3);
  vtkMRMLReadXMLVectorMacro(viewUp, ViewUp, double, 3);
  vtkMRMLReadXMLBooleanMacro(parallelProjection, ParallelProjection);
  vtkMRMLReadXMLFloatMacro(parallelScale, ParallelScale);
  vtkMRMLReadXMLFloatMacro(viewAngle, ViewAngle);
  vtkMRMLReadXMLStringMacro(activetag, ActiveTag);
  vtkMRMLReadXMLEndMacro();

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "active"))
      {
      // Legacy, was replaced by active tag, try to set ActiveTag instead
      // to link to the main viewer
      if (!this->GetActiveTag() && this->Scene)
        {
        vtkMRMLViewNode* vnode = vtkMRMLViewNode::SafeDownCast(
          this->Scene->GetFirstNodeByClass("vtkMRMLViewNode"));
        if (vnode)
        {
          this->SetActiveTag(vnode->GetID());
        }
        }
      }
    else if (!strcmp(attName, "appliedTransform"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for (int row=0; row<4; row++)
        {
        for (int col=0; col<4; col++)
          {
          ss >> val;
          this->GetAppliedTransform()->SetElement(row, col, val);
          }
        }
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLCameraNode::Copy(vtkMRMLNode* anode)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::Copy(anode);

  vtkMRMLCameraNode* node = vtkMRMLCameraNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  // Important, do not call SetActiveTag() or the owner of the current tag
  // (node) will lose its tag, and the active camera will be untagged, and
  // a the active camera of the current view will be reset to nullptr, and a
  // new camera will be created on the fly by VTK the next time an active
  // camera is need, one completely disconnected from Slicer3's MRML/internals
  this->SetInternalActiveTag(node->GetActiveTag());
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyVectorMacro(Position, double, 3);
  vtkMRMLCopyVectorMacro(FocalPoint, double, 3);
  vtkMRMLCopyVectorMacro(ViewUp, double, 3);
  vtkMRMLCopyBooleanMacro(ParallelProjection);
  vtkMRMLCopyFloatMacro(ParallelScale);
  vtkMRMLCopyFloatMacro(ViewAngle);
  vtkMRMLCopyEndMacro();

  vtkMRMLCameraNode* node = vtkMRMLCameraNode::SafeDownCast(anode);
  if (node)
    {
    this->AppliedTransform->DeepCopy(node->GetAppliedTransform());
    }

  // Maybe the new position and focalpoint combo doesn't fit the existing
  // clipping range
  this->ResetClippingRange();
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintVectorMacro(Position, double, 3);
  vtkMRMLPrintVectorMacro(FocalPoint, double, 3);
  vtkMRMLPrintVectorMacro(ViewUp, double, 3);
  vtkMRMLPrintBooleanMacro(ParallelProjection);
  vtkMRMLPrintFloatMacro(ParallelScale);
  vtkMRMLPrintFloatMacro(ViewAngle);
  vtkMRMLPrintStringMacro(ActiveTag);
  vtkMRMLPrintIntMacro(Interacting);
  vtkMRMLPrintEndMacro();

  os << indent << "AppliedTransform: " ;
  this->GetAppliedTransform()->PrintSelf(os, indent.GetNextIndent());
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::SetAndObserveCamera(vtkCamera* camera)
{
  if (this->Camera != nullptr)
    {
    this->SetCamera(nullptr);
    }
  this->SetCamera(camera);
  if ( this->Camera )
    {
    vtkEventBroker::GetInstance()->AddObservation (
      this->Camera, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetParallelProjection(int parallelProjection)
{
  this->Camera->SetParallelProjection(parallelProjection);
}

//---------------------------------------------------------------------------
int vtkMRMLCameraNode::GetParallelProjection()
{
  return this->Camera->GetParallelProjection();
};

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetParallelScale(double scale)
{
  this->Camera->SetParallelScale(scale);
}

//---------------------------------------------------------------------------
double vtkMRMLCameraNode::GetParallelScale()
{
  return this->Camera->GetParallelScale();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetPosition(double position[3])
{
  this->Camera->SetPosition(position);
}

//---------------------------------------------------------------------------
double* vtkMRMLCameraNode::GetPosition()
{
  return this->Camera->GetPosition();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::GetPosition(double position[3])
{
  this->Camera->GetPosition(position);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetFocalPoint(double focalPoint[3])
{
  this->Camera->SetFocalPoint(focalPoint);
}

//---------------------------------------------------------------------------
double* vtkMRMLCameraNode::GetFocalPoint()
{
  return this->Camera->GetFocalPoint();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::GetFocalPoint(double focalPoint[3])
{
  this->Camera->GetFocalPoint(focalPoint);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetViewUp(double viewUp[3])
{
  this->Camera->SetViewUp(viewUp);
}

//---------------------------------------------------------------------------
double* vtkMRMLCameraNode::GetViewUp()
{
  return this->Camera->GetViewUp();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::GetViewUp(double viewUp[3])
{
  this->Camera->GetViewUp(viewUp);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetViewAngle(double viewAngle)
{
  this->Camera->SetViewAngle(viewAngle);
}

//---------------------------------------------------------------------------
double vtkMRMLCameraNode::GetViewAngle()
{
  return this->Camera->GetViewAngle();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::ProcessMRMLEvents ( vtkObject* caller,
                                            unsigned long event,
                                            void* callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->Camera != nullptr &&
      this->Camera == vtkCamera::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent)
    {
    this->Modified();
    }

  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  if (this->Camera != nullptr &&
      tnode == vtkMRMLTransformNode::SafeDownCast(caller) &&
      event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {

    /*
     * calculate the delta transform Td, which is the incremental transform
     * that has not yet been applied to the current camera parameters.
     *
     * We started with Po (original parameter)
     * We have Pa = param with Ta (AppliedTransform applied)
     * we want Pn = param with new transform applied
     * Since Pn = Tn * Po
     * and Tn = Td * Ta
     * then
     * Td = Ta-1 * Tn
     * and
     * Pn = Td * Pa
     * then we save Tn as Ta for next time
     */
    vtkNew<vtkMatrix4x4> deltaTransform;
    vtkNew<vtkMatrix4x4> transformToWorld;
    transformToWorld->Identity();
    tnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());

    this->AppliedTransform->Invert();
    vtkMatrix4x4::Multiply4x4(transformToWorld.GetPointer(), this->AppliedTransform, deltaTransform.GetPointer());

    // transform the points and the vector through delta and store back to camera
    double v[4];
    // position is point - include translation with 1 in homogeneous coordinate
    v[0] = this->Camera->GetPosition()[0];
    v[1] = this->Camera->GetPosition()[1];
    v[2] = this->Camera->GetPosition()[2];
    v[3] = 1;
    deltaTransform->MultiplyPoint(v,v);
    this->Camera->SetPosition(v[0],v[1],v[2]);
    // focal point is point - include translation with 1 in homogeneous coordinate
    v[0] = this->Camera->GetFocalPoint()[0];
    v[1] = this->Camera->GetFocalPoint()[1];
    v[2] = this->Camera->GetFocalPoint()[2];
    v[3] = 1;
    deltaTransform->MultiplyPoint(v,v);
    this->Camera->SetFocalPoint(v[0],v[1],v[2]);
    // view up is vector - exclude translation with 0 in homogeneous coordinate
    v[0] = this->Camera->GetViewUp()[0];
    v[1] = this->Camera->GetViewUp()[1];
    v[2] = this->Camera->GetViewUp()[2];
    v[3] = 0;
    deltaTransform->MultiplyPoint(v,v);
    this->Camera->SetViewUp(v[0],v[1],v[2]);

    this->GetAppliedTransform()->DeepCopy(transformToWorld.GetPointer());
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  this->Scene->AddReferencedNodeID(this->GetActiveTag(), this);
}

//-----------------------------------------------------------
void vtkMRMLCameraNode::UpdateReferences()
{
  this->Superclass::UpdateReferences();

  if (this->GetActiveTag() != nullptr &&
      this->Scene->GetNodeByID(this->GetActiveTag()) == nullptr)
    {
    this->SetActiveTag(nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::UpdateReferenceID(const char* oldID, const char* newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->GetActiveTag() && !strcmp(oldID, this->GetActiveTag()))
    {
    this->SetActiveTag(newID);
    }
}

//---------------------------------------------------------------------------
const char* vtkMRMLCameraNode::GetActiveTag()
{
  return this->InternalActiveTag;
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetActiveTag(const char* _arg)
{
  if (this->GetActiveTag() == nullptr && _arg == nullptr)
    {
    return;
    }

  if (this->GetActiveTag() && _arg &&
      (!strcmp(this->GetActiveTag(), _arg)))
    {
    return;
    }
  // set this node's active tag first, then loop through and unset anyone
  // else's
  // do this first because the viewer widget will get an event when we set
  // other node's active tags to null and it will regrab an unassigned camera
  // node so as not to be without one.
  this->SetInternalActiveTag(_arg);

  // If any camera is already using that new tag, let's find them and set
  // their tags to null
  if (this->Scene != nullptr && _arg != nullptr)
    {
    vtkMRMLCameraNode* node = nullptr;
    int nnodes = this->Scene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
    for (int n=0; n<nnodes; n++)
      {
      node = vtkMRMLCameraNode::SafeDownCast (
                this->Scene->GetNthNodeByClass(n, "vtkMRMLCameraNode"));
      if (node &&
          node != this &&
          node->GetActiveTag() &&
          !strcmp(node->GetActiveTag(), _arg))
        {
        vtkWarningMacro("SetActiveTag: " << (this->GetID() ? this->GetID() : "NULL ID") << " found another node " << node->GetID() << " with the tag " << _arg);
        node->SetActiveTag(nullptr);
        }
      }
    }
  else
    {
    vtkDebugMacro("SetActiveTag: null scene or tag, not checking for duplicates on camera " << (this->GetName() ? this->GetName() : "no name")
                    << ", input arg = " << (_arg == nullptr ? "NULL" : _arg));
    }
  this->InvokeEvent(vtkMRMLCameraNode::ActiveTagModifiedEvent, nullptr);
}

//----------------------------------------------------------------------------
vtkMRMLCameraNode* vtkMRMLCameraNode::FindActiveTagInScene(const char* tag)
{
  if (this->Scene == nullptr || tag == nullptr)
    {
    return nullptr;
    }

  vtkMRMLCameraNode* node = nullptr;
  int nnodes = this->Scene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLCameraNode::SafeDownCast (
       this->Scene->GetNthNodeByClass(n, "vtkMRMLCameraNode"));
    if (node != this &&
        node->GetActiveTag() &&
        !strcmp(node->GetActiveTag(), tag))
      {
      return node;
      }
    }

  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::ResetClippingRange()
{
  this->InvokeEvent(vtkMRMLCameraNode::ResetCameraClippingEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::RotateTo(Direction position)
{
  double directionOfView[3];
  vtkMath::Subtract(this->GetFocalPoint(), this->GetPosition(),
                    directionOfView);
  double norm = vtkMath::Norm(directionOfView);

  double newDirectionOfView[3] = {0., 0., 0.};
  double newViewUp[3] = {0., 0., 0.};
  switch (position)
    {
    case Right:
      newDirectionOfView[0] = -1.;
      newViewUp[2] = 1.;
      break;
    case Left:
      newDirectionOfView[0] = 1.;
      newViewUp[2] = 1.;
      break;
    case Superior:
      newDirectionOfView[2] = -1.;
      newViewUp[1] = -1.;
      break;
    case Inferior:
      newDirectionOfView[2] = 1.;
      newViewUp[1] = 1.;
      break;
    case Anterior:
      newDirectionOfView[1] = -1.;
      newViewUp[2] = 1.;
      break;
    case Posterior:
      newDirectionOfView[1] = 1.;
      newViewUp[2] = 1.;
      break;
    }
  vtkMath::MultiplyScalar(newDirectionOfView, norm);

  double newPosition[3];
  vtkMath::Subtract(this->GetFocalPoint(), newDirectionOfView, newPosition);

  int wasModifying = this->StartModify();
  this->SetPosition(newPosition);
  this->SetViewUp(newViewUp);
  this->EndModify(wasModifying);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::RotateAround(RASAxis axis, bool clockWise)
{
  this->RotateAround(axis, clockWise ? -15. : 15.);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::RotateAround(RASAxis axis, double angle)
{
  switch (axis)
    {
    case R:
      this->Camera->Elevation(angle);
      break;
    case S:
      this->Camera->Azimuth(angle);
      break;
    case A:
      this->Camera->Yaw(angle);
      break;
    }
  this->Camera->OrthogonalizeViewUp();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::TranslateAlong(ScreenAxis screenAxis, bool positive)
{
  const double distance = this->Camera->GetDistance();
  const double viewAngle = vtkMath::RadiansFromDegrees( this->Camera->GetViewAngle() );
  const double height = tan( viewAngle / 2) * distance * 2.;
  const double shift = (positive ? 1. : -1. ) * height / 6.;
  double offset[3] = {0., 0., 0.};
  switch(screenAxis)
    {
    case X:
      {
      double* x = offset;
      double* y = this->GetViewUp();
      double z[3];
      this->Camera->GetDirectionOfProjection(z);
      vtkMath::Cross(y, z, x);
      break;
      }
    case Y:
      {
      this->Camera->GetViewUp(offset);
      break;
      }
    case Z:
      {
      this->Camera->GetDirectionOfProjection(offset);
      break;
      }
    }
  vtkMath::MultiplyScalar(offset, shift);
  int wasModifying = this->StartModify();
  const double* position = this->Camera->GetPosition();
  this->SetPosition(position[0] + offset[X],
                    position[1] + offset[Y],
                    position[2] + offset[Z]);
  const double* focalPoint = this->Camera->GetFocalPoint();
  this->SetFocalPoint(focalPoint[0] + offset[X],
                      focalPoint[1] + offset[Y],
                      focalPoint[2] + offset[Z]);

  this->ResetClippingRange();

  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraNode::Reset(bool resetRotation,
                              bool resetTranslation,
                              bool resetDistance,
                              vtkRenderer* renderer)
{
  double bounds[6] = {-5.0, 5.0, -5.0, 5.0, -5.0, 5.0};
  double center[3] = {0., 0., 0.};
  double distance = 10.;
  if (renderer)
    {
    renderer->ComputeVisiblePropBounds(bounds);
    center[0] = (bounds[1] + bounds[0]) / 2.0;
    center[1] = (bounds[3] + bounds[2]) / 2.0;
    center[2] = (bounds[5] + bounds[4]) / 2.0;
    const double w1 = bounds[1] - bounds[0];
    const double w2 = bounds[3] - bounds[2];
    const double w3 = bounds[5] - bounds[4];
    double radius = w1*w1 + w2*w2 + w3*w3;
    radius = (radius==0)?(1.0):(radius);
    // compute the radius of the enclosing sphere
    radius = sqrt(radius)*0.5;
    const double angle = vtkMath::RadiansFromDegrees(this->Camera->GetViewAngle());
    distance = radius / sin(angle / 2.);
    }
  int wasModifying = this->StartModify();
  if (resetRotation)
    {
    double position[3];
    this->Camera->GetPosition(position);
    vtkMath::Normalize(position); // not really needed
    int i = position[0]*position[0] > position[1]*position[1] ? 0 : 1;
    i = (position[i]*position[i] > position[2]*position[2]) ? i : 2;
    RASAxis closestAxis = (i == 0) ? R :((i == 1) ? A : S);
    int direction = position[i] > 0 ? 0: 1;
    this->RotateTo(static_cast<Direction>(2 * closestAxis  + direction));
    }
  if (resetTranslation)
    {
    double newPosition[3];
    this->Camera->GetViewPlaneNormal(newPosition);
    vtkMath::MultiplyScalar(newPosition, this->Camera->GetDistance());
    vtkMath::Add(center, newPosition, newPosition);
    this->SetFocalPoint(center);
    this->SetPosition(newPosition);
    }
  if (resetDistance)
    {
    double newPosition[3];
    this->Camera->GetViewPlaneNormal(newPosition);
    vtkMath::MultiplyScalar(newPosition, distance);
    vtkMath::Add(this->Camera->GetFocalPoint(), newPosition, newPosition);
    this->SetPosition(newPosition);
    }
  this->Camera->ComputeViewPlaneNormal();
  this->Camera->OrthogonalizeViewUp();
  if (renderer)
    {
    renderer->ResetCameraClippingRange(bounds);
    renderer->UpdateLightsGeometryToFollowCamera();
    }
  this->EndModify(wasModifying);
}

//-----------------------------------------------------------
void vtkMRMLCameraNode::SetInteracting(int interacting)
{
  // Don't call Modified()
  this->Interacting = interacting;
}

//-----------------------------------------------------------
void vtkMRMLCameraNode::SetInteractionFlags(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlags = flags;
}
