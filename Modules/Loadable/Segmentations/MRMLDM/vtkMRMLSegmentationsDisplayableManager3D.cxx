/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLSegmentationsDisplayableManager3D.h"

// Segmentations includes
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkDataSetAttributes.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGeneralTransform.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkMRMLSegmentationsDisplayableManager3D );

//---------------------------------------------------------------------------
class vtkMRMLSegmentationsDisplayableManager3D::vtkInternal
{
public:

  vtkInternal(vtkMRMLSegmentationsDisplayableManager3D* external);
  ~vtkInternal();

  struct Pipeline
    {

    Pipeline()
      {
      this->Actor = vtkSmartPointer<vtkActor>::New();
      vtkNew<vtkPolyDataMapper> mapper;
      this->Actor->SetMapper(mapper.GetPointer());
      this->Actor->SetVisibility(false);
      this->InputPolyData = vtkSmartPointer<vtkPolyData>::New();
      this->ModelWarper = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->NodeToWorld = vtkSmartPointer<vtkGeneralTransform>::New();
      this->ModelWarper->SetInputData(this->InputPolyData);
      mapper->SetInputConnection(this->ModelWarper->GetOutputPort());
      }

    vtkSmartPointer<vtkActor> Actor;
    vtkSmartPointer<vtkPolyData> InputPolyData;
    vtkSmartPointer<vtkTransformPolyDataFilter> ModelWarper;
    vtkSmartPointer<vtkGeneralTransform> NodeToWorld;
    };

  typedef std::map<std::string, const Pipeline*> PipelineMapType; // first: segment ID; second: display pipeline
  typedef std::map < vtkMRMLSegmentationDisplayNode*, PipelineMapType > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLSegmentationNode*, std::set< vtkMRMLSegmentationDisplayNode* > > SegmentationToDisplayCacheType;
  SegmentationToDisplayCacheType SegmentationToDisplayNodes;

  // Segmentations
  void AddSegmentationNode(vtkMRMLSegmentationNode* displayableNode);
  void RemoveSegmentationNode(vtkMRMLSegmentationNode* displayableNode);

  // Transforms
  void UpdateDisplayableTransforms(vtkMRMLSegmentationNode *node);
  void GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld);

  // Display Nodes
  void AddDisplayNode(vtkMRMLSegmentationNode*, vtkMRMLSegmentationDisplayNode*);
  Pipeline* CreateSegmentPipeline(std::string segmentID);
  void UpdateDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode);
  void UpdateAllDisplayNodesForSegment(vtkMRMLSegmentationNode* segmentationNode);
  void UpdateSegmentPipelines(vtkMRMLSegmentationDisplayNode*, PipelineMapType&);
  void UpdateDisplayNodePipeline(vtkMRMLSegmentationDisplayNode*, PipelineMapType);
  void RemoveDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLSegmentationNode* node);
  void RemoveObservations(vtkMRMLSegmentationNode* node);
  bool IsNodeObserved(vtkMRMLSegmentationNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLSegmentationDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLSegmentationNode* node);
  void ClearDisplayableNodes();

private:
  vtkMRMLSegmentationsDisplayableManager3D* External;
  bool AddingSegmentationNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::vtkInternal(vtkMRMLSegmentationsDisplayableManager3D * external)
