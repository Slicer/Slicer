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

#include "vtkMRMLVectorField2DViewPipeline.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
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
#include <string>
#include <vector>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVectorField2DViewPipeline);

namespace
{
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
class vtkMRMLVectorField2DViewPipeline::vtkInternal
{
public:
  vtkInternal();

  /// Set the plane to the slice plane of the matrix. Only modifies the plane when it moved.
  static void SetSlicePlaneFromMatrix(vtkMatrix4x4* matrix, vtkPlane* plane);

  /// Resamples the field in this slice plane, for resampleable sources (volumes, transforms). Owned here,
  /// not by the display node, because the display node can be shown in several slice views.
  vtkSmartPointer<vtkVectorFieldSampler> SliceSampler;
  /// Cuts a mesh with the slice plane, so that isolines come from the mesh itself
  vtkNew<vtkPlaneCutter> MeshCutter;
  /// Geometry of the grid and contour modes, and flat glyphs, in this slice view
  vtkNew<vtkVectorFieldGeometry> Geometry;
  vtkNew<vtkVectorFieldGlyph> Glyph;
  vtkNew<vtkGeneralTransform> NodeToWorld;
  /// NodeToWorld is only updated when the transform of the node changes: setting it makes the mesh warped again
  bool NodeToWorldUpdateNeeded{ true };
  vtkNew<vtkTransformFilter> ModelWarper;
  /// The slice plane, in the world: cuts the mesh, measures the distance of the mesh points
  /// from the slice and tells the streamlines where the slice is
  vtkNew<vtkPlane> Plane;
  vtkNew<vtkSampleImplicitFunctionFilter> SliceDistance;
  vtkNew<vtkThresholdPoints> SlabThreshold;
  /// Marks where the field is shown on the samples taken in the slice plane, and keeps the
  /// glyphs there
  vtkNew<vtkMRMLVectorFieldMask> Mask;
  vtkNew<vtkTransform> TransformToSlice;
  vtkNew<vtkArrayCalculator> ColorMagnitude;
  vtkNew<vtkTransformPolyDataFilter> Transformer;
  /// Streamlines traced by the display node, moved to world if transformed, then reduced to those
  /// crossing this slice, fading with distance.
  vtkNew<vtkTransformPolyDataFilter> StreamlineToWorld;
  vtkNew<vtkStreamlineSliceProjection> StreamlineProjection;
  vtkNew<vtkPolyDataMapper2D> Mapper;
  vtkNew<vtkActor2D> Actor;
  /// The undeformed grid, which has a color of its own
  vtkNew<vtkTransformPolyDataFilter> NonWarpedTransformer;
  vtkNew<vtkPolyDataMapper2D> NonWarpedMapper;
  vtkNew<vtkActor2D> NonWarpedActor;
};

//---------------------------------------------------------------------------
vtkMRMLVectorField2DViewPipeline::vtkInternal::vtkInternal()
{
  // SliceSampler is made in UpdatePipeline(): whether the source can be resampled depends on
  // data that may not be loaded yet (a model node is added before its mesh is read).
  this->MeshCutter->SetPlane(this->Plane);
  this->ColorMagnitude->SetAttributeTypeToPointData();
  this->StreamlineToWorld->SetTransform(this->NodeToWorld);
  // Not something to grab hold of, for the same reason as in a 3D view
  this->Actor->SetPickable(0);

  // Static parts of the pipeline; the point selection between slab and glyphs is rewired in
  // UpdatePipeline() because scalar thresholding is optional.
  this->ModelWarper->SetTransform(this->NodeToWorld);
  // Every vector array is transformed with the node, not only the active vectors: the field can be
  // any of them
  this->ModelWarper->TransformAllInputVectorsOn();
  this->SliceDistance->SetImplicitFunction(this->Plane);
  this->SliceDistance->SetScalarArrayName(SliceDistanceArrayName);
  this->SliceDistance->ComputeGradientsOff();
  this->SliceDistance->SetInputConnection(this->ModelWarper->GetOutputPort());
  this->SlabThreshold->SetInputConnection(this->SliceDistance->GetOutputPort());
  this->SlabThreshold->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, SliceDistanceArrayName);
  this->Transformer->SetTransform(this->TransformToSlice);
  this->Mapper->SetInputConnection(this->Transformer->GetOutputPort());
  this->Actor->SetMapper(this->Mapper);
  this->Actor->SetVisibility(0);
  this->NonWarpedTransformer->SetTransform(this->TransformToSlice);
  this->NonWarpedMapper->SetInputConnection(this->NonWarpedTransformer->GetOutputPort());
  this->NonWarpedMapper->ScalarVisibilityOff();
  this->NonWarpedActor->SetMapper(this->NonWarpedMapper);
  this->NonWarpedActor->SetPickable(0);
  this->NonWarpedActor->SetVisibility(0);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField2DViewPipeline::vtkInternal::SetSlicePlaneFromMatrix(vtkMatrix4x4* sliceMatrix, vtkPlane* plane)
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
// vtkMRMLVectorField2DViewPipeline methods

//---------------------------------------------------------------------------
vtkMRMLVectorField2DViewPipeline::vtkMRMLVectorField2DViewPipeline()
{
  this->Internal = new vtkInternal();
}

//---------------------------------------------------------------------------
vtkMRMLVectorField2DViewPipeline::~vtkMRMLVectorField2DViewPipeline()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField2DViewPipeline::OnRendererAdded(vtkRenderer* renderer)
{
  renderer->AddActor(this->Internal->Actor);
  renderer->AddActor(this->Internal->NonWarpedActor);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField2DViewPipeline::OnRendererRemoved(vtkRenderer* renderer)
{
  renderer->RemoveActor(this->Internal->Actor);
  renderer->RemoveActor(this->Internal->NonWarpedActor);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField2DViewPipeline::OnUpdate(vtkObject* caller, unsigned long event, void* callData)
{
  // The transform of the node changed, the node got (another) displayable node, or the scene was loaded
  if (event == vtkMRMLDisplayableNode::TransformModifiedEvent //
      || event == vtkMRMLNode::ReferenceAddedEvent            //
      || event == vtkMRMLNode::ReferenceRemovedEvent          //
      || event == vtkMRMLScene::EndBatchProcessEvent)
  {
    this->Internal->NodeToWorldUpdateNeeded = true;
  }
  this->Superclass::OnUpdate(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField2DViewPipeline::UpdatePipeline()
{
  vtkInternal* internal = this->Internal;
  internal->NonWarpedActor->SetVisibility(false);

  double hierarchyOpacity = 1.0;
  vtkMRMLDisplayNode* effectiveDisplayNode = this->GetEffectiveDisplayNode(hierarchyOpacity);
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(this->GetViewNode());
  if (!effectiveDisplayNode || !sliceNode)
  {
    internal->Actor->SetVisibility(false);
    return;
  }
  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = this->GetFieldDisplayNode();

  if (internal->NodeToWorldUpdateNeeded)
  {
    internal->NodeToWorld->Identity();
    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(this->GetDisplayableNode());
    vtkMRMLTransformNode* transformNode = transformableNode ? transformableNode->GetParentTransformNode() : nullptr;
    if (transformNode)
    {
      transformNode->GetTransformToWorld(internal->NodeToWorld);
    }
    internal->NodeToWorldUpdateNeeded = false;
  }

  // Slice plane in world, and a slab around it as thick as the slice (1 mm if the slice has no thickness).
  vtkMatrix4x4* sliceXYToRAS = sliceNode->GetXYToRAS();
  vtkInternal::SetSlicePlaneFromMatrix(sliceXYToRAS, internal->Plane);
  double sliceFieldOfView[3] = { 0.0, 0.0, 0.0 };
  sliceNode->GetFieldOfView(sliceFieldOfView);
  double sliceHalfThicknessMm = 0.5 * (sliceFieldOfView[2] > 0.0 ? sliceFieldOfView[2] : 1.0);

  // Resampleable sources (volumes, transforms, volumetric meshes) are sampled in the slice plane; for others
  // the mesh points within the slab are selected. Decided on each update, as the mesh may load or change later.
  // The view owns its sampler, kept between updates so its mesh cell locator is reused, released when not resampling.
  // The display node also sets the field-to-world transform (none for a transform, which is evaluated in the world).
  if (!internal->SliceSampler)
  {
    internal->SliceSampler = vtkSmartPointer<vtkVectorFieldSampler>::New();
  }
  if (!this->GetFieldPipeline()->UpdateSliceFieldSampler(internal->SliceSampler))
  {
    internal->SliceSampler = nullptr;
  }

  vtkAlgorithmOutput* glyphInputConnection = nullptr;
  if (internal->SliceSampler)
  {
    internal->SliceSampler->SetSlicePlane(sliceXYToRAS, sliceFieldOfView);
    internal->SliceSampler->SetSliceThickness(sliceFieldOfView[2]);
    // Not SamplingSpacing: transform display nodes use a per-mode spacing, refined so that grid lines are
    // a whole grid cell apart.
    internal->SliceSampler->SetSamplingSpacing(fieldDisplayNode->GetSamplingSpacingForField());
    internal->SliceSampler->SetLatticeGroupSize(fieldDisplayNode->GetGridSubdivision());
    // Explicit sample positions (points, curve, surface, plane) are shown where the slice crosses them; volume
    // regions are sampled in the slice plane instead, as few random volume samples would fall within the slice.
    vtkNew<vtkPoints> samplePositions_RAS;
    bool hasSamplePositions = false;
    if (fieldDisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph
        && fieldDisplayNode->GetEffectiveSamplingRegionPart() != vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume)
    {
      // Shared by all views, and computed again only when what defines them changes
      vtkPoints* sharedSamplePositions_RAS = this->GetFieldPipeline()->GetSamplePositions();
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
    internal->SliceSampler->SetSamplePositions(hasSamplePositions ? samplePositions_RAS.GetPointer() : nullptr);
    glyphInputConnection = internal->SliceSampler->GetOutputPort();
    // The samples are in the world, where the regions are
    if (fieldDisplayNode->HasRegionTest())
    {
      internal->Mask->SetInputConnection(glyphInputConnection);
      this->GetFieldPipeline()->UpdateMask(internal->Mask);
      internal->Mask->SetPointsToWorldTransform(nullptr);
      glyphInputConnection = internal->Mask->GetOutputPort();
    }
    else
    {
      internal->Mask->SetInputConnection(nullptr);
    }
  }
  else
  {
    vtkAlgorithmOutput* fieldConnection = this->GetFieldPipeline()->GetMaskedFieldConnection();
    if (!fieldConnection)
    {
      internal->Actor->SetVisibility(false);
      return;
    }
    // The region only bounds the 3D view: it is often another view's slice node, which would limit this view
    // to where that slice happens to be.
    internal->ModelWarper->SetInputConnection(fieldConnection);

    // Select the points that are within the slab around the slice plane
    internal->SlabThreshold->SetLowerThreshold(-sliceHalfThicknessMm);
    internal->SlabThreshold->SetUpperThreshold(sliceHalfThicknessMm);
    internal->SlabThreshold->SetThresholdFunction(vtkThresholdPoints::THRESHOLD_BETWEEN);
    glyphInputConnection = internal->SlabThreshold->GetOutputPort();
  }

  // Streamlines are traced in 3D, then cut to what runs near this slice; tracing in-plane would confine them
  // to the plane and find nothing in meshes thinner than the lattice.
  if (fieldDisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline)
  {
    this->UpdateStreamlines(effectiveDisplayNode, hierarchyOpacity, sliceHalfThicknessMm);
    return;
  }
  internal->StreamlineProjection->SetInputConnection(nullptr);
  internal->StreamlineToWorld->SetInputConnection(nullptr);

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
      vtkAlgorithmOutput* meshConnection = this->GetFieldPipeline()->GetMaskedFieldConnection();
      if (meshConnection)
      {
        // The mesh is moved into the world first, where the slice plane is
        internal->ModelWarper->SetInputConnection(meshConnection);
        internal->MeshCutter->SetInputConnection(internal->ModelWarper->GetOutputPort());
        modeInputConnection = internal->MeshCutter->GetOutputPort();
      }
    }
    if (this->GetFieldPipeline()->UpdateGeometryFilter(internal->Geometry, modeInputConnection, true))
    {
      geometryConnection = internal->Geometry->GetOutputPort();
    }
  }
  else
  {
    // Flat glyph outlines in the slice plane (crisp at any zoom, do not hide the image), built in RAS so vectors keep
    // their mm length. Scaled uniformly, so small vectors shrink away instead of leaving heads behind.
    if (this->GetFieldPipeline()->UpdateGlyphFilter(internal->Glyph, glyphInputConnection, sliceXYToRAS))
    {
      geometryConnection = internal->Glyph->GetOutputPort(vtkVectorFieldGlyph::GlyphGeometryPort);
    }
  }
  if (!geometryConnection)
  {
    internal->Actor->SetVisibility(false);
    return;
  }
  // Color magnitude is computed in RAS (as in 3D views), before the transform to slice coordinates scales the vectors.
  internal->Transformer->SetInputConnection(this->UpdateColorMagnitude(geometryConnection));
  this->UpdateScalarColoring(effectiveDisplayNode, hierarchyOpacity);
  this->UpdateActorProperties(effectiveDisplayNode, hierarchyOpacity);

  // Looks like the deformed grid, except for its color
  if (fieldDisplayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid && fieldDisplayNode->GetGridShowNonWarped())
  {
    internal->NonWarpedTransformer->SetInputConnection(internal->Geometry->GetOutputPort(vtkVectorFieldGeometry::GridUndeformedLinesPort));
    vtkProperty2D* nonWarpedProperties = internal->NonWarpedActor->GetProperty();
    nonWarpedProperties->SetColor(fieldDisplayNode->GetGridNonWarpedColor());
    nonWarpedProperties->SetLineWidth(effectiveDisplayNode->GetSliceIntersectionThickness());
    nonWarpedProperties->SetOpacity(hierarchyOpacity * effectiveDisplayNode->GetSliceIntersectionOpacity());
    internal->NonWarpedActor->SetVisibility(true);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField2DViewPipeline::UpdateStreamlines(vtkMRMLDisplayNode* effectiveDisplayNode, double hierarchyOpacity, double sliceHalfThicknessMm)
{
  vtkInternal* internal = this->Internal;
  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = this->GetFieldDisplayNode();
  vtkAlgorithmOutput* streamlinesConnection = this->GetFieldPipeline()->GetStreamlineConnection();
  if (!streamlinesConnection)
  {
    internal->Actor->SetVisibility(false);
    return;
  }
  // The streamlines are in the coordinates of the field, the slice is in the world. A node
  // that is not under a transform is in the world already.
  vtkMRMLDisplayableNode* displayableNode = this->GetDisplayableNode();
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(displayableNode);
  if (transformableNode && transformableNode->GetParentTransformNode() && !vtkMRMLTransformNode::SafeDownCast(displayableNode))
  {
    internal->StreamlineToWorld->SetInputConnection(streamlinesConnection);
    streamlinesConnection = internal->StreamlineToWorld->GetOutputPort();
  }
  else
  {
    internal->StreamlineToWorld->SetInputConnection(nullptr);
  }

  // The slice plane, in the world
  vtkStreamlineSliceProjection* projection = internal->StreamlineProjection;
  projection->SetInputConnection(streamlinesConnection);
  projection->SetPlaneOrigin(internal->Plane->GetOrigin());
  projection->SetPlaneNormal(internal->Plane->GetNormal());
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

  internal->Transformer->SetInputConnection(projection->GetOutputPort());
  // The colors are computed per point, with the fading in their alpha, and drawn as they are
  internal->Mapper->SetScalarVisibility(true);
  internal->Mapper->SetScalarModeToUsePointData();
  internal->Mapper->SetColorModeToDefault();
  this->UpdateActorProperties(effectiveDisplayNode, hierarchyOpacity);
}

//---------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLVectorField2DViewPipeline::UpdateColorMagnitude(vtkAlgorithmOutput* connection)
{
  vtkArrayCalculator* colorMagnitude = this->Internal->ColorMagnitude;
  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = this->GetFieldDisplayNode();
  // A multi-component array is colored by the chosen component or by its magnitude, computed
  // into a scalar array of its own
  if (!connection || !HasRenderedActiveScalar(fieldDisplayNode) || !fieldDisplayNode->IsRenderedActiveScalarMultiComponent())
  {
    colorMagnitude->SetInputConnection(nullptr);
    return connection;
  }
  const char* activeScalarName = fieldDisplayNode->GetRenderedActiveScalarName();
  colorMagnitude->SetInputConnection(connection);
  colorMagnitude->RemoveAllVariables();
  colorMagnitude->SetResultArrayName(ColorMagnitudeArrayName);
  int component = fieldDisplayNode->GetRenderedActiveScalarComponent();
  if (component >= 0)
  {
    colorMagnitude->AddScalarVariable("value", activeScalarName, component);
    colorMagnitude->SetFunction("value");
  }
  else
  {
    colorMagnitude->AddVectorVariable("value", activeScalarName);
    colorMagnitude->SetFunction("mag(value)");
  }
  return colorMagnitude->GetOutputPort();
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField2DViewPipeline::UpdateScalarColoring(vtkMRMLDisplayNode* effectiveDisplayNode, double hierarchyOpacity)
{
  vtkPolyDataMapper2D* mapper = this->Internal->Mapper;
  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = this->GetFieldDisplayNode();
  // Coloring: either by the active point data scalar array through the color node's
  // lookup table, or by the display node's solid color.
  if (!fieldDisplayNode->GetScalarVisibility() || !HasRenderedActiveScalar(fieldDisplayNode))
  {
    mapper->SetScalarVisibility(false);
    return;
  }
  // Multi-component arrays are colored by the magnitude array from UpdateColorMagnitude(); the field's own
  // magnitude cannot be used, as the color array may be a different array.
  bool multiComponent = fieldDisplayNode->IsRenderedActiveScalarMultiComponent();
  std::string colorArrayName = (multiComponent ? ColorMagnitudeArrayName : fieldDisplayNode->GetRenderedActiveScalarName());

  // The alpha is set on the table, so this view has a table of its own
  vtkSmartPointer<vtkLookupTable> lookupTable = CreateLookupTable(fieldDisplayNode->GetColorNode());
  lookupTable->SetAlpha(hierarchyOpacity * effectiveDisplayNode->GetSliceIntersectionOpacity());

  mapper->SetLookupTable(lookupTable);
  mapper->SetColorModeToMapScalars();
  mapper->SetScalarModeToUsePointFieldData();
  mapper->ColorByArrayComponent(const_cast<char*>(colorArrayName.c_str()), 0);
  mapper->UseLookupTableScalarRangeOff();
  mapper->SetScalarRange(fieldDisplayNode->GetScalarRange());
  mapper->SetScalarVisibility(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField2DViewPipeline::UpdateActorProperties(vtkMRMLDisplayNode* effectiveDisplayNode, double hierarchyOpacity)
{
  vtkInternal* internal = this->Internal;
  // Map the geometry from RAS into the slice view's XY coordinate system
  vtkNew<vtkMatrix4x4> rasToXY;
  vtkMatrix4x4::Invert(vtkMRMLSliceNode::SafeDownCast(this->GetViewNode())->GetXYToRAS(), rasToXY.GetPointer());
  internal->TransformToSlice->SetMatrix(rasToXY.GetPointer());

  vtkProperty2D* actorProperties = internal->Actor->GetProperty();
  actorProperties->SetColor(effectiveDisplayNode->GetColor());
  actorProperties->SetLineWidth(effectiveDisplayNode->GetSliceIntersectionThickness());
  actorProperties->SetOpacity(hierarchyOpacity * effectiveDisplayNode->GetSliceIntersectionOpacity());

  internal->Actor->SetPosition(0, 0);
  internal->Actor->SetVisibility(true);
}
