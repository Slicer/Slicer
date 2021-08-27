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
#include "vtkMRMLColorBarDisplayableManager.h"

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
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLColorBarDisplayNode.h>

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
#include <vtkScalarBarRepresentation.h>
#include <vtkScalarBarWidget.h>
#include <vtkScalarBarActor.h>

// STD includes
#include <algorithm>
#include <cstring>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLColorBarDisplayableManager);

//---------------------------------------------------------------------------
class vtkMRMLColorBarDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLColorBarDisplayableManager * external);
  virtual ~vtkInternal();

  vtkObserverManager* GetMRMLNodesObserverManager();
  void Modified();

  // Build the color bars
  void BuildColorBar();
  // Setup/update actor and widget representation
  void SetupActor();

  vtkScalarBarActor* GetActor() const { return this->ColorBarActor; }
  vtkScalarBarWidget* GetWidget() const { return this->ColorBarWidget; }

  vtkMRMLColorBarDisplayableManager*        External;

  vtkSmartPointer<vtkScalarBarActor> ColorBarActor;
  vtkSmartPointer<vtkScalarBarWidget> ColorBarWidget;
  vtkWeakPointer<vtkMRMLColorBarDisplayNode> ColorBarDisplayNode;
};


//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLColorBarDisplayableManager::vtkInternal::vtkInternal(vtkMRMLColorBarDisplayableManager* external)
  :
  External(external),
  ColorBarActor(vtkSmartPointer<vtkScalarBarActor>::New()),
  ColorBarWidget(vtkSmartPointer<vtkScalarBarWidget>::New())
{
  this->ColorBarWidget->SetScalarBarActor(this->ColorBarActor);

  vtkScalarBarRepresentation* repr = this->ColorBarWidget->GetScalarBarRepresentation();
  if (repr)
  {
    repr->SetShowHorizontalBorder(true);
    repr->SetShowVerticalBorder(true);
    // show the border when hovering over with the mouse
    repr->SetShowBorderToActive();
  }
}

//---------------------------------------------------------------------------
vtkMRMLColorBarDisplayableManager::vtkInternal::~vtkInternal()
{
  this->ColorBarWidget->SetScalarBarActor(nullptr);
  this->ColorBarWidget->SetInteractor(nullptr);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkMRMLColorBarDisplayableManager::vtkInternal::GetMRMLNodesObserverManager()
{
  return this->External->GetMRMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::vtkInternal::BuildColorBar()
{
  vtkMRMLViewNode* threeDViewNode = nullptr;
  vtkMRMLSliceNode* sliceNode = nullptr;
  vtkMRMLNode* node = this->External->GetMRMLDisplayableNode();
  if (node)
  {
    threeDViewNode = vtkMRMLViewNode::SafeDownCast(node);
    sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  }

  vtkRenderWindowInteractor* interactor = this->External->GetInteractor();
  if (!interactor)
  {
    // interactor is invalid
    return;
  }
  else if (!this->ColorBarWidget->GetInteractor())
  {
    this->ColorBarWidget->SetInteractor(interactor);
  }

  // Setup/update scalar bar actor
  if (this->ColorBarDisplayNode)
  {
    if (sliceNode)
    {
      this->ColorBarWidget->SetEnabled(this->ColorBarDisplayNode->GetVisibility2D());
    }
    else if (threeDViewNode)
    {
      this->ColorBarWidget->SetEnabled(this->ColorBarDisplayNode->GetVisibility3D());
    }
  }
  else
  {
    // Color bar display node is invalid
    return;
  }
}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::vtkInternal::SetupActor()
{
  if (!this->ColorBarDisplayNode)
  {
    return;
  }

  switch (this->ColorBarDisplayNode->GetPositionPreset())
  {
  case vtkMRMLColorBarDisplayNode::Vertical:
    this->ColorBarActor->SetOrientationToVertical();

//    actor->SetNumberOfLabels(11);
//    actor->SetTitle("(mm)");

    // it's a 2d actor, position it in screen space by percentages
//    actor->SetPosition(0.1, 0.1);
//    actor->SetWidth(0.1);
//    actor->SetHeight(0.8);

    // By default, color swatch is too wide (especially when showing long color names),
    // therefore, set it to a bit narrower.
//    actor->SetBarRatio(0.15);

//    actor->SetPosition(0.1, 0.1);
//    actor->SetWidth(0.1);
//    actor->SetHeight(0.8);
//    actor->SetPosition(0.1, 0.1);
//    actor->SetWidth(0.1);
//    actor->SetHeight(0.8);
    break;
  case vtkMRMLColorBarDisplayNode::Horizontal:
    this->ColorBarActor->SetOrientationToHorizontal();

//    actor->SetNumberOfLabels(11);
//    actor->SetTitle("(mm)");

    // it's a 2d actor, position it in screen space by percentages
//    actor->SetPosition(0.1, 0.1);
//    actor->SetWidth(0.8);
//    actor->SetHeight(0.1);

    // By default, color swatch is too wide (especially when showing long color names),
    // therefore, set it to a bit narrower.
//    actor->SetBarRatio(0.15);

//    actor->SetPosition(0.1, 0.1);
//    actor->SetWidth(0.8);
//    actor->SetHeight(0.1);
//    actor->SetPosition(0.1, 0.1);
//    actor->SetWidth(0.8);
//    actor->SetHeight(0.1);
    break;
  default:
    break;
  }
}

//---------------------------------------------------------------------------
// vtkMRMLColorBarDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLColorBarDisplayableManager::vtkMRMLColorBarDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLColorBarDisplayableManager::~vtkMRMLColorBarDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkScalarBarWidget* vtkMRMLColorBarDisplayableManager::GetScalarBarWidget() const
{
  return this->Internal->GetWidget();
}

//---------------------------------------------------------------------------
vtkScalarBarActor* vtkMRMLColorBarDisplayableManager::GetScalarBarActor() const
{
  return this->Internal->GetActor();
}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::Create()
{
}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::AdditionalInitializeStep()
{
  // Build the initial scalar bar
  this->Internal->BuildColorBar();
}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* vtkNotUsed(caller))
{
  vtkMRMLColorTableNode* colorTableNode = nullptr;
  
  if (this->Internal->ColorBarDisplayNode)
  {
    vtkMRMLDisplayableNode* displayableNode = this->Internal->ColorBarDisplayNode->GetDisplayableNode();
    colorTableNode = this->Internal->ColorBarDisplayNode->GetColorTableNode();

    if (!colorTableNode && displayableNode)
    {
      vtkMRMLDisplayableNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(displayableNode);
      if (volumeNode)
      {
        vtkMRMLDisplayNode* volumeDisplayNode = volumeNode->GetDisplayNode();
        if (vtkMRMLColorNode* colorNode = volumeDisplayNode->GetColorNode())
        {
          colorTableNode = vtkMRMLColorTableNode::SafeDownCast(colorNode);
        }
      }
    }
  }
  if (colorTableNode)
  {
    this->Internal->BuildColorBar();
    this->Internal->ColorBarActor->SetLookupTable(colorTableNode->GetScalarsToColors());
  }

}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

  if (node->IsA("vtkMRMLColorBarDisplayNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLColorBarDisplayableManager::ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData)
{
  this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
  switch (event)
  {
  case vtkCommand::ModifiedEvent:
    {
      this->Internal->ColorBarDisplayNode = vtkMRMLColorBarDisplayNode::SafeDownCast(caller);
      // Update scalar bars using new prorepties of the bars
      // Update color bar orientation
      this->Internal->SetupActor();
      this->RequestRender();
    }
    break;
  default:
    break;
  }
}
