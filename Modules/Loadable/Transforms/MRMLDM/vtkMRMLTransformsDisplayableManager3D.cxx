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
#include "vtkMRMLTransformsDisplayableManager3D.h"

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
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkMRMLTransformsDisplayableManager3D );

//---------------------------------------------------------------------------
class vtkMRMLTransformsDisplayableManager3D::vtkInternal
{
public:

  vtkInternal(vtkMRMLTransformsDisplayableManager3D* external);
  ~vtkInternal();

  struct Pipeline
    {
    vtkSmartPointer<vtkActor> Actor;
    vtkSmartPointer<vtkPolyData> InputPolyData;
    };

  typedef std::map < vtkMRMLTransformDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLTransformNode*, std::set< vtkMRMLTransformDisplayNode* > > TransformToDisplayCacheType;
  TransformToDisplayCacheType TransformToDisplayNodes;

  // Transforms
  void AddTransformNode(vtkMRMLTransformNode* displayableNode);
  void RemoveTransformNode(vtkMRMLTransformNode* displayableNode);
  void UpdateDisplayableTransforms(vtkMRMLTransformNode *node);

  // Display Nodes
  void AddDisplayNode(vtkMRMLTransformNode*, vtkMRMLTransformDisplayNode*);
  void UpdateDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkMRMLTransformDisplayNode*, const Pipeline*);
  void RemoveDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  void SetTransformDisplayProperty(vtkMRMLTransformDisplayNode *displayNode, vtkActor* actor);

  // Observations
  void AddObservations(vtkMRMLTransformNode* node);
  void RemoveObservations(vtkMRMLTransformNode* node);
  bool IsNodeObserved(vtkMRMLTransformNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLTransformDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLTransformNode* node);
  void ClearDisplayableNodes();

private:
  vtkMRMLTransformsDisplayableManager3D* External;
  bool AddingTransformNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLTransformsDisplayableManager3D::vtkInternal::vtkInternal(vtkMRMLTransformsDisplayableManager3D * external)
: External(external)
, AddingTransformNode(false)
{
}

