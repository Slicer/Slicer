/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLLinearTransformsDisplayableManager.h"
#include <vtkMRMLTransformHandleWidget.h>

// MRMLDM includes
#include <vtkMRMLInteractionEventData.h>

// MRML logic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkEventBroker.h>
#include <vtkObservation.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

//---------------------------------------------------------------------------
static const int RENDERER_LAYER = 1;

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLinearTransformsDisplayableManager);

//---------------------------------------------------------------------------
class vtkMRMLLinearTransformsDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLLinearTransformsDisplayableManager* external);
  ~vtkInternal();

  typedef std::map<vtkMRMLTransformDisplayNode*, vtkSmartPointer<vtkMRMLTransformHandleWidget>>
    InteractionWidgetsCacheType;
  InteractionWidgetsCacheType InteractionWidgets;

  // Display Nodes
  void UpdatePipelineFromDisplayNode(vtkMRMLTransformDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLTransformNode* node);
  void RemoveObservations(vtkMRMLTransformNode* node);
  void RemoveAllObservations();

  // Helper functions
  void SetupRenderer();
  bool UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  bool UseTransformNode(vtkMRMLTransformNode* displayNode);
  void ClearPipelines();

  vtkMRMLAbstractViewNode* GetAbstractViewNode();

  vtkSmartPointer<vtkRenderer> InteractionRenderer;

  vtkWeakPointer<vtkMRMLTransformHandleWidget> LastActiveWidget;
  vtkMRMLTransformHandleWidget* FindClosestWidget(vtkMRMLInteractionEventData* callData, double& closestDistance2);

