/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#include "vtkSlicerPlaneWidget.h"

#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLMarkupsPlaneDisplayNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerPlaneRepresentation2D.h"
#include "vtkSlicerPlaneRepresentation3D.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkEvent.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>
#include <vtkPointPlacer.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

vtkStandardNewMacro(vtkSlicerPlaneWidget);

//----------------------------------------------------------------------
vtkSlicerPlaneWidget::vtkSlicerPlaneWidget()
{
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::LeftButtonReleaseEvent, vtkEvent::AltModifier, WidgetEventControlPointPlacePlaneNormal);

  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget, vtkCommand::LeftButtonPressEvent, vtkEvent::ShiftModifier,
    WidgetStateTranslatePlane, WidgetEventPlaneMoveStart, WidgetEventPlaneMoveEnd);
  this->SetEventTranslationClickAndDrag(WidgetStateOnScaleHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::AltModifier,
    WidgetStateSymmetricScale, WidgetEventSymmetricScaleStart, WidgetEventSymmetricScaleEnd);
}

//----------------------------------------------------------------------
vtkSlicerPlaneWidget::~vtkSlicerPlaneWidget() = default;

//----------------------------------------------------------------------
void vtkSlicerPlaneWidget::CreateDefaultRepresentation(
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer)
{
  vtkSmartPointer<vtkSlicerMarkupsWidgetRepresentation> rep = nullptr;
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
    {
    rep = vtkSmartPointer<vtkSlicerPlaneRepresentation2D>::New();
    }
  else
    {
    rep = vtkSmartPointer<vtkSlicerPlaneRepresentation3D>::New();
    }
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetMarkupsDisplayNode(markupsDisplayNode);
  rep->UpdateFromMRML(nullptr, 0); // full update
}

//-----------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
    {
    return false;
    }
  if (this->WidgetState == WidgetStateTranslatePlane)
    {
    distance2 = 0.0;
    return true;
    }
  else if (this->WidgetState == WidgetStateSymmetricScale)
    {
    distance2 = 0.0;
    return true;
    }
  else if (eventData->GetType() == vtkCommand::LeftButtonPressEvent &&
           !(eventData->GetModifiers() & vtkEvent::ShiftModifier))
    {
    // Because the plane widget is so large, we don't want to interrupt the mouse button down event if it is not necessary,
    // because we would interfere with the camera rotation function.
    // If the shift modifier is not enabled, then return false when the active component is not a plane.
    vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
    if (displayNode && displayNode->GetActiveComponentType() == vtkMRMLMarkupsDisplayNode::ComponentPlane)
      {
      return false;
      }
    }

  return Superclass::CanProcessInteractionEvent(eventData, distance2);
}

//-----------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  this->ApplicationLogic->PauseRender();

  bool processedEvent = false;
  switch (widgetEvent)
  {
  case WidgetEventControlPointPlacePlaneNormal:
    processedEvent = this->PlacePlaneNormal(eventData);
    break;
  case WidgetEventPlaneMoveStart:
    processedEvent = this->ProcessPlaneMoveStart(eventData);
    break;
  case WidgetEventMouseMove:
    processedEvent = this->ProcessMouseMove(eventData);
    break;
  case WidgetEventPlaneMoveEnd:
    processedEvent = this->ProcessPlaneMoveEnd(eventData);
    break;
  case WidgetEventSymmetricScaleStart:
    processedEvent = ProcessWidgetSymmetricScaleStart(eventData);
    break;
  case WidgetEventSymmetricScaleEnd:
    processedEvent = ProcessEndMouseDrag(eventData);
    break;
  }

  if (!processedEvent)
    {
    processedEvent = Superclass::ProcessInteractionEvent(eventData);
    }

  this->ApplicationLogic->ResumeRender();
  return processedEvent;
}

//----------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessPlaneMoveStart(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!displayNode || displayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentPlane)
    {
    return false;
    }
  this->SetWidgetState(WidgetStateTranslatePlane);
  this->StartWidgetInteraction(eventData);
  return true;
}

