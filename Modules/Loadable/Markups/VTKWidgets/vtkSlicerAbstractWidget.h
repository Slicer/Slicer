/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

/**
 * @class   vtkSlicerAbstractWidget
 * @brief   Process interaction events to update state of MRML widget nodes
 *
 * vtkSlicerAbstractWidget is the abstract class that handles interaction events
 * received from the displayable manager. To decide which widget gets the chance
 * to process an interaction event, this class does not use VTK picking manager,
 * but interactor style class queries displayable managers about what events they can
 * process, displayable manager queries its widgets, and based on the returned
 * information, interactor style selects a displayable manager and the displayable
 * manager selects a widget.
 *
 * vtkAbstractWidget uses vtkSlicerWidgetEventTranslator to translate VTK
 * interaction events (defined in vtkCommand.h) into widget events (defined in
 * vtkSlicerAbstractWidget.h and subclasses). This class allows modification
 * of event bindings.
 *
 * @sa
 * vtkSlicerWidgetRepresentation vtkSlicerWidgetEventTranslator
 *
*/

#ifndef vtkSlicerAbstractWidget_h
#define vtkSlicerAbstractWidget_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkAbstractWidget.h"
#include "vtkWidgetCallbackMapper.h"

#include "vtkMRMLMarkupsNode.h"

class vtkMRMLAbstractViewNode;
class vtkMRMLApplicationLogic;
class vtkMRMLInteractionEventData;
class vtkMRMLInteractionNode;
class vtkIdList;
class vtkPolyData;
class vtkSlicerAbstractWidgetRepresentation;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerAbstractWidget : public vtkObject
{
public:
  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkSlicerAbstractWidget, vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Create the default widget representation and initializes the widget and representation.
  virtual void CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer) = 0;

  void SetMRMLApplicationLogic(vtkMRMLApplicationLogic* applicationLogic);
  vtkMRMLApplicationLogic* GetMRMLApplicationLogic();

  /// Set the representation.
  /// The widget takes over the ownership of this actor.
  virtual void SetRepresentation(vtkSlicerAbstractWidgetRepresentation *r);

  /// Get the representation
  virtual vtkSlicerAbstractWidgetRepresentation *GetRepresentation();

  /// Build the actors of the representation with the info stored in the vtkMRMLMarkupsNode
  virtual void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData = NULL);

  /// Convenient method to change what state the widget is in.
  vtkSetMacro(WidgetState,int);

  /// Convenient method to determine the state of the method
  vtkGetMacro(WidgetState,int);

  /// Convenient method to remap the horizonbtal axes constrain key.
  vtkSetMacro(HorizontalActiveKeyCode, const char);
  vtkGetMacro(HorizontalActiveKeyCode, char);

  /// Convenient method to remap the vertical axes constrain key.
  vtkSetMacro(VerticalActiveKeyCode, const char);
  vtkGetMacro(VerticalActiveKeyCode, char);

  /// Convenient method to remap the depth axes constrain key.
  vtkSetMacro(DepthActiveKeyCode, const char);
  vtkGetMacro(DepthActiveKeyCode, char);

  /// The state of the widget
  enum
  {
    Idle, // mouse pointer is outside the widget, click does not do anything
    Define, // click in empty area will place a new point
    OnWidget, // mouse pointer is over the widget, clicking will add a point or manipulate the line
    TranslateControlPoint, // translating the active point by mouse move
    Translate, // mouse move transforms the entire widget
    Scale,     // mouse move transforms the entire widget
    Rotate,     // mouse move transforms the entire widget
  };

  /// Widget events
  enum WidgetEvents
  {
    WidgetNoEvent,
    WidgetMouseMove,
    WidgetControlPointMoveStart,
    WidgetControlPointMoveEnd,
    WidgetControlPointDelete,
    WidgetTranslateStart,
    WidgetTranslateEnd,
    WidgetRotateStart,
    WidgetRotateEnd,
    WidgetScaleStart,
    WidgetScaleEnd,
    WidgetControlPointInsert,
    // MRML events
    WidgetPick, // generates a MRML Pick event (e.g., on left click)
    WidgetJumpCursor, // jumps cursor to the control point position
    WidgetAction, // generates a MRML Action event (e.g., left double-click)
    WidgetCustomAction1, // generates a MRML CustomAction1 event (allows modules to define custom widget actions and get notification via MRML node event)
    WidgetCustomAction2, // generates a MRML CustomAction1 event
    WidgetCustomAction3, // generates a MRML CustomAction1 event
    WidgetSelect, // change MRML node Selected attribute
    WidgetUnselect, // change MRML node Selected attribute
    WidgetToggleSelect, // change MRML node Selected attribute
    // Other actions
    WidgetMenu, // show context menu
    WidgetReset, // reset widget to initial state (clear all points)
  };

  // Returns true if one of the markup points are just being previewed and not placed yet.
  bool IsPointPreviewed();

  /// Add/update a point preview to the current active Markup at the specified position.
  void UpdatePreviewPoint(const int displayPos[2], const double worldPos[3], const char* associatedNodeID);

  /// Remove the point preview to the current active Markup.
  /// Returns true is preview point existed and now it is removed.
  bool RemovePreviewPoint();

  // Places a new markup point.
  // Reuses current preview point, if possible.
  // Returns true if the event is processed.
  bool PlacePoint(vtkMRMLInteractionEventData* eventData);

  /// Add a point to the current active Markup at input World coordiantes.
  virtual int AddPointFromWorldCoordinate(const double worldCoordinates[3]);

  /// Given a specific X, Y pixel location, add a new node
  /// on the widget at this location.
  virtual int AddNodeOnWidget(const int displayPos[2]);

  /// Return true if the widget can process the event.
  /// Distance2 is the squared distance in display coordinates from the closest interaction position.
  /// The displayable manager with the closest distance will get the chance to process the interaction event.
  virtual bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2);

  /// Allows injecting interaction events for processing, without directly observing window interactor events.
  /// Return true if the widget processed the event.
  virtual bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData);

  virtual unsigned long TranslateInteractionEventToWidgetEvent(vtkMRMLInteractionEventData* eventData);

  /// Called when the the widget loses the focus.
  virtual void Leave();

  void SetEventTranslation(unsigned long interactionEvent, int modifiers, unsigned long widgetEvent);
  void SetKeyboardEventTranslation(int modifier, char keyCode, int repeatCount, const char* keySym, unsigned long widgetEvent);

  void SetRenderer(vtkRenderer* renderer);
  vtkGetMacro(Renderer, vtkRenderer*);

  vtkMRMLInteractionNode* GetInteractionNode();

  // Allows the widget to request a cursor shape
  virtual int GetCursor();

  // Allows the widget to request grabbing of all events (even when the mouse pointer moves out of view)
  virtual bool GetGrabFocus();

  // Allows the widget to request interactive mode (faster updates)
  virtual bool GetInteractive();

  // Allows the widget to request render
  bool GetNeedToRender();

  // Acknowledge rendering request (rendering completed)
  void NeedToRenderOff();

  vtkMRMLMarkupsNode* GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* GetMarkupsDisplayNode();
  int GetActiveControlPoint();

