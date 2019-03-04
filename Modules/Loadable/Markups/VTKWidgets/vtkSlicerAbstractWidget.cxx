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

#include "vtkSlicerAbstractWidget.h"

#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkSlicerAbstractWidgetRepresentation.h"
#include "vtkSlicerAbstractWidgetRepresentation2D.h"
#include "vtkSlicerAbstractWidgetRepresentation3D.h"

// VTK includes
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkPointPlacer.h"
#include "vtkRenderer.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkSphereSource.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"
#include "vtkRenderWindow.h"

//----------------------------------------------------------------------
vtkSlicerAbstractWidget::vtkSlicerAbstractWidget()
{
  this->ApplicationLogic = NULL;
  this->Renderer = NULL;
  this->EventTranslator = vtkSmartPointer<vtkWidgetEventTranslator>::New();

  this->WidgetRep = NULL;

  this->WidgetState = vtkSlicerAbstractWidget::Idle;

  this->MousePressedSinceMarkupPlace = true;

  this->PreviewPointIndex = -1;

  this->SetEventTranslation(vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier, WidgetControlPointMoveStart);
  this->SetEventTranslation(vtkCommand::LeftButtonReleaseEvent, vtkEvent::AnyModifier, WidgetControlPointMoveEnd);

  this->SetEventTranslation(vtkCommand::MiddleButtonPressEvent, vtkEvent::NoModifier, WidgetTranslateStart);
  this->SetEventTranslation(vtkCommand::MiddleButtonReleaseEvent, vtkEvent::AnyModifier, WidgetTranslateEnd);

  //this->SetEventTranslation(vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetScaleStart);
  //this->SetEventTranslation(vtkCommand::RightButtonReleaseEvent, vtkEvent::AnyModifier, WidgetScaleEnd);

  this->SetEventTranslation(vtkCommand::MouseMoveEvent, vtkEvent::AnyModifier, WidgetMouseMove);

  this->SetKeyboardEventTranslation(vtkEvent::ShiftModifier, 127, 1, "Delete", WidgetReset);
  this->SetKeyboardEventTranslation(vtkEvent::NoModifier, 127, 1, "Delete", WidgetControlPointDelete);
  this->SetKeyboardEventTranslation(vtkEvent::NoModifier, 8, 1, "BackSpace", WidgetControlPointDelete);

  //this->SetEventTranslation(vtkMRMLInteractionEventData::LeftButtonClickEvent, vtkEvent::NoModifier, WidgetPick);
  this->SetEventTranslation(vtkMRMLInteractionEventData::LeftButtonClickEvent, vtkEvent::NoModifier, WidgetJumpCursor);

  this->SetEventTranslation(vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::AnyModifier, WidgetAction);
}

