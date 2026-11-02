/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Checks the vector field display node: that its settings are saved and loaded, and that
// it offers and builds the visualization each kind of source supports.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLVectorFieldPipeline.h"
#include "vtkMRMLVectorFieldMask.h"
#include "vtkMRMLVectorFieldSamplePointSource.h"
#include "vtkVectorFieldGlyph.h"
#include "vtkVectorFieldSampler.h"
#include "vtkMRMLVectorFieldTestingHelpers.h"
#include "vtkMRMLVectorVolumeNode.h"

// Segmentation includes
#include <vtkOrientedImageData.h>
#include <vtkSegment.h>
#include <vtkSegmentation.h>
#include <vtkBinaryLabelmapToClosedSurfaceConversionRule.h>
#include <vtkSegmentationConverter.h>
#include <vtkSegmentationConverterFactory.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkVector.h>

// STD includes
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

//----------------------------------------------------------------------------
/// The class of the filter that produces what the field is drawn from in 3D views.
std::string GetFieldProducerClassName(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  vtkAlgorithmOutput* fieldConnection = displayNode->GetFieldConnection();
  if (!fieldConnection || !fieldConnection->GetProducer())
  {
    return "(none)";
  }
  // A sampler is told apart by the source it samples
  vtkVectorFieldSampler* sampler = vtkVectorFieldSampler::SafeDownCast(fieldConnection->GetProducer());
  if (sampler)
  {
    return std::string("vtkVectorFieldSampler:") + vtkVectorFieldSampler::GetFieldSourceAsString(sampler->GetFieldSource());
  }
  return fieldConnection->GetProducer()->GetClassName();
}

//----------------------------------------------------------------------------
/// What the field is drawn from in 3D views, computed.
vtkPointSet* UpdateField(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  return UpdateConnection<vtkPointSet>(displayNode->GetFieldConnection());
}