//----------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessPlaneMoveEnd(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!displayNode || displayNode->GetActiveComponentType() != vtkMRMLMarkupsDisplayNode::ComponentPlane)
    {
    return false;
    }
  this->SetWidgetState(WidgetStateOnWidget);
  this->EndWidgetInteraction();
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessUpdatePlaneFromViewNormal(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode)
    {
    return false;
    }

  // Get world position
  double eventPos_World[3] = { 0.0 };
  double eventOrientation_World[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (eventData->IsWorldPositionValid() && eventData->IsWorldPositionAccurate())
    {
    eventData->GetWorldPosition(eventPos_World);

    double worldOrientationQuaternion[4] = { 0.0 };
    eventData->GetWorldOrientation(worldOrientationQuaternion);
    vtkMRMLMarkupsNode::ConvertOrientationWXYZToMatrix(worldOrientationQuaternion, eventOrientation_World);
    }
  else if (eventData->IsDisplayPositionValid())
    {
    int displayPos[2] = { 0 };
    eventData->GetDisplayPosition(displayPos);
    if (!this->ConvertDisplayPositionToWorld(displayPos, eventPos_World, eventOrientation_World))
      {
      eventData->GetWorldPosition(eventPos_World);
      }
    }
  eventData->SetWorldPosition(eventPos_World);

  vtkSlicerPlaneRepresentation2D* rep2d = vtkSlicerPlaneRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerPlaneRepresentation3D* rep3d = vtkSlicerPlaneRepresentation3D::SafeDownCast(this->WidgetRep);

  if (rep2d)
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(rep2d->GetViewNode());
    if (sliceNode)
      {
      // Orient the plane so that the axes of the plane line up with the axes of the slice.

      vtkNew<vtkTransform> sliceToRASTransform;
      sliceToRASTransform->SetMatrix(sliceNode->GetSliceToRAS());

      double yAxis_World[3] = { 0.0, 1.0, 0.0 };
      sliceToRASTransform->TransformVector(yAxis_World, yAxis_World);
      double zAxis_World[3] = { 0.0, 0.0, 1.0 };
      sliceToRASTransform->TransformVector(zAxis_World, zAxis_World);
      double xAxis_World[3] = { 1.0, 0.0, 0.0 };
      vtkMath::Cross(yAxis_World, zAxis_World, xAxis_World);
      vtkMath::Normalize(xAxis_World);

      planeNode->SetAxesWorld(xAxis_World, yAxis_World, zAxis_World);
      }
    }
  else if (rep3d)
    {
    int displayPos[2] = { 0, 0 };
    eventData->GetDisplayPosition(displayPos);
    double pickPos[3] = { 0.0, 0.0, 0.0 };
    double zAxis_World[3] = { 0.0, 0.0, 0.0 };
    bool pickSuccessful = rep3d->AccuratePick(displayPos[0], displayPos[1], pickPos, zAxis_World);

    double yAxis_World[3] = { 0.0, 1.0, 0.0 };
    vtkCamera* camera = this->Renderer->GetActiveCamera();
    if (camera)
      {
      camera->GetViewUp(yAxis_World);
      }

    if (camera && !pickSuccessful)
      {
      // Did not find any pickable surface.
      // Face plane towards the camera.
      camera->GetDirectionOfProjection(zAxis_World);
      vtkMath::MultiplyScalar(zAxis_World, -1.0);
      }
    vtkMath::Normalize(zAxis_World);

    double epsilon = 1e-2;
    if (vtkMath::Dot(zAxis_World, yAxis_World) >= 1.0 - epsilon)
      {
      // If the up vector and normal vector are pointing in the same direction, then just set the normal.
      // The plane node will sort out the other new axes directions.
      planeNode->SetNormalWorld(zAxis_World);
      }
    else
      {
      // Ensure that the axes are orthogonal
      double xAxis_World[3] = { 1.0, 0.0, 0.0 };
      vtkMath::Cross(yAxis_World, zAxis_World, xAxis_World);
      vtkMath::Normalize(xAxis_World);
      vtkMath::Cross(zAxis_World, xAxis_World, yAxis_World);
      vtkMath::Normalize(yAxis_World);
      planeNode->SetAxesWorld(xAxis_World, yAxis_World, zAxis_World);
      }

    }

  planeNode->SetIsPlaneValid(true);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessWidgetSymmetricScaleStart(vtkMRMLInteractionEventData* eventData)
{
  if ((this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateOnWidget && this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateOnScaleHandle)
    || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(WidgetStateSymmetricScale);
  this->StartWidgetInteraction(eventData);
  return true;
}

//----------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState == WidgetStateTranslatePlane)
    {
    return this->ProcessPlaneTranslate(eventData);
    }
  else if (this->WidgetState == WidgetStateSymmetricScale)
    {
    return this->ProcessPlaneSymmetricScale(eventData);
    }

  bool processed = Superclass::ProcessMouseMove(eventData);

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (planeNode && this->WidgetState == WidgetStateDefine && planeNode->GetPlaneType() == vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal &&
      planeNode->GetNumberOfControlPoints() == 1 && this->PreviewPointIndex == 0)
    {
    // If we are using a point-normal plane type, then update the plane so that its normal is aligned with the view normal
    processed |= this->ProcessUpdatePlaneFromViewNormal(eventData);
    }

  return processed;
}

