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
#include "vtkMRMLTransformsDisplayableManager2D.h"

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

// STD includes
#include <algorithm>
#include <cassert>
#include <set>
#include <map>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLTransformsDisplayableManager2D );

//---------------------------------------------------------------------------
class vtkMRMLTransformsDisplayableManager2D::vtkInternal
{
public:

  vtkInternal( vtkMRMLTransformsDisplayableManager2D* external );
  ~vtkInternal();
  struct Pipeline
    {
    vtkSmartPointer<vtkProp> Actor;
    vtkSmartPointer<vtkTransform> TransformToSlice;
    vtkSmartPointer<vtkTransformPolyDataFilter> Transformer;
    };

  typedef std::map < vtkMRMLTransformDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLTransformNode*, std::set< vtkMRMLTransformDisplayNode* > > TransformToDisplayCacheType;
  TransformToDisplayCacheType TransformToDisplayNodes;

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
  void UpdateDisplayNodePipeline(vtkMRMLTransformDisplayNode*, const Pipeline*);
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

private:
  vtkMRMLTransformsDisplayableManager2D* External;
  bool AddingTransformNode;
  vtkSmartPointer<vtkMRMLSliceNode> SliceNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLTransformsDisplayableManager2D::vtkInternal::vtkInternal(vtkMRMLTransformsDisplayableManager2D* external)
: External(external)
, AddingTransformNode(false)
{
}

//---------------------------------------------------------------------------
vtkMRMLTransformsDisplayableManager2D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
  this->SliceNode = nullptr;
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformsDisplayableManager2D::vtkInternal::UseDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
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
bool vtkMRMLTransformsDisplayableManager2D::vtkInternal::IsVisible(vtkMRMLTransformDisplayNode* displayNode)
{
  return displayNode && (displayNode->GetVisibility() != 0) && (displayNode->GetVisibility2D() != 0);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
    {
    return;
    }
  this->SliceNode=sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::UpdateSliceNode()
{
  // Update the Slice node transform

  PipelinesCacheType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end(); ++it)
    {
    this->UpdateDisplayNodePipeline(it->first, it->second);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::AddTransformNode(vtkMRMLTransformNode* node)
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
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::RemoveTransformNode(vtkMRMLTransformNode* node)
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
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::UpdateDisplayableTransforms(vtkMRMLTransformNode* mNode)
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
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::RemoveDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
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
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::AddDisplayNode(vtkMRMLTransformNode* mNode, vtkMRMLTransformDisplayNode* displayNode)
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

  vtkNew<vtkActor2D> actor;
  if (displayNode->IsA("vtkMRMLTransformDisplayNode"))
    {
    actor->SetMapper( vtkNew<vtkPolyDataMapper2D>().GetPointer() );
    }

  // Create pipeline
  Pipeline* pipeline = new Pipeline();
  pipeline->Actor = actor.GetPointer();
  pipeline->TransformToSlice = vtkSmartPointer<vtkTransform>::New();
  pipeline->Transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  // Set up pipeline
  pipeline->Transformer->SetTransform(pipeline->TransformToSlice);
  pipeline->Actor->SetVisibility(0);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::UpdateDisplayNode(vtkMRMLTransformDisplayNode* displayNode)
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
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::UpdateDisplayNodePipeline(vtkMRMLTransformDisplayNode* displayNode, const Pipeline* pipeline)
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
  if (!visible)
    {
    return;
    }

  vtkMRMLTransformDisplayNode* transformDisplayNode = vtkMRMLTransformDisplayNode::SafeDownCast(displayNode);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  vtkMRMLMarkupsNode* glyphPointsNode = vtkMRMLMarkupsNode::SafeDownCast(displayNode->GetGlyphPointsNode());
  vtkSlicerTransformLogic::GetVisualization2d(polyData, transformDisplayNode, this->SliceNode, glyphPointsNode);

  pipeline->Transformer->SetInputData(polyData);

  if (polyData->GetNumberOfPoints()==0)
    {
    return;
    }

  // Set PolyData Transform
  vtkNew<vtkMatrix4x4> rasToXY;
  vtkMatrix4x4::Invert(this->SliceNode->GetXYToRAS(), rasToXY.GetPointer());
  pipeline->TransformToSlice->SetMatrix(rasToXY.GetPointer());

  // Update pipeline actor
  vtkActor2D* actor = vtkActor2D::SafeDownCast(pipeline->Actor);
  vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper());
  mapper->SetInputConnection( pipeline->Transformer->GetOutputPort() );

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
      mapper->ColorByArrayComponent(const_cast<char*>(vtkSlicerTransformLogic::GetVisualizationDisplacementMagnitudeScalarName()),0);
      mapper->UseLookupTableScalarRangeOff();
      mapper->SetScalarRange(displayNode->GetScalarRange());
      scalarVisibility = true;
      }
    }
  mapper->SetScalarVisibility(scalarVisibility);

  actor->SetPosition(0,0);
  vtkProperty2D* actorProperties = actor->GetProperty();
  actorProperties->SetColor(displayNode->GetColor() );
  actorProperties->SetLineWidth(displayNode->GetSliceIntersectionThickness() );
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::AddObservations(vtkMRMLTransformNode* node)
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
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::RemoveObservations(vtkMRMLTransformNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformsDisplayableManager2D::vtkInternal::IsNodeObserved(vtkMRMLTransformNode* node)
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
void vtkMRMLTransformsDisplayableManager2D::vtkInternal::ClearDisplayableNodes()
{
  while(this->TransformToDisplayNodes.size() > 0)
    {
    this->RemoveTransformNode(this->TransformToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformsDisplayableManager2D::vtkInternal::UseDisplayableNode(vtkMRMLTransformNode* node)
{
  bool use = node && node->IsA("vtkMRMLTransformNode");
  return use;
}

//---------------------------------------------------------------------------
// vtkMRMLTransformsDisplayableManager2D methods

//---------------------------------------------------------------------------
vtkMRMLTransformsDisplayableManager2D::vtkMRMLTransformsDisplayableManager2D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLTransformsDisplayableManager2D::~vtkMRMLTransformsDisplayableManager2D()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkMRMLTransformsDisplayableManager2D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
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
void vtkMRMLTransformsDisplayableManager2D::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
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
void vtkMRMLTransformsDisplayableManager2D::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
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
void vtkMRMLTransformsDisplayableManager2D::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLTransformsDisplayableManager2D->UpdateFromMRML: Scene is not set.");
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
void vtkMRMLTransformsDisplayableManager2D::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformsDisplayableManager2D::Create()
{
  this->Internal->SetSliceNode(this->GetMRMLSliceNode());
  this->SetUpdateFromMRMLRequested(true);
}