//----------------------------------------------------------------------
vtkSlicerAbstractWidget::~vtkSlicerAbstractWidget()
{
  this->SetRenderer(nullptr);
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::SetEventTranslation(unsigned long interactionEvent, int modifiers, unsigned long widgetEvent)
{
  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(interactionEvent);
  ed->SetModifiers(modifiers);
  this->EventTranslator->SetTranslation(interactionEvent, ed, widgetEvent);
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::SetKeyboardEventTranslation(
  int modifier, char keyCode,
  int repeatCount, const char* keySym, unsigned long widgetEvent)
{
  this->EventTranslator->SetTranslation(vtkCommand::KeyPressEvent, modifier, keyCode,
    repeatCount, keySym, widgetEvent);
}

//-------------------------------------------------------------------------
void vtkSlicerAbstractWidget::SetRepresentation(vtkSlicerAbstractWidgetRepresentation *rep)
{
  if (rep == this->WidgetRep)
    {
    // no change
    return;
    }

  if (this->WidgetRep)
    {
    if (this->Renderer)
      {
      this->WidgetRep->SetRenderer(nullptr);
      this->Renderer->RemoveViewProp(this->WidgetRep);
      }
    }

  this->WidgetRep = rep;

  if (this->Renderer != nullptr && this->WidgetRep != nullptr)
    {
    this->WidgetRep->SetRenderer(this->Renderer);
    this->Renderer->AddViewProp(this->WidgetRep);
    }
}

//-------------------------------------------------------------------------
vtkSlicerAbstractWidgetRepresentation* vtkSlicerAbstractWidget::GetRepresentation()
{
  return this->WidgetRep;
}

//-------------------------------------------------------------------------
void vtkSlicerAbstractWidget::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData/*=NULL*/)
{
  if (!this->WidgetRep)
    {
    return;
    }

  this->WidgetRep->UpdateFromMRML(caller, event, callData);
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessControlPointMove(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkSlicerAbstractWidget::OnWidget)
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
  if (markupsNode->GetNthControlPointLocked(activeControlPoint))
    {
    return false;
    }
  this->SetWidgetState(TranslateControlPoint);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessControlPointInsert(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  // Can be implemented in derived classes
  return false;
}

//----------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessWidgetRotate(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkSlicerAbstractWidget::OnWidget || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(Rotate);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessWidgetScale(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkSlicerAbstractWidget::OnWidget || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(Scale);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessWidgetTranslate(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState != vtkSlicerAbstractWidget::OnWidget || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(Translate);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!this->WidgetRep || !markupsNode || !eventData)
    {
    return false;
    }

  int state = this->WidgetState;

  if (state == vtkSlicerAbstractWidget::Define)
    {
    const char* associatedNodeID = this->GetAssociatedNodeID(eventData);
    this->UpdatePreviewPoint(eventData->GetDisplayPosition(), eventData->GetWorldPosition(), associatedNodeID);
    }
  else if (state == Idle || state == OnWidget)
    {
    // update state
    const int* displayPosition = eventData->GetDisplayPosition();
    const double* worldPosition = eventData->GetWorldPosition();
    int foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
    int foundComponentIndex = -1;
    double closestDistance2 = 0.0;
    this->WidgetRep->CanInteract(displayPosition, worldPosition, foundComponentType, foundComponentIndex, closestDistance2);
    if (foundComponentType == vtkMRMLMarkupsDisplayNode::ComponentNone)
      {
      this->SetWidgetState(Idle);
      }
    else
      {
      this->SetWidgetState(OnWidget);
      this->GetMarkupsDisplayNode()->SetActiveComponent(foundComponentType, foundComponentIndex);
      }
    }
  else
    {
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
    if (state == TranslateControlPoint)
      {
      this->TranslatePoint(eventPos);
      }
    else if (state == Translate)
      {
      this->TranslateWidget(eventPos);
      }
    else if (state == Scale)
      {
      this->ScaleWidget(eventPos);
      }
    else if (state == Rotate)
      {
      this->RotateWidget(eventPos);
      }

    if (markupsNode->GetCurveClosed())
      {
      this->WidgetRep->UpdateCenter();
      }

    this->LastEventPosition[0] = eventPos[0];
    this->LastEventPosition[1] = eventPos[1];
    }

  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  if (!this->WidgetRep)
    {
    return false;
    }

  if ((this->WidgetState != vtkSlicerAbstractWidget::TranslateControlPoint
    && this->WidgetState != vtkSlicerAbstractWidget::Translate
    && this->WidgetState != vtkSlicerAbstractWidget::Scale
    && this->WidgetState != vtkSlicerAbstractWidget::Rotate
    ) || !this->WidgetRep)
    {
    return false;
    }

  this->SetWidgetState(OnWidget);

  this->EndWidgetInteraction();
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessWidgetReset(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return false;
    }
  markupsNode->RemoveAllControlPoints();
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessControlPointDelete(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  if (this->WidgetState != Define && this->WidgetState != OnWidget)
    {
    return false;
    }

  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return false;
    }
  int controlPointToDelete = -1;
  if (this->WidgetState == Define)
    {
    controlPointToDelete = markupsNode->GetNumberOfControlPoints() - 2;
    }
  else if (this->WidgetState == OnWidget)
    {
    controlPointToDelete = markupsDisplayNode->GetActiveControlPoint();
    }

  if (controlPointToDelete < 0 || controlPointToDelete >= markupsNode->GetNumberOfControlPoints())
    {
    return false;
    }

  markupsNode->RemoveNthControlPoint(controlPointToDelete);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessWidgetJumpCursor(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  if (this->WidgetState != OnWidget || !this->MousePressedSinceMarkupPlace)
    {
    return false;
    }
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return false;
    }
  int controlPointIndex = markupsDisplayNode->GetActiveControlPoint();
  if (controlPointIndex < 0 || controlPointIndex >= markupsNode->GetNumberOfControlPoints())
    {
    return false;
    }
  vtkNew<vtkMRMLInteractionEventData> jumpToPointEventData;
  jumpToPointEventData->SetType(vtkMRMLMarkupsDisplayNode::JumpToPointEvent);
  jumpToPointEventData->SetComponentType(vtkMRMLMarkupsDisplayNode::ComponentControlPoint);
  jumpToPointEventData->SetComponentIndex(controlPointIndex);
  jumpToPointEventData->SetViewNode(this->WidgetRep->GetViewNode());
  markupsDisplayNode->InvokeEvent(vtkMRMLMarkupsDisplayNode::JumpToPointEvent, jumpToPointEventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ConvertDisplayPositionToWorld(const int displayPos[2],
  double worldPos[3], double vtkNotUsed(worldOrientationMatrix)[9], double* refWorldPos/*=nullptr*/)
{
  vtkSlicerAbstractWidgetRepresentation2D* rep2d = vtkSlicerAbstractWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerAbstractWidgetRepresentation3D* rep3d = vtkSlicerAbstractWidgetRepresentation3D::SafeDownCast(this->WidgetRep);
  double doubleDisplayPos[3] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0 };
  if (rep2d)
    {
    // 2D view
    rep2d->GetSliceToWorldCoordinates(doubleDisplayPos, worldPos);
    return true;
    }
  else
    {
    // 3D view
    if (rep3d->AccuratePick(displayPos[0], displayPos[1], worldPos))
      {
      return true;
      }
    // try default picker method
    double worldOrientationMatrix[9] = { 0.0 };
    bool success = false;
    if (refWorldPos)
      {
      // reference position is available
      success = this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
        doubleDisplayPos, refWorldPos, worldPos, worldOrientationMatrix);
      }
    else
      {
      // reference position is unavailable
      success = this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
        doubleDisplayPos, worldPos, worldOrientationMatrix);
      }
    if (success)
      {
      return true;
      }
    }
  return false;
}

