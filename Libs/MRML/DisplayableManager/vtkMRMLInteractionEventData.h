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
#include "vtkCommand.h"
#include "vtkEvent.h"
#include "vtkEventData.h"
#include "vtkInteractorStyleUser.h"
#include "vtkMatrix4x4.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkWeakPointer.h"

// MRML includes
#include "vtkMRMLDisplayableManagerExport.h"

class vtkMRMLAbstractDisplayableManager;
class vtkMRMLSegmentationDisplayNode;
class vtkMRMLSliceLogic;
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLAbstractViewNode;

/// Class for storing all relevant details of mouse and keyboard events.
/// It stores additional information that is expensive to compute (such as 3D position)
/// or not always easy to get (such as modifiers).
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLInteractionEventData : public vtkEventData
{
public:
  vtkTypeMacro(vtkMRMLInteractionEventData, vtkEventData);
  static vtkMRMLInteractionEventData *New()
    {
    vtkMRMLInteractionEventData *ret = new vtkMRMLInteractionEventData;
    ret->InitializeObjectBase();
    return ret;
    };

  /// Extends vtkCommand events
  enum MRMLInteractionEvents
    {
    LeftButtonClickEvent = vtkCommand::UserEvent + 300, // button press and release without moving mouse
    MiddleButtonClickEvent,
    RightButtonClickEvent
    };

  void SetType(unsigned long v) { this->Type = v; }

  void SetModifiers(int v) { this->Modifiers = v; }
  int GetModifiers() { return this->Modifiers; }

  void GetWorldPosition(double v[3]) const
    {
    std::copy(this->WorldPosition, this->WorldPosition + 3, v);
    }
  const double *GetWorldPosition() const
    {
    return this->WorldPosition;
    }
  void SetWorldPosition(const double p[3])
    {
    this->WorldPosition[0] = p[0];
    this->WorldPosition[1] = p[1];
    this->WorldPosition[2] = p[2];
    this->WorldPositionValid = true;
    }
  bool IsWorldPositionValid()
    {
    return this->WorldPositionValid;
    }
  void SetWorldPositionInvalid()
    {
    this->WorldPositionValid = false;
    }

  void GetDisplayPosition(int v[2]) const
    {
    std::copy(this->DisplayPosition, this->DisplayPosition + 2, v);
    }
  const int *GetDisplayPosition() const
    {
    return this->DisplayPosition;
    }
  void SetDisplayPosition(const int p[2])
    {
    this->DisplayPosition[0] = p[0];
    this->DisplayPosition[1] = p[1];
    this->DisplayPositionValid = true;
    }
  bool IsDisplayPositionValid()
    {
    return this->DisplayPositionValid;
    }
  void SetDisplayPositionValid()
    {
    this->DisplayPositionValid = false;
    }

  void SetKeyCode(char v) { this->KeyCode = v; }
  char GetKeyCode() { return this->KeyCode; }
  void SetKeyRepeatCount(char v) { this->KeyRepeatCount = v; }
  int GetKeyRepeatCount() { return this->KeyRepeatCount; }
  void SetKeySym(const std::string &v) { this->KeySym = v; }
  const std::string& GetKeySym() { return this->KeySym; }

  void SetViewNode(vtkMRMLAbstractViewNode* viewNode) { this->ViewNode = viewNode; }
  vtkMRMLAbstractViewNode* GetViewNode() { return this->ViewNode; }

  void SetComponentType(int componentType) { this->ComponentType = componentType; }
  int GetComponentType() { return this->ComponentType; }

  void SetComponentIndex(int componentIndex) { this->ComponentIndex = componentIndex; }
  int GetComponentIndex() { return this->ComponentIndex; }

  void SetRotation(double v) { this->Rotation = v; }
  double GetRotation() { return this->Rotation; }
  void SetLastRotation(double v) { this->LastRotation = v; }
  double GetLastRotation() { return this->LastRotation; }

  /// Set Modifiers and Key... attributes from interactor
  void SetAttributesFromInteractor(vtkRenderWindowInteractor* interactor)
    {
    this->Modifiers = 0;
    if (interactor->GetShiftKey())
      {
      this->Modifiers |= vtkEvent::ShiftModifier;
      }
    if (interactor->GetControlKey())
      {
      this->Modifiers |= vtkEvent::ControlModifier;
      }
    if (interactor->GetAltKey())
      {
      this->Modifiers |= vtkEvent::AltModifier;
      }
    this->KeyCode = interactor->GetKeyCode();
    this->KeySym = (interactor->GetKeySym() ? interactor->GetKeySym() : "");
    this->KeyRepeatCount = interactor->GetRepeatCount();

    this->Rotation = interactor->GetRotation();
    this->LastRotation = interactor->GetLastRotation();
    }

protected:
  int Modifiers;
  int DisplayPosition[2];
  double WorldPosition[3];
  bool DisplayPositionValid;
  bool WorldPositionValid;
  vtkMRMLAbstractViewNode* ViewNode;
  int ComponentType;
  int ComponentIndex;

  // For KeyPressEvent
  char KeyCode;
  int KeyRepeatCount;
  std::string KeySym;

  // MacOSX touchpad events
  double Rotation;
  double LastRotation;

  bool Equivalent(const vtkEventData *e) const override
    {
    const vtkMRMLInteractionEventData *edd = static_cast<const vtkMRMLInteractionEventData *>(e);
    if (this->Type != edd->Type)
      {
      return false;
      }
    if (edd->Modifiers >= 0 && (this->Modifiers != edd->Modifiers))
      {
      return false;
      }
    return true;
    };

  vtkMRMLInteractionEventData()
    {
    this->Type = 0;
    this->Modifiers = 0;
    this->DisplayPosition[0] = 0;
    this->DisplayPosition[1] = 0;
    this->WorldPosition[0] = 0.0;
    this->WorldPosition[1] = 0.0;
    this->WorldPosition[2] = 0.0;
    this->DisplayPositionValid = false;
    this->WorldPositionValid = false;
    this->KeyRepeatCount = 0;
    this->KeyCode = 0;
    this->ViewNode = nullptr;
    this->Rotation = 0.0;
    this->LastRotation = 0.0;
    }

  ~vtkMRMLInteractionEventData() override {}

private:
  vtkMRMLInteractionEventData(const vtkMRMLInteractionEventData& c) = delete;
  void operator=(const vtkMRMLInteractionEventData&) = delete;
};

#endif
