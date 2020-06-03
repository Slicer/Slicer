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

#include "vtkMRMLInteractionEventData.h"

// VTK includes
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkEvent.h"
#include "vtkPoints.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"

//---------------------------------------------------------------------------
vtkMRMLInteractionEventData::vtkMRMLInteractionEventData()
{
  this->Type = 0;
  this->Modifiers = 0;
  this->DisplayPosition[0] = 0;
  this->DisplayPosition[1] = 0;
  this->WorldPosition[0] = 0.0;
  this->WorldPosition[1] = 0.0;
  this->WorldPosition[2] = 0.0;
  this->WorldOrientation[0] = 0.0;
  this->WorldOrientation[1] = 0.0;
  this->WorldOrientation[2] = 0.0;
  this->WorldOrientation[3] = 1.0;
  this->WorldDirection[0] = 0.0;
  this->WorldDirection[1] = 0.0;
  this->WorldDirection[2] = 1.0;
  this->TrackPadPosition[0] = 0.0;
  this->TrackPadPosition[1] = 0.0;
  this->DisplayPositionValid = false;
  this->WorldPositionValid = false;
  this->WorldPositionAccurate = false;
  this->ComputeAccurateWorldPositionAttempted = false;
  this->AccuratePicker = nullptr;
  this->Renderer = nullptr;
  this->KeyRepeatCount = 0;
  this->KeyCode = 0;
  this->ViewNode = nullptr;
  this->Rotation = 0.0;
  this->LastRotation = 0.0;
  this->Scale = 1.0;
  this->LastScale = 1.0;
  this->Translation[0] = this->Translation[1] = 0.0;
  this->LastTranslation[0] = this->LastTranslation[1] = 0.0;
  this->WorldToPhysicalScale = 1.0;
  this->InteractionContextName = "";
  this->ComponentType = -1;
  this->ComponentIndex = -1;
}