//-------------------------------------------------------------------------
void vtkSlicerAbstractWidget::UpdatePreviewPoint(const int displayPos[2], const double worldPos[3], const char* associatedNodeID)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  // Get accurate world position
  double accurateWorldPos[3] = { 0.0 };
  double accurateWorldOrientationMatrix[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  if (!this->ConvertDisplayPositionToWorld(displayPos, accurateWorldPos, accurateWorldOrientationMatrix))
    {
    accurateWorldPos[0] = worldPos[0];
    accurateWorldPos[1] = worldPos[1];
    accurateWorldPos[2] = worldPos[2];
    }

  // Add/update control point position and orientation

  const char* viewNodeID = nullptr;
  if (this->WidgetRep && this->WidgetRep->GetViewNode())
    {
    viewNodeID = this->WidgetRep->GetViewNode()->GetID();
    }

  this->PreviewPointIndex = this->GetMarkupsDisplayNode()->UpdateActiveControlPointWorld(
    this->PreviewPointIndex, accurateWorldPos, accurateWorldOrientationMatrix, viewNodeID, associatedNodeID);
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::RemovePreviewPoint()
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

  markupsNode->RemoveNthControlPoint(this->PreviewPointIndex);
  this->PreviewPointIndex = -1;
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  os << indent << "WidgetState: " << this->WidgetState << endl;
  os << indent << "PreviewPointIndex: " << this->PreviewPointIndex << endl;
}

