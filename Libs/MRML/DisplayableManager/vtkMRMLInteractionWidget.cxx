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

// MRMLDM includes
#include "vtkMRMLInteractionWidget.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionWidgetRepresentation.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkLine.h>
#include <vtkPlane.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>

//----------------------------------------------------------------------
vtkMRMLInteractionWidget::vtkMRMLInteractionWidget()
{
  this->LastEventPosition[0] = 0.0;
  this->LastEventPosition[1] = 0.0;
  this->StartEventOffsetPosition[0] = 0.0;
  this->StartEventOffsetPosition[1] = 0.0;

  // Update active component
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseMoveEvent, vtkEvent::AnyModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::Move3DEvent, vtkEvent::AnyModifier, WidgetEventMouseMove);

  // Translation
  this->SetEventTranslationClickAndDrag(WidgetStateOnTranslationHandle,
                                        vtkCommand::LeftButtonPressEvent,
                                        vtkEvent::NoModifier,
                                        WidgetStateTranslate,
                                        WidgetEventTranslateStart,
                                        WidgetEventTranslateEnd);

  // Rotation
  this->SetEventTranslationClickAndDrag(WidgetStateOnRotationHandle,
                                        vtkCommand::LeftButtonPressEvent,
                                        vtkEvent::NoModifier,
                                        WidgetStateRotate,
                                        WidgetEventRotateStart,
                                        WidgetEventRotateEnd);

  // Scale
  this->SetEventTranslationClickAndDrag(WidgetStateOnScaleHandle,
                                        vtkCommand::LeftButtonPressEvent,
                                        vtkEvent::NoModifier,
                                        WidgetStateScale,
                                        WidgetEventScaleStart,
                                        WidgetEventScaleEnd);

  // Uniform scale
  this->SetEventTranslationClickAndDrag(WidgetStateOnScaleHandle,
                                        vtkCommand::LeftButtonPressEvent,
                                        vtkEvent::AltModifier,
                                        WidgetStateUniformScale,
                                        WidgetEventUniformScaleStart,
                                        WidgetEventUniformScaleEnd);

  for (unsigned int interactionHandleState = WidgetStateInteraction_First;
       interactionHandleState < WidgetStateInteraction_Last;
       ++interactionHandleState)
  {
    // Jump slices
    this->SetEventTranslation(interactionHandleState,
                              vtkMRMLInteractionEventData::LeftButtonClickEvent,
                              vtkEvent::NoModifier,
                              WidgetEventJumpCursor);

    // Context menu events
    this->SetEventTranslation(
      interactionHandleState, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetEventReserved);
    this->SetEventTranslation(
      interactionHandleState, vtkCommand::RightButtonReleaseEvent, vtkEvent::NoModifier, WidgetEventReserved);
    this->SetEventTranslation(interactionHandleState,
                              vtkMRMLInteractionEventData::RightButtonClickEvent,
                              vtkEvent::NoModifier,
                              WidgetEventMenu);

    // Update active interaction handle component
    this->SetEventTranslation(
      interactionHandleState, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
    this->SetEventTranslation(
      interactionHandleState, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  }
}

//----------------------------------------------------------------------
vtkMRMLInteractionWidget::~vtkMRMLInteractionWidget() = default;

//----------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessWidgetRotateStart(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkMRMLInteractionWidget::WidgetStateOnRotationHandle)
  {
    return false;
  }
  this->SetWidgetState(WidgetStateRotate);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessWidgetScaleStart(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkMRMLInteractionWidget::WidgetStateOnScaleHandle)
  {
    return false;
  }
  this->SetWidgetState(WidgetStateScale);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessWidgetUniformScaleStart(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkMRMLInteractionWidget::WidgetStateOnScaleHandle)
  {
    return false;
  }
  this->SetWidgetState(WidgetStateUniformScale);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessWidgetTranslateStart(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkMRMLInteractionWidget::WidgetStateOnTranslationHandle)
  {
    return false;
  }
  this->SetWidgetState(WidgetStateTranslate);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLInteractionWidgetRepresentation* rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep || !eventData)
  {
    return false;
  }

  int state = this->WidgetState;
  if (state == WidgetStateIdle || state == WidgetStateOnTranslationHandle || state == WidgetStateOnRotationHandle
      || state == WidgetStateOnScaleHandle)
  {
    // update state
    int foundComponentType = InteractionNone;
    int foundComponentIndex = -1;
    double closestDistance2 = 0.0;
    rep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2);
    if (foundComponentType == InteractionNone)
    {
      this->SetWidgetState(WidgetStateIdle);
    }
    else if (foundComponentType == InteractionTranslationHandle)
    {
      this->SetWidgetState(WidgetStateOnTranslationHandle);
    }
    else if (foundComponentType == InteractionRotationHandle)
    {
      this->SetWidgetState(WidgetStateOnRotationHandle);
    }
    else if (foundComponentType == InteractionScaleHandle)
    {
      this->SetWidgetState(WidgetStateOnScaleHandle);
    }

    this->SetActiveComponentIndex(foundComponentIndex);
    this->SetActiveComponentType(foundComponentType);
  }
  else
  {
    // Process the motion
    // Based on the displacement vector (computed in display coordinates) and
    // the cursor state (which corresponds to which part of the widget has been
    // selected), the widget points are modified.
    // First construct a local coordinate system based on the display coordinates
    // of the widget.
    double eventPos[2]{
      static_cast<double>(eventData->GetDisplayPosition()[0]),
      static_cast<double>(eventData->GetDisplayPosition()[1]),
    };

    if (state == WidgetStateTranslate)
    {
      this->TranslateWidget(eventPos);
    }
    else if (state == WidgetStateScale)
    {
      this->ScaleWidget(eventPos);
    }
    else if (state == WidgetStateUniformScale)
    {
      this->ScaleWidget(eventPos, true);
    }
    else if (state == WidgetStateRotate)
    {
      this->RotateWidget(eventPos);
    }

    this->LastEventPosition[0] = eventPos[0];
    this->LastEventPosition[1] = eventPos[1];
  }
  return true;
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
  {
    // If this event is not recognized then give a chance to process it as a click event.
    return this->CanProcessButtonClickEvent(eventData, distance2);
  }

  vtkMRMLInteractionWidgetRepresentation* rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
  {
    return false;
  }

  // Currently interacting
  if (this->WidgetState == WidgetStateTranslate || this->WidgetState == WidgetStateRotate
      || this->WidgetState == WidgetStateScale || this->WidgetState == WidgetStateUniformScale)
  {
    distance2 = 0.0;
    return true;
  }

  int foundComponentType = InteractionNone;
  int foundComponentIndex = -1;
  double closestDistance2 = 0.0;
  rep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType == InteractionNone)
  {
    return false;
  }

  // TODO: This gives the widget a very high priority.
  // There may be a better way to handle this.
  distance2 = 0.01;

  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessWidgetMenuDisplayNodeTypeAndIndex(vtkMRMLInteractionEventData* eventData,
                                                                        vtkMRMLDisplayNode* displayNode,
                                                                        int type,
                                                                        int index)
{
  if (!displayNode)
  {
    return false;
  }

  vtkMRMLInteractionWidgetRepresentation* rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
  {
    return false;
  }

  vtkNew<vtkMRMLInteractionEventData> menuEventData;
  menuEventData->SetType(vtkMRMLDisplayNode::MenuEvent);
  menuEventData->SetComponentType(type);
  menuEventData->SetComponentIndex(index);
  menuEventData->SetViewNode(this->WidgetRep->GetViewNode());

  if (eventData->IsDisplayPositionValid())
  {
    // Copy display position
    menuEventData->SetDisplayPosition(eventData->GetDisplayPosition());
  }

  double eventPos_World[3] = { 0.0, 0.0, 0.0 };
  if (eventData->IsWorldPositionValid())
  {
    // Copy world position
    eventData->GetWorldPosition(eventPos_World);
  }
  else if (eventData->IsDisplayPositionValid())
  {
    // Compute world position
    double worldOrientationMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    int displayPos[2] = { 0, 0 };
    eventData->GetDisplayPosition(displayPos);
    double eventPos_Display[3] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0 };
    rep->GetPointPlacer()->ComputeWorldPosition(
      this->Renderer, eventPos_Display, eventPos_World, worldOrientationMatrix);
  }
  menuEventData->SetWorldPosition(eventPos_World, eventData->IsWorldPositionAccurate());

  displayNode->InvokeEvent(vtkMRMLDisplayNode::MenuEvent, menuEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
  {
    return false;
  }

  if (this->WidgetState != vtkMRMLInteractionWidget::WidgetStateTranslate
      && this->WidgetState != vtkMRMLInteractionWidget::WidgetStateScale
      && this->WidgetState != vtkMRMLInteractionWidget::WidgetStateUniformScale
      && this->WidgetState != vtkMRMLInteractionWidget::WidgetStateRotate)
  {
    return false;
  }

  int activeComponentType = this->GetActiveComponentType();
  if (activeComponentType == InteractionTranslationHandle)
  {
    this->SetWidgetState(WidgetStateOnTranslationHandle);
  }
  else if (activeComponentType == InteractionRotationHandle)
  {
    this->SetWidgetState(WidgetStateOnRotationHandle);
  }
  else if (activeComponentType == InteractionScaleHandle)
  {
    this->SetWidgetState(WidgetStateOnScaleHandle);
  }

  this->EndWidgetInteraction();

  // only claim this as processed if the mouse was moved (this allows the event to be interpreted as button click)
  bool processedEvent = eventData->GetMouseMovedSinceButtonDown();
  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessJumpCursor(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  int type = this->GetActiveComponentType();
  int index = this->GetActiveComponentIndex();
  return this->JumpToHandlePosition(type, index);
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::JumpToHandlePosition(int type, int index)
{
  vtkMRMLInteractionWidgetRepresentation* rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
  {
    return false;
  }

  vtkMRMLAbstractViewNode* viewNode = rep->GetViewNode();
  if (!viewNode)
  {
    return false;
  }

  vtkMRMLScene* scene = viewNode->GetScene();
  if (!scene)
  {
    return false;
  }

  double jumpPosition_World[3] = { 0.0, 0.0, 0.0 };
  rep->GetInteractionHandlePositionWorld(type, index, jumpPosition_World);

  int viewGroup = rep->GetViewNode()->GetViewGroup();
  vtkMRMLSliceNode::JumpAllSlices(scene,
                                  jumpPosition_World[0],
                                  jumpPosition_World[1],
                                  jumpPosition_World[2],
                                  -1,
                                  viewGroup,
                                  vtkMRMLSliceNode::SafeDownCast(viewNode));

  return true;
}

//-----------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  if (this->ApplicationLogic)
  {
    this->ApplicationLogic->PauseRender();
  }

  bool processedEvent = false;
  switch (widgetEvent)
  {
    case WidgetEventMouseMove:
      processedEvent = ProcessMouseMove(eventData);
      break;
    case WidgetEventMenu:
      processedEvent = ProcessWidgetMenu(eventData);
      break;
    case WidgetEventTranslateStart:
      processedEvent = ProcessWidgetTranslateStart(eventData);
      break;
    case WidgetEventTranslateEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventRotateStart:
      processedEvent = ProcessWidgetRotateStart(eventData);
      break;
    case WidgetEventRotateEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventScaleStart:
      processedEvent = ProcessWidgetScaleStart(eventData);
      break;
    case WidgetEventScaleEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventUniformScaleStart:
      processedEvent = ProcessWidgetUniformScaleStart(eventData);
      break;
    case WidgetEventUniformScaleEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventJumpCursor:
      processedEvent = ProcessJumpCursor(eventData);
      break;
    default:
      break;
  }

  if (!processedEvent)
  {
    processedEvent = this->ProcessButtonClickEvent(eventData);
  }

  if (this->ApplicationLogic)
  {
    this->ApplicationLogic->ResumeRender();
  }

  return processedEvent;
}

//-----------------------------------------------------------------------------
void vtkMRMLInteractionWidget::Leave(vtkMRMLInteractionEventData* eventData)
{
  Superclass::Leave(eventData);
  this->SetActiveComponentType(InteractionNone);
  this->SetActiveComponentType(-1);
  this->WidgetRep->NeedToRenderOn();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidget::StartWidgetInteraction(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLInteractionWidgetRepresentation* rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
  {
    return;
  }
  rep->InteractingOn();

  double startEventPos[2]{ static_cast<double>(eventData->GetDisplayPosition()[0]),
                           static_cast<double>(eventData->GetDisplayPosition()[1]) };

  // save the cursor position
  this->LastEventPosition[0] = startEventPos[0];
  this->LastEventPosition[1] = startEventPos[1];

  this->StartEventOffsetPosition[0] = 0;
  this->StartEventOffsetPosition[1] = 0;
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidget::EndWidgetInteraction()
{
  vtkMRMLInteractionWidgetRepresentation* rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
  {
    return;
  }
  rep->InteractingOff();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidget::TranslateWidget(double eventPos[2])
{
  double lastEventPos_World[3] = { 0.0, 0.0, 0.0 };
  double eventPos_World[3] = { 0.0, 0.0, 0.0 };

  vtkMRMLInteractionWidgetRepresentation* rep = vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
  {
    return;
  }

  if (rep->GetSliceNode())
  {
    // 2D view
    double eventPos_Slice[3] = { 0.0, 0.0, 0.0 };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);
  }
  else
  {
    // 3D view
    double orientation_World[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, this->LastEventPosition, lastEventPos_World, orientation_World))
    {
      return;
    }
    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos, lastEventPos_World, eventPos_World, orientation_World))
    {
      return;
    }
  }

  double translationVector_World[3] = { 0.0, 0.0, 0.0 };
  translationVector_World[0] = eventPos_World[0] - lastEventPos_World[0];
  translationVector_World[1] = eventPos_World[1] - lastEventPos_World[1];
  translationVector_World[2] = eventPos_World[2] - lastEventPos_World[2];

  int index = this->GetActiveComponentIndex();
  double translationAxis_World[3] = { 0.0, 0.0, 0.0 };
  rep->GetInteractionHandleAxisWorld(InteractionTranslationHandle, index, translationAxis_World);
  if (translationAxis_World[0] != 0.0 || translationAxis_World[1] != 0.0 || translationAxis_World[2] != 0)
  {
    // Only perform constrained translation if the length of the axis is non-zero.

    double lastEventPositionOnAxis_World[3] = { 0.0, 0.0, 0.0 };
    this->GetClosestPointOnInteractionAxis(
      InteractionTranslationHandle, index, this->LastEventPosition, lastEventPositionOnAxis_World);

    double eventPositionOnAxis_World[3] = { 0.0, 0.0, 0.0 };
    this->GetClosestPointOnInteractionAxis(InteractionTranslationHandle, index, eventPos, eventPositionOnAxis_World);

    vtkMath::Subtract(eventPositionOnAxis_World, lastEventPositionOnAxis_World, translationVector_World);
    double distance = vtkMath::Norm(translationVector_World);
    if (vtkMath::Dot(translationVector_World, translationAxis_World) < 0)
    {
      distance *= -1.0;
    }
    translationVector_World[0] = distance * translationAxis_World[0];
    translationVector_World[1] = distance * translationAxis_World[1];
    translationVector_World[2] = distance * translationAxis_World[2];
  }

  vtkNew<vtkTransform> translationTransform;
  translationTransform->Translate(translationVector_World);
  this->ApplyTransform(translationTransform);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidget::ScaleWidget(double eventPos[2], bool uniformScale /*=false*/)
{
  double center[3] = { 0.0, 0.0, 0.0 };
  double lastEventPos_World[3] = { 0.0, 0.0, 0.0 };
  double eventPos_World[3] = { 0.0, 0.0, 0.0 };

  vtkMRMLInteractionWidgetRepresentation* rep = vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
  {
    return;
  }

  if (rep->GetSliceNode())
  {
    // 2D view
    double slicePos[3] = { 0.0, 0.0, 0.0 };
    slicePos[0] = this->LastEventPosition[0];
    slicePos[1] = this->LastEventPosition[1];
    rep->GetSliceToWorldCoordinates(slicePos, lastEventPos_World);

    slicePos[0] = eventPos[0];
    slicePos[1] = eventPos[1];
    rep->GetSliceToWorldCoordinates(slicePos, eventPos_World);
  }
  else
  {
    // 3D view
    double orientation_World[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, this->LastEventPosition, lastEventPos_World, orientation_World))
    {
      return;
    }
    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos, lastEventPos_World, eventPos_World, orientation_World))
    {
      return;
    }
  }

  rep->GetTransformationReferencePoint(center);

  double r2 = vtkMath::Distance2BetweenPoints(lastEventPos_World, center);
  double d2 = vtkMath::Distance2BetweenPoints(eventPos_World, center);
  if (d2 < 0.0000001)
  {
    return;
  }

  double ratio = sqrt(d2 / r2);

  vtkTransform* handleToWorldTransform = rep->GetHandleToWorldTransform();
  vtkSmartPointer<vtkTransform> worldToHandleTransform =
    vtkTransform::SafeDownCast(handleToWorldTransform->GetInverse());

  double scaleVector_Local[3] = { 1.0, 1.0, 1.0 };
  if (!uniformScale)
  {
    rep->GetInteractionHandleAxisLocal(InteractionScaleHandle, this->GetActiveComponentIndex(), scaleVector_Local);
  }

  for (int i = 0; i < 3; ++i)
  {
    if (scaleVector_Local[i] == 0.0)
    {
      scaleVector_Local[i] = 1.0;
    }
    else
    {
      scaleVector_Local[i] = ratio;
    }
  }

  vtkNew<vtkTransform> scaleTransform;
  scaleTransform->PostMultiply();
  scaleTransform->Concatenate(worldToHandleTransform);
  scaleTransform->Scale(scaleVector_Local);
  scaleTransform->Concatenate(handleToWorldTransform);
  this->ApplyTransform(scaleTransform);
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidget::RotateWidget(double eventPos[2])
{
  double eventPos_World[3] = { 0.0, 0.0, 0.0 };
  double lastEventPos_World[3] = { 0.0, 0.0, 0.0 };

  vtkMRMLInteractionWidgetRepresentation* rep = vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
  {
    return;
  }

  if (rep->GetSliceNode())
  {
    // 2D view
    double eventPos_Slice[3] = { 0.0, 0.0, 0.0 };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);
  }
  else
  {
    // 3D view
    double orientation_World[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, this->LastEventPosition, lastEventPos_World, orientation_World))
    {
      return;
    }
    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos, lastEventPos_World, eventPos_World, orientation_World))
    {
      return;
    }
  }

  double origin_World[3] = { 0.0, 0.0, 0.0 };
  rep->GetInteractionHandleOriginWorld(origin_World);

  double epsilon = 1e-5;
  double distance2 = vtkMath::Distance2BetweenPoints(eventPos_World, origin_World);
  if (distance2 < epsilon)
  {
    return;
  }

  for (int i = 0; i < 3; i++)
  {
    lastEventPos_World[i] -= origin_World[i];
    eventPos_World[i] -= origin_World[i];
  }

  int type = this->GetActiveComponentType();
  int index = this->GetActiveComponentIndex();
  double eventPositionOnAxisPlane_World[3] = { 0.0, 0.0, 0.0 };
  if (!this->GetIntersectionOnAxisPlane(type, index, eventPos, eventPositionOnAxisPlane_World))
  {
    vtkWarningMacro("RotateWidget: Could not calculate intended orientation");
    return;
  }

  double rotationAxis_World[3] = { 0.0, 1.0, 0.0 };
  rep->GetInteractionHandleAxisWorld(type, index, rotationAxis_World); // Axis of rotation

  double lastEventPositionOnAxisPlane_World[3] = { 0.0, 0.0, 0.0 };
  if (!this->GetIntersectionOnAxisPlane(
        InteractionRotationHandle, index, this->LastEventPosition, lastEventPositionOnAxisPlane_World))
  {
    vtkWarningMacro("RotateWidget: Could not calculate previous orientation");
    return;
  }

  double rotationHandleVector_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Subtract(lastEventPositionOnAxisPlane_World, origin_World, rotationHandleVector_World);

  double destinationVector_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Subtract(eventPositionOnAxisPlane_World, origin_World, destinationVector_World);

  double angle =
    vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(rotationHandleVector_World, destinationVector_World));

  double rotationNormal_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Cross(rotationHandleVector_World, destinationVector_World, rotationNormal_World);

  if (vtkMath::Dot(rotationNormal_World, rotationAxis_World) < 0.0)
  {
    angle *= -1.0;
  }

  vtkNew<vtkTransform> rotateTransform;
  rotateTransform->Translate(origin_World);
  rotateTransform->RotateWXYZ(angle, rotationAxis_World);
  rotateTransform->Translate(-origin_World[0], -origin_World[1], -origin_World[2]);
  this->ApplyTransform(rotateTransform);
}

