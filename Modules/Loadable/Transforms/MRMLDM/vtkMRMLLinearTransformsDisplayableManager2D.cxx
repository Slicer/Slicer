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
#include "vtkMRMLLinearTransformsDisplayableManager2D.h"
#include <vtkMRMLTransformHandleWidget.h>

// MRMLDM includes
#include <vtkMRMLInteractionEventData.h>

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkEventBroker.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWeakPointer.h>
#include <vtkPointLocator.h>
#include <vtkRenderWindow.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <set>
#include <map>

//---------------------------------------------------------------------------
static const int RENDERER_LAYER = 1;

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLinearTransformsDisplayableManager2D);

//---------------------------------------------------------------------------
class vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal
{
public:

  vtkInternal( vtkMRMLLinearTransformsDisplayableManager2D* external );
  ~vtkInternal();

  typedef std::map < vtkMRMLTransformNode*, std::set< vtkMRMLTransformDisplayNode* > > TransformToDisplayCacheType;
  TransformToDisplayCacheType TransformToDisplayNodes;

  typedef std::map < vtkMRMLTransformDisplayNode*, vtkSmartPointer<vtkMRMLTransformHandleWidget> > InteractionWidgetsCacheType;
  InteractionWidgetsCacheType InteractionWidgets;

  // Transforms
  void AddTransformNode(vtkMRMLTransformNode* displayableNode);
  void RemoveTransformNode(vtkMRMLTransformNode* displayableNode);
  void UpdateDisplayableTransforms(vtkMRMLTransformNode *node);

  // Slice Node
  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  void UpdateSliceNode();

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

  void SetupRenderer();

  vtkSmartPointer<vtkRenderer> InteractionRenderer;

  vtkWeakPointer<vtkMRMLTransformHandleWidget> LastActiveWidget;
  vtkMRMLTransformHandleWidget* FindClosestWidget(vtkMRMLInteractionEventData* callData, double& closestDistance2);

private:
  vtkMRMLLinearTransformsDisplayableManager2D* External;
  bool AddingTransformNode;
  vtkSmartPointer<vtkMRMLSliceNode> SliceNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::vtkInternal(vtkMRMLLinearTransformsDisplayableManager2D* external)
: External(external)
, AddingTransformNode(false)
{
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
  this->SliceNode = nullptr;
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
   // allow nodes to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->SliceNode->GetID()))
    {
    return false;
    }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkMRMLTransformDisplayNode");

  return use;
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::IsVisible(vtkMRMLTransformDisplayNode* displayNode)
{
  return displayNode && (displayNode->GetVisibility() != 0) && (displayNode->GetVisibility2D() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
    {
    return;
    }
  this->SliceNode=sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::UpdateSliceNode()
{
  // Update the Slice node transform
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::AddTransformNode(vtkMRMLTransformNode* node)
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
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::RemoveTransformNode(vtkMRMLTransformNode* node)
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

  std::set< vtkMRMLTransformDisplayNode* > dnodes = displayableIt->second;
  std::set< vtkMRMLTransformDisplayNode* >::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->RemoveDisplayNode(*diter);
    }
  this->RemoveObservations(node);
  this->TransformToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::UpdateDisplayableTransforms(vtkMRMLTransformNode* vtkNotUsed(mNode))
{
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::RemoveDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
  this->InteractionWidgets.erase(displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::AddDisplayNode(vtkMRMLTransformNode* mNode, vtkMRMLTransformDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a transform node already associated with a display node
  // is copied into an other (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  InteractionWidgetsCacheType::iterator it;
  it = this->InteractionWidgets.find(displayNode);
  if (it != this->InteractionWidgets.end())
    {
    return;
    }

  vtkNew<vtkMRMLTransformHandleWidget> interactionWidget;
  interactionWidget->CreateDefaultRepresentation(displayNode, this->External->GetMRMLSliceNode(), this->InteractionRenderer);
  this->InteractionWidgets.insert(std::make_pair(displayNode, interactionWidget.GetPointer()));

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::UpdateDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update.
  //   otherwise, add as new node

  if (!displayNode)
    {
    return;
    }

  vtkSmartPointer<vtkMRMLTransformHandleWidget> widget;
  InteractionWidgetsCacheType::iterator pipelineIt;
  pipelineIt = this->InteractionWidgets.find(displayNode);

  bool visible = displayNode->GetEditorVisibility() && displayNode->GetEditorSliceIntersectionVisibility();
  if (visible && pipelineIt == this->InteractionWidgets.end())
    {
    // No pipeline, yet interaction visibility is on, create a new one

    vtkNew<vtkMRMLTransformHandleWidget> interactionWidget;
    interactionWidget->CreateDefaultRepresentation(displayNode, this->External->GetMRMLSliceNode(), this->InteractionRenderer);
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
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::AddObservations(vtkMRMLTransformNode* node)
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
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::RemoveObservations(vtkMRMLTransformNode* node)
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
bool vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::IsNodeObserved(vtkMRMLTransformNode* node)
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
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::ClearDisplayableNodes()
{
  while(this->TransformToDisplayNodes.size() > 0)
    {
    this->RemoveTransformNode(this->TransformToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::UseDisplayableNode(vtkMRMLTransformNode* node)
{
  bool use = node && node->IsA("vtkMRMLTransformNode");
  return use;
}

//---------------------------------------------------------------------------
// vtkMRMLLinearTransformsDisplayableManager2D methods

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager2D::vtkMRMLLinearTransformsDisplayableManager2D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLLinearTransformsDisplayableManager2D::~vtkMRMLLinearTransformsDisplayableManager2D()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkMRMLLinearTransformsDisplayableManager2D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
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
void vtkMRMLLinearTransformsDisplayableManager2D::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
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
void vtkMRMLLinearTransformsDisplayableManager2D::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene == nullptr || scene->IsBatchProcessing())
    {
    return;
    }

  vtkMRMLTransformNode* displayableNode = vtkMRMLTransformNode::SafeDownCast(caller);
  if ( displayableNode )
    {
    vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode *> (callData);
    vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(callDataNode);

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

    for (std::pair<vtkMRMLTransformDisplayNode*, vtkMRMLTransformHandleWidget*> interactionWidget : this->Internal->InteractionWidgets)
      {
      interactionWidget.second->UpdateFromMRML(displayableNode, event, callData);
      if (interactionWidget.second->GetNeedToRender())
        {
        this->RequestRender();
        }
      }
    }
  else if ( vtkMRMLSliceNode::SafeDownCast(caller) )
      {
      this->Internal->UpdateSliceNode();
      this->RequestRender();

      for (auto interactionWidget : this->Internal->InteractionWidgets)
        {
        interactionWidget.second->UpdateFromMRML(this->GetMRMLSliceNode(), event, callData);
        }
      }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLLinearTransformsDisplayableManager2D->UpdateFromMRML: Scene is not set.");
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
void vtkMRMLLinearTransformsDisplayableManager2D::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager2D::Create()
{
  this->Internal->SetSliceNode(this->GetMRMLSliceNode());
  this->Internal->SetupRenderer();
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
vtkMRMLTransformHandleWidget* vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::FindClosestWidget(vtkMRMLInteractionEventData* callData,
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
void vtkMRMLLinearTransformsDisplayableManager2D::vtkInternal::SetupRenderer()
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
bool vtkMRMLLinearTransformsDisplayableManager2D::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2)
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
bool vtkMRMLLinearTransformsDisplayableManager2D::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
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
