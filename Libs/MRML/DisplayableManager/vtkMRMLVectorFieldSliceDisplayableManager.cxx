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

  This file was further developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLVectorFieldSliceDisplayableManager.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorFieldPipeline.h>
#include <vtkMRMLVectorFieldMask.h>
#include <vtkMRMLVectorFieldSamplePointSource.h>
#include <vtkVectorFieldGeometry.h>
#include <vtkVectorFieldGlyph.h>
#include <vtkVectorFieldSampler.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkArrayCalculator.h>
#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkDataArray.h>
#include <vtkGeneralTransform.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPlaneCutter.h>
#include <vtkPointData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSampleImplicitFunctionFilter.h>
#include <vtkScalarsToColors.h>
#include <vtkSmartPointer.h>
#include <vtkThresholdPoints.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkUnsignedCharArray.h>

// STD includes
#include <algorithm>
#include <cmath>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVectorFieldSliceDisplayableManager);

namespace
{
/// The events of a displayable node that change what is drawn
const unsigned long ObservedEvents[] = {
  vtkMRMLDisplayableNode::TransformModifiedEvent,
  vtkMRMLDisplayableNode::DisplayModifiedEvent,
  vtkMRMLModelNode::PolyDataModifiedEvent,
  vtkMRMLVolumeNode::ImageDataModifiedEvent,
};

//---------------------------------------------------------------------------
/// True if the display node names an array to color the field by
bool HasRenderedActiveScalar(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  const char* activeScalarName = displayNode->GetRenderedActiveScalarName();
  return (activeScalarName && activeScalarName[0] != '\0');
}

//---------------------------------------------------------------------------
/// Copy of the color node's table (or a default table if there is none), so that setting range, alpha
/// and vector mode does not affect other mappers sharing the color node's table.
vtkSmartPointer<vtkLookupTable> CreateLookupTable(vtkMRMLColorNode* colorNode)
{
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::Take(colorNode ? colorNode->CreateLookupTableCopy() : nullptr);
  if (!lookupTable)
  {
    lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->Build();
  }
  return lookupTable;
}

/// Name of the point data array that stores the value that a multi-component array colors by
const char* ColorMagnitudeArrayName = "GlyphColorMagnitude";
/// Name of the point data array that stores the signed distance of each mesh point from the slice plane,
/// used for selecting the points within the displayed slab.
const char* SliceDistanceArrayName = "GlyphSliceDistance";

//---------------------------------------------------------------------------
/// \brief Keeps the 3D streamlines that cross (or come within HalfThickness of) a slice plane, fading with distance from it.
///
/// Points are colored as in 3D views (scalar through a lookup table, or solid color), with opacity falling from full at
/// the plane to zero at FadeDistance. Output active scalars are RGBA unsigned char colors, so a mapper draws them as is.
class vtkStreamlineSliceProjection : public vtkPolyDataAlgorithm
{
public:
  static vtkStreamlineSliceProjection* New();
  vtkTypeMacro(vtkStreamlineSliceProjection, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /// The slice plane, in the coordinate system of the input points.
  vtkSetVector3Macro(PlaneOrigin, double);
  vtkGetVector3Macro(PlaneOrigin, double);
  vtkSetVector3Macro(PlaneNormal, double);
  vtkGetVector3Macro(PlaneNormal, double);
  ///@}

  ///@{
  /// Distance from the plane at which a streamline has faded out completely, in mm.
  /// Default is 20.
  vtkSetClampMacro(FadeDistance, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(FadeDistance, double);
  ///@}

  ///@{
  /// Half the slice thickness, in mm: a streamline this close to the plane counts as crossing it.
  /// Default is 0.5.
  vtkSetClampMacro(HalfThickness, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(HalfThickness, double);
  ///@}

  ///@{
  /// Point data array that colors the streamlines (ColorArrayComponent of it if multi-component).
  /// The solid color is used if not set or not found.
  vtkSetStringMacro(ColorArrayName);
  vtkGetStringMacro(ColorArrayName);
  ///@}

  ///@{
  /// Component of a multi-component color array that colors the streamlines, -1 for its
  /// magnitude (also used for a component that the array does not have). Default is -1.
  vtkSetMacro(ColorArrayComponent, int);
  vtkGetMacro(ColorArrayComponent, int);
  ///@}

  ///@{
  /// Lookup table and the range of values that it is mapped across, for coloring by the array.
  virtual void SetLookupTable(vtkScalarsToColors* lookupTable);
  vtkScalarsToColors* GetLookupTable();
  vtkSetVector2Macro(ScalarRange, double);
  vtkGetVector2Macro(ScalarRange, double);
  ///@}

  ///@{
  /// Color of the streamlines when they are not colored by an array. Default is white.
  vtkSetVector3Macro(SolidColor, double);
  vtkGetVector3Macro(SolidColor, double);
  ///@}

  /// Name of the point data array of colors in the output.
  static const char* GetColorsArrayName();

protected:
  vtkStreamlineSliceProjection();
  ~vtkStreamlineSliceProjection() override;
  vtkStreamlineSliceProjection(const vtkStreamlineSliceProjection&) = delete;
  void operator=(const vtkStreamlineSliceProjection&) = delete;

  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;
  vtkMTimeType GetMTime() override;

  double PlaneOrigin[3];
  double PlaneNormal[3];
  double FadeDistance{ 20.0 };
  double HalfThickness{ 0.5 };
  char* ColorArrayName{ nullptr };
  int ColorArrayComponent{ -1 };
  vtkSmartPointer<vtkScalarsToColors> LookupTable;
  double ScalarRange[2];
  double SolidColor[3];
};

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkStreamlineSliceProjection);

//---------------------------------------------------------------------------
vtkStreamlineSliceProjection::vtkStreamlineSliceProjection()
{
  this->PlaneOrigin[0] = this->PlaneOrigin[1] = this->PlaneOrigin[2] = 0.0;
  this->PlaneNormal[0] = this->PlaneNormal[1] = 0.0;
  this->PlaneNormal[2] = 1.0;
  this->ScalarRange[0] = 0.0;
  this->ScalarRange[1] = 1.0;
  this->SolidColor[0] = this->SolidColor[1] = this->SolidColor[2] = 1.0;
}

//---------------------------------------------------------------------------
vtkStreamlineSliceProjection::~vtkStreamlineSliceProjection()
{
  this->SetColorArrayName(nullptr);
}

//---------------------------------------------------------------------------
void vtkStreamlineSliceProjection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PlaneOrigin: " << this->PlaneOrigin[0] << " " << this->PlaneOrigin[1] << " " << this->PlaneOrigin[2] << "\n";
  os << indent << "PlaneNormal: " << this->PlaneNormal[0] << " " << this->PlaneNormal[1] << " " << this->PlaneNormal[2] << "\n";
  os << indent << "FadeDistance: " << this->FadeDistance << "\n";
  os << indent << "HalfThickness: " << this->HalfThickness << "\n";
  os << indent << "ColorArrayName: " << (this->ColorArrayName ? this->ColorArrayName : "(none)") << "\n";
  os << indent << "ColorArrayComponent: " << this->ColorArrayComponent << "\n";
  os << indent << "ScalarRange: " << this->ScalarRange[0] << " " << this->ScalarRange[1] << "\n";
}

//---------------------------------------------------------------------------
const char* vtkStreamlineSliceProjection::GetColorsArrayName()
{
  return "StreamlineSliceColors";
}

//---------------------------------------------------------------------------
void vtkStreamlineSliceProjection::SetLookupTable(vtkScalarsToColors* lookupTable)
{
  if (this->LookupTable == lookupTable)
  {
    return;
  }
  this->LookupTable = lookupTable;
  this->Modified();
}

//---------------------------------------------------------------------------
vtkScalarsToColors* vtkStreamlineSliceProjection::GetLookupTable()
{
  return this->LookupTable;
}

//---------------------------------------------------------------------------
vtkMTimeType vtkStreamlineSliceProjection::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  if (this->LookupTable)
  {
    mTime = std::max(mTime, this->LookupTable->GetMTime());
  }
  return mTime;
}

//---------------------------------------------------------------------------
int vtkStreamlineSliceProjection::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0]);
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  if (!input || !output)
  {
    return 0;
  }
  output->Initialize();
  vtkPoints* points = input->GetPoints();
  vtkCellArray* lines = input->GetLines();
  if (!points || !lines || lines->GetNumberOfCells() == 0)
  {
    return 1;
  }

  double normal[3] = { this->PlaneNormal[0], this->PlaneNormal[1], this->PlaneNormal[2] };
  if (vtkMath::Normalize(normal) <= 0.0)
  {
    return 1;
  }
  // Signed distance of each point from the plane
  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  std::vector<double> distances(numberOfPoints, 0.0);
  for (vtkIdType pointIndex = 0; pointIndex < numberOfPoints; ++pointIndex)
  {
    double point[3] = { 0.0, 0.0, 0.0 };
    points->GetPoint(pointIndex, point);
    double offset[3] = { point[0] - this->PlaneOrigin[0], point[1] - this->PlaneOrigin[1], point[2] - this->PlaneOrigin[2] };
    distances[pointIndex] = vtkMath::Dot(offset, normal);
  }

  // The streamlines that cross the plane, or come within half a slice thickness of it
  vtkNew<vtkCellArray> crossingLines;
  std::vector<bool> pointUsed(numberOfPoints, false);
  auto lineIterator = vtk::TakeSmartPointer(lines->NewIterator());
  for (lineIterator->GoToFirstCell(); !lineIterator->IsDoneWithTraversal(); lineIterator->GoToNextCell())
  {
    vtkIdType numberOfLinePoints = 0;
    const vtkIdType* linePointIds = nullptr;
    lineIterator->GetCurrentCell(numberOfLinePoints, linePointIds);
    bool crosses = false;
    for (vtkIdType linePointIndex = 0; linePointIndex < numberOfLinePoints && !crosses; ++linePointIndex)
    {
      double distance = distances[linePointIds[linePointIndex]];
      if (std::abs(distance) <= this->HalfThickness)
      {
        crosses = true;
      }
      else if (linePointIndex > 0 && (distance > 0.0) != (distances[linePointIds[linePointIndex - 1]] > 0.0))
      {
        crosses = true;
      }
    }
    if (!crosses)
    {
      continue;
    }
    crossingLines->InsertNextCell(numberOfLinePoints, linePointIds);
    for (vtkIdType linePointIndex = 0; linePointIndex < numberOfLinePoints; ++linePointIndex)
    {
      pointUsed[linePointIds[linePointIndex]] = true;
    }
  }
  if (crossingLines->GetNumberOfCells() == 0)
  {
    return 1;
  }

  // Colors: the array through the lookup table, or the solid color; the alpha is the opacity
  // that falls off with the distance from the plane
  vtkDataArray* colorArray = (this->ColorArrayName && this->ColorArrayName[0] != '\0') ? input->GetPointData()->GetArray(this->ColorArrayName) : nullptr;
  bool colorByArray = (colorArray && this->LookupTable);
  if (colorByArray)
  {
    this->LookupTable->SetRange(this->ScalarRange[0], this->ScalarRange[1]);
  }
  vtkNew<vtkUnsignedCharArray> colors;
  colors->SetName(vtkStreamlineSliceProjection::GetColorsArrayName());
  colors->SetNumberOfComponents(4);
  colors->SetNumberOfTuples(numberOfPoints);
  for (vtkIdType pointIndex = 0; pointIndex < numberOfPoints; ++pointIndex)
  {
    if (!pointUsed[pointIndex])
    {
      colors->SetTuple4(pointIndex, 0, 0, 0, 0);
      continue;
    }
    double rgb[3] = { this->SolidColor[0], this->SolidColor[1], this->SolidColor[2] };
    if (colorByArray)
    {
      double value = 0.0;
      if (colorArray->GetNumberOfComponents() > 1 && this->ColorArrayComponent >= 0 && this->ColorArrayComponent < colorArray->GetNumberOfComponents())
      {
        value = colorArray->GetComponent(pointIndex, this->ColorArrayComponent);
      }
      else if (colorArray->GetNumberOfComponents() > 1)
      {
        for (int component = 0; component < colorArray->GetNumberOfComponents(); ++component)
        {
          double componentValue = colorArray->GetComponent(pointIndex, component);
          value += componentValue * componentValue;
        }
        value = std::sqrt(value);
      }
      else
      {
        value = colorArray->GetComponent(pointIndex, 0);
      }
      this->LookupTable->GetColor(value, rgb);
    }
    double fade = 1.0;
    if (this->FadeDistance > 0.0)
    {
      fade = std::max(0.0, 1.0 - std::abs(distances[pointIndex]) / this->FadeDistance);
    }
    else if (std::abs(distances[pointIndex]) > this->HalfThickness)
    {
      fade = 0.0;
    }
    colors->SetTuple4(pointIndex, 255.0 * rgb[0], 255.0 * rgb[1], 255.0 * rgb[2], 255.0 * fade);
  }

  output->SetPoints(points);
  output->SetLines(crossingLines);
  output->GetPointData()->PassData(input->GetPointData());
  output->GetPointData()->AddArray(colors);
  output->GetPointData()->SetActiveScalars(colors->GetName());
  return 1;
}

} // namespace