//-----------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (this->WidgetState == Define)
    {
    unsigned long eventid = eventData->GetType();

    // Widget interactions are quite different in Define (point placement) mode.
    // We process these events without event translator, but in the future we may add
    // state-specific event translations.
    if (eventid == vtkCommand::LeftButtonReleaseEvent)
      {
      this->PlacePoint(eventData);
      return true;
      }
    else if (eventid == vtkCommand::RightButtonReleaseEvent)
      {
      // cancel point placement
      this->GetInteractionNode()->SwitchToViewTransformMode();
      return true;
      }
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = false;
  switch (widgetEvent)
  {
  case WidgetMouseMove:
    processedEvent = ProcessMouseMove(eventData);
    break;
  case WidgetControlPointDelete:
    processedEvent = ProcessControlPointDelete(eventData);
    break;
  case WidgetControlPointMoveStart:
    processedEvent = ProcessControlPointMove(eventData);
    break;
  case WidgetControlPointInsert:
    processedEvent = ProcessControlPointInsert(eventData);
    break;
  case WidgetControlPointMoveEnd:
    processedEvent = ProcessEndMouseDrag(eventData);
    break;
  case WidgetTranslateStart:
    processedEvent = ProcessWidgetTranslate(eventData);
    break;
  case WidgetTranslateEnd:
    processedEvent = ProcessEndMouseDrag(eventData);
    break;
  case WidgetRotateStart:
    processedEvent = ProcessWidgetRotate(eventData);
    break;
  case WidgetRotateEnd:
    processedEvent = ProcessEndMouseDrag(eventData);
    break;
  case WidgetScaleStart:
    processedEvent = ProcessWidgetScale(eventData);
    break;
  case WidgetScaleEnd:
    processedEvent = ProcessEndMouseDrag(eventData);
    break;
  case WidgetReset:
    processedEvent = ProcessWidgetReset(eventData);
    break;
  case WidgetJumpCursor:
    processedEvent = ProcessWidgetJumpCursor(eventData);
    break;
  }

  return processedEvent;
}

//-----------------------------------------------------------------------------
unsigned long vtkSlicerAbstractWidget::TranslateInteractionEventToWidgetEvent(
  vtkMRMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = vtkWidgetEvent::NoEvent;

  if (!eventData)
    {
    return widgetEvent;
    }

  if (eventData->GetType() == vtkCommand::KeyPressEvent)
    {
    // We package keypress events information into event data,
    // unpack it for the event translator
    int modifier = eventData->GetModifiers();
    char keyCode = eventData->GetKeyCode();
    int repeatCount = eventData->GetKeyRepeatCount();
    const char* keySym = nullptr;
    if (!eventData->GetKeySym().empty())
      {
      keySym = eventData->GetKeySym().c_str();
      }

    // If neither the ctrl nor the shift keys are pressed, give
    // NoModifier a preference over AnyModifer.
    if (modifier == vtkEvent::AnyModifier)
      {
      widgetEvent = this->EventTranslator->GetTranslation(vtkCommand::KeyPressEvent,
        vtkEvent::NoModifier, keyCode, repeatCount, keySym);
      }
    if (widgetEvent == vtkWidgetEvent::NoEvent)
      {
      widgetEvent = this->EventTranslator->GetTranslation(vtkCommand::KeyPressEvent,
        vtkEvent::NoModifier, keyCode, repeatCount, keySym);
      }
    }
  else
    {
    widgetEvent = this->EventTranslator->GetTranslation(eventData->GetType(), eventData);
    }

  return widgetEvent;
}