protected:
  vtkSlicerAbstractWidget();
  ~vtkSlicerAbstractWidget() VTK_OVERRIDE;

  void StartWidgetInteraction(vtkMRMLInteractionEventData* eventData);
  void EndWidgetInteraction();

  virtual void TranslatePoint(double eventPos[2]);
  virtual void TranslateWidget(double eventPos[2]);
  virtual void ScaleWidget(double eventPos[2]);
  virtual void RotateWidget(double eventPos[2]);

  bool IsAnyControlPointLocked();

  /// Get ID of the node at the specified event position.
  /// Returns nullptr if nothing can be found.
  const char* GetAssociatedNodeID(vtkMRMLInteractionEventData* eventData);

  // Get accurate world position.
  // World position that comes in the event data may be inaccurate, this method computes a more reliable position.
  // Returns true on success.
  // refWorldPos is an optional reference position: if point distance from camera cannot be determined then
  // depth of this reference position is used.
  bool ConvertDisplayPositionToWorld(const int displayPos[2], double worldPos[3], double worldOrientationMatrix[9],
    double* refWorldPos = nullptr);

  vtkRenderer* Renderer;

  vtkMRMLApplicationLogic* ApplicationLogic;

  // Translates interaction event to widget event.
  // In the future, a vector of event translators could be added
  // (one for each state) to be able to define events
  // that are only allowed in a specific state.
  vtkSmartPointer<vtkWidgetEventTranslator> EventTranslator;

  int WidgetState;

  /// Index of the control point that is currently being previewed (follows the mouse pointer).
  /// If <0 it means that there is currently no point being previewed.
  int PreviewPointIndex;

  // Callback interface to capture events when
  // placing the widget.
  // Return true if the event is processed.
  virtual bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessControlPointDelete(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessControlPointMove(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessControlPointInsert(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetReset(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetRotate(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetScale(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetTranslate(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetJumpCursor(vtkMRMLInteractionEventData* eventData);

  // Manual axis constrain
  char HorizontalActiveKeyCode;
  char VerticalActiveKeyCode;
  char DepthActiveKeyCode;
  vtkSmartPointer<vtkCallbackCommand> KeyEventCallbackCommand;
  static void ProcessKeyEvents(vtkObject *, unsigned long, void *, void *);

  // Variables for translate/rotate/scale
  double LastEventPosition[2];
  double StartEventOffsetPosition[2];

  // True if mouse button pressed since a point was placed.
  // This is used to filter out "click" events that started before the point was placed.
  bool MousePressedSinceMarkupPlace;

  vtkSmartPointer<vtkSlicerAbstractWidgetRepresentation> WidgetRep;

private:
  vtkSlicerAbstractWidget(const vtkSlicerAbstractWidget&) = delete;
  void operator=(const vtkSlicerAbstractWidget&) = delete;
};

#endif