//----------------------------------------------------------------------------
bool HasArray(vtkMRMLVectorFieldDisplayNode* displayNode, const std::string& name, int expectedComponents)
{
  std::vector<std::string> names;
  std::vector<int> components;
  displayNode->GetFieldArrayInfo(names, components);
  for (size_t i = 0; i < names.size(); ++i)
  {
    if (names[i] == name)
    {
      return components[i] == expectedComponents;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
int TestEnumStrings()
{
  for (int i = 0; i < vtkMRMLVectorFieldDisplayNode::GlyphType_Last; ++i)
  {
    CHECK_INT(vtkMRMLVectorFieldDisplayNode::GetGlyphTypeFromString(vtkMRMLVectorFieldDisplayNode::GetGlyphTypeAsString(i)), i);
  }
  for (int i = 0; i < vtkMRMLVectorFieldDisplayNode::VisualizationMode_Last; ++i)
  {
    CHECK_INT(vtkMRMLVectorFieldDisplayNode::GetVisualizationModeFromString(vtkMRMLVectorFieldDisplayNode::GetVisualizationModeAsString(i)), i);
  }
  CHECK_INT(vtkMRMLVectorFieldDisplayNode::GetGlyphTypeFromString("invalid"), -1);
  CHECK_INT(vtkMRMLVectorFieldDisplayNode::GetVisualizationModeFromString(nullptr), -1);
  for (int i = 0; i < vtkMRMLVectorFieldDisplayNode::SamplingRegionPart_Last; ++i)
  {
    CHECK_INT(vtkMRMLVectorFieldDisplayNode::GetSamplingRegionPartFromString(vtkMRMLVectorFieldDisplayNode::GetSamplingRegionPartAsString(i)), i);
  }
  for (int i = 0; i < vtkMRMLVectorFieldDisplayNode::SamplingPattern_Last; ++i)
  {
    CHECK_INT(vtkMRMLVectorFieldDisplayNode::GetSamplingPatternFromString(vtkMRMLVectorFieldDisplayNode::GetSamplingPatternAsString(i)), i);
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestModeProperties()
{
  vtkNew<vtkMRMLVectorFieldDisplayNode> displayNode;
  // Samples are taken at random by default, a manageable number of them whatever the source
  CHECK_INT(displayNode->GetSamplingPattern(), vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  CHECK_INT(displayNode->GetSamplingPointsNumber(), 5000);
  // The threshold hides glyphs and parts of streamlines. Contour levels already say which
  // magnitudes are drawn, and a grid is not cut by it.
  const bool thresholdUsed[] = { true, false, false, true };
  for (int mode = 0; mode < vtkMRMLVectorFieldDisplayNode::VisualizationMode_Last; ++mode)
  {
    displayNode->SetVisualizationMode(mode);
    CHECK_BOOL(displayNode->IsThresholdUsed(), thresholdUsed[mode]);
  }

  // The contour opacity is for 3D views; scenes that named it contourOpacity still load
  CHECK_DOUBLE(displayNode->GetContourOpacity3D(), 0.8);
  const char* legacyAttributes[] = { "contourOpacity", "0.3", nullptr };
  displayNode->ReadXMLAttributes(legacyAttributes);
  CHECK_DOUBLE(displayNode->GetContourOpacity3D(), 0.3);
  const char* attributes[] = { "contourOpacity3D", "0.6", nullptr };
  displayNode->ReadXMLAttributes(attributes);
  CHECK_DOUBLE(displayNode->GetContourOpacity3D(), 0.6);
  displayNode->SetContourOpacity3D(0.4);
  CHECK_DOUBLE(displayNode->GetContourOpacity3D(), 0.4);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestSaveAndLoad()
{
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh();
  AddPointField(surface, "Field", SmoothField);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, surface);
  displayNode->SetFieldArrayName("Field");
  displayNode->SetGlyphType(vtkMRMLVectorFieldDisplayNode::GlyphTypeCylinder);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  displayNode->SetSamplingPointsNumber(123);
  displayNode->GlyphScaleByComponentsOn();
  displayNode->MaskingShowInsideOff();
  displayNode->SetMaskingSegmentIDs({ "Segment_1", "Segment;2" });
  displayNode->SetGlyphScaleFactor(2.5);
  displayNode->SetGridSpacing(7.5);
  double levels[3] = { 0.25, 0.5, 0.75 };
  displayNode->SetContourLevels(levels, 3);
  displayNode->SetThresholdEnabled(true);
  displayNode->SetThresholdRange(0.1, 0.9);
  displayNode->SetInterpolateBetweenPoints(true);
  displayNode->SetInterpolationMaximumDistance(12.5);
  displayNode->SetStreamlineSliceFadeDistance(7.5);
  displayNode->SetActiveScalarComponent(1);
  displayNode->SetGridFill(true);
  displayNode->SetGridLinesVisibility(false);
  displayNode->SetGridNonWarpedColor(0.25, 0.5, 0.75);
  vtkMRMLNode* samplingPlaneNode = scene->AddNewNodeByClass("vtkMRMLMarkupsPlaneNode");
  displayNode->SetAndObserveSamplingRegionNode(samplingPlaneNode);
  vtkMRMLNode* maskingROINode = scene->AddNewNodeByClass("vtkMRMLMarkupsROINode");
  displayNode->SetAndObserveMaskingRegionNode(maskingROINode);

  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene), true);
  vtkMRMLVectorFieldDisplayNode* reloaded = vtkMRMLVectorFieldDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLVectorFieldDisplayNode"));
  CHECK_NOT_NULL(reloaded);
  CHECK_STRING(reloaded->GetFieldArrayName(), "Field");
  CHECK_STRING(reloaded->GetGlyphOrientationArrayName(), "Field");
  CHECK_INT(reloaded->GetGlyphType(), vtkMRMLVectorFieldDisplayNode::GlyphTypeCylinder);
  CHECK_INT(reloaded->GetVisualizationMode(), vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  CHECK_INT(reloaded->GetSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  CHECK_INT(reloaded->GetSamplingPattern(), vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  CHECK_INT(reloaded->GetSamplingPointsNumber(), 123);
  CHECK_BOOL(reloaded->GetMaskingShowInside(), false);
  CHECK_BOOL(reloaded->GetGlyphScaleByComponents(), true);
  CHECK_BOOL(reloaded->GetMaskingSegmentIDs() == std::vector<std::string>({ "Segment_1", "Segment;2" }), true);
  CHECK_DOUBLE(reloaded->GetGlyphScaleFactor(), 2.5);
  CHECK_DOUBLE(reloaded->GetGridSpacing(), 7.5);
  CHECK_STD_STRING(reloaded->GetContourLevelsAsString(), "0.25 0.5 0.75");
  CHECK_BOOL(reloaded->GetThresholdEnabled(), true);
  CHECK_DOUBLE(reloaded->GetThresholdRange()[0], 0.1);
  CHECK_DOUBLE(reloaded->GetThresholdRange()[1], 0.9);
  CHECK_BOOL(reloaded->GetInterpolateBetweenPoints(), true);
  CHECK_INT(reloaded->GetActiveScalarComponent(), 1);
  CHECK_BOOL(reloaded->GetGridFill(), true);
  CHECK_BOOL(reloaded->GetGridLinesVisibility(), false);
  CHECK_DOUBLE(reloaded->GetGridNonWarpedColor()[0], 0.25);
  CHECK_DOUBLE(reloaded->GetGridNonWarpedColor()[1], 0.5);
  CHECK_DOUBLE(reloaded->GetGridNonWarpedColor()[2], 0.75);
  CHECK_DOUBLE(reloaded->GetInterpolationMaximumDistance(), 12.5);
  CHECK_DOUBLE(reloaded->GetStreamlineSliceFadeDistance(), 7.5);

  // The region nodes are observed after loading as well, so the field follows their changes
  vtkMRMLNode* reloadedSamplingPlaneNode = reloaded->GetSamplingRegionNode();
  vtkMRMLNode* reloadedMaskingROINode = reloaded->GetMaskingRegionNode();
  CHECK_STRING(reloadedSamplingPlaneNode->GetID(), samplingPlaneNode->GetID());
  CHECK_STRING(reloadedMaskingROINode->GetID(), maskingROINode->GetID());
  vtkMTimeType modifiedTime = reloaded->GetMTime();
  reloadedSamplingPlaneNode->Modified();
  CHECK_BOOL(reloaded->GetMTime() > modifiedTime, true);
  modifiedTime = reloaded->GetMTime();
  reloadedMaskingROINode->Modified();
  CHECK_BOOL(reloaded->GetMTime() > modifiedTime, true);

  // Copying a node copies the new settings too
  vtkNew<vtkMRMLVectorFieldDisplayNode> copy;
  copy->CopyContent(displayNode);
  CHECK_BOOL(copy->GetInterpolateBetweenPoints(), true);
  CHECK_DOUBLE(copy->GetInterpolationMaximumDistance(), 12.5);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestSurfaceMesh()
{
  std::cout << "Surface mesh" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh();
  AddPointField(surface, "Field", SmoothField);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, surface);
  displayNode->SetFieldArrayName("Field");
  CHECK_BOOL(HasArray(displayNode, "Field", 3), true);
  CHECK_BOOL(displayNode->HasFixedFieldArrays(), false);
  CHECK_BOOL(displayNode->HasVolumetricMesh(), false);

  // The vectors are only known at the points: glyphs on the points, and nothing else
  CHECK_BOOL(displayNode->CanSampleAtArbitraryPositions(), false);
  CHECK_BOOL(displayNode->IsVisualizationModeSupported(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph), true);
  CHECK_BOOL(displayNode->IsVisualizationModeSupported(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid), false);
  CHECK_BOOL(displayNode->IsVisualizationModeSupported(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour), false);
  CHECK_BOOL(displayNode->IsVisualizationModeSupported(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline), false);
  // Only its own points can get glyphs: all of them, or a random subset spread over its
  // bounds, surface or volume
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(displayNode), true);
  for (int part : { vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints,
                    vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface,
                    vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume })
  {
    CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(part), true);
  }
  CHECK_INT(displayNode->GetEffectiveSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(displayNode), true);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartDefault);
  CHECK_BOOL(displayNode->IsFieldSampled(), false);
  CHECK_NULL(CreateSliceFieldSampler(displayNode).GetPointer());
  CHECK_BOOL(displayNode->CanInterpolateBetweenPoints(), true);
  CHECK_BOOL(displayNode->IsInterpolationUsed(), false);
  CHECK_DOUBLE(displayNode->GetPipeline()->GetInterpolationResidual(), -1.0);

  // Interpolated between the points: every mode, a lattice, and the points of other nodes
  displayNode->SetInterpolateBetweenPoints(true);
  CHECK_BOOL(displayNode->IsInterpolationUsed(), true);
  CHECK_BOOL(displayNode->CanSampleAtArbitraryPositions(), true);
  for (int mode = 0; mode < vtkMRMLVectorFieldDisplayNode::VisualizationMode_Last; ++mode)
  {
    CHECK_BOOL(displayNode->IsVisualizationModeSupported(mode), true);
  }
  // Its inside can now be sampled on a lattice or at random; its points are still its own
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(displayNode), true);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(displayNode), false);
  double residual = displayNode->GetPipeline()->GetInterpolationResidual();
  std::cout << "  Fit residual: " << residual << std::endl;
  CHECK_BOOL(residual >= 0.0 && residual < 0.05, true);

  // Glyphs on the mesh points still come from the mesh itself, not from the fit
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);
  CHECK_BOOL(displayNode->IsFieldSampled(), false);
  // Glyphs on a lattice inside it come from the fit
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  displayNode->SetSamplingSpacing(10.0);
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkVectorFieldSampler:Scattered");
  CHECK_BOOL(displayNode->IsFieldSampled(), true);
  vtkPointSet* field = UpdateField(displayNode);
  CHECK_NOT_NULL(field);
  CHECK_BOOL(field->GetNumberOfPoints() > 0, true);
  CHECK_NOT_NULL(field->GetPointData()->GetArray(vtkVectorFieldSampler::GetVectorArrayName()));
  // The field is drawn up to the maximum distance from the points on every side, also
  // beyond the outermost ones, and fitted there too
  double maximumDistanceMm = displayNode->GetPipeline()->GetEffectiveInterpolationMaximumDistance();
  CHECK_DOUBLE_TOLERANCE(maximumDistanceMm, 0.1 * std::sqrt(3.0) * 100.0, 1.0);
  double fieldBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  field->GetBounds(fieldBounds);
  CHECK_BOOL(fieldBounds[1] > 50.0 + 0.5 * maximumDistanceMm, true);
  CHECK_BOOL(fieldBounds[0] < -50.0 - 0.5 * maximumDistanceMm, true);

  // Streamlines of a field fitted to the points are traced on a lattice, whose spacing applies
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), true);

  // So do the other modes, in 3D views and in slice views
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkVectorFieldSampler:Scattered");
  vtkSmartPointer<vtkVectorFieldSampler> sliceSampler = CreateSliceFieldSampler(displayNode);
  CHECK_INT(sliceSampler->GetFieldSource(), vtkVectorFieldSampler::FieldSourceScattered);
  // The fit is shared: the slice view does not fit the field again
  vtkVectorFieldSampler* fieldSampler = vtkVectorFieldSampler::SafeDownCast(displayNode->GetFieldConnection()->GetProducer());
  CHECK_POINTER(sliceSampler->GetScatteredFittedTransform(), fieldSampler->GetScatteredFittedTransform());

  // The distance that the field is drawn to is passed to the samplers
  displayNode->SetInterpolationMaximumDistance(3.0);
  displayNode->GetFieldConnection();
  CHECK_DOUBLE(fieldSampler->GetScatteredMaximumDistance(), 3.0);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestVolumetricMesh()
{
  std::cout << "Volumetric mesh" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddPointField(mesh, "Field", LinearField);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, mesh);
  displayNode->SetFieldArrayName("Field");
  CHECK_BOOL(displayNode->HasVolumetricMesh(), true);
  CHECK_BOOL(displayNode->CanSampleAtArbitraryPositions(), true);
  // The cells say exactly how the field varies between the points, but only inside the mesh;
  // a fit to the points extends it beyond them
  CHECK_BOOL(displayNode->CanInterpolateBetweenPoints(), true);
  CHECK_BOOL(displayNode->IsInterpolationUsed(), false);
  for (int mode = 0; mode < vtkMRMLVectorFieldDisplayNode::VisualizationMode_Last; ++mode)
  {
    CHECK_BOOL(displayNode->IsVisualizationModeSupported(mode), true);
  }
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkVectorFieldSampler:Mesh");
  CHECK_INT(CreateSliceFieldSampler(displayNode)->GetFieldSource(), vtkVectorFieldSampler::FieldSourceMesh);
  // Isosurfaces are built on the cells of the mesh, not on a lattice
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::NeedsLatticeSampling(displayNode), false);
  // everywhere, so nothing is sampled for them: no region, part, pattern or spacing applies
  CHECK_BOOL(displayNode->IsSamplingRegionUsed(), false);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), false);
  // And so are streamlines, but from seeds that are sampled in the region. Without a region
  // the seeds are the points of the mesh, so the sampling spacing does not apply to them.
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  CHECK_BOOL(displayNode->IsSamplingRegionUsed(), true);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), false);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(displayNode->IsSamplingRegionUsed(), true);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), true);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  // Without a node to start from, the streamlines start at the points of the mesh, or some of
  // them at random
  vtkNew<vtkPoints> seeds;
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(displayNode, seeds), true);
  CHECK_INT(seeds->GetNumberOfPoints(), mesh->GetNumberOfPoints());
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  displayNode->SetSamplingPointsNumber(20);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(displayNode, seeds), true);
  CHECK_INT(seeds->GetNumberOfPoints(), 20);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);

  // Interpolated between its points, the field extends beyond the mesh: every mode is built
  // from the fit, on a lattice, and draws outside the cells too
  displayNode->SetInterpolateBetweenPoints(true);
  CHECK_BOOL(displayNode->IsInterpolationUsed(), true);
  displayNode->SetInterpolationMaximumDistance(20.0);
  displayNode->SetSamplingSpacing(10.0);
  for (int mode : { vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid, vtkMRMLVectorFieldDisplayNode::VisualizationModeContour })
  {
    displayNode->SetVisualizationMode(mode);
    CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkVectorFieldSampler:Scattered");
    // A lattice is sampled in the region for isosurfaces too
    CHECK_BOOL(displayNode->IsSamplingRegionUsed(), true);
    CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), true);
  }
  CHECK_INT(CreateSliceFieldSampler(displayNode)->GetFieldSource(), vtkVectorFieldSampler::FieldSourceScattered);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  vtkPointSet* field = UpdateField(displayNode);
  CHECK_NOT_NULL(field);
  // The mesh spans 100 mm; the fitted field reaches the maximum distance beyond it
  double fieldBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  field->GetBounds(fieldBounds);
  std::cout << "  Extrapolated field from " << fieldBounds[0] << " to " << fieldBounds[1] << std::endl;
  CHECK_BOOL(fieldBounds[0] < -60.0 && fieldBounds[1] > 60.0, true);
  vtkDataArray* valid = field->GetPointData()->GetArray(vtkVectorFieldSampler::GetValidSampleArrayName());
  bool validOutside = false;
  for (vtkIdType pointIndex = 0; pointIndex < field->GetNumberOfPoints(); ++pointIndex)
  {
    validOutside = validOutside || (valid->GetComponent(pointIndex, 0) > 0.5 && std::abs(field->GetPoint(pointIndex)[0]) > 55.0);
  }
  CHECK_BOOL(validOutside, true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestCellData()
{
  std::cout << "Cell data" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh();
  AddPointField(surface, "Scalars", SmoothField);
  AddCellField(surface, "CellField", SmoothField);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, surface);

  // The cell arrays can be chosen alongside the point arrays
  CHECK_BOOL(HasArray(displayNode, "Scalars", 3), true);
  CHECK_BOOL(HasArray(displayNode, "CellField", 3), true);
  displayNode->SetFieldArrayName("Scalars");
  CHECK_BOOL(displayNode->IsFieldArrayCellData(), false);
  displayNode->SetFieldArrayName("CellField");
  CHECK_BOOL(displayNode->IsFieldArrayCellData(), true);
  CHECK_NOT_NULL(displayNode->GetGlyphOrientationArray());
  CHECK_POINTER(displayNode->GetGlyphOrientationArray(), surface->GetCellData()->GetArray("CellField"));
  // The colors are mapped over the magnitudes that the cells hold
  CHECK_NOT_NULL(displayNode->GetActiveScalarArray());
  double cellMagnitudeRange[2] = { 0.0, -1.0 };
  surface->GetCellData()->GetArray("CellField")->GetRange(cellMagnitudeRange, -1);
  double scalarRange[2] = { 0.0, -1.0 };
  displayNode->GetScalarRange(scalarRange);
  CHECK_DOUBLE_TOLERANCE(scalarRange[0], cellMagnitudeRange[0], 1e-9);
  CHECK_DOUBLE_TOLERANCE(scalarRange[1], cellMagnitudeRange[1], 1e-9);
  // A choice of scale factor that fits the cell vectors
  CHECK_BOOL(displayNode->ComputeDefaultGlyphScaleFactor() > 0.0, true);

  // One glyph per cell, at its center, with the vector of the cell
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);
  vtkPointSet* field = UpdateField(displayNode);
  CHECK_NOT_NULL(field);
  CHECK_INT(field->GetNumberOfPoints(), surface->GetNumberOfCells());
  vtkDataArray* vectors = field->GetPointData()->GetArray("CellField");
  CHECK_NOT_NULL(vectors);
  // The arrays used for scaling or coloring come along, averaged onto the cells
  CHECK_NOT_NULL(field->GetPointData()->GetArray("Scalars"));
  for (vtkIdType cellIndex = 0; cellIndex < surface->GetNumberOfCells(); cellIndex += 97)
  {
    double expected[3] = { 0.0, 0.0, 0.0 };
    surface->GetCellData()->GetArray("CellField")->GetTuple(cellIndex, expected);
    double actual[3] = { 0.0, 0.0, 0.0 };
    vectors->GetTuple(cellIndex, actual);
    CHECK_DOUBLE_TOLERANCE(Distance(expected, actual), 0.0, 1e-12);
  }

  // Glyphs cannot be spread over the cells: a cell data field is drawn at the cell centers,
  // which are points without cells
  CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints), true);
  CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface), false);
  CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume), false);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  CHECK_INT(displayNode->GetEffectiveSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);

  // Interpolated between the cell centers. A B-spline lattice only approximates the closely packed centers near the
  // sphere poles, so the largest error there is a few percent of the vectors; the residual is reported for that.
  displayNode->SetInterpolateBetweenPoints(true);
  double residual = displayNode->GetPipeline()->GetInterpolationResidual();
  std::cout << "  Fit residual: " << residual << std::endl;
  CHECK_BOOL(residual >= 0.0 && residual < 0.1, true);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkVectorFieldSampler:Scattered");
  field = UpdateField(displayNode);
  CHECK_NOT_NULL(field);
  CHECK_BOOL(field->GetNumberOfPoints() > 0, true);

  // A volumetric mesh with a cell data field: sampled inside the cells, and averaged onto
  // the points for isosurfaces and streamlines, which are built from point data
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddCellField(mesh, "CellField", LinearField);
  vtkMRMLVectorFieldDisplayNode* volumetricDisplayNode = AddModelWithFieldDisplay(scene, mesh);
  volumetricDisplayNode->SetFieldArrayName("CellField");
  CHECK_BOOL(volumetricDisplayNode->IsFieldArrayCellData(), true);
  volumetricDisplayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  volumetricDisplayNode->SetSamplingSpacing(10.0);
  CHECK_STD_STRING(GetFieldProducerClassName(volumetricDisplayNode), "vtkVectorFieldSampler:Mesh");
  field = UpdateField(volumetricDisplayNode);
  CHECK_NOT_NULL(field);
  vtkDataArray* sampledVectors = field->GetPointData()->GetArray(vtkVectorFieldSampler::GetVectorArrayName());
  CHECK_NOT_NULL(sampledVectors);
  CHECK_BOOL(sampledVectors->GetMaxNorm() > 0.0, true);
  // Isosurfaces are built on the cells of the mesh, wherever glyphs were placed before
  volumetricDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  CHECK_STD_STRING(GetFieldProducerClassName(volumetricDisplayNode), "vtkCellDataToPointData");
  field = UpdateField(volumetricDisplayNode);
  CHECK_NOT_NULL(field);
  CHECK_INT(field->GetNumberOfPoints(), mesh->GetNumberOfPoints());
  CHECK_NOT_NULL(field->GetPointData()->GetArray("CellField"));
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// The value at each valid sample of what the field is drawn from, compared with the
/// expected value there. Returns the largest difference.
double GetLargestColorError(vtkPointSet* field, const char* colorArrayName, double (*expected)(const double[3]))
{
  vtkDataArray* colors = field->GetPointData()->GetArray(colorArrayName);
  vtkDataArray* valid = field->GetPointData()->GetArray(vtkVectorFieldSampler::GetValidSampleArrayName());
  if (!colors)
  {
    return VTK_DOUBLE_MAX;
  }
  double largestError = 0.0;
  for (vtkIdType pointIndex = 0; pointIndex < field->GetNumberOfPoints(); ++pointIndex)
  {
    if (valid && valid->GetComponent(pointIndex, 0) == 0)
    {
      continue;
    }
    double position[3] = { 0.0, 0.0, 0.0 };
    field->GetPoint(pointIndex, position);
    largestError = std::max(largestError, std::abs(colors->GetComponent(pointIndex, 0) - expected(position)));
  }
  return largestError;
}

//----------------------------------------------------------------------------
double LinearFieldMagnitude(const double position[3])
{
  double vector[3] = { 0.0, 0.0, 0.0 };
  LinearField(position, vector);
  return vtkMath::Norm(vector);
}