//----------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessPlaneTranslate(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode)
    {
    return false;
    }

  double eventPos[2]
    {
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1]),
    };

  double ref[3] = { 0. };
  double worldPos[3], worldOrient[9];

  vtkSlicerPlaneRepresentation2D* rep2d = vtkSlicerPlaneRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerPlaneRepresentation3D* rep3d = vtkSlicerPlaneRepresentation3D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    // 2D view
    double slicePos[3] = { 0. };
    slicePos[0] = this->LastEventPosition[0];
    slicePos[1] = this->LastEventPosition[1];
    rep2d->GetSliceToWorldCoordinates(slicePos, ref);

    slicePos[0] = eventPos[0];
    slicePos[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(slicePos, worldPos);
    }
  else if (rep3d)
    {
    // 3D view
    int displayPos[2] = { 0 };

    displayPos[0] = static_cast<int>(std::floor(this->LastEventPosition[0]));
    displayPos[1] = static_cast<int>(std::floor(this->LastEventPosition[1]));

    if (!this->ConvertDisplayPositionToWorld(displayPos, worldPos, worldOrient))
      {
      return false;
      }
    ref[0] = worldPos[0];
    ref[1] = worldPos[1];
    ref[2] = worldPos[2];

    displayPos[0] = eventPos[0];
    displayPos[1] = eventPos[1];

    if (!this->ConvertDisplayPositionToWorld(displayPos, worldPos, worldOrient))
      {
      return false;
      }
    }

  double vector_World[3];
  vector_World[0] = worldPos[0] - ref[0];
  vector_World[1] = worldPos[1] - ref[1];
  vector_World[2] = worldPos[2] - ref[2];

  bool lockToNormal = false;
  if (lockToNormal)
    {
    double normal[3] = { 0 };
    markupsNode->GetNormal(normal);

    double magnitude = vtkMath::Dot(vector_World, normal);
    vtkMath::MultiplyScalar(normal, magnitude);
    for (int i = 0; i < 3; ++i)
      {
      vector_World[i] = normal[i];
      }
    }

  MRMLNodeModifyBlocker blocker(markupsNode);

  vtkNew<vtkMatrix4x4> worldToObjectMatrix;
  markupsNode->GetObjectToWorldMatrix(worldToObjectMatrix);
  worldToObjectMatrix->Invert();

  vtkNew<vtkTransform> worldToObjectTransform;
  worldToObjectTransform->PostMultiply();
  worldToObjectTransform->SetMatrix(worldToObjectMatrix);
  worldToObjectTransform->Concatenate(markupsNode->GetObjectToBaseMatrix());

  double vector_Plane[3] = { 0.0 };
  worldToObjectTransform->TransformVector(vector_World, vector_Plane);

  vtkNew<vtkTransform> objectToBaseTransform;
  objectToBaseTransform->PostMultiply();
  objectToBaseTransform->SetMatrix(markupsNode->GetObjectToBaseMatrix());
  objectToBaseTransform->Translate(vector_Plane);
  markupsNode->GetObjectToBaseMatrix()->DeepCopy(objectToBaseTransform->GetMatrix());

  markupsNode->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent);

  this->LastEventPosition[0] = eventPos[0];
  this->LastEventPosition[1] = eventPos[1];
  return true;
}

