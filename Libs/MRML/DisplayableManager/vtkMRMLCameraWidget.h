/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

/**
 * @class   vtkMRMLCameraWidget
 * @brief   Process camera manipulation events.
 *
 * This widget does not have a visible representation, only translates
 * and processes camera manipulation (rotate, translate, etc.) events.
 * It is implemented as a widget instead of an interactor style so that
 * mouse and keyboard interaction events can be dynamically remapped
 * to camera manipulation actions and to process all kinds of interaction
 * events (camera manipulation, markups manipulation, ...) in a similar way.
 */

#ifndef vtkMRMLCameraWidget_h
#define vtkMRMLCameraWidget_h

#include "vtkMRMLDisplayableManagerExport.h" // For export macro
#include "vtkMRMLAbstractWidget.h"
#include "vtkMRMLCameraNode.h"

class vtkSliceIntersectionRepresentation2D;
class vtkMRMLSegmentationDisplayNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLCameraWidget : public vtkMRMLAbstractWidget
{
public:
  /**
   * Instantiate this class.
   */
  static vtkMRMLCameraWidget* New();

  //@{
  /**
   * Standard VTK class macros.
   */
  vtkTypeMacro(vtkMRMLCameraWidget, vtkMRMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation();

  void SetCameraNode(vtkMRMLCameraNode* cameraNode);
  vtkMRMLCameraNode* GetCameraNode();

  /// Return true if the widget can process the event.
  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) override;

  /// Process interaction event.
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  //@{
  /// Get set tilt lock. It tilt is locked then the view cannot be rotated around the horizontal axis
  /// (can only be rotated along the vertical axis).
  bool GetTiltLocked();
  void SetTiltLocked(bool lockState);
  //@}

  /// Widget states
  enum
  {
    WidgetStateMoveCrosshair =
      WidgetStateUser, ///< Move crosshair position, can be used for moving the crosshair with click-and-drag.
    WidgetStateSpin,
    WidgetStateTouchGesture
  };

  /// Widget events
  enum
  {
    WidgetEventSpinStart = WidgetEventUser,
    WidgetEventSpinEnd,

    WidgetEventMoveCrosshairStart,
    WidgetEventMoveCrosshairEnd,

    WidgetEventCameraRotateToRight,
    WidgetEventCameraRotateToLeft,
    WidgetEventCameraRotateToAnterior,
    WidgetEventCameraRotateToPosterior,
    WidgetEventCameraRotateToSuperior,
    WidgetEventCameraRotateToInferior,

    WidgetEventCameraTranslateForwardX,
    WidgetEventCameraTranslateBackwardX,
    WidgetEventCameraTranslateForwardY,
    WidgetEventCameraTranslateBackwardY,
    WidgetEventCameraTranslateForwardZ,
    WidgetEventCameraTranslateBackwardZ,

    WidgetEventCameraRotateCcwX,
    WidgetEventCameraRotateCwX,
    WidgetEventCameraRotateCcwY,
    WidgetEventCameraRotateCwY,
    WidgetEventCameraRotateCcwZ,
    WidgetEventCameraRotateCwZ,

    WidgetEventCameraZoomIn,
    WidgetEventCameraZoomOut,
    WidgetEventCameraWheelZoomIn, // same as WidgetEventCameraZoomIn but with using wheel scaling factor
    WidgetEventCameraWheelZoomOut,

    WidgetEventToggleCameraTiltLock,

    WidgetEventCameraReset,
    WidgetEventCameraResetTranslation,
    WidgetEventCameraResetRotation,
    WidgetEventCameraResetFieldOfView, // VTK's standard camera reset (centers and resets field of view)

    WidgetEventCameraRotate,
    WidgetEventCameraPan,

    WidgetEventTouchGestureStart,
    WidgetEventTouchGestureEnd,
    WidgetEventTouchSpinCamera,
    WidgetEventTouchPinchZoom,
    WidgetEventTouchPanTranslate,

    WidgetEventSetCrosshairPosition,
    WidgetEventSetCrosshairPositionBackground, //< set crosshair position without consuming the event (so that other
                                               // widgets can process the event)
    WidgetEventMaximizeView,
  };

  /// Defines speed of rotation actions by mouse click-and-drag.
  vtkGetMacro(MotionFactor, double);
  vtkSetMacro(MotionFactor, double);

  /// Defines step size for mouse wheel actions.
  vtkGetMacro(MouseWheelMotionFactor, double);
  vtkSetMacro(MouseWheelMotionFactor, double);

protected:
  vtkMRMLCameraWidget();
  ~vtkMRMLCameraWidget() override;

  bool ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData);
  bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData);

  bool ProcessTranslate(vtkMRMLInteractionEventData* eventData);
  bool ProcessRotate(vtkMRMLInteractionEventData* eventData);
  bool ProcessScale(vtkMRMLInteractionEventData* eventData);
  bool ProcessSpin(vtkMRMLInteractionEventData* eventData);
  bool ProcessSetCrosshair(vtkMRMLInteractionEventData* eventData);
  bool ProcessSetCrosshairBackground(vtkMRMLInteractionEventData* eventData);

  bool ProcessTouchGestureStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchGestureEnd(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchCameraSpin(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchCameraZoom(vtkMRMLInteractionEventData* eventData);
  bool ProcessTouchCameraTranslate(vtkMRMLInteractionEventData* eventData);

  bool ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData);

  bool ProcessMaximizeView(vtkMRMLInteractionEventData* eventData);

  bool Dolly(double factor);
  vtkCamera* GetCamera();

  bool CameraModifyStart();
  void CameraModifyEnd(bool wasModified, bool updateClippingRange, bool updateLights);

  void SaveStateForUndo();

  double MotionFactor;
  double MouseWheelMotionFactor;
  bool CameraTiltLocked;

  vtkWeakPointer<vtkMRMLCameraNode> CameraNode;

  double StartEventPosition[2];
  int PreviousEventPosition[2];

  /// Indicates whether the shift key was used during the previous action.
  /// This is used to require shift-up after a click-and-drag before accepting shift+mousemove.
  bool ModifierKeyPressedSinceLastClickAndDrag;

private:
  vtkMRMLCameraWidget(const vtkMRMLCameraWidget&) = delete;
  void operator=(const vtkMRMLCameraWidget&) = delete;
};

#endif
