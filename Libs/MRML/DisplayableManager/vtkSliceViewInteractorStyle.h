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
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  ///
  /// Events are either handled here by changing the slice node
  /// and composite node (sometimes using the logic's methods) or
  /// they are passed to the vtkInteractorStyleUser, which conditionally
  /// passes them to observers if there are any.
  ///
  /// Generic event bindings
  virtual void OnMouseMove() VTK_OVERRIDE;
  virtual void OnLeftButtonDown() VTK_OVERRIDE;
  virtual void OnLeftButtonUp() VTK_OVERRIDE;
  virtual void OnMiddleButtonDown() VTK_OVERRIDE;
  virtual void OnMiddleButtonUp() VTK_OVERRIDE;
  virtual void OnRightButtonDown() VTK_OVERRIDE;
  virtual void OnRightButtonUp() VTK_OVERRIDE;
  /// MouseWheel callbacks added for slicer
  virtual void OnMouseWheelForward() VTK_OVERRIDE;
  virtual void OnMouseWheelBackward() VTK_OVERRIDE;
  ///
  /// Keyboard functions
  virtual void OnChar() VTK_OVERRIDE;
  virtual void OnKeyPress() VTK_OVERRIDE;
  virtual void OnKeyRelease() VTK_OVERRIDE;
  ///
  /// These are more esoteric events, but are useful in some cases.
  virtual void OnExpose() VTK_OVERRIDE;
  virtual void OnConfigure() VTK_OVERRIDE;
  virtual void OnEnter() VTK_OVERRIDE;
  virtual void OnLeave() VTK_OVERRIDE;

  /// Internal state management for multi-event sequences (like click-drag-release)

  /// Action State values and management
  enum
    {
    None = 0,
    Translate = 1,
    Zoom = 2,
    Rotate = 4, /* Rotate not currently used */
    Blend = 8, /* fg to bg, labelmap to bg */
    AdjustWindowLevelBackground = 16,
    AdjustWindowLevelForeground = 32,
    BrowseSlice = 64,
    ShowSlice = 128,
    AdjustLightbox = 256,
    SelectVolume = 512,
    SetCursorPosition = 1024, /* adjust cursor position in crosshair node as mouse is moved */
    AllActionsMask = Translate | Zoom | Rotate | Blend | AdjustWindowLevelBackground | AdjustWindowLevelForeground
      | BrowseSlice | ShowSlice | AdjustLightbox | SelectVolume | SetCursorPosition
    };
  vtkGetMacro(ActionState, int);
  vtkSetMacro(ActionState, int);

  /// Enable/disable the specified action (Translate, Zoom, Blend, etc.).
  /// Multiple actions can be specifed by providing the sum of action ids.
  /// Set the value to AllActionsMask to enable/disable all actions.
  /// All actions are enabled by default.
  void SetActionEnabled(int actionsMask, bool enable = true);
  /// Returns true if the specified action is allowed.
  /// If multiple actions are specified, the return value is true if all actions are enabled.
  bool GetActionEnabled(int actionsMask);

  /// Helper routines

  /// check for prescribed spacing, otherwise return best spacing amount
  /// for current layer setup (use logic to look for spacing of first non-null
  /// layer)
  double GetSliceSpacing();
  /// Adjust the slice position with respect to current slice node offset
  void IncrementSlice();
  void DecrementSlice();
  void MoveSlice(double delta);
  /// Adjust zoom factor. If zoomScaleFactor>1 then view is zoomed in,
  /// if 0<zoomScaleFactor<1 then view is zoomed out.
  void ScaleZoom(double zoomScaleFactor);

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

  /// Returns true if the volume's window/level values are editable
  /// on the GUI
  bool VolumeWindowLevelEditable(const char* volumeNodeID);

  int ActionState;
  int ActionsEnabled;

  /// Indicates whether the shift key was used during the previous action.
  /// This is used to require shift-up before returning to default mode.
  bool ShiftKeyUsedForPreviousAction;

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
