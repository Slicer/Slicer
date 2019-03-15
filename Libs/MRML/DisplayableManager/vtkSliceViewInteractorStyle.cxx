/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSliceViewInteractorStyle.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSliceViewInteractorStyle.h"

// MRML includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceLayerLogic.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include "vtkEvent.h"
#include "vtkGeneralTransform.h"
#include "vtkImageData.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkNew.h"

//STL includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkSliceViewInteractorStyle, SliceLogic, vtkMRMLSliceLogic);

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSliceViewInteractorStyle);

//----------------------------------------------------------------------------
vtkSliceViewInteractorStyle::vtkSliceViewInteractorStyle()
{
  this->FocusedDisplayableManager = nullptr;
  this->MouseMovedSinceButtonDown = false;
  this->EnableCursorUpdate = true;
  this->SliceLogic = nullptr;
}

//----------------------------------------------------------------------------
vtkSliceViewInteractorStyle::~vtkSliceViewInteractorStyle()
{
  this->SetSliceLogic(nullptr);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "\nSlice Logic:\n";
  if (this->SliceLogic)
    {
    this->SliceLogic->PrintSelf(os, indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnKeyPress()
{
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::KeyPressEvent))
    {
    return;
    }
  this->Superclass::OnKeyPress();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnKeyRelease()
{
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::KeyReleaseEvent))
    {
    return;
    }
  this->Superclass::OnKeyRelease();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnChar()
{
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::CharEvent))
    {
    return;
    }
  this->Superclass::OnChar();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseMove()
{
  this->MouseMovedSinceButtonDown = true;

  if (this->EnableCursorUpdate)
    {
    // Update the cursor position (show coordinates of current position in the data probe, etc.)
    vtkMRMLScene *scene = this->SliceLogic->GetMRMLScene();
    vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
    if (crosshairNode)
      {
      int eventPosition[2] = { 0 };
      this->GetInteractor()->GetEventPosition(eventPosition[0], eventPosition[1]);
      vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
      double xyz[3] = { 0.0 };
      vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetInteractor(),
        sliceNode, eventPosition[0], eventPosition[1], xyz);
      crosshairNode->SetCursorPositionXYZ(xyz, sliceNode);
      }
    }

  if (!this->ForwardInteractionEventToDisplayableManagers(vtkCommand::MouseMoveEvent))
    {
    return;
    }
  this->Superclass::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnRightButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::RightButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::RightButtonPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnRightButtonUp()
{
  if (!this->ForwardInteractionEventToDisplayableManagers(vtkCommand::RightButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::RightButtonReleaseEvent, nullptr);
    }
  if (!this->MouseMovedSinceButtonDown)
    {
    this->ForwardInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::RightButtonClickEvent);
    }
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMiddleButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (!this->ForwardInteractionEventToDisplayableManagers(vtkCommand::MiddleButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMiddleButtonUp()
{
  if (!this->ForwardInteractionEventToDisplayableManagers(vtkCommand::MiddleButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, nullptr);
    }
  if (!this->MouseMovedSinceButtonDown)
    {
    this->ForwardInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::MiddleButtonClickEvent);
    }
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnLeftButtonDown()
{
  this->MouseMovedSinceButtonDown = false;
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::LeftButtonPressEvent))
    {
    return;
    }
  this->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnLeftButtonUp()
{
  if (!this->ForwardInteractionEventToDisplayableManagers(vtkCommand::LeftButtonReleaseEvent))
    {
    this->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
    }
  if (!this->MouseMovedSinceButtonDown)
    {
    this->ForwardInteractionEventToDisplayableManagers(vtkMRMLInteractionEventData::LeftButtonClickEvent);
    }
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnEnter()
{
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::EnterEvent))
    {
    return;
    }
  this->Superclass::OnEnter();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnLeave()
{
  if (this->EnableCursorUpdate)
    {
    vtkMRMLScene *scene = this->SliceLogic->GetMRMLScene();
    vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
    if (crosshairNode)
      {
      crosshairNode->SetCursorPositionInvalid();
      }
    }
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::LeaveEvent))
    {
    return;
    }
  this->Superclass::OnLeave();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseWheelForward()
{
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::MouseWheelForwardEvent))
    {
    return;
    }
  this->Superclass::OnMouseWheelForward();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnMouseWheelBackward()
{
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::MouseWheelBackwardEvent))
    {
    return;
    }
  this->Superclass::OnMouseWheelBackward();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnExpose()
{
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::ExposeEvent))
    {
    return;
    }
  this->Superclass::OnExpose();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::OnConfigure()
{
  if (this->ForwardInteractionEventToDisplayableManagers(vtkCommand::ConfigureEvent))
    {
    return;
    }
  this->Superclass::OnConfigure();
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::SetDisplayableManagers(vtkMRMLDisplayableManagerGroup* displayableManagerGroup)
{
  this->DisplayableManagers = displayableManagerGroup;
}

//----------------------------------------------------------------------------
bool vtkSliceViewInteractorStyle::ForwardInteractionEventToDisplayableManagers(unsigned long event)
{
  if (!this->DisplayableManagers)
    {
    return false;
    }
  double canProcessEvent = false;
  double closestDistance2 = VTK_DOUBLE_MAX;
  vtkMRMLAbstractDisplayableManager* closestDisplayableManager = nullptr;
  int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();

  // Get display and world position
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);
  double displayPosition[4] =
    {
    static_cast<double>(displayPositionInt[0] - pokedRenderer->GetOrigin()[0]),
    static_cast<double>(displayPositionInt[1] - pokedRenderer->GetOrigin()[1]),
    0.0,
    1.0
    };
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  vtkMatrix4x4 * xyToRasMatrix = sliceNode->GetXYToRAS();
  double worldPosition[4] = { 0.0, 0.0, 0.0, 1.0 };
  xyToRasMatrix->MultiplyPoint(displayPosition, worldPosition);

  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(event);
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0], displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
  ed->SetWorldPosition(worldPosition);
  ed->SetAttributesFromInteractor(this->GetInteractor());

  // Find the most suitable displayable manager
  for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers; ++displayableManagerIndex)
    {
    vtkMRMLAbstractDisplayableManager* displayableManager = vtkMRMLAbstractDisplayableManager::SafeDownCast(
      this->DisplayableManagers->GetNthDisplayableManager(displayableManagerIndex));
    if (!displayableManager)
      {
      continue;
      }
    double distance2 = VTK_DOUBLE_MAX;
    if (displayableManager->CanProcessInteractionEvent(ed, distance2))
      {
      if (!canProcessEvent || (distance2 < closestDistance2))
        {
        canProcessEvent = true;
        closestDisplayableManager = displayableManager;
        closestDistance2 = distance2;
        }
      }
    }

  if (!canProcessEvent)
    {
    // none of the displayable managers can process the event, just ignore it
    return false;
    }

  // Notify displayable managers about focus change
  vtkMRMLAbstractDisplayableManager* oldFocusedDisplayableManager = this->FocusedDisplayableManager;
  if (oldFocusedDisplayableManager != closestDisplayableManager)
    {
    if (oldFocusedDisplayableManager != nullptr)
      {
      oldFocusedDisplayableManager->SetHasFocus(false);
      }
    this->FocusedDisplayableManager = closestDisplayableManager;
    if (closestDisplayableManager != nullptr)
      {
      closestDisplayableManager->SetHasFocus(true);
      }
    }

  // Process event with new displayable manager
  if (!this->FocusedDisplayableManager)
    {
    return false;
    }

  return this->FocusedDisplayableManager->ProcessInteractionEvent(ed);
}