: External(external)
, AddingSegmentationNode(false)
{
}

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UseDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode)
{
  // Allow segmentations to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->External->GetMRMLViewNode()->GetID()))
    {
    return false;
    }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkMRMLSegmentationDisplayNode");

  return use;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::IsVisible(vtkMRMLSegmentationDisplayNode* displayNode)
{
  return displayNode && displayNode->GetVisibility() && displayNode->GetVisibility3D();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::AddSegmentationNode(vtkMRMLSegmentationNode* node)
{
  if (this->AddingSegmentationNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingSegmentationNode = true;

  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();

  this->AddObservations(node);

  for (int i=0; i<nnodes; i++)
    {
    vtkMRMLSegmentationDisplayNode *dnode = vtkMRMLSegmentationDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));
    if ( this->UseDisplayNode(dnode) )
      {
      this->SegmentationToDisplayNodes[node].insert(dnode);
      this->AddDisplayNode( node, dnode );
      }
    }
  this->AddingSegmentationNode = false;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::RemoveSegmentationNode(vtkMRMLSegmentationNode* node)
{
  if (!node)
    {
    return;
    }
  vtkInternal::SegmentationToDisplayCacheType::iterator displayableIt =
    this->SegmentationToDisplayNodes.find(node);
  if(displayableIt == this->SegmentationToDisplayNodes.end())
    {
    return;
    }

  std::set<vtkMRMLSegmentationDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkMRMLSegmentationDisplayNode *>::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->RemoveDisplayNode(*diter);
    }
  this->RemoveObservations(node);
  this->SegmentationToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld)
{
  if (!node || !transformToWorld)
    {
    return;
    }

  vtkMRMLTransformNode* tnode = node->GetParentTransformNode();

  transformToWorld->Identity();
  if (tnode)
    {
    tnode->GetTransformToWorld(transformToWorld);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateDisplayableTransforms(vtkMRMLSegmentationNode* mNode)
{
  // Update the pipeline for all tracked DisplayableNode
  PipelinesCacheType::iterator pipelinesIter;
  std::set< vtkMRMLSegmentationDisplayNode* > displayNodes = this->SegmentationToDisplayNodes[mNode];
  std::set< vtkMRMLSegmentationDisplayNode* >::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
      for (PipelineMapType::iterator pipelineIt=pipelinesIter->second.begin(); pipelineIt!=pipelinesIter->second.end(); ++pipelineIt)
        {
        const Pipeline* currentPipeline = pipelineIt->second;
        this->GetNodeTransformToWorld(mNode, currentPipeline->NodeToWorld);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::RemoveDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode)
{
  PipelinesCacheType::iterator pipelinesIter = this->DisplayPipelines.find(displayNode);
  if (pipelinesIter == this->DisplayPipelines.end())
    {
    return;
    }
  PipelineMapType::iterator pipelineIt;
  for (pipelineIt = pipelinesIter->second.begin(); pipelineIt != pipelinesIter->second.end(); ++pipelineIt)
    {
    const Pipeline* pipeline = pipelineIt->second;
    this->External->GetRenderer()->RemoveActor(pipeline->Actor);
    delete pipeline;
    }
  this->DisplayPipelines.erase(pipelinesIter);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::AddDisplayNode(vtkMRMLSegmentationNode* mNode, vtkMRMLSegmentationDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a segmentation node already associated with a display node
  // is copied into an other (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    return;
    }

  // Create pipelines for each segment
  vtkSegmentation* segmentation = mNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }
  PipelineMapType pipelineVector;
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    pipelineVector[*segmentIdIt] = this->CreateSegmentPipeline(*segmentIdIt);
    }

  this->DisplayPipelines.insert( std::make_pair(displayNode, pipelineVector) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::Pipeline*
vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::CreateSegmentPipeline(
    std::string vtkNotUsed(segmentID))
{
  Pipeline* pipeline = new Pipeline();

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );

  return pipeline;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update. Otherwise, add as new node
  if (!displayNode)
    {
    return;
    }
  PipelinesCacheType::iterator displayNodeIt;
  displayNodeIt = this->DisplayPipelines.find(displayNode);
  if (displayNodeIt != this->DisplayPipelines.end())
    {
    this->UpdateSegmentPipelines(displayNode, displayNodeIt->second);
    this->UpdateDisplayNodePipeline(displayNode, displayNodeIt->second);
    }
  else
    {
    this->AddSegmentationNode( vtkMRMLSegmentationNode::SafeDownCast(displayNode->GetDisplayableNode()) );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateAllDisplayNodesForSegment(vtkMRMLSegmentationNode* segmentationNode)
{
  std::set<vtkMRMLSegmentationDisplayNode *> displayNodes = this->SegmentationToDisplayNodes[segmentationNode];
  for (std::set<vtkMRMLSegmentationDisplayNode *>::iterator dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++)
    {
    this->UpdateDisplayNode(*dnodesIter);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateSegmentPipelines(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType &pipelines)
{
  // Get segmentation
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (!segmentationNode)
    {
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }

  // Make sure each segment has a pipeline
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    // If segment does not have a pipeline, create one
    PipelineMapType::iterator pipelineIt = pipelines.find(*segmentIdIt);
    if (pipelineIt == pipelines.end())
      {
      pipelines[*segmentIdIt] = this->CreateSegmentPipeline(*segmentIdIt);
      this->GetNodeTransformToWorld(segmentationNode, pipelines[*segmentIdIt]->NodeToWorld);
      }
    }

  // Make sure each pipeline belongs to an existing segment
  PipelineMapType::iterator pipelineIt = pipelines.begin();
  while (pipelineIt != pipelines.end())
    {
    const Pipeline* pipeline = pipelineIt->second;
    vtkSegment* segment = segmentation->GetSegment(pipelineIt->first);
    if (segment == NULL)
      {
      PipelineMapType::iterator erasedIt = pipelineIt;
      ++pipelineIt;
      pipelines.erase(erasedIt);
      this->External->GetRenderer()->RemoveActor(pipeline->Actor);
      delete pipeline;
      }
    else
      {
      ++pipelineIt;
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateDisplayNodePipeline(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType pipelines)
{
  // Sets visibility, set pipeline polydata input, update color
  if (!displayNode)
    {
    return;
    }
  bool displayNodeVisible = displayNode->GetVisibility3D() && displayNode->GetOpacity3D()>0
    && displayNode->GetVisibility(this->External->GetMRMLViewNode()->GetID());

  // Get segmentation display node
  vtkMRMLSegmentationDisplayNode* segmentationDisplayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(displayNode);

  // Determine which representation to show
  std::string shownRepresentationName = segmentationDisplayNode->GetDisplayRepresentationName3D();
  if (shownRepresentationName.empty())
    {
    // Hide segmentation if there is no poly data representation to show
    for (PipelineMapType::iterator pipelineIt=pipelines.begin(); pipelineIt!=pipelines.end(); ++pipelineIt)
      {
      pipelineIt->second->Actor->SetVisibility(false);
      }
    return;
    }

  // Get segmentation
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    segmentationDisplayNode->GetDisplayableNode() );
  if (!segmentationNode)
    {
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }
  // Make sure the requested representation exists
  if (!segmentation->CreateRepresentation(shownRepresentationName))
    {
    return;
    }

  // For all pipelines (pipeline per segment)
  for (PipelineMapType::iterator pipelineIt=pipelines.begin(); pipelineIt!=pipelines.end(); ++pipelineIt)
    {
    const Pipeline* pipeline = pipelineIt->second;

    // Update visibility
    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    displayNode->GetSegmentDisplayProperties(pipelineIt->first, properties);
    bool segmentVisible = displayNodeVisible && properties.Visible && properties.Visible3D;
    pipeline->Actor->SetVisibility(segmentVisible);
    if (!segmentVisible)
      {
      continue;
      }

    // Get poly data to display
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(
      segmentation->GetSegmentRepresentation(pipelineIt->first, shownRepresentationName));
    if (!polyData || polyData->GetNumberOfPoints() == 0)
      {
      pipeline->Actor->SetVisibility(false);
      continue;
      }

    pipeline->ModelWarper->SetTransform(pipeline->NodeToWorld);
    pipeline->InputPolyData->ShallowCopy(polyData);

    // Get displayed color (if no override is defined then use the color from the segment)
    double color[3] = {vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2]};
    displayNode->GetSegmentColor(pipelineIt->first, color);

    // Update pipeline actor
    pipeline->Actor->GetProperty()->SetRepresentation(displayNode->GetRepresentation());
    pipeline->Actor->GetProperty()->SetPointSize(displayNode->GetPointSize());
    pipeline->Actor->GetProperty()->SetLineWidth(displayNode->GetLineWidth());
    pipeline->Actor->GetProperty()->SetLighting(displayNode->GetLighting());
    pipeline->Actor->GetProperty()->SetInterpolation(displayNode->GetInterpolation());
    pipeline->Actor->GetProperty()->SetShading(displayNode->GetShading());
    pipeline->Actor->GetProperty()->SetFrontfaceCulling(displayNode->GetFrontfaceCulling());
    pipeline->Actor->GetProperty()->SetBackfaceCulling(displayNode->GetBackfaceCulling());

    pipeline->Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
    pipeline->Actor->GetProperty()->SetOpacity(properties.Opacity3D * displayNode->GetOpacity3D());

    if (displayNode->GetSelected())
      {
      pipeline->Actor->GetProperty()->SetAmbient(displayNode->GetSelectedAmbient());
      pipeline->Actor->GetProperty()->SetSpecular(displayNode->GetSelectedSpecular());
      }
    else
      {
      pipeline->Actor->GetProperty()->SetAmbient(displayNode->GetAmbient());
      pipeline->Actor->GetProperty()->SetSpecular(displayNode->GetSpecular());
      }
    pipeline->Actor->GetProperty()->SetDiffuse(displayNode->GetDiffuse());
    pipeline->Actor->GetProperty()->SetSpecularPower(displayNode->GetPower());
    pipeline->Actor->GetProperty()->SetEdgeVisibility(displayNode->GetEdgeVisibility());
    pipeline->Actor->GetProperty()->SetEdgeColor(displayNode->GetEdgeColor());

    pipeline->Actor->SetTexture(0);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::AddObservations(vtkMRMLSegmentationNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
    }
  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::SegmentModified, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkSegmentation::SegmentModified, this->External, this->External->GetMRMLNodesCallbackCommand());
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::RemoveObservations(vtkMRMLSegmentationNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::SegmentModified, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::IsNodeObserved(vtkMRMLSegmentationNode* node)
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
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::ClearDisplayableNodes()
{
  while(this->SegmentationToDisplayNodes.size() > 0)
    {
    this->RemoveSegmentationNode(this->SegmentationToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UseDisplayableNode(vtkMRMLSegmentationNode* node)
{
  bool use = node && node->IsA("vtkMRMLSegmentationNode");
  return use;
}

//---------------------------------------------------------------------------
// vtkMRMLSegmentationsDisplayableManager3D methods

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager3D::vtkMRMLSegmentationsDisplayableManager3D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager3D::~vtkMRMLSegmentationsDisplayableManager3D()
{
  delete this->Internal;
  this->Internal=NULL;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkMRMLSegmentationsDisplayableManager3D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( !node->IsA("vtkMRMLSegmentationNode") )
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromMRMLRequested(1);
    return;
    }

  this->Internal->AddSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node
    && (!node->IsA("vtkMRMLSegmentationNode"))
    && (!node->IsA("vtkMRMLSegmentationDisplayNode")) )
    {
    return;
    }

  vtkMRMLSegmentationNode* segmentationNode = NULL;
  vtkMRMLSegmentationDisplayNode* displayNode = NULL;

  bool modified = false;
  if ( (segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveSegmentationNode(segmentationNode);
    modified = true;
    }
  else if ( (displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(node)) )
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
void vtkMRMLSegmentationsDisplayableManager3D::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if ( scene->IsBatchProcessing() )
    {
    return;
    }

  vtkMRMLSegmentationNode* displayableNode = vtkMRMLSegmentationNode::SafeDownCast(caller);
  if (displayableNode)
    {
    if (event == vtkMRMLDisplayableNode::DisplayModifiedEvent)
      {
      vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode *> (callData);
      vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(callDataNode);
      if (displayNode)
        {
        this->Internal->UpdateDisplayNode(displayNode);
        this->RequestRender();
        }
      }
    else if ( (event == vtkMRMLDisplayableNode::TransformModifiedEvent)
           || (event == vtkMRMLTransformableNode::TransformModifiedEvent)
           || (event == vtkSegmentation::RepresentationModified)
           || (event == vtkSegmentation::SegmentModified) )
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
      }
    else if ( (event == vtkCommand::ModifiedEvent) // segmentation object may be replaced
           || (event == vtkSegmentation::SegmentAdded)
           || (event == vtkSegmentation::SegmentRemoved) )
      {
      this->Internal->UpdateAllDisplayNodesForSegment(displayableNode);
      this->RequestRender();
      }
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(0);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLSegmentationsDisplayableManager3D->UpdateFromMRML: Scene is not set.")
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLSegmentationNode* mNode = NULL;
  std::vector<vtkMRMLNode *> mNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkMRMLSegmentationNode", mNodes) : 0;
  for (int i=0; i<nnodes; i++)
    {
    mNode  = vtkMRMLSegmentationNode::SafeDownCast(mNodes[i]);
    if (mNode && this->Internal->UseDisplayableNode(mNode))
      {
      this->Internal->AddSegmentationNode(mNode);
      }
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(1);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(1);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::Create()
{
  Superclass::Create();
  this->SetUpdateFromMRMLRequested(1);
}