//---------------------------------------------------------------------------
vtkMRMLInteractionEventData* vtkMRMLInteractionEventData::New()
{
  vtkMRMLInteractionEventData *ret = new vtkMRMLInteractionEventData;
  ret->InitializeObjectBase();
  return ret;
};

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetType(unsigned long v)
{
  this->Type = v;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetModifiers(int v)
{
  this->Modifiers = v;
}

//---------------------------------------------------------------------------
int vtkMRMLInteractionEventData::GetModifiers()
{
  return this->Modifiers;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetWorldPosition(const double p[3], bool accurate/*=true*/)
{
  this->WorldPosition[0] = p[0];
  this->WorldPosition[1] = p[1];
  this->WorldPosition[2] = p[2];
  this->WorldPositionValid = true;
  this->WorldPositionAccurate = accurate;
}

//---------------------------------------------------------------------------
bool vtkMRMLInteractionEventData::IsWorldPositionValid()
{
  return this->WorldPositionValid;
}

//---------------------------------------------------------------------------
bool vtkMRMLInteractionEventData::IsWorldPositionAccurate()
{
  return this->WorldPositionValid && this->WorldPositionAccurate;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetWorldPositionInvalid()
{
  this->WorldPositionValid = false;
  this->WorldPositionAccurate = false;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::GetDisplayPosition(int v[2]) const
{
  std::copy(this->DisplayPosition, this->DisplayPosition + 2, v);
}

//---------------------------------------------------------------------------
const int* vtkMRMLInteractionEventData::GetDisplayPosition() const
{
  return this->DisplayPosition;
}
//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetDisplayPosition(const int p[2])
{
  this->DisplayPosition[0] = p[0];
  this->DisplayPosition[1] = p[1];
  this->DisplayPositionValid = true;
  this->ComputeAccurateWorldPositionAttempted = false;
}

//---------------------------------------------------------------------------
bool vtkMRMLInteractionEventData::IsDisplayPositionValid()
{
  return this->DisplayPositionValid;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetDisplayPositionInvalid()
{
  this->DisplayPositionValid = false;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetKeyCode(char v)
{
  this->KeyCode = v;
}

//---------------------------------------------------------------------------
char vtkMRMLInteractionEventData::GetKeyCode()
{
  return this->KeyCode;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetKeyRepeatCount(char v)
{
  this->KeyRepeatCount = v;
}

//---------------------------------------------------------------------------
int vtkMRMLInteractionEventData::GetKeyRepeatCount()
{
  return this->KeyRepeatCount;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetKeySym(const std::string &v)
{
  this->KeySym = v;
}

//---------------------------------------------------------------------------
const std::string& vtkMRMLInteractionEventData::GetKeySym()
{
  return this->KeySym;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  this->ViewNode = viewNode;
}

//---------------------------------------------------------------------------
vtkMRMLAbstractViewNode* vtkMRMLInteractionEventData::GetViewNode() const
{
  return this->ViewNode;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetComponentType(int componentType) { this->ComponentType = componentType; }

//---------------------------------------------------------------------------
int vtkMRMLInteractionEventData::GetComponentType() const
{
  return this->ComponentType;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetComponentIndex(int componentIndex)
{
  this->ComponentIndex = componentIndex;
}

//---------------------------------------------------------------------------
int vtkMRMLInteractionEventData::GetComponentIndex() const
{
  return this->ComponentIndex;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetRotation(double v)
{
  this->Rotation = v;
}

//---------------------------------------------------------------------------
double vtkMRMLInteractionEventData::GetRotation() const
{
  return this->Rotation;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetLastRotation(double v)
{
  this->LastRotation = v;
}

//---------------------------------------------------------------------------
double vtkMRMLInteractionEventData::GetLastRotation() const
{
  return this->LastRotation;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetWorldToPhysicalScale(double v)
{
  this->WorldToPhysicalScale = v;
}

//---------------------------------------------------------------------------
double vtkMRMLInteractionEventData::GetWorldToPhysicalScale() const
{
  return this->WorldToPhysicalScale;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetAttributesFromInteractor(vtkRenderWindowInteractor* interactor)
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
  this->Scale = interactor->GetScale();
  this->LastScale = interactor->GetLastScale();
  this->SetTranslation(interactor->GetTranslation());
  this->SetLastTranslation(interactor->GetLastTranslation());
}

//---------------------------------------------------------------------------
vtkRenderer* vtkMRMLInteractionEventData::GetRenderer() const
{
  return this->Renderer;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetRenderer(vtkRenderer* ren)
{
  this->Renderer = ren;
}

//---------------------------------------------------------------------------
vtkCellPicker* vtkMRMLInteractionEventData::GetAccuratePicker() const
{
  return this->AccuratePicker;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetAccuratePicker(vtkCellPicker* picker)
{
  this->AccuratePicker = picker;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetInteractionContextName(const std::string& contextName)
{
  this->InteractionContextName = contextName;
}

//---------------------------------------------------------------------------
const std::string& vtkMRMLInteractionEventData::GetInteractionContextName()
{
  return this->InteractionContextName;
}

//---------------------------------------------------------------------------
bool vtkMRMLInteractionEventData::ComputeAccurateWorldPosition(bool force/*=false*/)
{
  if (this->IsWorldPositionAccurate())
    {
    return true;
    }
  if (this->ComputeAccurateWorldPositionAttempted && !force)
    {
    // by default do not attempt to compute accurate position again
    // if it failed once
    return false;
    }
  if (!this->AccuratePicker || !this->Renderer || !this->DisplayPositionValid)
    {
    return false;
    }
  this->ComputeAccurateWorldPositionAttempted = true;
  if (!this->AccuratePicker->Pick(static_cast<double>(this->DisplayPosition[0]), static_cast<double>(this->DisplayPosition[1]), 0, this->Renderer))
    {
    return false;
    }
  vtkPoints* pickPositions = this->AccuratePicker->GetPickedPositions();
  int numberOfPickedPositions = pickPositions->GetNumberOfPoints();
  if (numberOfPickedPositions < 1)
    {
    return false;
    }
  // There may be multiple picked positions, choose the one closest to the camera
  double cameraPosition[3] = { 0,0,0 };
  this->Renderer->GetActiveCamera()->GetPosition(cameraPosition);
  pickPositions->GetPoint(0, this->WorldPosition);
  double minDist2 = vtkMath::Distance2BetweenPoints(this->WorldPosition, cameraPosition);
  for (int i = 1; i < numberOfPickedPositions; i++)
    {
    double currentMinDist2 = vtkMath::Distance2BetweenPoints(pickPositions->GetPoint(i), cameraPosition);
    if (currentMinDist2 < minDist2)
      {
      pickPositions->GetPoint(i, this->WorldPosition);
      minDist2 = currentMinDist2;
      }
    }
  this->WorldPositionValid = true;
  this->WorldPositionAccurate = true;
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLInteractionEventData::Equivalent(const vtkEventData *e) const
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

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetScale(double scale)
{
  this->Scale = scale;
}

//---------------------------------------------------------------------------
double vtkMRMLInteractionEventData::GetScale() const
{
  return this->Scale;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetLastScale(double lastScale)
{
  this->LastScale = lastScale;
}

//---------------------------------------------------------------------------
double vtkMRMLInteractionEventData::GetLastScale() const
{
  return this->LastScale;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetTranslation(const double translation[2])
{
  this->Translation[0] = translation[0];
  this->Translation[1] = translation[1];
}

//---------------------------------------------------------------------------
const double* vtkMRMLInteractionEventData::GetTranslation() const
{
  return this->Translation;
}

//---------------------------------------------------------------------------
void vtkMRMLInteractionEventData::SetLastTranslation(const double lastTranslation[2])
{
  this->LastTranslation[0] = lastTranslation[0];
  this->LastTranslation[1] = lastTranslation[1];
}

//---------------------------------------------------------------------------
const double* vtkMRMLInteractionEventData::GetLastTranslation() const
{
  return this->LastTranslation;
}
