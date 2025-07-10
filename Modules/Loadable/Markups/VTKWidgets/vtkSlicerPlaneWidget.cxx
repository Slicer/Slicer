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
  this->SetEventTranslation(WidgetStateDefine,
                            vtkCommand::LeftButtonReleaseEvent,
                            vtkEvent::AltModifier,
                            WidgetEventControlPointPlacePlaneNormal);

  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget,
                                        vtkCommand::LeftButtonPressEvent,
                                        vtkEvent::ShiftModifier,
                                        WidgetStateTranslatePlane,
                                        WidgetEventPlaneMoveStart,
                                        WidgetEventPlaneMoveEnd);
}

//----------------------------------------------------------------------
vtkSlicerPlaneWidget::~vtkSlicerPlaneWidget() = default;

//----------------------------------------------------------------------
void vtkSlicerPlaneWidget::CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode,
                                                       vtkMRMLAbstractViewNode* viewNode,
                                                       vtkRenderer* renderer)
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
  else if (eventData->GetType() == vtkCommand::LeftButtonPressEvent
           && !(eventData->GetModifiers() & vtkEvent::ShiftModifier))
  {
    // Because the plane widget is so large, we don't want to interrupt the mouse button down event if it is not
    // necessary, because we would interfere with the camera rotation function. If the shift modifier is not enabled,
    // then return false when the active component is not a plane.
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
  if ((this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateOnWidget) || this->IsAnyControlPointLocked())
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
  if (planeNode && this->WidgetState == WidgetStateDefine
      && planeNode->GetPlaneType() == vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal
      && planeNode->GetNumberOfControlPoints() == 1 && this->PreviewPointIndex == 0)
  {
    // If we are using a point-normal plane type, then update the plane so that its normal is aligned with the view
    // normal
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

  double eventPos[2]{
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
bool vtkSlicerPlaneWidget::ProcessPlaneSymmetricScale(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  return false;
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
    this->SetWidgetState(WidgetStateOnWidget);
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

//---------------------------------------------------------------------------
bool vtkSlicerPlaneWidget::PlacePoint(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!planeNode)
  {
    return false;
  }

  if (planeNode->GetPlaneType() == vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal
      && planeNode->GetNumberOfDefinedControlPoints() > 0)
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
