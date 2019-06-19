/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkInteractorStyleTrackballCamera.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkMRMLThreeDViewInteractorStyle_h
#define __vtkMRMLThreeDViewInteractorStyle_h

#include "vtkMRMLDisplayableManagerExport.h"

// MRML includes
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLViewInteractorStyle.h"

// VTK includes
#include "vtkObject.h"
#include "vtkSmartPointer.h"

class vtkCellPicker;
class vtkWorldPointPicker;

/// \brief Interactive manipulation of the camera.
///
/// This class is based on vtkInteractorStyleTrackballCamera, but includes
/// extra features and event invocations to support extra features of slicer.
///
/// vtkInteractorStyleTrackballCamera allows the user to interactively
/// manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.  In
/// trackball interaction, the magnitude of the mouse motion is proportional
/// to the camera motion associated with a particular mouse binding. For
/// example, small left-button motions cause small changes in the rotation of
/// the camera around its focal point. For a 3-button mouse, the left button
/// is for rotation, the right button for zooming, the middle button for
/// panning, and ctrl + left button for spinning.  (With fewer mouse buttons,
/// ctrl + shift + left button is for zooming, and shift + left button is for
/// panning.)
/// \sa vtkInteractorStyleTrackballActor
/// \sa vtkInteractorStyleJoystickCamera
/// \sa vtkInteractorStyleJoystickActor
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLThreeDViewInteractorStyle :
  public vtkMRMLViewInteractorStyle
{
public:
  static vtkMRMLThreeDViewInteractorStyle *New();
  vtkTypeMacro(vtkMRMLThreeDViewInteractorStyle, vtkMRMLViewInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Event bindings controlling the effects of pressing mouse buttons
  /// or moving the mouse.
  void OnLeave() override;

  /// Give a chance to displayable managers to process the event.
  /// Return true if the event is processed.
  using vtkMRMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers;
  bool DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData) override;

  ///
  /// Get/Set the CameraNode
  vtkGetObjectMacro ( CameraNode, vtkMRMLCameraNode );
  vtkSetObjectMacro ( CameraNode, vtkMRMLCameraNode );

  ///
  /// Reimplemented to set the default interactive update rate
  void SetInteractor(vtkRenderWindowInteractor *interactor) override;

protected:
  vtkMRMLThreeDViewInteractorStyle();
  ~vtkMRMLThreeDViewInteractorStyle() override;

  bool QuickPick(int x, int y, double pickPoint[3]);

  vtkMRMLCameraNode *CameraNode;

  /// For jump to slice feature (when mouse is moved while shift key is pressed)
  vtkSmartPointer<vtkCellPicker> AccuratePicker;
  vtkSmartPointer<vtkWorldPointPicker> QuickPicker;

private:
  vtkMRMLThreeDViewInteractorStyle(const vtkMRMLThreeDViewInteractorStyle&) = delete;
  void operator=(const vtkMRMLThreeDViewInteractorStyle&) = delete;
};

#endif
