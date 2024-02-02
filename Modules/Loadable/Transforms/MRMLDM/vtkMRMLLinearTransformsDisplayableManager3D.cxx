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
#include "vtkMRMLLinearTransformsDisplayableManager3D.h"

#include "vtkSlicerTransformLogic.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkDataSetAttributes.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

// MRMLDM includes
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLTransformHandleWidget.h>

static const int RENDERER_LAYER = 1;

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkMRMLLinearTransformsDisplayableManager3D );

//---------------------------------------------------------------------------
class vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
{
public:

  vtkInternal(vtkMRMLLinearTransformsDisplayableManager3D* external);
  ~vtkInternal();

  typedef std::map < vtkMRMLTransformDisplayNode*, vtkSmartPointer<vtkMRMLTransformHandleWidget> > InteractionPipelinesCacheType;
  InteractionPipelinesCacheType InteractionPipelines;

  typedef std::map < vtkMRMLTransformNode*, std::set< vtkMRMLTransformDisplayNode* > > TransformToDisplayCacheType;
  TransformToDisplayCacheType TransformToDisplayNodes;

  // Transforms
  void AddTransformNode(vtkMRMLTransformNode* displayableNode);
  void RemoveTransformNode(vtkMRMLTransformNode* displayableNode);
  void UpdateDisplayableTransforms(vtkMRMLTransformNode *node);

  // Display Nodes
  void AddDisplayNode(vtkMRMLTransformNode*, vtkMRMLTransformDisplayNode*);
  void UpdateDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  void RemoveDisplayNode(vtkMRMLTransformDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLTransformNode* node);
  void RemoveObservations(vtkMRMLTransformNode* node);
  bool IsNodeObserved(vtkMRMLTransformNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLTransformDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLTransformNode* node);
  void ClearDisplayableNodes();

  void UpdateInteractionPipeline(vtkMRMLTransformNode* transformNode, unsigned long event);
  void UpdateInteractionPipeline(vtkMRMLTransformNode* transformNode, unsigned long event, vtkMRMLTransformDisplayNode* displayNode);

  void SetupRenderer();

  vtkSmartPointer<vtkRenderer> InteractionRenderer;