//----------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessPlaneSymmetricScale(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep || !markupsNode || !eventData)
    {
    return false;
    }

  // Process the motion
  // Based on the displacement vector (computed in display coordinates) and
  // the cursor state (which corresponds to which part of the widget has been
  // selected), the widget points are modified.
  // First construct a local coordinate system based on the display coordinates
  // of the widget.
  double eventPos[2]
    {
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1]),
    };

  this->ScaleWidget(eventPos, true);

  this->LastEventPosition[0] = eventPos[0];
  this->LastEventPosition[1] = eventPos[1];

  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return false;
    }

  if (this->WidgetState == vtkSlicerPlaneWidget::WidgetStateSymmetricScale)
    {
    int activeComponentType = this->GetActiveComponentType();
    if (activeComponentType == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
      {
      this->SetWidgetState(WidgetStateOnScaleHandle);
      }
    else
      {
      this->SetWidgetState(WidgetStateOnWidget);
      }
    this->EndWidgetInteraction();
    }

  return Superclass::ProcessEndMouseDrag(eventData);
}

//-------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::ProcessWidgetStopPlace(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (markupsNode)
    {
    markupsNode->SetNormalPointRequired(false);
    }
  return Superclass::ProcessWidgetStopPlace(eventData);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneWidget::RotateWidget(double eventPos[2])
{
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode)
    {
    return;
    }

  double eventPos_World[3] = { 0. };
  double lastEventPos_World[3] = { 0. };
  double orientation_World[9] = { 0. };
  double eventPos_Display[2] = { 0. };

  vtkSlicerMarkupsWidgetRepresentation* rep = vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  vtkSlicerMarkupsWidgetRepresentation2D* rep2d = vtkSlicerMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerMarkupsWidgetRepresentation3D* rep3d = vtkSlicerMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    double eventPos_Slice[3] = { 0. };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);

    eventPos_Display[0] = eventPos_Slice[0];
    eventPos_Display[1] = eventPos_Slice[1];
    }
  else if (rep3d)
    {
    if (rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      this->LastEventPosition, lastEventPos_World, orientation_World))
      {
      for (int i = 0; i < 3; i++)
        {
        eventPos_World[i] = lastEventPos_World[i];
        }
      }
    else
      {
      return;
      }

    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      eventPos, eventPos_World, eventPos_World, orientation_World))
      {
      return;
      }
    }

  double origin_World[3] = { 0.0 };
  rep->GetInteractionHandleOriginWorld(origin_World);

  double epsilon = 1e-5;
  double d2 = vtkMath::Distance2BetweenPoints(eventPos_World, origin_World);
  if (d2 < epsilon)
    {
    return;
    }

  for (int i = 0; i < 3; i++)
    {
    lastEventPos_World[i] -= origin_World[i];
    eventPos_World[i] -= origin_World[i];
    }

  double angle = vtkMath::DegreesFromRadians(
    vtkMath::AngleBetweenVectors(lastEventPos_World, eventPos_World));
  double rotationNormal_World[3] = { 0.0 };
  vtkMath::Cross(lastEventPos_World, eventPos_World, rotationNormal_World);
  double rotationAxis_World[3] = { 0.0, 1.0, 0.0 };
  int type = this->GetActiveComponentType();
  if (type == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle)
    {
    int index = this->GetMarkupsDisplayNode()->GetActiveComponentIndex();
    double eventPositionOnAxisPlane_World[3] = { 0.0, 0.0, 0.0 };
    if (!this->GetIntersectionOnAxisPlane(type, index, eventPos, eventPositionOnAxisPlane_World))
      {
      vtkWarningMacro("RotateWidget: Could not calculate intended orientation");
      return;
      }

    rep->GetInteractionHandleAxisWorld(type, index, rotationAxis_World); // Axis of rotation
    double origin_World[3] = { 0.0, 0.0, 0.0 };
    rep->GetInteractionHandleOriginWorld(origin_World);

    double lastEventPositionOnAxisPlane_World[3] = { 0.0, 0.0, 0.0 };
    if (!this->GetIntersectionOnAxisPlane(
      vtkMRMLMarkupsDisplayNode::ComponentRotationHandle, index, this->LastEventPosition,lastEventPositionOnAxisPlane_World))
      {
      vtkWarningMacro("RotateWidget: Could not calculate previous orientation");
      return;
      }

    double rotationHandleVector_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Subtract(lastEventPositionOnAxisPlane_World, origin_World, rotationHandleVector_World);

    double destinationVector_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Subtract(eventPositionOnAxisPlane_World, origin_World, destinationVector_World);

    angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(rotationHandleVector_World, destinationVector_World));
    vtkMath::Cross(rotationHandleVector_World, destinationVector_World, rotationNormal_World);
    }
  else
    {
    rotationAxis_World[0] = rotationNormal_World[0];
    rotationAxis_World[1] = rotationNormal_World[1];
    rotationAxis_World[2] = rotationNormal_World[2];
    }

  if (vtkMath::Dot(rotationNormal_World, rotationAxis_World) < 0.0)
    {
    angle *= -1.0;
    }

  vtkNew<vtkMatrix4x4> objectToNodeMatrix;
  markupsNode->GetObjectToNodeMatrix(objectToNodeMatrix);

  vtkNew<vtkMatrix4x4> nodeToObjectMatrix;
  vtkMatrix4x4::Invert(objectToNodeMatrix, nodeToObjectMatrix);

  vtkNew<vtkTransform> objectToNodeTransform;
  objectToNodeTransform->SetMatrix(objectToNodeMatrix);

  vtkNew<vtkMatrix4x4> worldToObjectMatrix;
  markupsNode->GetObjectToWorldMatrix(worldToObjectMatrix);
  worldToObjectMatrix->Invert();
  vtkNew<vtkTransform> worldToObjectTransform;
  worldToObjectTransform->SetMatrix(worldToObjectMatrix);

  double rotationAxis_Object[3] = { 0.0, 0.0, 0.0 };
  worldToObjectTransform->TransformVector(rotationAxis_World, rotationAxis_Object);

  vtkNew<vtkTransform> rotateTransform;
  rotateTransform->PostMultiply();
  rotateTransform->Concatenate(nodeToObjectMatrix);
  rotateTransform->RotateWXYZ(angle, rotationAxis_Object);
  rotateTransform->Concatenate(objectToNodeMatrix);
  markupsNode->ApplyTransform(rotateTransform);
}