//---------------------------------------------------------------------------
class vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal
{
public:
  struct Pipeline
  {
    /// Resamples the field in this slice plane, for resampleable sources (volumes, transforms). Owned here,
    /// not by the display node, because the display node can be shown in several slice views.
    vtkSmartPointer<vtkVectorFieldSampler> SliceSampler;
    /// Cuts a mesh with the slice plane, so that isolines come from the mesh itself
    vtkSmartPointer<vtkPlaneCutter> MeshCutter;
    /// Geometry of the grid and contour modes, and flat glyphs, in this slice view
    vtkSmartPointer<vtkVectorFieldGeometry> Geometry;
    vtkSmartPointer<vtkVectorFieldGlyph> Glyph;
    vtkSmartPointer<vtkGeneralTransform> NodeToWorld;
    vtkSmartPointer<vtkTransformFilter> ModelWarper;
    /// The slice plane, in the world: cuts the mesh, measures the distance of the mesh points
    /// from the slice and tells the streamlines where the slice is
    vtkSmartPointer<vtkPlane> Plane;
    vtkSmartPointer<vtkSampleImplicitFunctionFilter> SliceDistance;
    vtkSmartPointer<vtkThresholdPoints> SlabThreshold;
    /// Marks where the field is shown on the samples taken in the slice plane, and keeps the
    /// glyphs there
    vtkSmartPointer<vtkMRMLVectorFieldMask> Mask;
    vtkSmartPointer<vtkTransform> TransformToSlice;
    vtkSmartPointer<vtkArrayCalculator> ColorMagnitude;
    vtkSmartPointer<vtkTransformPolyDataFilter> Transformer;
    /// Streamlines traced by the display node, moved to world if transformed, then reduced to those
    /// crossing this slice, fading with distance.
    vtkSmartPointer<vtkTransformPolyDataFilter> StreamlineToWorld;
    vtkSmartPointer<vtkStreamlineSliceProjection> StreamlineProjection;
    vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
    vtkSmartPointer<vtkActor2D> Actor;
    /// The undeformed grid, which has a color of its own
    vtkSmartPointer<vtkTransformPolyDataFilter> NonWarpedTransformer;
    vtkSmartPointer<vtkPolyDataMapper2D> NonWarpedMapper;
    vtkSmartPointer<vtkActor2D> NonWarpedActor;
  };