//----------------------------------------------------------------------------
void vtkSliceViewInteractorStyle::SetActionEnabled(int actionsMask, bool enable /*=true*/)
{
  vtkMRMLCrosshairDisplayableManager* crosshairDisplayableManager = this->GetCrosshairDisplayableManager();
  if (!crosshairDisplayableManager)
    {
    vtkErrorMacro("vtkSliceViewInteractorStyle::SetActionEnabled failed: crosshair displayable manager not found");
    return;
    }

  int actionsEnabled = crosshairDisplayableManager->GetActionsEnabled();
  if (enable)
    {
    actionsEnabled |= actionsMask;
    }
  else
    {
    actionsEnabled  &= (~actionsMask);
    }
  crosshairDisplayableManager->SetActionsEnabled(actionsEnabled);
  this->EnableCursorUpdate = ((actionsEnabled & SetCursorPosition) != 0);
}

//----------------------------------------------------------------------------
bool vtkSliceViewInteractorStyle::GetActionEnabled(int actionsMask)
{
  vtkMRMLCrosshairDisplayableManager* crosshairDisplayableManager = this->GetCrosshairDisplayableManager();
  if (!crosshairDisplayableManager)
    {
    vtkErrorMacro("vtkSliceViewInteractorStyle::SetActionEnabled failed: crosshair displayable manager not found");
    return false;
    }

  int actionsEnabled = crosshairDisplayableManager->GetActionsEnabled();
  return (actionsEnabled & actionsMask) == actionsMask;
}

//----------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager* vtkSliceViewInteractorStyle::GetCrosshairDisplayableManager()
{
  int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();
  for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers; ++displayableManagerIndex)
    {
    vtkMRMLCrosshairDisplayableManager* displayableManager = vtkMRMLCrosshairDisplayableManager::SafeDownCast(
      this->DisplayableManagers->GetNthDisplayableManager(displayableManagerIndex));
    if (displayableManager)
      {
      return displayableManager;
      }
    }
  return nullptr;
}
