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

// Checks that each vector field sampler produces its source's field at the requested positions, with RAS vectors,
// and marks samples where the source has no value invalid.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLVectorFieldTestingHelpers.h"
#include "vtkVectorFieldSampler.h"

// vtkAddon includes
#include <vtkScatteredBSplineTransform.h>

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtkTransform.h>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

//----------------------------------------------------------------------------
vtkDataArray* GetSampledVectors(vtkVectorFieldSampler* sampler)
{
  return sampler->GetOutput()->GetPointData()->GetArray(vtkVectorFieldSampler::GetVectorArrayName());
}

//----------------------------------------------------------------------------
vtkDataArray* GetSampledColors(vtkVectorFieldSampler* sampler)
{
  return sampler->GetOutput()->GetPointData()->GetArray(vtkVectorFieldSampler::GetColorArrayName());
}

//----------------------------------------------------------------------------
vtkDataArray* GetValidSamples(vtkVectorFieldSampler* sampler)
{
  return sampler->GetOutput()->GetPointData()->GetArray(vtkVectorFieldSampler::GetValidSampleArrayName());
}

//----------------------------------------------------------------------------
/// Largest distance between the sampled vectors and the field at the sampled positions,
/// over the samples that are valid.
double GetLargestError(vtkVectorFieldSampler* sampler, void (*field)(const double[3], double[3]))
{
  vtkUnstructuredGrid* output = sampler->GetOutput();
  vtkDataArray* vectors = GetSampledVectors(sampler);
  vtkDataArray* valid = GetValidSamples(sampler);
  double largestError = 0.0;
  for (vtkIdType pointIndex = 0; pointIndex < output->GetNumberOfPoints(); ++pointIndex)
  {
    if (valid && valid->GetComponent(pointIndex, 0) == 0)
    {
      continue;
    }
    double position[3] = { 0.0, 0.0, 0.0 };
    output->GetPoint(pointIndex, position);
    double expected[3] = { 0.0, 0.0, 0.0 };
    field(position, expected);
    double actual[3] = { 0.0, 0.0, 0.0 };
    vectors->GetTuple(pointIndex, actual);
    largestError = std::max(largestError, Distance(expected, actual));
  }
  return largestError;
}

//----------------------------------------------------------------------------
int CountValidSamples(vtkVectorFieldSampler* sampler)
{
  vtkDataArray* valid = GetValidSamples(sampler);
  int count = 0;
  for (vtkIdType pointIndex = 0; valid && pointIndex < valid->GetNumberOfTuples(); ++pointIndex)
  {
    count += (valid->GetComponent(pointIndex, 0) != 0 ? 1 : 0);
  }
  return count;
}

