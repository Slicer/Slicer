/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

#include "vtkSlicerMarkupsWidget.h"

#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerMarkupsWidgetRepresentation.h"
#include "vtkSlicerMarkupsWidgetRepresentation2D.h"
#include "vtkSlicerMarkupsWidgetRepresentation3D.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkLine.h>
#include <vtkPlane.h>
#include <vtkPointPlacer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>

//----------------------------------------------------------------------
vtkSlicerMarkupsWidget::vtkSlicerMarkupsWidget()
{
  this->LastEventPosition[0] = 0.0;
  this->LastEventPosition[1] = 0.0;
  this->StartEventOffsetPosition[0] = 0.0;
  this->StartEventOffsetPosition[1] = 0.0;

  this->PreviewPointIndex = -1;

  // Place
  this->SetEventTranslation(
    WidgetStateDefine, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier, WidgetEventReserved);
  this->SetEventTranslation(
    WidgetStateDefine, vtkCommand::LeftButtonReleaseEvent, vtkEvent::NoModifier, WidgetEventControlPointPlace);
  this->SetEventTranslation(
    WidgetStateDefine, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetEventReserved);
  this->SetEventTranslation(
    WidgetStateDefine, vtkCommand::RightButtonReleaseEvent, vtkEvent::NoModifier, WidgetEventStopPlace);
  this->SetEventTranslation(
    WidgetStateDefine, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventStopPlace);

  // Manipulate
  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget,
                                        vtkCommand::LeftButtonPressEvent,
                                        vtkEvent::NoModifier,
                                        WidgetStateTranslateControlPoint,
                                        WidgetEventControlPointMoveStart,
                                        WidgetEventControlPointMoveEnd);
  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget,
                                        vtkCommand::MiddleButtonPressEvent,
                                        vtkEvent::NoModifier,
                                        WidgetStateTranslate,
                                        WidgetEventTranslateStart,
                                        WidgetEventTranslateEnd);
  this->SetKeyboardEventTranslation(
    WidgetStateOnWidget, vtkEvent::NoModifier, 115, 1, "s", WidgetEventControlPointSnapToSlice);
  this->SetKeyboardEventTranslation(WidgetStateOnWidget, vtkEvent::ShiftModifier, 127, 1, "Delete", WidgetEventReset);
  this->SetKeyboardEventTranslation(
    WidgetStateOnWidget, vtkEvent::NoModifier, 127, 1, "Delete", WidgetEventControlPointDelete);
  this->SetKeyboardEventTranslation(
    WidgetStateOnWidget, vtkEvent::NoModifier, 8, 1, "BackSpace", WidgetEventControlPointDelete);

  this->SetEventTranslation(WidgetStateOnWidget,
                            vtkMRMLInteractionEventData::LeftButtonClickEvent,
                            vtkEvent::NoModifier,
                            WidgetEventJumpCursor);
  this->SetEventTranslation(
    WidgetStateOnWidget, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventAction);

  this->SetEventTranslation(
    WidgetStateOnWidget, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetEventReserved);
  this->SetEventTranslation(
    WidgetStateOnWidget, vtkCommand::RightButtonReleaseEvent, vtkEvent::NoModifier, WidgetEventReserved);
  this->SetEventTranslation(
    WidgetStateOnWidget, vtkMRMLInteractionEventData::RightButtonClickEvent, vtkEvent::NoModifier, WidgetEventMenu);

  // Update active component
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(
    WidgetStateOnWidget, vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  // Allow AnyModifier when defining the markup position. This allows the markup preview to be continually updated, even
  // when using shift + mouse-move to change the slice positions.
  // We still do not allow shift+left click for placement however, so that the shift + left-click-and-drag interaction
  // can still be used to pan the slice.
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::MouseMoveEvent, vtkEvent::AnyModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateIdle, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
  this->SetEventTranslation(WidgetStateDefine, vtkCommand::Move3DEvent, vtkEvent::NoModifier, WidgetEventMouseMove);
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidget::~vtkSlicerMarkupsWidget() = default;

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessControlPointMoveStart(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateOnWidget)
  {
    return false;
  }
  int activeControlPoint = this->GetActiveControlPoint();
  if (activeControlPoint < 0)
  {
    return false;
  }
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return false;
  }
  // Do not reject this event if control point is locked
  // because then we would not receive the mouse release event
  // and so we could not process mouse clicks.
  this->SetWidgetState(WidgetStateTranslateControlPoint);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessControlPointInsert(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  // Can be implemented in derived classes
  return false;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessWidgetTranslateStart(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateOnWidget)
  {
    return false;
  }

  this->SetWidgetState(WidgetStateTranslate);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep || !markupsNode || !eventData)
  {
    return false;
  }

  int state = this->WidgetState;

  if (state == vtkSlicerMarkupsWidget::WidgetStateDefine)
  {
    const char* associatedNodeID = this->GetAssociatedNodeID(eventData);
    int positionPreviewState = vtkMRMLMarkupsNode::PositionPreview;
    this->UpdatePreviewPointIndex(eventData);
    if (PreviewPointIndex >= 0)
    {
      positionPreviewState = markupsNode->GetNthControlPointPositionStatus(PreviewPointIndex);
    }
    this->UpdatePreviewPoint(eventData, associatedNodeID, positionPreviewState);
  }
  else if (state == WidgetStateIdle || state == WidgetStateOnWidget)
  {
    // update state
    int foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
    int foundComponentIndex = -1;
    double closestDistance2 = 0.0;
    rep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2);
    if (foundComponentType == vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
      this->SetWidgetState(WidgetStateIdle);
    }
    else
    {
      this->SetWidgetState(WidgetStateOnWidget);
    }

    this->GetMarkupsDisplayNode()->SetActiveComponent(
      foundComponentType, foundComponentIndex, eventData->GetInteractionContextName());
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
    if (state == WidgetStateTranslateControlPoint)
    {
      this->TranslatePoint(eventPos);
    }
    else if (state == WidgetStateTranslate)
    {
      this->TranslateWidget(eventPos);
    }

    if (markupsNode->GetCurveClosed())
    {
      rep->UpdateCenterOfRotation();
    }

    this->LastEventPosition[0] = eventPos[0];
    this->LastEventPosition[1] = eventPos[1];
  }

  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
  {
    return false;
  }

  if ((this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateTranslateControlPoint
       && this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateTranslate)
      || !this->WidgetRep)
  {
    return false;
  }

  this->SetWidgetState(WidgetStateOnWidget);

  this->EndWidgetInteraction();

  // only claim this as processed if the mouse was moved (this lets the event interpreted as button click)
  bool processedEvent = eventData->GetMouseMovedSinceButtonDown();
  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessWidgetReset(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return false;
  }
  markupsNode->GetScene()->SaveStateForUndo();
  markupsNode->RemoveAllControlPoints();
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessControlPointSnapToSlice(vtkMRMLInteractionEventData* eventData)
{
  vtkSlicerMarkupsWidgetRepresentation2D* rep2d = vtkSlicerMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  if (!rep2d)
  {
    return false;
  }

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return false;
  }

  markupsNode->GetScene()->SaveStateForUndo();
  double eventPos[2]{
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1]),
  };
  this->StartWidgetInteraction(eventData);
  this->TranslatePoint(eventPos, true);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessControlPointDelete(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  if (this->WidgetState != WidgetStateDefine && this->WidgetState != WidgetStateOnWidget)
  {
    return false;
  }

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
  {
    return false;
  }
  std::vector<int> controlPointsToDelete;
  if (this->WidgetState == WidgetStateDefine)
  {
    controlPointsToDelete.push_back(markupsNode->GetNumberOfControlPoints() - 2);
  }
  else if (this->WidgetState == WidgetStateOnWidget)
  {
    markupsDisplayNode->GetActiveControlPoints(controlPointsToDelete);
  }
  if (controlPointsToDelete.empty())
  {
    return false;
  }

  markupsNode->GetScene()->SaveStateForUndo();

  for (std::vector<int>::iterator cpIt = controlPointsToDelete.begin(); cpIt != controlPointsToDelete.end(); ++cpIt)
  {
    int controlPointToDelete = (*cpIt);
    if (controlPointToDelete < 0 || controlPointToDelete >= markupsNode->GetNumberOfControlPoints())
    {
      continue;
    }
    markupsNode->RemoveNthControlPoint(controlPointToDelete);
  }

  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessWidgetJumpCursor(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  if (this->WidgetState != WidgetStateOnWidget)
  {
    return false;
  }

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
  {
    return false;
  }

  int componentIndex = markupsDisplayNode->GetActiveComponentIndex();
  ;
  int componentType = markupsDisplayNode->GetActiveComponentType();

  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  if (componentType == vtkMRMLMarkupsDisplayNode::ComponentControlPoint)
  {
    std::vector<int> activeControlPointIndices;
    markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);

    if (!activeControlPointIndices.empty())
    {
      componentIndex = activeControlPointIndices[0];
    }
    if (componentIndex < 0 || componentIndex >= markupsNode->GetNumberOfControlPoints())
    {
      return false;
    }
  }

  markupsNode->GetScene()->SaveStateForUndo();

  vtkNew<vtkMRMLInteractionEventData> jumpToPointEventData;
  jumpToPointEventData->SetType(vtkMRMLMarkupsDisplayNode::JumpToPointEvent);
  jumpToPointEventData->SetComponentType(componentType);
  jumpToPointEventData->SetComponentIndex(componentIndex);
  jumpToPointEventData->SetViewNode(this->WidgetRep->GetViewNode());
  markupsDisplayNode->InvokeEvent(vtkMRMLMarkupsDisplayNode::JumpToPointEvent, jumpToPointEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ConvertDisplayPositionToWorld(const int displayPos[2],
                                                           double worldPos[3],
                                                           double worldOrientationMatrix[9],
                                                           double* refWorldPos /*=nullptr*/)
{
  vtkSlicerMarkupsWidgetRepresentation2D* rep2d = vtkSlicerMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerMarkupsWidgetRepresentation3D* rep3d = vtkSlicerMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);
  double doubleDisplayPos[3] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0 };
  if (rep2d)
  {
    // 2D view
    rep2d->GetSliceToWorldCoordinates(doubleDisplayPos, worldPos);
    return true;
  }
  else if (rep3d)
  {
    // 3D view
    bool preferPickOnSurface = true;
    if (refWorldPos != nullptr)
    {
      // If reference position is provided then we may use that instead of picking on visible surface.
      vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
      if (markupsDisplayNode)
      {
        preferPickOnSurface =
          (markupsDisplayNode->GetSnapMode() == vtkMRMLMarkupsDisplayNode::SnapModeToVisibleSurface);
      }
    }
    if (preferPickOnSurface)
    {
      // SnapModeToVisibleSurface
      // Try to pick on surface and pick on camera plane if nothing is found.
      if (rep3d->AccuratePick(displayPos[0], displayPos[1], worldPos))
      {
        return true;
      }
      if (refWorldPos)
      {
        // Reference position is available (most likely, moving the point).
        return (rep3d->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, doubleDisplayPos, refWorldPos, worldPos, worldOrientationMatrix));
      }
    }
    else
    {
      // SnapModeUnconstrained
      // Move the point relative to reference position, not restricted to surfaces if possible.
      if (refWorldPos)
      {
        // Reference position is available (most likely, moving the point).
        return (rep3d->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, doubleDisplayPos, refWorldPos, worldPos, worldOrientationMatrix));
      }
      else
      {
        // Reference position is unavailable (e.g., not moving of an existing point but first placement)
        // Even if the constraining on the surface is no preferred, it is still better to
        // place it on a visible surface in 3D views rather on the .
        if (rep3d->AccuratePick(displayPos[0], displayPos[1], worldPos))
        {
          return true;
        }
      }
    }
    // Last resort: place a point on the camera plane
    // (no reference position is available and no surface is visible there)
    return (rep3d->GetPointPlacer()->ComputeWorldPosition(
      this->Renderer, doubleDisplayPos, worldPos, worldOrientationMatrix));
  }
  return false;
}