//-----------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == vtkWidgetEvent::NoEvent)
    {
    return false;
    }
  if (!this->GetRepresentation())
    {
    return false;
    }

  int eventid = eventData->GetType();
  if (eventid == vtkCommand::LeftButtonPressEvent
    || eventid == vtkCommand::MiddleButtonPressEvent
    || eventid == vtkCommand::RightButtonPressEvent)
    {
    this->MousePressedSinceMarkupPlace = true;
    }

  // If we are currently translating then we interact everywhere
  if (this->WidgetState == TranslateControlPoint
    || this->WidgetState == Translate
    || this->WidgetState == Rotate
    || this->WidgetState == Scale)
    {
    distance2 = 0.0;
    return true;
    }

  vtkMRMLInteractionEventData* interactionEventData = vtkMRMLInteractionEventData::SafeDownCast(eventData);
  if (!interactionEventData)
  {
    // only 3D event types are supported for now
    return false;
  }

  int foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  int foundComponentIndex = -1;
  double closestDistance2 = 0.0;
  this->GetRepresentation()->CanInteract(interactionEventData->GetDisplayPosition(), interactionEventData->GetWorldPosition(),
    foundComponentType, foundComponentIndex, closestDistance2);
  if (foundComponentType == vtkMRMLMarkupsDisplayNode::ComponentNone)
    {
    return false;
    }
  distance2 = closestDistance2;
  return true;
}

