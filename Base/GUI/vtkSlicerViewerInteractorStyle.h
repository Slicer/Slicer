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
// .NAME vtkSlicerViewerInteractorStyle - interactive manipulation of the camera
// .SECTION Description
// This class is based on vtkInteractorStyleTrackballCamera, but includes
// extra features and event invocations to support extra features of slicer.
//
// vtkInteractorStyleTrackballCamera allows the user to interactively
// manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.  In
// trackball interaction, the magnitude of the mouse motion is proportional
// to the camera motion associated with a particular mouse binding. For
// example, small left-button motions cause small changes in the rotation of
// the camera around its focal point. For a 3-button mouse, the left button
// is for rotation, the right button for zooming, the middle button for
// panning, and ctrl + left button for spinning.  (With fewer mouse buttons,
// ctrl + shift + left button is for zooming, and shift + left button is for
// panning.)

// .SECTION See Also
// vtkInteractorStyleTrackballActor vtkInteractorStyleJoystickCamera
// vtkInteractorStyleJoystickActor

#ifndef __vtkSlicerViewerInteractorStyle_h
#define __vtkSlicerViewerInteractorStyle_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkInteractorStyle.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerViewerInteractorStyle : public vtkInteractorStyle
{
public:
  static vtkSlicerViewerInteractorStyle *New();
  vtkTypeRevisionMacro(vtkSlicerViewerInteractorStyle,vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Event bindings controlling the effects of pressing mouse buttons
  // or moving the mouse.
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();

  // These methods for the different interactions in different modes
  // are overridden in subclasses to perform the correct motion. Since
  // they are called by OnTimer, they do not have mouse coord parameters
  // (use interactor's GetEventPosition and GetLastEventPosition)
  virtual void Rotate();
  virtual void Spin();
  virtual void Pan();
  virtual void Dolly();
  
protected:
  vtkSlicerViewerInteractorStyle();
  ~vtkSlicerViewerInteractorStyle();

  double MotionFactor;

  virtual void Dolly(double factor);

private:
  vtkSlicerViewerInteractorStyle(const vtkSlicerViewerInteractorStyle&);  // Not implemented.
  void operator=(const vtkSlicerViewerInteractorStyle&);  // Not implemented.
};

#endif