// -------------------------------------------------------------------------
void vtkSlicerMarkupsWidget::UpdatePreviewPointIndex(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return;
  }
  int numberOfControlPoints = markupsNode->GetNumberOfControlPoints();
  for (int i = 0; i < numberOfControlPoints; i++)
  {
    int pointStatus = markupsNode->GetNthControlPointPositionStatus(i);
    if (pointStatus == vtkMRMLMarkupsNode::PositionPreview)
    {
      this->PreviewPointIndex = i;
      return;
    }
  }
  // if no preview points found, set to -1
  this->PreviewPointIndex = -1;
}

//-------------------------------------------------------------------------
void vtkSlicerMarkupsWidget::UpdatePreviewPoint(vtkMRMLInteractionEventData* eventData,
                                                const char* associatedNodeID,
                                                int positionStatus)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return;
  }

  // Get accurate world position
  double accurateWorldPos[3] = { 0.0 };
  double accurateWorldOrientationMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (eventData->IsWorldPositionValid() && eventData->IsWorldPositionAccurate())
  {
    eventData->GetWorldPosition(accurateWorldPos);

    double worldOrientationQuaternion[4] = { 0.0 };
    eventData->GetWorldOrientation(worldOrientationQuaternion);
    vtkMRMLMarkupsNode::ConvertOrientationWXYZToMatrix(worldOrientationQuaternion, accurateWorldOrientationMatrix);
  }
  else if (eventData->IsDisplayPositionValid())
  {
    int displayPos[2] = { 0 };
    eventData->GetDisplayPosition(displayPos);
    if (!this->ConvertDisplayPositionToWorld(displayPos, accurateWorldPos, accurateWorldOrientationMatrix))
    {
      eventData->GetWorldPosition(accurateWorldPos);
    }
  }
  eventData->SetWorldPosition(accurateWorldPos);

  // Add/update control point position and orientation

  const char* viewNodeID = nullptr;
  if (this->WidgetRep && this->WidgetRep->GetViewNode())
  {
    viewNodeID = this->WidgetRep->GetViewNode()->GetID();
  }

  this->UpdatePreviewPointIndex(eventData);
  int updatePointID = this->PreviewPointIndex;
  if (updatePointID < 0)
  {
    updatePointID = markupsNode->GetControlPointPlacementStartIndex();
  }

  this->PreviewPointIndex = this->GetMarkupsDisplayNode()->UpdateActiveControlPointWorld(
    updatePointID, eventData, accurateWorldOrientationMatrix, viewNodeID, associatedNodeID, positionStatus);
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::RemovePreviewPoint()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return false;
  }
  if (this->PreviewPointIndex < 0)
  {
    // no preview point
    return false;
  }
  int previewIndex = this->PreviewPointIndex;
  int status = markupsNode->GetNthControlPointPositionStatus(previewIndex);
  if (status == vtkMRMLMarkupsNode::PositionPreview)
  {
    if (markupsNode->GetNthControlPointAutoCreated(previewIndex))
    {
      markupsNode->RemoveNthControlPoint(previewIndex);
      markupsNode->GetMarkupsDisplayNode()->SetActiveControlPoint(-1);
    }
    else
    {
      markupsNode->UnsetNthControlPointPosition(previewIndex);
    }
  }
  this->PreviewPointIndex = -1;
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PreviewPointIndex: " << this->PreviewPointIndex << endl;
}

