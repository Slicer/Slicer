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

// Checks that grid, contour and streamline modes build geometry for every source offering them: vector volume, volumetric
// mesh (point and cell data), interpolated surface mesh; and that a mesh drawn as grid then streamlines is released.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLVectorFieldPipeline.h"
#include "vtkMRMLVectorFieldSamplePointSource.h"
#include "vtkVectorFieldSampler.h"
#include "vtkMRMLVectorFieldTestingHelpers.h"
#include "vtkMRMLVectorVolumeNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkCell.h>
#include <vtkLogger.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkVector.h>

// STD includes
#include <algorithm>
#include <cmath>
#include <string>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

//----------------------------------------------------------------------------
/// The geometry that the current visualization mode draws in 3D views.
vtkPolyData* UpdateModeGeometry(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  return UpdateConnection<vtkPolyData>(displayNode->GetPipeline()->GetOutputPolyDataConnection());
}

//----------------------------------------------------------------------------
/// Every mode that is not glyphs draws something for this display node.
int CheckModes(vtkMRMLVectorFieldDisplayNode* displayNode, const std::string& description)
{
  std::cout << description << std::endl;
  // Plain lines rather than tubes, so that what is drawn can be counted as lines. Slice
  // views always draw them this way.
  displayNode->SetGridLineDiameter(0.0);
  displayNode->SetStreamlineTubeDiameter(0.0);
  // Glyphs are drawn by the displayable managers, not as poly data
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  CHECK_NULL(displayNode->GetPipeline()->GetOutputPolyDataConnection());

  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  vtkPolyData* grid = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(grid);
  std::cout << "  Grid: " << grid->GetNumberOfLines() << " lines" << std::endl;
  CHECK_BOOL(grid->GetNumberOfLines() > 0, true);

  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  vtkPolyData* contour = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(contour);
  std::cout << "  Contour: " << contour->GetNumberOfCells() << " cells" << std::endl;
  CHECK_BOOL(contour->GetNumberOfCells() > 0, true);

  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  vtkPolyData* streamlines = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(streamlines);
  std::cout << "  Streamlines: " << streamlines->GetNumberOfLines() << " lines, " << streamlines->GetNumberOfPoints() << " points" << std::endl;
  CHECK_BOOL(streamlines->GetNumberOfLines() > 0, true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestVectorVolume(bool obliqueVoxelAxes)
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (obliqueVoxelAxes)
  {
    // Voxel axes permuted and flipped (sagittal acquisition stored in LPS). Without a region the field is sampled on the
    // voxel grid, so the cells that the modes are built on are not RAS-aligned, and the grid is left-handed.
    double directions[3][3] = { { 0.0, 0.0, -1.0 }, { 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 } };
    volumeNode->SetIJKToRASDirections(directions);
    volumeNode->SetOrigin(50.0, -50.0, 50.0);
  }
  displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(-20.0, 10.0, 5.0) }));
  displayNode->SetGridSpacing(20.0);
  displayNode->SetSamplingSpacing(10.0);
  // The magnitude of the linear field is between about 0.6 and 2.4 over the volume
  double levels[2] = { 1.2, 1.5 };
  displayNode->SetContourLevels(levels, 2);
  CHECK_EXIT_SUCCESS(CheckModes(displayNode, obliqueVoxelAxes ? "Vector volume, oblique voxel axes" : "Vector volume"));

  // The threshold hides the parts of the streamlines where the magnitude is out of range
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  vtkIdType allPoints = UpdateModeGeometry(displayNode)->GetNumberOfPoints();
  displayNode->SetThresholdEnabled(true);
  displayNode->SetThresholdRange(100.0, 200.0); // the magnitudes are between about 0.6 and 2.4
  CHECK_INT(UpdateModeGeometry(displayNode)->GetNumberOfPoints(), 0);
  displayNode->SetThresholdRange(0.0, 1.2);
  vtkIdType thresholdedPoints = UpdateModeGeometry(displayNode)->GetNumberOfPoints();
  std::cout << "  Streamline points: " << allPoints << ", " << thresholdedPoints << " with magnitude up to 1.2" << std::endl;
  CHECK_BOOL(thresholdedPoints > 0 && thresholdedPoints < allPoints, true);
  displayNode->SetThresholdEnabled(false);
  CHECK_INT(UpdateModeGeometry(displayNode)->GetNumberOfPoints(), allPoints);

  // Updated again without a change, the streamlines are not traced again: the seeds are
  // only replaced when they move
  vtkPolyData* streamlines = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(streamlines);
  vtkMTimeType tracedTime = streamlines->GetMTime();
  CHECK_BOOL(UpdateModeGeometry(displayNode)->GetMTime() == tracedTime, true);

  // The cells that connect the samples are only built by the modes that draw them
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_NOT_NULL(UpdateModeGeometry(displayNode));
  vtkVectorFieldSampler* sampler = vtkVectorFieldSampler::SafeDownCast(displayNode->GetFieldConnection()->GetProducer());
  CHECK_NOT_NULL(sampler);
  CHECK_BOOL(sampler->GetGenerateCells(), true);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  CHECK_NOT_NULL(displayNode->GetFieldConnection());
  CHECK_BOOL(sampler->GetGenerateCells(), false);

  // Within a region that is rotated, the field is sampled on a lattice that is not aligned
  // with the RAS axes, and every mode is still built on it
  vtkMRMLMarkupsROINode* roiNode = AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(35.0, 35.0, 35.0));
  vtkNew<vtkTransform> roiRotation;
  roiRotation->RotateZ(30.0);
  roiRotation->RotateX(20.0);
  roiNode->SetAndObserveObjectToNodeMatrix(roiRotation->GetMatrix());
  displayNode->SetAndObserveSamplingRegionNode(roiNode);
  displayNode->SetSamplingSpacing(5.0);
  // Streamlines start at a few random positions in it
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  displayNode->SetSamplingPointsNumber(10);
  CHECK_EXIT_SUCCESS(CheckModes(displayNode, "  ...within a rotated region"));
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestMaximumNumberOfStreamlineSeeds()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  displayNode->SetStreamlineTubeDiameter(0.0);
  // One line per seed
  displayNode->SetStreamlineBidirectional(false);
  // Without a sampling region node, the seeds are a lattice over the whole volume
  displayNode->SetSamplingSpacing(5.0);
  vtkNew<vtkPoints> seedPositions;
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetStreamlineSeedPositions(displayNode, seedPositions), true);
  const vtkIdType numberOfSeeds = seedPositions->GetNumberOfPoints();

  vtkMRMLVectorFieldPipeline* modePipeline = displayNode->GetPipeline();
  CHECK_NOT_NULL(modePipeline);
  CHECK_INT(modePipeline->GetMaximumNumberOfStreamlineSeeds(), 5000);
  CHECK_BOOL(numberOfSeeds > modePipeline->GetMaximumNumberOfStreamlineSeeds(), true);
  vtkIdType defaultLines = UpdateModeGeometry(displayNode)->GetNumberOfLines();

  // Fewer seeds are traced when the limit is lowered
  modePipeline->SetMaximumNumberOfStreamlineSeeds(10);
  CHECK_INT(modePipeline->GetMaximumNumberOfStreamlineSeeds(), 10);
  displayNode->Modified();
  vtkIdType limitedLines = UpdateModeGeometry(displayNode)->GetNumberOfLines();

  // All of them when it is raised above the number of seeds
  modePipeline->SetMaximumNumberOfStreamlineSeeds(static_cast<int>(numberOfSeeds));
  displayNode->Modified();
  vtkIdType allLines = UpdateModeGeometry(displayNode)->GetNumberOfLines();
  std::cout << "Streamline seeds: " << numberOfSeeds << "; lines: " << defaultLines << " with the default limit, " << limitedLines << " with a limit of 10, " << allLines
            << " without a limit" << std::endl;
  CHECK_BOOL(limitedLines > 0 && limitedLines <= 10, true);
  CHECK_BOOL(defaultLines > limitedLines && defaultLines <= 5000, true);
  CHECK_BOOL(allLines > defaultLines, true);

  // At least one seed is traced
  modePipeline->SetMaximumNumberOfStreamlineSeeds(0);
  CHECK_INT(modePipeline->GetMaximumNumberOfStreamlineSeeds(), 1);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestVolumetricMesh(bool cellData)
{
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  if (cellData)
  {
    AddCellField(mesh, "Field", LinearField);
  }
  else
  {
    AddPointField(mesh, "Field", LinearField);
  }
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, mesh);
  displayNode->SetFieldArrayName("Field");
  displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(-20.0, 10.0, 5.0) }));
  displayNode->SetGridSpacing(20.0);
  displayNode->SetSamplingSpacing(10.0);
  double levels[2] = { 1.2, 1.5 };
  displayNode->SetContourLevels(levels, 2);
  return CheckModes(displayNode, cellData ? "Volumetric mesh, cell data" : "Volumetric mesh, point data");
}