//----------------------------------------------------------------------------
int TestModeVectorArrays()
{
  std::cout << "Vectors of each mode" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddPointField(mesh, "Field", LinearField);
  AddPointField(mesh, "Other", SmoothField);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, mesh);
  displayNode->SetFieldArrayName("Field");
  CHECK_STRING(displayNode->GetGlyphOrientationArrayName(), "Field");
  CHECK_STRING(displayNode->GetGlyphScaleArrayName(), "Field");

  // Glyphs can be oriented by another array, but a grid is warped, an isosurface is a level
  // of and a streamline follows the field array
  displayNode->SetGlyphOrientationArrayName("Other");
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  CHECK_STRING(displayNode->GetModeVectorArrayName(), "Other");
  for (int mode : { vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid,
                    vtkMRMLVectorFieldDisplayNode::VisualizationModeContour,
                    vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline })
  {
    displayNode->SetVisualizationMode(mode);
    CHECK_STRING(displayNode->GetModeVectorArrayName(), "Field");
  }
  // The grid is sampled from the field array
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkVectorFieldSampler:Mesh");
  vtkVectorFieldSampler* sampler = vtkVectorFieldSampler::SafeDownCast(displayNode->GetFieldConnection()->GetProducer());
  CHECK_STRING(sampler->GetMeshVectorArrayName(), "Field");
  // In glyph mode on a lattice, the glyph orientation array is sampled
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  displayNode->GetFieldConnection();
  CHECK_STRING(sampler->GetMeshVectorArrayName(), "Other");

  // Each falls back to the other when it is not set
  displayNode->SetGlyphOrientationArrayName(nullptr);
  CHECK_STRING(displayNode->GetModeVectorArrayName(), "Field");
  displayNode->SetGlyphOrientationArrayName("Other");
  displayNode->SetFieldArrayName(nullptr);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  CHECK_STRING(displayNode->GetModeVectorArrayName(), "Other");

  // The renamed attributes are saved and loaded
  displayNode->SetFieldArrayName("Field");
  displayNode->SetGlyphOrientationArrayName("Other");
  displayNode->SetGlyphScaleArrayName("Other");
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene), true);
  CHECK_BOOL(std::string(scene->GetSceneXMLString()).find("glyphOrientationArrayName=\"Other\"") != std::string::npos, true);
  CHECK_BOOL(std::string(scene->GetSceneXMLString()).find("glyphScaleArrayName=\"Other\"") != std::string::npos, true);
  vtkMRMLVectorFieldDisplayNode* reloaded = vtkMRMLVectorFieldDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLVectorFieldDisplayNode"));
  CHECK_STRING(reloaded->GetFieldArrayName(), "Field");
  CHECK_STRING(reloaded->GetGlyphOrientationArrayName(), "Other");
  CHECK_STRING(reloaded->GetGlyphScaleArrayName(), "Other");
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestNoVectorArray()
{
  std::cout << "No field without a vector array" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  // A mesh that has a scalar cell array but no vector array, such as a mesh that only stores
  // which partition of a simulation each cell belongs to
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddCellScalar(mesh, "Partition", LinearScalar);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, mesh);
  // Nothing is drawn: no glyphs are placed at the points without vectors to show
  for (int mode : { vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph,
                    vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid,
                    vtkMRMLVectorFieldDisplayNode::VisualizationModeContour,
                    vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline })
  {
    displayNode->SetVisualizationMode(mode);
    CHECK_NULL(displayNode->GetFieldConnection());
    CHECK_NULL(displayNode->GetPipeline()->GetMaskedFieldConnection());
  }
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);

  // Nor when the chosen array is not in the mesh
  displayNode->SetFieldArrayName("Missing");
  CHECK_NULL(displayNode->GetFieldConnection());

  // The field is drawn once the mesh has the array
  AddPointField(mesh, "Missing", LinearField);
  mesh->Modified();
  CHECK_NOT_NULL(displayNode->GetFieldConnection());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestColorArrays()
{
  std::cout << "Color arrays" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddPointField(mesh, "Field", LinearField);
  AddPointField(mesh, "Other", SmoothField);
  AddPointScalar(mesh, "Pressure", LinearScalar);
  AddCellScalar(mesh, "CellPressure", LinearScalar);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, mesh);
  displayNode->SetFieldArrayName("Field");
  displayNode->SetGridSpacing(20.0);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);

  // Colored by the field itself: by its magnitude, which every sampler produces
  CHECK_STRING(displayNode->GetActiveScalarName(), "Field");
  CHECK_NULL(displayNode->GetSampledColorArrayName());
  CHECK_STRING(displayNode->GetRenderedActiveScalarName(), vtkVectorFieldSampler::GetMagnitudeArrayName());

  // Colored by another array of the mesh: the sampler carries it along with the vectors
  displayNode->SetActiveScalar("Pressure", vtkAssignAttribute::POINT_DATA);
  CHECK_STRING(displayNode->GetSampledColorArrayName(), "Pressure");
  CHECK_STRING(displayNode->GetRenderedActiveScalarName(), vtkVectorFieldSampler::GetColorArrayName());
  vtkPointSet* field = UpdateField(displayNode);
  CHECK_NOT_NULL(field);
  // A linear scalar is reproduced exactly inside the cells
  CHECK_DOUBLE_TOLERANCE(GetLargestColorError(field, vtkVectorFieldSampler::GetColorArrayName(), LinearScalar), 0.0, 1e-9);
  CHECK_POINTER(displayNode->GetRenderedActiveScalarArray(), field->GetPointData()->GetArray(vtkVectorFieldSampler::GetColorArrayName()));
  // The colors are mapped over the range of the array in the mesh
  double scalarRange[2] = { 0.0, -1.0 };
  displayNode->GetScalarRange(scalarRange);
  double pressureRange[2] = { 0.0, -1.0 };
  mesh->GetPointData()->GetArray("Pressure")->GetRange(pressureRange);
  CHECK_DOUBLE_TOLERANCE(scalarRange[0], pressureRange[0], 1e-9);
  CHECK_DOUBLE_TOLERANCE(scalarRange[1], pressureRange[1], 1e-9);
  // The slice view samplers carry it too
  vtkSmartPointer<vtkVectorFieldSampler> sliceSampler = CreateSliceFieldSampler(displayNode);
  CHECK_STRING(sliceSampler->GetMeshColorArrayName(), "Pressure");

  // A display node read from a scene, before the mesh of its model is loaded: the color range
  // stored with it may belong to other data, and must follow the mesh once it arrives.
  {
    vtkMRMLModelNode* loadedModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
    vtkMRMLVectorFieldDisplayNode* loadedDisplayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorFieldDisplayNode"));
    loadedModelNode->AddAndObserveDisplayNodeID(loadedDisplayNode->GetID());
    loadedDisplayNode->SetFieldArrayName("Field");
    loadedDisplayNode->SetActiveScalar("Field", vtkAssignAttribute::POINT_DATA);
    loadedDisplayNode->SetScalarRangeFlag(vtkMRMLDisplayNode::UseDataScalarRange);
    loadedDisplayNode->SetScalarRange(0.0, 1.0);
    loadedModelNode->SetAndObserveMesh(mesh);
    CHECK_NOT_NULL(UpdateField(loadedDisplayNode));
    double magnitudeRange[2] = { 0.0, -1.0 };
    mesh->GetPointData()->GetArray("Field")->GetRange(magnitudeRange, -1);
    loadedDisplayNode->GetScalarRange(scalarRange);
    CHECK_DOUBLE_TOLERANCE(scalarRange[0], magnitudeRange[0], 1e-9);
    CHECK_DOUBLE_TOLERANCE(scalarRange[1], magnitudeRange[1], 1e-9);
  }

  // Glyphs oriented by another array, colored by the field array: the field array is not what
  // is drawn, so it is carried as a color array, and colored by its magnitude
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  displayNode->SetSamplingSpacing(10.0);
  displayNode->SetGlyphOrientationArrayName("Other");
  displayNode->SetActiveScalar("Field", vtkAssignAttribute::POINT_DATA);
  CHECK_STRING(displayNode->GetSampledColorArrayName(), "Field");
  field = UpdateField(displayNode);
  CHECK_DOUBLE_TOLERANCE(GetLargestColorError(field, vtkVectorFieldSampler::GetColorArrayName(), LinearFieldMagnitude), 0.0, 1e-9);
  displayNode->SetGlyphOrientationArrayName("Field");
  CHECK_NULL(displayNode->GetSampledColorArrayName());

  // Isosurfaces of a volumetric mesh are built on its cells, not sampled. A cell data array
  // that colors them is averaged onto the points, where the isosurfaces are interpolated.
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  displayNode->SetActiveScalar("CellPressure", vtkAssignAttribute::CELL_DATA);
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkCellDataToPointData");
  CHECK_STRING(displayNode->GetRenderedActiveScalarName(), "CellPressure");
  field = UpdateField(displayNode);
  CHECK_NOT_NULL(field->GetPointData()->GetArray("CellPressure"));
  CHECK_NOT_NULL(field->GetPointData()->GetArray("Field"));
  // A point data array needs nothing of the sort
  displayNode->SetActiveScalar("Pressure", vtkAssignAttribute::POINT_DATA);
  CHECK_POINTER(displayNode->GetFieldConnection(), vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode())->GetMeshConnection());

  // Interpolated between the points of a surface: the values are fitted too
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh();
  AddPointField(surface, "Field", SmoothField);
  AddPointScalar(surface, "Pressure", SmoothScalar);
  vtkMRMLVectorFieldDisplayNode* surfaceDisplayNode = AddModelWithFieldDisplay(scene, surface);
  surfaceDisplayNode->SetFieldArrayName("Field");
  surfaceDisplayNode->SetInterpolateBetweenPoints(true);
  surfaceDisplayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  surfaceDisplayNode->SetSamplingSpacing(10.0);
  surfaceDisplayNode->SetActiveScalar("Pressure", vtkAssignAttribute::POINT_DATA);
  CHECK_STD_STRING(GetFieldProducerClassName(surfaceDisplayNode), "vtkVectorFieldSampler:Scattered");
  vtkVectorFieldSampler* scatteredSampler = vtkVectorFieldSampler::SafeDownCast(surfaceDisplayNode->GetFieldConnection()->GetProducer());
  CHECK_NOT_NULL(scatteredSampler->GetScatteredColorFittedTransform());
  field = UpdateField(surfaceDisplayNode);
  // Valid samples reach up to the maximum distance from the points, where the fit
  // extrapolates, so the values are compared near the surface, where the data is
  vtkDataArray* fittedColors = field->GetPointData()->GetArray(vtkVectorFieldSampler::GetColorArrayName());
  CHECK_NOT_NULL(fittedColors);
  double largestColorError = 0.0;
  int numberOfSamplesNearSurface = 0;
  for (vtkIdType pointIndex = 0; pointIndex < field->GetNumberOfPoints(); ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    field->GetPoint(pointIndex, position);
    if (std::abs(vtkMath::Norm(position) - 50.0) > 3.0)
    {
      continue;
    }
    ++numberOfSamplesNearSurface;
    largestColorError = std::max(largestColorError, std::abs(fittedColors->GetComponent(pointIndex, 0) - SmoothScalar(position)));
  }
  std::cout << "  Largest error of the fitted colors near the surface: " << largestColorError << " at " << numberOfSamplesNearSurface << " samples" << std::endl;
  CHECK_BOOL(numberOfSamplesNearSurface > 0, true);
  // The values are between about 1.5 and 4.5
  CHECK_BOOL(largestColorError < 0.1, true);
  // The fit of the values is shared with the slice views, as the fit of the vectors is
  vtkSmartPointer<vtkVectorFieldSampler> surfaceSliceSampler = CreateSliceFieldSampler(surfaceDisplayNode);
  CHECK_NOT_NULL(surfaceSliceSampler);
  CHECK_POINTER(surfaceSliceSampler->GetScatteredColorFittedTransform(), scatteredSampler->GetScatteredColorFittedTransform());
  // Coloring by the field again drops it
  surfaceDisplayNode->SetActiveScalar("Field", vtkAssignAttribute::POINT_DATA);
  surfaceDisplayNode->GetFieldConnection();
  CHECK_NULL(scatteredSampler->GetScatteredColorFittedTransform());

  // A vector volume has no arrays other than its vectors to color by
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  volumeNode->SetAndObserveImageData(CreateVectorImage(LinearField));
  vtkMRMLVectorFieldDisplayNode* volumeDisplayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorFieldDisplayNode"));
  volumeNode->AddAndObserveDisplayNodeID(volumeDisplayNode->GetID());
  volumeDisplayNode->SetActiveScalar("Pressure", vtkAssignAttribute::POINT_DATA);
  CHECK_NULL(volumeDisplayNode->GetSampledColorArrayName());
  CHECK_STRING(volumeDisplayNode->GetRenderedActiveScalarName(), vtkVectorFieldSampler::GetMagnitudeArrayName());

  // The field can be colored by one component of its vectors: they are drawn from the sampled
  // vectors, and the range is the range of that component of the voxels
  volumeDisplayNode->SetActiveScalarName(vtkVectorFieldSampler::GetMagnitudeArrayName());
  CHECK_INT(volumeDisplayNode->GetActiveScalarNumberOfComponents(), 3);
  CHECK_INT(volumeDisplayNode->GetRenderedActiveScalarComponent(), 0);
  CHECK_BOOL(volumeDisplayNode->IsRenderedActiveScalarMultiComponent(), false);
  vtkDataArray* voxelVectors = volumeNode->GetImageData()->GetPointData()->GetScalars();
  for (int component = 0; component < 3; ++component)
  {
    volumeDisplayNode->SetActiveScalarComponent(component);
    CHECK_STRING(volumeDisplayNode->GetRenderedActiveScalarName(), vtkVectorFieldSampler::GetVectorArrayName());
    CHECK_INT(volumeDisplayNode->GetRenderedActiveScalarComponent(), component);
    CHECK_BOOL(volumeDisplayNode->IsRenderedActiveScalarMultiComponent(), true);
    double componentRange[2] = { 0.0, -1.0 };
    voxelVectors->GetRange(componentRange, component);
    CHECK_DOUBLE(volumeDisplayNode->GetScalarRange()[0], componentRange[0]);
    CHECK_DOUBLE(volumeDisplayNode->GetScalarRange()[1], componentRange[1]);
  }
  volumeDisplayNode->SetActiveScalarComponent(-1);
  CHECK_STRING(volumeDisplayNode->GetRenderedActiveScalarName(), vtkVectorFieldSampler::GetMagnitudeArrayName());
  double magnitudeRange[2] = { 0.0, -1.0 };
  voxelVectors->GetRange(magnitudeRange, -1);
  CHECK_DOUBLE(volumeDisplayNode->GetScalarRange()[1], magnitudeRange[1]);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestColoringSource()
{
  std::cout << "Coloring of the surface" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh();
  AddPointField(surface, "Field", SmoothField);
  AddPointScalar(surface, "Pressure", SmoothScalar);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, surface);
  displayNode->SetFieldArrayName("Field");
  displayNode->SetScalarVisibility(false);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode());
  vtkMRMLModelDisplayNode* surfaceDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelDisplayNode"));
  modelNode->AddAndObserveDisplayNodeID(surfaceDisplayNode->GetID());
  surfaceDisplayNode->SetActiveScalar("Pressure", vtkAssignAttribute::POINT_DATA);
  surfaceDisplayNode->SetScalarVisibility(true);
  surfaceDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
  surfaceDisplayNode->SetScalarRangeFlag(vtkMRMLDisplayNode::UseManualScalarRange);
  surfaceDisplayNode->SetScalarRange(1.0, 5.0);

  // Linking takes the coloring of the surface
  CHECK_NULL(displayNode->GetColoringSourceDisplayNode());
  displayNode->SetAndObserveColoringSourceDisplayNode(surfaceDisplayNode);
  CHECK_POINTER(displayNode->GetColoringSourceDisplayNode(), surfaceDisplayNode);
  CHECK_BOOL(displayNode->GetScalarVisibility() != 0, true);
  CHECK_STRING(displayNode->GetActiveScalarName(), "Pressure");
  CHECK_INT(displayNode->GetActiveAttributeLocation(), vtkAssignAttribute::POINT_DATA);
  CHECK_STRING(displayNode->GetColorNodeID(), "vtkMRMLColorTableNodeRainbow");
  CHECK_INT(displayNode->GetScalarRangeFlag(), vtkMRMLDisplayNode::UseManualScalarRange);
  CHECK_DOUBLE(displayNode->GetScalarRange()[0], 1.0);
  CHECK_DOUBLE(displayNode->GetScalarRange()[1], 5.0);

  // And follows it when it changes
  surfaceDisplayNode->SetScalarRange(2.0, 7.0);
  CHECK_DOUBLE(displayNode->GetScalarRange()[0], 2.0);
  CHECK_DOUBLE(displayNode->GetScalarRange()[1], 7.0);
  surfaceDisplayNode->SetActiveScalar("Field", vtkAssignAttribute::POINT_DATA);
  CHECK_STRING(displayNode->GetActiveScalarName(), "Field");
  surfaceDisplayNode->SetScalarVisibility(false);
  CHECK_BOOL(displayNode->GetScalarVisibility() != 0, false);
  surfaceDisplayNode->SetScalarVisibility(true);
  surfaceDisplayNode->SetActiveScalar("Pressure", vtkAssignAttribute::POINT_DATA);
  // Even a range worked out from the data here keeps the range of the surface
  displayNode->UpdateScalarRange();
  CHECK_DOUBLE(displayNode->GetScalarRange()[1], 7.0);

  // The link is saved, and followed again once the scene is loaded
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene), true);
  vtkMRMLVectorFieldDisplayNode* reloaded = vtkMRMLVectorFieldDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLVectorFieldDisplayNode"));
  CHECK_NOT_NULL(reloaded);
  vtkMRMLModelDisplayNode* reloadedSurfaceDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(reloadedScene->GetFirstNodeByClass("vtkMRMLModelDisplayNode"));
  CHECK_POINTER(reloaded->GetColoringSourceDisplayNode(), reloadedSurfaceDisplayNode);
  CHECK_STRING(reloaded->GetActiveScalarName(), "Pressure");
  CHECK_DOUBLE(reloaded->GetScalarRange()[1], 7.0);
  reloadedSurfaceDisplayNode->SetScalarRange(3.0, 9.0);
  CHECK_DOUBLE(reloaded->GetScalarRange()[1], 9.0);

  // Removing the link keeps the coloring, and the surface is no longer followed
  displayNode->SetAndObserveColoringSourceDisplayNode(nullptr);
  CHECK_NULL(displayNode->GetColoringSourceDisplayNode());
  CHECK_STRING(displayNode->GetActiveScalarName(), "Pressure");
  CHECK_DOUBLE(displayNode->GetScalarRange()[1], 7.0);
  surfaceDisplayNode->SetScalarRange(0.0, 1.0);
  CHECK_DOUBLE(displayNode->GetScalarRange()[1], 7.0);

  // A node cannot follow itself
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  displayNode->SetAndObserveColoringSourceDisplayNode(displayNode);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(displayNode->GetColoringSourceDisplayNode());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// A segmentation with one segment per sphere: a binary labelmap with 2 mm voxels over -60 to 60 mm along each axis,
/// voxels within radius of the center labeled with the segment.
vtkMRMLSegmentationNode* AddSegmentation(vtkMRMLScene* scene, const std::vector<std::pair<vtkVector3d, double>>& spheres)
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSegmentationNode"));
  segmentationNode->GetSegmentation()->SetSourceRepresentationName(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
  for (size_t sphereIndex = 0; sphereIndex < spheres.size(); ++sphereIndex)
  {
    vtkNew<vtkOrientedImageData> labelmap;
    labelmap->SetDimensions(61, 61, 61);
    labelmap->SetSpacing(2.0, 2.0, 2.0);
    labelmap->SetOrigin(-60.0, -60.0, -60.0);
    labelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    for (int k = 0; k < 61; ++k)
    {
      for (int j = 0; j < 61; ++j)
      {
        for (int i = 0; i < 61; ++i)
        {
          vtkVector3d position(-60.0 + 2.0 * i, -60.0 + 2.0 * j, -60.0 + 2.0 * k);
          bool inside = ((position - spheres[sphereIndex].first).Norm() < spheres[sphereIndex].second);
          labelmap->SetScalarComponentFromDouble(i, j, k, 0, inside ? 1.0 : 0.0);
        }
      }
    }
    vtkNew<vtkSegment> segment;
    segment->SetName(("Sphere" + std::to_string(sphereIndex)).c_str());
    segment->SetLabelValue(1);
    segment->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), labelmap);
    segmentationNode->GetSegmentation()->AddSegment(segment, "Sphere" + std::to_string(sphereIndex));
  }
  return segmentationNode;
}

