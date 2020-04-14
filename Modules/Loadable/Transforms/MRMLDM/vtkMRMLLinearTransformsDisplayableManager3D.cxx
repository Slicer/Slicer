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
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget2.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkLineSource.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPickingManager.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

const double DEFAULT_SCALE[3] = {100.0, 100.0, 100.0};

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkMRMLLinearTransformsDisplayableManager3D );

//---------------------------------------------------------------------------
// vtkMRMLLinearTransformsDisplayableManager3D Callback
class vtkLinearTransformWidgetCallback : public vtkCommand
{
public:
  static vtkLinearTransformWidgetCallback *New()
  { return new vtkLinearTransformWidgetCallback; }

  vtkLinearTransformWidgetCallback() = default;

  void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void *vtkNotUsed(callData)) override
  {
    if ((event == vtkCommand::StartInteractionEvent) || (event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
      {
      // sanity checks
      if (!this->DisplayableManager)
        {
        return;
        }
      if (!this->Node)
        {
        return;
        }
      if (!this->Widget)
        {
        return;
        }
      // sanity checks end
      }

    if (event == vtkCommand::StartInteractionEvent)
      {
      // save the state of the node when starting interaction
      if (this->Node->GetScene())
        {
        this->Node->GetScene()->SaveStateForUndo();
        }
      }
  }

  void SetWidget(vtkAbstractWidget *w)
    {
    this->Widget = w;
    }
  void SetNode(vtkMRMLTransformNode *n)
    {
    this->Node = n;
    }
  void SetDisplayableManager(vtkMRMLLinearTransformsDisplayableManager3D* dm)
    {
    this->DisplayableManager = dm;
    }

  vtkAbstractWidget * Widget{nullptr};
  vtkMRMLTransformNode* Node{nullptr};
  vtkMRMLLinearTransformsDisplayableManager3D* DisplayableManager{nullptr};
};

//---------------------------------------------------------------------------
class vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
{
public:

  vtkInternal(vtkMRMLLinearTransformsDisplayableManager3D* external);
  ~vtkInternal();

  struct Pipeline
    {
    vtkSmartPointer<vtkBoxWidget2> Widget;
    vtkSmartPointer<vtkTransform> WidgetDisplayTransform;
    bool UpdateWidgetBounds;
    };

  typedef std::map < vtkMRMLTransformDisplayNode*, Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLTransformNode*, std::set< vtkMRMLTransformDisplayNode* > > TransformToDisplayCacheType;
  TransformToDisplayCacheType TransformToDisplayNodes;

  typedef std::map < vtkBoxWidget2*, vtkMRMLTransformDisplayNode* > WidgetToNodeMapType;
  WidgetToNodeMapType WidgetMap;

  // Transforms
  void AddTransformNode(vtkMRMLTransformNode* displayableNode);
  void RemoveTransformNode(vtkMRMLTransformNode* displayableNode);
  void UpdateDisplayableTransforms(vtkMRMLTransformNode *node, bool);

  // Display Nodes
  void AddDisplayNode(vtkMRMLTransformNode*, vtkMRMLTransformDisplayNode*);
  void UpdateDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkMRMLTransformDisplayNode*, Pipeline*);
  void RemoveDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  void SetTransformDisplayProperty(vtkMRMLTransformDisplayNode *displayNode, vtkActor* actor);

  // Widget
  void UpdateWidgetDisplayTransform(Pipeline*, vtkMRMLTransformNode*);
  void UpdateWidgetFromNode(vtkMRMLTransformDisplayNode*, vtkMRMLTransformNode*, Pipeline*);
  void UpdateNodeFromWidget(vtkBoxWidget2*);

  // Observations
  void AddObservations(vtkMRMLTransformNode* node);
  void RemoveObservations(vtkMRMLTransformNode* node);
  void AddDisplayObservations(vtkMRMLTransformDisplayNode* node);
  void RemoveDisplayObservations(vtkMRMLTransformDisplayNode* node);

  // Helper functions
  bool UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLTransformNode* node);
  void ClearDisplayableNodes();

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
   // allow annotations to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->External->GetMRMLViewNode()->GetID()))
    {
    return false;
    }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkMRMLTransformDisplayNode");

  return use;
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
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::UpdateDisplayableTransforms(vtkMRMLTransformNode* mNode, bool updateBounds)
{
  // Update the pipeline for all tracked DisplayableNode
  PipelinesCacheType::iterator pipelinesIter;
  std::set< vtkMRMLTransformDisplayNode* > displayNodes = this->TransformToDisplayNodes[mNode];
  std::set< vtkMRMLTransformDisplayNode* >::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      Pipeline* pipeline = pipelinesIter->second;
      pipeline->UpdateWidgetBounds |= updateBounds;
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipeline);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::RemoveDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
{
  PipelinesCacheType::iterator actorsIt = this->DisplayPipelines.find(displayNode);
  if(actorsIt == this->DisplayPipelines.end())
    {
    return;
    }
  this->RemoveDisplayObservations(displayNode);
  Pipeline* pipeline = actorsIt->second;
  this->WidgetMap.erase(pipeline->Widget);
  delete pipeline;
  this->DisplayPipelines.erase(actorsIt);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::AddDisplayNode(vtkMRMLTransformNode* mNode, vtkMRMLTransformDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a transform node already associated with a display node
  // is copied into an other (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    return;
    }

  this->AddDisplayObservations(displayNode);

  // Create pipeline
  Pipeline* pipeline = new Pipeline();
  pipeline->UpdateWidgetBounds = true;
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Interaction VTK
  //  - Widget
  pipeline->Widget = vtkSmartPointer<vtkBoxWidget2>::New();
  vtkNew<vtkBoxRepresentation> WidgetRep;
  pipeline->Widget->SetRepresentation(WidgetRep.GetPointer());
  //  - Transform
  pipeline->WidgetDisplayTransform = vtkSmartPointer<vtkTransform>::New();
  // - Widget events
  pipeline->Widget->AddObserver(
    vtkCommand::InteractionEvent, this->External->GetWidgetsCallbackCommand());

  if (this->External->GetInteractor()->GetPickingManager())
    {
    if (!(this->External->GetInteractor()->GetPickingManager()->GetEnabled()))
      {
      // if the picking manager is not already turned on for this
      // interactor, enable it
      this->External->GetInteractor()->GetPickingManager()->EnabledOn();
      }
    }

  // Add actor / set renderers and cache
  pipeline->Widget->SetInteractor(this->External->GetInteractor());
  pipeline->Widget->SetCurrentRenderer(this->External->GetRenderer());
  this->WidgetMap.insert( std::make_pair(pipeline->Widget, displayNode) );

  // add the callback
  vtkLinearTransformWidgetCallback *widgetCallback = vtkLinearTransformWidgetCallback::New();
  widgetCallback->SetNode(mNode);
  widgetCallback->SetWidget(pipeline->Widget);
  widgetCallback->SetDisplayableManager(this->External);
  pipeline->Widget->AddObserver(vtkCommand::StartInteractionEvent, widgetCallback);
  pipeline->Widget->AddObserver(vtkCommand::EndInteractionEvent, widgetCallback);
  pipeline->Widget->AddObserver(vtkCommand::InteractionEvent, widgetCallback);
  widgetCallback->Delete();

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode, true);
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
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    this->UpdateDisplayNodePipeline(displayNode, it->second);
    }
  else
    {
    this->AddTransformNode( vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode()) );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
