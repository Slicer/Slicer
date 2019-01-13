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
#include "vtkMRMLSegmentationsDisplayableManager2D.h"

// MRML includes
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLTransformNode.h>

// MRML logic includes
#include "vtkImageLabelOutline.h"

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"

// VTK includes
#include <vtkVersion.h> // must precede reference to VTK_MAJOR_VERSION
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkCellArray.h>
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkPlaneCutter.h>
#else
#include <vtkCutter.h>
#endif
#include <vtkCleanPolyData.h>
#include <vtkContourTriangulator.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkEventBroker.h>
#include <vtkGeneralTransform.h>
#include <vtkImageMapper.h>
#include <vtkImageMapToRGBA.h>
#include <vtkImageThreshold.h>
#include <vtkImageReslice.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkStripper.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>

// STD includes
#include <algorithm>
#include <set>
#include <map>
#include <sstream>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLSegmentationsDisplayableManager2D );

//---------------------------------------------------------------------------
// Convert a linear transform that is almost exactly a permute transform
// to an exact permute transform.
// vtkImageReslice works about 10-20% faster if it reslices along an axis
// (transformation is just a permutation). However, vtkImageReslice
// checks for strict (floatValue!=0) to consider a matrix element zero.
// Here we set a small floatValue to 0 if it is several magnitudes
// (controlled by SUPPRESSION_FACTOR parameter) smaller than the
// maximum norm of the axis.
//----------------------------------------------------------------------------
void SnapToPermuteMatrix(vtkTransform* transform)
{
  const double SUPPRESSION_FACTOR = 1e-3;
  vtkHomogeneousTransform* linearTransform = vtkHomogeneousTransform::SafeDownCast(transform);
  if (!linearTransform)
    {
    // it is not a simple linear transform, so it cannot be snapped to a permute matrix
    return;
    }
  bool modified = false;
  vtkNew<vtkMatrix4x4> transformMatrix;
  linearTransform->GetMatrix(transformMatrix.GetPointer());
  for (int c=0; c<3; c++)
    {
    double absValues[3] = {fabs(transformMatrix->Element[0][c]), fabs(transformMatrix->Element[1][c]), fabs(transformMatrix->Element[2][c])};
    double maxValue = std::max(absValues[0], std::max(absValues[1], absValues[2]));
    double zeroThreshold = SUPPRESSION_FACTOR * maxValue;
    for (int r=0; r<3; r++)
      {
      if (absValues[r]!=0 && absValues[r]<zeroThreshold)
        {
        transformMatrix->Element[r][c]=0;
        modified = true;
        }
      }
    }
  if (modified)
    {
    transform->SetMatrix(transformMatrix.GetPointer());
    }
}

//---------------------------------------------------------------------------
class vtkMRMLSegmentationsDisplayableManager2D::vtkInternal
{
public:

  vtkInternal( vtkMRMLSegmentationsDisplayableManager2D* external );
  ~vtkInternal();

  struct Pipeline
    {
    Pipeline()
      {
      this->WorldToSliceTransform = vtkSmartPointer<vtkTransform>::New();
      this->NodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      this->WorldToNodeTransform = vtkSmartPointer<vtkGeneralTransform>::New();

      this->SliceIntersectionUpdatedTime = 0;

      // Create poly data pipeline
      this->PolyDataOutlineActor = vtkSmartPointer<vtkActor2D>::New();
      this->PolyDataFillActor = vtkSmartPointer<vtkActor2D>::New();
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
      this->Cutter = vtkSmartPointer<vtkPlaneCutter>::New();
#else
      this->Cutter = vtkSmartPointer<vtkCutter>::New();
#endif
      this->ModelWarper = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->Plane = vtkSmartPointer<vtkPlane>::New();
      this->Triangulator = vtkSmartPointer<vtkContourTriangulator>::New();

      // Set up poly data outline pipeline
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
      this->Cutter->SetInputConnection(this->ModelWarper->GetOutputPort());
      this->Cutter->SetPlane(this->Plane);
      this->Cutter->BuildTreeOff(); // the cutter crashes for complex geometries if build tree is enabled
      vtkSmartPointer<vtkTransformPolyDataFilter> polyDataOutlineTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      vtkNew<vtkCompositeDataGeometryFilter> geometryFilter; // merge multi-piece output of vtkPlaneCutter
      geometryFilter->SetInputConnection(this->Cutter->GetOutputPort());
      polyDataOutlineTransformer->SetInputConnection(geometryFilter->GetOutputPort());
#else
      this->Cutter->SetCutFunction(this->Plane);
      this->Cutter->SetGenerateCutScalars(0);
      vtkSmartPointer<vtkTransformPolyDataFilter> polyDataOutlineTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      polyDataOutlineTransformer->SetInputConnection(this->Cutter->GetOutputPort());
#endif
      polyDataOutlineTransformer->SetTransform(this->WorldToSliceTransform);
      vtkSmartPointer<vtkPolyDataMapper2D> polyDataOutlineMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      polyDataOutlineMapper->SetInputConnection(polyDataOutlineTransformer->GetOutputPort());
      polyDataOutlineMapper->ScalarVisibilityOff();
      this->PolyDataOutlineActor->SetMapper(polyDataOutlineMapper);
      this->PolyDataOutlineActor->SetVisibility(0);

      // Set up poly data fill pipeline
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
      vtkNew<vtkCleanPolyData> pointMerger;
      pointMerger->PointMergingOn();
      pointMerger->SetInputConnection(geometryFilter->GetOutputPort());
      this->Triangulator->SetInputConnection(pointMerger->GetOutputPort());
#else
      this->Triangulator->SetInputConnection(this->Cutter->GetOutputPort());
#endif
      vtkSmartPointer<vtkTransformPolyDataFilter> polyDataFillTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      polyDataFillTransformer->SetInputConnection(this->Triangulator->GetOutputPort());
      polyDataFillTransformer->SetTransform(this->WorldToSliceTransform);
      vtkSmartPointer<vtkPolyDataMapper2D> polyDataFillMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      polyDataFillMapper->SetInputConnection(polyDataFillTransformer->GetOutputPort());
      polyDataFillMapper->ScalarVisibilityOff();
      this->PolyDataFillActor->SetMapper(polyDataFillMapper);
      this->PolyDataFillActor->SetVisibility(0);

      // Create image pipeline
      this->ImageOutlineActor = vtkSmartPointer<vtkActor2D>::New();
      this->ImageFillActor = vtkSmartPointer<vtkActor2D>::New();
      this->Reslice = vtkSmartPointer<vtkImageReslice>::New();
      this->SliceToImageTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      this->LabelOutline = vtkSmartPointer<vtkImageLabelOutline>::New();
      this->LookupTableOutline = vtkSmartPointer<vtkLookupTable>::New();
      this->LookupTableFill = vtkSmartPointer<vtkLookupTable>::New();
      this->ImageThreshold = vtkSmartPointer<vtkImageThreshold>::New();

      // Set up image pipeline
      this->Reslice->SetBackgroundColor(0.0, 0.0, 0.0, 0.0);
      this->Reslice->AutoCropOutputOff();
      this->Reslice->SetOptimization(1);
      this->Reslice->SetOutputOrigin(0.0, 0.0, 0.0);
      this->Reslice->SetOutputSpacing(1.0, 1.0, 1.0);
      this->Reslice->SetOutputDimensionality(3);
      this->Reslice->GenerateStencilOutputOn();

      this->SliceToImageTransform->PostMultiply();

      this->ImageThreshold->SetInputConnection(this->Reslice->GetOutputPort());
      this->ImageThreshold->SetOutValue(1);
      this->ImageThreshold->SetInValue(0);

      // Image outline
      this->LabelOutline->SetInputConnection(this->Reslice->GetOutputPort());
      vtkSmartPointer<vtkImageMapToRGBA> outlineColorMapper = vtkSmartPointer<vtkImageMapToRGBA>::New();
      outlineColorMapper->SetInputConnection(this->LabelOutline->GetOutputPort());
      outlineColorMapper->SetOutputFormatToRGBA();
      outlineColorMapper->SetLookupTable(this->LookupTableOutline);
      vtkSmartPointer<vtkImageMapper> imageOutlineMapper = vtkSmartPointer<vtkImageMapper>::New();
      imageOutlineMapper->SetInputConnection(outlineColorMapper->GetOutputPort());
      imageOutlineMapper->SetColorWindow(255);
      imageOutlineMapper->SetColorLevel(127.5);
      this->ImageOutlineActor->SetMapper(imageOutlineMapper);
      this->ImageOutlineActor->SetVisibility(0);

      // Image fill
      vtkSmartPointer<vtkImageMapToRGBA> fillColorMapper = vtkSmartPointer<vtkImageMapToRGBA>::New();
      fillColorMapper->SetInputConnection(this->Reslice->GetOutputPort());
      fillColorMapper->SetOutputFormatToRGBA();
      fillColorMapper->SetLookupTable(this->LookupTableFill);
      vtkSmartPointer<vtkImageMapper> imageFillMapper = vtkSmartPointer<vtkImageMapper>::New();
      imageFillMapper->SetInputConnection(fillColorMapper->GetOutputPort());
      imageFillMapper->SetColorWindow(255);
      imageFillMapper->SetColorLevel(127.5);
      this->ImageFillActor->SetMapper(imageFillMapper);
      this->ImageFillActor->SetVisibility(0);
      }