//----------------------------------------------------------------------------
/// What the field is drawn from, with where it is shown marked on it.
vtkDataSet* UpdateMaskedField(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  return UpdateConnection(displayNode->GetPipeline()->GetMaskedFieldConnection());
}

//----------------------------------------------------------------------------
/// The number of field points that are shown; checks each against expected(world position): 1 must be shown,
/// 0 must not be, -1 either is right (near a boundary that is only approximated).
int CountShownPoints(vtkDataSet* field, const std::function<int(const vtkVector3d&)>& expected, vtkAbstractTransform* pointsToWorld = nullptr)
{
  vtkDataArray* inside = field->GetPointData()->GetArray(vtkMRMLVectorFieldMask::GetInsideArrayName());
  if (!inside)
  {
    std::cerr << "No " << vtkMRMLVectorFieldMask::GetInsideArrayName() << " array" << std::endl;
    return -1;
  }
  int numberOfShownPoints = 0;
  for (vtkIdType pointIndex = 0; pointIndex < field->GetNumberOfPoints(); ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    field->GetPoint(pointIndex, position);
    if (pointsToWorld)
    {
      pointsToWorld->TransformPoint(position, position);
    }
    bool shown = (inside->GetComponent(pointIndex, 0) > 0.5);
    int expectation = expected(vtkVector3d(position[0], position[1], position[2]));
    if (expectation >= 0 && shown != (expectation == 1))
    {
      std::cerr << "Point " << position[0] << " " << position[1] << " " << position[2] << " is " << (shown ? "" : "not ") << "shown" << std::endl;
      return -1;
    }
    numberOfShownPoints += (shown ? 1 : 0);
  }
  return numberOfShownPoints;
}

