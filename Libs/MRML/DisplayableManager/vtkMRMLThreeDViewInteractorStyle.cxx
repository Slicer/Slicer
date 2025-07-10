/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMRMLThreeDViewInteractorStyle.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or https://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMRMLThreeDViewInteractorStyle.h"

// MRML includes
#include "vtkMRMLCameraDisplayableManager.h"
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkEvent.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkVolumeCollection.h>
#include <vtkVolumePicker.h>
#include <vtkWorldPointPicker.h>

vtkStandardNewMacro(vtkMRMLThreeDViewInteractorStyle);

//----------------------------------------------------------------------------
vtkMRMLThreeDViewInteractorStyle::vtkMRMLThreeDViewInteractorStyle()
{
  this->CameraNode = nullptr;
  this->AccuratePicker = vtkSmartPointer<vtkCellPicker>::New();
  this->AccuratePicker->SetTolerance(.005);
  this->QuickPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
  this->QuickVolumePicker = vtkSmartPointer<vtkVolumePicker>::New();
  this->QuickVolumePicker->SetPickFromList(true); // will only pick volumes
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewInteractorStyle::~vtkMRMLThreeDViewInteractorStyle()
{
  this->SetCameraNode(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewInteractorStyle::SetDisplayableManagers(vtkMRMLDisplayableManagerGroup* displayableManagers)
{
  if (displayableManagers == this->DisplayableManagers)
  {
    return;
  }

  this->Superclass::SetDisplayableManagers(displayableManagers);

  // Observe ActiveCameraChangedEvent
  vtkMRMLAbstractDisplayableManager* cameraDisplayableManager =
    this->DisplayableManagers->GetDisplayableManagerByClassName("vtkMRMLCameraDisplayableManager");
  if (cameraDisplayableManager)
  {
    // Listen for ActiveCameraChangedEvent to detect when the camera displayable manager
    // sets its camera node.
    // See vtkMRMLThreeDViewInteractorStyle::ProcessDisplayableManagerEvents for details on how the event
    // is handled.
    // A simpler approach would be to directly store a reference to the camera in the view node.
    // See https://github.com/Slicer/Slicer/issues/7333
    cameraDisplayableManager->AddObserver(vtkMRMLCameraDisplayableManager::ActiveCameraChangedEvent,
                                          this->DisplayableManagerCallbackCommand);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewInteractorStyle::OnLeave()
{
  if (this->GetCameraNode() == nullptr || this->GetCameraNode()->GetScene() == nullptr)
  {
    // interactor is not initialized
    return;
  }
  vtkMRMLScene* scene = this->GetCameraNode()->GetScene();
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
  if (crosshairNode)
  {
    crosshairNode->SetCursorPositionInvalid();
  }
}

//----------------------------------------------------------------------------
bool vtkMRMLThreeDViewInteractorStyle::DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData)
{
  if (!inputEventData)
  {
    return false;
  }
  // Get display and world position
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);
  vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->GetInteractor()->GetInteractorStyle());
  interactorStyle->SetCurrentRenderer(pokedRenderer);
  if (!pokedRenderer)
  {
    // can happen during application shutdown
    return false;
  }

  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(inputEventData->GetType());
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0],
                                      displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
  double worldPosition[4] = { 0.0, 0.0, 0.0, 1.0 };
  if (this->QuickPick(displayPositionInt[0], displayPositionInt[1], worldPosition))
  {
    // set "inaccurate" world position
    ed->SetWorldPosition(worldPosition, false);

    // update the cursor position on mouse move
    if (this->GetCameraNode() != nullptr && this->GetCameraNode()->GetScene() != nullptr
        && inputEventData->GetType() == vtkCommand::MouseMoveEvent)
    {
      vtkMRMLScene* scene = this->GetCameraNode()->GetScene();
      vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(scene);
      if (crosshairNode)
      {
        crosshairNode->SetCursorPositionRAS(worldPosition);
      }
    }
  }
  ed->SetMouseMovedSinceButtonDown(this->MouseMovedSinceButtonDown);
  ed->SetAccuratePicker(this->AccuratePicker);
  ed->SetRenderer(pokedRenderer);

  ed->SetAttributesFromInteractor(this->GetInteractor());

  return this->DelegateInteractionEventDataToDisplayableManagers(ed);
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewInteractorStyle::SetInteractor(vtkRenderWindowInteractor* interactor)
{
  if (interactor)
  {
    // A default FPS of 30. seems good enough, but feel free to increase if needed
    // Please note that the VolumeRendering module changes it.
    interactor->SetDesiredUpdateRate(30.);
  }
  this->Superclass::SetInteractor(interactor);
}

//---------------------------------------------------------------------------
bool vtkMRMLThreeDViewInteractorStyle::QuickPick(int x, int y, double pickPoint[3])
{
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(x, y);
  vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->GetInteractor()->GetInteractorStyle());
  interactorStyle->SetCurrentRenderer(pokedRenderer);
  if (pokedRenderer == nullptr)
  {
    vtkDebugMacro("Pick: couldn't find the poked renderer at event position " << x << ", " << y);
    return false;
  }

  bool quickPicked = (this->QuickPicker->Pick(x, y, 0, pokedRenderer) > 0);
  this->QuickPicker->GetPickPosition(pickPoint);

  // QuickPicker ignores volume-rendered images, do a volume picking, too.
  if (this->CameraNode)
  {
    // Set picklist to volume actors to restrict the volume picker to only pick volumes
    // (otherwise it would also perform cell picking on meshes, which can take a long time).
    vtkPropCollection* pickList = this->QuickVolumePicker->GetPickList();
    // We could get the volumes using pokedRenderer->GetVolumes()
    // but then we would need to copy the collection and this is a hot loop
    // (run each time the mouse moves over a 3D view).
    pickList->RemoveAllItems();
    vtkPropCollection* props = pokedRenderer->GetViewProps();
    vtkCollectionSimpleIterator pit;
    vtkProp* aProp = nullptr;
    for (props->InitTraversal(pit); (aProp = props->GetNextProp(pit));)
    {
      aProp->GetVolumes(pickList);
    }

    if (pickList->GetNumberOfItems() > 0 && this->QuickVolumePicker->Pick(x, y, 0, pokedRenderer))
    {
      double volumePickPoint[3] = { 0.0, 0.0, 0.0 };
      this->QuickVolumePicker->GetPickPosition(volumePickPoint);
      double* cameraPosition = this->CameraNode->GetPosition();
      // Use QuickVolumePicker result instead of QuickPicker result if picked volume point
      // is closer to the camera (or QuickPicker did not find anything).
      if (!quickPicked
          || vtkMath::Distance2BetweenPoints(volumePickPoint, cameraPosition)
               < vtkMath::Distance2BetweenPoints(pickPoint, cameraPosition))
      {
        pickPoint[0] = volumePickPoint[0];
        pickPoint[1] = volumePickPoint[1];
        pickPoint[2] = volumePickPoint[2];
      }
    }
  }

  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLThreeDViewInteractorStyle::ProcessDisplayableManagerEvents(
  vtkMRMLAbstractDisplayableManager* displayableManager,
  unsigned long event,
  void* callData)
{
  this->Superclass::ProcessDisplayableManagerEvents(displayableManager, event, callData);

  if (vtkMRMLCameraDisplayableManager::SafeDownCast(displayableManager))
  {
    vtkMRMLCameraDisplayableManager* cameraDisplayableManager =
      vtkMRMLCameraDisplayableManager::SafeDownCast(displayableManager);
    if (event == vtkMRMLCameraDisplayableManager::ActiveCameraChangedEvent)
    {
      this->SetCameraNode(cameraDisplayableManager->GetCameraNode());
    }
  }
}
