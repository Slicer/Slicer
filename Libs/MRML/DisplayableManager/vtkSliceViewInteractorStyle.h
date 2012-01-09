/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSliceViewInteractorStyle.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

///  vtkSliceViewInteractorStyle - provides customizable interaction routines
/// 
/// 
/// Relies on vtkInteractorStyleUser, but with MouseWheelEvents.
/// and mapping to control the slicer slice logic (manipulates the 
/// vtkMRMLSliceNode and vtkMRMLSliceCompositeNode.

/// TODO:
/// * Do we need Rotate Mode?  Probably better to just rely on the reformat widget
/// * Do we need to set the slice spacing on EnterEvent (I say no, nothing to do
///   with linked slices should go in here)
///

#ifndef __vtkSliceViewInteractorStyle_h
#define __vtkSliceViewInteractorStyle_h

// VTK includes
#include "vtkInteractorStyleUser.h"
#include "vtkMatrix4x4.h"

// MRML includes
#include "vtkMRMLDisplayableManagerWin32Header.h"


class vtkMRMLSliceLogic;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkSliceViewInteractorStyle : public vtkInteractorStyleUser
{
public:
  static vtkSliceViewInteractorStyle *New();
  vtkTypeRevisionMacro(vtkSliceViewInteractorStyle,vtkInteractorStyleUser);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  /// 
  /// Events are either handled here by changing the slice node
  /// and composite node (sometimes using the logic's methods) or
  /// they are passed to the vtkInteractorStyleUser, which conditionally
  /// passes them to observers if there are any.
  /// 
  /// Generic event bindings
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  /// MouseWheel callbacks added for slicer
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();
  /// 
  /// Keyboard functions
  virtual void OnChar();
  virtual void OnKeyPress();
  virtual void OnKeyRelease();
  /// 
  /// These are more esoteric events, but are useful in some cases.
  virtual void OnExpose();
  virtual void OnConfigure();
  virtual void OnEnter();
  virtual void OnLeave();

  /// Internal state management for multi-event sequences (like click-drag-release)

  /// Action State values and management
  enum
    {
    None = 0,
    Translate,
    Zoom,
    Rotate, /* Rotate not currently used */
    AdjustWindowLevel
    };
  vtkGetMacro(ActionState, int);
  vtkSetMacro(ActionState, int);

  /// state of things when the current action started
  /// - ras is mouse pointer in patient space 
  /// - fov is the slice node field of view
  /// - window is mouse pointer with respect to the whole viewer
  /// - xyz is mouse pointer with respect to the light box view (z is which light box viewer)
  vtkGetVector3Macro(ActionStartRAS, double);
  vtkSetVector3Macro(ActionStartRAS, double);
  vtkGetVector3Macro(ActionStartFOV, double);
  vtkSetVector3Macro(ActionStartFOV, double);
  vtkGetVector2Macro(ActionStartWindow, int);
  vtkSetVector2Macro(ActionStartWindow, int);
  vtkGetVector2Macro(ActionLastXY, int);
  vtkSetVector2Macro(ActionLastXY, int);

  /// what was the state of the slice node when the action started
  vtkGetObjectMacro(ActionStartSliceToRAS, vtkMatrix4x4);
  vtkGetObjectMacro(ActionStartXYToRAS, vtkMatrix4x4);
  /// an internal scratch matrix for calculations without
  /// the overhead of re-allocating the object
  vtkGetObjectMacro(ScratchMatrix, vtkMatrix4x4);

  /// State for label/foreground opacity toggles
  vtkSetMacro(LastLabelOpacity, double);
  vtkGetMacro(LastLabelOpacity, double);
  vtkSetMacro(LastForegroundOpacity, double);
  vtkGetMacro(LastForegroundOpacity, double);

  /// Helper routines

  /// check for prescribed spacing, otherwise return best spacing amount
  /// for current layer setup (use logic to look for spacing of first non-null
  /// layer)
  double GetSliceSpacing(); 
  /// Adjust the slice position with respect to current slice node offset
  void IncrementSlice(); 
  void DecrementSlice(); 
  void MoveSlice(double delta); 

  /// Collect some boilerplate management steps so they can be used
  /// in more than one place
  void StartTranslate();
  void EndTranslate();
  void StartAdjustWindowLevel();

  /// Get the RAS coordinates of the interactor's EventPosition
  /// with respect to the current poked renderer (taking into
  /// account the lightbox)
  void GetEventRAS(double ras[4]);
  void GetEventRASWithRespectToEventStart(double ras[4]);

  /// Convert event coordinates (with respect to viewport) into
  /// xyz coordinates, where z is the slice number of the lightbox
  /// and xy is the offset within the lightbox view.  The xyz coordinates
  /// can be used to map to RAS with the slice node's XYToRAS matrix.
  /// The 4th component is 1 so it can be used with a homogenous transform.
  void GetEventXYZ(double xyz[4]);

  /// 
  /// Get/Set the SliceLogic
  void SetSliceLogic(vtkMRMLSliceLogic* SliceLogic);
  vtkGetObjectMacro(SliceLogic, vtkMRMLSliceLogic);

protected:

  vtkSliceViewInteractorStyle();
  ~vtkSliceViewInteractorStyle();

  int ActionState;

  double ActionStartRAS[3];
  double ActionStartFOV[3];
  int ActionStartWindow[2];
  int ActionLastXY[2];

  vtkMatrix4x4 *ActionStartSliceToRAS;
  vtkMatrix4x4 *ActionStartXYToRAS;
  vtkMatrix4x4 *ScratchMatrix;

  double LastLabelOpacity;
  double LastForegroundOpacity;

  vtkMRMLSliceLogic *SliceLogic;

private:
  vtkSliceViewInteractorStyle(const vtkSliceViewInteractorStyle&);  /// Not implemented.
  void operator=(const vtkSliceViewInteractorStyle&);  /// Not implemented.
};

#endif