//----------------------------------------------------------------------------
int TestInterpolatedSurfaceMesh()
{
  vtkNew<vtkMRMLScene> scene;
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh();
  AddPointField(surface, "Field", SmoothField);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddModelWithFieldDisplay(scene, surface);
  displayNode->SetFieldArrayName("Field");
  // Streamlines start near the surface, where the field is drawn
  displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(50.0, 0.0, 0.0), vtkVector3d(0.0, -50.0, 0.0), vtkVector3d(0.0, 0.0, 50.0) }));
  displayNode->SetGridSpacing(10.0);
  displayNode->SetSamplingSpacing(5.0);

  // Without interpolation a surface mesh has nothing to build these modes from
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(displayNode->IsVisualizationModeSupported(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid), false);

  displayNode->SetInterpolateBetweenPoints(true);
  return CheckModes(displayNode, "Surface mesh, interpolated between points");
}

//----------------------------------------------------------------------------
/// The grid can be filled with the surface of the lattice it is drawn on, deformed and colored
/// as the lines are
int TestGridFill()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  displayNode->SetGridSpacing(20.0);
  displayNode->SetSamplingSpacing(10.0);
  displayNode->SetGridLineDiameter(0.0);
  // Undeformed, so that the fill is the surface of the sampled box
  displayNode->SetGridScaleFactor(0.0);

  // Lines only by default
  vtkPolyData* grid = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(grid);
  CHECK_BOOL(grid->GetNumberOfLines() > 0, true);
  CHECK_INT(static_cast<int>(grid->GetNumberOfPolys()), 0);
  vtkIdType numberOfLines = grid->GetNumberOfLines();
  double linesBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  grid->GetBounds(linesBounds);

  // Filled: the lines are kept, and the surface of the box that they are drawn on is added
  displayNode->SetGridFill(true);
  grid = UpdateModeGeometry(displayNode);
  CHECK_INT(static_cast<int>(grid->GetNumberOfLines()), static_cast<int>(numberOfLines));
  std::cout << "  Grid fill: " << grid->GetNumberOfPolys() << " polygons" << std::endl;
  CHECK_BOOL(grid->GetNumberOfPolys() > 0, true);
  double filledBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  grid->GetBounds(filledBounds);
  for (int i = 0; i < 6; ++i)
  {
    CHECK_DOUBLE_TOLERANCE(filledBounds[i], linesBounds[i], 1e-6);
  }
  // Colored by the same values as the lines
  CHECK_NOT_NULL(grid->GetPointData()->GetArray(vtkVectorFieldSampler::GetMagnitudeArrayName()));

  // Deformed as the lines are
  displayNode->SetGridScaleFactor(1.0);
  grid = UpdateModeGeometry(displayNode);
  double deformedBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  grid->GetBounds(deformedBounds);
  CHECK_BOOL(std::abs(deformedBounds[1] - filledBounds[1]) > 1e-3 || std::abs(deformedBounds[0] - filledBounds[0]) > 1e-3, true);

  // Cut where the field is not shown, as the lines are
  vtkMRMLMarkupsROINode* maskNode = AddROI(scene, vtkVector3d(25.0, 0.0, 0.0), vtkVector3d(25.0, 100.0, 100.0));
  displayNode->SetAndObserveMaskingRegionNode(maskNode);
  displayNode->SetGridScaleFactor(0.0);
  grid = UpdateModeGeometry(displayNode);
  grid->GetBounds(filledBounds);
  CHECK_BOOL(grid->GetNumberOfPolys() > 0, true);
  // The mask starts at x = 0, between samples at x = -10 and 0: the lines and the fill are
  // cut halfway between them
  std::cout << "  Masked grid fill along x: " << filledBounds[0] << " to " << filledBounds[1] << std::endl;
  CHECK_DOUBLE_TOLERANCE(filledBounds[0], -5.0, 1e-3);

  displayNode->SetGridFill(false);
  grid = UpdateModeGeometry(displayNode);
  CHECK_INT(static_cast<int>(grid->GetNumberOfPolys()), 0);
  double maskedLinesBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  grid->GetBounds(maskedLinesBounds);
  CHECK_DOUBLE_TOLERANCE(maskedLinesBounds[0], filledBounds[0], 1e-3);

  // The grid lines can be hidden: the fill alone, the undeformed grid alone, or nothing
  displayNode->SetAndObserveMaskingRegionNode(nullptr);
  displayNode->SetGridScaleFactor(1.0);
  displayNode->SetGridLinesVisibility(false);
  displayNode->SetGridFill(true);
  grid = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(grid);
  CHECK_INT(static_cast<int>(grid->GetNumberOfLines()), 0);
  CHECK_BOOL(grid->GetNumberOfPolys() > 0, true);
  displayNode->SetGridFill(false);
  displayNode->SetGridShowNonWarped(true);
  grid = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(grid);
  CHECK_INT(static_cast<int>(grid->GetNumberOfLines()), 0);
  CHECK_INT(static_cast<int>(grid->GetNumberOfPolys()), 0);
  vtkPolyData* nonWarpedGrid = UpdateConnection<vtkPolyData>(displayNode->GetPipeline()->GetNonWarpedGridConnection());
  CHECK_NOT_NULL(nonWarpedGrid);
  CHECK_INT(static_cast<int>(nonWarpedGrid->GetNumberOfLines()), static_cast<int>(numberOfLines));
  // undeformed: where the lines were without deformation
  double nonWarpedBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  nonWarpedGrid->GetBounds(nonWarpedBounds);
  for (int i = 0; i < 6; ++i)
  {
    CHECK_DOUBLE_TOLERANCE(nonWarpedBounds[i], linesBounds[i], 1e-6);
  }
  displayNode->SetGridShowNonWarped(false);
  CHECK_NULL(displayNode->GetPipeline()->GetOutputPolyDataConnection());
  CHECK_NULL(displayNode->GetPipeline()->GetNonWarpedGridConnection());
  // Both the deformed and the undeformed lines, apart so that they can have different colors
  displayNode->SetGridLinesVisibility(true);
  displayNode->SetGridShowNonWarped(true);
  grid = UpdateModeGeometry(displayNode);
  CHECK_INT(static_cast<int>(grid->GetNumberOfLines()), static_cast<int>(numberOfLines));
  nonWarpedGrid = UpdateConnection<vtkPolyData>(displayNode->GetPipeline()->GetNonWarpedGridConnection());
  CHECK_INT(static_cast<int>(nonWarpedGrid->GetNumberOfLines()), static_cast<int>(numberOfLines));
  // An exported mesh has both
  vtkNew<vtkPolyData> exportedMesh;
  CHECK_BOOL(displayNode->GetVisualizationMesh(exportedMesh), true);
  CHECK_INT(static_cast<int>(exportedMesh->GetNumberOfLines()), static_cast<int>(2 * numberOfLines));
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// A field that points along +X everywhere
void UniformXField(const double vtkNotUsed(position)[3], double vector[3])
{
  vector[0] = 1.0;
  vector[1] = 0.0;
  vector[2] = 0.0;
}

