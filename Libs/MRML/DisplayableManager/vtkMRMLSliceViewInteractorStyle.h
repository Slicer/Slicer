/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMRMLSliceViewInteractorStyle.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkMRMLSliceViewInteractorStyle_h
#define __vtkMRMLSliceViewInteractorStyle_h

// VTK includes
#include "vtkWeakPointer.h"

// MRML includes
#include "vtkMRMLDisplayableManagerExport.h"
#include "vtkMRMLViewInteractorStyle.h"

class vtkMRMLAbstractDisplayableManager;
class vtkMRMLCrosshairDisplayableManager;
class vtkMRMLScalarBarDisplayableManager;
class vtkMRMLSegmentationDisplayNode;
class vtkMRMLSliceLogic;
class vtkTimerLog;

/// \brief Provides customizable interaction routines.
///
/// Relies on vtkInteractorStyleUser, but with MouseWheelEvents.
/// and mapping to control the slicer slice logic (manipulates the
/// vtkMRMLSliceNode and vtkMRMLSliceCompositeNode.
/// TODO:
/// * Do we need Rotate Mode?  Probably better to just rely on the reformat widget
/// * Do we need to set the slice spacing on EnterEvent (I say no, nothing to do
///   with linked slices should go in here)
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLSliceViewInteractorStyle :
  public vtkMRMLViewInteractorStyle
{
public:
  static vtkMRMLSliceViewInteractorStyle *New();
  vtkTypeMacro(vtkMRMLSliceViewInteractorStyle,vtkMRMLViewInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Give a chance to displayable managers to process the event.
  /// Return true if the event is processed.
  using vtkMRMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers;
  bool DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData) override;

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
    AdjustLightbox = 256, /* not used */
    SelectVolume = 512,
    SetCursorPosition = 1024, /* adjust cursor position in crosshair node as mouse is moved */
    SetCrosshairPosition = 2048,
    TranslateSliceIntersection = 4096,
    RotateSliceIntersection = 8192,
    AllActionsMask = Translate | Zoom | Rotate | Blend | AdjustWindowLevelBackground | AdjustWindowLevelForeground
      | BrowseSlice | ShowSlice | AdjustLightbox | SelectVolume | SetCursorPosition | SetCrosshairPosition
      | TranslateSliceIntersection | RotateSliceIntersection
    };

  /// Enable/disable the specified action (Translate, Zoom, Blend, etc.).
  /// Multiple actions can be specified by providing the sum of action ids.
  /// Set the value to AllActionsMask to enable/disable all actions.
  /// All actions are enabled by default.
  void SetActionEnabled(int actionsMask, bool enable = true);
  /// Returns true if the specified action is allowed.
  /// If multiple actions are specified, the return value is true if all actions are enabled.
  bool GetActionEnabled(int actionsMask);

  ///
  /// Get/Set the SliceLogic
  void SetSliceLogic(vtkMRMLSliceLogic* SliceLogic);
  vtkGetObjectMacro(SliceLogic, vtkMRMLSliceLogic);

  vtkMRMLCrosshairDisplayableManager* GetCrosshairDisplayableManager();

  vtkMRMLScalarBarDisplayableManager* GetScalarBarDisplayableManager();

protected:
  vtkMRMLSliceViewInteractorStyle();
  ~vtkMRMLSliceViewInteractorStyle() override;

  static void SliceViewProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  void SetMouseCursor(int cursor);

  vtkMRMLSliceLogic *SliceLogic;

  //bool MouseMovedSinceButtonDown;

  //vtkSmartPointer<vtkTimerLog> ClickTimer;
  //int NumberOfClicks;

  bool EnableCursorUpdate;

private:
  vtkMRMLSliceViewInteractorStyle(const vtkMRMLSliceViewInteractorStyle&) = delete;
  void operator=(const vtkMRMLSliceViewInteractorStyle&) = delete;
};

#endif
