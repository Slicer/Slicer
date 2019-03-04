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

#ifndef __vtkThreeDViewInteractorStyle_h
#define __vtkThreeDViewInteractorStyle_h

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLDisplayableManagerGroup.h"

// VTK includes
#include "vtkObject.h"
#include "vtkInteractorStyle.h"
#include "vtkSmartPointer.h"

#include "vtkMRMLDisplayableManagerExport.h"

class vtkCellPicker;
class vtkMRMLModelDisplayableManager;
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
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkThreeDViewInteractorStyle :
  public vtkInteractorStyle
{
public:
  static vtkThreeDViewInteractorStyle *New();
  vtkTypeMacro(vtkThreeDViewInteractorStyle,vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  ///
  /// Event bindings controlling the effects of pressing mouse buttons
  /// or moving the mouse.
  virtual void OnMouseMove() VTK_OVERRIDE;
  virtual void OnEnter() VTK_OVERRIDE;
  virtual void OnLeave() VTK_OVERRIDE;
  virtual void OnLeftButtonDown() VTK_OVERRIDE;
  virtual void OnLeftButtonUp() VTK_OVERRIDE;
  virtual void OnMiddleButtonDown() VTK_OVERRIDE;
  virtual void OnMiddleButtonUp() VTK_OVERRIDE;
  virtual void OnRightButtonDown() VTK_OVERRIDE;
  virtual void OnRightButtonUp() VTK_OVERRIDE;
  virtual void OnMouseWheelForward() VTK_OVERRIDE;
  virtual void OnMouseWheelBackward() VTK_OVERRIDE;

  /// Keyboard functions
  virtual void OnChar() VTK_OVERRIDE;
  virtual void OnKeyPress() VTK_OVERRIDE;
  virtual void OnKeyRelease() VTK_OVERRIDE;

  /// These are more esoteric events, but are useful in some cases.
  virtual void OnExpose() VTK_OVERRIDE;
  virtual void OnConfigure() VTK_OVERRIDE;

  void SetDisplayableManagers(vtkMRMLDisplayableManagerGroup* displayableManagers);

  /// Give a chance to displayable managers to process the event.
  /// Return true if the event is processed.
  bool ForwardInteractionEventToDisplayableManagers(unsigned long event);

  ///
  /// Get/Set the CameraNode
  vtkGetObjectMacro ( CameraNode, vtkMRMLCameraNode );
  vtkSetObjectMacro ( CameraNode, vtkMRMLCameraNode );

  ///
  /// Reimplemented to set the default interactive update rate
  virtual void SetInteractor(vtkRenderWindowInteractor *interactor) VTK_OVERRIDE;

  ///
  /// Get/Set the ModelDisplayableManager, for picking
  vtkGetObjectMacro(ModelDisplayableManager, vtkMRMLModelDisplayableManager);
  virtual void SetModelDisplayableManager(vtkMRMLModelDisplayableManager *modelDisplayableManager);
  /// These methods for the different interactions in different modes
  /// are overridden in subclasses to perform the correct motion. Since
  /// they are called by OnTimer, they do not have mouse coord parameters
  /// (use interactor's GetEventPosition and GetLastEventPosition)
  virtual void Rotate() VTK_OVERRIDE;
  virtual void Spin() VTK_OVERRIDE;
  virtual void Pan() VTK_OVERRIDE;
  virtual void Dolly() VTK_OVERRIDE;
  virtual void Dolly(double factor);


protected:
  vtkThreeDViewInteractorStyle();
  ~vtkThreeDViewInteractorStyle();

  static void ThreeDViewProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  bool AccuratePick(int x, int y, double pickPoint[3]);
  bool QuickPick(int x, int y, double pickPoint[3]);

  vtkMRMLCameraNode *CameraNode;

  double MotionFactor;

  ///
  /// A pointer back to the ModelDisplayableManager, useful for picking
  vtkMRMLModelDisplayableManager * ModelDisplayableManager;

  /// For jump to slice feature (when mouse is moved while shift key is pressed)
  vtkSmartPointer<vtkCellPicker> AccuratePicker;
  vtkSmartPointer<vtkWorldPointPicker> QuickPicker;

  bool MouseMovedSinceButtonDown;
  /// Indicates whether the shift key was used during the previous action.
  /// This is used to require shift-up before returning to default mode.
  bool ShiftKeyUsedForPreviousAction;

  vtkWeakPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagers;
  vtkMRMLAbstractDisplayableManager* FocusedDisplayableManager;

private:
  vtkThreeDViewInteractorStyle(const vtkThreeDViewInteractorStyle&);  /// Not implemented.
  void operator=(const vtkThreeDViewInteractorStyle&);  /// Not implemented.
};

#endif
