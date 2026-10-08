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
#include "vtkSegmentationLabelmapSurfaceMapper.h"

// Segmentations includes
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLClipNode.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// vtkAddon includes
#include <vtkCapPolyData.h>

// SegmentationCore includes
#include <vtkBinaryLabelmapToClosedSurfaceConversionRule.h>
#include <vtkOrientedImageData.h>
#include <vtkOrientedImageDataResample.h>
#include <vtkSegmentationConverter.h>

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkCallbackCommand.h>
#include <vtkCellPicker.h>
#include <vtkClipPolyData.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkGeneralTransform.h>
#include <vtkImplicitBoolean.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPlanes.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVariant.h>

// STD includes
#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>
#include <set>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLSegmentationsDisplayableManager3D);

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

      this->CapActor = vtkSmartPointer<vtkActor>::New();
      vtkNew<vtkPolyDataMapper> capMapper;
      capMapper->SetScalarVisibility(false); // ignore any scalars that an input mesh may contain
      this->CapActor->SetMapper(capMapper.GetPointer());
      this->CapActor->SetVisibility(false);

      this->NodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      this->ModelWarper = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->ModelWarper->SetTransform(this->NodeToWorldTransform);

      this->Clipper = vtkSmartPointer<vtkClipPolyData>::New();
      this->Clipper->SetInputConnection(this->ModelWarper->GetOutputPort());

      this->ExtractPolyData = vtkSmartPointer<vtkExtractPolyDataGeometry>::New();
      this->ExtractPolyData->SetInputConnection(this->ModelWarper->GetOutputPort());
      this->ExtractPolyData->ExtractInsideOff();

      this->Capper = vtkSmartPointer<vtkCapPolyData>::New();
      this->Capper->SetInputConnection(this->ModelWarper->GetOutputPort());

      mapper->SetInputConnection(this->ModelWarper->GetOutputPort());

      this->FastTransformationNodeToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      this->UsingFastTransformation = false;
    }

    vtkSmartPointer<vtkActor> Actor;
    vtkSmartPointer<vtkActor> CapActor;
    vtkSmartPointer<vtkGeneralTransform> NodeToWorldTransform;
    vtkSmartPointer<vtkTransformPolyDataFilter> ModelWarper;
    vtkSmartPointer<vtkClipPolyData> Clipper;
    vtkSmartPointer<vtkExtractPolyDataGeometry> ExtractPolyData;
    vtkSmartPointer<vtkCapPolyData> Capper;

    // Cached matrix for linear transforms (used with SetUserMatrix for performance)
    // Mutable because these are cache/state tracking members
    mutable vtkSmartPointer<vtkMatrix4x4> FastTransformationNodeToWorldMatrix;
    mutable bool UsingFastTransformation;
  };

  typedef std::map<std::string, const Pipeline*> PipelineMapType; // first: segment ID; second: display pipeline
  typedef std::map<vtkMRMLSegmentationDisplayNode*, PipelineMapType> PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map<vtkMRMLSegmentationNode*, std::set<vtkMRMLSegmentationDisplayNode*>> SegmentationToDisplayCacheType;
  SegmentationToDisplayCacheType SegmentationToDisplayNodes;

  /// Display pipeline of segments of a binary labelmap layer, used when the 3D representation is binary labelmap.
  /// The labelmap is rendered as smooth surfaces, computed on the GPU. Segments of a layer that have the same (snapped)
  /// opacity are shown by one pipeline (translucent ones are composited front to back by the mapper, without the
  /// surfaces where they touch each other), so that rendering stays fast when many segments are translucent.
  struct LabelmapPipeline
  {
    LabelmapPipeline()
    {
      this->Mapper = vtkSmartPointer<vtkSegmentationLabelmapSurfaceMapper>::New();
      this->Actor = vtkSmartPointer<vtkActor>::New();
      this->Actor->SetMapper(this->Mapper);
      this->ImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      this->Actor->SetUserMatrix(this->ImageToWorldMatrix);
    }
    vtkSmartPointer<vtkActor> Actor;
    vtkSmartPointer<vtkSegmentationLabelmapSurfaceMapper> Mapper;
    vtkSmartPointer<vtkMatrix4x4> ImageToWorldMatrix;
    /// Pickable segments shown by the pipeline
    std::map<int, std::string> PickableSegmentIDsByLabel;
  };
  /// Opacity of translucent segments is snapped to the closest of a few evenly spaced levels between 0 and 1
  /// (\sa vtkMRMLSegmentationDisplayNode::SetNumberOfLabelmapSurfaceOpacityLevels; 0 and 1 are not used, because
  /// the segment would be hidden or opaque), so that the number of pipelines (each with its own distance field on
  /// the GPU) is limited per layer, and changing the opacity of a segment slightly does not move it to another
  /// pipeline (which would compute the distance field again). The levels are defined by the display node, so that
  /// automatic opacity computation can choose opacities that are rendered as distinct levels.
  /// first: shared labelmap of the layer, second: -1 for the opaque segments, otherwise the snapped opacity level of
  /// the translucent segments (\sa vtkMRMLSegmentationDisplayNode::GetLabelmapSurfaceTranslucentOpacityLevel)
  typedef std::pair<vtkOrientedImageData*, int> LabelmapPipelineKey;
  typedef std::map<LabelmapPipelineKey, LabelmapPipeline> LabelmapPipelineMapType;
  std::map<vtkMRMLSegmentationDisplayNode*, LabelmapPipelineMapType> LabelmapPipelines;

  /// Labelmap resampled into world coordinates, for segmentations under non-linear transforms
  struct TransformedLabelmap
  {
    vtkSmartPointer<vtkOrientedImageData> Labelmap;
    vtkMTimeType SourceLabelmapTime{ 0 };
    vtkSmartPointer<vtkGeneralTransform> NodeToWorldTransform;
  };
  std::map<vtkMRMLSegmentationDisplayNode*, std::map<vtkOrientedImageData*, TransformedLabelmap>> TransformedLabelmaps;
  vtkOrientedImageData* GetTransformedLabelmap(vtkMRMLSegmentationDisplayNode* displayNode, vtkOrientedImageData* labelmap, vtkGeneralTransform* nodeToWorld);

  // Segmentations
  void AddSegmentationNode(vtkMRMLSegmentationNode* displayableNode);
  void RemoveSegmentationNode(vtkMRMLSegmentationNode* displayableNode);

  // Transforms
  void UpdateDisplayableTransforms(vtkMRMLSegmentationNode* node);
  void GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld);

  // Display Nodes
  void AddDisplayNode(vtkMRMLSegmentationNode*, vtkMRMLSegmentationDisplayNode*);
  Pipeline* CreateSegmentPipeline(std::string segmentID);
  void UpdateDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode);
  void UpdateAllDisplayNodesForSegment(vtkMRMLSegmentationNode* segmentationNode);
  void UpdateSegmentPipelines(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType& segmentPipelines);
  void UpdateDisplayNodePipeline(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType& segmentPipelines);
  void UpdateLabelmapPipelines(vtkMRMLSegmentationDisplayNode* displayNode,
                               bool visible,
                               double hierarchyOpacity,
                               vtkMRMLDisplayNode* genericDisplayNode,
                               vtkMRMLDisplayNode* overrideHierarchyDisplayNode);
  void RemoveLabelmapPipelines(vtkMRMLSegmentationDisplayNode* displayNode);
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
  /// Find segment shown as binary labelmap at a position and set PickedNodeID in Internal
  bool FindPickedDisplayNodeFromLabelmaps(const double ras[3]);

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
vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::vtkInternal(vtkMRMLSegmentationsDisplayableManager3D* external)
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
  return displayNode                                                               //
         && displayNode->GetVisibility(this->External->GetMRMLViewNode()->GetID()) //
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

  for (int i = 0; i < nnodes; i++)
  {
    vtkMRMLSegmentationDisplayNode* dnode = vtkMRMLSegmentationDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));
    if (this->UseDisplayNode(dnode))
    {
      this->SegmentationToDisplayNodes[node].insert(dnode);
      this->AddDisplayNode(node, dnode);
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
  vtkInternal::SegmentationToDisplayCacheType::iterator displayableIt = this->SegmentationToDisplayNodes.find(node);
  if (displayableIt == this->SegmentationToDisplayNodes.end())
  {
    return;
  }

  std::set<vtkMRMLSegmentationDisplayNode*> dnodes = displayableIt->second;
  std::set<vtkMRMLSegmentationDisplayNode*>::iterator diter;
  for (diter = dnodes.begin(); diter != dnodes.end(); ++diter)
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
  std::set<vtkMRMLSegmentationDisplayNode*> displayNodes = this->SegmentationToDisplayNodes[mNode];
  std::set<vtkMRMLSegmentationDisplayNode*>::iterator dnodesIter;
  for (dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++)
  {
    if (((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()))
    {
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::RemoveDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode)
{
  this->RemoveLabelmapPipelines(displayNode);
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
    this->External->GetRenderer()->RemoveActor(pipeline->CapActor);
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
  std::vector<std::string> segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector<std::string>::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
  {
    pipelineVector[*segmentIdIt] = this->CreateSegmentPipeline(*segmentIdIt);
  }

  this->DisplayPipelines.insert(std::make_pair(displayNode, pipelineVector));

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::Pipeline* vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::CreateSegmentPipeline(std::string vtkNotUsed(segmentID))
{
  Pipeline* pipeline = new Pipeline();

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor(pipeline->Actor);
  this->External->GetRenderer()->AddActor(pipeline->CapActor);

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
    this->AddSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(displayNode->GetDisplayableNode()));
  }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateAllDisplayNodesForSegment(vtkMRMLSegmentationNode* segmentationNode)
{
  std::set<vtkMRMLSegmentationDisplayNode*> displayNodes = this->SegmentationToDisplayNodes[segmentationNode];
  for (std::set<vtkMRMLSegmentationDisplayNode*>::iterator dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++)
  {
    this->UpdateDisplayNode(*dnodesIter);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateSegmentPipelines(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType& segmentPipelines)
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
  std::vector<std::string> segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector<std::string>::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
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
      this->External->GetRenderer()->RemoveActor(pipeline->CapActor);
      delete pipeline;
    }
    else
    {
      ++pipelineIt;
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateDisplayNodePipeline(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType& segmentPipelines)
{
  // Sets visibility, set pipeline polydata input, update color
  if (!displayNode)
  {
    return;
  }

  // Get display node from hierarchy that applies display properties on branch
  vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  vtkMRMLDisplayNode* overrideHierarchyDisplayNode = vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);
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
  bool showLabelmap = (shownRepresentationName == vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  if (shownRepresentationName.empty() || showLabelmap)
  {
    // Hide poly data if there is no poly data representation to show
    for (PipelineMapType::iterator pipelineIt = segmentPipelines.begin(); pipelineIt != segmentPipelines.end(); ++pipelineIt)
    {
      pipelineIt->second->Actor->SetVisibility(false);
      pipelineIt->second->CapActor->SetVisibility(false);
    }
  }
  if (showLabelmap)
  {
    this->UpdateLabelmapPipelines(displayNode, hierarchyVisibility && displayNodeVisible, hierarchyOpacity, genericDisplayNode, overrideHierarchyDisplayNode);
    return;
  }
  this->RemoveLabelmapPipelines(displayNode);
  if (shownRepresentationName.empty())
  {
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
  for (PipelineMapType::iterator pipelineIt = segmentPipelines.begin(); pipelineIt != segmentPipelines.end(); ++pipelineIt)
  {
    const Pipeline* pipeline = pipelineIt->second;

    // Update visibility
    vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
    displayNode->GetSegmentDisplayProperties(pipelineIt->first, properties);
    bool segmentVisible = hierarchyVisibility && displayNodeVisible && properties.Visible && properties.Visible3D;
    bool clipping = displayNode->GetClipping();
    vtkMRMLClipNode* clipNode = displayNode->GetClipNode();
    int numberOfClipNodes = clipNode ? clipNode->GetNumberOfClippingNodes() : 0;
    if (clipping && clipNode)
    {
      bool allClippingOff = true;
      for (int i = 0; i < numberOfClipNodes; ++i)
      {
        if (clipNode->GetNthClippingNodeState(i) != vtkMRMLClipNode::ClipOff)
        {
          allClippingOff = false;
          break;
        }
      }
      if (allClippingOff)
      {
        clipping = false;
      }
    }
    bool capSurface = clipping && displayNode->GetClippingCapSurface();
    bool clipOutline = clipping && displayNode->GetClippingOutline();
    pipeline->Actor->SetVisibility(segmentVisible);
    pipeline->CapActor->SetVisibility(segmentVisible && (capSurface || clipOutline));
    if (!segmentVisible)
    {
      continue;
    }

    // Get poly data to display
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(segmentation->GetSegmentRepresentation(pipelineIt->first, shownRepresentationName));
    if (!polyData || polyData->GetNumberOfPoints() == 0)
    {
      pipeline->Actor->SetVisibility(false);
      pipeline->CapActor->SetVisibility(false);
      continue;
    }

    // We use a fast rendering path (positioning the actor in the renderer instead of transforming
    // each polydata point using the CPU-based warping filter) if the segmentation is only under
    // a linear transform.
    vtkMRMLTransformNode* transformNode = segmentationNode->GetParentTransformNode();
    bool fastTransformation = (transformNode == nullptr) || transformNode->IsTransformToWorldLinear();

    if (fastTransformation)
    {
      // Fast clip path: mesh stays in node coordinate system. SetUserMatrix handles the transform to world.

      vtkNew<vtkMatrix4x4> nodeToWorldMatrix;
      if (transformNode)
      {
        transformNode->GetMatrixTransformToWorld(nodeToWorldMatrix);
      }

      bool needsUpdate = !pipeline->UsingFastTransformation || !vtkAddonMathUtilities::MatrixAreEqual(pipeline->FastTransformationNodeToWorldMatrix, nodeToWorldMatrix);
      if (needsUpdate)
      {
        pipeline->UsingFastTransformation = true;
        // Save state for fast path
        pipeline->FastTransformationNodeToWorldMatrix->DeepCopy(nodeToWorldMatrix);
        // Fast path: apply linear transform via SetUserMatrix (GPU-side, zero CPU cost)
        pipeline->Actor->SetUserMatrix(nodeToWorldMatrix);
        pipeline->CapActor->SetUserMatrix(nodeToWorldMatrix);
      }

      if (clipping && clipNode)
      {
        // Linear transform with clipping
        // Connect clipper/extractor/capper to polyData (bypass ModelWarper)

        vtkImplicitFunction* clipFunction = clipNode->GetImplicitFunctionWorld();
        vtkNew<vtkImplicitBoolean> implicitBoolean;
        implicitBoolean->AddFunction(clipFunction);

        // Transform the clipping function from world coordinate system to the node coordinate system.
        vtkNew<vtkTransform> nodeToWorldLinear;
        nodeToWorldLinear->SetMatrix(nodeToWorldMatrix);
        implicitBoolean->SetTransform(nodeToWorldLinear);

        pipeline->Clipper->SetClipFunction(implicitBoolean);
        pipeline->ExtractPolyData->SetImplicitFunction(implicitBoolean);
        pipeline->Capper->SetClipFunction(implicitBoolean);

        if (pipeline->Clipper->GetInputDataObject(0, 0) != polyData)
        {
          pipeline->Clipper->SetInputData(polyData);
        }
        if (pipeline->ExtractPolyData->GetInputDataObject(0, 0) != polyData)
        {
          pipeline->ExtractPolyData->SetInputData(polyData);
        }
        if (pipeline->Capper->GetInputDataObject(0, 0) != polyData)
        {
          pipeline->Capper->SetInputData(polyData);
        }

        if (clipNode->GetClippingMethod() == vtkMRMLClipNode::Straight)
        {
          pipeline->Actor->GetMapper()->SetInputConnection(pipeline->Clipper->GetOutputPort());
        }
        else
        {
          pipeline->Actor->GetMapper()->SetInputConnection(pipeline->ExtractPolyData->GetOutputPort());
        }
        pipeline->CapActor->GetMapper()->SetInputConnection(pipeline->Capper->GetOutputPort());
      }
      else
      {
        // Linear transform without clipping: connect polyData directly to the mapper (bypass ModelWarper and clipper/extractor/capper)
        vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(pipeline->Actor->GetMapper());
        if (mapper->GetInputDataObject(0, 0) != polyData)
        {
          mapper->SetInputData(polyData);
        }
      }
    }
    else
    {
      // General (slow) path: update NodeToWorldTransform and feed ModelWarper.
      // Works for non-linear transforms, with or without clipping.

      pipeline->UsingFastTransformation = false;
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

      if (clipNode && clipping)
      {
        // Non-linear transform with clipping

        pipeline->Clipper->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
        pipeline->ExtractPolyData->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
        pipeline->Capper->SetInputConnection(pipeline->ModelWarper->GetOutputPort());

        vtkImplicitFunction* clipFunction = clipNode->GetImplicitFunctionWorld();
        pipeline->Clipper->SetClipFunction(clipFunction);
        pipeline->ExtractPolyData->SetImplicitFunction(clipFunction);
        pipeline->Capper->SetClipFunction(clipFunction);

        if (clipNode->GetClippingMethod() == vtkMRMLClipNode::Straight)
        {
          pipeline->Actor->GetMapper()->SetInputConnection(pipeline->Clipper->GetOutputPort());
        }
        else
        {
          pipeline->Actor->GetMapper()->SetInputConnection(pipeline->ExtractPolyData->GetOutputPort());
        }
        pipeline->CapActor->GetMapper()->SetInputConnection(pipeline->Capper->GetOutputPort());
      }
      else
      {
        // Non-linear transform, no clipping
        pipeline->Actor->GetMapper()->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
      }

      // Model is already in the world coordinate system
      pipeline->Actor->SetUserMatrix(nullptr);
      pipeline->CapActor->SetUserMatrix(nullptr);
    }

    // Get displayed color (if no override is defined then use the color from the segment)
    double color[3] = { vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2] };
    if (overrideHierarchyDisplayNode)
    {
      overrideHierarchyDisplayNode->GetColor(color);
    }
    else
    {
      displayNode->GetSegmentColor(pipelineIt->first, color);
    }

    pipeline->Actor->SetPickable(segmentationNode->GetSelectable() && properties.Pickable);
    pipeline->Actor->SetTexture(nullptr);
    pipeline->CapActor->SetPickable(segmentationNode->GetSelectable() && properties.Pickable);
    pipeline->CapActor->SetTexture(nullptr);

    // Update pipeline actor
    vtkProperty* actorProperty = pipeline->Actor->GetProperty();
    actorProperty->SetRepresentation(genericDisplayNode->GetRepresentation());
    actorProperty->SetPointSize(genericDisplayNode->GetPointSize());
    actorProperty->SetLineWidth(genericDisplayNode->GetLineWidth());
    actorProperty->SetLighting(genericDisplayNode->GetLighting());
    actorProperty->SetInterpolation(genericDisplayNode->GetInterpolation());
    actorProperty->SetShading(genericDisplayNode->GetShading());
    actorProperty->SetFrontfaceCulling(genericDisplayNode->GetFrontfaceCulling());
    actorProperty->SetBackfaceCulling(genericDisplayNode->GetBackfaceCulling());

    actorProperty->SetColor(color[0], color[1], color[2]);

    double opacity = hierarchyOpacity * properties.Opacity3D * displayNode->GetOpacity3D() * genericDisplayNode->GetOpacity();
    actorProperty->SetOpacity(opacity);

    if (genericDisplayNode->GetSelected())
    {
      actorProperty->SetAmbient(genericDisplayNode->GetSelectedAmbient());
      actorProperty->SetSpecular(genericDisplayNode->GetSelectedSpecular());
    }
    else
    {
      actorProperty->SetAmbient(genericDisplayNode->GetAmbient());
      actorProperty->SetSpecular(genericDisplayNode->GetSpecular());
    }
    actorProperty->SetDiffuse(genericDisplayNode->GetDiffuse());
    actorProperty->SetSpecularPower(genericDisplayNode->GetPower());
    actorProperty->SetMetallic(genericDisplayNode->GetMetallic());
    actorProperty->SetRoughness(genericDisplayNode->GetRoughness());
    actorProperty->SetEdgeVisibility(genericDisplayNode->GetEdgeVisibility());
    actorProperty->SetEdgeColor(genericDisplayNode->GetEdgeColor());

    // Update cap actor display properties
    pipeline->CapActor->GetProperty()->DeepCopy(actorProperty);

    // Create a lookup table to map cell data to colors.
    vtkNew<vtkLookupTable> lut;
    lut->SetTableRange(VTK_LINE, VTK_POLY_LINE);
    lut->SetNumberOfColors(1);
    lut->Build();
    lut->UseBelowRangeColorOn();
    lut->UseAboveRangeColorOn();
    lut->SetBelowRangeColor(color[0], color[1], color[2], capSurface ? opacity * displayNode->GetClippingCapOpacity() : 0.0);
    lut->SetAboveRangeColor(color[0], color[1], color[2], capSurface ? opacity * displayNode->GetClippingCapOpacity() : 0.0);
    lut->SetTableValue(0, 0.0, 0.0, 0.0, clipOutline ? opacity : 0.0);

    vtkMapper* capMapper = pipeline->CapActor->GetMapper();
    capMapper->SetScalarModeToUseCellData();
    capMapper->UseLookupTableScalarRangeOn();
    capMapper->SetColorModeToMapScalars();
    capMapper->SetLookupTable(lut);
    capMapper->SetScalarVisibility(true);
  }
}

//---------------------------------------------------------------------------
vtkOrientedImageData* vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::GetTransformedLabelmap(vtkMRMLSegmentationDisplayNode* displayNode,
                                                                                                    vtkOrientedImageData* labelmap,
                                                                                                    vtkGeneralTransform* nodeToWorld)
{
  TransformedLabelmap& transformed = this->TransformedLabelmaps[displayNode][labelmap];
  bool labelmapCurrent = transformed.Labelmap && transformed.SourceLabelmapTime == labelmap->GetMTime();
  if (labelmapCurrent && vtkMRMLTransformNode::AreTransformsEqual(nodeToWorld, transformed.NodeToWorldTransform))
  {
    return transformed.Labelmap;
  }
  // Resample the labelmap (on the CPU) into world coordinates
  transformed.Labelmap = vtkSmartPointer<vtkOrientedImageData>::New();
  transformed.Labelmap->DeepCopy(labelmap);
  vtkOrientedImageDataResample::TransformOrientedImage(transformed.Labelmap, nodeToWorld, false, true);
  transformed.SourceLabelmapTime = labelmap->GetMTime();
  transformed.NodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  transformed.NodeToWorldTransform->DeepCopy(nodeToWorld);
  return transformed.Labelmap;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::UpdateLabelmapPipelines(vtkMRMLSegmentationDisplayNode* displayNode,
                                                                                    bool visible,
                                                                                    double hierarchyOpacity,
                                                                                    vtkMRMLDisplayNode* genericDisplayNode,
                                                                                    vtkMRMLDisplayNode* overrideHierarchyDisplayNode)
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(displayNode->GetDisplayableNode());
  vtkSegmentation* segmentation = segmentationNode ? segmentationNode->GetSegmentation() : nullptr;
  if (!segmentation)
  {
    this->RemoveLabelmapPipelines(displayNode);
    return;
  }

  // Transform from the segmentation node to world. Labelmaps under non-linear transforms are resampled into world coordinates.
  vtkNew<vtkMatrix4x4> nodeToWorldMatrix;
  vtkMRMLTransformNode* transformNode = segmentationNode->GetParentTransformNode();
  vtkSmartPointer<vtkGeneralTransform> nonLinearNodeToWorld;
  if (transformNode)
  {
    if (transformNode->IsTransformToWorldLinear())
    {
      transformNode->GetMatrixTransformToWorld(nodeToWorldMatrix);
    }
    else
    {
      nonLinearNodeToWorld = vtkSmartPointer<vtkGeneralTransform>::New();
      this->GetNodeTransformToWorld(segmentationNode, nonLinearNodeToWorld);
    }
  }

  // Shown segments of each labelmap layer, grouped by opacity
  const char* binaryLabelmapName = vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName();
  std::map<LabelmapPipelineKey, std::vector<std::string>> shownSegmentIDsInPipelines;
  std::map<LabelmapPipelineKey, double> pipelineOpacities;
  double displayOpacity = hierarchyOpacity * displayNode->GetOpacity3D() * genericDisplayNode->GetOpacity();
  if (visible && displayOpacity > 0.0)
  {
    std::vector<std::string> segmentIDs;
    segmentation->GetSegmentIDs(segmentIDs);
    for (const std::string& segmentID : segmentIDs)
    {
      vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
      displayNode->GetSegmentDisplayProperties(segmentID, properties);
      double opacity = displayOpacity * properties.Opacity3D;
      if (!properties.Visible || !properties.Visible3D || opacity <= 0.0)
      {
        continue;
      }
      vtkOrientedImageData* labelmap = vtkOrientedImageData::SafeDownCast(segmentation->GetSegmentRepresentation(segmentID, binaryLabelmapName));
      if (!labelmap)
      {
        continue;
      }
      bool opaque = (opacity >= 1.0);
      LabelmapPipelineKey key(labelmap, opaque ? -1 : displayNode->GetLabelmapSurfaceTranslucentOpacityLevel(opacity));
      shownSegmentIDsInPipelines[key].push_back(segmentID);
      pipelineOpacities[key] = opaque ? 1.0 : displayNode->GetLabelmapSurfaceTranslucentOpacity(key.second);
    }
  }

  LabelmapPipelineMapType& pipelines = this->LabelmapPipelines[displayNode];

  // Remove pipelines that have no shown segments
  for (LabelmapPipelineMapType::iterator pipelineIt = pipelines.begin(); pipelineIt != pipelines.end();)
  {
    if (shownSegmentIDsInPipelines.find(pipelineIt->first) == shownSegmentIDsInPipelines.end())
    {
      this->External->GetRenderer()->RemoveActor(pipelineIt->second.Actor);
      pipelineIt = pipelines.erase(pipelineIt);
    }
    else
    {
      ++pipelineIt;
    }
  }

  // Resampled labelmaps that are not used anymore
  std::map<vtkOrientedImageData*, TransformedLabelmap>& transformedLabelmaps = this->TransformedLabelmaps[displayNode];
  for (auto transformedIt = transformedLabelmaps.begin(); transformedIt != transformedLabelmaps.end();)
  {
    bool used = false;
    for (const auto& shownSegmentIDsInPipeline : shownSegmentIDsInPipelines)
    {
      used |= (nonLinearNodeToWorld && shownSegmentIDsInPipeline.first.first == transformedIt->first);
    }
    transformedIt = used ? std::next(transformedIt) : transformedLabelmaps.erase(transformedIt);
  }

  // Same smoothing as closed surface representation (negative value means smoothing is disabled)
  std::string smoothingFactorString = segmentation->GetConversionParameter(vtkBinaryLabelmapToClosedSurfaceConversionRule::GetSmoothingFactorParameterName());
  double smoothingFactor = smoothingFactorString.empty() ? 0.5 : std::clamp(vtkVariant(smoothingFactorString).ToDouble(), 0.0, 1.0);

  // Clipping
  vtkSmartPointer<vtkPlaneCollection> clippingPlanes;
  vtkMRMLClipNode* clipNode = displayNode->GetClipNode();
  if (displayNode->GetClipping() && clipNode)
  {
    clippingPlanes = vtkSmartPointer<vtkPlaneCollection>::New();
    clipNode->GetClippingPlanes(clippingPlanes);
    if (clippingPlanes->GetNumberOfItems() == 0)
    {
      clippingPlanes = nullptr;
    }
  }

  for (const auto& shownSegmentIDsInPipeline : shownSegmentIDsInPipelines)
  {
    const LabelmapPipelineKey& key = shownSegmentIDsInPipeline.first;
    LabelmapPipelineMapType::iterator pipelineIt = pipelines.find(key);
    if (pipelineIt == pipelines.end())
    {
      pipelineIt = pipelines.insert(std::make_pair(key, LabelmapPipeline())).first;
      this->External->GetRenderer()->AddActor(pipelineIt->second.Actor);
    }
    LabelmapPipeline& pipeline = pipelineIt->second;
    vtkOrientedImageData* labelmap = key.first;
    if (nonLinearNodeToWorld)
    {
      labelmap = this->GetTransformedLabelmap(displayNode, labelmap, nonLinearNodeToWorld);
    }
    pipeline.Mapper->SetLabelmap(labelmap);
    pipeline.Mapper->RemoveAllLabelColors();
    pipeline.PickableSegmentIDsByLabel.clear();
    for (const std::string& segmentID : shownSegmentIDsInPipeline.second)
    {
      double color[3] = { vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2] };
      if (overrideHierarchyDisplayNode)
      {
        overrideHierarchyDisplayNode->GetColor(color);
      }
      else
      {
        displayNode->GetSegmentColor(segmentID, color);
      }
      int labelValue = segmentation->GetSegment(segmentID)->GetLabelValue();
      pipeline.Mapper->SetLabelColor(labelValue, color[0], color[1], color[2]);
      vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
      displayNode->GetSegmentDisplayProperties(segmentID, properties);
      if (segmentationNode->GetSelectable() && properties.Pickable)
      {
        pipeline.PickableSegmentIDsByLabel[labelValue] = segmentID;
      }
    }

    // Model coordinates of the mapper are IJK coordinates of the labelmap
    vtkNew<vtkMatrix4x4> imageToNodeMatrix;
    labelmap->GetImageToWorldMatrix(imageToNodeMatrix);
    vtkNew<vtkMatrix4x4> imageToWorldMatrix;
    vtkMatrix4x4::Multiply4x4(nodeToWorldMatrix, imageToNodeMatrix, imageToWorldMatrix);
    if (!vtkAddonMathUtilities::MatrixAreEqual(imageToWorldMatrix, pipeline.ImageToWorldMatrix))
    {
      pipeline.ImageToWorldMatrix->DeepCopy(imageToWorldMatrix);
    }

    pipeline.Mapper->SetSmoothingFactor(smoothingFactor);
    pipeline.Mapper->SetClippingPlanes(clippingPlanes);
    // ClipUnion clips away the union of the clipped spaces (keeps where all planes keep), ClipIntersection only their
    // intersection (keeps where any plane keeps), as for models
    pipeline.Mapper->SetKeepWhereAnyClippingPlaneKeeps(clipNode && clipNode->GetClipType() == vtkMRMLClipNode::ClipIntersection);
    pipeline.Mapper->SetCapClippedSurface(displayNode->GetClippingCapSurface());
    pipeline.Mapper->SetCapOpacity(displayNode->GetClippingCapOpacity());
    // Clipping outline as models show it: in the edge color of the display node
    pipeline.Mapper->SetClippingOutline(displayNode->GetClippingOutline());
    pipeline.Mapper->SetOutlineColor(displayNode->GetEdgeColor());
    pipeline.Mapper->SetOutlineWidth(std::max(1.0, static_cast<double>(displayNode->GetLineWidth())));

    vtkProperty* actorProperty = pipeline.Actor->GetProperty();
    actorProperty->SetOpacity(pipelineOpacities[key]);
    actorProperty->SetAmbient(genericDisplayNode->GetSelected() ? genericDisplayNode->GetSelectedAmbient() : genericDisplayNode->GetAmbient());
    actorProperty->SetSpecular(genericDisplayNode->GetSelected() ? genericDisplayNode->GetSelectedSpecular() : genericDisplayNode->GetSpecular());
    actorProperty->SetDiffuse(genericDisplayNode->GetDiffuse());
    actorProperty->SetSpecularPower(genericDisplayNode->GetPower());
    pipeline.Actor->SetVisibility(true);
    // The mapper intersects pick rays with the labelmap (it is a vtkMRMLRayCastMapper, see vtkMRMLAccuratePicker),
    // the segment at the picked position is found by FindPickedDisplayNodeFromLabelmaps
    pipeline.Actor->SetPickable(!pipeline.PickableSegmentIDsByLabel.empty());
  }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::RemoveLabelmapPipelines(vtkMRMLSegmentationDisplayNode* displayNode)
{
  this->TransformedLabelmaps.erase(displayNode);
  auto pipelinesIt = this->LabelmapPipelines.find(displayNode);
  if (pipelinesIt == this->LabelmapPipelines.end())
  {
    return;
  }
  for (auto& pipeline : pipelinesIt->second)
  {
    this->External->GetRenderer()->RemoveActor(pipeline.second.Actor);
  }
  this->LabelmapPipelines.erase(pipelinesIt);
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager3D::vtkInternal::FindPickedDisplayNodeFromLabelmaps(const double ras[3])
{
  for (auto& displayNodePipelines : this->LabelmapPipelines)
  {
    for (auto& keyPipeline : displayNodePipelines.second)
    {
      LabelmapPipeline& pipeline = keyPipeline.second;
      if (!pipeline.Actor->GetVisibility() || pipeline.PickableSegmentIDsByLabel.empty())
      {
        continue;
      }
      vtkNew<vtkMatrix4x4> worldToImage;
      vtkMatrix4x4::Invert(pipeline.ImageToWorldMatrix, worldToImage);
      double rasPoint[4] = { ras[0], ras[1], ras[2], 1.0 };
      double ijk[4] = { 0.0, 0.0, 0.0, 1.0 };
      worldToImage->MultiplyPoint(rasPoint, ijk);
      int label = pipeline.Mapper->GetShownLabelAtPosition(ijk);
      auto segmentIt = pipeline.PickableSegmentIDsByLabel.find(label);
      if (segmentIt != pipeline.PickableSegmentIDsByLabel.end())
      {
        this->PickedDisplayNodeID = displayNodePipelines.first->GetID();
        this->PickedSegmentID = segmentIt->second;
        return true;
      }
    }
  }
  return false;
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
    broker->AddObservation(node, vtkMRMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  }
  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand()))
  {
    broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  }
  if (!broker->GetObservationExist(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand()))
  {
    broker->AddObservation(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand());
  }
  if (!broker->GetObservationExist(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetMRMLNodesCallbackCommand()))
  {
    broker->AddObservation(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetMRMLNodesCallbackCommand());
  }
  if (!broker->GetObservationExist(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetMRMLNodesCallbackCommand()))
  {
    broker->AddObservation(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetMRMLNodesCallbackCommand());
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
  observations = broker->GetObservations(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetMRMLNodesCallbackCommand());
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
  while (this->SegmentationToDisplayNodes.size() > 0)
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
  for (pipelinesIt = this->DisplayPipelines.begin(); pipelinesIt != this->DisplayPipelines.end(); ++pipelinesIt)
  {
    vtkMRMLSegmentationDisplayNode* currentDisplayNode = pipelinesIt->first;
    for (PipelineMapType::iterator pipelineIt = pipelinesIt->second.begin(); pipelineIt != pipelinesIt->second.end(); ++pipelineIt)
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
  for (int propIndex = 0; propIndex < props->GetNumberOfItems(); ++propIndex)
  {
    vtkProp3D* pickedProp = vtkProp3D::SafeDownCast(props->GetItemAsObject(propIndex));
    if (!pickedProp)
    {
      continue;
    }

    PipelinesCacheType::iterator pipelinesIt;
    for (pipelinesIt = this->DisplayPipelines.begin(); pipelinesIt != this->DisplayPipelines.end(); ++pipelinesIt)
    {
      vtkMRMLSegmentationDisplayNode* currentDisplayNode = pipelinesIt->first;
      for (PipelineMapType::iterator pipelineIt = pipelinesIt->second.begin(); pipelineIt != pipelinesIt->second.end(); ++pipelineIt)
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
  this->Internal = nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkMRMLSegmentationsDisplayableManager3D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager3D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node->IsA("vtkMRMLSegmentationNode"))
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
  if (node                                       //
      && (!node->IsA("vtkMRMLSegmentationNode")) //
      && (!node->IsA("vtkMRMLSegmentationDisplayNode")))
  {
    return;
  }

  vtkMRMLSegmentationNode* segmentationNode = nullptr;
  vtkMRMLSegmentationDisplayNode* displayNode = nullptr;

  bool modified = false;
  if ((segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node)))
  {
    this->Internal->RemoveSegmentationNode(segmentationNode);
    modified = true;
  }
  else if ((displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(node)))
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

  if (scene == nullptr || scene->IsBatchProcessing())
  {
    return;
  }

  vtkMRMLSegmentationNode* displayableNode = vtkMRMLSegmentationNode::SafeDownCast(caller);
  if (displayableNode)
  {
    if (event == vtkMRMLDisplayableNode::DisplayModifiedEvent)
    {
      vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode*>(callData);
      vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(callDataNode);
      if (displayNode)
      {
        this->Internal->UpdateDisplayNode(displayNode);
      }
      else
      {
        // The event does not tell which display node was modified if it was invoked after modification
        // of the segmentation node ended (MRMLNodeModifyBlocker): update all display nodes
        this->Internal->UpdateAllDisplayNodesForSegment(displayableNode);
      }
      this->RequestRender();
    }
    else if ((event == vtkMRMLDisplayableNode::TransformModifiedEvent)      //
             || (event == vtkMRMLTransformableNode::TransformModifiedEvent) //
             || (event == vtkSegmentation::RepresentationModified)          //
             || (event == vtkSegmentation::SegmentModified))
    {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
    }
    else if ((event == vtkCommand::ModifiedEvent)        // segmentation object may be replaced
             || (event == vtkSegmentation::SegmentAdded) //
             || (event == vtkSegmentation::SegmentRemoved))
    {
      this->Internal->UpdateAllDisplayNodesForSegment(displayableNode);
      this->RequestRender();
    }
  }
  else if (vtkMRMLClipNode::SafeDownCast(caller))
  {
    vtkMRMLClipNode* clipNode = vtkMRMLClipNode::SafeDownCast(caller);
    for (auto displayNodeIt = this->Internal->DisplayPipelines.begin(); displayNodeIt != this->Internal->DisplayPipelines.end(); ++displayNodeIt)
    {
      if (displayNodeIt->first->GetClipNode() != clipNode)
      {
        continue;
      }
      this->Internal->UpdateDisplayNodePipeline(displayNodeIt->first, displayNodeIt->second);
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
    vtkDebugMacro("vtkMRMLSegmentationsDisplayableManager3D::UpdateFromMRML: Scene is not set");
    return;
  }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLSegmentationNode* segmentationNode = nullptr;
  std::vector<vtkMRMLNode*> segmentationNodes;
  int numOfSegmentationNodes = scene ? scene->GetNodesByClass("vtkMRMLSegmentationNode", segmentationNodes) : 0;
  for (int i = 0; i < numOfSegmentationNodes; i++)
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

  if (this->Internal->CellPicker->Pick3DPoint(ras, ren))
  {
    // Find first picked segmentation and segment from picker
    // Note: Getting the mesh using GetDataSet is not a good solution as the dataset is the first
    //   one that is picked and it may be of different type (volume, model, etc.)
    this->Internal->FindFirstPickedDisplayNodeFromPickerProp3Ds();
  }
  if (this->Internal->PickedDisplayNodeID.empty())
  {
    // Segments shown as binary labelmap
    this->Internal->FindPickedDisplayNodeFromLabelmaps(ras);
  }

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