  vtkWeakPointer<vtkMRMLTransformHandleWidget> LastActiveWidget;
  vtkMRMLTransformHandleWidget* FindClosestWidget(vtkMRMLInteractionEventData* callData, double& closestDistance2);

private:
  vtkMRMLLinearTransformsDisplayableManager3D* External;
  bool AddingTransformNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::vtkInternal(vtkMRMLLinearTransformsDisplayableManager3D * external)
: External(external)
, AddingTransformNode(false)
{
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
   // allow nodes to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->External->GetMRMLViewNode()->GetID()))
    {
    return false;
    }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkMRMLTransformDisplayNode");

  return use;
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::IsVisible(vtkMRMLTransformDisplayNode* displayNode)
{
  return displayNode && (displayNode->GetVisibility() != 0) && (displayNode->GetVisibility3D() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::AddTransformNode(vtkMRMLTransformNode* node)
{
  if (this->AddingTransformNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingTransformNode = true;
  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();

  this->AddObservations(node);

  for (int i=0; i<nnodes; i++)
    {
    vtkMRMLTransformDisplayNode *dnode = vtkMRMLTransformDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));
    if ( this->UseDisplayNode(dnode) )
      {
      this->TransformToDisplayNodes[node].insert(dnode);
      this->AddDisplayNode( node, dnode );
      }
    }
  this->AddingTransformNode = false;
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::RemoveTransformNode(vtkMRMLTransformNode* node)
{
  if (!node)
    {
    return;
    }
  vtkInternal::TransformToDisplayCacheType::iterator displayableIt =
    this->TransformToDisplayNodes.find(node);
  if(displayableIt == this->TransformToDisplayNodes.end())
    {
    return;
    }

  std::set<vtkMRMLTransformDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkMRMLTransformDisplayNode *>::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->RemoveDisplayNode(*diter);
    }
  this->RemoveObservations(node);
  this->TransformToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::UpdateDisplayableTransforms(vtkMRMLTransformNode* mNode)
{
  // Update the pipeline for all tracked DisplayableNode

  InteractionPipelinesCacheType::iterator pipelinesIter;
  std::set< vtkMRMLTransformDisplayNode* > displayNodes = this->TransformToDisplayNodes[mNode];
  std::set< vtkMRMLTransformDisplayNode* >::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    vtkMRMLTransformDisplayNode* displayNode = *dnodesIter;
    this->UpdateDisplayNode(displayNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::RemoveDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return;
    }
  this->InteractionPipelines.erase(displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::AddDisplayNode(vtkMRMLTransformNode* mNode, vtkMRMLTransformDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::UpdateDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update.
  //   otherwise, add as new node

  if (!displayNode)
    {
    return;
    }

  InteractionPipelinesCacheType::iterator it;
  it = this->InteractionPipelines.find(displayNode);
  vtkMRMLTransformNode* displayableNode = vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  this->UpdateInteractionPipeline(displayableNode, vtkCommand::ModifiedEvent, displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::AddObservations(vtkMRMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }

}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::RemoveObservations(vtkMRMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkMRMLTransformableNode::TransformModifiedEvent,
    this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::IsNodeObserved(vtkMRMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkCollection* observations = broker->GetObservationsForSubject(node);
  if (observations->GetNumberOfItems() > 0)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::ClearDisplayableNodes()
{
  while(this->TransformToDisplayNodes.size() > 0)
    {
    this->RemoveTransformNode(this->TransformToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::UseDisplayableNode(vtkMRMLTransformNode* node)
{
  bool use = node && node->IsA("vtkMRMLTransformNode");
  return use;
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::UpdateInteractionPipeline(vtkMRMLTransformNode* displayableNode, unsigned long event)
{
  if (!displayableNode)
    {
    return;
    }

  auto displayNodesIt = this->TransformToDisplayNodes.find(displayableNode);
  if (displayNodesIt != this->TransformToDisplayNodes.end())
    {
    for (vtkMRMLTransformDisplayNode* displayNode : displayNodesIt->second)
      {
      this->UpdateInteractionPipeline(displayableNode, event, displayNode);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::UpdateInteractionPipeline(
  vtkMRMLTransformNode* displayableNode, unsigned long event, vtkMRMLTransformDisplayNode* displayNode)
{
  if (!displayableNode || !displayNode)
    {
    return;
    }

  vtkSmartPointer<vtkMRMLTransformHandleWidget> widget;
  InteractionPipelinesCacheType::iterator pipelineIt = this->InteractionPipelines.find(displayNode);

  bool visible = displayNode->GetEditorVisibility();
  if (visible && pipelineIt == this->InteractionPipelines.end())
    {
    // No pipeline, yet interaction visibility is on, create a new one

    vtkNew<vtkMRMLTransformHandleWidget> interactionWidget;
    interactionWidget->CreateDefaultRepresentation(displayNode, this->External->GetMRMLViewNode(), this->InteractionRenderer);
    this->InteractionPipelines[displayNode] = interactionWidget;
    widget = interactionWidget;
    }
  else if (!visible && pipelineIt != this->InteractionPipelines.end())
    {
    // Pipeline exists, but interaction visibility is off, remove it
    this->InteractionPipelines.erase(pipelineIt);
    }
  else if (pipelineIt != this->InteractionPipelines.end())
    {
    widget = pipelineIt->second;
    }

  if (widget)
    {
    widget->UpdateFromMRML(displayableNode, event, displayNode);
    if (widget->GetNeedToRender())
      {
      this->External->RequestRender();
      }
    }
}

//---------------------------------------------------------------------------
// vtkMRMLLinearTransformsDisplayableManager3D methods

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager3D::vtkMRMLLinearTransformsDisplayableManager3D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager3D::~vtkMRMLLinearTransformsDisplayableManager3D()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkMRMLLinearTransformsDisplayableManager3D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( !node->IsA("vtkMRMLTransformNode") )
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromMRMLRequested(true);
    return;
    }

  this->Internal->AddTransformNode(vtkMRMLTransformNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node
    && (!node->IsA("vtkMRMLTransformNode"))
    && (!node->IsA("vtkMRMLTransformDisplayNode")) )
    {
    return;
    }

  vtkMRMLTransformNode* transformNode = nullptr;
  vtkMRMLTransformDisplayNode* displayNode = nullptr;

  bool modified = false;
  if ( (transformNode = vtkMRMLTransformNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveTransformNode(transformNode);
    modified = true;
    }
  else if ( (displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveDisplayNode(displayNode);
    modified = true;
    }
  if (modified)
    {
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if (scene == nullptr || scene->IsBatchProcessing())
    {
    return;
    }

  vtkMRMLTransformNode* displayableNode = vtkMRMLTransformNode::SafeDownCast(caller);
  vtkMRMLTransformDisplayNode* displayNode = nullptr;
  if ( displayableNode )
    {
    vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode *> (callData);
    displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(callDataNode);

    if ( displayNode && (event == vtkMRMLDisplayableNode::DisplayModifiedEvent) )
      {
      this->Internal->UpdateDisplayNode(displayNode);
      this->RequestRender();
      }
    else if (event == vtkMRMLTransformableNode::TransformModifiedEvent)
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
      }
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }

  if (displayNode)
    {
    this->Internal->UpdateInteractionPipeline(displayableNode, event, displayNode);
    }
  else if (displayableNode)
    {
    this->Internal->UpdateInteractionPipeline(displayableNode, event);
    }

}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLLinearTransformsDisplayableManager3D->UpdateFromMRML: Scene is not set.");
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLTransformNode* mNode = nullptr;
  std::vector<vtkMRMLNode *> mNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkMRMLTransformNode", mNodes) : 0;
  for (int i=0; i<nnodes; i++)
    {
    mNode  = vtkMRMLTransformNode::SafeDownCast(mNodes[i]);
    if (mNode && this->Internal->UseDisplayableNode(mNode))
      {
      this->Internal->AddTransformNode(mNode);
      }
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::Create()
{
  Superclass::Create();
  this->Internal->SetupRenderer();
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
vtkMRMLTransformHandleWidget* vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::FindClosestWidget(vtkMRMLInteractionEventData* callData,
  double& closestDistance2)
{
  vtkMRMLTransformHandleWidget* closestWidget = nullptr;
  closestDistance2 = VTK_DOUBLE_MAX;

  for (std::pair<vtkMRMLTransformDisplayNode*, vtkMRMLTransformHandleWidget*> widgetIterator : this->InteractionPipelines)
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
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::SetupRenderer()
{
  vtkRenderer* renderer = this->External->GetRenderer();
  if (renderer == nullptr)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::SetupRenderer() failed: renderer is invalid");
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
bool vtkMRMLLinearTransformsDisplayableManager3D::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2)
{
  // New point can be placed anywhere
  int eventid = eventData->GetType();

  if (eventid == vtkCommand::LeaveEvent && this->Internal->LastActiveWidget != nullptr)
    {
    if (this->Internal->LastActiveWidget->GetDisplayNode()
      && this->Internal->LastActiveWidget->GetDisplayNode()->GetActiveInteractionType() > vtkMRMLTransformHandleWidget::InteractionNone)
      {
      // this widget has active component, therefore leave event is relevant
      closestDistance2 = 0.0;
      return this->Internal->LastActiveWidget;
      }
    }

  // Other interactions
  bool canProcess = (this->Internal->FindClosestWidget(eventData, closestDistance2) != nullptr);

  if (!canProcess && this->Internal->LastActiveWidget != nullptr
    && (eventid == vtkCommand::MouseMoveEvent || eventid == vtkCommand::Move3DEvent) )
    {
    // TODO: handle multiple contexts
    this->Internal->LastActiveWidget->Leave(eventData);
    this->Internal->LastActiveWidget = nullptr;
    }

  return canProcess;
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager3D::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
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