//-----------------------------------------------------------------------------
void vtkSlicerAbstractWidget::Leave()
{
  this->RemovePreviewPoint();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (markupsDisplayNode)
  {
    markupsDisplayNode->SetActiveComponent(vtkMRMLMarkupsDisplayNode::ComponentNone, -1);
  }
  this->SetWidgetState(Idle);
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::StartWidgetInteraction(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return;
    }
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return;
    }
  int activeControlPointIndex = markupsDisplayNode->GetActiveControlPoint();
  if (activeControlPointIndex < 0 || activeControlPointIndex >= markupsNode->GetNumberOfControlPoints())
    {
    return;
    }

  //this->GrabFocus(this->EventCallbackCommand);
  //this->StartInteraction();
  double startEventPos[2]
    {
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1])
    };

  // How far is this in pixels from the position of this widget?
  // Maintain this during interaction such as translating (don't
  // force center of widget to snap to mouse position)

  // GetActiveNode position
  double pos[2] = { 0.0 };
  if (this->WidgetRep->GetNthNodeDisplayPosition(activeControlPointIndex, pos))
    {
    // save offset
    this->StartEventOffsetPosition[0] = startEventPos[0] - pos[0];
    this->StartEventOffsetPosition[1] = startEventPos[1] - pos[1];
    }
  else
    {
    this->StartEventOffsetPosition[0] = 0;
    this->StartEventOffsetPosition[1] = 0;
    }

  // save also the cursor pos
  this->LastEventPosition[0] = startEventPos[0];
  this->LastEventPosition[1] = startEventPos[1];
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::EndWidgetInteraction()
{
  //this->ReleaseFocus();
  //this->EndInteraction();
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::TranslatePoint(double eventPos[2])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return;
    }
  int activeControlPointIndex = markupsDisplayNode->GetActiveControlPoint();
  if (activeControlPointIndex < 0 || activeControlPointIndex >= markupsNode->GetNumberOfControlPoints())
    {
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
    if (!this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer, eventPos, oldWorldPos, worldPos, worldOrientationMatrix))
      {
      return;
      }
    }

  markupsNode->SetNthControlPointPositionWorldFromArray(activeControlPointIndex, worldPos);
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::TranslateWidget(double eventPos[2])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  double ref[3] = { 0. };
  double worldPos[3], worldOrient[9];

  vtkSlicerAbstractWidgetRepresentation2D* rep2d = vtkSlicerAbstractWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
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
  else
    {
    // 3D view
    double displayPos[2] = { 0. };

    displayPos[0] = this->LastEventPosition[0];
    displayPos[1] = this->LastEventPosition[1];

    if (!this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      eventPos, ref, worldPos,
      worldOrient))
      {
      return;
      }
    ref[0] = worldPos[0];
    ref[1] = worldPos[1];
    ref[2] = worldPos[2];

    displayPos[0] = eventPos[0];
    displayPos[1] = eventPos[1];

    if (!this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      displayPos, ref, worldPos,
      worldOrient))
      {
      return;
      }
    }


  double vector[3];
  vector[0] = worldPos[0] - ref[0];
  vector[1] = worldPos[1] - ref[1];
  vector[2] = worldPos[2] - ref[2];

  int wasModified = markupsNode->StartModify();
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    if (markupsNode->GetNthControlPointLocked(i))
      {
      continue;
      }

    markupsNode->GetNthControlPointPositionWorld(i, ref);
    for (int j = 0; j < 3; j++)
      {
      worldPos[j] = ref[j] + vector[j];
      }
    markupsNode->SetNthControlPointPositionWorldFromArray(i, worldPos);
    }
  markupsNode->EndModify(wasModified);
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::ScaleWidget(double eventPos[2])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  double ref[3] = { 0. };
  double worldPos[3], worldOrient[9];

  double center[3];
  this->WidgetRep->GetTransformationReferencePoint(center);

  vtkSlicerAbstractWidgetRepresentation2D* rep2d = vtkSlicerAbstractWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    double slicePos[3] = { 0. };
    slicePos[0] = this->LastEventPosition[0];
    slicePos[1] = this->LastEventPosition[1];
    rep2d->GetSliceToWorldCoordinates(slicePos, ref);

    slicePos[0] = eventPos[0];
    slicePos[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(slicePos, worldPos);
    }
  else
    {
    double displayPos[2] = { 0. };
    displayPos[0] = this->LastEventPosition[0];
    displayPos[1] = this->LastEventPosition[1];
    if (this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      displayPos, ref, worldPos,
      worldOrient))
      {
      for (int i = 0; i < 3; i++)
        {
        ref[i] = worldPos[i];
        }
      }
    else
      {
      return;
      }
    displayPos[0] = eventPos[0];
    displayPos[1] = eventPos[1];

    if (!this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      displayPos, ref, worldPos,
      worldOrient))
      {
      return;
      }

    }

  double r2 = vtkMath::Distance2BetweenPoints(ref, center);
  double d2 = vtkMath::Distance2BetweenPoints(worldPos, center);
  if (d2 < 0.0000001)
    {
    return;
    }

  double ratio = sqrt(d2 / r2);

  int wasModified = markupsNode->StartModify();
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    if (markupsNode->GetNthControlPointLocked(i))
      {
      continue;
      }

    markupsNode->GetNthControlPointPositionWorld(i, ref);
    for (int j = 0; j < 3; j++)
      {
      worldPos[j] = center[j] + ratio * (ref[j] - center[j]);
      }

    markupsNode->SetNthControlPointPositionWorldFromArray(i, worldPos);
    }
  markupsNode->EndModify(wasModified);
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::RotateWidget(double eventPos[2])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  double ref[3] = { 0. };
  double lastWorldPos[3] = { 0. };
  double worldPos[3], worldOrient[9];

  double center[3];
  this->WidgetRep->GetTransformationReferencePoint(center);

  vtkSlicerAbstractWidgetRepresentation2D* rep2d = vtkSlicerAbstractWidgetRepresentation2D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    double slicePos[3] = { 0. };
    slicePos[0] = this->LastEventPosition[0];
    slicePos[1] = this->LastEventPosition[1];

    rep2d->GetSliceToWorldCoordinates(slicePos, lastWorldPos);

    slicePos[0] = eventPos[0];
    slicePos[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(slicePos, worldPos);
    }
  else
    {
    double displayPos[2] = { 0. };

    displayPos[0] = this->LastEventPosition[0];
    displayPos[1] = this->LastEventPosition[1];

    if (this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      displayPos, ref, lastWorldPos,
      worldOrient))
      {
      for (int i = 0; i < 3; i++)
        {
        ref[i] = worldPos[i];
        }
      }
    else
      {
      return;
      }

    displayPos[0] = eventPos[0];
    displayPos[1] = eventPos[1];

    if (!this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      displayPos, ref, worldPos,
      worldOrient))
      {
      return;
      }
    }


  double d2 = vtkMath::Distance2BetweenPoints(worldPos, center);
  if (d2 < 0.0000001)
    {
    return;
    }

  for (int i = 0; i < 3; i++)
    {
    lastWorldPos[i] -= center[i];
    worldPos[i] -= center[i];
    }
  double angle = -vtkMath::DegreesFromRadians
  (vtkMath::AngleBetweenVectors(lastWorldPos, worldPos));

  int wasModified = markupsNode->StartModify();
  for (int i = 0; i < markupsNode->GetNumberOfControlPoints(); i++)
    {
    markupsNode->GetNthControlPointPositionWorld(i, ref);
    for (int j = 0; j < 3; j++)
      {
      ref[j] -= center[j];
      }
    vtkNew<vtkTransform> RotateTransform;
    RotateTransform->RotateY(angle);
    RotateTransform->TransformPoint(ref, worldPos);

    for (int j = 0; j < 3; j++)
      {
      worldPos[j] += center[j];
      }

    markupsNode->SetNthControlPointPositionWorldFromArray(i, worldPos);
    }
  markupsNode->EndModify(wasModified);
}

