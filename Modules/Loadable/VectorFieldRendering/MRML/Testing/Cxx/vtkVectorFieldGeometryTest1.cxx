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

// Checks the geometry that vtkVectorFieldGeometry builds in each mode, and that settings that do not affect the
// streamlines do not trace them again.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkVectorFieldGeometry.h"
#include "vtkVectorFieldSampler.h"

// VTK includes
#include <vtkAppendFilter.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

// STD includes
#include <algorithm>
#include <cmath>

namespace
{
const char VectorArrayName[] = "Velocity";

//----------------------------------------------------------------------------
/// An image over -50..50 mm, 10 mm voxels, with the vector (1, 0.01 x, 0) at each voxel: magnitude grows away from x = 0.
/// With a direction matrix the image is rotated about its center, as the image of a volume that is not axis-aligned;
/// the vectors stay the same function of the position.
vtkSmartPointer<vtkImageData> CreateField(const double* direction = nullptr)
{
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetDimensions(11, 11, 11);
  image->SetSpacing(10.0, 10.0, 10.0);
  image->SetOrigin(-50.0, -50.0, -50.0);
  if (direction)
  {
    image->SetDirectionMatrix(direction);
    double center[3] = { 0.0, 0.0, 0.0 };
    const double centerIndex[3] = { 5.0, 5.0, 5.0 };
    image->TransformContinuousIndexToPhysicalPoint(centerIndex, center);
    image->SetOrigin(-50.0 - center[0], -50.0 - center[1], -50.0 - center[2]);
  }
  vtkNew<vtkDoubleArray> vectors;
  vectors->SetName(VectorArrayName);
  vectors->SetNumberOfComponents(3);
  vectors->SetNumberOfTuples(image->GetNumberOfPoints());
  for (vtkIdType pointId = 0; pointId < image->GetNumberOfPoints(); ++pointId)
  {
    double* p = image->GetPoint(pointId);
    vectors->SetTuple3(pointId, 1.0, 0.01 * p[0], 0.0);
  }
  image->GetPointData()->AddArray(vectors);
  return image;
}

//----------------------------------------------------------------------------
vtkPolyData* GetOutput(vtkVectorFieldGeometry* geometry, int port = 0)
{
  geometry->Update();
  return vtkPolyData::SafeDownCast(geometry->GetOutputDataObject(port));
}

//----------------------------------------------------------------------------
int TestNoneAndContour()
{
  vtkNew<vtkVectorFieldGeometry> geometry;
  geometry->SetInputData(CreateField());
  geometry->SetVectorArrayName(VectorArrayName);
  CHECK_INT(GetOutput(geometry)->GetNumberOfPoints(), 0);

  // The magnitude is computed from the vectors when the field has none: at |x| = 20 mm it is sqrt(1.04)
  geometry->SetMode(vtkVectorFieldGeometry::ModeContour);
  geometry->SetContourLevels({ std::sqrt(1.04) });
  vtkPolyData* contour = GetOutput(geometry);
  CHECK_BOOL(contour->GetNumberOfPolys() > 0, true);
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  contour->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[0], -20.0, 0.5);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 20.0, 0.5);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestGrid()
{
  // A 5 x 5 lattice, 10 mm apart, of a transform that moves everything 5 mm along x
  vtkNew<vtkTransform> translation;
  translation->Translate(5.0, 0.0, 0.0);
  vtkNew<vtkVectorFieldSampler> sampler;
  sampler->SetFieldSourceToTransform();
  sampler->SetTransform(translation);
  vtkNew<vtkMatrix4x4> latticeToRAS;
  for (int axis = 0; axis < 3; ++axis)
  {
    latticeToRAS->SetElement(axis, axis, 10.0);
  }
  int latticeSize[3] = { 5, 5, 1 };
  sampler->SetSamplingRegion(latticeToRAS, latticeSize);
  sampler->GenerateCellsOn();

  vtkNew<vtkVectorFieldGeometry> geometry;
  geometry->SetInputConnection(sampler->GetOutputPort());
  geometry->SetMode(vtkVectorFieldGeometry::ModeGrid);
  geometry->SetVectorArrayName(vtkVectorFieldSampler::GetVectorArrayName());
  vtkPolyData* grid = GetOutput(geometry);
  CHECK_BOOL(grid->GetNumberOfLines() > 0, true);
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  grid->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[0], 5.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 45.0, 1e-6);

  // Nothing to draw without lines and fill
  geometry->SetGridDeformedLinesVisibility(false);
  CHECK_INT(GetOutput(geometry)->GetNumberOfPoints(), 0);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestStreamlines()
{
  vtkNew<vtkVectorFieldGeometry> geometry;
  geometry->SetInputData(CreateField());
  geometry->SetMode(vtkVectorFieldGeometry::ModeStreamline);
  geometry->SetVectorArrayName(VectorArrayName);
  CHECK_INT(GetOutput(geometry)->GetNumberOfPoints(), 0); // no seeds

  vtkNew<vtkPoints> seeds;
  seeds->InsertNextPoint(0.0, 0.0, 0.0);
  seeds->InsertNextPoint(0.0, 20.0, 10.0);
  geometry->SetStreamlineSeeds(seeds);
  vtkPolyData* lines = GetOutput(geometry);
  CHECK_BOOL(lines->GetNumberOfLines() > 0, true);
  vtkPolyData* streamlineLines = GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort);
  CHECK_INT(static_cast<int>(streamlineLines->GetNumberOfLines()), static_cast<int>(lines->GetNumberOfLines()));
  // Kept, so that new points of a new trace cannot be allocated at the same address
  vtkSmartPointer<vtkPoints> tracedPoints = streamlineLines->GetPoints();

  // The same seeds again do not modify the filter
  vtkMTimeType modifiedTime = geometry->GetMTime();
  vtkNew<vtkPoints> sameSeeds;
  sameSeeds->DeepCopy(seeds);
  geometry->SetStreamlineSeeds(sameSeeds);
  CHECK_BOOL(geometry->GetMTime() == modifiedTime, true);

  // Tubes are made from the same streamlines: they are not traced again
  geometry->SetStreamlineTubeDiameter(2.0);
  vtkPolyData* tubes = GetOutput(geometry);
  CHECK_BOOL(tubes->GetNumberOfPolys() + tubes->GetNumberOfStrips() > 0, true);
  CHECK_POINTER(GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort)->GetPoints(), tracedPoints.GetPointer());

  // Flat output draws lines
  geometry->FlatOn();
  CHECK_INT(static_cast<int>(GetOutput(geometry)->GetNumberOfPolys() + GetOutput(geometry)->GetNumberOfStrips()), 0);

  // Other seeds trace again
  seeds->SetPoint(1, 0.0, -20.0, 10.0);
  geometry->SetStreamlineSeeds(seeds);
  CHECK_BOOL(GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort)->GetPoints() != tracedPoints, true);

  // Thresholded by the y component: only the parts where it is above 0 (x > 0) are kept
  geometry->SetThresholdEnabled(true);
  geometry->SetThresholdArrayName(VectorArrayName);
  geometry->SetThresholdComponent(1);
  geometry->SetThresholdRange(0.0, 10.0);
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort)->GetBounds(bounds);
  CHECK_BOOL(bounds[0] >= -1e-6, true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// The streamline of the field of CreateField() from the center is y = 0.005 x^2 (the field is linear, so it is traced
/// exactly) and gets near the edges of the image at x = -50 and 50 mm, whatever the orientation of the image is. The
/// vectors along it are those of the field, (1, 0.01 x, 0), not those of the axis-aligned space that a non-axis-aligned
/// image is traced in.
int CheckWholeStreamlineFromCenter(vtkPolyData* lines)
{
  CHECK_BOOL(lines->GetNumberOfLines() > 0, true);
  vtkDataArray* vectors = lines->GetPointData()->GetArray(VectorArrayName);
  CHECK_NOT_NULL(vectors);
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  lines->GetBounds(bounds);
  double maximumDeviationMm = 0.0;
  double maximumVectorError = 0.0;
  for (vtkIdType pointId = 0; pointId < lines->GetNumberOfPoints(); ++pointId)
  {
    double* p = lines->GetPoint(pointId);
    maximumDeviationMm = std::max({ maximumDeviationMm, std::abs(p[1] - 0.005 * p[0] * p[0]), std::abs(p[2]) });
    double* vector = vectors->GetTuple3(pointId);
    maximumVectorError = std::max({ maximumVectorError, std::abs(vector[0] - 1.0), std::abs(vector[1] - 0.01 * p[0]), std::abs(vector[2]) });
  }
  std::cout << "  Whole streamline from " << bounds[0] << " to " << bounds[1] << " mm, at most " << maximumDeviationMm << " mm off, vectors at most " << maximumVectorError
            << " off" << std::endl;
  CHECK_BOOL(bounds[0] < -40.0, true);
  CHECK_BOOL(bounds[1] > 40.0, true);
  CHECK_BOOL(maximumDeviationMm < 0.01, true);
  // The last point may be just outside the image, where the vectors of the nearest position in the image are used
  CHECK_BOOL(maximumVectorError < 1e-3, true);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Streamlines are only traced in the domain, where a point array of the field is not zero, and where the threshold
/// array is in the threshold range. The image of a volume is oriented as the volume is, so these have to work for any
/// direction matrix. The other cells of an image are hidden in it, those of a mesh are left out of it, so both kinds of
/// field are tested.
int TestStreamlineDomain()
{
  const char domainArrayName[] = "Domain";
  const double domainHalfSizeMm = 25.0;
  // The threshold array is out of the threshold range in a slab across x. The slab is thicker than two cells of a
  // rotated image, so the cells around its middle are in it entirely.
  const char thresholdArrayName[] = "Speed";
  const double slabMinMm = 5.0;
  const double slabMiddleMm = 20.0;
  const double slabMaxMm = 35.0;
  const double identity[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
  const double flipped[9] = { -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 1.0 };  // an axial acquisition stored in LPS
  const double permuted[9] = { 0.0, 0.0, 1.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0 }; // a sagittal acquisition
  const double oblique[9] = { 0.8, -0.6, 0.0, 0.6, 0.8, 0.0, 0.0, 0.0, 1.0 };
  const int numberOfFields = 5;
  const double* directions[numberOfFields] = { identity, flipped, permuted, oblique, identity };
  for (int fieldIndex = 0; fieldIndex < numberOfFields; ++fieldIndex)
  {
    const double* direction = directions[fieldIndex];
    bool mesh = (fieldIndex == numberOfFields - 1);
    std::cout << "Streamline domain, " << (mesh ? "mesh" : "image") << " direction: " << direction[0] << " " << direction[1] << " " << direction[2] << " ..." << std::endl;
    // The domain is a box around the center of the image
    vtkSmartPointer<vtkImageData> image = CreateField(direction);
    vtkNew<vtkUnsignedCharArray> domain;
    domain->SetName(domainArrayName);
    domain->SetNumberOfTuples(image->GetNumberOfPoints());
    vtkNew<vtkDoubleArray> speed;
    speed->SetName(thresholdArrayName);
    speed->SetNumberOfTuples(image->GetNumberOfPoints());
    for (vtkIdType pointId = 0; pointId < image->GetNumberOfPoints(); ++pointId)
    {
      double* p = image->GetPoint(pointId);
      bool inside = (std::abs(p[0]) <= domainHalfSizeMm && std::abs(p[1]) <= domainHalfSizeMm && std::abs(p[2]) <= domainHalfSizeMm);
      domain->SetValue(pointId, inside ? 1 : 0);
      speed->SetValue(pointId, (p[0] > slabMinMm && p[0] < slabMaxMm) ? 0.0 : 1.0);
    }
    image->GetPointData()->AddArray(domain);
    image->GetPointData()->AddArray(speed);

    vtkSmartPointer<vtkDataSet> field = image;
    if (mesh)
    {
      vtkNew<vtkAppendFilter> imageToMesh;
      imageToMesh->AddInputData(image);
      imageToMesh->Update();
      field = imageToMesh->GetOutput();
    }

    vtkNew<vtkVectorFieldGeometry> geometry;
    geometry->SetInputData(field);
    geometry->SetMode(vtkVectorFieldGeometry::ModeStreamline);
    geometry->SetVectorArrayName(VectorArrayName);
    // A streamline stops up to a step before the end of the cells that it is traced through: 1 mm at most
    geometry->SetStreamlineInitialIntegrationStep(1.0);

    // Without a domain the streamline from the center is traced through the whole image
    vtkNew<vtkPoints> seeds;
    seeds->InsertNextPoint(0.0, 0.0, 0.0);
    geometry->SetStreamlineSeeds(seeds);
    CHECK_EXIT_SUCCESS(CheckWholeStreamlineFromCenter(GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort)));

    // With a domain the streamline stops where it leaves the domain
    geometry->SetStreamlineDomainArrayName(domainArrayName);
    vtkPolyData* lines = GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort);
    CHECK_BOOL(lines->GetNumberOfLines() > 0, true);
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    lines->GetBounds(bounds);
    std::cout << "  Streamline from " << bounds[0] << " to " << bounds[1] << " mm" << std::endl;
    CHECK_BOOL(bounds[0] >= -domainHalfSizeMm - 1e-6, true);
    CHECK_BOOL(bounds[1] <= domainHalfSizeMm + 1e-6, true);
    // The cells that are in the domain entirely reach at least 10 mm from the center, whatever the image orientation is
    CHECK_BOOL(bounds[0] < -10.0, true);
    CHECK_BOOL(bounds[1] > 10.0, true);

    // A seed outside the domain starts nothing
    seeds->SetPoint(0, 40.0, 0.0, 0.0);
    geometry->SetStreamlineSeeds(seeds);
    CHECK_INT(static_cast<int>(GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort)->GetNumberOfLines()), 0);

    // The threshold limits where streamlines are traced too. The streamline from the center gets near the edge of the
    // image at x = -50 mm, but stops in the slab at x > 0: the part beyond the slab, where the value is in the range
    // again, is not traced.
    geometry->SetStreamlineDomainArrayName("");
    geometry->SetThresholdEnabled(true);
    geometry->SetThresholdArrayName(thresholdArrayName);
    geometry->SetThresholdComponent(0);
    geometry->SetThresholdRange(0.5, 2.0);
    seeds->SetPoint(0, 0.0, 0.0, 0.0);
    geometry->SetStreamlineSeeds(seeds);
    lines = GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort);
    CHECK_BOOL(lines->GetNumberOfLines() > 0, true);
    lines->GetBounds(bounds);
    std::cout << "  Thresholded streamline from " << bounds[0] << " to " << bounds[1] << " mm" << std::endl;
    CHECK_BOOL(bounds[0] < -40.0, true);
    CHECK_BOOL(bounds[1] < slabMiddleMm, true);

    // With the domain, the streamline stops where it leaves either of them
    geometry->SetStreamlineDomainArrayName(domainArrayName);
    lines = GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort);
    CHECK_BOOL(lines->GetNumberOfLines() > 0, true);
    lines->GetBounds(bounds);
    CHECK_BOOL(bounds[0] >= -domainHalfSizeMm - 1e-6, true);
    CHECK_BOOL(bounds[0] < -10.0, true);
    CHECK_BOOL(bounds[1] < slabMiddleMm, true);
    geometry->SetStreamlineDomainArrayName("");

    // A seed where the value is out of the range starts nothing, although its streamline would get to where the value
    // is in the range
    seeds->SetPoint(0, slabMiddleMm, 0.0, 0.0);
    geometry->SetStreamlineSeeds(seeds);
    CHECK_INT(static_cast<int>(GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort)->GetNumberOfLines()), 0);

    // Thresholded by a component of the vectors, which is that of the field whatever the image orientation is: the y
    // component is 0.01 x, so the streamline is only traced where x is not negative
    geometry->SetThresholdArrayName(VectorArrayName);
    geometry->SetThresholdComponent(1);
    geometry->SetThresholdRange(0.0, 10.0);
    seeds->SetPoint(0, 0.0, 0.0, 0.0);
    geometry->SetStreamlineSeeds(seeds);
    lines = GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort);
    CHECK_BOOL(lines->GetNumberOfLines() > 0, true);
    lines->GetBounds(bounds);
    std::cout << "  Streamline thresholded by a vector component from " << bounds[0] << " to " << bounds[1] << " mm" << std::endl;
    CHECK_BOOL(bounds[0] > -1e-3, true);
    CHECK_BOOL(bounds[1] > 40.0, true);

    // Without the threshold the whole streamline is traced again, as exactly as the first time
    geometry->SetThresholdEnabled(false);
    geometry->SetStreamlineSeeds(seeds);
    CHECK_EXIT_SUCCESS(CheckWholeStreamlineFromCenter(GetOutput(geometry, vtkVectorFieldGeometry::StreamlineLinesPort)));
  }
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkVectorFieldGeometryTest1(int, char*[])
{
  vtkNew<vtkVectorFieldGeometry> geometry;
  EXERCISE_BASIC_OBJECT_METHODS(geometry.GetPointer());
  CHECK_EXIT_SUCCESS(TestNoneAndContour());
  CHECK_EXIT_SUCCESS(TestGrid());
  CHECK_EXIT_SUCCESS(TestStreamlines());
  CHECK_EXIT_SUCCESS(TestStreamlineDomain());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