//----------------------------------------------------------------------------
int TestSamplingRegions()
{
  std::cout << "Sampling regions" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  displayNode->SetSamplingSpacing(20.0);
  vtkNew<vtkPoints> positions;
  vtkNew<vtkMatrix4x4> latticeToRAS;
  int latticeSize[3] = { 0, 0, 0 };

  // No node: the volume samples itself, on a lattice over its extent
  CHECK_NULL(displayNode->GetSamplingRegionNode());
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), false);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(displayNode, latticeToRAS, latticeSize), false);
  CHECK_BOOL(UpdateField(displayNode)->GetNumberOfPoints() > 0, true);
  // At random in its extent
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  displayNode->SetSamplingPointsNumber(30);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  CHECK_INT(positions->GetNumberOfPoints(), 30);
  CHECK_INT(UpdateField(displayNode)->GetNumberOfPoints(), 30);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  // Streamlines start from the same samples in the volume
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(displayNode, positions), true);
  CHECK_BOOL(positions->GetNumberOfPoints() > 0, true);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);

  // A point list: its points, all of them or some at random
  vtkMRMLMarkupsFiducialNode* pointList = AddPointList(scene, { vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(10.0, -20.0, 30.0), vtkVector3d(-40.0, 5.0, 5.0) });
  displayNode->SetAndObserveSamplingRegionNode(pointList);
  CHECK_INT(displayNode->GetEffectiveSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);
  CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume), false);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  CHECK_INT(positions->GetNumberOfPoints(), 3);
  CHECK_INT(UpdateField(displayNode)->GetNumberOfPoints(), 3);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  displayNode->SetSamplingPointsNumber(2);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  CHECK_INT(positions->GetNumberOfPoints(), 2);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  // The same points start the streamlines
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(displayNode, positions), true);
  CHECK_INT(positions->GetNumberOfPoints(), 3);
  // A grid is built over the whole field: a point list has no inside to build it in
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(displayNode, latticeToRAS, latticeSize), false);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), false);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);

  // A curve: steps along it, the sampling spacing apart
  vtkMRMLMarkupsCurveNode* curve = vtkMRMLMarkupsCurveNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsCurveNode"));
  curve->AddControlPoint(vtkVector3d(-40.0, 0.0, 0.0));
  curve->AddControlPoint(vtkVector3d(40.0, 0.0, 0.0));
  displayNode->SetAndObserveSamplingRegionNode(curve);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), true);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  CHECK_INT(positions->GetNumberOfPoints(), 5);

  // A plane: a lattice in it, or random positions in its rectangle
  vtkMRMLMarkupsPlaneNode* plane = vtkMRMLMarkupsPlaneNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsPlaneNode"));
  plane->SetCenter(0.0, 0.0, 10.0);
  plane->SetNormal(0.0, 0.0, 1.0);
  plane->SetSize(60.0, 40.0);
  displayNode->SetAndObserveSamplingRegionNode(plane);
  CHECK_INT(displayNode->GetEffectiveSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(displayNode, latticeToRAS, latticeSize), true);
  CHECK_INT(latticeSize[2], 1);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  displayNode->SetSamplingPointsNumber(25);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  CHECK_INT(positions->GetNumberOfPoints(), 25);
  for (vtkIdType pointIndex = 0; pointIndex < positions->GetNumberOfPoints(); ++pointIndex)
  {
    double* position = positions->GetPoint(pointIndex);
    CHECK_DOUBLE_TOLERANCE(position[2], 10.0, 1e-6);
    CHECK_BOOL(std::abs(position[0]) <= 30.0 + 1e-6 && std::abs(position[1]) <= 20.0 + 1e-6, true);
  }
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);

  // A plane whose rectangle is not centered on its origin, as after dragging one of its edges:
  // the lattice and the random positions are in its rectangle, and the lattice is centered in it
  {
    vtkMRMLMarkupsPlaneNode* cornerPlane = vtkMRMLMarkupsPlaneNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsPlaneNode"));
    cornerPlane->SetCenter(5.0, -5.0, 20.0);
    cornerPlane->SetNormal(0.0, 0.6, 0.8);
    cornerPlane->SetSizeMode(vtkMRMLMarkupsPlaneNode::SizeModeAbsolute);
    cornerPlane->SetPlaneBounds(-10.0, 50.0, 5.0, 45.0);
    double planeBounds[4] = { 0.0, 0.0, 0.0, 0.0 };
    cornerPlane->GetPlaneBounds(planeBounds);
    std::cout << "  Plane bounds: " << planeBounds[0] << " " << planeBounds[1] << " " << planeBounds[2] << " " << planeBounds[3] << std::endl;
    // Not centered on the origin of the plane
    CHECK_BOOL(std::abs(planeBounds[0] + planeBounds[1]) > 1.0 || std::abs(planeBounds[2] + planeBounds[3]) > 1.0, true);
    vtkNew<vtkMatrix4x4> worldToPlane;
    cornerPlane->GetObjectToWorldMatrix(worldToPlane);
    worldToPlane->Invert();
    auto checkInRectangle = [&](vtkPoints* points, double tolerance, double center_Plane[2])
    {
      center_Plane[0] = 0.0;
      center_Plane[1] = 0.0;
      for (vtkIdType pointIndex = 0; pointIndex < points->GetNumberOfPoints(); ++pointIndex)
      {
        double position_World[4] = { 0.0, 0.0, 0.0, 1.0 };
        points->GetPoint(pointIndex, position_World);
        double position_Plane[4] = { 0.0, 0.0, 0.0, 1.0 };
        worldToPlane->MultiplyPoint(position_World, position_Plane);
        if (position_Plane[0] < planeBounds[0] - tolerance || position_Plane[0] > planeBounds[1] + tolerance //
            || position_Plane[1] < planeBounds[2] - tolerance || position_Plane[1] > planeBounds[3] + tolerance
            || std::abs(position_Plane[2]) > 1e-6)
        {
          std::cerr << "Position " << position_Plane[0] << ", " << position_Plane[1] << ", " << position_Plane[2] << " is outside the plane rectangle" << std::endl;
          return false;
        }
        center_Plane[0] += position_Plane[0] / points->GetNumberOfPoints();
        center_Plane[1] += position_Plane[1] / points->GetNumberOfPoints();
      }
      return points->GetNumberOfPoints() > 0;
    };
    displayNode->SetAndObserveSamplingRegionNode(cornerPlane);
    CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(displayNode, latticeToRAS, latticeSize), true);
    vtkNew<vtkPoints> latticePositions;
    vtkVectorFieldSampler::GetLatticePositions(latticeToRAS, latticeSize, latticePositions);
    // The lattice may stick out of the rectangle by up to half a step, where the rectangle is
    // not a whole number of steps
    double spacing = displayNode->GetSamplingSpacingForField();
    double latticeCenter[2] = { 0.0, 0.0 };
    CHECK_BOOL(checkInRectangle(latticePositions, 0.5 * spacing + 1e-6, latticeCenter), true);
    CHECK_DOUBLE_TOLERANCE(latticeCenter[0], 0.5 * (planeBounds[0] + planeBounds[1]), 1e-6);
    CHECK_DOUBLE_TOLERANCE(latticeCenter[1], 0.5 * (planeBounds[2] + planeBounds[3]), 1e-6);
    displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
    vtkNew<vtkPoints> randomPositions;
    CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, randomPositions), true);
    double randomCenter[2] = { 0.0, 0.0 };
    CHECK_BOOL(checkInRectangle(randomPositions, 1e-6, randomCenter), true);
    displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  }

  // A region of interest: a lattice in its box, which needs no test of its own
  vtkMRMLMarkupsROINode* roi = AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(20.0, 20.0, 20.0));
  displayNode->SetAndObserveSamplingRegionNode(roi);
  CHECK_INT(displayNode->GetEffectiveSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplingLattice(displayNode, latticeToRAS, latticeSize), true);
  CHECK_INT(latticeSize[0], 3);
  CHECK_BOOL(displayNode->HasRegionTest(), true);

  // A closed model: its points, random points on its surface, or a lattice inside it
  vtkSmartPointer<vtkPolyData> sphere = CreateSurfaceMesh(30.0);
  vtkMRMLModelNode* sphereModel = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  sphereModel->SetAndObserveMesh(sphere);
  displayNode->SetAndObserveSamplingRegionNode(sphereModel);
  CHECK_INT(displayNode->GetEffectiveSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  CHECK_INT(positions->GetNumberOfPoints(), sphere->GetNumberOfPoints());
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  displayNode->SetSamplingPointsNumber(40);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  CHECK_BOOL(positions->GetNumberOfPoints() > 0 && positions->GetNumberOfPoints() <= 40, true);
  for (vtkIdType pointIndex = 0; pointIndex < positions->GetNumberOfPoints(); ++pointIndex)
  {
    CHECK_DOUBLE_TOLERANCE(vtkMath::Norm(positions->GetPoint(pointIndex)), 30.0, 0.5);
  }
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  displayNode->SetSamplingSpacing(5.0);
  CHECK_BOOL(displayNode->HasRegionTest(), true);
  vtkDataSet* field = UpdateMaskedField(displayNode);
  CHECK_NOT_NULL(field);
  // A sphere with 40 sides is a little inside the true one between its points
  int shownInModel = CountShownPoints(field, [](const vtkVector3d& p) { return p.Norm() < 29.0 ? 1 : (p.Norm() > 30.5 ? 0 : -1); });
  std::cout << "  Lattice samples inside the model: " << shownInModel << " of " << field->GetNumberOfPoints() << std::endl;
  CHECK_BOOL(shownInModel > 0 && shownInModel < field->GetNumberOfPoints(), true);

  // A segmentation: the inside of its segments by default, or their surfaces
  vtkMRMLSegmentationNode* segmentationNode = AddSegmentation(scene, { { vtkVector3d(0.0, 0.0, 0.0), 20.0 } });
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartDefault);
  displayNode->SetAndObserveSamplingRegionNode(segmentationNode);
  CHECK_INT(displayNode->GetEffectiveSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface), true);
  CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints), false);
  field = UpdateMaskedField(displayNode);
  CHECK_NOT_NULL(field);
  // The labelmap has 2 mm voxels, so the boundary is that coarse
  int shownInSegment = CountShownPoints(field, [](const vtkVector3d& p) { return p.Norm() < 18.0 ? 1 : (p.Norm() > 22.0 ? 0 : -1); });
  std::cout << "  Lattice samples inside the segment: " << shownInSegment << " of " << field->GetNumberOfPoints() << std::endl;
  CHECK_BOOL(shownInSegment > 0 && shownInSegment < field->GetNumberOfPoints(), true);

  // The surface of the segments, without giving the segmentation a closed surface
  // representation that it did not have
  const std::string closedSurfaceName = vtkSegmentationConverter::GetClosedSurfaceRepresentationName();
  CHECK_BOOL(segmentationNode->GetSegmentation()->ContainsRepresentation(closedSurfaceName), false);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  vtkNew<vtkPoints> surfacePositions_RAS;
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, surfacePositions_RAS), true);
  CHECK_BOOL(surfacePositions_RAS->GetNumberOfPoints() > 0, true);
  CHECK_BOOL(segmentationNode->GetSegmentation()->ContainsRepresentation(closedSurfaceName), false);
  auto isOnSphere = [](vtkPoints* positions, const vtkVector3d& center)
  {
    for (vtkIdType pointIndex = 0; pointIndex < positions->GetNumberOfPoints(); ++pointIndex)
    {
      double* p = positions->GetPoint(pointIndex);
      double distance = (vtkVector3d(p[0], p[1], p[2]) - center).Norm();
      if (distance < 17.0 || distance > 23.0)
      {
        return false;
      }
    }
    return true;
  };
  CHECK_BOOL(isOnSphere(surfacePositions_RAS, vtkVector3d(0.0, 0.0, 0.0)), true);
  // The surface is kept, and computed again where the segmentation is moved to
  segmentationNode->SetAndObserveTransformNodeID(AddTranslation(scene, 0.0, 15.0, 0.0)->GetID());
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, surfacePositions_RAS), true);
  CHECK_BOOL(isOnSphere(surfacePositions_RAS, vtkVector3d(0.0, 15.0, 0.0)), true);
  // A closed surface that the segmentation has is used as it is
  segmentationNode->SetAndObserveTransformNodeID(nullptr);
  segmentationNode->CreateClosedSurfaceRepresentation();
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, surfacePositions_RAS), true);
  CHECK_BOOL(isOnSphere(surfacePositions_RAS, vtkVector3d(0.0, 0.0, 0.0)), true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestMaskingRegion()
{
  std::cout << "Masking region" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  displayNode->SetSamplingSpacing(10.0);
  CHECK_BOOL(displayNode->HasRegionTest(), false);
  // Without a region nothing is marked
  CHECK_NULL(UpdateMaskedField(displayNode)->GetPointData()->GetArray(vtkMRMLVectorFieldMask::GetInsideArrayName()));

  // A region of interest: shown inside it, or outside of it
  vtkMRMLMarkupsROINode* roi = AddROI(scene, vtkVector3d(10.0, 0.0, 0.0), vtkVector3d(15.0, 25.0, 35.0));
  auto insideROI = [](const vtkVector3d& p) { return std::abs(p[0] - 10.0) <= 15.0 && std::abs(p[1]) <= 25.0 && std::abs(p[2]) <= 35.0; };
  displayNode->SetAndObserveMaskingRegionNode(roi);
  CHECK_BOOL(displayNode->HasRegionTest(), true);
  int shownInside = CountShownPoints(UpdateMaskedField(displayNode), insideROI);
  CHECK_BOOL(shownInside > 0, true);
  displayNode->MaskingShowInsideOff();
  int shownOutside = CountShownPoints(UpdateMaskedField(displayNode), [&insideROI](const vtkVector3d& p) { return !insideROI(p); });
  CHECK_BOOL(shownOutside > 0, true);
  CHECK_INT(shownInside + shownOutside, static_cast<int>(UpdateMaskedField(displayNode)->GetNumberOfPoints()));
  displayNode->MaskingShowInsideOn();
  // Moving the region tests the points again
  roi->SetXYZ(-10.0, 0.0, 0.0);
  CHECK_BOOL(
    CountShownPoints(UpdateMaskedField(displayNode), [](const vtkVector3d& p) { return std::abs(p[0] + 10.0) <= 15.0 && std::abs(p[1]) <= 25.0 && std::abs(p[2]) <= 35.0; }) > 0,
    true);

  // Some of the segments of a segmentation
  vtkMRMLSegmentationNode* segmentationNode = AddSegmentation(scene, { { vtkVector3d(-25.0, 0.0, 0.0), 15.0 }, { vtkVector3d(25.0, 0.0, 0.0), 15.0 } });
  displayNode->SetAndObserveMaskingRegionNode(segmentationNode);
  vtkDataSet* field = UpdateMaskedField(displayNode);
  vtkDataArray* inside = field->GetPointData()->GetArray(vtkMRMLVectorFieldMask::GetInsideArrayName());
  auto countShown = [&field, &inside](double x)
  {
    int count = 0;
    for (vtkIdType pointIndex = 0; pointIndex < field->GetNumberOfPoints(); ++pointIndex)
    {
      double* p = field->GetPoint(pointIndex);
      count += (inside->GetComponent(pointIndex, 0) > 0.5 && std::abs(p[0] - x) < 12.0 && std::abs(p[1]) < 12.0 && std::abs(p[2]) < 12.0 ? 1 : 0);
    }
    return count;
  };
  // All segments when none is chosen
  CHECK_BOOL(countShown(-25.0) > 0 && countShown(25.0) > 0, true);
  displayNode->SetMaskingSegmentIDs({ "Sphere1" });
  field = UpdateMaskedField(displayNode);
  inside = field->GetPointData()->GetArray(vtkMRMLVectorFieldMask::GetInsideArrayName());
  CHECK_INT(countShown(-25.0), 0);
  CHECK_BOOL(countShown(25.0) > 0, true);

  // A segment with a labelmap at -25 and a closed surface at +25: the source representation decides
  vtkMRMLSegmentationNode* twoRepresentationsNode = AddSegmentation(scene, { { vtkVector3d(-25.0, 0.0, 0.0), 15.0 } });
  vtkSmartPointer<vtkPolyData> sphereAtRight = CreateSurfaceMesh(15.0, 32);
  for (vtkIdType pointIndex = 0; pointIndex < sphereAtRight->GetNumberOfPoints(); ++pointIndex)
  {
    double* p = sphereAtRight->GetPoint(pointIndex);
    sphereAtRight->GetPoints()->SetPoint(pointIndex, p[0] + 25.0, p[1], p[2]);
  }
  twoRepresentationsNode->GetSegmentation()->GetSegment("Sphere0")->AddRepresentation(vtkSegmentationConverter::GetClosedSurfaceRepresentationName(), sphereAtRight);
  displayNode->SetMaskingSegmentIDs({});
  displayNode->SetAndObserveMaskingRegionNode(twoRepresentationsNode);
  field = UpdateMaskedField(displayNode);
  inside = field->GetPointData()->GetArray(vtkMRMLVectorFieldMask::GetInsideArrayName());
  CHECK_BOOL(countShown(-25.0) > 0, true);
  CHECK_INT(countShown(25.0), 0);
  // Changing the source drops the other representations, so the labelmap is added back as a derived one
  vtkSegment* twoRepresentationsSegment = twoRepresentationsNode->GetSegmentation()->GetSegment("Sphere0");
  vtkSmartPointer<vtkDataObject> labelmapAtLeft = twoRepresentationsSegment->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
  twoRepresentationsNode->GetSegmentation()->SetSourceRepresentationName(vtkSegmentationConverter::GetClosedSurfaceRepresentationName());
  twoRepresentationsSegment->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), labelmapAtLeft);
  CHECK_NOT_NULL(twoRepresentationsSegment->GetRepresentation(vtkSegmentationConverter::GetClosedSurfaceRepresentationName()));
  field = UpdateMaskedField(displayNode);
  inside = field->GetPointData()->GetArray(vtkMRMLVectorFieldMask::GetInsideArrayName());
  CHECK_INT(countShown(-25.0), 0);
  CHECK_BOOL(countShown(25.0) > 0, true);

  // A volume under a transform: the samples are in the coordinates of the volume, the region
  // is in the world
  vtkMRMLLinearTransformNode* transformNode = AddTranslation(scene, 100.0, 0.0, 0.0);
  displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(transformNode->GetID());
  roi->SetXYZ(100.0, 0.0, 0.0);
  displayNode->SetAndObserveMaskingRegionNode(roi);
  field = UpdateMaskedField(displayNode);
  int shownMoved = CountShownPoints(field, [](const vtkVector3d& p) { return std::abs(p[0] - 100.0) <= 15.0 && std::abs(p[1]) <= 25.0 && std::abs(p[2]) <= 35.0; }, //
                                     transformNode->GetTransformToParent());
  CHECK_BOOL(shownMoved > 0, true);
  displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(nullptr);
  roi->SetXYZ(0.0, 0.0, 0.0);

  // A closed model under two transforms: editing the outer transform moves the region, which
  // modifies neither the model, nor its parent transform node, nor the outer transform node
  vtkMRMLModelNode* maskModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  maskModelNode->SetAndObserveMesh(CreateSurfaceMesh(15.0, 32));
  vtkMRMLLinearTransformNode* innerTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
  vtkMRMLLinearTransformNode* outerTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
  innerTransformNode->SetAndObserveTransformNodeID(outerTransformNode->GetID());
  maskModelNode->SetAndObserveTransformNodeID(innerTransformNode->GetID());
  displayNode->SetAndObserveMaskingRegionNode(maskModelNode);
  auto insideSphereAt = [](double x)
  {
    return [x](const vtkVector3d& p)
    {
      double distance = (p - vtkVector3d(x, 0.0, 0.0)).Norm();
      return (distance < 13.5 ? 1 : (distance > 16.5 ? 0 : -1));
    };
  };
  CHECK_BOOL(CountShownPoints(UpdateMaskedField(displayNode), insideSphereAt(0.0)) > 0, true);
  vtkNew<vtkTransform> outerTranslation;
  outerTranslation->Translate(25.0, 0.0, 0.0);
  outerTransformNode->SetMatrixTransformToParent(outerTranslation->GetMatrix());
  CHECK_BOOL(CountShownPoints(UpdateMaskedField(displayNode), insideSphereAt(25.0)) > 0, true);
  displayNode->SetAndObserveMaskingRegionNode(roi);

  // The modes that draw lines and surfaces are cut where they leave the region
  for (int mode : { vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid, vtkMRMLVectorFieldDisplayNode::VisualizationModeContour })
  {
    displayNode->SetVisualizationMode(mode);
    double levels[2] = { 1.2, 1.5 };
    displayNode->SetContourLevels(levels, 2);
    displayNode->SetGridSpacing(10.0);
    displayNode->SetSamplingSpacing(5.0);
    displayNode->SetGridLineDiameter(0.0);
    displayNode->SetAndObserveMaskingRegionNode(nullptr);
    vtkPolyData* geometry = UpdateConnection<vtkPolyData>(displayNode->GetPipeline()->GetOutputPolyDataConnection());
    CHECK_NOT_NULL(geometry);
    vtkIdType allPoints = geometry->GetNumberOfPoints();
    displayNode->SetAndObserveMaskingRegionNode(roi);
    geometry = UpdateConnection<vtkPolyData>(displayNode->GetPipeline()->GetOutputPolyDataConnection());
    std::cout << "  " << displayNode->GetVisualizationModeAsString() << ": " << allPoints << " points, " << geometry->GetNumberOfPoints() << " in the region" << std::endl;
    CHECK_BOOL(geometry->GetNumberOfPoints() > 0 && geometry->GetNumberOfPoints() < allPoints, true);
    for (vtkIdType pointIndex = 0; pointIndex < geometry->GetNumberOfPoints(); ++pointIndex)
    {
      // The cut is between the samples, so it can be up to one sample out (grid: warped, too)
      double* p = geometry->GetPoint(pointIndex);
      CHECK_BOOL(std::abs(p[1]) <= 25.0 + 10.0 && std::abs(p[2]) <= 35.0 + 10.0, true);
    }
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestVectorVolume()
{
  std::cout << "Vector volume" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  volumeNode->SetAndObserveImageData(CreateVectorImage(LinearField));
  vtkMRMLVectorFieldDisplayNode* displayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorFieldDisplayNode"));
  volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  // A sampled image always gives the same arrays, so there is nothing to choose
  CHECK_BOOL(displayNode->HasFixedFieldArrays(), true);
  CHECK_BOOL(displayNode->CanSampleAtArbitraryPositions(), true);
  // Only a mesh has points of its own, and only a mesh is interpolated between them
  CHECK_BOOL(displayNode->CanInterpolateBetweenPoints(), false);
  // A volume is sampled inside its extent by default, or at its voxels; it has no surface
  CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints), true);
  CHECK_BOOL(displayNode->IsSamplingRegionPartSupported(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface), false);
  CHECK_INT(displayNode->GetEffectiveSamplingRegionPart(), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::IsSamplingOwnPoints(displayNode), false);
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkVectorFieldSampler:Image");
  CHECK_INT(CreateSliceFieldSampler(displayNode)->GetFieldSource(), vtkVectorFieldSampler::FieldSourceImage);

  // Random samples are counted, not spaced; in a regular pattern the spacing is the spacing of
  // the glyphs, and in streamline mode the spacing of the seeds
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), false);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), true);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  volumeNode->SetSpacing(2.0, 1.5, 3.0);
  displayNode->SetSamplingSpacing(10.0);
  CHECK_DOUBLE(displayNode->GetSamplingSpacing(), 10.0);
  // They are traced through the voxels directly, not through a field sampled from them,
  // which would have to be sampled and searched again for every time point of a sequence
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkImageFieldSource");
  CHECK_BOOL(displayNode->IsFieldSampled(), true);
  vtkPointSet* streamlineField = UpdateField(displayNode);
  CHECK_NULL(streamlineField); // an image, not a point set
  vtkDataSet* streamlineImage = displayNode->GetSampledFieldDataSet();
  CHECK_NOT_NULL(vtkImageData::SafeDownCast(streamlineImage));
  CHECK_NOT_NULL(streamlineImage->GetPointData()->GetArray(vtkVectorFieldSampler::GetVectorArrayName()));
  CHECK_NOT_NULL(streamlineImage->GetPointData()->GetArray(vtkVectorFieldSampler::GetMagnitudeArrayName()));
  // The streamlines are traced once and shared: the same connection every time, and the 3D view tubes are made from it
  // Without a node they start from samples in the whole volume
  CHECK_NOT_NULL(displayNode->GetPipeline()->GetStreamlineConnection());
  vtkNew<vtkPoints> volumeSeeds;
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(displayNode, volumeSeeds), true);
  CHECK_BOOL(volumeSeeds->GetNumberOfPoints() > 0, true);
  // A fine lattice over the volume gives all of its points as seeds; vtkMRMLVectorFieldPipeline limits
  // how many of them are traced (vtkMRMLVectorFieldGeometryTest1)
  {
    double samplingSpacing = displayNode->GetSamplingSpacing();
    int samplingPattern = displayNode->GetSamplingPattern();
    displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
    double volumeBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    displayNode->GetDisplayableNode()->GetRASBounds(volumeBounds);
    double sizeMm = volumeBounds[1] - volumeBounds[0];
    // 40 steps along the first axis: tens of thousands of positions
    displayNode->SetSamplingSpacing(sizeMm / 40.0);
    vtkNew<vtkPoints> fineSeeds;
    CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(displayNode, fineSeeds), true);
    CHECK_BOOL(fineSeeds->GetNumberOfPoints() > 5000, true);
    // A random subset of them is spread over the whole volume
    vtkNew<vtkPoints> seedSubset;
    vtkMRMLVectorFieldSamplePointSource::SelectRandomSubset(fineSeeds, 5000, seedSubset);
    CHECK_INT(static_cast<int>(seedSubset->GetNumberOfPoints()), 5000);
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    seedSubset->GetBounds(bounds);
    for (int axis = 0; axis < 3; ++axis)
    {
      CHECK_BOOL(bounds[2 * axis] < volumeBounds[2 * axis] + 0.1 * sizeMm && bounds[2 * axis + 1] > volumeBounds[2 * axis + 1] - 0.1 * sizeMm, true);
    }
    displayNode->SetSamplingSpacing(samplingSpacing);
    displayNode->SetSamplingPattern(samplingPattern);
  }
  displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(0.0, 0.0, 0.0) }));
  vtkAlgorithmOutput* streamlineConnection = displayNode->GetPipeline()->GetStreamlineConnection();
  CHECK_NOT_NULL(streamlineConnection);
  CHECK_POINTER(displayNode->GetPipeline()->GetStreamlineConnection(), streamlineConnection);
  vtkAlgorithmOutput* tubesConnection = displayNode->GetPipeline()->GetOutputPolyDataConnection();
  CHECK_NOT_NULL(tubesConnection);
  // Tubes and lines are two outputs of the same geometry filter, so the tubes are made from the shared lines
  CHECK_POINTER(tubesConnection->GetProducer(), streamlineConnection->GetProducer());
  CHECK_INT(streamlineConnection->GetIndex(), vtkVectorFieldGeometry::StreamlineLinesPort);
  // All 3D views get the same tubes
  CHECK_POINTER(displayNode->GetPipeline()->GetOutputPolyDataConnection(), tubesConnection);
  // Whatever they start from, they are traced through the voxels
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsROINode"));
  displayNode->SetAndObserveSamplingRegionNode(roiNode);
  CHECK_STD_STRING(GetFieldProducerClassName(displayNode), "vtkImageFieldSource");
  displayNode->SetAndObserveSamplingRegionNode(nullptr);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  volumeNode->SetSpacing(1.0, 1.0, 1.0);

  // The colors are mapped over the magnitudes of the voxel vectors, before anything has been
  // sampled. An empty range ([0, -1]) would make every mapper report a bad lookup table range.
  double voxelMagnitudeRange[2] = { 0.0, -1.0 };
  volumeNode->GetImageData()->GetPointData()->GetScalars()->GetRange(voxelMagnitudeRange, -1);
  double scalarRange[2] = { 0.0, -1.0 };
  displayNode->GetScalarRange(scalarRange);
  CHECK_DOUBLE_TOLERANCE(scalarRange[0], voxelMagnitudeRange[0], 1e-9);
  CHECK_DOUBLE_TOLERANCE(scalarRange[1], voxelMagnitudeRange[1], 1e-9);
  CHECK_BOOL(scalarRange[0] < scalarRange[1], true);

  // A new image (a sequence browsed to another time point) brings its own range
  vtkSmartPointer<vtkImageData> scaledImage = CreateVectorImage(LinearField);
  vtkDataArray* scaledVectors = scaledImage->GetPointData()->GetScalars();
  for (vtkIdType tupleIndex = 0; tupleIndex < scaledVectors->GetNumberOfTuples(); ++tupleIndex)
  {
    double vector[3] = { 0.0, 0.0, 0.0 };
    scaledVectors->GetTuple(tupleIndex, vector);
    scaledVectors->SetTuple3(tupleIndex, 3.0 * vector[0], 3.0 * vector[1], 3.0 * vector[2]);
  }
  volumeNode->SetAndObserveImageData(scaledImage);
  displayNode->GetFieldConnection(); // what a view does when the image changes
  displayNode->GetScalarRange(scalarRange);
  CHECK_DOUBLE_TOLERANCE(scalarRange[1], 3.0 * voxelMagnitudeRange[1], 1e-9);

  // Every way of choosing the color range works without the field having to be sampled:
  // working out the range must not ask for the field, which works out the range again
  displayNode->SetScalarRangeFlag(vtkMRMLDisplayNode::UseManualScalarRange);
  displayNode->SetScalarRange(0.5, 1.5);
  CHECK_NOT_NULL(displayNode->GetFieldConnection());
  displayNode->GetScalarRange(scalarRange);
  CHECK_DOUBLE(scalarRange[0], 0.5);
  CHECK_DOUBLE(scalarRange[1], 1.5);
  displayNode->SetActiveScalarName("SomethingElse");
  displayNode->SetScalarRangeFlag(vtkMRMLDisplayNode::UseDataScalarRange);
  CHECK_NOT_NULL(displayNode->GetFieldConnection());
  displayNode->SetActiveScalarName(vtkVectorFieldSampler::GetMagnitudeArrayName());
  displayNode->GetFieldConnection();
  displayNode->GetScalarRange(scalarRange);
  CHECK_DOUBLE_TOLERANCE(scalarRange[1], 3.0 * voxelMagnitudeRange[1], 1e-9);

  // Voxel vectors are RAS directions regardless of the voxel axes orientation and the voxel vector type. Here the voxel
  // axes are permuted and flipped, as in a sagittal acquisition, and every voxel holds (1, 0, 0).
  vtkNew<vtkImageData> constantImage;
  constantImage->SetDimensions(5, 5, 5);
  constantImage->AllocateScalars(VTK_DOUBLE, 3);
  vtkDataArray* constantVectors = constantImage->GetPointData()->GetScalars();
  for (vtkIdType tupleIndex = 0; tupleIndex < constantVectors->GetNumberOfTuples(); ++tupleIndex)
  {
    constantVectors->SetTuple3(tupleIndex, 1.0, 0.0, 0.0);
  }
  volumeNode->SetAndObserveImageData(constantImage);
  double directions[3][3] = { { 0.0, 0.0, -1.0 }, { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 } };
  volumeNode->SetIJKToRASDirections(directions);
  volumeNode->SetSpacing(2.0, 2.0, 2.0);
  CHECK_INT(volumeNode->GetVoxelVectorType(), vtkMRMLVolumeNode::VoxelVectorTypeUndefined);
  for (int voxelVectorType : { vtkMRMLVolumeNode::VoxelVectorTypeUndefined, vtkMRMLVolumeNode::VoxelVectorTypeSpatial })
  {
    volumeNode->SetVoxelVectorType(voxelVectorType);
    vtkPointSet* field = UpdateField(displayNode);
    CHECK_NOT_NULL(field);
    vtkDataArray* sampledVectors = field->GetPointData()->GetArray(vtkVectorFieldSampler::GetVectorArrayName());
    CHECK_NOT_NULL(sampledVectors);
    CHECK_BOOL(sampledVectors->GetNumberOfTuples() > 0, true);
    double sampled[3] = { 0.0, 0.0, 0.0 };
    sampledVectors->GetTuple(0, sampled);
    CHECK_DOUBLE_TOLERANCE(sampled[0], 1.0, 1e-6);
    CHECK_DOUBLE_TOLERANCE(sampled[1], 0.0, 1e-6);
    CHECK_DOUBLE_TOLERANCE(sampled[2], 0.0, 1e-6);
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// A field under a transform: it is sampled where the sampling region is in the world, and what is drawn from it is
/// in field coordinates, which the 3D view moves into the world.
int TestTransformedSource()
{
  std::cout << "Transformed source" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  vtkMRMLLinearTransformNode* transformNode = AddTranslation(scene, 200.0, 0.0, 0.0);
  CHECK_NULL(displayNode->GetPipeline()->GetFieldToWorldTransform());
  displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(transformNode->GetID());
  CHECK_NOT_NULL(displayNode->GetPipeline()->GetFieldToWorldTransform());
  {
    // A slice sampler updated after the volume moved to another parent uses the new parent
    vtkSmartPointer<vtkVectorFieldSampler> sliceSampler = CreateSliceFieldSampler(displayNode);
    CHECK_NOT_NULL(sliceSampler.GetPointer());
    displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(AddTranslation(scene, -100.0, 0.0, 0.0)->GetID());
    CHECK_BOOL(displayNode->UpdateSliceFieldSampler(sliceSampler), true);
    double origin[3] = { 0.0, 0.0, 0.0 };
    double origin_World[3] = { 0.0, 0.0, 0.0 };
    sliceSampler->GetFieldToWorldTransform()->TransformPoint(origin, origin_World);
    std::cout << "  Slice sampler field origin in world after reparenting: " << origin_World[0] << std::endl;
    CHECK_DOUBLE_TOLERANCE(origin_World[0], -100.0, 1e-6);
    displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(transformNode->GetID());
  }
  // Inside the volume, with or without a margin of half a voxel
  auto isInVolume = [](const double p[3], double shiftX) { return std::abs(p[0] - shiftX) <= 55.0 && std::abs(p[1]) <= 55.0 && std::abs(p[2]) <= 55.0; };

  // Random samples inside the volume itself are where the volume is in the world
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  displayNode->SetSamplingPointsNumber(100);
  vtkNew<vtkPoints> samplePositions_RAS;
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, samplePositions_RAS), true);
  CHECK_INT(static_cast<int>(samplePositions_RAS->GetNumberOfPoints()), 100);
  for (vtkIdType pointIndex = 0; pointIndex < samplePositions_RAS->GetNumberOfPoints(); ++pointIndex)
  {
    CHECK_BOOL(isInVolume(samplePositions_RAS->GetPoint(pointIndex), 200.0), true);
  }
  // and the field is sampled there, in its own coordinates
  vtkDataSet* field = UpdateMaskedField(displayNode);
  CHECK_NOT_NULL(field);
  CHECK_INT(static_cast<int>(field->GetNumberOfPoints()), 100);
  for (vtkIdType pointIndex = 0; pointIndex < field->GetNumberOfPoints(); ++pointIndex)
  {
    CHECK_BOOL(isInVolume(field->GetPoint(pointIndex), 0.0), true);
  }

  // A point in the world
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(210.0, 0.0, 0.0) }));
  field = UpdateMaskedField(displayNode);
  CHECK_NOT_NULL(field);
  CHECK_INT(static_cast<int>(field->GetNumberOfPoints()), 1);
  CHECK_DOUBLE_TOLERANCE(field->GetPoint(0)[0], 10.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(field->GetPoint(0)[1], 0.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(field->GetPoint(0)[2], 0.0, 1e-6);

  // Streamlines start from the point
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  vtkPolyData* streamlines = UpdateConnection<vtkPolyData>(displayNode->GetPipeline()->GetOutputPolyDataConnection());
  CHECK_NOT_NULL(streamlines);
  CHECK_BOOL(streamlines->GetNumberOfPoints() > 1, true);
  for (vtkIdType pointIndex = 0; pointIndex < streamlines->GetNumberOfPoints(); ++pointIndex)
  {
    CHECK_BOOL(isInVolume(streamlines->GetPoint(pointIndex), 0.0), true);
  }

  // A lattice over a region of interest in the world
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  displayNode->SetSamplingSpacing(10.0);
  vtkMRMLMarkupsROINode* roi = AddROI(scene, vtkVector3d(200.0, 0.0, 0.0), vtkVector3d(20.0, 20.0, 20.0));
  displayNode->SetAndObserveSamplingRegionNode(roi);
  field = UpdateMaskedField(displayNode);
  CHECK_NOT_NULL(field);
  CHECK_BOOL(field->GetNumberOfPoints() > 0, true);
  for (vtkIdType pointIndex = 0; pointIndex < field->GetNumberOfPoints(); ++pointIndex)
  {
    double* p = field->GetPoint(pointIndex);
    CHECK_BOOL(std::abs(p[0]) <= 20.0 + 1e-6 && std::abs(p[1]) <= 20.0 + 1e-6 && std::abs(p[2]) <= 20.0 + 1e-6, true);
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// The 3D view glyph points are selected once, by display node filters that all 3D views share, and moved into the
/// world there if the parent transform is not linear; a linear one is left to the views.
int TestGlyphPoints()
{
  std::cout << "Glyph points" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  // A lattice 10 mm apart in a 40 mm box: 5 x 5 x 5 points
  displayNode->SetSamplingSpacing(10.0);
  vtkMRMLMarkupsROINode* roi = AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(20.0, 20.0, 20.0));
  displayNode->SetAndObserveSamplingRegionNode(roi);

  vtkAlgorithmOutput* glyphPointsConnection = displayNode->GetGlyphPointsConnection();
  CHECK_NOT_NULL(glyphPointsConnection);
  // Shared: every view that asks gets the same filters
  CHECK_POINTER(displayNode->GetGlyphPointsConnection(), glyphPointsConnection);
  vtkDataSet* glyphPoints = UpdateConnection(glyphPointsConnection);
  CHECK_NOT_NULL(glyphPoints);
  CHECK_INT(static_cast<int>(glyphPoints->GetNumberOfPoints()), 125);
  // Only in glyph mode
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_NULL(displayNode->GetGlyphPointsConnection());
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);

  // The threshold removes the points whose magnitude is out of range
  displayNode->SetThresholdEnabled(true);
  displayNode->SetThresholdRange(1e6, 2e6);
  CHECK_INT(static_cast<int>(UpdateConnection(displayNode->GetGlyphPointsConnection())->GetNumberOfPoints()), 0);
  displayNode->SetThresholdEnabled(false);
  CHECK_INT(static_cast<int>(UpdateConnection(displayNode->GetGlyphPointsConnection())->GetNumberOfPoints()), 125);

  // Directionally scaled glyphs carry their per-axis scale
  displayNode->SetGlyphType(vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow);
  displayNode->SetGlyphDiameterAbsolute(true);
  displayNode->SetGlyphScaleDirectional(true);
  CHECK_BOOL(vtkMRMLVectorFieldPipeline::IsDirectionalScaleUsed(displayNode), true);
  glyphPoints = UpdateConnection(displayNode->GetGlyphPointsConnection());
  vtkDataArray* directionalScale = glyphPoints->GetPointData()->GetArray(vtkVectorFieldGlyph::GetDirectionalScaleArrayName());
  CHECK_NOT_NULL(directionalScale);
  CHECK_INT(directionalScale->GetNumberOfComponents(), 3);
  CHECK_DOUBLE_TOLERANCE(directionalScale->GetComponent(0, 1), 1.0, 1e-9);
  displayNode->SetGlyphScaleDirectional(false);
  CHECK_NULL(UpdateConnection(displayNode->GetGlyphPointsConnection())->GetPointData()->GetArray(vtkVectorFieldGlyph::GetDirectionalScaleArrayName()));

  // The glyph geometry is shared as well, and only generated again when its settings change.
  // It is the shape output of the glyph filter that the glyph points come from.
  vtkAlgorithmOutput* glyphSourceConnection = displayNode->GetGlyphSourceConnection();
  CHECK_NOT_NULL(glyphSourceConnection);
  CHECK_POINTER(displayNode->GetGlyphSourceConnection(), glyphSourceConnection);
  CHECK_POINTER(glyphSourceConnection->GetProducer(), displayNode->GetGlyphFilter());
  CHECK_INT(glyphSourceConnection->GetIndex(), vtkVectorFieldGlyph::GlyphShapePort);
  vtkDataSet* arrow = UpdateConnection(glyphSourceConnection);
  CHECK_NOT_NULL(arrow);
  vtkMTimeType arrowMTime = arrow->GetMTime();
  CHECK_POINTER(displayNode->GetGlyphSourceConnection(), glyphSourceConnection);
  CHECK_INT(static_cast<int>(UpdateConnection(glyphSourceConnection)->GetMTime()), static_cast<int>(arrowMTime));
  double arrowBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  arrow->GetBounds(arrowBounds);
  // One unit long along +X
  CHECK_DOUBLE_TOLERANCE(arrowBounds[1] - arrowBounds[0], 1.0, 1e-6);
  displayNode->SetGlyphResolution(displayNode->GetGlyphResolution() + 2);
  CHECK_POINTER(displayNode->GetGlyphSourceConnection(), glyphSourceConnection);
  CHECK_BOOL(UpdateConnection(glyphSourceConnection)->GetMTime() > arrowMTime, true);
  // Every shape lies along +X; a cylinder is rotated to lie along +X too
  displayNode->SetGlyphType(vtkMRMLVectorFieldDisplayNode::GlyphTypeCylinder);
  CHECK_POINTER(displayNode->GetGlyphSourceConnection(), glyphSourceConnection);
  double cylinderBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  UpdateConnection(glyphSourceConnection)->GetBounds(cylinderBounds);
  CHECK_DOUBLE_TOLERANCE(cylinderBounds[1] - cylinderBounds[0], 1.0, 1e-6);
  displayNode->SetGlyphType(vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow);
  CHECK_POINTER(displayNode->GetGlyphSourceConnection(), glyphSourceConnection);

  // Glyphs at three points in the world, near x = 200. Under a transform they are sampled
  // where the points are in the coordinates of the field.
  vtkMRMLMarkupsFiducialNode* pointListNode = AddPointList(scene, { vtkVector3d(190.0, 0.0, 0.0), vtkVector3d(200.0, 10.0, 0.0), vtkVector3d(210.0, 0.0, -10.0) });
  displayNode->SetAndObserveSamplingRegionNode(pointListNode);
  auto checkPointsAroundX = [](vtkDataSet* points, double centerX)
  {
    for (vtkIdType pointIndex = 0; pointIndex < points->GetNumberOfPoints(); ++pointIndex)
    {
      double* p = points->GetPoint(pointIndex);
      if (std::abs(p[0] - centerX) > 10.0 + 1e-3 || std::abs(p[1]) > 10.0 + 1e-3 || std::abs(p[2]) > 10.0 + 1e-3)
      {
        std::cerr << "Point " << p[0] << ", " << p[1] << ", " << p[2] << " is not within 10 mm of x = " << centerX << std::endl;
        return false;
      }
    }
    return points->GetNumberOfPoints() == 3;
  };

  // Under a linear transform the points stay in the coordinates of the field: the views
  // apply the transform to the actor
  vtkMRMLLinearTransformNode* linearTransformNode = AddTranslation(scene, 200.0, 0.0, 0.0);
  displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(linearTransformNode->GetID());
  CHECK_NULL(displayNode->GetPipeline()->GetNonLinearFieldToWorldTransform());
  CHECK_BOOL(checkPointsAroundX(UpdateConnection(displayNode->GetGlyphPointsConnection()), 0.0), true);

  // The exported glyphs are in the world: the linear transform that the views apply to the
  // actor is applied to the mesh. Every glyph is there, as a copy of the glyph shape.
  {
    displayNode->SetGlyphScaleFactor(0.5);
    vtkNew<vtkPolyData> glyphMesh;
    CHECK_BOOL(displayNode->GetVisualizationMesh(glyphMesh), true);
    vtkIdType shapePoints = UpdateConnection(displayNode->GetGlyphSourceConnection())->GetNumberOfPoints();
    CHECK_INT(static_cast<int>(glyphMesh->GetNumberOfPoints()), static_cast<int>(3 * shapePoints));
    CHECK_BOOL(glyphMesh->GetNumberOfPolys() > 0, true);
    double meshBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    glyphMesh->GetBounds(meshBounds);
    std::cout << "  Exported glyphs along x: " << meshBounds[0] << " to " << meshBounds[1] << std::endl;
    CHECK_BOOL(meshBounds[0] > 150.0 && meshBounds[1] < 250.0, true);
    // The views draw from a filter of their own, which does not build the geometry
    CHECK_BOOL(displayNode->GetGlyphFilter()->GetGenerateGeometry(), false);
    displayNode->SetGlyphScaleFactor(1.0);
  }

  // Under a non-linear transform they are moved into the world, the glyph points as well as
  // the grid
  vtkMRMLTransformNode* nonLinearTransformNode = AddNonLinearTranslation(scene, 200.0);
  displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(nonLinearTransformNode->GetID());
  CHECK_NOT_NULL(displayNode->GetPipeline()->GetNonLinearFieldToWorldTransform());
  CHECK_BOOL(checkPointsAroundX(UpdateConnection(displayNode->GetGlyphPointsConnection()), 200.0), true);
  // A grid over the whole volume, which is from -50 to 50 mm in its own coordinates
  displayNode->SetAndObserveSamplingRegionNode(nullptr);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  displayNode->SetGridSpacing(10.0);
  displayNode->SetGridScaleFactor(0.0);
  // Lines rather than tubes, which would add their radius to the bounds
  displayNode->SetGridLineDiameter(0.0);
  vtkAlgorithmOutput* gridConnection = displayNode->GetPipeline()->GetOutputPolyDataConnection();
  CHECK_POINTER(displayNode->GetPipeline()->GetOutputPolyDataConnection(), gridConnection);
  double gridBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  UpdateConnection(gridConnection)->GetBounds(gridBounds);
  std::cout << "  Grid under a non-linear transform along x: " << gridBounds[0] << " to " << gridBounds[1] << std::endl;
  CHECK_DOUBLE_TOLERANCE(gridBounds[0], 150.0, 1e-3);
  CHECK_DOUBLE_TOLERANCE(gridBounds[1], 250.0, 1e-3);
  // and under a linear one it is not
  displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(linearTransformNode->GetID());
  UpdateConnection(displayNode->GetPipeline()->GetOutputPolyDataConnection())->GetBounds(gridBounds);
  CHECK_DOUBLE_TOLERANCE(gridBounds[0], -50.0, 1e-3);
  CHECK_DOUBLE_TOLERANCE(gridBounds[1], 50.0, 1e-3);
  // Exported, the grid is in the world under a linear transform too, and so are isosurfaces
  {
    vtkNew<vtkPolyData> exported;
    CHECK_BOOL(displayNode->GetVisualizationMesh(exported), true);
    double exportedBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    exported->GetBounds(exportedBounds);
    CHECK_DOUBLE_TOLERANCE(exportedBounds[0], 150.0, 1e-3);
    CHECK_DOUBLE_TOLERANCE(exportedBounds[1], 250.0, 1e-3);
    displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
    double levels[1] = { 1.0 };
    displayNode->SetContourLevels(levels, 1);
    CHECK_BOOL(displayNode->GetVisualizationMesh(exported), true);
    CHECK_BOOL(exported->GetNumberOfPolys() > 0, true);
    exported->GetBounds(exportedBounds);
    std::cout << "  Exported isosurface along x: " << exportedBounds[0] << " to " << exportedBounds[1] << std::endl;
    CHECK_BOOL(exportedBounds[0] >= 150.0 - 1e-3 && exportedBounds[1] <= 250.0 + 1e-3, true);
    displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  }
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);

  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
/// Sample positions computed again on every update (here the points of a point list) do not make the sampler
/// sample again, as long as they are the same.
int TestSamplePositionsKeptWhenUnchanged()
{
  std::cout << "Sample positions kept when unchanged" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  vtkMRMLMarkupsFiducialNode* pointList = AddPointList(scene, { vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(10.0, 5.0, -5.0) });
  displayNode->SetAndObserveSamplingRegionNode(pointList);
  vtkVectorFieldSampler* sampler = vtkVectorFieldSampler::SafeDownCast(displayNode->GetFieldConnection()->GetProducer());
  CHECK_NOT_NULL(sampler);
  sampler->Update();
  CHECK_INT(static_cast<int>(sampler->GetOutput()->GetNumberOfPoints()), 2);
  vtkMTimeType sampledTime = sampler->GetOutput()->GetMTime();
  displayNode->GetFieldConnection();
  sampler->Update();
  CHECK_BOOL(sampler->GetOutput()->GetMTime() == sampledTime, true);
  // A moved point is sampled again
  pointList->SetNthControlPointPosition(1, 20.0, 5.0, -5.0);
  displayNode->GetFieldConnection();
  sampler->Update();
  CHECK_BOOL(sampler->GetOutput()->GetMTime() > sampledTime, true);
  return EXIT_SUCCESS;
}

namespace
{
//----------------------------------------------------------------------------
/// Regularly on a surface: its points about the sampling spacing apart
int TestSurfaceSamplingSpacing()
{
  vtkNew<vtkMRMLScene> scene;

  // The surface of the mesh that carries the field: glyphs on some of its boundary points, none inside
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh(20, 50.0);
  AddPointField(mesh, "Field", LinearField);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, mesh);
  displayNode->SetFieldArrayName("Field");
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), false);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), true);
  displayNode->SetSamplingSpacing(10.0);
  vtkDataSet* glyphPoints = UpdateConnection(displayNode->GetGlyphPointsConnection());
  CHECK_NOT_NULL(glyphPoints);
  vtkIdType pointsAt10Mm = glyphPoints->GetNumberOfPoints();
  std::cout << "Mesh surface at 10 mm spacing: " << pointsAt10Mm << " of " << mesh->GetNumberOfPoints() << " points" << std::endl;
  CHECK_BOOL(pointsAt10Mm > 0 && pointsAt10Mm < mesh->GetNumberOfPoints(), true);
  for (vtkIdType pointIndex = 0; pointIndex < pointsAt10Mm; ++pointIndex)
  {
    double* p = glyphPoints->GetPoint(pointIndex);
    CHECK_DOUBLE_TOLERANCE(std::max(std::abs(p[0]), std::max(std::abs(p[1]), std::abs(p[2]))), 50.0, 1e-3);
  }
  displayNode->SetSamplingSpacing(25.0);
  glyphPoints = UpdateConnection(displayNode->GetGlyphPointsConnection());
  CHECK_BOOL(glyphPoints->GetNumberOfPoints() > 0 && glyphPoints->GetNumberOfPoints() < pointsAt10Mm, true);
  // Random sampling has a number of points instead
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), false);
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);

  // The surface of another model
  vtkMRMLModelNode* regionNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  vtkSmartPointer<vtkPolyData> regionSurface = CreateSurfaceMesh(30.0, 40);
  regionNode->SetAndObserveMesh(regionSurface);
  displayNode->SetAndObserveSamplingRegionNode(regionNode);
  displayNode->SetSamplingRegionPart(vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  CHECK_BOOL(displayNode->IsSamplingSpacingUsed(), true);
  vtkNew<vtkPoints> positions;
  displayNode->SetSamplingSpacing(5.0);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  vtkIdType positionsAt5Mm = positions->GetNumberOfPoints();
  displayNode->SetSamplingSpacing(15.0);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  vtkIdType positionsAt15Mm = positions->GetNumberOfPoints();
  std::cout << "Other surface: " << positionsAt5Mm << " points at 5 mm, " << positionsAt15Mm << " at 15 mm, of " << regionSurface->GetNumberOfPoints() << std::endl;
  CHECK_BOOL(positionsAt15Mm > 0 && positionsAt15Mm < positionsAt5Mm && positionsAt5Mm < regionSurface->GetNumberOfPoints(), true);
  // They are points of the surface
  const double origin[3] = { 0.0, 0.0, 0.0 };
  for (vtkIdType pointIndex = 0; pointIndex < positionsAt15Mm; ++pointIndex)
  {
    CHECK_DOUBLE_TOLERANCE(Distance(positions->GetPoint(pointIndex), origin), 30.0, 1e-3);
  }
  // No spacing is the same as every point
  displayNode->SetSamplingSpacing(0.0);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetSamplePositions(displayNode, positions), true);
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), static_cast<int>(regionSurface->GetNumberOfPoints()));
  return EXIT_SUCCESS;
}
} // namespace

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldDisplayNodeTest1(int, char*[])
{
  // Segment surfaces are made from the labelmaps of the test segmentations
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New());

  vtkNew<vtkMRMLVectorFieldDisplayNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_EXIT_SUCCESS(TestEnumStrings());
  CHECK_EXIT_SUCCESS(TestModeProperties());
  CHECK_EXIT_SUCCESS(TestSaveAndLoad());
  CHECK_EXIT_SUCCESS(TestSurfaceMesh());
  CHECK_EXIT_SUCCESS(TestVolumetricMesh());
  CHECK_EXIT_SUCCESS(TestCellData());
  CHECK_EXIT_SUCCESS(TestVectorVolume());
  CHECK_EXIT_SUCCESS(TestModeVectorArrays());
  CHECK_EXIT_SUCCESS(TestNoVectorArray());
  CHECK_EXIT_SUCCESS(TestColorArrays());
  CHECK_EXIT_SUCCESS(TestColoringSource());
  CHECK_EXIT_SUCCESS(TestSamplingRegions());
  CHECK_EXIT_SUCCESS(TestMaskingRegion());
  CHECK_EXIT_SUCCESS(TestTransformedSource());
  CHECK_EXIT_SUCCESS(TestSamplePositionsKeptWhenUnchanged());
  CHECK_EXIT_SUCCESS(TestGlyphPoints());
  CHECK_EXIT_SUCCESS(TestSurfaceSamplingSpacing());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
