/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMRMLSliceViewInteractorStyle.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkMRMLSliceViewInteractorStyle.h"

// MRML includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScalarBarDisplayableManager.h"
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
vtkCxxSetObjectMacro(vtkMRMLSliceViewInteractorStyle, SliceLogic, vtkMRMLSliceLogic);

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLSliceViewInteractorStyle);

//----------------------------------------------------------------------------
vtkMRMLSliceViewInteractorStyle::vtkMRMLSliceViewInteractorStyle()
{
  this->EnableCursorUpdate = true;
  this->SliceLogic = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLSliceViewInteractorStyle::~vtkMRMLSliceViewInteractorStyle()
{
  this->SetSliceLogic(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "\nSlice Logic:\n";
  if (this->SliceLogic)
    {
    this->SliceLogic->PrintSelf(os, indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceViewInteractorStyle::DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData)
{
  if (!this->DisplayableManagers || !inputEventData)
    {
    //this->SetMouseCursor(VTK_CURSOR_DEFAULT);
    return false;
    }

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
  ed->SetType(inputEventData->GetType());
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0], displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
  ed->SetWorldPosition(worldPosition);
  ed->SetAttributesFromInteractor(this->GetInteractor());

  // Update cursor position
  if (this->EnableCursorUpdate)
    {
    if (inputEventData->GetType() == vtkCommand::MouseMoveEvent)
      {
      // Update the cursor position (show coordinates of current position in the data probe, etc.)
      vtkMRMLScene* scene = this->SliceLogic->GetMRMLScene();
      vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
      if (crosshairNode)
        {
        double xyz[3] = { 0.0 };
        vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetInteractor(),
          sliceNode, displayPositionInt[0], displayPositionInt[1], xyz);
        crosshairNode->SetCursorPositionXYZ(xyz, sliceNode);
        }
      }
    else if (inputEventData->GetType() == vtkCommand::LeaveEvent)
      {
      vtkMRMLScene* scene = this->SliceLogic->GetMRMLScene();
      vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
      if (crosshairNode)
        {
        crosshairNode->SetCursorPositionInvalid();
        }
      }
    }

  return this->DelegateInteractionEventDataToDisplayableManagers(ed);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceViewInteractorStyle::SetActionEnabled(int actionsMask, bool enable /*=true*/)
{
  this->EnableCursorUpdate = ((actionsMask & SetCursorPosition) != 0);

  vtkMRMLCrosshairDisplayableManager* crosshairDisplayableManager = this->GetCrosshairDisplayableManager();
  if (crosshairDisplayableManager)
    {
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
    }

  vtkMRMLScalarBarDisplayableManager* scalarBarDisplayableManager = this->GetScalarBarDisplayableManager();
  if (scalarBarDisplayableManager)
    {
    scalarBarDisplayableManager->SetAdjustBackgroundWindowLevelEnabled((actionsMask & AdjustWindowLevelBackground) != 0);
    scalarBarDisplayableManager->SetAdjustForegroundWindowLevelEnabled((actionsMask & AdjustWindowLevelForeground) != 0);
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLSliceViewInteractorStyle::GetActionEnabled(int actionsMask)
{
  int actionsEnabled = 0;

  if (this->EnableCursorUpdate)
    {
    actionsEnabled |= SetCursorPosition;
    }

  vtkMRMLCrosshairDisplayableManager* crosshairDisplayableManager = this->GetCrosshairDisplayableManager();
  if (crosshairDisplayableManager)
    {
    actionsEnabled |= crosshairDisplayableManager->GetActionsEnabled();
    }

   vtkMRMLScalarBarDisplayableManager* scalarBarDisplayableManager = this->GetScalarBarDisplayableManager();
  if (scalarBarDisplayableManager)
    {
    if (scalarBarDisplayableManager->GetAdjustBackgroundWindowLevelEnabled())
      {
      actionsEnabled |= AdjustWindowLevelBackground;
      }
    if (scalarBarDisplayableManager->GetAdjustForegroundWindowLevelEnabled())
      {
      actionsEnabled |= AdjustWindowLevelForeground;
      }
    }

  return (actionsEnabled & actionsMask) == actionsMask;
}

//----------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager* vtkMRMLSliceViewInteractorStyle::GetCrosshairDisplayableManager()
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

//----------------------------------------------------------------------------
vtkMRMLScalarBarDisplayableManager* vtkMRMLSliceViewInteractorStyle::GetScalarBarDisplayableManager()
{
  int numberOfDisplayableManagers = this->DisplayableManagers->GetDisplayableManagerCount();
  for (int displayableManagerIndex = 0; displayableManagerIndex < numberOfDisplayableManagers; ++displayableManagerIndex)
    {
    vtkMRMLScalarBarDisplayableManager* displayableManager = vtkMRMLScalarBarDisplayableManager::SafeDownCast(
      this->DisplayableManagers->GetNthDisplayableManager(displayableManagerIndex));
    if (displayableManager)
      {
      return displayableManager;
      }
    }
  return nullptr;
}
