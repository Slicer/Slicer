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

#ifndef __vtkSliceViewInteractorStyle_h
#define __vtkSliceViewInteractorStyle_h

// VTK includes
#include "vtkInteractorStyleUser.h"
#include "vtkMatrix4x4.h"

// MRML includes
#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLSegmentationDisplayNode;
class vtkMRMLSliceLogic;

/// \brief Provides customizable interaction routines.
///
/// Relies on vtkInteractorStyleUser, but with MouseWheelEvents.
/// and mapping to control the slicer slice logic (manipulates the
/// vtkMRMLSliceNode and vtkMRMLSliceCompositeNode.
/// TODO:
/// * Do we need Rotate Mode?  Probably better to just rely on the reformat widget
/// * Do we need to set the slice spacing on EnterEvent (I say no, nothing to do
///   with linked slices should go in here)
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkSliceViewInteractorStyle : public vtkInteractorStyleUser
{
public:
  static vtkSliceViewInteractorStyle *New();
  vtkTypeMacro(vtkSliceViewInteractorStyle,vtkInteractorStyleUser);
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
    Blend, /* fg to bg, labelmap to bg */
    AdjustWindowLevelBackground,
    AdjustWindowLevelForeground
    };
  vtkGetMacro(ActionState, int);
  vtkSetMacro(ActionState, int);

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

  /// Enter a mode where the mouse moves are used to change the foreground
  /// or labelmap opacity.
  void StartBlend();
  void EndBlend();

  /// Enter a mode where the mouse moves are used to change the window/level
  /// setting.
  void StartAdjustWindowLevel();
  void EndAdjustWindowLevel();

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

  vtkMRMLSegmentationDisplayNode* GetVisibleSegmentationDisplayNode();

  ///
  /// Change the displayed volume in the selected layer by moving
  /// in a loop trough the volumes available in the scene.
  ///  - layer: are 0,1,2 for bg, fg, lb
  ///  - direction: positive or negative (wraps through volumes in scene)
  void CycleVolumeLayer(int layer, int direction);

  /// Get/Set labelmap or segmentation opacity
  void SetLabelOpacity(double opacity);
  double GetLabelOpacity();

protected:

  vtkSliceViewInteractorStyle();
  ~vtkSliceViewInteractorStyle();

  int GetMouseInteractionMode();

  /// Returns true if mouse is inside the selected layer volume.
  /// Use background flag to choose between foreground/background layer.
  bool IsMouseInsideVolume(bool background);

  int ActionState;
  int StartActionEventPosition[2];
  double StartActionFOV[3];
  double VolumeScalarRange[2];
  vtkMRMLSegmentationDisplayNode* StartActionSegmentationDisplayNode;

  int LastEventPosition[2];
  double LastForegroundOpacity;
  double LastLabelOpacity;
  double LastVolumeWindowLevel[2];

  vtkMRMLSliceLogic *SliceLogic;

private:
  vtkSliceViewInteractorStyle(const vtkSliceViewInteractorStyle&);  /// Not implemented.
  void operator=(const vtkSliceViewInteractorStyle&);  /// Not implemented.
};

#endif