private:
  vtkMRMLLinearTransformsDisplayableManager* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager::vtkInternal::vtkInternal(vtkMRMLLinearTransformsDisplayableManager* external)
  : External(external)
{
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager::vtkInternal::~vtkInternal()
{
  this->RemoveAllObservations();
  this->ClearPipelines();
}

//---------------------------------------------------------------------------
vtkMRMLAbstractViewNode* vtkMRMLLinearTransformsDisplayableManager::vtkInternal::GetAbstractViewNode()
{
  return vtkMRMLAbstractViewNode::SafeDownCast(this->External->GetMRMLDisplayableNode());
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager::vtkInternal::UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
  return displayNode && displayNode->GetScene() && displayNode->GetEditorVisibility()
         && displayNode->GetEditorSliceIntersectionVisibility()
         && displayNode->IsDisplayableInView(this->GetAbstractViewNode()->GetID());
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager::vtkInternal::UseTransformNode(vtkMRMLTransformNode* transformNode)
{
  return transformNode && transformNode->GetScene() && transformNode->IsLinear();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::vtkInternal::UpdatePipelineFromDisplayNode(
  vtkMRMLTransformDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update.
  //   otherwise, add as new node
  if (!displayNode)
  {
    return;
  }

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  bool visible = this->UseTransformNode(transformNode) && this->UseDisplayNode(displayNode);

  vtkSmartPointer<vtkMRMLTransformHandleWidget> widget;
  InteractionWidgetsCacheType::iterator pipelineIt;
  pipelineIt = this->InteractionWidgets.find(displayNode);
  if (visible && pipelineIt == this->InteractionWidgets.end())
  {
    // No pipeline, yet interaction visibility is on, create a new one
    vtkNew<vtkMRMLTransformHandleWidget> interactionWidget;
    interactionWidget->CreateDefaultRepresentation(displayNode, this->GetAbstractViewNode(), this->InteractionRenderer);
    this->InteractionWidgets[displayNode] = interactionWidget;
    widget = interactionWidget;
  }
  else if (!visible && pipelineIt != this->InteractionWidgets.end())
  {
    // Pipeline exists, but interaction visibility is off, remove it
    this->InteractionWidgets.erase(pipelineIt);
  }
  else if (pipelineIt != this->InteractionWidgets.end())
  {
    // Pipeline exists, and interaction visibility is on, update it
    widget = pipelineIt->second;
  }

  if (widget)
  {
    unsigned long event = vtkMRMLDisplayableNode::DisplayModifiedEvent;
    widget->UpdateFromMRML(displayNode->GetDisplayableNode(), event, displayNode);
    if (widget->GetNeedToRender())
    {
      this->External->RequestRender();
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::vtkInternal::AddObservations(vtkMRMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node,
                                   vtkMRMLDisplayableNode::DisplayModifiedEvent,
                                   this->External,
                                   this->External->GetMRMLNodesCallbackCommand()))
  {
    broker->AddObservation(node,
                           vtkMRMLDisplayableNode::DisplayModifiedEvent,
                           this->External,
                           this->External->GetMRMLNodesCallbackCommand());
  }
  if (!broker->GetObservationExist(node,
                                   vtkMRMLTransformableNode::TransformModifiedEvent,
                                   this->External,
                                   this->External->GetMRMLNodesCallbackCommand()))
  {
    broker->AddObservation(node,
                           vtkMRMLTransformableNode::TransformModifiedEvent,
                           this->External,
                           this->External->GetMRMLNodesCallbackCommand());
  }
  if (!broker->GetObservationExist(
        node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand()))
  {
    broker->AddObservation(
      node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::vtkInternal::RemoveObservations(vtkMRMLTransformNode* node)
{
  if (!node)
  {
    return;
  }

  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node,
                                         vtkMRMLTransformableNode::TransformModifiedEvent,
                                         this->External,
                                         this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::vtkInternal::RemoveAllObservations()
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkSmartPointer<vtkCollection> observations = vtkSmartPointer<vtkCollection>::Take(
    broker->GetObservationsForCallback(this->External->GetMRMLNodesCallbackCommand()));
  for (int i = 0; i < observations->GetNumberOfItems(); i++)
  {
    vtkObservation* observation = vtkObservation::SafeDownCast(observations->GetItemAsObject(i));
    if (observation->GetSubject() && observation->GetSubject()->IsA("vtkMRMLTransformNode"))
    {
      // Remove all vtkMRMLTransformNode observers.
      broker->RemoveObservation(observation);
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::vtkInternal::ClearPipelines()
{
  this->InteractionWidgets.clear();
}

//---------------------------------------------------------------------------
// vtkMRMLLinearTransformsDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager::vtkMRMLLinearTransformsDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager::~vtkMRMLLinearTransformsDisplayableManager()
{
  delete this->Internal;
  this->Internal = nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkMRMLLinearTransformsDisplayableManager: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || (!node->IsA("vtkMRMLTransformNode") && !node->IsA("vtkMRMLTransformDisplayNode")))
  {
    return;
  }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
  {
    this->SetUpdateFromMRMLRequested(true);
    return;
  }

  vtkMRMLTransformNode* transformNode = nullptr;
  vtkMRMLTransformDisplayNode* displayNode = nullptr;
  if (transformNode = vtkMRMLTransformNode::SafeDownCast(node))
  {
    this->Internal->AddObservations(transformNode);
  }
  else if (displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(node))
  {
    this->Internal->UpdatePipelineFromDisplayNode(vtkMRMLTransformDisplayNode::SafeDownCast(node));
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!node || (!node->IsA("vtkMRMLTransformNode") && !node->IsA("vtkMRMLTransformDisplayNode")))
  {
    return;
  }

  vtkMRMLTransformNode* transformNode = nullptr;
  vtkMRMLTransformDisplayNode* displayNode = nullptr;

  bool modified = false;
  if (transformNode = vtkMRMLTransformNode::SafeDownCast(node))
  {
    this->Internal->RemoveObservations(transformNode);
    modified = true;
  }
  else if (displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(node))
  {
    this->Internal->UpdatePipelineFromDisplayNode(displayNode);
    modified = true;
  }

  if (modified)
  {
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::ProcessMRMLNodesEvents(vtkObject* caller,
                                                                       unsigned long event,
                                                                       void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene == nullptr || scene->IsBatchProcessing())
  {
    return;
  }

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(caller);
  if (transformNode)
  {
    vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode*>(callData);
    vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(callDataNode);

    if (displayNode && (event == vtkMRMLDisplayableNode::DisplayModifiedEvent))
    {
      this->Internal->UpdatePipelineFromDisplayNode(displayNode);
      this->RequestRender();
    }
    else if (event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
      for (int i = 0; i < transformNode->GetNumberOfDisplayNodes(); i++)
      {
        vtkMRMLTransformDisplayNode* displayNode =
          vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetNthDisplayNode(i));
        if (displayNode)
        {
          this->Internal->UpdatePipelineFromDisplayNode(displayNode);
        }
      }
      this->RequestRender();
    }

    for (std::pair<vtkMRMLTransformDisplayNode*, vtkMRMLTransformHandleWidget*> interactionWidget :
         this->Internal->InteractionWidgets)
    {
      interactionWidget.second->UpdateFromMRML(transformNode, event, callData);
      if (interactionWidget.second->GetNeedToRender())
      {
        this->RequestRender();
      }
    }
  }
  else if (vtkMRMLAbstractViewNode::SafeDownCast(caller))
  {
    this->RequestRender();

    for (auto interactionWidget : this->Internal->InteractionWidgets)
    {
      interactionWidget.second->UpdateFromMRML(this->Internal->GetAbstractViewNode(), event, callData);
    }
  }
  else
  {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  this->GetMRMLApplicationLogic()->PauseRender();

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkDebugMacro("vtkMRMLLinearTransformsDisplayableManager->UpdateFromMRML: Scene is not set.");
    return;
  }

  this->Internal->RemoveAllObservations();
  this->Internal->ClearPipelines();

  std::vector<vtkMRMLNode*> displayNodes;
  scene->GetNodesByClass("vtkMRMLTransformDisplayNode", displayNodes);
  for (auto displayNode : displayNodes)
  {
    this->Internal->UpdatePipelineFromDisplayNode(vtkMRMLTransformDisplayNode::SafeDownCast(displayNode));
  }

  std::vector<vtkMRMLNode*> transformNodes;
  scene->GetNodesByClass("vtkMRMLTransformNode", transformNodes);
  for (auto node : transformNodes)
  {
    this->Internal->AddObservations(vtkMRMLTransformNode::SafeDownCast(node));
  }

  this->RequestRender();
  this->GetMRMLApplicationLogic()->ResumeRender();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->RemoveAllObservations();
  this->Internal->ClearPipelines();
  this->Superclass::UnobserveMRMLScene();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::OnMRMLSceneStartClose()
{
  this->Internal->RemoveAllObservations();
  this->Internal->ClearPipelines();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::Create()
{
  Superclass::Create();
  this->Internal->SetupRenderer();
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
vtkMRMLTransformHandleWidget* vtkMRMLLinearTransformsDisplayableManager::vtkInternal::FindClosestWidget(
  vtkMRMLInteractionEventData* callData,
  double& closestDistance2)
{
  vtkMRMLTransformHandleWidget* closestWidget = nullptr;
  closestDistance2 = VTK_DOUBLE_MAX;

  for (auto widgetIterator : this->InteractionWidgets)
  {
    vtkMRMLTransformHandleWidget* widget = widgetIterator.second;
    if (!widget)
    {
      continue;
    }
    double distance2FromWidget = VTK_DOUBLE_MAX;
    if (widget->CanProcessInteractionEvent(callData, distance2FromWidget))
    {
      if (!closestWidget || distance2FromWidget < closestDistance2)
      {
        closestDistance2 = distance2FromWidget;
        closestWidget = widget;
      }
    }
  }
  return closestWidget;
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager::vtkInternal::SetupRenderer()
{
  vtkRenderer* renderer = this->External->GetRenderer();
  if (renderer == nullptr)
  {
    vtkErrorWithObjectMacro(
      this->External,
      "vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::SetupRenderer() failed: renderer is invalid");
    return;
  }

  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();
  if (renderWindow->GetNumberOfLayers() < RENDERER_LAYER + 1)
  {
    renderWindow->SetNumberOfLayers(RENDERER_LAYER + 1);
  }

  this->InteractionRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->InteractionRenderer->UseDepthPeelingOn();
  this->InteractionRenderer->InteractiveOff();
  this->InteractionRenderer->SetActiveCamera(renderer->GetActiveCamera());
  this->InteractionRenderer->SetLayer(RENDERER_LAYER);
  renderWindow->AddRenderer(this->InteractionRenderer);
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData,
                                                                           double& closestDistance2)
{
  // New point can be placed anywhere
  int eventid = eventData->GetType();

  if (eventid == vtkCommand::LeaveEvent && this->Internal->LastActiveWidget != nullptr)
  {
    if (this->Internal->LastActiveWidget->GetDisplayNode()
        && this->Internal->LastActiveWidget->GetDisplayNode()->GetActiveInteractionType()
             > vtkMRMLTransformHandleWidget::InteractionNone)
    {
      // this widget has active component, therefore leave event is relevant
      closestDistance2 = 0.0;
      return this->Internal->LastActiveWidget;
    }
  }

  // Other interactions
  bool canProcess = (this->Internal->FindClosestWidget(eventData, closestDistance2) != nullptr);

  if (!canProcess && this->Internal->LastActiveWidget != nullptr
      && (eventid == vtkCommand::MouseMoveEvent || eventid == vtkCommand::Move3DEvent))
  {
    // TODO: handle multiple contexts
    this->Internal->LastActiveWidget->Leave(eventData);
    this->Internal->LastActiveWidget = nullptr;
  }

  return canProcess;
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  int eventid = eventData->GetType();

  if (eventid == vtkCommand::LeaveEvent)
  {
    if (this->Internal->LastActiveWidget != nullptr)
    {
      this->Internal->LastActiveWidget->Leave(eventData);
      this->Internal->LastActiveWidget = nullptr;
    }
  }

  // Find/create active widget
  vtkMRMLTransformHandleWidget* activeWidget = nullptr;
  double closestDistance2 = VTK_DOUBLE_MAX;
  activeWidget = this->Internal->FindClosestWidget(eventData, closestDistance2);

  // Deactivate previous widget
  if (this->Internal->LastActiveWidget != nullptr && this->Internal->LastActiveWidget != activeWidget)
  {
    this->Internal->LastActiveWidget->Leave(eventData);
  }
  this->Internal->LastActiveWidget = activeWidget;
  if (!activeWidget)
  {
    // deactivate widget if we move far from it
    if (eventid == vtkCommand::MouseMoveEvent && this->Internal->LastActiveWidget != nullptr)
    {
      this->Internal->LastActiveWidget->Leave(eventData);
      this->Internal->LastActiveWidget = nullptr;
    }
    return false;
  }

  // Pass on the interaction event to the active widget
  return activeWidget->ProcessInteractionEvent(eventData);
}

//---------------------------------------------------------------------------
vtkMRMLTransformHandleWidget* vtkMRMLLinearTransformsDisplayableManager::GetWidget(
  vtkMRMLTransformDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return nullptr;
  }

  auto interactionWidgetIterator = this->Internal->InteractionWidgets.find(displayNode);
  if (interactionWidgetIterator != this->Internal->InteractionWidgets.end())
  {
    return interactionWidgetIterator->second;
  }
  return nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLTransformHandleWidget* vtkMRMLLinearTransformsDisplayableManager::GetWidget(vtkMRMLTransformNode* transformNode)
{
  if (!transformNode)
  {
    return nullptr;
  }

  vtkMRMLTransformDisplayNode* markupsDisplayNode =
    vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  if (!markupsDisplayNode)
  {
    return nullptr;
  }

  auto interactionWidgetIterator = this->Internal->InteractionWidgets.find(markupsDisplayNode);
  if (interactionWidgetIterator != this->Internal->InteractionWidgets.end())
  {
    return interactionWidgetIterator->second;
  }
  return nullptr;
}
