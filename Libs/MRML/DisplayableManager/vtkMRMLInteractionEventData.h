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

#ifndef __vtkMRMLInteractionEventData_h
#define __vtkMRMLInteractionEventData_h

// VTK includes
#include "vtkEventData.h"

// MRML includes
#include "vtkMRMLDisplayableManagerExport.h"

class vtkMRMLAbstractViewNode;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkCellPicker;

/// Class for storing all relevant details of mouse and keyboard events.
/// It stores additional information that is expensive to compute (such as 3D position)
/// or not always easy to get (such as modifiers).
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLInteractionEventData : public vtkEventData
{
public:
  vtkTypeMacro(vtkMRMLInteractionEventData, vtkEventData);
  static vtkMRMLInteractionEventData *New();

  /// Extends vtkCommand events
  enum MRMLInteractionEvents
    {
    LeftButtonClickEvent = vtkCommand::UserEvent + 300, // button press and release without moving mouse
    MiddleButtonClickEvent,
    RightButtonClickEvent,
    };

  void SetType(unsigned long v);

  void SetModifiers(int v);
  int GetModifiers();

  void GetWorldPosition(double v[3]) const;
  const double *GetWorldPosition() const;

  // It may be expensive to compute world position accurately (e.g., in a 3D view).
  // If accurate parameter is set to false then it indicates
  // that the position may be inaccurate.
  void SetWorldPosition(const double p[3], bool accurate = true);
  bool IsWorldPositionValid();
  bool IsWorldPositionAccurate();
  void SetWorldPositionInvalid();

  bool ComputeAccurateWorldPosition(bool force = false);

  void GetDisplayPosition(int v[2]) const;
  const int *GetDisplayPosition() const;
  void SetDisplayPosition(const int p[2]);
  bool IsDisplayPositionValid();
  void SetDisplayPositionValid();

  void SetKeyCode(char v);
  char GetKeyCode();
  void SetKeyRepeatCount(char v);
  int GetKeyRepeatCount();
  void SetKeySym(const std::string &v);
  const std::string& GetKeySym();

  void SetViewNode(vtkMRMLAbstractViewNode* viewNode);
  vtkMRMLAbstractViewNode* GetViewNode() const;

  void SetComponentType(int componentType);
  int GetComponentType() const;

  void SetComponentIndex(int componentIndex);
  int GetComponentIndex() const;

  void SetRotation(double v);
  double GetRotation() const;
  void SetLastRotation(double v);
  double GetLastRotation() const;

  /// Set Modifiers and Key... attributes from interactor
  void SetAttributesFromInteractor(vtkRenderWindowInteractor* interactor);

  vtkRenderer* GetRenderer() const;
  void SetRenderer(vtkRenderer* ren);

  void SetAccuratePicker(vtkCellPicker* picker);
  vtkCellPicker* GetAccuratePicker() const;

protected:
  int Modifiers;
  int DisplayPosition[2];
  double WorldPosition[3];
  bool DisplayPositionValid;
  bool WorldPositionValid;
  bool WorldPositionAccurate;
  bool ComputeAccurateWorldPositionAttempted;
  vtkMRMLAbstractViewNode* ViewNode;
  vtkRenderer* Renderer;
  vtkCellPicker* AccuratePicker;
  int ComponentType;
  int ComponentIndex;

  // For KeyPressEvent
  char KeyCode;
  int KeyRepeatCount;
  std::string KeySym;

  // MacOSX touchpad events
  double Rotation;
  double LastRotation;

  bool Equivalent(const vtkEventData *e) const override;

  vtkMRMLInteractionEventData();
  ~vtkMRMLInteractionEventData() override  = default;

private:
  vtkMRMLInteractionEventData(const vtkMRMLInteractionEventData& c) = delete;
  void operator=(const vtkMRMLInteractionEventData&) = delete;
};

#endif