::UpdateWidgetDisplayTransform(
  Pipeline* pipeline, vtkMRMLTransformNode* transformNode)
{
  // WidgetDisplayTransform is in charge of moving the widget around
  // the bounding box of the objects under the transform, wherever they
  // are.
  assert(pipeline && transformNode);

  std::vector<vtkMRMLDisplayableNode*> transformedNodes;
  vtkSlicerTransformLogic::GetTransformedNodes(
    this->External->GetMRMLScene(), transformNode, transformedNodes, true);

  bool validBounds = false;
  double bounds[6];
  if (transformedNodes.size() > 0)
    {
    vtkSlicerTransformLogic::GetNodesBounds(transformedNodes, bounds);
    validBounds =
      (bounds[0] <= bounds[1] || bounds[2] <= bounds[3] || bounds[4] <= bounds[5]);
    }

  if (validBounds)
    {
    // Get the bounding box around the UNTRANSFORMED objects so we have
    // the actual box around the object.
    double center[3], scales[3];
    for (int i = 0; i < 3; ++i)
      {
      double scale = 0.5*(bounds[2*i + 1] - bounds[2*i]);
      center[i] = bounds[2*i] + scale;
      scales[i] = 4*scale;
      }

    pipeline->WidgetDisplayTransform->Identity();
    pipeline->WidgetDisplayTransform->Translate(center);
    pipeline->WidgetDisplayTransform->Scale(scales);
    }
  else
    {
    // No objects, just add a default scaling so the widget can be interacted
    // with more easily.
    pipeline->WidgetDisplayTransform->Identity();
    pipeline->WidgetDisplayTransform->Scale(DEFAULT_SCALE);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
::UpdateWidgetFromNode(vtkMRMLTransformDisplayNode* displayNode,
                       vtkMRMLTransformNode* transformNode,
                       Pipeline* pipeline)
{
  assert(displayNode && transformNode && pipeline);

  if (pipeline->UpdateWidgetBounds)
    {
    this->UpdateWidgetDisplayTransform(pipeline, transformNode);
    pipeline->UpdateWidgetBounds = false;
    }

  vtkNew<vtkMatrix4x4> toWorldMatrix;
  transformNode->GetMatrixTransformToWorld(toWorldMatrix.GetPointer());
  vtkNew<vtkTransform> widgetTransform;
  widgetTransform->Concatenate(toWorldMatrix.GetPointer());
  widgetTransform->Concatenate(pipeline->WidgetDisplayTransform);

  vtkBoxRepresentation* representation =
    vtkBoxRepresentation::SafeDownCast(pipeline->Widget->GetRepresentation());

  representation->SetTransform(widgetTransform.GetPointer());

  pipeline->Widget->SetTranslationEnabled(
    displayNode->GetEditorTranslationEnabled());
  pipeline->Widget->SetRotationEnabled(
    displayNode->GetEditorRotationEnabled());
  pipeline->Widget->SetScalingEnabled(
    displayNode->GetEditorScalingEnabled());
  pipeline->Widget->SetMoveFacesEnabled(
    displayNode->GetEditorScalingEnabled());
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
::UpdateNodeFromWidget(vtkBoxWidget2* widget)
{
  assert(widget);
  vtkMRMLTransformDisplayNode* displayNode = this->WidgetMap[widget];
  assert(displayNode);
  Pipeline* pipeline = this->DisplayPipelines[displayNode];
  vtkMRMLTransformNode* node =
    vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());

  vtkBoxRepresentation* representation =
    vtkBoxRepresentation::SafeDownCast(widget->GetRepresentation());

  vtkNew<vtkTransform> widgetTransform;
  representation->GetTransform(widgetTransform.GetPointer());

  vtkNew<vtkTransform> toParent;
  vtkMRMLTransformNode* parentNode = node->GetParentTransformNode();
  if (parentNode)
    {
    vtkNew<vtkMatrix4x4> worldFromParentMatrix;
    parentNode->GetMatrixTransformFromWorld(worldFromParentMatrix.GetPointer());
    toParent->Concatenate(worldFromParentMatrix.GetPointer());
    }
  toParent->Concatenate(widgetTransform.GetPointer());
  toParent->Concatenate(pipeline->WidgetDisplayTransform->GetLinearInverse());

  node->SetMatrixTransformToParent(toParent->GetMatrix());
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
::UpdateDisplayNodePipeline(
  vtkMRMLTransformDisplayNode* displayNode,
  Pipeline* pipeline)
{
  if (!displayNode || !pipeline)
    {
    return;
    }

  vtkMRMLTransformNode* transformNode =
    vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (transformNode==nullptr)
    {
    pipeline->Widget->SetEnabled(false);
    return;
    }

  if (!transformNode->IsLinear())
    {
    vtkDebugWithObjectMacro(transformNode, "Cannot show interactive widget: Transform is not linear");
    pipeline->Widget->SetEnabled(false);
    return;
    }

  bool visible = displayNode->GetEditorVisibility();
  pipeline->Widget->SetEnabled(visible);
  if (visible)
    {
    this->UpdateWidgetFromNode(displayNode, transformNode, pipeline);
    return;
    }
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
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
::AddDisplayObservations(vtkMRMLTransformDisplayNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkMRMLTransformDisplayNode::TransformUpdateEditorBoundsEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLTransformDisplayNode::TransformUpdateEditorBoundsEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal::RemoveObservations(vtkMRMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  observations = broker->GetObservations(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
::RemoveDisplayObservations(vtkMRMLTransformDisplayNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkMRMLTransformDisplayNode::TransformUpdateEditorBoundsEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
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
  os << indent << "vtkMRMLLinearTransformsDisplayableManager3D: "
     << this->GetClassName() << "\n";
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

  if ( scene->IsBatchProcessing() )
    {
    return;
    }

  vtkMRMLTransformNode* displayableNode = vtkMRMLTransformNode::SafeDownCast(caller);
  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(caller);

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
      this->Internal->UpdateDisplayableTransforms(displayableNode, false);
      this->RequestRender();
      }
    }
  else if ( displayNode )
    {
    displayableNode = vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
    if ( displayNode && event == vtkMRMLTransformDisplayNode::TransformUpdateEditorBoundsEvent)
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode, true);
      this->RequestRender();
      }
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
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
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLLinearTransformsDisplayableManager3D
::ProcessWidgetsEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessWidgetsEvents(caller, event, callData);

  vtkBoxWidget2* boxWidget = vtkBoxWidget2::SafeDownCast(caller);
  if (boxWidget)
    {
    this->Internal->UpdateNodeFromWidget(boxWidget);
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
vtkAbstractWidget* vtkMRMLLinearTransformsDisplayableManager3D
::GetWidget(vtkMRMLTransformDisplayNode* displayNode)
{
  vtkMRMLLinearTransformsDisplayableManager3D::vtkInternal
    ::PipelinesCacheType::iterator pipelineIter =
      this->Internal->DisplayPipelines.find(displayNode);
  if (pipelineIter != this->Internal->DisplayPipelines.end())
    {
    return pipelineIter->second->Widget;
    }
  return nullptr;
}