//----------------------------------------------------------------------------
int TestImageFieldSampler()
{
  std::cout << "Image field sampler" << std::endl;
  vtkSmartPointer<vtkImageData> image = CreateVectorImage(LinearField);
  vtkNew<vtkMatrix4x4> ijkToRAS;
  for (int axis = 0; axis < 3; ++axis)
  {
    ijkToRAS->SetElement(axis, axis, 10.0);
    ijkToRAS->SetElement(axis, 3, -50.0);
  }
  vtkNew<vtkVectorFieldSampler> sampler;
  sampler->SetFieldSourceToImage();
  sampler->SetInputData(image);
  sampler->SetIJKToRAS(ijkToRAS);
  sampler->VectorsInRASOn();

  // Positions between the voxels: a linear field is reproduced by linear interpolation
  vtkNew<vtkPoints> positions;
  positions->InsertNextPoint(1.0, 2.0, 3.0);
  positions->InsertNextPoint(-23.5, 17.25, 41.0);
  positions->InsertNextPoint(44.0, -44.0, 0.5);
  sampler->SetSamplePositions(positions);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 3);
  // The image is interpolated in single precision, which is plenty for drawing it
  CHECK_DOUBLE_TOLERANCE(GetLargestError(sampler, LinearField), 0.0, 1e-5);

  // The positions are copied: the same positions again do not modify the sampler, other positions do
  vtkMTimeType positionsTime = sampler->GetMTime();
  vtkNew<vtkPoints> samePositions;
  samePositions->DeepCopy(positions);
  sampler->SetSamplePositions(samePositions);
  CHECK_BOOL(sampler->GetMTime() == positionsTime, true);
  samePositions->SetPoint(0, 2.0, 2.0, 3.0);
  CHECK_BOOL(sampler->GetMTime() == positionsTime, true);
  sampler->SetSamplePositions(samePositions);
  CHECK_BOOL(sampler->GetMTime() > positionsTime, true);
  sampler->SetSamplePositions(positions);

  // Without positions, the source samples a lattice over its own extent
  sampler->SetSamplePositions(nullptr);
  sampler->SetSamplingSpacing(20.0);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 6 * 6 * 6);
  CHECK_DOUBLE_TOLERANCE(GetLargestError(sampler, LinearField), 0.0, 1e-5);

  // The volume is rotated by 90 degrees around S and moved by 10 mm along R: world positions are looked up in volume
  // coordinates, and the vectors found there are rotated into world directions.
  vtkNew<vtkTransform> fieldToWorld;
  fieldToWorld->Translate(10.0, 0.0, 0.0);
  fieldToWorld->RotateZ(90.0);
  sampler->SetFieldToWorldTransform(fieldToWorld);
  vtkNew<vtkPoints> worldPositions;
  worldPositions->InsertNextPoint(12.0, -3.0, 4.0);
  worldPositions->InsertNextPoint(-20.0, 15.0, -30.0);
  sampler->SetSamplePositions(worldPositions);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 2);
  for (vtkIdType pointIndex = 0; pointIndex < 2; ++pointIndex)
  {
    double position_World[3] = { 0.0, 0.0, 0.0 };
    sampler->GetOutput()->GetPoint(pointIndex, position_World);
    // The samples are where they were asked for, in world coordinates
    double requested[3] = { 0.0, 0.0, 0.0 };
    worldPositions->GetPoint(pointIndex, requested);
    CHECK_DOUBLE_TOLERANCE(Distance(position_World, requested), 0.0, 1e-6);
    double position_Field[3] = { 0.0, 0.0, 0.0 };
    fieldToWorld->GetInverse()->TransformPoint(position_World, position_Field);
    double vector_Field[3] = { 0.0, 0.0, 0.0 };
    LinearField(position_Field, vector_Field);
    double expected_World[3] = { 0.0, 0.0, 0.0 };
    fieldToWorld->TransformVector(vector_Field, expected_World);
    double actual[3] = { 0.0, 0.0, 0.0 };
    GetSampledVectors(sampler)->GetTuple(pointIndex, actual);
    CHECK_DOUBLE_TOLERANCE(Distance(expected_World, actual), 0.0, 1e-5);
  }
  sampler->SetFieldToWorldTransform(nullptr);

  // In a slice view only the positions within the slice are sampled. As in the XYToRAS matrix of a slice node, the
  // third axis of the slice coordinate system is as long as the slice spacing, while the slice thickness is in mm.
  const double sliceSpacingMm = 2.0;
  vtkNew<vtkMatrix4x4> sliceXYToRAS;
  for (int axis = 0; axis < 2; ++axis)
  {
    sliceXYToRAS->SetElement(axis, axis, 0.5); // 0.5 mm per pixel
    sliceXYToRAS->SetElement(axis, 3, -50.0);  // the view is 200 pixels wide, centered on the volume
  }
  sliceXYToRAS->SetElement(2, 2, sliceSpacingMm);
  const double fieldOfViewSizeMm[2] = { 100.0, 100.0 };
  // Layers of positions that are a slice spacing apart, as the voxels of a volume are
  vtkNew<vtkPoints> layeredPositions;
  for (int layer = -2; layer <= 2; ++layer)
  {
    layeredPositions->InsertNextPoint(-10.0, 5.0, layer * sliceSpacingMm);
    layeredPositions->InsertNextPoint(20.0, -15.0, layer * sliceSpacingMm);
  }
  sampler->SetSamplePositions(layeredPositions);
  sampler->SetSliceThickness(sliceSpacingMm);
  // Wherever the slice is between two layers (also halfway), it shows one layer, the one that it is the closest to
  for (double sliceOffsetMm : { 0.0, 0.7, 1.0, 1.3, -0.9 })
  {
    sliceXYToRAS->SetElement(2, 3, sliceOffsetMm);
    sampler->SetSlicePlane(sliceXYToRAS, fieldOfViewSizeMm);
    sampler->Update();
    CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 2);
    for (vtkIdType pointIndex = 0; pointIndex < 2; ++pointIndex)
    {
      double position[3] = { 0.0, 0.0, 0.0 };
      sampler->GetOutput()->GetPoint(pointIndex, position);
      CHECK_BOOL(std::abs(position[2] - sliceOffsetMm) <= 0.5 * sliceSpacingMm, true);
    }
  }

  // A lattice in the slice plane has no samples if the slice is farther from the volume (which ends at 50 mm) than
  // half the slice thickness
  sampler->SetSamplePositions(nullptr);
  sliceXYToRAS->SetElement(2, 3, 50.5);
  sampler->SetSlicePlane(sliceXYToRAS, fieldOfViewSizeMm);
  sampler->Update();
  CHECK_BOOL(sampler->GetOutput()->GetNumberOfPoints() > 0, true);
  sliceXYToRAS->SetElement(2, 3, 51.5);
  sampler->SetSlicePlane(sliceXYToRAS, fieldOfViewSizeMm);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 0);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestMeshFieldSampler()
{
  std::cout << "Mesh field sampler" << std::endl;
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddPointField(mesh, "PointField", LinearField);
  AddCellField(mesh, "CellField", LinearField);

  vtkNew<vtkVectorFieldSampler> sampler;
  sampler->SetFieldSourceToMesh();
  sampler->SetInputData(mesh);
  sampler->SetMeshVectorArrayName("PointField");
  vtkNew<vtkPoints> positions;
  positions->InsertNextPoint(1.0, 2.0, 3.0);       // inside
  positions->InsertNextPoint(-33.0, 47.5, -12.25); // inside
  positions->InsertNextPoint(80.0, 0.0, 0.0);      // outside the mesh
  sampler->SetSamplePositions(positions);
  sampler->Update();
  // Outside the mesh there is no field, so no sample is taken there
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 2);
  CHECK_INT(CountValidSamples(sampler), 2);
  // Inside the cells the point data is interpolated, which reproduces a linear field
  CHECK_DOUBLE_TOLERANCE(GetLargestError(sampler, LinearField), 0.0, 1e-9);

  // A cell data array: the value of the cell that the position falls in
  sampler->SetMeshVectorArrayName("CellField");
  sampler->Update();
  CHECK_INT(CountValidSamples(sampler), 2);
  // The cells are 10 mm wide, so (1, 2, 3) is in the cell centered at (5, 5, 5)
  double expected[3] = { 0.0, 0.0, 0.0 };
  const double cellCenter[3] = { 5.0, 5.0, 5.0 };
  LinearField(cellCenter, expected);
  double actual[3] = { 0.0, 0.0, 0.0 };
  GetSampledVectors(sampler)->GetTuple(0, actual);
  CHECK_DOUBLE_TOLERANCE(Distance(expected, actual), 0.0, 1e-9);

  // Without a color array, the field is colored by its magnitude, which is always there
  sampler->SetMeshVectorArrayName("PointField");
  sampler->Update();
  CHECK_NULL(GetSampledColors(sampler));

  // The array the field is colored by is sampled along with the vectors: a point data array
  // is interpolated inside the cells
  AddPointScalar(mesh, "PointScalar", LinearScalar);
  AddCellScalar(mesh, "CellScalar", LinearScalar);
  sampler->SetMeshColorArrayName("PointScalar");
  sampler->Update();
  vtkDataArray* colors = GetSampledColors(sampler);
  CHECK_NOT_NULL(colors);
  CHECK_INT(colors->GetNumberOfComponents(), 1);
  for (vtkIdType pointIndex = 0; pointIndex < 2; ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    sampler->GetOutput()->GetPoint(pointIndex, position);
    CHECK_DOUBLE_TOLERANCE(colors->GetComponent(pointIndex, 0), LinearScalar(position), 1e-9);
  }
  // A cell data array: the value of the cell that the position falls in
  sampler->SetMeshColorArrayName("CellScalar");
  sampler->Update();
  CHECK_DOUBLE_TOLERANCE(GetSampledColors(sampler)->GetComponent(0, 0), LinearScalar(cellCenter), 1e-9);
  // A multi-component array is colored by its magnitude
  sampler->SetMeshColorArrayName("CellField");
  sampler->Update();
  CHECK_INT(GetSampledColors(sampler)->GetNumberOfComponents(), 1);
  CHECK_DOUBLE_TOLERANCE(GetSampledColors(sampler)->GetComponent(0, 0), vtkMath::Norm(expected), 1e-9);
  // or by one of its components, if one is chosen
  for (int component = 0; component < 3; ++component)
  {
    sampler->SetMeshColorArrayComponent(component);
    sampler->Update();
    CHECK_DOUBLE_TOLERANCE(GetSampledColors(sampler)->GetComponent(0, 0), expected[component], 1e-9);
  }
  // A component that the array does not have colors it by its magnitude
  sampler->SetMeshColorArrayComponent(3);
  sampler->Update();
  CHECK_DOUBLE_TOLERANCE(GetSampledColors(sampler)->GetComponent(0, 0), vtkMath::Norm(expected), 1e-9);
  sampler->SetMeshColorArrayComponent(-1);
  // An array the mesh does not have gives no colors, rather than wrong ones
  sampler->SetMeshColorArrayName("NoSuchArray");
  sampler->Update();
  CHECK_NULL(GetSampledColors(sampler));
  sampler->SetMeshColorArrayName(nullptr);

  // Lattice cells only connect samples that the mesh had a value at
  sampler->SetSamplePositions(nullptr);
  sampler->SetMeshVectorArrayName("PointField");
  sampler->SetSamplingSpacing(10.0);
  sampler->GenerateCellsOn();
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 11 * 11 * 11);
  CHECK_INT(sampler->GetOutput()->GetNumberOfCells(), 10 * 10 * 10);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestTransformFieldSampler()
{
  std::cout << "Transform field sampler" << std::endl;
  vtkNew<vtkTransform> translation;
  translation->Translate(1.0, 2.0, 3.0);
  vtkNew<vtkGeneralTransform> transformToWorld;
  transformToWorld->Concatenate(translation);

  vtkNew<vtkVectorFieldSampler> sampler;
  sampler->SetFieldSourceToTransform();
  sampler->SetTransform(transformToWorld);
  vtkNew<vtkPoints> positions;
  positions->InsertNextPoint(0.0, 0.0, 0.0);
  positions->InsertNextPoint(100.0, -50.0, 7.0);
  sampler->SetSamplePositions(positions);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 2);
  // The vector is the displacement that the transform applies
  for (vtkIdType pointIndex = 0; pointIndex < 2; ++pointIndex)
  {
    double vector[3] = { 0.0, 0.0, 0.0 };
    GetSampledVectors(sampler)->GetTuple(pointIndex, vector);
    CHECK_DOUBLE_TOLERANCE(vector[0], 1.0, 1e-9);
    CHECK_DOUBLE_TOLERANCE(vector[1], 2.0, 1e-9);
    CHECK_DOUBLE_TOLERANCE(vector[2], 3.0, 1e-9);
  }

  // Changing a transform that is concatenated re-samples the field on the next update
  translation->Translate(1.0, 0.0, 0.0);
  sampler->Update();
  double vector[3] = { 0.0, 0.0, 0.0 };
  GetSampledVectors(sampler)->GetTuple(0, vector);
  CHECK_DOUBLE_TOLERANCE(vector[0], 2.0, 1e-9);

  // A transform has no extent of its own: nothing is sampled without positions or a region
  sampler->SetSamplePositions(nullptr);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 0);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestSourceSwitch()
{
  std::cout << "Switching the source of a sampler" << std::endl;
  // One sampler, reconfigured from one source to the next, gives what a sampler made for
  // that source gives, and lets go of what the previous source used
  vtkSmartPointer<vtkUnstructuredGrid> mesh = CreateVolumetricMesh();
  AddPointField(mesh, "PointField", LinearField);
  vtkSmartPointer<vtkImageData> image = CreateVectorImage(LinearField);
  vtkNew<vtkMatrix4x4> ijkToRAS;
  for (int axis = 0; axis < 3; ++axis)
  {
    ijkToRAS->SetElement(axis, axis, 10.0);
    ijkToRAS->SetElement(axis, 3, -50.0);
  }
  vtkNew<vtkPoints> positions;
  positions->InsertNextPoint(1.0, 2.0, 3.0);
  positions->InsertNextPoint(-23.5, 17.25, 41.0);

  vtkNew<vtkVectorFieldSampler> sampler;
  sampler->SetSamplePositions(positions);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 0);

  sampler->SetFieldSourceToMesh();
  sampler->SetInputData(mesh);
  sampler->SetMeshVectorArrayName("PointField");
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 2);
  CHECK_DOUBLE_TOLERANCE(GetLargestError(sampler, LinearField), 0.0, 1e-5);
  CHECK_NOT_NULL(GetValidSamples(sampler));

  sampler->SetFieldSourceToImage();
  CHECK_NULL(sampler->GetInputDataObject(0, 0));
  sampler->SetInputData(image);
  sampler->SetIJKToRAS(ijkToRAS);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 2);
  CHECK_DOUBLE_TOLERANCE(GetLargestError(sampler, LinearField), 0.0, 1e-5);
  // The image source does not mark samples invalid
  CHECK_NULL(GetValidSamples(sampler));

  vtkNew<vtkTransform> translation;
  translation->Translate(1.0, 2.0, 3.0);
  sampler->SetFieldSourceToTransform();
  CHECK_NULL(sampler->GetInputDataObject(0, 0));
  sampler->SetTransform(translation);
  sampler->Update();
  double vector[3] = { 0.0, 0.0, 0.0 };
  GetSampledVectors(sampler)->GetTuple(1, vector);
  CHECK_DOUBLE_TOLERANCE(vector[0], 1.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(vector[2], 3.0, 1e-9);

  // Back to the mesh: the transform is let go, and the mesh is sampled again
  sampler->SetFieldSourceToMesh();
  CHECK_NULL(sampler->GetTransform());
  sampler->SetInputData(mesh);
  sampler->Update();
  CHECK_DOUBLE_TOLERANCE(GetLargestError(sampler, LinearField), 0.0, 1e-5);

  // No source: the input is let go and the output is empty, as for a new sampler
  sampler->SetFieldSourceToNone();
  CHECK_INT(sampler->GetFieldSource(), vtkVectorFieldSampler::FieldSourceNone);
  CHECK_NULL(sampler->GetInputDataObject(0, 0));
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), 0);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestScatteredFieldSampler()
{
  std::cout << "Scattered field sampler" << std::endl;
  // A field that is only known at the points of a surface: there are no cells to
  // interpolate in, so a smooth field is fitted to the points
  const double radiusMm = 50.0;
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(radiusMm);
  AddPointField(surface, "Field", SmoothField);

  vtkNew<vtkScatteredBSplineTransform> fittedTransform;
  fittedTransform->SetInputPointSet(surface);
  fittedTransform->SetDisplacementArrayName("Field");
  fittedTransform->UseLinearApproximationOn();
  fittedTransform->SetMinimumControlPointSpacing(1.0);
  fittedTransform->SetTolerance(0.001);

  vtkNew<vtkVectorFieldSampler> sampler;
  sampler->SetFieldSourceToScattered();
  sampler->SetScatteredFittedTransform(fittedTransform);

  // Positions on the sphere between its points, and slightly off the surface
  vtkNew<vtkPoints> positions;
  const int numberOfDirections = 50;
  for (int directionIndex = 0; directionIndex < numberOfDirections; ++directionIndex)
  {
    // A spiral that does not follow the latitude and longitude lines of the sphere
    double z = -0.9 + 1.8 * directionIndex / (numberOfDirections - 1);
    double angle = 2.4 * directionIndex;
    double radiusXY = std::sqrt(1.0 - z * z);
    double direction[3] = { radiusXY * std::cos(angle), radiusXY * std::sin(angle), z };
    positions->InsertNextPoint(radiusMm * direction[0], radiusMm * direction[1], radiusMm * direction[2]);
  }
  sampler->SetSamplePositions(positions);
  sampler->Update();
  CHECK_INT(sampler->GetOutput()->GetNumberOfPoints(), numberOfDirections);
  CHECK_INT(CountValidSamples(sampler), numberOfDirections);
  // The field is approximated between the points: its vectors are about 1 long, and the fit
  // follows them to within a few percent
  double largestError = GetLargestError(sampler, SmoothField);
  std::cout << "  Largest error between the points: " << largestError << std::endl;
  CHECK_BOOL(largestError < 0.05, true);
  // At the points themselves it is as close as the fit reports
  CHECK_BOOL(fittedTransform->GetResidual() >= 0.0, true);
  CHECK_BOOL(fittedTransform->GetResidual() < 0.05, true);
  // Without a fit of the values to color by, the field is colored by its magnitude
  CHECK_NULL(GetSampledColors(sampler));

  // The values the field is colored by are fitted the same way, as the first component of a
  // displacement, and evaluated at the same samples
  vtkNew<vtkDoubleArray> colorValues;
  colorValues->SetName("ColorValue");
  colorValues->SetNumberOfComponents(3);
  colorValues->SetNumberOfTuples(surface->GetNumberOfPoints());
  for (vtkIdType pointIndex = 0; pointIndex < surface->GetNumberOfPoints(); ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    surface->GetPoint(pointIndex, position);
    colorValues->SetTuple3(pointIndex, SmoothScalar(position), 0.0, 0.0);
  }
  vtkNew<vtkPolyData> colorPoints;
  colorPoints->SetPoints(surface->GetPoints());
  colorPoints->GetPointData()->AddArray(colorValues);
  vtkNew<vtkScatteredBSplineTransform> colorFittedTransform;
  colorFittedTransform->SetInputPointSet(colorPoints);
  colorFittedTransform->SetDisplacementArrayName("ColorValue");
  colorFittedTransform->UseLinearApproximationOn();
  colorFittedTransform->SetMinimumControlPointSpacing(1.0);
  colorFittedTransform->SetTolerance(0.001);
  sampler->SetScatteredColorFittedTransform(colorFittedTransform);
  sampler->Update();
  vtkDataArray* colors = GetSampledColors(sampler);
  CHECK_NOT_NULL(colors);
  CHECK_INT(colors->GetNumberOfComponents(), 1);
  double largestColorError = 0.0;
  for (vtkIdType pointIndex = 0; pointIndex < sampler->GetOutput()->GetNumberOfPoints(); ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    sampler->GetOutput()->GetPoint(pointIndex, position);
    largestColorError = std::max(largestColorError, std::abs(colors->GetComponent(pointIndex, 0) - SmoothScalar(position)));
  }
  std::cout << "  Largest error of the colors between the points: " << largestColorError << std::endl;
  // The values are between about 1.5 and 4.5
  CHECK_BOOL(largestColorError < 0.05, true);
  sampler->SetScatteredColorFittedTransform(nullptr);
  sampler->Update();
  CHECK_NULL(GetSampledColors(sampler));

  // Far from the data the fit says nothing, so nothing is drawn there. The automatic
  // distance is a tenth of the diagonal of the bounds of the points.
  CHECK_DOUBLE_TOLERANCE(sampler->GetScatteredEffectiveMaximumDistance(), 0.1 * std::sqrt(3.0) * 2.0 * radiusMm, 0.5);
  vtkNew<vtkPoints> farPositions;
  farPositions->InsertNextPoint(0.0, 0.0, 0.0);            // center of the sphere, 50 mm from any point
  farPositions->InsertNextPoint(radiusMm + 5.0, 0.0, 0.0); // 5 mm off the surface
  sampler->SetSamplePositions(farPositions);
  sampler->Update();
  vtkDataArray* valid = GetValidSamples(sampler);
  CHECK_NOT_NULL(valid);
  CHECK_INT(static_cast<int>(valid->GetComponent(0, 0)), 0);
  CHECK_INT(static_cast<int>(valid->GetComponent(1, 0)), 1);
  double centerVector[3] = { 1.0, 1.0, 1.0 };
  GetSampledVectors(sampler)->GetTuple(0, centerVector);
  CHECK_DOUBLE(vtkMath::Norm(centerVector), 0.0);
  // A smaller distance leaves out the sample that is 5 mm off the surface too
  sampler->SetScatteredMaximumDistance(2.0);
  sampler->Update();
  CHECK_INT(CountValidSamples(sampler), 0);
  sampler->SetScatteredMaximumDistance(0.0);

  // In the plane of a slice view: the samples near the circle where the slice cuts the
  // sphere are drawn, and the ones near its center are not
  vtkNew<vtkMatrix4x4> sliceXYToRAS;
  sliceXYToRAS->Identity();
  for (int axis = 0; axis < 2; ++axis)
  {
    sliceXYToRAS->SetElement(axis, axis, 0.5); // 0.5 mm per pixel
    sliceXYToRAS->SetElement(axis, 3, -60.0);  // the view is 240 pixels wide, centered on the sphere
  }
  const double fieldOfViewSizeMm[2] = { 120.0, 120.0 };
  sampler->SetSamplePositions(nullptr);
  sampler->SetSlicePlane(sliceXYToRAS, fieldOfViewSizeMm);
  sampler->SetSamplingSpacing(5.0);
  sampler->Update();
  vtkUnstructuredGrid* sliceSamples = sampler->GetOutput();
  int numberOfValidSliceSamples = CountValidSamples(sampler);
  std::cout << "  Valid samples in the slice: " << numberOfValidSliceSamples << " of " << sliceSamples->GetNumberOfPoints() << std::endl;
  CHECK_BOOL(numberOfValidSliceSamples > 0, true);
  CHECK_BOOL(numberOfValidSliceSamples < sliceSamples->GetNumberOfPoints(), true);
  valid = GetValidSamples(sampler);
  double maximumDistanceMm = sampler->GetScatteredEffectiveMaximumDistance();
  for (vtkIdType pointIndex = 0; pointIndex < sliceSamples->GetNumberOfPoints(); ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    sliceSamples->GetPoint(pointIndex, position);
    // The slice is at z = 0, through the center of the sphere
    CHECK_DOUBLE_TOLERANCE(position[2], 0.0, 1e-9);
    double distanceFromSurfaceMm = std::abs(vtkMath::Norm(position) - radiusMm);
    if (valid->GetComponent(pointIndex, 0) != 0)
    {
      // The mesh points are a few mm apart, so a sample can be a little farther from the
      // nearest point than from the sphere itself
      CHECK_BOOL(distanceFromSurfaceMm < maximumDistanceMm + 1e-6, true);
    }
    else
    {
      CHECK_BOOL(distanceFromSurfaceMm > maximumDistanceMm - 8.0, true);
    }
  }

  // Lattice cells only connect samples near the data
  sampler->GenerateCellsOn();
  sampler->Update();
  CHECK_BOOL(sampler->GetOutput()->GetNumberOfCells() > 0, true);
  int latticeSize[3] = { 0, 0, 0 };
  sampler->GetOutputLatticeSize(latticeSize);
  CHECK_BOOL(sampler->GetOutput()->GetNumberOfCells() < (latticeSize[0] - 1) * (latticeSize[1] - 1), true);
  sampler->GenerateCellsOff();

  // Changing the data re-fits the field on the next update
  vtkDataArray* field = surface->GetPointData()->GetArray("Field");
  for (vtkIdType pointIndex = 0; pointIndex < field->GetNumberOfTuples(); ++pointIndex)
  {
    double vector[3] = { 0.0, 0.0, 0.0 };
    field->GetTuple(pointIndex, vector);
    field->SetTuple3(pointIndex, 2.0 * vector[0], 2.0 * vector[1], 2.0 * vector[2]);
  }
  field->Modified();
  sampler->SetSlicePlane(sliceXYToRAS, fieldOfViewSizeMm);
  sampler->SetSamplePositions(positions);
  sampler->Update();
  auto doubledSmoothField = [](const double position[3], double vector[3])
  {
    SmoothField(position, vector);
    vtkMath::MultiplyScalar(vector, 2.0);
  };
  largestError = GetLargestError(sampler, doubledSmoothField);
  std::cout << "  Largest error after the data changed: " << largestError << std::endl;
  CHECK_BOOL(largestError < 0.1, true);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkVectorFieldSamplerTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestImageFieldSampler());
  CHECK_EXIT_SUCCESS(TestMeshFieldSampler());
  CHECK_EXIT_SUCCESS(TestTransformFieldSampler());
  CHECK_EXIT_SUCCESS(TestScatteredFieldSampler());
  CHECK_EXIT_SUCCESS(TestSourceSwitch());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
