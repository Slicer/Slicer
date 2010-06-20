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
///  vtkMRMLThreeDRenderViewInteractorStyle - interactive manipulation of the camera
/// 
/// This class is based on vtkInteractorStyleTrackballCamera, but includes
/// extra features and event invocations to support extra features of slicer.
//
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

/// .SECTION See Also
/// vtkInteractorStyleTrackballActor vtkInteractorStyleJoystickCamera
/// vtkInteractorStyleJoystickActor

#ifndef __vtkMRMLThreeDRenderViewInteractorStyle_h
#define __vtkMRMLThreeDRenderViewInteractorStyle_h

// VTK includes
#include "vtkObject.h"
#include "vtkInteractorStyle.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLCameraNode.h"

#include "qMRMLWidgetsExport.h"
// #include "vtkSlicerApplicationLogic.h"

// class vtkSlicerViewerWidget;
class QMRML_WIDGETS_EXPORT vtkMRMLThreeDRenderViewInteractorStyle : public vtkInteractorStyle
{
public:
  static vtkMRMLThreeDRenderViewInteractorStyle *New();
  vtkTypeRevisionMacro(vtkMRMLThreeDRenderViewInteractorStyle, vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent);

//   vtkGetObjectMacro(ApplicationLogic, vtkSlicerApplicationLogic );
//   vtkSetObjectMacro(ApplicationLogic, vtkSlicerApplicationLogic );

  /// 
  /// Event bindings controlling the effects of pressing mouse buttons
  /// or moving the mouse.
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();

  /// These methods for the different interactions in different modes
  /// are overridden in subclasses to perform the correct motion. Since
  /// they are called by OnTimer, they do not have mouse coord parameters
  /// (use interactor's GetEventPosition and GetLastEventPosition)
  virtual void Rotate();
  virtual void Spin();
  virtual void Pan();
  virtual void Dolly();
  virtual void Dolly(double factor);

  virtual void OnEnter();
  virtual void OnLeave();
  virtual void OnExpose();


  /// 
  /// Get/Set the CameraNode
  vtkGetObjectMacro ( CameraNode, vtkMRMLCameraNode );
  vtkSetObjectMacro ( CameraNode, vtkMRMLCameraNode );

  ///
  /// Get the number of mouse 'pick' events
  vtkGetMacro (NumberOfPicks, int);

  ///
  /// Get the max number of 'pick' events allowed before interaction
  /// mode switches back to 'transform'.
  vtkGetMacro (NumberOfTransientPicks, int );

  ///
  /// Get the number of mouse 'place' events
  vtkGetMacro (NumberOfPlaces, int);

  ///
  /// Get the max number of 'place' events allowed before interaction mode switches
  /// back to 'transform'.
  vtkGetMacro ( NumberOfTransientPlaces, int );

  
  /// 
  /// Get/Set the main slicer viewer widget, for picking
//   vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
//   virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  /// 
  /// Events
  //BTX
  enum
  {
      PickEvent,
      PlaceEvent,
      SelectRegionEvent,
      PlotEvent,
  };
  //ETX
    
protected:
  vtkMRMLThreeDRenderViewInteractorStyle();
  ~vtkMRMLThreeDRenderViewInteractorStyle();

  vtkMRMLCameraNode *CameraNode;
//   vtkSlicerApplicationLogic *ApplicationLogic;

  double MotionFactor;
  
  ///
  /// Keep track of the number of picks so for resetting mouse modes when
  /// transient pick or place mode has been selected.
  int NumberOfPicks;
  int NumberOfPlaces;
  
  ///
  /// The number of "clicks" the transient mouse-modes come loaded with.
  /// Currently makes sense to set this to 1 -- but we can change it if appropriate.
  int NumberOfTransientPicks;
  int NumberOfTransientPlaces;

  /// 
  /// A pointer back to the viewer widget, useful for picking
//   vtkSlicerViewerWidget *ViewerWidget;
  
private:
  vtkMRMLThreeDRenderViewInteractorStyle(const vtkMRMLThreeDRenderViewInteractorStyle&);  /// Not implemented.
  void operator=(const vtkMRMLThreeDRenderViewInteractorStyle&);  /// Not implemented.
};

#endif
