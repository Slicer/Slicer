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
 * @class   vtkMRMLAbstractWidget
 * @brief   Process interaction events to update state of MRML widget nodes
 *
 * vtkMRMLAbstractWidget is the abstract class that handles interaction events
 * received from the displayable manager. To decide which widget gets the chance
 * to process an interaction event, this class does not use VTK picking manager,
 * but interactor style class queries displayable managers about what events they can
 * process, displayable manager queries its widgets, and based on the returned
 * information, interactor style selects a displayable manager and the displayable
 * manager selects a widget.
 *
 * vtkAbstractWidget uses vtkSlicerWidgetEventTranslator to translate VTK
 * interaction events (defined in vtkCommand.h) into widget events (defined in
 * vtkMRMLAbstractWidget.h and subclasses). This class allows modification
 * of event bindings.
 *
 * @sa
 * vtkSlicerWidgetRepresentation vtkSlicerWidgetEventTranslator
 *
*/

#ifndef vtkMRMLAbstractWidget_h
#define vtkMRMLAbstractWidget_h

#include "vtkMRMLDisplayableManagerExport.h"
#include "vtkObject.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"
#include <vector>

class vtkMRMLAbstractViewNode;
class vtkMRMLApplicationLogic;
class vtkMRMLInteractionEventData;
class vtkMRMLInteractionNode;
class vtkMRMLNode;
class vtkMRMLAbstractWidgetRepresentation;
class vtkRenderer;
class vtkWidgetEventTranslator;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractWidget : public vtkObject
{
public:
  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkMRMLAbstractWidget, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual void SetMRMLApplicationLogic(vtkMRMLApplicationLogic* applicationLogic);
  vtkMRMLApplicationLogic* GetMRMLApplicationLogic();

  /// Set the representation.
  /// The widget takes over the ownership of this actor.
  virtual void SetRepresentation(vtkMRMLAbstractWidgetRepresentation *r);

  /// Get the representation
  virtual vtkMRMLAbstractWidgetRepresentation *GetRepresentation();

