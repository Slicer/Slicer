/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMRMLSliceIntersectionWidget.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkMRMLSliceIntersectionWidget
 * @brief   Show slice intersection lines
 *
 * The vtkMRMLSliceIntersectionWidget allows moving slices by interacting with
 * displayed slice intersection lines.
 *
 *
*/

#ifndef vtkMRMLSliceIntersectionWidget_h
#define vtkMRMLSliceIntersectionWidget_h

#include "vtkMRMLDisplayableManagerExport.h" // For export macro
#include "vtkMRMLAbstractWidget.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"

#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkSliceIntersectionRepresentation2D;
class vtkMRMLApplicationLogic;
class vtkMRMLSegmentationDisplayNode;


class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLSliceIntersectionWidget : public vtkMRMLAbstractWidget
{
public:
  /**
   * Instantiate this class.
   */
  static vtkMRMLSliceIntersectionWidget *New();

  //@{
  /**
   * Standard VTK class macros.
   */
  vtkTypeMacro(vtkMRMLSliceIntersectionWidget, vtkMRMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation();

  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  vtkMRMLSliceNode* GetSliceNode();

  void SetMRMLApplicationLogic(vtkMRMLApplicationLogic* applicationLogic) override;

  /// Return true if the widget can process the event.
  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2) override;

  /// Process interaction event.
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  /// Called when the the widget loses the focus.
  void Leave(vtkMRMLInteractionEventData* eventData) override;

  /// Widget states
  enum
    {
    WidgetStateFollowCursor = WidgetStateUser,
    WidgetStateBlend,
    WidgetStateTranslateSlice,
    WidgetStateZoomSlice,
    WidgetStateTouchGesture,
    };

  /// Widget events
  enum
    {
    WidgetEventTouchGestureStart = WidgetEventUser,
    WidgetEventTouchGestureEnd,
    WidgetEventTouchRotateSliceIntersection,
    WidgetEventTouchZoomSlice,
    WidgetEventTouchTranslateSlice,
    WidgetEventBlendStart,
    WidgetEventBlendEnd,
    WidgetEventToggleLabelOpacity,
    WidgetEventToggleForegroundOpacity,
    WidgetEventIncrementSlice,
    WidgetEventDecrementSlice,
    WidgetEventZoomInSlice,
    WidgetEventZoomOutSlice,
    WidgetEventToggleSliceVisibility,
    WidgetEventToggleAllSlicesVisibility, // currently does not work, only toggles current slice
    WidgetEventResetFieldOfView,
    WidgetEventShowNextBackgroundVolume,
    WidgetEventShowPreviousBackgroundVolume,
    WidgetEventShowNextForegroundVolume,
    WidgetEventShowPreviousForegroundVolume,
    WidgetEventTranslateSliceStart,
    WidgetEventTranslateSliceEnd,
    WidgetEventZoomSliceStart,
    WidgetEventZoomSliceEnd,
    WidgetEventSetCrosshairPosition,
    };

  /// Action State values and management
  enum
  {
    ActionNone = 0,
    ActionTranslate = 1,
    ActionZoom = 2,
    ActionRotate = 4, /* not used */
    ActionBlend = 8, /* fg to bg, labelmap to bg */
    ActionBrowseSlice = 64,
    ActionShowSlice = 128,
    ActionAdjustLightbox = 256,
    ActionSelectVolume = 512,
    ActionSetCursorPosition = 1024, /* adjust cursor position in crosshair node as mouse is moved */
    ActionSetCrosshairPosition = 2048, /* adjust cursor position in crosshair node as mouse is moved */
    ActionTranslateSliceIntersection = 4096,
    ActionRotateSliceIntersection = 8192,
    ActionAll = ActionTranslate | ActionZoom | ActionRotate | ActionBlend
    | ActionBrowseSlice | ActionShowSlice | ActionAdjustLightbox | ActionSelectVolume
    | ActionSetCursorPosition | ActionSetCrosshairPosition
    | ActionTranslateSliceIntersection | ActionRotateSliceIntersection
  };

  /// Set exact list of actions to enable.
  void SetActionsEnabled(int actions);

  /// Set full list of enabled actions.
  int GetActionsEnabled();

