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
#include <vtkMRMLFolderDisplayNode.h>
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
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGeneralTransform.h>
#include <vtkCellPicker.h>

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
      mapper->SetScalarVisibility(false); // ignore any scalars that an input mesh may contain
      this->Actor->SetMapper(mapper.GetPointer());
      this->Actor->SetVisibility(false);

      this->NodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      this->ModelWarper = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->ModelWarper->SetTransform(this->NodeToWorldTransform);
      mapper->SetInputConnection(this->ModelWarper->GetOutputPort());
      }

    vtkSmartPointer<vtkActor> Actor;
    vtkSmartPointer<vtkGeneralTransform> NodeToWorldTransform;
    vtkSmartPointer<vtkTransformPolyDataFilter> ModelWarper;
    };

  typedef std::map<std::string, const Pipeline*> PipelineMapType; // first: segment ID; second: display pipeline
  typedef std::map<vtkMRMLSegmentationDisplayNode*, PipelineMapType> PipelinesCacheType;
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
  void UpdateSegmentPipelines(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType &segmentPipelines);
  void UpdateDisplayNodePipeline(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType &segmentPipelines);
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

  /// Find picked node from mesh and set PickedNodeID in Internal
  void FindPickedDisplayNodeFromMesh(vtkPointSet* mesh);
  /// Find first picked node from prop3Ds in cell picker and set PickedNodeID in Internal
  void FindFirstPickedDisplayNodeFromPickerProp3Ds();