  /// Build the actors of the representation with the info stored in the MRML scene
  virtual void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData = nullptr);

  /// Convenient method to change what state the widget is in.
  vtkSetMacro(WidgetState,int);

  /// Convenient method to determine the state of the method
  vtkGetMacro(WidgetState,int);

  /// The state of the widget
  enum
  {
    WidgetStateAny, // this state is used for referring to any widget state (for defining event translations)
    WidgetStateIdle, // mouse pointer is outside the widget, click does not do anything
    WidgetStateOnWidget, // mouse pointer is over the widget, clicking will add a point or manipulate the line
    WidgetStateTranslate, // mouse move transforms the entire widget
    WidgetStateRotate,     // mouse move transforms the entire widget
    WidgetStateScale,     // mouse move transforms the entire widget
    WidgetStateUser // this is a starting index that can be used for widget-specific states
  };

  /// Widget events
  enum WidgetEvents
  {
    WidgetEventNone,
    WidgetEventMouseMove,
    WidgetEventTranslateStart,
    WidgetEventTranslateEnd,
    WidgetEventRotateStart,
    WidgetEventRotateEnd,
    WidgetEventScaleStart,
    WidgetEventScaleEnd,
    // MRML events
    WidgetEventPick, // generates a MRML Pick event (e.g., on left click)
    WidgetEventJumpCursor, // jumps cursor to the selected position
    WidgetEventAction, // generates a MRML Action event (e.g., left double-click)
    WidgetEventCustomAction1, // generates a MRML CustomAction1 event (allows modules to define custom widget actions and get notification via MRML node event)
    WidgetEventCustomAction2, // generates a MRML CustomAction1 event
    WidgetEventCustomAction3, // generates a MRML CustomAction1 event
    WidgetEventSelect, // change MRML node Selected attribute
    WidgetEventUnselect, // change MRML node Selected attribute
    WidgetEventToggleSelect, // change MRML node Selected attribute
    // Other actions
    WidgetEventMenu, // show context menu
    WidgetEventReset, // reset widget to initial state (clear all points)
    WidgetEventUser // this is a starting index that can be used for widget-specific events
  };

  /// Return true if the widget can process the event.
  /// Distance2 is the squared distance in display coordinates from the closest interaction position.
  /// The displayable manager with the closest distance will get the chance to process the interaction event.
  virtual bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2);

  /// Allows injecting interaction events for processing, without directly observing window interactor events.
  /// Return true if the widget processed the event.
  virtual bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData);

  /// Define interaction event to widget event translation for mouse and other controller events
  /// Used in the specified widget state only.
  void SetEventTranslation(int widgetState, unsigned long interactionEvent, int modifiers, unsigned long widgetEvent);

  void SetEventTranslationClickAndDrag(int widgetState, unsigned long startInteractionEvent, int modifiers,
    int widgetStateDragging, unsigned long widgetStartEvent, unsigned long widgetEndEvent);

  /// Define interaction event to widget event translation for mouse and other controller events.
  /// Used in any widget state.
  void SetEventTranslation(unsigned long interactionEvent, int modifiers, unsigned long widgetEvent);

  /// Define interaction event to widget event translation for keyboard events.
  /// Used in any widget state.
  void SetKeyboardEventTranslation(int modifier, char keyCode, int repeatCount, const char* keySym, unsigned long widgetEvent);

  /// Define interaction event to widget event translation for keyboard events.
  /// Used in the specified widget state only.
  void SetKeyboardEventTranslation(int widgetState, int modifier, char keyCode, int repeatCount, const char* keySym, unsigned long widgetEvent);

  /// Get widget event from translation event
  virtual unsigned long TranslateInteractionEventToWidgetEvent(vtkMRMLInteractionEventData* eventData);

  /// Called when the the widget loses the focus.
  virtual void Leave(vtkMRMLInteractionEventData* eventData);

  void SetRenderer(vtkRenderer* renderer);
  vtkRenderer* GetRenderer();

  vtkMRMLInteractionNode* GetInteractionNode();

  // Allows the widget to request a cursor shape
  virtual int GetMouseCursor();

  // Allows the widget to request grabbing of all events (even when the mouse pointer moves out of view)
  virtual bool GetGrabFocus();

  // Allows the widget to request interactive mode (faster updates)
  virtual bool GetInteractive();

  // Allows the widget to request render
  virtual bool GetNeedToRender();

  // Acknowledge rendering request (rendering completed)
  virtual void NeedToRenderOff();

protected:
  vtkMRMLAbstractWidget();
  ~vtkMRMLAbstractWidget() override;

  /// Get ID of the node at the specified event position.
  /// Returns nullptr if nothing can be found.
  const char* GetAssociatedNodeID(vtkMRMLInteractionEventData* eventData);

  /// Helper function that attempts to translate an event with a specific translator only.
  unsigned long TranslateInteractionEventToWidgetEvent(
    vtkWidgetEventTranslator* translator, vtkMRMLInteractionEventData* eventData);

  vtkWeakPointer<vtkRenderer> Renderer;

  vtkWeakPointer<vtkMRMLApplicationLogic> ApplicationLogic;

  // Translates interaction event to widget event.
  // In the future, a vector of event translators could be added
  // (one for each state) to be able to define events
  // that are only allowed in a specific state.
  std::vector< vtkSmartPointer<vtkWidgetEventTranslator> > EventTranslators;

  int WidgetState;

  vtkSmartPointer<vtkMRMLAbstractWidgetRepresentation> WidgetRep;

//  bool NeedToRender;

private:
  vtkMRMLAbstractWidget(const vtkMRMLAbstractWidget&) = delete;
  void operator=(const vtkMRMLAbstractWidget&) = delete;
};

#endif