    vtkSmartPointer<vtkTransform> WorldToSliceTransform;
    vtkSmartPointer<vtkGeneralTransform> NodeToWorldTransform;
    vtkSmartPointer<vtkGeneralTransform> WorldToNodeTransform;

    vtkSmartPointer<vtkActor2D> PolyDataOutlineActor;
    vtkSmartPointer<vtkActor2D> PolyDataFillActor;
    vtkSmartPointer<vtkTransformPolyDataFilter> ModelWarper;
    vtkSmartPointer<vtkPlane> Plane;
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
    vtkSmartPointer<vtkPlaneCutter> Cutter;
#else
    vtkSmartPointer<vtkCutter> Cutter;
#endif
    vtkSmartPointer<vtkContourTriangulator> Triangulator;

    vtkSmartPointer<vtkActor2D> ImageOutlineActor;
    vtkSmartPointer<vtkActor2D> ImageFillActor;
    vtkSmartPointer<vtkImageReslice> Reslice;
    vtkSmartPointer<vtkGeneralTransform> SliceToImageTransform;
    vtkSmartPointer<vtkImageLabelOutline> LabelOutline;
    vtkSmartPointer<vtkLookupTable> LookupTableOutline;
    vtkSmartPointer<vtkLookupTable> LookupTableFill;
    vtkSmartPointer<vtkImageThreshold> ImageThreshold;

    vtkMTimeType SliceIntersectionUpdatedTime;
    };

  typedef std::map<vtkSmartPointer<vtkDataObject>, Pipeline*> PipelineMapType; // first: representation object; second: display pipeline
  typedef std::map < vtkMRMLSegmentationDisplayNode*, PipelineMapType > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLSegmentationNode*, std::set< vtkMRMLSegmentationDisplayNode* > > SegmentationToDisplayCacheType;
  SegmentationToDisplayCacheType SegmentationToDisplayNodes;

  // Segmentations
  void AddSegmentationNode(vtkMRMLSegmentationNode* displayableNode);
  void RemoveSegmentationNode(vtkMRMLSegmentationNode* displayableNode);

  // Transforms
  void UpdateDisplayableTransforms(vtkMRMLSegmentationNode *node);
  void GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld, vtkGeneralTransform* transformFromWorld);

  // Slice Node
  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  void UpdateSliceNode();
  void SetSlicePlaneFromMatrix(vtkMatrix4x4* matrix, vtkPlane* plane);

  // Display Nodes
  void AddDisplayNode(vtkMRMLSegmentationNode*, vtkMRMLSegmentationDisplayNode*);
  Pipeline* CreateSegmentPipeline();
  void UpdateDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode);
  void UpdateAllDisplayNodesForSegment(vtkMRMLSegmentationNode* segmentationNode);
  void UpdateSegmentPipelines(vtkMRMLSegmentationDisplayNode*, PipelineMapType&);
  void UpdateDisplayNodePipeline(vtkMRMLSegmentationDisplayNode*, PipelineMapType&);
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
  bool IsSegmentVisibleInCurrentSlice(vtkMRMLSegmentationDisplayNode* displayNode, Pipeline* pipeline, const std::string &segmentID);

private:
  vtkSmartPointer<vtkMatrix4x4> SliceXYToRAS;
  vtkMRMLSegmentationsDisplayableManager2D* External;
  bool AddingSegmentationNode;
  vtkSmartPointer<vtkMRMLSliceNode> SliceNode;

  bool SmoothFractionalLabelMapBorder;
  vtkIdType DefaultFractionalInterpolationType;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::vtkInternal(vtkMRMLSegmentationsDisplayableManager2D* external)