//----------------------------------------------------------------------------
/// The masking region limits where streamlines are integrated: a streamline stops where it
/// leaves what the region shows, and does not go on behind a part that is hidden.
int TestStreamlineMasking()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(UniformXField));
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  displayNode->SetStreamlineTubeDiameter(0.0);
  displayNode->SetStreamlineBidirectional(false);
  displayNode->SetStreamlineMaximumPropagation(200.0);
  // Two seeds on the left of a wall at x = -5..5, one on its right
  displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(-40.0, 0.0, 0.0), vtkVector3d(-40.0, 10.0, 0.0), vtkVector3d(20.0, -10.0, 0.0) }));

  auto xRange = [](vtkPolyData* lines, double range[2])
  {
    range[0] = VTK_DOUBLE_MAX;
    range[1] = VTK_DOUBLE_MIN;
    for (vtkIdType pointIndex = 0; pointIndex < lines->GetNumberOfPoints(); ++pointIndex)
    {
      range[0] = std::min(range[0], lines->GetPoint(pointIndex)[0]);
      range[1] = std::max(range[1], lines->GetPoint(pointIndex)[0]);
    }
  };

  // Without a mask they run through the whole volume
  vtkPolyData* streamlines = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(streamlines);
  CHECK_INT(static_cast<int>(streamlines->GetNumberOfLines()), 3);
  double range[2] = { 0.0, -1.0 };
  xRange(streamlines, range);
  CHECK_BOOL(range[1] > 45.0, true);

  // The wall hides the field: the streamlines that reach it stop there, and are not drawn again
  // behind it. The one that starts behind it is not affected.
  vtkMRMLMarkupsROINode* wall = AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(5.0, 100.0, 100.0));
  displayNode->SetAndObserveMaskingRegionNode(wall);
  displayNode->MaskingShowInsideOff();
  streamlines = UpdateModeGeometry(displayNode);
  CHECK_INT(static_cast<int>(streamlines->GetNumberOfLines()), 3);
  int linesBehindTheWall = 0;
  for (vtkIdType lineIndex = 0; lineIndex < streamlines->GetNumberOfCells(); ++lineIndex)
  {
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    streamlines->GetCell(lineIndex)->GetBounds(bounds);
    std::cout << "  Streamline " << lineIndex << " along x: " << bounds[0] << " to " << bounds[1] << std::endl;
    if (bounds[0] < -5.0)
    {
      // Started in front of the wall: stops within an integration step (1 mm at most) of the end
      // of the last cell that is shown entirely (the voxels are 10 mm apart and the one at x = 0
      // is hidden, so the cells are shown up to x = -10)
      CHECK_BOOL(bounds[1] < -3.0, true);
      CHECK_BOOL(bounds[1] > -11.5, true);
    }
    else
    {
      ++linesBehindTheWall;
      CHECK_BOOL(bounds[0] > 5.0, true);
      CHECK_BOOL(bounds[1] > 45.0, true);
    }
  }
  CHECK_INT(linesBehindTheWall, 1);

  // Showing only the inside of the wall: the seeds are outside and no cell is entirely inside, so the field is empty.
  // The stream tracer must not be run on an empty field: it logs an error per thread and crashes at random.
  displayNode->MaskingShowInsideOn();
  int loggedErrors = 0;
  vtkLogger::AddCallback("StreamlineMaskingErrors", [](void* userData, const vtkLogger::Message&) { ++*static_cast<int*>(userData); }, &loggedErrors, vtkLogger::VERBOSITY_ERROR);
  streamlines = UpdateModeGeometry(displayNode);
  vtkLogger::RemoveCallback("StreamlineMaskingErrors");
  CHECK_INT(loggedErrors, 0);
  CHECK_INT(static_cast<int>(streamlines ? streamlines->GetNumberOfLines() : 0), 0);

  // Moving the wall traces them again
  displayNode->MaskingShowInsideOff();
  wall->SetXYZ(-20.0, 0.0, 0.0);
  streamlines = UpdateModeGeometry(displayNode);
  xRange(streamlines, range);
  for (vtkIdType lineIndex = 0; lineIndex < streamlines->GetNumberOfCells(); ++lineIndex)
  {
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    streamlines->GetCell(lineIndex)->GetBounds(bounds);
    if (bounds[0] < -25.0)
    {
      CHECK_BOOL(bounds[1] < -23.0, true);
    }
  }

  // Without the mask they run through again
  displayNode->SetAndObserveMaskingRegionNode(nullptr);
  streamlines = UpdateModeGeometry(displayNode);
  xRange(streamlines, range);
  CHECK_BOOL(range[1] > 45.0, true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Under a warping transform the streamlines are traced in the coordinates of the volume, where the seeds are taken
/// to, the masking region is tested in the world, and the streamlines are moved into the world afterwards.
int TestStreamlinesUnderWarp()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(UniformXField));
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  displayNode->SetStreamlineTubeDiameter(0.0);
  displayNode->SetStreamlineBidirectional(false);
  displayNode->SetStreamlineMaximumPropagation(200.0);

  // Everything is stretched to twice its size along x: the volume is from -100 to 100 mm in the world. The transform
  // is not a linear one for the pipeline (a thin-plate spline reproduces the affine part of its landmark displacements
  // exactly).
  const double stretch = 2.0;
  vtkNew<vtkPoints> sourceLandmarks;
  vtkNew<vtkPoints> targetLandmarks;
  const double landmarks[5][3] = { { 0, 0, 0 }, { 100, 0, 0 }, { 0, 100, 0 }, { 0, 0, 100 }, { 100, 100, 100 } };
  for (const double* landmark : landmarks)
  {
    sourceLandmarks->InsertNextPoint(landmark);
    targetLandmarks->InsertNextPoint(stretch * landmark[0], landmark[1], landmark[2]);
  }
  vtkNew<vtkThinPlateSplineTransform> thinPlateSpline;
  thinPlateSpline->SetBasisToR();
  thinPlateSpline->SetSourceLandmarks(sourceLandmarks);
  thinPlateSpline->SetTargetLandmarks(targetLandmarks);
  vtkMRMLTransformNode* warpNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  warpNode->SetAndObserveTransformToParent(thinPlateSpline);
  displayNode->GetDisplayableNode()->SetAndObserveTransformNodeID(warpNode->GetID());
  CHECK_NOT_NULL(displayNode->GetPipeline()->GetNonLinearFieldToWorldTransform());

  // Seeds in the world, two on the left of a wall and one on its right. In the volume they are at x = -20 and 10 mm.
  displayNode->SetAndObserveSamplingRegionNode(AddPointList(scene, { vtkVector3d(-40.0, 0.0, 0.0), vtkVector3d(-40.0, 10.0, 0.0), vtkVector3d(20.0, -10.0, 0.0) }));

  // Without a mask the streamlines start at the seeds and run to the end of the volume in the world
  vtkPolyData* streamlines = UpdateModeGeometry(displayNode);
  CHECK_NOT_NULL(streamlines);
  CHECK_INT(static_cast<int>(streamlines->GetNumberOfLines()), 3);
  for (vtkIdType lineIndex = 0; lineIndex < streamlines->GetNumberOfCells(); ++lineIndex)
  {
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    streamlines->GetCell(lineIndex)->GetBounds(bounds);
    std::cout << "  Streamline " << lineIndex << " under a warp along x: " << bounds[0] << " to " << bounds[1] << std::endl;
    CHECK_BOOL(std::abs(bounds[0] + 40.0) < 0.01 || std::abs(bounds[0] - 20.0) < 0.01, true);
    CHECK_BOOL(bounds[1] > 95.0, true);
  }

  // A wall in the world at x = -5..5 hides the voxels at x = 0 of the volume (-2.5..2.5 mm there), so its cells are
  // shown up to x = -10 mm, which is -20 mm in the world. The streamlines that reach it stop within an integration
  // step of that (1 mm at most, 2 mm in the world), and the one that starts behind it is not affected.
  displayNode->SetAndObserveMaskingRegionNode(AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(5.0, 100.0, 100.0)));
  displayNode->MaskingShowInsideOff();
  streamlines = UpdateModeGeometry(displayNode);
  CHECK_INT(static_cast<int>(streamlines->GetNumberOfLines()), 3);
  int linesBehindTheWall = 0;
  for (vtkIdType lineIndex = 0; lineIndex < streamlines->GetNumberOfCells(); ++lineIndex)
  {
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    streamlines->GetCell(lineIndex)->GetBounds(bounds);
    std::cout << "  Masked streamline " << lineIndex << " under a warp along x: " << bounds[0] << " to " << bounds[1] << std::endl;
    if (bounds[0] < -5.0)
    {
      CHECK_BOOL(bounds[1] < -19.0, true);
      CHECK_BOOL(bounds[1] > -23.0, true);
    }
    else
    {
      ++linesBehindTheWall;
      CHECK_BOOL(bounds[0] > 5.0, true);
      CHECK_BOOL(bounds[1] > 95.0, true);
    }
  }
  CHECK_INT(linesBehindTheWall, 1);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldGeometryTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestVectorVolume(false));
  CHECK_EXIT_SUCCESS(TestVectorVolume(true));
  CHECK_EXIT_SUCCESS(TestMaximumNumberOfStreamlineSeeds());
  CHECK_EXIT_SUCCESS(TestVolumetricMesh(false));
  CHECK_EXIT_SUCCESS(TestVolumetricMesh(true));
  CHECK_EXIT_SUCCESS(TestInterpolatedSurfaceMesh());
  CHECK_EXIT_SUCCESS(TestStreamlineMasking());
  CHECK_EXIT_SUCCESS(TestStreamlinesUnderWarp());
  CHECK_EXIT_SUCCESS(TestGridFill());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