//----------------------------------------------------------------------
void vtkSlicerPlaneWidget::ScaleWidget(double eventPos[2])
{
  this->ScaleWidget(eventPos, false);
}

//----------------------------------------------------------------------
void vtkSlicerPlaneWidget::ScaleWidget(double eventPos[2], bool symmetricScale)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !displayNode)
    {
    return;
    }

  double lastEventPos_World[3] = { 0.0 };
  double eventPos_World[3] = { 0.0 };
  double orientation_World[9] = { 0.0 };

  vtkSlicerPlaneRepresentation2D* rep2d = vtkSlicerPlaneRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerPlaneRepresentation3D* rep3d = vtkSlicerPlaneRepresentation3D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    // 2D view
    double eventPos_Slice[3] = { 0. };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);
    }
  else if (rep3d)
    {
    // 3D view
    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      this->LastEventPosition, lastEventPos_World, orientation_World))
      {
      return;
      }

    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      eventPos, lastEventPos_World, eventPos_World, orientation_World))
      {
      return;
      }
    }

  if (this->GetActiveComponentType() == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    vtkNew<vtkMatrix4x4> worldToObjectMatrix;
    markupsNode->GetObjectToWorldMatrix(worldToObjectMatrix);
    worldToObjectMatrix->Invert();
    vtkNew<vtkTransform> worldToObjectTransform;
    worldToObjectTransform->SetMatrix(worldToObjectMatrix);

    int index = displayNode->GetActiveComponentIndex();
    if (index <= vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge)
      {
      // We are interacting with one of the edges.
      // Restrict scaling to apply only in that direction.
      this->GetClosestPointOnInteractionAxis(
        vtkMRMLMarkupsDisplayNode::ComponentScaleHandle, index, this->LastEventPosition, lastEventPos_World);
      this->GetClosestPointOnInteractionAxis(
        vtkMRMLMarkupsDisplayNode::ComponentScaleHandle, index, eventPos, eventPos_World);
      }
    else
      {
      // We are interacting with one of the corners
      // Restrict scaling so that the projection of the interaction handle and the event position will be at the same display position.
      double cameraDirectionEventPos_World[3] = { 0.0, 0.0, 0.0 };
      double cameraDirectionLastEventPos_World[3] = { 0.0, 0.0, 0.0 };
      if (rep2d)
        {
        vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(rep2d->GetViewNode());
        if (sliceNode)
          {
          // Get the slice view normal
          double normal4_World[4] = { 0.0, 0.0, 1.0, 0.0 };
          sliceNode->GetSliceToRAS()->MultiplyPoint(normal4_World, normal4_World);
          cameraDirectionEventPos_World[0] = normal4_World[0];
          cameraDirectionEventPos_World[1] = normal4_World[1];
          cameraDirectionEventPos_World[2] = normal4_World[2];
          cameraDirectionLastEventPos_World[0] = normal4_World[0];
          cameraDirectionLastEventPos_World[1] = normal4_World[1];
          cameraDirectionLastEventPos_World[2] = normal4_World[2];
          }
        }
      else if (rep3d)
        {
        vtkCamera* camera = this->Renderer->GetActiveCamera();
        if (camera && camera->GetParallelProjection())
          {
          camera->GetDirectionOfProjection(cameraDirectionEventPos_World);
          camera->GetDirectionOfProjection(cameraDirectionLastEventPos_World);
          }
        else if (camera)
          {
          // Camera position
          double cameraPosition_World[4] = { 0.0 };
          camera->GetPosition(cameraPosition_World);

          //  Compute the ray endpoints. The ray is along the line running from
          //  the camera position to the selection point, starting where this line
          //  intersects the front clipping plane, and terminating where this
          //  line intersects the back clipping plane.
          vtkMath::Subtract(cameraPosition_World, eventPos_World, cameraDirectionEventPos_World);
          vtkMath::Subtract(cameraPosition_World, lastEventPos_World, cameraDirectionLastEventPos_World);
          }
        }

      // Create a second point running along the line from the camera to the event position
      double eventPos2_World[3];
      vtkMath::Add(eventPos_World, cameraDirectionEventPos_World, eventPos2_World);
      double lastEventPos2_World[3];
      vtkMath::Add(lastEventPos_World, cameraDirectionLastEventPos_World, lastEventPos2_World);
      double t; // not used

      double normal_World[3] = { 0.0, 0.0, 0.0 };
      markupsNode->GetNormalWorld(normal_World);
      double origin_World[3] = { 0.0, 0.0, 0.0 };
      markupsNode->GetOriginWorld(origin_World);

      // Find the intersection of the vector from event positions, along the vector from the camera at that point
      vtkNew<vtkPlane> plane;
      plane->SetOrigin(origin_World);
      plane->SetNormal(normal_World);
      plane->IntersectWithLine(eventPos_World, eventPos2_World, t, eventPos_World);
      plane->IntersectWithLine(lastEventPos_World, lastEventPos2_World, t, lastEventPos_World);
      }

    double scaleVector_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Subtract(eventPos_World, lastEventPos_World, scaleVector_World);

    double scaleVector_Object[3] = { 0.0, 0.0, 0.0 };
    worldToObjectTransform->TransformVector(scaleVector_World, scaleVector_Object);

    double bounds_Plane[4] = { 0.0, -1.0, 0.0, -1.0 };
    markupsNode->GetPlaneBounds(bounds_Plane);

    switch (index)
      {
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
        bounds_Plane[0] += scaleVector_Object[0];
        if (symmetricScale)
          {
          bounds_Plane[1] -= scaleVector_Object[0];
          }
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleREdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
        bounds_Plane[1] += scaleVector_Object[0];
        if (symmetricScale)
          {
          bounds_Plane[0] -= scaleVector_Object[0];
          }
        break;
      default:
        break;
      }

    switch (index)
      {
      case vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
        bounds_Plane[2] += scaleVector_Object[1];
        if (symmetricScale)
          {
          bounds_Plane[3] -= scaleVector_Object[1];
          }
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
        bounds_Plane[3] += scaleVector_Object[1];
        if (symmetricScale)
          {
          bounds_Plane[2] -= scaleVector_Object[1];
          }
        break;
      default:
        break;
      }

    // If we have crossed over the origin, we need to flip the selected handle across the Right and/or Anterior axis.
    bool flipLRHandle = bounds_Plane[1] < bounds_Plane[0];
    bool flipPAHandle = bounds_Plane[3] < bounds_Plane[2];
    if (flipLRHandle || flipPAHandle)
      {
      this->FlipPlaneHandles(flipLRHandle, flipPAHandle);
      }

    if (bounds_Plane[1] < bounds_Plane[0])
      {
      double tempBounds_Plane0 = bounds_Plane[0];
      bounds_Plane[0] = bounds_Plane[1];
      bounds_Plane[1] = tempBounds_Plane0;
      }

    if (bounds_Plane[3] < bounds_Plane[2])
      {
      double tempBounds_Plane2 = bounds_Plane[2];
      bounds_Plane[2] = bounds_Plane[3];
      bounds_Plane[3] = tempBounds_Plane2;
      }

    markupsNode->SetPlaneBounds(bounds_Plane);
    markupsNode->SetSizeMode(vtkMRMLMarkupsPlaneNode::SizeModeAbsolute);
    }
}

