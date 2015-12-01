/*==============================================================================

  Program: 3D Slicer

  Copyright (c)

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLModelSliceDisplayableManager.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkEventBroker.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWeakPointer.h>
#include <vtkPointLocator.h>
#include <vtkVersion.h>

// VTK includes: customization
#include <vtkCutter.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <set>
#include <map>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLModelSliceDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLModelSliceDisplayableManager::vtkInternal
{
public:
  struct Pipeline
    {
    vtkSmartPointer<vtkGeneralTransform> NodeToWorld;
    vtkSmartPointer<vtkTransform> TransformToSlice;
    vtkSmartPointer<vtkTransformPolyDataFilter> Transformer;
    vtkSmartPointer<vtkTransformPolyDataFilter> ModelWarper;
    vtkSmartPointer<vtkPlane> Plane;
    vtkSmartPointer<vtkCutter> Cutter;
    vtkSmartPointer<vtkProp> Actor;
    };

  typedef std::map < vtkMRMLDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLDisplayableNode*, std::set< vtkMRMLDisplayNode* > > ModelToDisplayCacheType;
  ModelToDisplayCacheType ModelToDisplayNodes;

  // Transforms
  void UpdateDisplayableTransforms(vtkMRMLDisplayableNode *node);
  void GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld);
  // Slice Node
  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  void UpdateSliceNode();
  void SetSlicePlaneFromMatrix(vtkMatrix4x4* matrix, vtkPlane* plane);

  // Display Nodes
  void AddDisplayNode(vtkMRMLDisplayableNode*, vtkMRMLDisplayNode*);
  void UpdateDisplayNode(vtkMRMLDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkMRMLDisplayNode*, const Pipeline*);
  void RemoveDisplayNode(vtkMRMLDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLDisplayableNode* node);
  void RemoveObservations(vtkMRMLDisplayableNode* node);
  bool IsNodeObserved(vtkMRMLDisplayableNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLDisplayableNode* displayNode);
  void ClearDisplayableNodes();

  vtkInternal( vtkMRMLModelSliceDisplayableManager* external );
  ~vtkInternal();

private:
  vtkSmartPointer<vtkMatrix4x4> SliceXYToRAS;
  vtkSmartPointer<vtkMRMLSliceNode> SliceNode;
  vtkMRMLModelSliceDisplayableManager* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods
vtkMRMLModelSliceDisplayableManager::vtkInternal
::vtkInternal(vtkMRMLModelSliceDisplayableManager* external)
{
  this->External = external;
  this->SliceXYToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  this->SliceXYToRAS->Identity();
}

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::vtkInternal
::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal
::UseDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  // Check whether DisplayNode should be shown in this view
  bool show = displayNode
              && displayNode->IsA("vtkMRMLModelDisplayNode")
              && ( !displayNode->IsA("vtkMRMLFiberBundleLineDisplayNode") )
              && ( !displayNode->IsA("vtkMRMLFiberBundleGlyphDisplayNode") ) ;
  return show;
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal
::IsVisible(vtkMRMLDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return 0;
    }
  bool visibleOnNode = true;
  // for volume slice model display nodes, don't want to only show the slice
  // intersection if the slice is visible in 3d, but do want to do the check for
  // other types of models
  bool isSliceModel = vtkMRMLSliceLogic::IsSliceModelDisplayNode(displayNode);
  if (!isSliceModel)
    {
    vtkMRMLSliceNode *sliceNode = this->SliceNode;
    if (sliceNode)
      {
      visibleOnNode = displayNode->GetVisibility(sliceNode->GetID());
      }
    else
      {
      visibleOnNode = (displayNode->GetVisibility() == 1 ? true : false);
      }
    }
  return visibleOnNode && (displayNode->GetSliceIntersectionVisibility() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
    {
    return;
    }
  this->SliceNode = sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateSliceNode()
{
  // Update the Slice node transform
  //   then update the DisplayNode pipelines to account for plane location

  this->SliceXYToRAS->DeepCopy( this->SliceNode->GetXYToRAS() );
  PipelinesCacheType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end(); ++it)
    {
    this->UpdateDisplayNodePipeline(it->first, it->second);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::SetSlicePlaneFromMatrix(vtkMatrix4x4* sliceMatrix, vtkPlane* plane)
{
  double normal[3];
  double origin[3];

  // +/-1: orientation of the normal
  const int planeOrientation = 1;
  for (int i = 0; i < 3; i++)
    {
    normal[i] = planeOrientation * sliceMatrix->GetElement(i,2);
    origin[i] = sliceMatrix->GetElement(i,3);
    }

  plane->SetNormal(normal);
  plane->SetOrigin(origin);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld)
{
  if (!node || !transformToWorld)
    {
    return;
    }

  vtkMRMLTransformNode* tnode =
    node->GetParentTransformNode();

  transformToWorld->Identity();
  if (tnode)
    {
    tnode->GetTransformToWorld(transformToWorld);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayableTransforms(vtkMRMLDisplayableNode* mNode)
{
  // Update the NodeToWorld matrix for all tracked DisplayableNode

  PipelinesCacheType::iterator pipelinesIter;
  std::set<vtkMRMLDisplayNode *> displayNodes = this->ModelToDisplayNodes[mNode];
  std::set<vtkMRMLDisplayNode *>::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      this->GetNodeTransformToWorld( mNode, pipelinesIter->second->NodeToWorld);
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::RemoveDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  PipelinesCacheType::iterator actorsIt = this->DisplayPipelines.find(displayNode);
  if(actorsIt == this->DisplayPipelines.end())
    {
    return;
    }
  const Pipeline* pipeline = actorsIt->second;
  this->External->GetRenderer()->RemoveActor(pipeline->Actor);
  delete pipeline;
  this->DisplayPipelines.erase(actorsIt);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::AddDisplayNode(vtkMRMLDisplayableNode* mNode, vtkMRMLDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a model node already associated with a display node
  // is copied into an other (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    return;
    }

  vtkNew<vtkActor2D> actor;
  if (displayNode->IsA("vtkMRMLModelDisplayNode"))
    {
    actor->SetMapper( vtkNew<vtkPolyDataMapper2D>().GetPointer() );
    }

  // Create pipeline
  Pipeline* pipeline = new Pipeline();
  pipeline->Actor = actor.GetPointer();
  pipeline->Cutter = vtkSmartPointer<vtkCutter>::New();
  pipeline->TransformToSlice = vtkSmartPointer<vtkTransform>::New();
  pipeline->NodeToWorld = vtkSmartPointer<vtkGeneralTransform>::New();
  pipeline->Transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  pipeline->ModelWarper = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  pipeline->Plane = vtkSmartPointer<vtkPlane>::New();

  // Set up pipeline
  pipeline->Transformer->SetTransform(pipeline->TransformToSlice);
  pipeline->Transformer->SetInputConnection(pipeline->Cutter->GetOutputPort());
  pipeline->Cutter->SetCutFunction(pipeline->Plane);
  pipeline->Cutter->SetGenerateCutScalars(0);
  pipeline->Cutter->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
  pipeline->Actor->SetVisibility(0);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayNode(vtkMRMLDisplayNode* displayNode)
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
    this->External->AddDisplayableNode( displayNode->GetDisplayableNode() );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayNodePipeline(vtkMRMLDisplayNode* displayNode, const Pipeline* pipeline)
{
  // Sets visibility, set pipeline polydata input, update color
  //   calculate and set pipeline transforms.

  if (!displayNode || !pipeline)
    {
    return;
    }

  // Update visibility
  bool visible = this->IsVisible(displayNode);
  pipeline->Actor->SetVisibility(visible);

  if (visible)
    {
    vtkMRMLModelDisplayNode* modelDisplayNode =
      vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
    vtkPolyData* polyData = modelDisplayNode->GetOutputPolyData();
    if (!polyData)
      {
      return;
      }
    pipeline->ModelWarper->SetInputData(polyData);
    // need this to update bounds of the locator, to avoid crash in the cutter
    modelDisplayNode->GetOutputPolyDataConnection()->GetProducer()->Update();

    pipeline->ModelWarper->SetTransform(pipeline->NodeToWorld);

    //  Set Plane Transform
    this->SetSlicePlaneFromMatrix(this->SliceXYToRAS, pipeline->Plane);
    pipeline->Plane->Modified();

    //  Set PolyData Transform
    vtkNew<vtkMatrix4x4> rasToSliceXY;
    vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY.GetPointer());
    pipeline->TransformToSlice->SetMatrix(rasToSliceXY.GetPointer());

    // optimization for slice to slice intersections which are 1 quad polydatas
    // no need for 50^3 default locator divisons
    if (polyData->GetPoints() != NULL && polyData->GetNumberOfPoints() <= 4)
    {
      vtkNew<vtkPointLocator> locator;
      double *bounds = polyData->GetBounds();
      locator->SetDivisions(2,2,2);
      locator->InitPointInsertion(polyData->GetPoints(), bounds);
      pipeline->Cutter->SetLocator(locator.GetPointer());
    }

    // Update pipeline actor
    vtkActor2D* actor = vtkActor2D::SafeDownCast(pipeline->Actor);
    vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(
      actor->GetMapper());
    mapper->SetInputConnection( pipeline->Transformer->GetOutputPort() );
    mapper->SetLookupTable( displayNode->GetColorNode() ?
                            displayNode->GetColorNode()->GetScalarsToColors() : 0);
    mapper->SetScalarRange(modelDisplayNode->GetScalarRange());
    actor->SetPosition(0,0);
    vtkProperty2D* actorProperties = actor->GetProperty();
    actorProperties->SetColor(displayNode->GetColor() );
    actorProperties->SetLineWidth(displayNode->GetSliceIntersectionThickness() );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::AddObservations(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;

  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::TransformModifiedEvent,
                                          this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::TransformModifiedEvent,
                            this->External, this->External->GetMRMLNodesCallbackCommand() );
    }

  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                                          this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                            this->External, this->External->GetMRMLNodesCallbackCommand() );
    }

  if (!broker->GetObservationExist(node, vtkMRMLModelNode::PolyDataModifiedEvent,
                                          this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLModelNode::PolyDataModifiedEvent,
                            this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::RemoveObservations(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(
    node, vtkMRMLModelNode::PolyDataModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkMRMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal
::IsNodeObserved(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkCollection* observations;
  observations = broker->GetObservationsForSubject(node);
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
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::ClearDisplayableNodes()
{
  while(this->ModelToDisplayNodes.size() > 0)
    {
    this->External->RemoveDisplayableNode(this->ModelToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal
::UseDisplayableNode(vtkMRMLDisplayableNode* node)
{
  bool show = node && node->IsA("vtkMRMLModelNode");

  if (!this->SliceNode->GetLayoutName())
    {
    vtkErrorWithObjectMacro(this->External, "No layout name to slice node " << this->SliceNode->GetID());
    return false;
    }
  // Ignore the slice model node for this slice DM.
  std::string cmpstr = std::string(this->SliceNode->GetLayoutName()) + " Volume Slice";
  show = show && ( cmpstr.compare(node->GetName()) );

  return show;
}

//---------------------------------------------------------------------------
// vtkMRMLModelSliceDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::vtkMRMLModelSliceDisplayableManager()
{
  this->Internal = new vtkInternal(this);
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::~vtkMRMLModelSliceDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::AddDisplayableNode(
  vtkMRMLDisplayableNode* node)
{
  if (this->AddingDisplayableNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->Internal->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingDisplayableNode = 1;
  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();

  this->Internal->AddObservations(node);

  for (int i=0; i<nnodes; i++)
  {
    vtkMRMLDisplayNode *dnode = node->GetNthDisplayNode(i);
    if ( this->Internal->UseDisplayNode(dnode) )
      {
      this->Internal->ModelToDisplayNodes[node].insert(dnode);
      this->Internal->AddDisplayNode( node, dnode );
      }
    }
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::RemoveDisplayableNode(vtkMRMLDisplayableNode* node)
{
  if (!node)
    {
    return;
    }
  vtkInternal::ModelToDisplayCacheType::iterator displayableIt =
    this->Internal->ModelToDisplayNodes.find(node);
  if(displayableIt == this->Internal->ModelToDisplayNodes.end())
    {
    return;
    }

  std::set<vtkMRMLDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkMRMLDisplayNode *>::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->Internal->RemoveDisplayNode(*diter);
    }
  this->Internal->RemoveObservations(node);
  this->Internal->ModelToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node->IsA("vtkMRMLModelNode"))
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromMRMLRequested(1);
    return;
    }

  this->AddDisplayableNode(vtkMRMLDisplayableNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node && !node->IsA("vtkMRMLModelNode")
       && !node->IsA("vtkMRMLModelDisplayNode") )
    {
    return;
    }

  vtkMRMLDisplayableNode* modelNode = NULL;
  vtkMRMLDisplayNode* displayNode = NULL;

  bool modified = false;
  if ( (modelNode = vtkMRMLDisplayableNode::SafeDownCast(node)) )
    {
    this->RemoveDisplayableNode(modelNode);
    modified = true;
    }
  else if ( (displayNode = vtkMRMLDisplayNode::SafeDownCast(node)) )
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
void vtkMRMLModelSliceDisplayableManager
::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if ( scene->IsBatchProcessing() )
    {
    return;
    }

  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(caller);

  if ( displayableNode )
    {
    vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode *> (callData);
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(callDataNode);

    if ( displayNode && (event == vtkMRMLDisplayableNode::DisplayModifiedEvent) )
      {
      this->Internal->UpdateDisplayNode(displayNode);
      this->RequestRender();
      }
    else if ( (event == vtkMRMLDisplayableNode::TransformModifiedEvent)
             || (event == vtkMRMLModelNode::PolyDataModifiedEvent))
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
      }
    }
  else if ( vtkMRMLSliceNode::SafeDownCast(caller) )
      {
      this->Internal->UpdateSliceNode();
      this->RequestRender();
      }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(0);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLModelSliceDisplayableManager->UpdateFromMRML: Scene is not set.")
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLDisplayableNode* mNode = NULL;
  std::vector<vtkMRMLNode *> mNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkMRMLDisplayableNode", mNodes) : 0;
  for (int i=0; i<nnodes; i++)
    {
    mNode  = vtkMRMLDisplayableNode::SafeDownCast(mNodes[i]);
    if (mNode && this->Internal->UseDisplayableNode(mNode))
      {
      this->AddDisplayableNode(mNode);
      }
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(1);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(1);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::Create()
{
  this->Internal->SetSliceNode(this->GetMRMLSliceNode());
  this->SetUpdateFromMRMLRequested(1);
}
