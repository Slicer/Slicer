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
class vtkMRMLSliceDisplayNode;

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

  vtkMRMLSliceDisplayNode* GetSliceDisplayNode();

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
    // Interactions without handles
    WidgetStateFollowCursor = WidgetStateUser, ///< Set intersecting slices to current cursor position
    WidgetStateBlend, ///< Fade between foreground/background volumes
    WidgetStateTranslateSlice, ///< Pan (translate in-plane) the current slice (using shift+left-click-and-drag or middle-click-and-drag)
    WidgetStateRotateIntersectingSlices, ///< Rotate all intersecting slices (ctrl+alt+left-click-and-drag)
    WidgetStateZoomSlice, ///< Zoom slice (using right-button or mouse wheel)
    WidgetStateTouchGesture, ///< Pinch/zoom/pan using touch gestures

    // Interactions with slice intersection handles
    WidgetStateOnTranslateIntersectingSlicesHandle, ///< hovering over a slice intersection point
    WidgetStateTranslateIntersectingSlicesHandle, ///< translating all intersecting slices by drag-and-dropping handle
    WidgetStateOnRotateIntersectingSlicesHandle, ///< hovering over a rotation interaction handle
    WidgetStateRotateIntersectingSlicesHandle, ///< rotating all intersecting slices by drag-and-dropping handle
    WidgetStateOnTranslateSingleIntersectingSliceHandle, ///< hovering over a single-slice translation interaction handle
    WidgetStateTranslateSingleIntersectingSliceHandle, ///< translating a single slice by drag-and-dropping handle

    WidgetState_Last
    };

  /// Widget events
  enum
    {
    // Interactions without handles
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
    WidgetEventRotateIntersectingSlicesStart, // rotate all intersecting slices (ctrl-alt-left-click-and-drag)
    WidgetEventRotateIntersectingSlicesEnd,
    WidgetEventTranslateSliceStart,
    WidgetEventTranslateSliceEnd,
    WidgetEventZoomSliceStart,
    WidgetEventZoomSliceEnd,
    WidgetEventSetCrosshairPosition,
    WidgetEventMaximizeView,

    // Interactions with slice intersection handles
    // WidgetStateOnTranslateIntersectingSlicesHandle/WidgetStateTranslateIntersectingSlicesHandle
    WidgetEventTranslateIntersectingSlicesHandleStart,
    WidgetEventTranslateIntersectingSlicesHandleEnd,
    // WidgetStateOnRotateIntersectingSlicesHandle/WidgetStateRotateIntersectingSlicesHandle
    WidgetEventRotateIntersectingSlicesHandleStart,
    WidgetEventRotateIntersectingSlicesHandleEnd,
    // WidgetStateOnTranslateSingleIntersectingSliceHandle/WidgetStateOnTranslateSingleIntersectingSliceHandle
    WidgetEventTranslateSingleIntersectingSliceHandleStart,
    WidgetEventTranslateSingleIntersectingSliceHandleEnd,
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

  // Allows the widget to request a cursor shape
  int GetMouseCursor() override;

protected:
  vtkMRMLSliceIntersectionWidget();
  ~vtkMRMLSliceIntersectionWidget() override;

  bool ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData);
  bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData);
  bool ProcessBlend(vtkMRMLInteractionEventData* eventData);

  bool ProcessRotateIntersectingSlicesStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessRotateIntersectingSlices(vtkMRMLInteractionEventData* eventData);
  bool ProcessSetCrosshair(vtkMRMLInteractionEventData* eventData);
  double GetSliceRotationAngleRad(double eventPos[2]);

  // Pan (move slice in-plane) by click-and-drag
  bool ProcessTranslateSlice(vtkMRMLInteractionEventData* eventData);

  bool ProcessZoomSlice(vtkMRMLInteractionEventData* eventData);

  bool ProcessTouchGestureStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchGestureEnd(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchRotate(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchZoom(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchTranslate(vtkMRMLInteractionEventData* eventData);


  bool ProcessTranslateIntersectingSlicesHandleStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessTranslateIntersectingSlicesHandle(vtkMRMLInteractionEventData* eventData);
  bool ProcessTranslateSingleIntersectingSliceHandleStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessTranslateSingleIntersectingSliceHandle(vtkMRMLInteractionEventData* eventData);
  bool ProcessRotateIntersectingSlicesHandleStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessRotateIntersectingSlicesHandle(vtkMRMLInteractionEventData* eventData);

  bool ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData);

  bool ProcessMaximizeView(vtkMRMLInteractionEventData* eventData);

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

  static void SliceModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
  static void SliceLogicsModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

  vtkWeakPointer<vtkCollection> SliceLogics;
  vtkWeakPointer<vtkMRMLSliceNode> SliceNode;
  vtkWeakPointer<vtkMRMLSliceLogic> SliceLogic;
  vtkNew<vtkCallbackCommand> SliceLogicsModifiedCommand;
  vtkNew<vtkCallbackCommand> SliceModifiedCommand;

  double StartEventPosition[2];
  double PreviousRotationAngleRad;
  int PreviousEventPosition[2];
  double StartRotationCenter[2];
  double StartRotationCenter_RAS[4];

  double StartTranslationPoint[2];
  double StartTranslationPoint_RAS[3];
  double CurrentTranslationPoint_RAS[3];

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
  /// in a loop through the volumes available in the scene.
  ///  - layer: are 0,1,2 for bg, fg, lb
  ///  - direction: positive or negative (wraps through volumes in scene)
  void CycleVolumeLayer(int layer, int direction);

  bool IsSliceIntersectionInteractive();

  /// Indicates whether the shift key was used during the previous action.
  /// This is used to require shift-up after a click-and-drag before accepting shift+mousemove.
  bool ModifierKeyPressedSinceLastClickAndDrag;

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

  // Last intersecting slice node where interaction occurred
  std::string LastIntersectingSliceNodeID;

private:
  vtkMRMLSliceIntersectionWidget(const vtkMRMLSliceIntersectionWidget&) = delete;
  void operator=(const vtkMRMLSliceIntersectionWidget&) = delete;
};

#endif