//----------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkSlicerAbstractWidget::GetMarkupsNode()
{
  if (!this->WidgetRep)
    {
    return nullptr;
    }
  return this->WidgetRep->GetMarkupsNode();
}

//----------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* vtkSlicerAbstractWidget::GetMarkupsDisplayNode()
{
  if (!this->WidgetRep)
    {
    return nullptr;
    }
  return this->WidgetRep->GetMarkupsDisplayNode();
}

//----------------------------------------------------------------------
int vtkSlicerAbstractWidget::GetActiveControlPoint()
{
  if (!this->WidgetRep)
    {
    return -1;
    }
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->WidgetRep->GetMarkupsDisplayNode();
  if (!markupsDisplayNode)
    {
    return -1;
    }
  return markupsDisplayNode->GetActiveControlPoint();
}

//----------------------------------------------------------------------
bool vtkSlicerAbstractWidget::IsAnyControlPointLocked()
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
int vtkSlicerAbstractWidget::AddPointFromWorldCoordinate(const double worldCoordinates[3])
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
    markupsNode->SetNthControlPointPositionWorldFromArray(addedControlPoint, worldCoordinates);
    this->PreviewPointIndex = -1;
    }
  else
    {
    addedControlPoint = this->GetMarkupsNode()->AddControlPointWorld(vtkVector3d(worldCoordinates));
    }

  if (addedControlPoint>=0 && this->GetMarkupsDisplayNode())
    {
    this->GetMarkupsDisplayNode()->SetActiveControlPoint(addedControlPoint);
    }

  return addedControlPoint;
}

