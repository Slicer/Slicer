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

#ifndef __vtkMRMLViewInteractorStyle_h
#define __vtkMRMLViewInteractorStyle_h

// VTK includes
#include "vtkObject.h"
#include "vtkWeakPointer.h"

// MRML includes
#include "vtkMRMLDisplayableManagerExport.h"

class vtkCallbackCommand;
class vtkEventData;
class vtkInteractorStyle;
class vtkMRMLAbstractDisplayableManager;
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLInteractionEventData;
class vtkRenderWindowInteractor;

/// \brief Common base class for processing interaction events in MRML views
///
/// Events are converted to new-style VTK interaction events and
/// forwarded to displayable managers for processing.
/// Some additional high-level events (such as click and double-click)
/// are generated here.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLViewInteractorStyle : public vtkObject
{
public:
  static vtkMRMLViewInteractorStyle* New();
  vtkTypeMacro(vtkMRMLViewInteractorStyle, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Mouse functions
  virtual void OnMouseMove();
  virtual void OnEnter();
  virtual void OnLeave();
  virtual void OnLeftButtonDoubleClick();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDoubleClick();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDoubleClick();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  virtual void OnMouseWheelForward();
  virtual void OnMouseWheelBackward();

  // Touch gesture interaction events
  virtual void OnStartPinch();
  virtual void OnPinch();
  virtual void OnEndPinch();
  virtual void OnStartRotate();
  virtual void OnRotate();
  virtual void OnEndRotate();
  virtual void OnStartPan();
  virtual void OnPan();
  virtual void OnEndPan();
  virtual void OnTap();
  virtual void OnLongTap();

  /// Keyboard functions
  virtual void OnChar();
  virtual void OnKeyPress();
  virtual void OnKeyRelease();

  /// 3D event bindings
  virtual void OnButton3D(vtkEventData* eventData);
  virtual void OnMove3D(vtkEventData* eventData);

  virtual void OnExpose();
  virtual void OnConfigure();

  virtual void SetDisplayableManagers(vtkMRMLDisplayableManagerGroup* displayableManagers);

  /// Give a chance to displayable managers to process the event.
  /// It just creates vtkMRMLInteractionEventData and calls
  /// DelegateInteractionEventDataToDisplayableManagers.
  /// Return true if the event is processed.
  virtual bool DelegateInteractionEventToDisplayableManagers(unsigned long event);

  /// Give a chance to displayable managers to process the event.
  /// It just creates vtkMRMLInteractionEventData and calls
  /// DelegateInteractionEventDataToDisplayableManagers.
  /// Return true if the event is processed.
  virtual bool DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData);

  /// Give a chance to displayable managers to process the event.
  /// Return true if the event is processed.
  virtual bool DelegateInteractionEventDataToDisplayableManagers(vtkMRMLInteractionEventData* eventData);

  vtkGetObjectMacro(Interactor, vtkRenderWindowInteractor);
  virtual void SetInteractor(vtkRenderWindowInteractor* interactor);

protected:
  vtkMRMLViewInteractorStyle();
  ~vtkMRMLViewInteractorStyle() override;

  vtkRenderWindowInteractor* Interactor{ nullptr };
  vtkInteractorStyle* GetInteractorStyle();

  vtkCallbackCommand* EventCallbackCommand;

  /// Main process event method.
  ///
  /// Set MouseMovedSinceButtonDown variable based on the mouse events.
  ///
  /// Then, if calling DelegateInteractionEventToDisplayableManagers() returns false or if the current motion flag
  /// returned by vtkInteractorStyle::GetState() is VTKIS_NONE, call ProcessEvents().
  static void CustomProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  /// Process events not already delegated to displayable managers by CustomProcessEvents().
  static void ProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  vtkCallbackCommand* DisplayableManagerCallbackCommand;
  static void DisplayableManagerCallback(vtkObject* object, unsigned long event, void* clientData, void* callData);
  virtual void ProcessDisplayableManagerEvents(vtkMRMLAbstractDisplayableManager* displayableManager,
                                               unsigned long event,
                                               void* callData);

  void SetMouseCursor(int cursor);

  bool MouseMovedSinceButtonDown;

  vtkWeakPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagers;
  vtkMRMLAbstractDisplayableManager* FocusedDisplayableManager;

private:
  vtkMRMLViewInteractorStyle(const vtkMRMLViewInteractorStyle&) = delete;
  void operator=(const vtkMRMLViewInteractorStyle&) = delete;
};

#endif