  typedef std::map<vtkMRMLDisplayNode*, Pipeline*> PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map<vtkMRMLDisplayableNode*, std::set<vtkMRMLDisplayNode*>> DisplayableToDisplayCacheType;
  DisplayableToDisplayCacheType DisplayableToDisplayNodes;

  // Transforms
  void UpdateDisplayableTransforms(vtkMRMLDisplayableNode* node);
  void GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld);
  // Slice Node
  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  void UpdateSliceNode();
  /// Set the plane to the slice plane of the matrix. Only modifies the plane when it moved.
  void SetSlicePlaneFromMatrix(vtkMatrix4x4* matrix, vtkPlane* plane);

  // Display Nodes
  void AddDisplayNode(vtkMRMLDisplayableNode*, vtkMRMLDisplayNode*);
  void UpdateDisplayNode(vtkMRMLDisplayNode* displayNode);
  /// The pipeline is not const because the sampler of the slice is created on demand
  void UpdateDisplayNodePipeline(vtkMRMLDisplayNode*, Pipeline*);
  /// Draws the streamlines that the display node traced for all views, the ones that cross
  /// this slice, fading out with the distance from it.
  void UpdateStreamlines(vtkMRMLVectorFieldDisplayNode* fieldDisplayNode,
                         vtkMRMLDisplayNode* effectiveDisplayNode,
                         vtkMRMLDisplayableNode* displayableNode,
                         const Pipeline* pipeline,
                         double hierarchyOpacity,
                         double sliceHalfThicknessMm);
  void UpdateScalarColoring(vtkMRMLVectorFieldDisplayNode* fieldDisplayNode, vtkMRMLDisplayNode* effectiveDisplayNode, const Pipeline* pipeline, double hierarchyOpacity);
  /// Adds the magnitude of a multi-component color array as ColorMagnitudeArrayName and returns the connection
  /// to color from (the input connection itself for single-component arrays).
  vtkAlgorithmOutput* UpdateColorMagnitude(vtkMRMLVectorFieldDisplayNode* fieldDisplayNode, const Pipeline* pipeline, vtkAlgorithmOutput* connection);
  void UpdateActorProperties(vtkMRMLDisplayNode* effectiveDisplayNode, const Pipeline* pipeline, double hierarchyOpacity);
  void RemoveDisplayNode(vtkMRMLDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLDisplayableNode* node);
  void RemoveObservations(vtkMRMLDisplayableNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLDisplayableNode* displayNode);
  bool HasDisplayPipeline(vtkMRMLDisplayableNode* displayNode);
  void ClearDisplayableNodes();

  vtkInternal(vtkMRMLVectorFieldSliceDisplayableManager* external);
  ~vtkInternal();

private:
  vtkSmartPointer<vtkMatrix4x4> SliceXYToRAS;
  vtkSmartPointer<vtkMRMLSliceNode> SliceNode;
  vtkMRMLVectorFieldSliceDisplayableManager* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods
vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::vtkInternal(vtkMRMLVectorFieldSliceDisplayableManager* external)
{
  this->External = external;
  this->SliceXYToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  this->SliceXYToRAS->Identity();
}

