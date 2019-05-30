/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMRMLThreeDViewInteractorStyle.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMRMLThreeDViewInteractorStyle.h"

// MRML includes
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLModelDisplayableManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkEvent.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWorldPointPicker.h>

vtkStandardNewMacro(vtkMRMLThreeDViewInteractorStyle);

//----------------------------------------------------------------------------
vtkMRMLThreeDViewInteractorStyle::vtkMRMLThreeDViewInteractorStyle()
{
  this->CameraNode = nullptr;
  this->AccuratePicker = vtkSmartPointer<vtkCellPicker>::New();
  this->AccuratePicker->SetTolerance( .005 );
  this->QuickPicker = vtkSmartPointer<vtkWorldPointPicker>::New();
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewInteractorStyle::~vtkMRMLThreeDViewInteractorStyle()
{
  this->SetCameraNode(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
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
  // Get display and world position
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);
  if (!pokedRenderer || !inputEventData)
    {
    // can happen during application shutdown
    return false;
    }

  vtkNew<vtkMRMLInteractionEventData> ed;
  ed->SetType(inputEventData->GetType());
  int displayPositionCorrected[2] = { displayPositionInt[0] - pokedRenderer->GetOrigin()[0], displayPositionInt[1] - pokedRenderer->GetOrigin()[1] };
  ed->SetDisplayPosition(displayPositionCorrected);
  double worldPosition[4] = { 0.0, 0.0, 0.0, 1.0 };
  if (this->QuickPick(displayPositionInt[0], displayPositionInt[1], worldPosition))
    {
    // set "inaccurate" world position
    ed->SetWorldPosition(worldPosition, false);
    }
  ed->SetAccuratePicker(this->AccuratePicker);
  ed->SetRenderer(this->CurrentRenderer);

  ed->SetAttributesFromInteractor(this->GetInteractor());

  return this->DelegateInteractionEventDataToDisplayableManagers(ed);
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewInteractorStyle::SetInteractor(vtkRenderWindowInteractor *interactor)
{
  if (interactor)
    {
    // A default FPS of 30. seems good enough, but feel free to increase if needed
    // Please note that the VolumeRendering module changes it.
    interactor->SetDesiredUpdateRate( 30.);
    }
  this->Superclass::SetInteractor(interactor);
}

//---------------------------------------------------------------------------
bool vtkMRMLThreeDViewInteractorStyle::QuickPick(int x, int y, double pickPoint[3])
{
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == nullptr)
  {
    vtkDebugMacro("Pick: couldn't find the poked renderer at event position " << x << ", " << y);
    return false;
  }

  this->QuickPicker->Pick(x, y, 0, this->CurrentRenderer);

  this->QuickPicker->GetPickPosition(pickPoint);

  return true;
}
