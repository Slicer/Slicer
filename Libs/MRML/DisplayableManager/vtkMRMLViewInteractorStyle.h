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
#include "vtkInteractorStyle3D.h"
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"

// MRML includes
#include "vtkMRMLDisplayableManagerExport.h"

class vtkMRMLAbstractDisplayableManager;
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLInteractionEventData;
class vtkTimerLog;

/// \brief Common base class for processing interaction events in MRML views
///
/// Events are converted to new-style VTK interaction events and
/// forwarded to displayable managers for processing.
/// Some additional high-level events (such as click and double-click)
/// are generated here.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLViewInteractorStyle :
  public vtkInteractorStyle3D
{
public:
  static vtkMRMLViewInteractorStyle *New();
  vtkTypeMacro(vtkMRMLViewInteractorStyle,vtkInteractorStyle3D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void OnMouseMove() override;
  void OnEnter() override;
  void OnLeave() override;
  void OnLeftButtonDown() override;
  void OnLeftButtonUp() override;
  void OnMiddleButtonDown() override;
  void OnMiddleButtonUp() override;
  void OnRightButtonDown() override;
  void OnRightButtonUp() override;
  void OnMouseWheelForward() override;
  void OnMouseWheelBackward() override;

  // Touch gesture interaction events
  void OnPinch() override;
  void OnRotate() override;
  void OnPan() override;
  void OnTap() override;
  void OnLongTap() override;

  /// Keyboard functions
  void OnChar() override;
  void OnKeyPress() override;
  void OnKeyRelease() override;

  /// 3D event bindings
  void OnButton3D(vtkEventData* eventData) override;
  void OnMove3D(vtkEventData* eventData) override;

  void OnExpose() override;
  void OnConfigure() override;

  void SetDisplayableManagers(vtkMRMLDisplayableManagerGroup* displayableManagers);

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

  ///
  /// Reimplemented to set additional observers
  void SetInteractor(vtkRenderWindowInteractor *interactor) override;

protected:
  vtkMRMLViewInteractorStyle();
  ~vtkMRMLViewInteractorStyle() override;

  static void CustomProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  void SetMouseCursor(int cursor);

  bool MouseMovedSinceButtonDown;

  /// Measures time elapsed since first button press.
  vtkSmartPointer<vtkTimerLog> ClickTimer;
  int NumberOfClicks;
  double DoubleClickIntervalTimeSec;

  vtkWeakPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagers;
  vtkMRMLAbstractDisplayableManager* FocusedDisplayableManager;

private:
  vtkMRMLViewInteractorStyle(const vtkMRMLViewInteractorStyle&) = delete;
  void operator=(const vtkMRMLViewInteractorStyle&) = delete;
};

#endif