//----------------------------------------------------------------------
bool vtkMRMLInteractionWidget::GetIntersectionOnAxisPlane(int type,
                                                          int index,
                                                          const double input_Display[2],
                                                          double outputIntersection_World[3])
{
  vtkMRMLInteractionWidgetRepresentation* rep = vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
  {
    return false;
  }

  double rotationAxis[3] = { 0.0, 0.0, 0.0 };
  rep->GetInteractionHandleAxisWorld(type, index, rotationAxis); // Axis of rotation
  double origin[3] = { 0.0, 0.0, 0.0 };
  rep->GetInteractionHandleOriginWorld(origin);

  vtkNew<vtkPlane> axisPlaneWorld;
  axisPlaneWorld->SetNormal(rotationAxis);
  axisPlaneWorld->SetOrigin(origin);

  double inputPoint0_World[3] = { 0.0, 0.0, 0.0 };
  double inputPoint1_World[3] = { 0.0, 0.0, 1.0 };
  double projectionVector_World[3] = { 0.0, 0.0, 0.0 };
  if (!rep->GetSliceNode())
  {
    // 3D view
    vtkRenderer* renderer = rep->GetRenderer();
    vtkCamera* camera = renderer->GetActiveCamera();

    // Focal point position
    double cameraFP_World[4] = { 0.0, 0.0, 0.0, 0.0 };
    camera->GetFocalPoint(cameraFP_World);

    renderer->SetWorldPoint(cameraFP_World[0], cameraFP_World[1], cameraFP_World[2], cameraFP_World[3]);
    renderer->WorldToDisplay();
    double* cameraFP_Display = renderer->GetDisplayPoint();
    double selectionZ_Display = cameraFP_Display[2];

    renderer->SetDisplayPoint(input_Display[0], input_Display[1], selectionZ_Display);
    renderer->DisplayToWorld();
    double* input_World = renderer->GetWorldPoint();
    if (input_World[3] == 0.0)
    {
      vtkWarningMacro("Bad homogeneous coordinates");
      return false;
    }
    double pickPosition_World[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < 3; i++)
    {
      pickPosition_World[i] = input_World[i] / input_World[3];
    }
    if (camera->GetParallelProjection())
    {
      camera->GetDirectionOfProjection(projectionVector_World);
      for (int i = 0; i < 3; i++)
      {
        inputPoint0_World[i] = pickPosition_World[i];
      }
    }
    else
    {
      // Camera position
      double cameraPosition_World[4] = { 0.0, 0.0, 0.0, 0.0 };
      camera->GetPosition(cameraPosition_World);

      //  Compute the ray endpoints.  The ray is along the line running from
      //  the camera position to the selection point, starting where this line
      //  intersects the front clipping plane, and terminating where this
      //  line intersects the back clipping plane.
      for (int i = 0; i < 3; i++)
      {
        projectionVector_World[i] = pickPosition_World[i] - cameraPosition_World[i];
        inputPoint0_World[i] = cameraPosition_World[i];
      }
    }
    vtkMath::Add(inputPoint0_World, projectionVector_World, inputPoint1_World);
  }
  else
  {
    // 2D view
    double inputPoint0_Display[3] = { input_Display[0], input_Display[1], 0.0 };
    double inputPoint1_Display[3] = { input_Display[0], input_Display[1], 1.0 };

    vtkNew<vtkTransform> displayToWorldTransform;
    vtkMRMLSliceNode* sliceNode = rep->GetSliceNode();
    vtkMatrix4x4* xyToRASMatrix = sliceNode->GetXYToRAS();
    displayToWorldTransform->SetMatrix(xyToRASMatrix);
    displayToWorldTransform->TransformPoint(inputPoint0_Display, inputPoint0_World);
    displayToWorldTransform->TransformPoint(inputPoint1_Display, inputPoint1_World);
  }

  double t = 0.0; // not used
  axisPlaneWorld->IntersectWithLine(inputPoint0_World, inputPoint1_World, t, outputIntersection_World);
  return true;
}