//-----------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
  {
    // If this event is not recognized then give a chance to process it as a click event.
    return this->CanProcessButtonClickEvent(eventData, distance2);
  }
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
  {
    return false;
  }

  // If we are placing markups or dragging the mouse then we interact everywhere
  if (this->WidgetState == WidgetStateDefine || this->WidgetState == WidgetStateTranslateControlPoint
      || this->WidgetState == WidgetStateTranslate)
  {
    distance2 = 0.0;
    return true;
  }

  int foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  int foundComponentIndex = -1;
  double closestDistance2 = 0.0;
  rep->CanInteract(eventData, foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType == vtkMRMLMarkupsDisplayNode::ComponentNone)
  {
    return false;
  }
  distance2 = closestDistance2;
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != WidgetStateOnWidget)
  {
    return false;
  }

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
  {
    return false;
  }

  vtkNew<vtkMRMLInteractionEventData> menuEventData;
  menuEventData->SetType(vtkMRMLDisplayNode::MenuEvent);
  menuEventData->SetComponentType(
    markupsDisplayNode->GetActiveComponentType()); // TODO: This will always pass the active component for the mouse
  menuEventData->SetComponentIndex(markupsDisplayNode->GetActiveComponentIndex());
  menuEventData->SetViewNode(this->WidgetRep->GetViewNode());

  // Copy display position
  if (eventData->IsDisplayPositionValid())
  {
    menuEventData->SetDisplayPosition(eventData->GetDisplayPosition());
  }

  // Copy/compute world position
  double worldPos[3] = { 0.0 };
  if (eventData->IsWorldPositionValid())
  {
    eventData->GetWorldPosition(worldPos);
    menuEventData->SetWorldPosition(worldPos, eventData->IsWorldPositionAccurate());
  }
  else if (eventData->IsDisplayPositionValid())
  {
    double worldOrientationMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    int displayPos[2] = { 0 };
    eventData->GetDisplayPosition(displayPos);
    if (this->ConvertDisplayPositionToWorld(displayPos, worldPos, worldOrientationMatrix))
    {
      menuEventData->SetWorldPosition(worldPos);
    }
  }

  markupsDisplayNode->InvokeEvent(vtkMRMLDisplayNode::MenuEvent, menuEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessWidgetAction(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != WidgetStateOnWidget)
  {
    return false;
  }
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
  {
    return false;
  }
  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int controlPointIndex = -1;
  if (!activeControlPointIndices.empty())
  {
    controlPointIndex = activeControlPointIndices[0];
  }
  if (controlPointIndex < 0 || controlPointIndex >= markupsNode->GetNumberOfControlPoints())
  {
    return false;
  }
  markupsNode->GetScene()->SaveStateForUndo();

  // Convert widget action to display node event
  unsigned long displayNodeEvent = vtkMRMLMarkupsDisplayNode::ActionEvent;
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  switch (widgetEvent)
  {
    case WidgetEventCustomAction1:
      displayNodeEvent = vtkMRMLMarkupsDisplayNode::CustomActionEvent1;
      break;
    case WidgetEventCustomAction2:
      displayNodeEvent = vtkMRMLMarkupsDisplayNode::CustomActionEvent2;
      break;
    case WidgetEventCustomAction3:
      displayNodeEvent = vtkMRMLMarkupsDisplayNode::CustomActionEvent3;
      break;
    case WidgetEventCustomAction4:
      displayNodeEvent = vtkMRMLMarkupsDisplayNode::CustomActionEvent4;
      break;
    case WidgetEventCustomAction5:
      displayNodeEvent = vtkMRMLMarkupsDisplayNode::CustomActionEvent5;
      break;
    case WidgetEventCustomAction6:
      displayNodeEvent = vtkMRMLMarkupsDisplayNode::CustomActionEvent6;
      break;
    case WidgetEventAction:
    default:
      displayNodeEvent = vtkMRMLMarkupsDisplayNode::ActionEvent;
      break;
  }

  vtkNew<vtkMRMLInteractionEventData> actionEventData;
  actionEventData->SetType(displayNodeEvent);
  actionEventData->SetComponentType(vtkMRMLMarkupsDisplayNode::ComponentControlPoint);
  actionEventData->SetComponentIndex(controlPointIndex);
  actionEventData->SetViewNode(this->WidgetRep->GetViewNode());
  markupsDisplayNode->InvokeEvent(displayNodeEvent, actionEventData);
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessWidgetStopPlace(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return false;
  }
  markupsNode->Modified();
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  if (this->ApplicationLogic)
  {
    this->ApplicationLogic->PauseRender();
  }

  bool processedEvent = false;
  switch (widgetEvent)
  {
    case WidgetEventControlPointPlace:
      processedEvent = this->PlacePoint(eventData);
      break;
    case WidgetEventStopPlace:
      // cancel point placement
      this->GetInteractionNode()->SwitchToViewTransformMode();
      processedEvent = ProcessWidgetStopPlace(eventData);
      break;
    case WidgetEventMouseMove:
      processedEvent = ProcessMouseMove(eventData);
      break;
    case WidgetEventMenu:
      processedEvent = ProcessWidgetMenu(eventData);
      break;
    case WidgetEventAction:
    case WidgetEventCustomAction1:
    case WidgetEventCustomAction2:
    case WidgetEventCustomAction3:
      processedEvent = ProcessWidgetAction(eventData);
      break;
    case WidgetEventControlPointSnapToSlice:
      processedEvent = ProcessControlPointSnapToSlice(eventData);
      break;
    case WidgetEventControlPointDelete:
      processedEvent = ProcessControlPointDelete(eventData);
      break;
    case WidgetEventControlPointMoveStart:
      processedEvent = ProcessControlPointMoveStart(eventData);
      break;
    case WidgetEventControlPointInsert:
      processedEvent = ProcessControlPointInsert(eventData);
      break;
    case WidgetEventControlPointMoveEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventTranslateStart:
      processedEvent = ProcessWidgetTranslateStart(eventData);
      break;
    case WidgetEventTranslateEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventReset:
      processedEvent = ProcessWidgetReset(eventData);
      break;
    case WidgetEventJumpCursor:
      processedEvent = ProcessWidgetJumpCursor(eventData);
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
void vtkSlicerMarkupsWidget::Leave(vtkMRMLInteractionEventData* eventData)
{
  this->RemovePreviewPoint();

  // Ensure that EndInteractionEvent is invoked, even if interrupted by an unexpected event
  if (this->WidgetState == vtkSlicerMarkupsWidget::WidgetStateTranslateControlPoint
      || this->WidgetState == vtkSlicerMarkupsWidget::WidgetStateTranslate)
  {
    this->EndWidgetInteraction();
  }

  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (markupsDisplayNode)
  {
    std::string interactionContext("");
    if (eventData)
    {
      interactionContext = eventData->GetInteractionContextName();
    }
    markupsDisplayNode->SetActiveComponent(vtkMRMLMarkupsDisplayNode::ComponentNone, -1, interactionContext);
  }
  Superclass::Leave(eventData);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidget::StartWidgetInteraction(vtkMRMLInteractionEventData* eventData)
{
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
  {
    return;
  }
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
  {
    return;
  }

  markupsNode->GetScene()->SaveStateForUndo();

  double startEventPos[2]{ static_cast<double>(eventData->GetDisplayPosition()[0]),
                           static_cast<double>(eventData->GetDisplayPosition()[1]) };

  // save the cursor position
  this->LastEventPosition[0] = startEventPos[0];
  this->LastEventPosition[1] = startEventPos[1];

  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int activeControlPointIndex = -1;
  if (!activeControlPointIndices.empty())
  {
    activeControlPointIndex = activeControlPointIndices[0];
  }
  if (activeControlPointIndex >= 0 || activeControlPointIndex < markupsNode->GetNumberOfControlPoints())
  {
    // How far is this in pixels from the position of this widget?
    // Maintain this during interaction such as translating (don't
    // force center of widget to snap to mouse position)
    double pos[2] = { 0.0 };
    if (rep->GetNthControlPointDisplayPosition(activeControlPointIndex, pos))
    {
      // save offset
      this->StartEventOffsetPosition[0] = startEventPos[0] - pos[0];
      this->StartEventOffsetPosition[1] = startEventPos[1] - pos[1];
    }
    // AddControlPoint will fire modified events anyway, so we temporarily disable events
    // to add a new point with a minimum number of events.
    bool wasDisabled = markupsNode->GetDisableModifiedEvent();
    markupsNode->DisableModifiedEventOn();
    const char* layoutName = nullptr;
    if (rep->GetViewNode() && rep->GetViewNode()->GetLayoutName())
    {
      layoutName = rep->GetViewNode()->GetLayoutName();
    }
    markupsNode->SetAttribute("Markups.MovingInSliceView", layoutName ? layoutName : "");
    std::ostringstream controlPointIndexStr;
    controlPointIndexStr << activeControlPointIndex;
    markupsNode->SetAttribute("Markups.MovingMarkupIndex", controlPointIndexStr.str().c_str());
    markupsNode->SetDisableModifiedEvent(wasDisabled);
    markupsNode->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointStartInteractionEvent);
  }
  else
  {
    // Picking line or something else - not handled in this base class
    this->StartEventOffsetPosition[0] = 0;
    this->StartEventOffsetPosition[1] = 0;
  }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidget::EndWidgetInteraction()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return;
  }
  markupsNode->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointEndInteractionEvent);
  // AddControlPoint will fire modified events anyway, so we temporarily disable events
  // to add a new point with a minimum number of events.
  bool wasDisabled = markupsNode->GetDisableModifiedEvent();
  markupsNode->DisableModifiedEventOn();
  markupsNode->SetAttribute("Markups.MovingInSliceView", "");
  markupsNode->SetAttribute("Markups.MovingMarkupIndex", "");
  markupsNode->SetDisableModifiedEvent(wasDisabled);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidget::TranslatePoint(double eventPos[2], bool snapToSlice /* = false*/)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
  {
    return;
  }

  // Use first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int activeControlPointIndex = -1;
  if (!activeControlPointIndices.empty())
  {
    activeControlPointIndex = activeControlPointIndices[0];
  }
  if (activeControlPointIndex < 0 || activeControlPointIndex >= markupsNode->GetNumberOfControlPoints())
  {
    return;
  }

  if (markupsNode->GetNthControlPointLocked(activeControlPointIndex))
  {
    // point is locked, do not translate
    return;
  }

  eventPos[0] -= this->StartEventOffsetPosition[0];
  eventPos[1] -= this->StartEventOffsetPosition[1];

  double oldWorldPos[3] = { 0.0 };
  markupsNode->GetNthControlPointPositionWorld(activeControlPointIndex, oldWorldPos);

  // Get accurate world position
  int displayPos[2] = { int(eventPos[0] + 0.5), int(eventPos[1] + 0.5) };
  double worldPos[3] = { 0.0 };
  double worldOrientationMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (!this->ConvertDisplayPositionToWorld(displayPos, worldPos, worldOrientationMatrix, oldWorldPos))
  {
    vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
    if (!rep
        || !rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos, oldWorldPos, worldPos, worldOrientationMatrix))
    {
      return;
    }
  }

  vtkSlicerMarkupsWidgetRepresentation2D* rep2d = vtkSlicerMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  if (rep2d && markupsDisplayNode->GetSliceProjection() && !snapToSlice)
  {
    double doubleDisplayPos[2], oldWorldPos2[3];
    rep2d->GetWorldToSliceCoordinates(oldWorldPos, doubleDisplayPos);
    rep2d->GetSliceToWorldCoordinates(doubleDisplayPos, oldWorldPos2);
    double worldPosProjDiff[3] = {
      oldWorldPos[0] - oldWorldPos2[0],
      oldWorldPos[1] - oldWorldPos2[1],
      oldWorldPos[2] - oldWorldPos2[2],
    };

    worldPos[0] += worldPosProjDiff[0];
    worldPos[1] += worldPosProjDiff[1];
    worldPos[2] += worldPosProjDiff[2];
  }

  markupsNode->SetNthControlPointPositionWorld(activeControlPointIndex, worldPos);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsWidget::TranslateWidget(double eventPos[2])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return;
  }

  double lastEventPos_World[3] = { 0.0 };
  double eventPos_World[3] = { 0.0 };
  double orientation_World[9] = { 0.0 };

  vtkSlicerMarkupsWidgetRepresentation2D* rep2d = vtkSlicerMarkupsWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerMarkupsWidgetRepresentation3D* rep3d = vtkSlicerMarkupsWidgetRepresentation3D::SafeDownCast(this->WidgetRep);
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
    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, this->LastEventPosition, lastEventPos_World, orientation_World))
    {
      return;
    }
    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos, lastEventPos_World, eventPos_World, orientation_World))
    {
      return;
    }
  }

  double translationVector_World[3];
  translationVector_World[0] = eventPos_World[0] - lastEventPos_World[0];
  translationVector_World[1] = eventPos_World[1] - lastEventPos_World[1];
  translationVector_World[2] = eventPos_World[2] - lastEventPos_World[2];

  vtkNew<vtkTransform> translationTransform;
  translationTransform->Translate(translationVector_World);

  vtkNew<vtkPoints> transformedPoints_World;
  transformedPoints_World->SetNumberOfPoints(markupsNode->GetNumberOfControlPoints());
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
  {
    double currentControlPointPosition_World[3] = { 0.0 };
    markupsNode->GetNthControlPointPositionWorld(i, currentControlPointPosition_World);
    if (markupsNode->GetNthControlPointLocked(i))
    {
      transformedPoints_World->SetPoint(i, currentControlPointPosition_World);
    }
    else
    {
      double newControlPointPosition_World[3] = { 0.0 };
      translationTransform->TransformPoint(currentControlPointPosition_World, newControlPointPosition_World);
      transformedPoints_World->SetPoint(i, newControlPointPosition_World);
    }
  }
  bool setUndefinedPoints = false;
  markupsNode->SetControlPointPositionsWorld(transformedPoints_World, setUndefinedPoints);
}