  /// Enable/disable the specified action (Translate, Zoom, Blend, etc.).
  /// Multiple actions can be specified by providing the sum of action ids.
  /// Set the value to AllActionsMask to enable/disable all actions.
  /// All actions are enabled by default.
  void SetActionEnabled(int actionsMask, bool enable = true);
  /// Returns true if the specified action is allowed.
  /// If multiple actions are specified, the return value is true if all actions are enabled.
  bool GetActionEnabled(int actionsMask);

  void UpdateInteractionEventMapping();

protected:
  vtkMRMLSliceIntersectionWidget();
  ~vtkMRMLSliceIntersectionWidget() override;

  bool ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData);
  bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData);
  bool ProcessBlend(vtkMRMLInteractionEventData* eventData);

  bool ProcessRotateStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessRotate(vtkMRMLInteractionEventData* eventData);
  bool ProcessSetCrosshair(vtkMRMLInteractionEventData* eventData);
  double GetSliceRotationAngleRad(double eventPos[2]);

  // Move slice in-plane by click-and-drag
  bool ProcessTranslateSlice(vtkMRMLInteractionEventData* eventData);

  bool ProcessZoomSlice(vtkMRMLInteractionEventData* eventData);

  bool ProcessTouchGestureStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchGestureEnd(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchRotate(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchZoom(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchTranslate(vtkMRMLInteractionEventData* eventData);

  /// Rotate the message by the specified amount. Used for touchpad events.
  bool Rotate(double sliceRotationAngleRad);

  /// Adjust zoom factor. If zoomScaleFactor>1 then view is zoomed in,
  /// if 0<zoomScaleFactor<1 then view is zoomed out.
  /// Event position is used as center for zoom operation.
  void ScaleZoom(double zoomScaleFactor, vtkMRMLInteractionEventData* eventData);

  /// Get/Set labelmap or segmentation opacity
  void SetLabelOpacity(double opacity);
  double GetLabelOpacity();

  /// Returns true if mouse is inside the selected layer volume.
  /// Use background flag to choose between foreground/background layer.
  bool IsEventInsideVolume(bool background, vtkMRMLInteractionEventData* eventData);

  vtkMRMLSegmentationDisplayNode* GetVisibleSegmentationDisplayNode();

  static void SliceLogicsModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  vtkWeakPointer<vtkCollection> SliceLogics;
  vtkWeakPointer<vtkMRMLSliceNode> SliceNode;
  vtkWeakPointer<vtkMRMLSliceLogic> SliceLogic;
  vtkNew<vtkCallbackCommand> SliceLogicsModifiedCommand;

  double StartEventPosition[2];
  double PreviousRotationAngleRad;
  int PreviousEventPosition[2];
  double StartRotationCenter[2];
  double StartRotationCenter_RAS[4];

  double StartActionFOV[3];
  double VolumeScalarRange[2];

  enum
  {
    LayerBackground,
    LayerForeground,
    LayerLabelmap
  };

  // Blend
  double LastForegroundOpacity;
  double LastLabelOpacity;
  vtkMRMLSegmentationDisplayNode* StartActionSegmentationDisplayNode;

  // Browse slice
  /// check for prescribed spacing, otherwise return best spacing amount
  /// for current layer setup (use logic to look for spacing of first non-null
  /// layer)
  double GetSliceSpacing();
  /// Adjust the slice position with respect to current slice node offset
  void IncrementSlice();
  void DecrementSlice();
  void MoveSlice(double delta);

  ///
  /// Change the displayed volume in the selected layer by moving
  /// in a loop trough the volumes available in the scene.
  ///  - layer: are 0,1,2 for bg, fg, lb
  ///  - direction: positive or negative (wraps through volumes in scene)
  void CycleVolumeLayer(int layer, int direction);

  /// Indicates whether the shift key was used during the previous action.
  /// This is used to require shift-up before returning to default mode.
  bool ModifierKeyPressedSinceLastMouseButtonRelease;

  int ActionsEnabled;

  double TouchRotationThreshold;
  double TouchTranslationThreshold;
  double TouchZoomThreshold;
  double TotalTouchRotation;
  bool TouchRotateEnabled;
  double TotalTouchTranslation;
  bool TouchTranslationEnabled;
  double TotalTouchZoom;
  bool TouchZoomEnabled;

private:
  vtkMRMLSliceIntersectionWidget(const vtkMRMLSliceIntersectionWidget&) = delete;
  void operator=(const vtkMRMLSliceIntersectionWidget&) = delete;
};

#endif