//----------------------------------------------------------------------
int vtkSlicerAbstractWidget::AddNodeOnWidget(const int displayPos[2])
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  if (!markupsNode)
    {
    return 0;
    }
  double displayPosDouble[3] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0 };
  double worldPos[3] = { 0.0 };
  double worldOrient[9] = { 0.0 };
  if (!this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer, displayPosDouble, worldPos, worldOrient))
    {
    return 0;
    }

  int idx = -1;
  double closestPosWorld[3];
  if (!this->WidgetRep->FindClosestPointOnWidget(displayPos, closestPosWorld, &idx))
    {
    return 0;
    }

  if (!this->WidgetRep->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
    displayPosDouble, closestPosWorld, worldPos, worldOrient))
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
int vtkSlicerAbstractWidget::GetCursor()
{
  switch (this->WidgetState)
    {
    case OnWidget:
    case TranslateControlPoint:
    case Translate:
    case Scale:
    case Rotate:
      return VTK_CURSOR_HAND;
    default:
      return VTK_CURSOR_DEFAULT;
    }
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::GetGrabFocus()
{
  // we need to grab focus when interactively dragging points
  return this->GetInteractive();
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::GetInteractive()
{
  switch (this->WidgetState)
    {
    case TranslateControlPoint:
    case Translate:
    case Scale:
    case Rotate:
      return true;
    default:
      return false;
    }
}

//-------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::GetNeedToRender()
{
  if (!this->WidgetRep)
    {
    return false;
    }
  return this->WidgetRep->GetNeedToRender();
}

//-------------------------------------------------------------------------
void vtkSlicerAbstractWidget::NeedToRenderOff()
{
  if (!this->WidgetRep)
    {
    return;
    }
  this->WidgetRep->NeedToRenderOff();
}

//----------------------------------------------------------------------
void vtkSlicerAbstractWidget::SetRenderer(vtkRenderer* renderer)
{
  if (renderer == this->Renderer)
    {
    return;
    }

  if (this->Renderer != nullptr && this->WidgetRep != nullptr)
    {
    this->Renderer->RemoveViewProp(this->WidgetRep);
    }

  this->Renderer = renderer;

  if (this->WidgetRep != nullptr && this->Renderer != nullptr)
    {
    this->WidgetRep->SetRenderer(this->Renderer);
    this->Renderer->AddViewProp(this->WidgetRep);
    }
}


//----------------------------------------------------------------------
bool vtkSlicerAbstractWidget::IsPointPreviewed()
{
  return this->PreviewPointIndex >= 0;
}

//---------------------------------------------------------------------------
const char* vtkSlicerAbstractWidget::GetAssociatedNodeID(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  if (!this->WidgetRep)
    {
    return nullptr;
    }
  // is there a volume in the background?
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(this->WidgetRep->GetViewNode());
  if (!sliceNode)
    {
    // this only works for slice views for now
    return nullptr;
    }
  // find the slice composite node in the scene with the matching layout name
  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    return nullptr;
    }
  vtkMRMLSliceLogic *sliceLogic = mrmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode(sliceNode);
  if (!sliceCompositeNode)
    {
    return nullptr;
    }
  if (sliceCompositeNode->GetBackgroundVolumeID())
    {
    return sliceCompositeNode->GetBackgroundVolumeID();
    }
  else if (sliceCompositeNode->GetForegroundVolumeID())
    {
    return sliceCompositeNode->GetForegroundVolumeID();
    }
  else if (sliceCompositeNode->GetLabelVolumeID())
    {
    return sliceCompositeNode->GetLabelVolumeID();
    }
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkSlicerAbstractWidget::SetMRMLApplicationLogic(vtkMRMLApplicationLogic* applicationLogic)
{
  this->ApplicationLogic = applicationLogic;
}

//---------------------------------------------------------------------------
vtkMRMLApplicationLogic* vtkSlicerAbstractWidget::GetMRMLApplicationLogic()
{
  return this->ApplicationLogic;
}

//---------------------------------------------------------------------------
bool vtkSlicerAbstractWidget::PlacePoint(vtkMRMLInteractionEventData* eventData)
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
  this->UpdatePreviewPoint(eventData->GetDisplayPosition(), eventData->GetWorldPosition(), associatedNodeID);
  int controlPointIndex = this->PreviewPointIndex;
  // Convert the preview point to a proper control point
  this->PreviewPointIndex = -1;

  // if this was a one time place, go back to view transform mode
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode && !interactionNode->GetPlaceModePersistence()
    && markupsNode->GetNumberOfControlPoints() >= markupsNode->GetRequiredNumberOfControlPoints())
    {
    vtkDebugMacro("End of one time place, place mode persistence = " << interactionNode->GetPlaceModePersistence());
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);

    // The mouse is over the control point and we are not in place mode anymore
    if (this->GetMarkupsDisplayNode())
      {
      this->GetMarkupsDisplayNode()->SetActiveControlPoint(controlPointIndex);
      }
    this->WidgetState = OnWidget;
    }

  bool success = (controlPointIndex >= 0);
  if (success)
    {
    this->MousePressedSinceMarkupPlace = false;
    }

  return success;
}

//---------------------------------------------------------------------------
vtkMRMLInteractionNode* vtkSlicerAbstractWidget::GetInteractionNode()
{
  if (!this->WidgetRep)
    {
    return nullptr;
    }
  vtkMRMLAbstractViewNode* viewNode = this->WidgetRep->GetViewNode();
  if (!viewNode)
    {
    return nullptr;
    }
  return viewNode->GetInteractionNode();
}