//---------------------------------------------------------------------------
vtkMRMLTransformsDisplayableManager3D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformsDisplayableManager3D::vtkInternal::UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
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
bool vtkMRMLTransformsDisplayableManager3D::vtkInternal::IsVisible(vtkMRMLTransformDisplayNode* displayNode)
{
  return displayNode && (displayNode->GetVisibility() != 0) && (displayNode->GetVisibility3D() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::AddTransformNode(vtkMRMLTransformNode* node)
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
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::RemoveTransformNode(vtkMRMLTransformNode* node)
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
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::UpdateDisplayableTransforms(vtkMRMLTransformNode* mNode)
{
  // Update the pipeline for all tracked DisplayableNode

  PipelinesCacheType::iterator pipelinesIter;
  std::set< vtkMRMLTransformDisplayNode* > displayNodes = this->TransformToDisplayNodes[mNode];
  std::set< vtkMRMLTransformDisplayNode* >::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::RemoveDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
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
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::AddDisplayNode(vtkMRMLTransformNode* mNode, vtkMRMLTransformDisplayNode* displayNode)
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

  // Create pipeline
  Pipeline* pipeline = new Pipeline();

  pipeline->Actor = vtkSmartPointer<vtkActor>::New();
  vtkNew<vtkPolyDataMapper> mapper;
  pipeline->Actor->SetMapper(mapper.GetPointer());
  pipeline->Actor->SetVisibility(false);
  pipeline->Actor->SetPickable(false);
  pipeline->InputPolyData = vtkSmartPointer<vtkPolyData>::New();
  mapper->SetInputData(pipeline->InputPolyData);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::UpdateDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
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
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::UpdateDisplayNodePipeline(vtkMRMLTransformDisplayNode* displayNode, const Pipeline* pipeline)
{
  // Sets visibility, set pipeline polydata input, update color
  //   calculate and set pipeline transforms.

  if (!displayNode || !pipeline)
    {
    return;
    }

  vtkMRMLTransformNode* transformNode=vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (transformNode==nullptr)
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  vtkMRMLNode* regionNode = displayNode->GetRegionNode();
  if (regionNode==nullptr)
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  // Update visibility
  bool visible = this->IsVisible(displayNode);
  pipeline->Actor->SetVisibility(visible);
  if (!visible)
    {
    return;
    }

  vtkMRMLMarkupsNode* glyphPointsNode = vtkMRMLMarkupsNode::SafeDownCast(displayNode->GetGlyphPointsNode());
  if (!vtkSlicerTransformLogic::GetVisualization3d(pipeline->InputPolyData, displayNode, regionNode, glyphPointsNode))
    {
    vtkWarningWithObjectMacro(displayNode, "Failed to show transform in 3D: unsupported ROI type");
    pipeline->Actor->SetVisibility(false);
    return;
    }

  if (pipeline->InputPolyData->GetNumberOfPoints()==0)
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  // Update pipeline actor
  this->SetTransformDisplayProperty(displayNode, pipeline->Actor);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::AddObservations(vtkMRMLTransformNode* node)
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
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::RemoveObservations(vtkMRMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformsDisplayableManager3D::vtkInternal::IsNodeObserved(vtkMRMLTransformNode* node)
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
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::ClearDisplayableNodes()
{
  while(this->TransformToDisplayNodes.size() > 0)
    {
    this->RemoveTransformNode(this->TransformToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformsDisplayableManager3D::vtkInternal::UseDisplayableNode(vtkMRMLTransformNode* node)
{
  bool use = node && node->IsA("vtkMRMLTransformNode");
  return use;
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::vtkInternal::SetTransformDisplayProperty(vtkMRMLTransformDisplayNode *displayNode, vtkActor* actor)
{
  bool visible = this->IsVisible(displayNode);
  actor->SetVisibility(visible);

  vtkMapper* mapper=actor->GetMapper();

  // if the scalars are visible, set active scalars
  bool scalarVisibility = false;
  if (displayNode->GetScalarVisibility())
    {
    vtkColorTransferFunction* colorTransferFunction=displayNode->GetColorMap();
    if (colorTransferFunction != nullptr && colorTransferFunction->GetSize()>0)
      {
      // Copy the transfer function to not share them between multiple mappers
      vtkNew<vtkColorTransferFunction> colorTransferFunctionCopy;
      colorTransferFunctionCopy->DeepCopy(colorTransferFunction);
      mapper->SetLookupTable(colorTransferFunctionCopy.GetPointer());
      mapper->SetScalarModeToUsePointData();
      mapper->SetColorModeToMapScalars();
      mapper->SelectColorArray(vtkSlicerTransformLogic::GetVisualizationDisplacementMagnitudeScalarName());
      mapper->UseLookupTableScalarRangeOff();
      mapper->SetScalarRange(displayNode->GetScalarRange());
      scalarVisibility = true;
      }
    }
  mapper->SetScalarVisibility(scalarVisibility);

  actor->GetProperty()->SetRepresentation(displayNode->GetRepresentation());
  actor->GetProperty()->SetPointSize(displayNode->GetPointSize());
  actor->GetProperty()->SetLineWidth(displayNode->GetLineWidth());
  actor->GetProperty()->SetLighting(displayNode->GetLighting());
  actor->GetProperty()->SetInterpolation(displayNode->GetInterpolation());
  actor->GetProperty()->SetShading(displayNode->GetShading());
  actor->GetProperty()->SetFrontfaceCulling(displayNode->GetFrontfaceCulling());
  actor->GetProperty()->SetBackfaceCulling(displayNode->GetBackfaceCulling());

  if (displayNode->GetSelected())
    {
    actor->GetProperty()->SetColor(displayNode->GetSelectedColor());
    actor->GetProperty()->SetAmbient(displayNode->GetSelectedAmbient());
    actor->GetProperty()->SetSpecular(displayNode->GetSelectedSpecular());
    }
  else
    {
    actor->GetProperty()->SetColor(displayNode->GetColor());
    actor->GetProperty()->SetAmbient(displayNode->GetAmbient());
    actor->GetProperty()->SetSpecular(displayNode->GetSpecular());
    }
  actor->GetProperty()->SetOpacity(displayNode->GetOpacity());
  actor->GetProperty()->SetDiffuse(displayNode->GetDiffuse());
  actor->GetProperty()->SetSpecularPower(displayNode->GetPower());
  actor->GetProperty()->SetMetallic(displayNode->GetMetallic());
  actor->GetProperty()->SetRoughness(displayNode->GetRoughness());
  actor->GetProperty()->SetEdgeVisibility(displayNode->GetEdgeVisibility());
  actor->GetProperty()->SetEdgeColor(displayNode->GetEdgeColor());

  actor->SetTexture(nullptr);
}

//---------------------------------------------------------------------------
// vtkMRMLTransformsDisplayableManager3D methods

//---------------------------------------------------------------------------
vtkMRMLTransformsDisplayableManager3D::vtkMRMLTransformsDisplayableManager3D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLTransformsDisplayableManager3D::~vtkMRMLTransformsDisplayableManager3D()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkMRMLTransformsDisplayableManager3D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
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
void vtkMRMLTransformsDisplayableManager3D::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
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
void vtkMRMLTransformsDisplayableManager3D::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
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
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLTransformsDisplayableManager3D->UpdateFromMRML: Scene is not set.");
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
void vtkMRMLTransformsDisplayableManager3D::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager3D::Create()
{
  Superclass::Create();
  this->SetUpdateFromMRMLRequested(true);
}