//----------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkSlicerMarkupsWidget::GetMarkupsNode()
{
  vtkSlicerMarkupsWidgetRepresentation* widgetRep = vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!widgetRep)
  {
    return nullptr;
  }
  return widgetRep->GetMarkupsNode();
}

//----------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* vtkSlicerMarkupsWidget::GetMarkupsDisplayNode()
{
  vtkSlicerMarkupsWidgetRepresentation* widgetRep = vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!widgetRep)
  {
    return nullptr;
  }
  return widgetRep->GetMarkupsDisplayNode();
}

//----------------------------------------------------------------------
vtkSlicerMarkupsWidgetRepresentation* vtkSlicerMarkupsWidget::GetMarkupsRepresentation()
{
  return vtkSlicerMarkupsWidgetRepresentation::SafeDownCast(this->WidgetRep);
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidget::GetActiveControlPoint()
{
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
  {
    return -1;
  }
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = rep->GetMarkupsDisplayNode();
  if (!markupsDisplayNode)
  {
    return -1;
  }

  // Return first active control point for jumping //TODO: Have an 'even more active' point concept
  std::vector<int> activeControlPointIndices;
  markupsDisplayNode->GetActiveControlPoints(activeControlPointIndices);
  int activeControlPointIndex = -1;
  if (!activeControlPointIndices.empty())
  {
    activeControlPointIndex = activeControlPointIndices[0];
  }
  return activeControlPointIndex;
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::IsAnyControlPointLocked()
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return false;
  }
  // If any node is locked return
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
  {
    if (markupsNode->GetNthControlPointLocked(i))
    {
      return true;
    }
  }
  return false;
}