//---------------------------------------------------------------------------
vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UseDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  return displayNode && displayNode->IsA("vtkMRMLVectorFieldDisplayNode");
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::IsVisible(vtkMRMLDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return false;
  }
  bool visibleInView = (this->SliceNode == nullptr) || displayNode->IsDisplayableInView(this->SliceNode->GetID());
  return displayNode->GetVisibility() && displayNode->GetVisibility2D() && visibleInView;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
  {
    return;
  }
  this->SliceNode = sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UpdateSliceNode()
{
  // Update the slice node transform, then update the pipelines to account for the new plane location
  this->SliceXYToRAS->DeepCopy(this->SliceNode->GetXYToRAS());
  for (auto& pipelineIt : this->DisplayPipelines)
  {
    this->UpdateDisplayNodePipeline(pipelineIt.first, pipelineIt.second);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::SetSlicePlaneFromMatrix(vtkMatrix4x4* sliceMatrix, vtkPlane* plane)
{
  double normal[3] = { 0.0, 0.0, 0.0 };
  double origin[3] = { 0.0, 0.0, 0.0 };
  for (int i = 0; i < 3; i++)
  {
    normal[i] = sliceMatrix->GetElement(i, 2);
    origin[i] = sliceMatrix->GetElement(i, 3);
  }
  vtkMath::Normalize(normal);
  // The setters only mark the plane modified when the value changes, so the filters that use
  // the plane do not re-execute when the slice did not move
  plane->SetNormal(normal);
  plane->SetOrigin(origin);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld)
{
  if (!node || !transformToWorld)
  {
    return;
  }
  vtkMRMLTransformNode* transformNode = node->GetParentTransformNode();
  transformToWorld->Identity();
  if (transformNode)
  {
    transformNode->GetTransformToWorld(transformToWorld);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UpdateDisplayableTransforms(vtkMRMLDisplayableNode* node)
{
  std::set<vtkMRMLDisplayNode*> displayNodes = this->DisplayableToDisplayNodes[node];
  for (auto displayNodeIt = displayNodes.begin(); displayNodeIt != displayNodes.end(); ++displayNodeIt)
  {
    PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(*displayNodeIt);
    if (pipelineIt != this->DisplayPipelines.end())
    {
      this->GetNodeTransformToWorld(node, pipelineIt->second->NodeToWorld);
      this->UpdateDisplayNodePipeline(pipelineIt->first, pipelineIt->second);
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::RemoveDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(displayNode);
  if (pipelineIt == this->DisplayPipelines.end())
  {
    return;
  }
  const Pipeline* pipeline = pipelineIt->second;
  this->External->GetRenderer()->RemoveActor(pipeline->Actor);
  this->External->GetRenderer()->RemoveActor(pipeline->NonWarpedActor);
  delete pipeline;
  this->DisplayPipelines.erase(pipelineIt);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::AddDisplayNode(vtkMRMLDisplayableNode* displayableNode, vtkMRMLDisplayNode* displayNode)
{
  if (!displayableNode || !displayNode)
  {
    return;
  }

  // Do not add the display node if it is already associated with a pipeline object
  if (this->DisplayPipelines.find(displayNode) != this->DisplayPipelines.end())
  {
    return;
  }

  Pipeline* pipeline = new Pipeline();
  // SliceSampler is made in UpdateDisplayNodePipeline(): whether the source can be resampled depends on
  // data that may not be loaded yet (a model node is added before its mesh is read).
  pipeline->MeshCutter = vtkSmartPointer<vtkPlaneCutter>::New();
  pipeline->NodeToWorld = vtkSmartPointer<vtkGeneralTransform>::New();
  pipeline->ModelWarper = vtkSmartPointer<vtkTransformFilter>::New();
  pipeline->Plane = vtkSmartPointer<vtkPlane>::New();
  pipeline->MeshCutter->SetPlane(pipeline->Plane);
  pipeline->SliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  pipeline->SlabThreshold = vtkSmartPointer<vtkThresholdPoints>::New();
  pipeline->Mask = vtkSmartPointer<vtkMRMLVectorFieldMask>::New();
  pipeline->Geometry = vtkSmartPointer<vtkVectorFieldGeometry>::New();
  pipeline->Glyph = vtkSmartPointer<vtkVectorFieldGlyph>::New();
  pipeline->ColorMagnitude = vtkSmartPointer<vtkArrayCalculator>::New();
  pipeline->ColorMagnitude->SetAttributeTypeToPointData();
  pipeline->TransformToSlice = vtkSmartPointer<vtkTransform>::New();
  pipeline->Transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  pipeline->StreamlineToWorld = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  pipeline->StreamlineToWorld->SetTransform(pipeline->NodeToWorld);
  pipeline->StreamlineProjection = vtkSmartPointer<vtkStreamlineSliceProjection>::New();
  pipeline->Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  pipeline->Actor = vtkSmartPointer<vtkActor2D>::New();
  // Not something to grab hold of, for the same reason as in a 3D view
  pipeline->Actor->SetPickable(0);

  // Static parts of the pipeline; the point selection between slab and glyphs is rewired in
  // UpdateDisplayNodePipeline() because scalar thresholding is optional.
  pipeline->ModelWarper->SetTransform(pipeline->NodeToWorld);
  // Every vector array is transformed with the node, not only the active vectors: the field can be
  // any of them
  pipeline->ModelWarper->TransformAllInputVectorsOn();
  pipeline->SliceDistance->SetImplicitFunction(pipeline->Plane);
  pipeline->SliceDistance->SetScalarArrayName(SliceDistanceArrayName);
  pipeline->SliceDistance->ComputeGradientsOff();
  pipeline->SliceDistance->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
  pipeline->SlabThreshold->SetInputConnection(pipeline->SliceDistance->GetOutputPort());
  pipeline->SlabThreshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, SliceDistanceArrayName);
  pipeline->Transformer->SetTransform(pipeline->TransformToSlice);
  pipeline->Mapper->SetInputConnection(pipeline->Transformer->GetOutputPort());
  pipeline->Actor->SetMapper(pipeline->Mapper);
  pipeline->Actor->SetVisibility(0);
  pipeline->NonWarpedTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  pipeline->NonWarpedTransformer->SetTransform(pipeline->TransformToSlice);
  pipeline->NonWarpedMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  pipeline->NonWarpedMapper->SetInputConnection(pipeline->NonWarpedTransformer->GetOutputPort());
  pipeline->NonWarpedMapper->ScalarVisibilityOff();
  pipeline->NonWarpedActor = vtkSmartPointer<vtkActor2D>::New();
  pipeline->NonWarpedActor->SetMapper(pipeline->NonWarpedMapper);
  pipeline->NonWarpedActor->SetPickable(0);
  pipeline->NonWarpedActor->SetVisibility(0);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor(pipeline->Actor);
  this->External->GetRenderer()->AddActor(pipeline->NonWarpedActor);
  this->DisplayPipelines.insert(std::make_pair(displayNode, pipeline));

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(displayableNode);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UpdateDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return;
  }
  PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(displayNode);
  if (pipelineIt != this->DisplayPipelines.end())
  {
    this->UpdateDisplayNodePipeline(displayNode, pipelineIt->second);
  }
  else
  {
    this->External->AddDisplayableNode(displayNode->GetDisplayableNode());
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UpdateDisplayNodePipeline(vtkMRMLDisplayNode* displayNode, Pipeline* pipeline)
{
  if (!pipeline)
  {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLVectorFieldSliceDisplayableManager::UpdateDisplayNodePipeline failed: pipeline is invalid");
    return;
  }

  pipeline->NonWarpedActor->SetVisibility(false);

  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(displayNode);
  if (!fieldDisplayNode)
  {
    pipeline->Actor->SetVisibility(false);
    return;
  }

  // Get display node from hierarchy that applies display properties on branch
  vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  vtkMRMLDisplayNode* overrideHierarchyDisplayNode = vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);

  // Use hierarchy display node if any, and if overriding is allowed for the current display node.
  bool hierarchyVisibility = true;
  double hierarchyOpacity = 1.0;
  vtkMRMLDisplayNode* effectiveDisplayNode = fieldDisplayNode;
  if (fieldDisplayNode->GetFolderDisplayOverrideAllowed())
  {
    if (overrideHierarchyDisplayNode)
    {
      effectiveDisplayNode = overrideHierarchyDisplayNode;
    }
    hierarchyVisibility = vtkMRMLFolderDisplayNode::GetHierarchyVisibility(displayableNode);
    hierarchyOpacity = vtkMRMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
  }

  if (!this->SliceNode || !hierarchyVisibility || !this->IsVisible(fieldDisplayNode))
  {
    pipeline->Actor->SetVisibility(false);
    return;
  }

  // Slice plane in world, and a slab around it as thick as the slice (1 mm if the slice has no thickness).
  this->SetSlicePlaneFromMatrix(this->SliceXYToRAS, pipeline->Plane);
  double sliceFieldOfView[3] = { 0.0, 0.0, 0.0 };
  this->SliceNode->GetFieldOfView(sliceFieldOfView);
  double sliceHalfThicknessMm = 0.5 * (sliceFieldOfView[2] > 0.0 ? sliceFieldOfView[2] : 1.0);

  // Resampleable sources (volumes, transforms, volumetric meshes) are sampled in the slice plane; for others
  // the mesh points within the slab are selected. Decided on each update, as the mesh may load or change later.
  // The view owns its sampler, kept between updates so its mesh cell locator is reused, released when not resampling.
  // The display node also sets the field-to-world transform (none for a transform, which is evaluated in the world).
  if (!pipeline->SliceSampler)
  {
    pipeline->SliceSampler = vtkSmartPointer<vtkVectorFieldSampler>::New();
  }
  if (!fieldDisplayNode->UpdateSliceFieldSampler(pipeline->SliceSampler))
  {
    pipeline->SliceSampler = nullptr;
  }

  vtkAlgorithmOutput* glyphInputConnection = nullptr;
  if (pipeline->SliceSampler)
  {
    pipeline->SliceSampler->SetSlicePlane(this->SliceXYToRAS, sliceFieldOfView);
    pipeline->SliceSampler->SetSliceThickness(sliceFieldOfView[2]);
    // Not SamplingSpacing: transform display nodes use a per-mode spacing, refined so that grid lines are
    // a whole grid cell apart.
    pipeline->SliceSampler->SetSamplingSpacing(fieldDisplayNode->GetSamplingSpacingForField());
    pipeline->SliceSampler->SetLatticeGroupSize(fieldDisplayNode->GetGridSubdivision());
    // Explicit sample positions (points, curve, surface, plane) are shown where the slice crosses them; volume
    // regions are sampled in the slice plane instead, as few random volume samples would fall within the slice.
    vtkNew<vtkPoints> samplePositions_RAS;
    bool hasSamplePositions = false;
    if (fieldDisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph
        && fieldDisplayNode->GetEffectiveSamplingRegionPart() != vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume)
    {
      // Shared by all views, and computed again only when what defines them changes
      vtkPoints* sharedSamplePositions_RAS = fieldDisplayNode->GetPipeline()->GetSamplePositions();
      hasSamplePositions = (sharedSamplePositions_RAS != nullptr);
      if (hasSamplePositions)
      {
        samplePositions_RAS->ShallowCopy(sharedSamplePositions_RAS);
      }
      vtkNew<vtkMatrix4x4> latticeToRAS;
      int latticeSize[3] = { 0, 0, 0 };
      if (!hasSamplePositions && vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(fieldDisplayNode, latticeToRAS, latticeSize))
      {
        vtkVectorFieldSampler::GetLatticePositions(latticeToRAS, latticeSize, samplePositions_RAS);
        hasSamplePositions = true;
      }
    }
    pipeline->SliceSampler->SetSamplePositions(hasSamplePositions ? samplePositions_RAS.GetPointer() : nullptr);
    glyphInputConnection = pipeline->SliceSampler->GetOutputPort();
    // The samples are in the world, where the regions are
    if (fieldDisplayNode->HasRegionTest())
    {
      pipeline->Mask->SetInputConnection(glyphInputConnection);
      fieldDisplayNode->GetPipeline()->UpdateMask(pipeline->Mask);
      pipeline->Mask->SetPointsToWorldTransform(nullptr);
      glyphInputConnection = pipeline->Mask->GetOutputPort();
    }
    else
    {
      pipeline->Mask->SetInputConnection(nullptr);
    }
  }
  else
  {
    vtkAlgorithmOutput* fieldConnection = fieldDisplayNode->GetPipeline()->GetMaskedFieldConnection();
    if (!fieldConnection)
    {
      pipeline->Actor->SetVisibility(false);
      return;
    }
    // The region only bounds the 3D view: it is often another view's slice node, which would limit this view
    // to where that slice happens to be.
    pipeline->ModelWarper->SetInputConnection(fieldConnection);

    // Select the points that are within the slab around the slice plane
    pipeline->SlabThreshold->SetLowerThreshold(-sliceHalfThicknessMm);
    pipeline->SlabThreshold->SetUpperThreshold(sliceHalfThicknessMm);
    pipeline->SlabThreshold->SetThresholdFunction(vtkThresholdPoints::THRESHOLD_BETWEEN);
    glyphInputConnection = pipeline->SlabThreshold->GetOutputPort();
  }

  // Streamlines are traced in 3D, then cut to what runs near this slice; tracing in-plane would confine them
  // to the plane and find nothing in meshes thinner than the lattice.
  if (fieldDisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline)
  {
    this->UpdateStreamlines(fieldDisplayNode, effectiveDisplayNode, displayableNode, pipeline, hierarchyOpacity, sliceHalfThicknessMm);
    return;
  }
  pipeline->StreamlineProjection->SetInputConnection(nullptr);
  pipeline->StreamlineToWorld->SetInputConnection(nullptr);

  vtkAlgorithmOutput* geometryConnection = nullptr;
  if (fieldDisplayNode->IsPolyDataVisualizationMode())
  {
    // Grid and contour modes build geometry from the field sampled in this slice plane instead of glyphs.
    vtkAlgorithmOutput* modeInputConnection = glyphInputConnection;
    if (fieldDisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeContour //
        && !vtkMRMLVectorFieldSamplePointSource::NeedsLatticeSampling(fieldDisplayNode))
    {
      // Mesh isolines come from cutting the mesh with the slice plane: lattice sampling misses meshes thinner
      // than the lattice spacing (e.g., a shell one cell thick yields no isolines).
      vtkAlgorithmOutput* meshConnection = fieldDisplayNode->GetPipeline()->GetMaskedFieldConnection();
      if (meshConnection)
      {
        // The mesh is moved into the world first, where the slice plane is
        pipeline->ModelWarper->SetInputConnection(meshConnection);
        pipeline->MeshCutter->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
        modeInputConnection = pipeline->MeshCutter->GetOutputPort();
      }
    }
    if (fieldDisplayNode->GetPipeline()->UpdateGeometryFilter(pipeline->Geometry, modeInputConnection, true))
    {
      geometryConnection = pipeline->Geometry->GetOutputPort();
    }
  }
  else
  {
    // Flat glyph outlines in the slice plane (crisp at any zoom, do not hide the image), built in RAS so vectors keep
    // their mm length. Scaled uniformly, so small vectors shrink away instead of leaving heads behind.
    if (fieldDisplayNode->GetPipeline()->UpdateGlyphFilter(pipeline->Glyph, glyphInputConnection, this->SliceXYToRAS))
    {
      geometryConnection = pipeline->Glyph->GetOutputPort(vtkVectorFieldGlyph::GlyphGeometryPort);
    }
  }
  if (!geometryConnection)
  {
    pipeline->Actor->SetVisibility(false);
    return;
  }
  // Color magnitude is computed in RAS (as in 3D views), before the transform to slice coordinates scales the vectors.
  pipeline->Transformer->SetInputConnection(this->UpdateColorMagnitude(fieldDisplayNode, pipeline, geometryConnection));
  this->UpdateScalarColoring(fieldDisplayNode, effectiveDisplayNode, pipeline, hierarchyOpacity);
  this->UpdateActorProperties(effectiveDisplayNode, pipeline, hierarchyOpacity);

  // Looks like the deformed grid, except for its color
  if (fieldDisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid && fieldDisplayNode->GetGridShowNonWarped())
  {
    pipeline->NonWarpedTransformer->SetInputConnection(pipeline->Geometry->GetOutputPort(vtkVectorFieldGeometry::GridUndeformedLinesPort));
    vtkProperty2D* nonWarpedProperties = pipeline->NonWarpedActor->GetProperty();
    nonWarpedProperties->SetColor(fieldDisplayNode->GetGridNonWarpedColor());
    nonWarpedProperties->SetLineWidth(effectiveDisplayNode->GetSliceIntersectionThickness());
    nonWarpedProperties->SetOpacity(hierarchyOpacity * effectiveDisplayNode->GetSliceIntersectionOpacity());
    pipeline->NonWarpedActor->SetVisibility(true);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UpdateStreamlines(vtkMRMLVectorFieldDisplayNode* fieldDisplayNode,
                                                                               vtkMRMLDisplayNode* effectiveDisplayNode,
                                                                               vtkMRMLDisplayableNode* displayableNode,
                                                                               const Pipeline* pipeline,
                                                                               double hierarchyOpacity,
                                                                               double sliceHalfThicknessMm)
{
  vtkAlgorithmOutput* streamlinesConnection = fieldDisplayNode->GetPipeline()->GetStreamlineConnection();
  if (!streamlinesConnection)
  {
    pipeline->Actor->SetVisibility(false);
    return;
  }
  // The streamlines are in the coordinates of the field, the slice is in the world. A node
  // that is not under a transform is in the world already.
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(displayableNode);
  if (transformableNode && transformableNode->GetParentTransformNode() && !vtkMRMLTransformNode::SafeDownCast(displayableNode))
  {
    pipeline->StreamlineToWorld->SetInputConnection(streamlinesConnection);
    streamlinesConnection = pipeline->StreamlineToWorld->GetOutputPort();
  }
  else
  {
    pipeline->StreamlineToWorld->SetInputConnection(nullptr);
  }

  // The slice plane, in the world
  vtkStreamlineSliceProjection* projection = pipeline->StreamlineProjection;
  projection->SetInputConnection(streamlinesConnection);
  projection->SetPlaneOrigin(pipeline->Plane->GetOrigin());
  projection->SetPlaneNormal(pipeline->Plane->GetNormal());
  projection->SetHalfThickness(sliceHalfThicknessMm);
  projection->SetFadeDistance(fieldDisplayNode->GetStreamlineSliceFadeDistance());

  // Colored as in 3D views; the opacity of the display node is the opacity of the actor, so
  // the projection only makes the streamlines fade out
  bool colorByScalar = (fieldDisplayNode->GetScalarVisibility() && HasRenderedActiveScalar(fieldDisplayNode));
  projection->SetColorArrayName(colorByScalar ? fieldDisplayNode->GetRenderedActiveScalarName() : nullptr);
  projection->SetColorArrayComponent(fieldDisplayNode->GetRenderedActiveScalarComponent());
  vtkSmartPointer<vtkLookupTable> lookupTable;
  if (colorByScalar)
  {
    lookupTable = CreateLookupTable(fieldDisplayNode->GetColorNode());
  }
  projection->SetLookupTable(lookupTable);
  projection->SetScalarRange(fieldDisplayNode->GetScalarRange());
  projection->SetSolidColor(effectiveDisplayNode->GetColor());

  pipeline->Transformer->SetInputConnection(projection->GetOutputPort());
  // The colors are computed per point, with the fading in their alpha, and drawn as they are
  pipeline->Mapper->SetScalarVisibility(true);
  pipeline->Mapper->SetScalarModeToUsePointData();
  pipeline->Mapper->SetColorModeToDefault();
  this->UpdateActorProperties(effectiveDisplayNode, pipeline, hierarchyOpacity);
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UpdateColorMagnitude(vtkMRMLVectorFieldDisplayNode* fieldDisplayNode,
                                                                                                 const Pipeline* pipeline,
                                                                                                 vtkAlgorithmOutput* connection)
{
  // A multi-component array is colored by the chosen component or by its magnitude, computed
  // into a scalar array of its own
  if (!connection || !HasRenderedActiveScalar(fieldDisplayNode) || !fieldDisplayNode->IsRenderedActiveScalarMultiComponent())
  {
    pipeline->ColorMagnitude->SetInputConnection(nullptr);
    return connection;
  }
  const char* activeScalarName = fieldDisplayNode->GetRenderedActiveScalarName();
  pipeline->ColorMagnitude->SetInputConnection(connection);
  pipeline->ColorMagnitude->RemoveAllVariables();
  pipeline->ColorMagnitude->SetResultArrayName(ColorMagnitudeArrayName);
  int component = fieldDisplayNode->GetRenderedActiveScalarComponent();
  if (component >= 0)
  {
    pipeline->ColorMagnitude->AddScalarVariable("value", activeScalarName, component);
    pipeline->ColorMagnitude->SetFunction("value");
  }
  else
  {
    pipeline->ColorMagnitude->AddVectorVariable("value", activeScalarName);
    pipeline->ColorMagnitude->SetFunction("mag(value)");
  }
  return pipeline->ColorMagnitude->GetOutputPort();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UpdateScalarColoring(vtkMRMLVectorFieldDisplayNode* fieldDisplayNode,
                                                                                  vtkMRMLDisplayNode* effectiveDisplayNode,
                                                                                  const Pipeline* pipeline,
                                                                                  double hierarchyOpacity)
{
  // Coloring: either by the active point data scalar array through the color node's
  // lookup table, or by the display node's solid color.
  if (!fieldDisplayNode->GetScalarVisibility() || !HasRenderedActiveScalar(fieldDisplayNode))
  {
    pipeline->Mapper->SetScalarVisibility(false);
    return;
  }
  // Multi-component arrays are colored by the magnitude array from UpdateColorMagnitude(); the field's own
  // magnitude cannot be used, as the color array may be a different array.
  bool multiComponent = fieldDisplayNode->IsRenderedActiveScalarMultiComponent();
  std::string colorArrayName = (multiComponent ? ColorMagnitudeArrayName : fieldDisplayNode->GetRenderedActiveScalarName());

  // The alpha is set on the table, so this view has a table of its own
  vtkSmartPointer<vtkLookupTable> lookupTable = CreateLookupTable(fieldDisplayNode->GetColorNode());
  lookupTable->SetAlpha(hierarchyOpacity * effectiveDisplayNode->GetSliceIntersectionOpacity());

  pipeline->Mapper->SetLookupTable(lookupTable);
  pipeline->Mapper->SetColorModeToMapScalars();
  pipeline->Mapper->SetScalarModeToUsePointFieldData();
  pipeline->Mapper->ColorByArrayComponent(const_cast<char*>(colorArrayName.c_str()), 0);
  pipeline->Mapper->UseLookupTableScalarRangeOff();
  pipeline->Mapper->SetScalarRange(fieldDisplayNode->GetScalarRange());
  pipeline->Mapper->SetScalarVisibility(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UpdateActorProperties(vtkMRMLDisplayNode* effectiveDisplayNode, const Pipeline* pipeline, double hierarchyOpacity)
{
  // Map the geometry from RAS into the slice view's XY coordinate system
  vtkNew<vtkMatrix4x4> rasToXY;
  vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToXY.GetPointer());
  pipeline->TransformToSlice->SetMatrix(rasToXY.GetPointer());

  vtkProperty2D* actorProperties = pipeline->Actor->GetProperty();
  actorProperties->SetColor(effectiveDisplayNode->GetColor());
  actorProperties->SetLineWidth(effectiveDisplayNode->GetSliceIntersectionThickness());
  actorProperties->SetOpacity(hierarchyOpacity * effectiveDisplayNode->GetSliceIntersectionOpacity());

  pipeline->Actor->SetPosition(0, 0);
  pipeline->Actor->SetVisibility(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::AddObservations(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  for (unsigned long event : ObservedEvents)
  {
    if (!broker->GetObservationExist(node, event, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
      broker->AddObservation(node, event, this->External, this->External->GetMRMLNodesCallbackCommand());
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::RemoveObservations(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  for (unsigned long event : ObservedEvents)
  {
    vtkEventBroker::ObservationVector observations = broker->GetObservations(node, event, this->External, this->External->GetMRMLNodesCallbackCommand());
    broker->RemoveObservations(observations);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::ClearDisplayableNodes()
{
  while (this->DisplayableToDisplayNodes.size() > 0)
  {
    this->External->RemoveDisplayableNode(this->DisplayableToDisplayNodes.begin()->first);
  }
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::UseDisplayableNode(vtkMRMLDisplayableNode* node)
{
  // Any displayable node may store a vector field and get a vector field display node later, so observe all;
  // nodes without one have no pipeline (see HasDisplayPipeline()).
  return node != nullptr;
}

//---------------------------------------------------------------------------
bool vtkMRMLVectorFieldSliceDisplayableManager::vtkInternal::HasDisplayPipeline(vtkMRMLDisplayableNode* node)
{
  auto displayableIt = this->DisplayableToDisplayNodes.find(node);
  return displayableIt != this->DisplayableToDisplayNodes.end() && !displayableIt->second.empty();
}

//---------------------------------------------------------------------------
// vtkMRMLVectorFieldSliceDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLVectorFieldSliceDisplayableManager::vtkMRMLVectorFieldSliceDisplayableManager()
{
  this->Internal = new vtkInternal(this);
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
vtkMRMLVectorFieldSliceDisplayableManager::~vtkMRMLVectorFieldSliceDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::AddDisplayableNode(vtkMRMLDisplayableNode* node)
{
  if (this->AddingDisplayableNode)
  {
    return;
  }
  if (!this->Internal->UseDisplayableNode(node))
  {
    return;
  }

  this->AddingDisplayableNode = 1;
  this->Internal->AddObservations(node);
  // Registers the node even if it has no vector field display node yet, so that one can be
  // added to it later.
  this->Internal->DisplayableToDisplayNodes[node];

  int numberOfDisplayNodes = node->GetNumberOfDisplayNodes();
  for (int i = 0; i < numberOfDisplayNodes; i++)
  {
    vtkMRMLDisplayNode* displayNode = node->GetNthDisplayNode(i);
    if (this->Internal->UseDisplayNode(displayNode))
    {
      this->Internal->DisplayableToDisplayNodes[node].insert(displayNode);
      this->Internal->AddDisplayNode(node, displayNode);
    }
  }
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::RemoveDisplayableNode(vtkMRMLDisplayableNode* node)
{
  if (!node)
  {
    return;
  }
  vtkInternal::DisplayableToDisplayCacheType::iterator displayableIt = this->Internal->DisplayableToDisplayNodes.find(node);
  if (displayableIt == this->Internal->DisplayableToDisplayNodes.end())
  {
    return;
  }

  std::set<vtkMRMLDisplayNode*> displayNodes = displayableIt->second;
  for (auto displayNodeIt = displayNodes.begin(); displayNodeIt != displayNodes.end(); ++displayNodeIt)
  {
    this->Internal->RemoveDisplayNode(*displayNodeIt);
  }
  this->Internal->RemoveObservations(node);
  this->Internal->DisplayableToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!displayableNode)
  {
    return;
  }

  // Escape if the scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
  {
    this->SetUpdateFromMRMLRequested(true);
    return;
  }

  this->AddDisplayableNode(displayableNode);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  bool modified = false;
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (displayableNode)
  {
    this->RemoveDisplayableNode(displayableNode);
    modified = true;
  }
  else
  {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(node);
    if (displayNode)
    {
      this->Internal->RemoveDisplayNode(displayNode);
      modified = true;
    }
  }
  if (modified)
  {
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if (scene == nullptr || scene->IsBatchProcessing())
  {
    return;
  }

  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(caller);

  if (displayableNode)
  {
    if (event == vtkMRMLDisplayableNode::DisplayModifiedEvent)
    {
      vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(reinterpret_cast<vtkMRMLNode*>(callData));
      if (displayNode && this->Internal->UseDisplayNode(displayNode))
      {
        // A vector field display node was added to the node, or one of its properties changed
        this->Internal->DisplayableToDisplayNodes[displayableNode].insert(displayNode);
        this->Internal->UpdateDisplayNode(displayNode);
        this->RequestRender();
      }
      else if (this->Internal->HasDisplayPipeline(displayableNode))
      {
        this->Internal->UpdateDisplayableTransforms(displayableNode);
        this->RequestRender();
      }
    }
    else if ((event == vtkMRMLDisplayableNode::TransformModifiedEvent) //
             || (event == vtkMRMLModelNode::PolyDataModifiedEvent)     //
             || (event == vtkMRMLVolumeNode::ImageDataModifiedEvent))
    {
      if (!this->Internal->HasDisplayPipeline(displayableNode))
      {
        // Nothing is drawn for this node, there is nothing to update or to re-render
        return;
      }
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
    }
  }
  else if (vtkMRMLSliceNode::SafeDownCast(caller))
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
void vtkMRMLVectorFieldSliceDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkDebugMacro("vtkMRMLVectorFieldSliceDisplayableManager->UpdateFromMRML: Scene is not set.");
    return;
  }

  // Find the display nodes that need to be removed
  // (not deleted immediately because it would invalidate the pipeline iteration)
  std::set<vtkMRMLDisplayableNode*> displayableNodesToRemove;
  std::deque<vtkMRMLDisplayNode*> displayNodesToRemove;
  for (auto displayNodeToPipeline : this->Internal->DisplayPipelines)
  {
    vtkMRMLDisplayNode* displayNode = displayNodeToPipeline.first;
    if (!displayNode)
    {
      continue;
    }
    if (!scene->IsNodePresent(displayNode))
    {
      displayNodesToRemove.push_back(displayNode);
      continue;
    }
    vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
    if (displayableNodesToRemove.find(displayableNode) != displayableNodesToRemove.end())
    {
      continue;
    }
    if (!this->Internal->UseDisplayableNode(displayableNode))
    {
      displayableNodesToRemove.insert(displayableNode);
      continue;
    }
    if (!this->Internal->UseDisplayNode(displayNode))
    {
      displayNodesToRemove.push_back(displayNode);
      continue;
    }
  }
  for (vtkMRMLDisplayableNode* displayableNodeToRemove : displayableNodesToRemove)
  {
    this->RemoveDisplayableNode(displayableNodeToRemove);
  }
  for (vtkMRMLDisplayNode* displayNodeToRemove : displayNodesToRemove)
  {
    this->Internal->RemoveDisplayNode(displayNodeToRemove);
  }

  std::vector<vtkMRMLNode*> displayableNodes;
  int numberOfDisplayableNodes = scene->GetNodesByClass("vtkMRMLDisplayableNode", displayableNodes);
  for (int i = 0; i < numberOfDisplayableNodes; i++)
  {
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(displayableNodes[i]);
    if (this->Internal->UseDisplayableNode(displayableNode))
    {
      this->AddDisplayableNode(displayableNode);
    }
  }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorFieldSliceDisplayableManager::Create()
{
  this->Internal->SetSliceNode(this->GetMRMLSliceNode());
  this->SetUpdateFromMRMLRequested(true);
}