//----------------------------------------------------------------------
void vtkSlicerPlaneWidget::FlipPlaneHandles(bool flipLRHandle, bool flipPAHandle)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !displayNode)
    {
    return;
    }

  int index = displayNode->GetActiveComponentIndex();
  if (flipLRHandle)
    {
    switch (index)
      {
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleREdge;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleREdge:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner;
        break;
      default:
        break;
      }
    }

  if (flipPAHandle)
    {
    switch (index)
      {
      case vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner;
        break;
      default:
        break;
      }
    }

  displayNode->SetActiveComponent(displayNode->GetActiveComponentType(), index);
}

//---------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::PlacePoint(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode)
    {
      return false;
    }

  if (planeNode->GetPlaneType() == vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal &&
    planeNode->GetNumberOfDefinedControlPoints() > 0)
    {
    planeNode->SetNormalPointRequired(false);
    }

  bool success = Superclass::PlacePoint(eventData);
  if (!success)
    {
    return false;
    }

  // We have returned to place mode. Need to delete the unnecessary points
  if (planeNode->GetPlaneType() == vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal)
    {
    planeNode->UpdateControlPointsFromPlane();
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::PlacePlaneNormal(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode)
    {
    return false;
    }

  if (planeNode->GetPlaneType() != vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal)
    {
    // We don't do anything different if we are not in point normal mode.
    return this->PlacePoint(eventData);
    }

  if (planeNode->GetNumberOfDefinedControlPoints() == 0)
    {
    planeNode->SetNormalPointRequired(true);
    }
  else
    {
    planeNode->SetNormalPointRequired(false);
    }

  if (!this->PlacePoint(eventData))
    {
    return false;
    }

  return true;
}