//-------------------------------------------------------------------------
int vtkSlicerMarkupsWidget::AddPointFromWorldCoordinate(const double worldCoordinates[3])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return -1;
  }

  int addedControlPoint = -1;
  if (this->PreviewPointIndex)
  {
    // convert point preview to final point
    addedControlPoint = this->PreviewPointIndex;
    markupsNode->SetNthControlPointPositionWorld(addedControlPoint, worldCoordinates);
    this->PreviewPointIndex = -1;
  }
  else
  {
    addedControlPoint = this->GetMarkupsNode()->AddControlPointWorld(vtkVector3d(worldCoordinates));
  }

  if (addedControlPoint >= 0 && this->GetMarkupsDisplayNode())
  {
    this->GetMarkupsDisplayNode()->SetActiveControlPoint(addedControlPoint);
  }

  return addedControlPoint;
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsWidget::AddNodeOnWidget(const int displayPos[2])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return 0;
  }
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
  {
    return 0;
  }
  double displayPosDouble[3] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0 };
  double worldPos[3] = { 0.0 };
  double worldOrient[9] = { 0.0 };
  if (!rep->GetPointPlacer()->ComputeWorldPosition(this->Renderer, displayPosDouble, worldPos, worldOrient))
  {
    return 0;
  }

  int idx = -1;
  double closestPosWorld[3];
  if (!rep->FindClosestPointOnWidget(displayPos, closestPosWorld, &idx))
  {
    return 0;
  }

  if (!rep->GetPointPlacer()->ComputeWorldPosition(
        this->Renderer, displayPosDouble, closestPosWorld, worldPos, worldOrient))
  {
    return 0;
  }

  // Add a new point at this position
  vtkMRMLMarkupsNode::ControlPoint* controlPoint = new vtkMRMLMarkupsNode::ControlPoint;
  markupsNode->TransformPointFromWorld(worldPos, controlPoint->Position);

  markupsNode->InsertControlPoint(controlPoint, idx);
  markupsNode->SetNthControlPointOrientationMatrixWorld(idx, worldOrient);
  return 1;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::GetInteractive()
{
  switch (this->WidgetState)
  {
    case WidgetStateTranslateControlPoint:
    case WidgetStateTranslate:
      return true;
    default:
      return false;
  }
}

