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

// MRMLDisplayableManager includes
#include "vtkMRMLScalarBarDisplayableManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLightBoxRendererManagerProxy.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLWindowLevelWidget.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPickingManager.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProp.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLScalarBarDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLScalarBarDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLScalarBarDisplayableManager * external);
  ~vtkInternal();

  vtkObserverManager* GetMRMLNodesObserverManager();
  void Modified();

  // Slice
  vtkMRMLSliceNode* GetSliceNode();
  void UpdateSliceNode();

  // Build the crosshair representation
  void BuildScalarBar();

  vtkMRMLScalarBarDisplayableManager*        External;

  vtkSmartPointer<vtkMRMLWindowLevelWidget> WindowLevelWidget;
};


//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLScalarBarDisplayableManager::vtkInternal
::vtkInternal(vtkMRMLScalarBarDisplayableManager * external)
{
  this->External = external;
  this->WindowLevelWidget = vtkSmartPointer<vtkMRMLWindowLevelWidget>::New();
}

//---------------------------------------------------------------------------
vtkMRMLScalarBarDisplayableManager::vtkInternal::~vtkInternal()
{
  this->WindowLevelWidget->SetMRMLApplicationLogic(nullptr);
  this->WindowLevelWidget->SetRenderer(nullptr);
  this->WindowLevelWidget->SetSliceNode(nullptr);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkMRMLScalarBarDisplayableManager::vtkInternal::GetMRMLNodesObserverManager()
{
  return this->External->GetMRMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLScalarBarDisplayableManager::vtkInternal
::GetSliceNode()
{
  return this->External->GetMRMLSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::vtkInternal::UpdateSliceNode()
{
  if (this->External->GetMRMLScene() == nullptr)
    {
    this->WindowLevelWidget->SetSliceNode(nullptr);
    return;
    }

  if (!this->WindowLevelWidget->GetRenderer())
    {
    vtkMRMLApplicationLogic *mrmlAppLogic = this->External->GetMRMLApplicationLogic();
    this->WindowLevelWidget->SetMRMLApplicationLogic(mrmlAppLogic);
    this->WindowLevelWidget->CreateDefaultRepresentation();
    this->WindowLevelWidget->SetRenderer(this->External->GetRenderer());
    }
  this->WindowLevelWidget->SetSliceNode(this->GetSliceNode());
}

//---------------------------------------------------------------------------
// vtkMRMLScalarBarDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLScalarBarDisplayableManager::vtkMRMLScalarBarDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLScalarBarDisplayableManager::~vtkMRMLScalarBarDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::UpdateFromMRMLScene()
{
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->WindowLevelWidget->SetSliceNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::Create()
{
  // Setup the SliceNode, ScalarBarNode
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::AdditionalInitializeStep()
{
  // Build the initial crosshair representation
  //this->Internal->BuildScalarBar();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::OnMRMLSliceNodeModifiedEvent()
{
}

//---------------------------------------------------------------------------
bool vtkMRMLScalarBarDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2)
{
  int eventid = eventData->GetType();
  if (eventid == vtkCommand::LeaveEvent)
    {
    this->Internal->WindowLevelWidget->Leave(eventData);
    }

  // Find/create active widget
  if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::AdjustWindowLevel)
    {
    return this->Internal->WindowLevelWidget->CanProcessInteractionEvent(eventData, closestDistance2);
    }

  return false;
}

//---------------------------------------------------------------------------
bool vtkMRMLScalarBarDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  bool processed = this->Internal->WindowLevelWidget->ProcessInteractionEvent(eventData);
  if (this->Internal->WindowLevelWidget->GetNeedToRender())
    {
    this->RequestRender();
    this->Internal->WindowLevelWidget->NeedToRenderOff();
    }
  return processed;
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::SetAdjustForegroundWindowLevelEnabled(bool enabled)
{
  this->Internal->WindowLevelWidget->SetForegroundVolumeEditable(enabled);
}

//---------------------------------------------------------------------------
bool vtkMRMLScalarBarDisplayableManager::GetAdjustForegroundWindowLevelEnabled()
{
  return this->Internal->WindowLevelWidget->GetForegroundVolumeEditable();
}

//---------------------------------------------------------------------------
void vtkMRMLScalarBarDisplayableManager::SetAdjustBackgroundWindowLevelEnabled(bool enabled)
{
  this->Internal->WindowLevelWidget->SetBackgroundVolumeEditable(enabled);
}

//---------------------------------------------------------------------------
bool vtkMRMLScalarBarDisplayableManager::GetAdjustBackgroundWindowLevelEnabled()
{
  return this->Internal->WindowLevelWidget->GetBackgroundVolumeEditable();
}