: External(external)
, AddingSegmentationNode(false)
{
  this->SliceXYToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  this->SliceXYToRAS->Identity();

  this->SmoothFractionalLabelMapBorder = false;
  this->DefaultFractionalInterpolationType = VTK_LINEAR_INTERPOLATION;
}

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
  this->SliceNode = nullptr;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::UseDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode)
{
   // allow annotations to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->SliceNode->GetID()))
    {
    return false;
    }

  // Check whether DisplayNode should be shown in this view
  bool use = displayNode && displayNode->IsA("vtkMRMLSegmentationDisplayNode");

  return use;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::IsVisible(vtkMRMLSegmentationDisplayNode* displayNode)
{
  return displayNode && displayNode->GetVisibility() && displayNode->GetVisibility2D()
      && displayNode->GetVisibility(this->External->GetMRMLSliceNode()->GetID())
      && (displayNode->GetVisibility2DOutline() || displayNode->GetVisibility2DFill())
      && (displayNode->GetOpacity2DOutline() > 0 || displayNode->GetOpacity2DFill() > 0);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
    {
    return;
    }
  this->SliceNode=sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::UpdateSliceNode()
{
  // Update the Slice node transform then update the DisplayNode pipelines to account for plane location
  this->SliceXYToRAS->DeepCopy( this->SliceNode->GetXYToRAS() );
  PipelinesCacheType::iterator displayNodeIt;
  for (displayNodeIt = this->DisplayPipelines.begin(); displayNodeIt != this->DisplayPipelines.end(); ++displayNodeIt)
    {
    this->UpdateDisplayNodePipeline(displayNodeIt->first, displayNodeIt->second);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::SetSlicePlaneFromMatrix(vtkMatrix4x4* sliceMatrix, vtkPlane* plane)
{
  double normal[3] = {0.0,0.0,0.0};
  double origin[3] = {0.0,0.0,0.0};

  // +/-1: orientation of the normal
  const int planeOrientation = 1;
  for (int i = 0; i < 3; i++)
    {
    normal[i] = planeOrientation * sliceMatrix->GetElement(i,2);
    origin[i] = sliceMatrix->GetElement(i,3);
    }

  vtkMath::Normalize(normal);
  plane->SetNormal(normal);
  plane->SetOrigin(origin);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::AddSegmentationNode(vtkMRMLSegmentationNode* node)
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
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::RemoveSegmentationNode(vtkMRMLSegmentationNode* node)
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

  std::set< vtkMRMLSegmentationDisplayNode* > dnodes = displayableIt->second;
  std::set< vtkMRMLSegmentationDisplayNode* >::iterator diter;
  for (diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->RemoveDisplayNode(*diter);
    }
  this->RemoveObservations(node);
  this->SegmentationToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld, vtkGeneralTransform* transformFromWorld)
{
  if (!node || !transformToWorld)
    {
    return;
    }

  vtkMRMLTransformNode* tnode = node->GetParentTransformNode();

  transformToWorld->Identity();
  transformFromWorld->Identity();
  if (tnode)
    {
    tnode->GetTransformToWorld(transformToWorld);
    // Need inverse of the transform for image resampling
    tnode->GetTransformFromWorld(transformFromWorld);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::UpdateDisplayableTransforms(vtkMRMLSegmentationNode* mNode)
{
  // Update the NodeToWorld matrix for all tracked DisplayableNode
  PipelinesCacheType::iterator pipelinesIter;
  std::set<vtkMRMLSegmentationDisplayNode *> displayNodes = this->SegmentationToDisplayNodes[mNode];
  std::set<vtkMRMLSegmentationDisplayNode *>::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      for (PipelineMapType::iterator pipelineIt=pipelinesIter->second.begin(); pipelineIt!=pipelinesIter->second.end(); ++pipelineIt)
        {
        Pipeline* currentPipeline = pipelineIt->second;
        currentPipeline->SliceIntersectionUpdatedTime = 0; // Trigger slice intersection recomputation
        this->GetNodeTransformToWorld(mNode, currentPipeline->NodeToWorldTransform, currentPipeline->WorldToNodeTransform);
        }
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::RemoveDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode)
{
  PipelinesCacheType::iterator pipelinesIter = this->DisplayPipelines.find(displayNode);
  if (pipelinesIter == this->DisplayPipelines.end())
    {
    return;
    }
  PipelineMapType::iterator pipelineIt;
  for (pipelineIt = pipelinesIter->second.begin(); pipelineIt != pipelinesIter->second.end(); ++pipelineIt)
    {
    Pipeline* pipeline = pipelineIt->second;
    this->External->GetRenderer()->RemoveActor(pipeline->PolyDataOutlineActor);
    this->External->GetRenderer()->RemoveActor(pipeline->PolyDataFillActor);
    this->External->GetRenderer()->RemoveActor(pipeline->ImageOutlineActor);
    this->External->GetRenderer()->RemoveActor(pipeline->ImageFillActor);
    delete pipeline;
    }
  this->DisplayPipelines.erase(pipelinesIter);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::AddDisplayNode(vtkMRMLSegmentationNode* mNode, vtkMRMLSegmentationDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if displayNodeIt is already associated with a pipeline object.
  // This happens when a segmentation node already associated with a display node
  // is copied into an other (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator displayNodeIt;
  displayNodeIt = this->DisplayPipelines.find(displayNode);
  if (displayNodeIt != this->DisplayPipelines.end())
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
    vtkSegment* segment = segmentation->GetSegment(*segmentIdIt);
    if (!segment)
      {
      continue;
      }

    vtkDataObject* representation = segment->GetRepresentation(displayNode->GetDisplayRepresentationName2D());
    if (pipelineVector.find(representation) == pipelineVector.end())
      {
      pipelineVector[representation] = this->CreateSegmentPipeline();
      }
    }

  this->DisplayPipelines.insert( std::make_pair(displayNode, pipelineVector) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::Pipeline*
vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::CreateSegmentPipeline()
{
  Pipeline* pipeline = new Pipeline();

  // Add actors to Renderer
  this->External->GetRenderer()->AddActor( pipeline->PolyDataOutlineActor );
  this->External->GetRenderer()->AddActor( pipeline->PolyDataFillActor );
  this->External->GetRenderer()->AddActor( pipeline->ImageOutlineActor );
  this->External->GetRenderer()->AddActor( pipeline->ImageFillActor );

  return pipeline;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::UpdateDisplayNode(vtkMRMLSegmentationDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update. Otherwise, add as new node
  if (!displayNode)
    {
    return;
    }
  PipelinesCacheType::iterator displayNodeIt = this->DisplayPipelines.find(displayNode);
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
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::UpdateAllDisplayNodesForSegment(vtkMRMLSegmentationNode* segmentationNode)
{
  std::set<vtkMRMLSegmentationDisplayNode *> displayNodes = this->SegmentationToDisplayNodes[segmentationNode];
  for (std::set<vtkMRMLSegmentationDisplayNode *>::iterator dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++)
    {
    this->UpdateDisplayNode(*dnodesIter);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::UpdateSegmentPipelines(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType &pipelines)
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

  bool requestTransformUpdate = false;
  // Make sure each segment has a pipeline
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    vtkSegment* segment = segmentation->GetSegment(*segmentIdIt);
    vtkDataObject* representationObject = segment->GetRepresentation(displayNode->GetDisplayRepresentationName2D());

    // If segment does not have a pipeline, create one
    if (representationObject && pipelines.find(representationObject) == pipelines.end())
      {
      pipelines[representationObject] = this->CreateSegmentPipeline();
      requestTransformUpdate = true;
      }
    }

  // Make sure each pipeline belongs to an existing segment
  PipelineMapType::iterator pipelineIt = pipelines.begin();
  while (pipelineIt != pipelines.end())
    {
    Pipeline* pipeline = pipelineIt->second;
    vtkDataObject* dataObject = pipelineIt->first;
    bool displayObjectInSegment = false;
    for (int i = 0; i < segmentation->GetNumberOfSegments(); ++i)
      {
      vtkSegment* segment = segmentation->GetNthSegment(i);
      std::string displayRepresentation = displayNode->GetDisplayRepresentationName2D();
      vtkDataObject* displayObject = segment->GetRepresentation(displayRepresentation);
      if (dataObject && displayObject == dataObject)
        {
        displayObjectInSegment = true;
        break;
        }
      }

    if (!displayObjectInSegment)
      {
      PipelineMapType::iterator erasedIt = pipelineIt;
      ++pipelineIt;
      pipelines.erase(erasedIt);
      this->External->GetRenderer()->RemoveActor(pipeline->PolyDataOutlineActor);
      this->External->GetRenderer()->RemoveActor(pipeline->PolyDataFillActor);
      this->External->GetRenderer()->RemoveActor(pipeline->ImageOutlineActor);
      this->External->GetRenderer()->RemoveActor(pipeline->ImageFillActor);
      delete pipeline;
      }
    else
      {
      ++pipelineIt;
      }
    }

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  if (requestTransformUpdate)
    {
    this->UpdateDisplayableTransforms(segmentationNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::UpdateDisplayNodePipeline(vtkMRMLSegmentationDisplayNode* displayNode, PipelineMapType &pipelines)
{
  // Sets visibility, set pipeline polydata input, update color calculate and set pipeline segments.
  if (!displayNode)
    {
    return;
    }
  bool displayNodeVisible = this->IsVisible(displayNode);

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

  // Determine which representation to show
  std::string shownRepresenatationName = displayNode->GetDisplayRepresentationName2D();
  if (shownRepresenatationName.empty())
    {
    // Hide segmentation if there is no 2D representation to show
    for (PipelineMapType::iterator pipelineIt=pipelines.begin(); pipelineIt!=pipelines.end(); ++pipelineIt)
      {
      pipelineIt->second->PolyDataOutlineActor->SetVisibility(false);
      pipelineIt->second->PolyDataFillActor->SetVisibility(false);
      pipelineIt->second->ImageOutlineActor->SetVisibility(false);
      pipelineIt->second->ImageFillActor->SetVisibility(false);
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
  if (!segmentation->CreateRepresentation(shownRepresenatationName))
    {
    return;
    }

  // For all pipelines (pipeline per segment)
  for (PipelineMapType::iterator pipelineIt=pipelines.begin(); pipelineIt!=pipelines.end(); ++pipelineIt)
    {
    Pipeline* pipeline = pipelineIt->second;

    vtkDataObject* dataObject = pipelineIt->first;
    std::vector<std::string> sharedSegmentIds = segmentation->GetSegmentIDsForDataObject(dataObject, displayNode->GetDisplayRepresentationName2D());

    // Get representation to display
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(dataObject);
    vtkOrientedImageData* imageData = vtkOrientedImageData::SafeDownCast(dataObject);
    if (imageData)
      {
      int* imageExtent = imageData->GetExtent();
      if (imageExtent[0]>imageExtent[1] || imageExtent[2]>imageExtent[3] || imageExtent[4]>imageExtent[5])
        {
        // empty image
        imageData = nullptr;
        }
      }

    bool pipelineVisiblity = false;
    for (std::string segmentId : sharedSegmentIds)
      {
      pipelineVisiblity |= this->IsSegmentVisibleInCurrentSlice(displayNode, pipeline, segmentId);
      }

    if (!pipelineVisiblity)
      {
      pipeline->PolyDataOutlineActor->SetVisibility(false);
      pipeline->PolyDataFillActor->SetVisibility(false);
      pipeline->ImageOutlineActor->SetVisibility(false);
      pipeline->ImageFillActor->SetVisibility(false);
      continue;
      }

    // If shown representation is poly data
    if (polyData)
      {
      // Get visibility
      std::string segmentID = sharedSegmentIds[0];

      vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
      displayNode->GetSegmentDisplayProperties(segmentID, properties);

      double outlineOpacity = hierarchyOpacity * properties.Opacity2DOutline * displayNode->GetOpacity2DOutline() * genericDisplayNode->GetOpacity();
      bool segmentOutlineVisible = hierarchyVisibility && displayNodeVisible && properties.Visible &&
        properties.Visible2DOutline && displayNode->GetVisibility2DOutline() && (outlineOpacity > 0.0);
      double fillOpacity = hierarchyOpacity * properties.Opacity2DFill * displayNode->GetOpacity2DFill() * genericDisplayNode->GetOpacity();
      bool segmentFillVisible = hierarchyVisibility && displayNodeVisible && properties.Visible &&
        properties.Visible2DFill && displayNode->GetVisibility2DFill() && (fillOpacity > 0.0);

      // Turn off image visibility when showing poly data
      pipeline->ImageOutlineActor->SetVisibility(false);
      pipeline->ImageFillActor->SetVisibility(false);

      if ((!segmentOutlineVisible && !segmentFillVisible) || (!polyData || polyData->GetNumberOfPoints() == 0))
        {
        pipeline->PolyDataOutlineActor->SetVisibility(false);
        pipeline->PolyDataFillActor->SetVisibility(false);
        pipeline->ImageOutlineActor->SetVisibility(false);
        pipeline->ImageFillActor->SetVisibility(false);
        continue;
        }

      // Only update slice intersection if it has changed
      if ( pipeline->SliceIntersectionUpdatedTime < polyData->GetMTime()
        || pipeline->SliceIntersectionUpdatedTime < this->SliceXYToRAS->GetMTime() )
        {
        pipeline->ModelWarper->SetInputData(polyData);
        pipeline->ModelWarper->SetTransform(pipeline->NodeToWorldTransform);

        // Set Plane transform
        this->SetSlicePlaneFromMatrix(this->SliceXYToRAS, pipeline->Plane);
        pipeline->Plane->Modified();

        // Set PolyData transform
        vtkNew<vtkMatrix4x4> rasToSliceXY;
        vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY.GetPointer());
        pipeline->WorldToSliceTransform->SetMatrix(rasToSliceXY.GetPointer());

        if (segmentFillVisible)
          {
          // Save time of slice intersection update
          pipeline->SliceIntersectionUpdatedTime = (polyData->GetMTime() > this->SliceXYToRAS->GetMTime() ?
            polyData->GetMTime() : this->SliceXYToRAS->GetMTime());
          }
        }

      // Get displayed color (if no override is defined then use the color from the segment)
      double color[3] = { vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2] };
      if (overrideHierarchyDisplayNode)
        {
        overrideHierarchyDisplayNode->GetColor(color);
        }
      else
        {
        displayNode->GetSegmentColor(segmentID, color);
        }

      // Update pipeline actors
      pipeline->PolyDataOutlineActor->SetVisibility(segmentOutlineVisible);
      pipeline->PolyDataOutlineActor->GetProperty()->SetColor(color);
      pipeline->PolyDataOutlineActor->GetProperty()->SetOpacity(outlineOpacity);
      pipeline->PolyDataOutlineActor->GetProperty()->SetLineWidth(genericDisplayNode->GetSliceIntersectionThickness());
      pipeline->PolyDataOutlineActor->SetPosition(0,0);
      pipeline->PolyDataFillActor->SetVisibility(segmentFillVisible);
      pipeline->PolyDataFillActor->GetProperty()->SetColor(color[0], color[1], color[2]);
      pipeline->PolyDataFillActor->GetProperty()->SetOpacity(fillOpacity);
      pipeline->PolyDataFillActor->SetPosition(0,0);
      }
    // If shown representation is image data
    else if (imageData)
      {
      // Turn off poly data visibility when showing image
      pipeline->PolyDataOutlineActor->SetVisibility(false);
      pipeline->PolyDataFillActor->SetVisibility(false);

      bool outlineVisible = false;
      bool fillVisible = false;
      for (std::string segmentId : sharedSegmentIds)
        {
        vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
        displayNode->GetSegmentDisplayProperties(segmentId, properties);

        double outlineOpacity = properties.Opacity2DOutline * displayNode->GetOpacity2DOutline() * displayNode->GetOpacity();
        outlineVisible |= displayNodeVisible && properties.Visible
          && properties.Visible2DOutline && displayNode->GetVisibility2DOutline() && (outlineOpacity > 0.0);

        double fillOpacity = properties.Opacity2DFill * displayNode->GetOpacity2DFill() * displayNode->GetOpacity();
        fillVisible |= displayNodeVisible && properties.Visible
          && properties.Visible2DFill && displayNode->GetVisibility2DFill() && (fillOpacity > 0.0);

        if (outlineVisible && fillVisible)
          {
          break;
          }
        }

      // Update pipeline actors
      pipeline->ImageOutlineActor->SetVisibility(outlineVisible);
      pipeline->ImageOutlineActor->SetPosition(0, 0);
      pipeline->ImageFillActor->SetVisibility(fillVisible);
      pipeline->ImageFillActor->SetPosition(0, 0);

      if (!outlineVisible && !fillVisible)
        {
        continue;
        }

      // Set outline properties and turn it off if not shown
      if (outlineVisible)
        {
        pipeline->LabelOutline->SetOutline(genericDisplayNode->GetSliceIntersectionThickness());
        }
      else
        {
        pipeline->LabelOutline->SetInputConnection(nullptr);
        }

      // Set the range of the scalars in the image data from the ScalarRange field if it exists
      // Default to the scalar range of 0.0 to 1.0 otherwise
      double minimumValue = 0.0;
      double maximumValue = 1.0;
      vtkDoubleArray* scalarRange = vtkDoubleArray::SafeDownCast(
        imageData->GetFieldData()->GetAbstractArray(vtkSegmentationConverter::GetScalarRangeFieldName()));
      if (scalarRange && scalarRange->GetNumberOfValues() == 2)
        {
        minimumValue = scalarRange->GetValue(0);
        maximumValue = scalarRange->GetValue(1);
        }

      // Set segment color
      int minLabelmapValue = 0;
      int maxLabelmapValue = 0;

      for (std::string segmentId : sharedSegmentIds)
        {
        vtkSegment* segment = segmentation->GetSegment(segmentId);
        int labelmapValue = segment->GetLabelValue();
        minLabelmapValue = std::min(minLabelmapValue, labelmapValue);
        maxLabelmapValue = std::max(maxLabelmapValue, labelmapValue);
        }

      if (displayNode->GetDisplayRepresentationName2D() == vtkSegmentationConverter::GetFractionalLabelmapRepresentationName())
        {
        pipeline->LookupTableFill->SetNumberOfTableValues(maximumValue - minimumValue + 1);
        pipeline->LookupTableFill->SetTableRange(minimumValue, maximumValue);
        }
      else
        {
        int numberOfValues = maxLabelmapValue - minLabelmapValue + 1;
        pipeline->LookupTableOutline->SetNumberOfTableValues(numberOfValues);
        pipeline->LookupTableOutline->SetRange(minLabelmapValue, maxLabelmapValue);
        pipeline->LookupTableOutline->IndexedLookupOff();
        pipeline->LookupTableOutline->Build();

        pipeline->LookupTableFill->SetNumberOfTableValues(numberOfValues);
        pipeline->LookupTableFill->SetRange(minLabelmapValue, maxLabelmapValue);
        pipeline->LookupTableFill->IndexedLookupOff();
        pipeline->LookupTableFill->Build();

        int index = pipeline->LookupTableOutline->GetIndex(0.0);
        pipeline->LookupTableOutline->SetTableValue(index, 0, 0, 0, 0);
        index = pipeline->LookupTableFill->GetIndex(0.0);
        pipeline->LookupTableFill->SetTableValue(index, 0, 0, 0, 0);
        }

      for (std::string segmentId : sharedSegmentIds)
        {
        vtkSegment* segment = segmentation->GetSegment(segmentId);
        int labelmapValue = segment->GetLabelValue();

        // Get visibility
        vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
        displayNode->GetSegmentDisplayProperties(segmentId, properties);

        double outlineOpacity = hierarchyOpacity * properties.Opacity2DOutline * displayNode->GetOpacity2DOutline() * genericDisplayNode->GetOpacity();
        bool segmentOutlineVisible = displayNodeVisible && properties.Visible
          && properties.Visible2DOutline && displayNode->GetVisibility2DOutline() && (outlineOpacity > 0.0);
        if (!segmentOutlineVisible)
          {
          outlineOpacity = 0.0;
          }

        double fillOpacity = hierarchyOpacity * properties.Opacity2DFill * displayNode->GetOpacity2DFill() * genericDisplayNode->GetOpacity();
        bool segmentFillVisible = displayNodeVisible && properties.Visible
          && properties.Visible2DFill && displayNode->GetVisibility2DFill() && (fillOpacity > 0.0);
        if (!segmentFillVisible)
          {
          fillOpacity = 0.0;
          }

        // Get displayed color (if no override is defined then use the color from the segment)
        double color[4] = { vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2], 1.0};
        if (overrideHierarchyDisplayNode)
          {
          overrideHierarchyDisplayNode->GetColor(color);
          }
        else
          {
          displayNode->GetSegmentColor(segmentId, color);
          }

        if (displayNode->GetDisplayRepresentationName2D() == vtkSegmentationConverter::GetFractionalLabelmapRepresentationName())
          {
          pipeline->LookupTableFill->SetRampToLinear();
          if (!this->SmoothFractionalLabelMapBorder)
            {
            pipeline->LookupTableFill->SetNumberOfTableValues(2);
            }
          else
            {
            pipeline->LookupTableFill->SetNumberOfTableValues(maximumValue - minimumValue + 1);
            }
          double hsv[3] = { 0,0,0 };
          vtkMath::RGBToHSV(color, hsv);
          pipeline->LookupTableFill->SetHueRange(hsv[0], hsv[0]);
          pipeline->LookupTableFill->SetSaturationRange(hsv[1], hsv[1]);
          pipeline->LookupTableFill->SetValueRange(hsv[2], hsv[2]);
          pipeline->LookupTableFill->SetAlphaRange(0.0,
            hierarchyOpacity* properties.Opacity2DFill* displayNode->GetOpacity2DFill()* genericDisplayNode->GetOpacity());
          pipeline->LookupTableFill->SetTableRange(minimumValue, maximumValue);
          pipeline->LookupTableFill->ForceBuild();

          pipeline->LookupTableOutline->SetTableValue(0,color[0], color[1], color[2], 0.0);
          pipeline->LookupTableOutline->SetTableValue(1,
            color[0], color[1], color[2],
            hierarchyOpacity* properties.Opacity2DOutline* displayNode->GetOpacity2DOutline()* genericDisplayNode->GetOpacity());
          pipeline->LookupTableOutline->SetNumberOfTableValues(2);
          pipeline->LookupTableOutline->SetTableRange(0, 1);
          }
        else
          {
          int index = pipeline->LookupTableFill->GetIndex(labelmapValue);
          pipeline->LookupTableOutline->SetTableValue(index, color[0], color[1], color[2], outlineOpacity);
          index = pipeline->LookupTableFill->GetIndex(labelmapValue);
          pipeline->LookupTableFill->SetTableValue(index, color[0], color[1], color[2], fillOpacity);
          }
        }
      pipeline->Reslice->SetBackgroundLevel(minimumValue);

      // Calculate image IJK to world RAS transform
      pipeline->SliceToImageTransform->Identity();
      pipeline->SliceToImageTransform->Concatenate(this->SliceXYToRAS);
      pipeline->SliceToImageTransform->Concatenate(pipeline->WorldToNodeTransform);
      vtkSmartPointer<vtkMatrix4x4> worldToImageMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      imageData->GetWorldToImageMatrix(worldToImageMatrix);
      pipeline->SliceToImageTransform->Concatenate(worldToImageMatrix);

      // Create temporary copy of the segment image with default origin and spacing
      vtkSmartPointer<vtkImageData> identityImageData = vtkSmartPointer<vtkImageData>::New();
      identityImageData->ShallowCopy(imageData);
      identityImageData->SetOrigin(0.0, 0.0, 0.0);
      identityImageData->SetSpacing(1.0, 1.0, 1.0);

      // Set Reslice transform
      // vtkImageReslice works faster if the input is a linear transform, so try to convert it
      // to a linear transform.
      // Also attempt to make it a permute transform, as it makes reslicing even faster.
      vtkSmartPointer<vtkTransform> linearSliceToImageTransform = vtkSmartPointer<vtkTransform>::New();
      if (vtkMRMLTransformNode::IsGeneralTransformLinear(pipeline->SliceToImageTransform, linearSliceToImageTransform))
        {
        SnapToPermuteMatrix(linearSliceToImageTransform);
        pipeline->Reslice->SetResliceTransform(linearSliceToImageTransform);
        }
      else
        {
        pipeline->Reslice->SetResliceTransform(pipeline->SliceToImageTransform);
        }

      // Set the interpolation mode from the InterpolationType field if it exists
      // Default to nearest neighbor interpolation otherwise
      pipeline->Reslice->SetInterpolationModeToNearestNeighbor();
      vtkIntArray* interpolationType = vtkIntArray::SafeDownCast(
        imageData->GetFieldData()->GetAbstractArray(vtkSegmentationConverter::GetInterpolationTypeFieldName()));
      if (interpolationType && interpolationType->GetNumberOfValues() == 1)
        {
        pipeline->Reslice->SetInterpolationMode(interpolationType->GetValue(0));
        }
      else if (scalarRange && scalarRange->GetNumberOfValues() == 2)
        {
        pipeline->Reslice->SetInterpolationMode(this->DefaultFractionalInterpolationType);
        }

      pipeline->Reslice->SetInputData(identityImageData);

      int dimensions[3] = { 0, 0, 0 };
      this->SliceNode->GetDimensions(dimensions);
      int sliceOutputExtent[6] = { 0, dimensions[0] - 1, 0, dimensions[1] - 1, 0, dimensions[2] - 1 };
      pipeline->Reslice->SetOutputExtent(sliceOutputExtent);

      // Smooth the border of fractional labelmaps
      pipeline->LabelOutline->SetInputConnection(pipeline->Reslice->GetOutputPort());
      pipeline->ImageFillActor->GetMapper()->GetInputAlgorithm()->SetInputConnection(pipeline->Reslice->GetOutputPort());
      if (shownRepresenatationName == vtkSegmentationConverter::GetSegmentationFractionalLabelmapRepresentationName())
        {
        // If ThresholdValue is not specified, then do not perform thresholding
        vtkDoubleArray* thresholdValue = vtkDoubleArray::SafeDownCast(
          imageData->GetFieldData()->GetAbstractArray(vtkSegmentationConverter::GetThresholdValueFieldName()));
        if (thresholdValue && thresholdValue->GetNumberOfValues() == 1)
          {
          if (!this->SmoothFractionalLabelMapBorder && thresholdValue && thresholdValue->GetNumberOfValues() == 1)
            {
            pipeline->ImageFillActor->GetMapper()->GetInputAlgorithm()->SetInputConnection(pipeline->ImageThreshold->GetOutputPort());
            }
          pipeline->ImageThreshold->ThresholdByLower(thresholdValue->GetValue(0));
          pipeline->LabelOutline->SetInputConnection(pipeline->ImageThreshold->GetOutputPort());
          }
        }
      }
    else
      {
      // Display representation object is not available.
      pipeline->PolyDataOutlineActor->SetVisibility(false);
      pipeline->PolyDataFillActor->SetVisibility(false);
      pipeline->ImageOutlineActor->SetVisibility(false);
      pipeline->ImageFillActor->SetVisibility(false);
      continue;
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::AddObservations(vtkMRMLSegmentationNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
    }
  if (!broker->GetObservationExist(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkSegmentation::RepresentationModified, this->External, this->External->GetMRMLNodesCallbackCommand() );
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
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::RemoveObservations(vtkMRMLSegmentationNode* node)
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
  observations = broker->GetObservations(node, vtkSegmentation::SegmentAdded, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::SegmentRemoved, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkSegmentation::SegmentModified, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::IsNodeObserved(vtkMRMLSegmentationNode* node)
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
void vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::ClearDisplayableNodes()
{
  while(this->SegmentationToDisplayNodes.size() > 0)
    {
    this->RemoveSegmentationNode(this->SegmentationToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::UseDisplayableNode(vtkMRMLSegmentationNode* node)
{
  bool use = node && node->IsA("vtkMRMLSegmentationNode");
  return use;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationsDisplayableManager2D::vtkInternal::IsSegmentVisibleInCurrentSlice(
  vtkMRMLSegmentationDisplayNode* displayNode, Pipeline* pipeline, const std::string &segmentID)
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    displayNode->GetDisplayableNode() );
  if (!segmentationNode)
    {
    return false;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return false;
    }

  double segmentBounds_Segment[6] = { 0 };
  vtkSegment* segment = segmentation->GetSegment(segmentID);
  if (!segment)
    {
    return false;
    }

  if (displayNode->GetDisplayRepresentationName2D() == vtkSegmentationConverter::GetClosedSurfaceRepresentationName())
    {
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(segment->GetRepresentation(displayNode->GetDisplayRepresentationName2D()));
    polyData->GetBounds(segmentBounds_Segment);
    }
  else if (displayNode->GetDisplayRepresentationName2D() == vtkSegmentationConverter::GetBinaryLabelmapRepresentationName() ||
    displayNode->GetDisplayRepresentationName2D() == vtkSegmentationConverter::GetFractionalLabelmapRepresentationName())
    {
    vtkOrientedImageData* imageData = vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(displayNode->GetDisplayRepresentationName2D()));
    imageData->GetBounds(segmentBounds_Segment);
    }
  else
    {
    segment->GetBounds(segmentBounds_Segment);
    }

  vtkSmartPointer<vtkGeneralTransform> segmentationToSliceTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  vtkNew<vtkMatrix4x4> rasToSliceXY;
  vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY.GetPointer());
  segmentationToSliceTransform->Concatenate(rasToSliceXY.GetPointer());
  segmentationToSliceTransform->Concatenate(pipeline->NodeToWorldTransform);

  double segmentBounds_Slice[6] = { 0 };
  vtkOrientedImageDataResample::TransformBounds(segmentBounds_Segment, segmentationToSliceTransform, segmentBounds_Slice);

  bool visibleInCurrentSlice = true;

  // if segment does not intersect the slice plane then hide actors
  const double slicePositionTolerance = 0.1;
  if ((segmentBounds_Slice[4]<-slicePositionTolerance && segmentBounds_Slice[5]<-slicePositionTolerance)
    || (segmentBounds_Slice[4]>slicePositionTolerance && segmentBounds_Slice[5]>slicePositionTolerance))
    {
    visibleInCurrentSlice = false;
    }
  else
    {
    int outlineWidth = displayNode->GetSliceIntersectionThickness();
      for (int i=0; i<3; i++)
        {
        int startExtent = int(floor(segmentBounds_Slice[i * 2]) - outlineWidth);
        int endExtent = int(ceil(segmentBounds_Slice[i * 2 + 1]) + outlineWidth);
        if (segmentBounds_Slice[i * 2] < startExtent)
          {
          segmentBounds_Slice[i * 2] = startExtent;
          }
        if (segmentBounds_Slice[i * 2 + 1] > endExtent)
          {
          segmentBounds_Slice[i * 2 + 1] = endExtent;
          }
        }
      if (segmentBounds_Slice[0]>segmentBounds_Slice[1]
        || segmentBounds_Slice[2]>segmentBounds_Slice[3])
        {
        visibleInCurrentSlice = false;
        }
    }

  return visibleInCurrentSlice;
}


//---------------------------------------------------------------------------
// vtkMRMLSegmentationsDisplayableManager2D methods

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager2D::vtkMRMLSegmentationsDisplayableManager2D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLSegmentationsDisplayableManager2D::~vtkMRMLSegmentationsDisplayableManager2D()
{
  delete this->Internal;
  this->Internal = nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkMRMLSegmentationsDisplayableManager2D: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if ( !node->IsA("vtkMRMLSegmentationNode") )
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromMRMLRequested(true);
    return;
    }

  this->Internal->AddSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
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
void vtkMRMLSegmentationsDisplayableManager2D::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
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
           || (event == vtkSegmentation::RepresentationModified))
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
      }
    else if ( (event == vtkCommand::ModifiedEvent) // segmentation object may be replaced
           || (event == vtkSegmentation::SegmentAdded)
           || (event == vtkSegmentation::SegmentModified)
           || (event == vtkSegmentation::SegmentRemoved) )
      {
      this->Internal->UpdateAllDisplayNodesForSegment(displayableNode);
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
void vtkMRMLSegmentationsDisplayableManager2D::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro("vtkMRMLSegmentationsDisplayableManager2D->UpdateFromMRML: Scene is not set.");
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLSegmentationNode* mNode = nullptr;
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
void vtkMRMLSegmentationsDisplayableManager2D::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::Create()
{
  this->Internal->SetSliceNode(this->GetMRMLSliceNode());
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
std::string vtkMRMLSegmentationsDisplayableManager2D::GetDataProbeInfoStringForPosition(double xyz[3])
{
  std::string segmentsAtPositionInfoStr("");

  if (this->Internal->DisplayPipelines.size() == 0)
    {
    return "";
    }

  // Convert from slice view XYZ to RAS coordinates
  double xyzw[4] = {xyz[0], xyz[1], xyz[2], 1.0 };
  double rasw[4] = {0.0, 0.0, 0.0, 1.0};
  this->GetMRMLSliceNode()->GetXYToRAS()->MultiplyPoint(xyzw, rasw);
  double ras[3] = { rasw[0], rasw[1], rasw[2] };

  bool firstSegmentationNode = true;
  vtkInternal::PipelinesCacheType::iterator displayNodeIt;
  for (displayNodeIt = this->Internal->DisplayPipelines.begin(); displayNodeIt != this->Internal->DisplayPipelines.end(); ++displayNodeIt)
    {
    vtkMRMLSegmentationDisplayNode* displayNode = displayNodeIt->first;
    if (!displayNode)
      {
      continue;
      }

    vtkNew<vtkStringArray> segmentIDs;
    vtkSmartPointer<vtkDoubleArray> segmentValues;
    std::string shownRepresenatationName = displayNode->GetDisplayRepresentationName2D();
    if (shownRepresenatationName == vtkSegmentationConverter::GetSegmentationFractionalLabelmapRepresentationName())
      {
      segmentValues = vtkSmartPointer<vtkDoubleArray>::New();
      }
    this->GetVisibleSegmentsForPosition(ras, displayNode, segmentIDs.GetPointer(), segmentValues.GetPointer());

    if (segmentIDs->GetNumberOfValues() == 0)
      {
      continue;
      }

    vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
      displayNode->GetDisplayableNode());
    if (!segmentationNode)
      {
      continue;
      }

    // Assemble info string for current segmentation
    if (firstSegmentationNode)
      {
      firstSegmentationNode = false;
      }
    else
      {
      segmentsAtPositionInfoStr.append(" "); // separate segmentations
      }
    segmentsAtPositionInfoStr.append( "<b>" + std::string(segmentationNode->GetName()) + ":</b> " );
    std::string segmentsInfoStr;
    for (int segmentIdIndex = 0; segmentIdIndex < segmentIDs->GetNumberOfValues(); ++segmentIdIndex)
      {
      const char* segmentId = segmentIDs->GetValue(segmentIdIndex);
      vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
      if (segment)
        {
        // Add color indicator
        double color[3] = {vtkSegment::SEGMENT_COLOR_INVALID[0], vtkSegment::SEGMENT_COLOR_INVALID[1], vtkSegment::SEGMENT_COLOR_INVALID[2]};
        displayNode->GetSegmentColor(segmentId, color);
        std::stringstream colorStream;
        colorStream << "#" << std::hex << std::setfill('0')
            << std::setw(2) << (int)(color[0] * 255.0)
            << std::setw(2) << (int)(color[1] * 255.0)
            << std::setw(2) << (int)(color[2] * 255.0);
        segmentsInfoStr.append("<font color=\"" + colorStream.str() + "\">&#x25cf;</font>");

        segmentsInfoStr.append(segment->GetName() ? segment->GetName() : "");

        // If the segmentation representation is a fractional labelmap then display the fill percentage
        if (segmentValues.GetPointer() != nullptr)
          {
          // Get the minimum and maximum scalar values from the fractional labelmap (default to 0.0 and 1.0)
          vtkOrientedImageData* imageData = vtkOrientedImageData::SafeDownCast(
            segment->GetRepresentation(shownRepresenatationName));
          if (imageData)
            {
            vtkDoubleArray* scalarRange = vtkDoubleArray::SafeDownCast(
              imageData->GetFieldData()->GetAbstractArray(vtkSegmentationConverter::GetScalarRangeFieldName()));
            double minimumValue = 0.0;
            double maximumValue = 1.0;
            if (scalarRange && scalarRange->GetNumberOfValues() == 2)
            {
              minimumValue = scalarRange->GetValue(0);
              maximumValue = scalarRange->GetValue(1);
            }

            // Calculate the voxel fill percent based on the maximum and minimum values.
            double segmentVoxelFillPercent = 100 * (segmentValues->GetValue(segmentIdIndex) - minimumValue) / (maximumValue - minimumValue);
            std::stringstream percentStream;
            percentStream << " " << std::fixed << std::setprecision(1) << segmentVoxelFillPercent << "%";
            segmentsInfoStr.append(percentStream.str());
            }
          }

        segmentsInfoStr.append(" ");
        }
      }
    segmentsAtPositionInfoStr.append( segmentsInfoStr.substr(0, segmentsInfoStr.size()-1) );

    } // For each display node

  if (segmentsAtPositionInfoStr.empty())
    {
    return "";
    }
  return "S " + segmentsAtPositionInfoStr;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationsDisplayableManager2D::GetVisibleSegmentsForPosition(double ras[3],
  vtkMRMLSegmentationDisplayNode* displayNode, vtkStringArray* segmentIDs, vtkDoubleArray* segmentValues/*=nullptr*/)
{
  if (!segmentIDs)
    {
    return;
    }
  segmentIDs->Reset();
  if (segmentValues)
    {
    segmentValues->Reset();
    }

  vtkInternal::PipelinesCacheType::iterator pipelinesIter = this->Internal->DisplayPipelines.find(displayNode);
  if (pipelinesIter == this->Internal->DisplayPipelines.end())
    {
    return;
    }

  bool displayNodeVisible = this->Internal->IsVisible(displayNode);
  if (!displayNodeVisible)
    {
    return;
    }

  // Get segmentation
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    displayNode->GetDisplayableNode());
  if (!segmentationNode)
    {
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }

  // Get name of displayed representation
  std::string shownRepresenatationName = displayNode->GetDisplayRepresentationName2D();

  // For all pipelines (pipeline per segment)
  std::set<std::string> segmentIDsAtPosition;
  std::map<std::string, double> valueForSegment;

  for (vtkInternal::PipelineMapType::iterator pipelineIt = pipelinesIter->second.begin(); pipelineIt != pipelinesIter->second.end(); ++pipelineIt)
    {
    vtkInternal::Pipeline* pipeline = pipelineIt->second;

    // Get displayed representation
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(pipelineIt->first);
    vtkOrientedImageData* imageData = vtkOrientedImageData::SafeDownCast(pipelineIt->first);
    if (imageData)
      {
      int* imageExtent = imageData->GetExtent();
      if (imageExtent[0]>imageExtent[1] || imageExtent[2]>imageExtent[3] || imageExtent[4]>imageExtent[5])
        {
        continue;
        }

      // Decide if point is in segment (image data)
      vtkNew<vtkMatrix4x4> segmentationToOrientedImageIjkMatrix;
      imageData->GetWorldToImageMatrix(segmentationToOrientedImageIjkMatrix.GetPointer());
      vtkNew<vtkGeneralTransform> worldToOrientedImageIjkToTransform;
      worldToOrientedImageIjkToTransform->Concatenate(segmentationToOrientedImageIjkMatrix.GetPointer());
      worldToOrientedImageIjkToTransform->Concatenate(pipeline->WorldToNodeTransform);
      double* ijkDouble = worldToOrientedImageIjkToTransform->TransformPoint(ras);
      int ijk[3] = { (int)(ijkDouble[0] + 0.5), (int)(ijkDouble[1] + 0.5), (int)(ijkDouble[2] + 0.5) };
      if (ijk[0] < imageExtent[0] || ijk[0] > imageExtent[1] ||
        ijk[1] < imageExtent[2] || ijk[1] > imageExtent[3] ||
        ijk[2] < imageExtent[4] || ijk[2] > imageExtent[5])
        {
        continue;
        }
      double voxelValue = imageData->GetScalarComponentAsDouble(
        ijk[0], ijk[1], ijk[2], 0);

      vtkDoubleArray* scalarRange = vtkDoubleArray::SafeDownCast(
        imageData->GetFieldData()->GetAbstractArray(vtkSegmentationConverter::GetScalarRangeFieldName()));

      double minimumValue = 0.0;
      if (scalarRange && scalarRange->GetNumberOfValues() == 2)
        {
        minimumValue = scalarRange->GetValue(0);
        }

      if (voxelValue <= minimumValue)
        {
        continue;
        }

      for (int i = 0; i < segmentation->GetNumberOfSegments(); ++i)
        {
        vtkSegment* segment = segmentation->GetNthSegment(i);
        std::string segmentID = segmentation->GetSegmentIdBySegment(segment);

        // Skip if segment is not visible in the current slice
        if (!this->Internal->IsSegmentVisibleInCurrentSlice(displayNode, pipeline, segmentID))
          {
          continue;
          }

        // Get visibility
        vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
        displayNode->GetSegmentDisplayProperties(segmentID, properties);
        bool segmentVisible = displayNodeVisible && properties.Visible
          && (properties.Visible2DOutline || properties.Visible2DFill);
        if (!segmentVisible)
          {
          continue;
          }

        int labelmapValue = segment->GetLabelValue();
        if ((shownRepresenatationName == vtkSegmentationConverter::GetBinaryLabelmapRepresentationName() && voxelValue != labelmapValue) ||
          segment->GetRepresentation(shownRepresenatationName) != imageData)
          {
          continue;
          }
        segmentIDsAtPosition.insert(segmentID);

        if (shownRepresenatationName == vtkSegmentationConverter::GetFractionalLabelmapRepresentationName())
          {
          valueForSegment.insert(std::make_pair(segmentID, voxelValue));
          }
        }
      }
    else if (polyData)
      {
      if (polyData->GetNumberOfPoints() == 0)
        {
        continue;
        }

      // Use poly data that is displayed in the slice view
      vtkPolyData* sliceFillPolyData = pipeline->Triangulator->GetOutput();
      if (!sliceFillPolyData)
        {
        continue;
        }
      double tolerance = 0.0001;
      int subId = -1;
      double dist2 = 0.0;
      double pcoords[3] = { 0.0, 0.0, 0.0 };
      double* weights = new double[sliceFillPolyData->GetMaxCellSize()];
      for (int index = 0; index<sliceFillPolyData->GetNumberOfCells(); ++index)
        {
        vtkCell* cell = sliceFillPolyData->GetCell(index);
        // If out of bounds, then do not investigate this cell further
        double* bounds = cell->GetBounds();
        if (ras[0]<bounds[0] - tolerance || ras[0]>bounds[1] + tolerance ||
          ras[1]<bounds[2] - tolerance || ras[1]>bounds[3] + tolerance ||
          ras[2]<bounds[4] - tolerance || ras[2]>bounds[5] + tolerance)
          {
          continue;
          }
        // Inside bounds the position is evaluated in the cell
        if (cell->EvaluatePosition(ras, nullptr, subId, pcoords, dist2, weights) == 1)
          {
          std::string segmentID;
          for (int i = 0; i < segmentation->GetNumberOfSegments(); ++i)
            {
            vtkSegment* segment = segmentation->GetNthSegment(i);
            if (pipelineIt->first == segment->GetRepresentation(displayNode->GetDisplayRepresentationName2D()))
              {
              segmentID = segmentation->GetSegmentIdBySegment(segment);
              break;
              }
            }
          if (!segmentID.empty())
            {
            segmentIDsAtPosition.insert(segmentID);
            }
          break;
          }
        }
      }
    } // For each pipeline (=segment)

  for (std::set<std::string>::iterator segmentIt = segmentIDsAtPosition.begin(); segmentIt != segmentIDsAtPosition.end(); ++segmentIt)
    {
    segmentIDs->InsertNextValue(*segmentIt);
    if (segmentValues)
      {
      segmentValues->InsertNextValue(valueForSegment[*segmentIt]);
      }
    }
}