//-------------------------------------------------------------------------
int vtkSlicerMarkupsWidget::GetMouseCursor()
{
  if (this->WidgetState == WidgetStateIdle
      || this->WidgetState == WidgetStateDefine) // default cursor shape is the "place" cursor
  {
    return VTK_CURSOR_DEFAULT;
  }
  else
  {
    return VTK_CURSOR_HAND;
  }
}

//----------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::IsPointPreviewed()
{
  return this->PreviewPointIndex >= 0;
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsWidget::PlacePoint(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
  {
    return false;
  }
  // save for undo and add the node to the scene after any reset of the
  // interaction node so that don't end up back in place mode
  markupsNode->GetScene()->SaveStateForUndo();

  // Add/update preview point
  const char* associatedNodeID = this->GetAssociatedNodeID(eventData);
  this->UpdatePreviewPoint(eventData, associatedNodeID, vtkMRMLMarkupsNode::PositionDefined);
  int controlPointIndex = this->PreviewPointIndex;
  // Convert the preview point to a proper control point
  this->PreviewPointIndex = -1;

  // if this was a one time place, go back to view transform mode
  vtkMRMLInteractionNode* interactionNode = this->GetInteractionNode();

  if (interactionNode)
  {
    bool hasRequiredPoints = markupsNode->GetRequiredNumberOfControlPoints() > 0;
    bool hasRequiredPointNumber =
      markupsNode->GetNumberOfControlPoints() >= markupsNode->GetRequiredNumberOfControlPoints();
    bool requiredPointsReached =
      hasRequiredPoints && hasRequiredPointNumber && !bool(markupsNode->GetNumberOfUndefinedControlPoints() > 0);
    bool lockedPointsReached =
      markupsNode->GetFixedNumberOfControlPoints() && !bool(markupsNode->GetNumberOfUndefinedControlPoints() > 0);

    if ((requiredPointsReached && !interactionNode->GetPlaceModePersistence())
        || (!hasRequiredPoints && !interactionNode->GetPlaceModePersistence()) || lockedPointsReached)
    {
      vtkDebugMacro("End of one time place, place mode persistence = " << interactionNode->GetPlaceModePersistence());
      interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);

      // The mouse is over the control point and we are not in place mode anymore
      if (this->GetMarkupsDisplayNode())
      {
        this->GetMarkupsDisplayNode()->SetActiveControlPoint(controlPointIndex);
      }
      this->WidgetState = WidgetStateOnWidget;
    }
  }

  bool success = (controlPointIndex >= 0);
  return success;
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsWidget::GetActiveComponentType()
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!displayNode)
  {
    return vtkMRMLMarkupsDisplayNode::ComponentNone;
  }
  return displayNode->GetActiveComponentType();
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsWidget::GetActiveComponentIndex()
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  if (!displayNode)
  {
    return -1;
  }
  return displayNode->GetActiveComponentIndex();
}

//-----------------------------------------------------------------------------
vtkMRMLSelectionNode* vtkSlicerMarkupsWidget::selectionNode()
{
  return vtkMRMLSelectionNode::SafeDownCast(
    this->GetMarkupsNode()->GetScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
}