//----------------------------------------------------------------------
bool vtkMRMLInteractionWidget::GetClosestPointOnInteractionAxis(int type,
                                                                int index,
                                                                const double input_Display[2],
                                                                double outputClosestPoint_World[3])
{
  vtkMRMLInteractionWidgetRepresentation* rep = vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
  {
    return false;
  }

  double translationAxis_World[3] = { 0.0, 0.0, 0.0 };
  rep->GetInteractionHandleAxisWorld(type, index, translationAxis_World); // Axis of rotation
  double origin_World[3] = { 0.0, 0.0, 0.0 };
  rep->GetInteractionHandleOriginWorld(origin_World);

  double inputPoint0_World[3] = { 0.0, 0.0, 0.0 };
  double inputPoint1_World[3] = { 0.0, 0.0, 1.0 };
  if (!rep->GetSliceNode())
  {
    // 3D view
    vtkRenderer* renderer = rep->GetRenderer();
    vtkCamera* camera = renderer->GetActiveCamera();

    // Focal point position
    double cameraFP_World[4] = { 0.0, 0.0, 0.0, 0.0 };
    camera->GetFocalPoint(cameraFP_World);

    renderer->SetWorldPoint(cameraFP_World[0], cameraFP_World[1], cameraFP_World[2], cameraFP_World[3]);
    renderer->WorldToDisplay();
    double* displayCoords = renderer->GetDisplayPoint();
    double selectionZ = displayCoords[2];

    renderer->SetDisplayPoint(input_Display[0], input_Display[1], selectionZ);
    renderer->DisplayToWorld();

    double* input_World = renderer->GetWorldPoint();
    if (input_World[3] == 0.0)
    {
      vtkWarningMacro("Bad homogeneous coordinates");
      return false;
    }

    double pickPosition_World[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < 3; i++)
    {
      pickPosition_World[i] = input_World[i] / input_World[3];
    }

    double projectionVector_World[3] = { 0.0, 0.0, 0.0 };
    if (camera->GetParallelProjection())
    {
      camera->GetDirectionOfProjection(projectionVector_World);
      for (int i = 0; i < 3; i++)
      {
        inputPoint0_World[i] = pickPosition_World[i];
      }
    }
    else
    {
      // Camera position
      double cameraPosition_World[4] = { 0.0, 0.0, 0.0, 0.0 };
      camera->GetPosition(cameraPosition_World);

      //  Compute the ray endpoints.  The ray is along the line running from
      //  the camera position to the selection point, starting where this line
      //  intersects the front clipping plane, and terminating where this
      //  line intersects the back clipping plane.
      for (int i = 0; i < 3; i++)
      {
        inputPoint0_World[i] = cameraPosition_World[i];
        projectionVector_World[i] = pickPosition_World[i] - cameraPosition_World[i];
      }
    }
    vtkMath::Add(inputPoint0_World, projectionVector_World, inputPoint1_World);
  }
  else
  {
    // 2D view
    double inputPoint0_Display[3] = { input_Display[0], input_Display[1], 0.0 };
    double inputPoint1_Display[3] = { input_Display[0], input_Display[1], 1.0 };

    vtkNew<vtkTransform> displayToWorldTransform;
    vtkMRMLSliceNode* sliceNode = rep->GetSliceNode();
    vtkMatrix4x4* xyToRASMatrix = sliceNode->GetXYToRAS();
    displayToWorldTransform->SetMatrix(xyToRASMatrix);
    displayToWorldTransform->TransformPoint(inputPoint0_Display, inputPoint0_World);
    displayToWorldTransform->TransformPoint(inputPoint1_Display, inputPoint1_World);
  }

  double t1; // not used
  double t2; // not used
  double closestPointNotUsed[3] = { 0.0, 0.0, 0.0 };
  double translationVectorPoint[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(origin_World, translationAxis_World, translationVectorPoint);
  vtkLine::DistanceBetweenLines(origin_World,
                                translationVectorPoint,
                                inputPoint0_World,
                                inputPoint1_World,
                                outputClosestPoint_World,
                                closestPointNotUsed,
                                t1,
                                t2);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLInteractionWidget::GetInteractive()
{
  switch (this->WidgetState)
  {
    case WidgetStateTranslate:
    case WidgetStateScale:
    case WidgetStateUniformScale:
    case WidgetStateRotate:
      return true;
    default:
      return false;
  }
  return false;
}

//-------------------------------------------------------------------------
int vtkMRMLInteractionWidget::GetMouseCursor()
{
  if (this->WidgetState == WidgetStateIdle)
  {
    return VTK_CURSOR_DEFAULT;
  }
  else
  {
    return VTK_CURSOR_HAND;
  }
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidget::GetActiveComponentType()
{
  vtkSmartPointer<vtkMRMLInteractionWidgetRepresentation> rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  return rep->GetActiveComponentType();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidget::SetActiveComponentType(int type)
{
  vtkSmartPointer<vtkMRMLInteractionWidgetRepresentation> rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  rep->SetActiveComponentType(type);
}

//----------------------------------------------------------------------
int vtkMRMLInteractionWidget::GetActiveComponentIndex()
{
  vtkSmartPointer<vtkMRMLInteractionWidgetRepresentation> rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  return rep->GetActiveComponentIndex();
}

//----------------------------------------------------------------------
void vtkMRMLInteractionWidget::SetActiveComponentIndex(int index)
{
  vtkSmartPointer<vtkMRMLInteractionWidgetRepresentation> rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  rep->SetActiveComponentIndex(index);
}