public:
  /// Picker of segment prop in renderer
  vtkSmartPointer<vtkCellPicker> CellPicker;

  /// Last picked segmentation display node ID
  std::string PickedDisplayNodeID;

  /// Last picked segment ID
  std::string PickedSegmentID;

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
  this->CellPicker = vtkSmartPointer<vtkCellPicker>::New();
  this->CellPicker->SetTolerance(0.00001);
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
  return displayNode && displayNode->GetVisibility() && displayNode->GetVisibility3D()
    && displayNode->GetVisibility(this->External->GetMRMLViewNode()->GetID())
    && displayNode->GetOpacity3D() > 0;
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
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::GetNodeTransformToWorld(
  vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld)
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
        vtkNew<vtkGeneralTransform> nodeToWorld;
        this->GetNodeTransformToWorld(mNode, nodeToWorld.GetPointer());
        // It is important to only update the transform if the transform chain is actually changed,
        // because recomputing a non-linear transformation on a complex model may be very time-consuming.
        if (!vtkMRMLTransformNode::AreTransformsEqual(nodeToWorld.GetPointer(), currentPipeline->NodeToWorldTransform))
          {
          currentPipeline->NodeToWorldTransform->DeepCopy(nodeToWorld);
          }
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
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateSegmentPipelines(
  vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType &segmentPipelines)
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
    PipelineMapType::iterator pipelineIt = segmentPipelines.find(*segmentIdIt);
    if (pipelineIt == segmentPipelines.end())
      {
      segmentPipelines[*segmentIdIt] = this->CreateSegmentPipeline(*segmentIdIt);
      vtkNew<vtkGeneralTransform> nodeToWorld;
      this->GetNodeTransformToWorld(segmentationNode, nodeToWorld.GetPointer());
      // It is important to only update the transform if the transform chain is actually changed,
      // because recomputing a non-linear transformation on a complex model may be very time-consuming.
      if (!vtkMRMLTransformNode::AreTransformsEqual(nodeToWorld.GetPointer(), segmentPipelines[*segmentIdIt]->NodeToWorldTransform))
        {
        segmentPipelines[*segmentIdIt]->NodeToWorldTransform->DeepCopy(nodeToWorld);
        }
      }
    }

  // Make sure each pipeline belongs to an existing segment
  PipelineMapType::iterator pipelineIt = segmentPipelines.begin();
  while (pipelineIt != segmentPipelines.end())
    {
    const Pipeline* pipeline = pipelineIt->second;
    vtkSegment* segment = segmentation->GetSegment(pipelineIt->first);
    if (segment == nullptr)
      {
      PipelineMapType::iterator erasedIt = pipelineIt;
      ++pipelineIt;
      segmentPipelines.erase(erasedIt);
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
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateDisplayNodePipeline(
  vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType &segmentPipelines)
{
  // Sets visibility, set pipeline polydata input, update color
  if (!displayNode)
    {
    return;
    }

  // Get display node from hierarchy that applies display properties on branch
  vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  vtkMRMLDisplayNode* overrideHierarchyDisplayNode =
    vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);
  vtkMRMLDisplayNode* genericDisplayNode = displayNode;

  // Use hierarchy display node if any, and if overriding is allowed for the current display node.
  // If override is explicitly disabled, then do not apply hierarchy visibility or opacity either.
  bool hierarchyVisibility = true;
  double hierarchyOpacity = 1.0;
  if (displayNode->GetFolderDisplayOverrideAllowed())
    {
    if (overrideHierarchyDisplayNode)
      {
      genericDisplayNode = overrideHierarchyDisplayNode;
      }

    // Get visibility and opacity defined by the hierarchy.
    // These two properties are influenced by the hierarchy regardless the fact whether there is override
    // or not. Visibility of items defined by hierarchy is off if any of the ancestors is explicitly hidden,
    // and the opacity is the product of the ancestors' opacities.
    // However, this does not apply on display nodes that do not allow overrides (FolderDisplayOverrideAllowed)
    hierarchyVisibility = vtkMRMLFolderDisplayNode::GetHierarchyVisibility(displayableNode);
    hierarchyOpacity = vtkMRMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
    }

  bool displayNodeVisible = this->IsVisible(displayNode);

  // Determine which representation to show
  std::string shownRepresentationName = displayNode->GetDisplayRepresentationName3D();
  if (shownRepresentationName.empty())
    {
    // Hide segmentation if there is no poly data representation to show
    for (PipelineMapType::iterator pipelineIt=segmentPipelines.begin(); pipelineIt!=segmentPipelines.end(); ++pipelineIt)
      {
      pipelineIt->second->Actor->SetVisibility(false);
      }
    return;
    }

  // Get segmentation
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(displayableNode);
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
  for (PipelineMapType::iterator pipelineIt=segmentPipelines.begin(); pipelineIt!=segmentPipelines.end(); ++pipelineIt)
    {
    const Pipeline* pipeline = pipelineIt->second;

    // Update visibility
    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    displayNode->GetSegmentDisplayProperties(pipelineIt->first, properties);
    bool segmentVisible = hierarchyVisibility && displayNodeVisible && properties.Visible && properties.Visible3D;
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

    // Set node to world transform to identity by default
    // For all pipelines (pipeline per segment)
    vtkNew<vtkGeneralTransform> nodeToWorld;
    this->GetNodeTransformToWorld(segmentationNode, nodeToWorld.GetPointer());
    if (!vtkMRMLTransformNode::AreTransformsEqual(nodeToWorld.GetPointer(), pipeline->NodeToWorldTransform))
      {
      pipeline->NodeToWorldTransform->DeepCopy(nodeToWorld);
      }
    if (pipeline->ModelWarper->GetInputDataObject(0, 0) != polyData)
      {
      pipeline->ModelWarper->SetInputData(polyData);
      }

    // Get displayed color (if no override is defined then use the color from the segment)
    double color[3] = {vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2]};
    if (overrideHierarchyDisplayNode)
      {
      overrideHierarchyDisplayNode->GetColor(color);
      }
    else
      {
      displayNode->GetSegmentColor(pipelineIt->first, color);
      }

    // Update pipeline actor
    pipeline->Actor->GetProperty()->SetRepresentation(genericDisplayNode->GetRepresentation());
    pipeline->Actor->GetProperty()->SetPointSize(genericDisplayNode->GetPointSize());
    pipeline->Actor->GetProperty()->SetLineWidth(genericDisplayNode->GetLineWidth());
    pipeline->Actor->GetProperty()->SetLighting(genericDisplayNode->GetLighting());
    pipeline->Actor->GetProperty()->SetInterpolation(genericDisplayNode->GetInterpolation());
    pipeline->Actor->GetProperty()->SetShading(genericDisplayNode->GetShading());
    pipeline->Actor->GetProperty()->SetFrontfaceCulling(genericDisplayNode->GetFrontfaceCulling());
    pipeline->Actor->GetProperty()->SetBackfaceCulling(genericDisplayNode->GetBackfaceCulling());

    pipeline->Actor->GetProperty()->SetColor(color[0], color[1], color[2]);
    pipeline->Actor->GetProperty()->SetOpacity(
      hierarchyOpacity * properties.Opacity3D * displayNode->GetOpacity3D() * genericDisplayNode->GetOpacity());

    pipeline->Actor->SetPickable(segmentationNode->GetSelectable());
    if (genericDisplayNode->GetSelected())
      {
      pipeline->Actor->GetProperty()->SetAmbient(genericDisplayNode->GetSelectedAmbient());
      pipeline->Actor->GetProperty()->SetSpecular(genericDisplayNode->GetSelectedSpecular());
      }
    else
      {
      pipeline->Actor->GetProperty()->SetAmbient(genericDisplayNode->GetAmbient());
      pipeline->Actor->GetProperty()->SetSpecular(genericDisplayNode->GetSpecular());
      }
    pipeline->Actor->GetProperty()->SetDiffuse(genericDisplayNode->GetDiffuse());
    pipeline->Actor->GetProperty()->SetSpecularPower(genericDisplayNode->GetPower());
    pipeline->Actor->GetProperty()->SetEdgeVisibility(genericDisplayNode->GetEdgeVisibility());
    pipeline->Actor->GetProperty()->SetEdgeColor(genericDisplayNode->GetEdgeColor());

    pipeline->Actor->SetTexture(nullptr);
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
  observations = broker->GetObservations(
    node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkSegmentation::SegmentAdded, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkSegmentation::SegmentModified, this->External, this->External->GetMRMLNodesCallbackCommand());
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
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::FindPickedDisplayNodeFromMesh(vtkPointSet* mesh)
{
  this->PickedDisplayNodeID = "";
  this->PickedSegmentID = "";
  if (!mesh)
    {
    return;
    }

  PipelinesCacheType::iterator pipelinesIt;
  for (pipelinesIt = this->DisplayPipelines.begin(); pipelinesIt!=this->DisplayPipelines.end(); ++pipelinesIt)
    {
    vtkMRMLSegmentationDisplayNode* currentDisplayNode = pipelinesIt->first;
    for (PipelineMapType::iterator pipelineIt=pipelinesIt->second.begin(); pipelineIt!=pipelinesIt->second.end(); ++pipelineIt)
      {
      if (pipelineIt->second->ModelWarper->GetOutput() == mesh)
        {
        this->PickedDisplayNodeID = currentDisplayNode->GetID();
        this->PickedSegmentID = pipelineIt->first;
        return; // Display node and segment found
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::FindFirstPickedDisplayNodeFromPickerProp3Ds()
{
  this->PickedDisplayNodeID = "";
  this->PickedSegmentID = "";
  if (!this->CellPicker)
    {
    return;
    }

  vtkProp3DCollection* props = this->CellPicker->GetProp3Ds();
  for (int propIndex=0; propIndex<props->GetNumberOfItems(); ++propIndex)
    {
    vtkProp3D* pickedProp = vtkProp3D::SafeDownCast(props->GetItemAsObject(propIndex));
    if (!pickedProp)
      {
      continue;
      }

    PipelinesCacheType::iterator pipelinesIt;
    for (pipelinesIt = this->DisplayPipelines.begin(); pipelinesIt!=this->DisplayPipelines.end(); ++pipelinesIt)
      {
      vtkMRMLSegmentationDisplayNode* currentDisplayNode = pipelinesIt->first;
      for (PipelineMapType::iterator pipelineIt=pipelinesIt->second.begin(); pipelineIt!=pipelinesIt->second.end(); ++pipelineIt)
        {
        if (pipelineIt->second->Actor.GetPointer() == pickedProp)
          {
          this->PickedDisplayNodeID = currentDisplayNode->GetID();
          this->PickedSegmentID = pipelineIt->first;
          return; // Display node and segment found
          }
        }
      }
    }
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
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::PrintSelf(ostream& os, vtkIndent indent)
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

  // Escape if the scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromMRMLRequested(true);
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

  vtkMRMLSegmentationNode* segmentationNode = nullptr;
  vtkMRMLSegmentationDisplayNode* displayNode = nullptr;

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
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLSegmentationsDisplayableManager3D::UpdateFromMRML: Scene is not set");
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLSegmentationNode* segmentationNode = nullptr;
  std::vector<vtkMRMLNode*> segmentationNodes;
  int numOfSegmentationNodes = scene ? scene->GetNodesByClass("vtkMRMLSegmentationNode", segmentationNodes) : 0;
  for (int i=0; i<numOfSegmentationNodes; i++)
    {
    segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(segmentationNodes[i]);
    if (segmentationNode && this->Internal->UseDisplayableNode(segmentationNode))
      {
      this->Internal->AddSegmentationNode(segmentationNode);
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
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::Create()
{
  Superclass::Create();
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
int vtkMRMLSegmentationsDisplayableManager3D::Pick3D(double ras[3])
{
  this->Internal->PickedDisplayNodeID = "";
  this->Internal->PickedSegmentID = "";

  vtkRenderer* ren = this->GetRenderer();
  if (!ren)
    {
    vtkErrorMacro("Pick3D: Unable to get renderer");
    return 0;
    }

#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  if (this->Internal->CellPicker->Pick3DPoint(ras, ren))
    {
    // Find first picked segmentation and segment from picker
    // Note: Getting the mesh using GetDataSet is not a good solution as the dataset is the first
    //   one that is picked and it may be of different type (volume, model, etc.)
    this->Internal->FindFirstPickedDisplayNodeFromPickerProp3Ds();
    }
#else
  vtkErrorMacro("Pick3D: This function is only accessible in newer VTK version");
  (void)ras; // not used
#endif

  return 1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLSegmentationsDisplayableManager3D::GetPickedNodeID()
{
  return this->Internal->PickedDisplayNodeID.c_str();
}

//---------------------------------------------------------------------------
const char* vtkMRMLSegmentationsDisplayableManager3D::GetPickedSegmentID()
{
  return this->Internal->PickedSegmentID.c_str();
}
