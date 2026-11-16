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

// Checks vtkVectorFieldGlyph: which points get a glyph, the arrays it adds for them, the shape,
// and the glyph geometry, which has to match what vtkGlyph3DMapper draws on the GPU.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkVectorFieldGlyph.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkGlyph3D.h>
#include <vtkGlyph3DMapper.h>
#include <vtkInformation.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

// STD includes
#include <cmath>

namespace
{

//----------------------------------------------------------------------------
/// Ten points along X, 10 mm apart, with arrays "Vector" (glyph orientations, several directions), "Size" (3-component,
/// magnitude 1 + index), "Scalar" (index), and "Inside" (1 for the first seven points, 0 for the others).
vtkSmartPointer<vtkPolyData> CreatePoints()
{
  const double directions[10][3] = { { 1.0, 0.0, 0.0 }, { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 },  { 0.0, -2.0, 0.0 }, { 0.0, 0.0, 3.0 },
                                     { 1.0, 2.0, 3.0 }, { -2.0, 1.0, 0.5 }, { 0.3, -0.4, 2.0 }, { 2.0, 2.0, -1.0 }, { -1.0, -1.0, -1.0 } };
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPoints> points;
  vtkNew<vtkDoubleArray> vectors;
  vectors->SetName("Vector");
  vectors->SetNumberOfComponents(3);
  vtkNew<vtkDoubleArray> sizes;
  sizes->SetName("Size");
  sizes->SetNumberOfComponents(3);
  vtkNew<vtkDoubleArray> scalars;
  scalars->SetName("Scalar");
  vtkNew<vtkDoubleArray> inside;
  inside->SetName("Inside");
  for (int index = 0; index < 10; ++index)
  {
    points->InsertNextPoint(10.0 * index, 0.0, 0.0);
    vectors->InsertNextTuple(directions[index]);
    sizes->InsertNextTuple3(0.0, 0.6 * (1.0 + index), 0.8 * (1.0 + index));
    scalars->InsertNextValue(index);
    inside->InsertNextValue(index < 7 ? 1.0 : 0.0);
  }
  polyData->SetPoints(points);
  polyData->GetPointData()->AddArray(vectors);
  polyData->GetPointData()->AddArray(sizes);
  polyData->GetPointData()->AddArray(scalars);
  polyData->GetPointData()->AddArray(inside);
  return polyData;
}

//----------------------------------------------------------------------------
vtkPolyData* GetOutput(vtkVectorFieldGlyph* glyph, int port)
{
  glyph->Update();
  return vtkPolyData::SafeDownCast(glyph->GetOutputDataObject(port));
}

//----------------------------------------------------------------------------
/// Largest distance between the points of two meshes with the same points
double GetLargestPointDistance(vtkPolyData* mesh, vtkPolyData* reference)
{
  if (!mesh || !reference || mesh->GetNumberOfPoints() != reference->GetNumberOfPoints() || mesh->GetNumberOfPoints() == 0)
  {
    std::cerr << "Meshes with different numbers of points: " << (mesh ? mesh->GetNumberOfPoints() : -1) << " and " << (reference ? reference->GetNumberOfPoints() : -1)
              << std::endl;
    return VTK_DOUBLE_MAX;
  }
  double largestDistance = 0.0;
  for (vtkIdType pointIndex = 0; pointIndex < mesh->GetNumberOfPoints(); ++pointIndex)
  {
    largestDistance = std::max(largestDistance, std::sqrt(vtkMath::Distance2BetweenPoints(mesh->GetPoint(pointIndex), reference->GetPoint(pointIndex))));
  }
  return largestDistance;
}

//----------------------------------------------------------------------------
/// Name of the array that a glyph mapper uses for something (vtkGlyph3DMapper::ArrayIndexes)
std::string GetMapperArrayName(vtkGlyph3DMapper* mapper, int arrayIndex)
{
  vtkInformation* information = mapper->GetInputArrayInformation(arrayIndex);
  const char* name = (information ? information->Get(vtkDataObject::FIELD_NAME()) : nullptr);
  return name ? name : "";
}

//----------------------------------------------------------------------------
int TestPointSelection()
{
  std::cout << "Point selection" << std::endl;
  vtkSmartPointer<vtkPolyData> points = CreatePoints();
  vtkNew<vtkVectorFieldGlyph> glyph;
  glyph->SetInputData(points);
  CHECK_INT(static_cast<int>(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetNumberOfPoints()), 10);
  // The arrays come along
  CHECK_NOT_NULL(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetPointData()->GetArray("Size"));

  // Threshold by a scalar: 2 to 5 inclusive
  glyph->ThresholdEnabledOn();
  glyph->SetThresholdArrayName("Scalar");
  glyph->SetThresholdRange(2.0, 5.0);
  CHECK_INT(static_cast<int>(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetNumberOfPoints()), 4);
  // By a component of a vector array, and by its magnitude
  glyph->SetThresholdArrayName("Size");
  glyph->SetThresholdComponent(2);
  glyph->SetThresholdRange(0.0, 2.5);
  CHECK_INT(static_cast<int>(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetNumberOfPoints()), 3);
  glyph->SetThresholdComponent(-1);
  glyph->SetThresholdRange(0.0, 3.0);
  CHECK_INT(static_cast<int>(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetNumberOfPoints()), 3);
  // An array that the input does not have hides nothing
  glyph->SetThresholdArrayName("NoSuchArray");
  CHECK_INT(static_cast<int>(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetNumberOfPoints()), 10);
  glyph->ThresholdEnabledOff();

  // Only where the region array says the field is shown
  glyph->SetRegionArrayName("Inside");
  CHECK_INT(static_cast<int>(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetNumberOfPoints()), 7);

  // A random subset of the points that are shown, the same one every time
  glyph->SetSubsetMode(vtkVectorFieldGlyph::SubsetRandomBounds);
  glyph->SetMaximumNumberOfPoints(3);
  vtkNew<vtkPolyData> subset;
  subset->DeepCopy(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort));
  CHECK_BOOL(subset->GetNumberOfPoints() > 0 && subset->GetNumberOfPoints() <= 3, true);
  vtkDataArray* insideOfSubset = subset->GetPointData()->GetArray("Inside");
  CHECK_NOT_NULL(insideOfSubset);
  for (vtkIdType pointIndex = 0; pointIndex < subset->GetNumberOfPoints(); ++pointIndex)
  {
    CHECK_DOUBLE(insideOfSubset->GetComponent(pointIndex, 0), 1.0);
  }
  glyph->GenerateGeometryOn();
  vtkPolyData* sameSubset = GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort);
  CHECK_INT(static_cast<int>(sameSubset->GetNumberOfPoints()), static_cast<int>(subset->GetNumberOfPoints()));
  CHECK_DOUBLE(GetLargestPointDistance(sameSubset, subset), 0.0);

  // A spaced subset of the points that are shown: one for a spacing that holds them all, all of them for no spacing
  glyph->SetSubsetMode(vtkVectorFieldGlyph::SubsetSpaced);
  glyph->SetSubsetSpacing(1e6);
  vtkPolyData* spacedSubset = GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort);
  CHECK_BOOL(spacedSubset->GetNumberOfPoints() >= 1 && spacedSubset->GetNumberOfPoints() <= 8, true);
  vtkDataArray* insideOfSpacedSubset = spacedSubset->GetPointData()->GetArray("Inside");
  CHECK_NOT_NULL(insideOfSpacedSubset);
  for (vtkIdType pointIndex = 0; pointIndex < spacedSubset->GetNumberOfPoints(); ++pointIndex)
  {
    CHECK_DOUBLE(insideOfSpacedSubset->GetComponent(pointIndex, 0), 1.0);
  }
  glyph->SetSubsetSpacing(0.0);
  CHECK_INT(static_cast<int>(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetNumberOfPoints()), 7);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestGlyphArrays()
{
  std::cout << "Glyph arrays" << std::endl;
  vtkSmartPointer<vtkPolyData> points = CreatePoints();
  vtkNew<vtkVectorFieldGlyph> glyph;
  glyph->SetInputData(points);
  glyph->SetOrientationArrayName("Vector");
  glyph->SetScaleArrayName("Size");
  glyph->SetScaleFactor(2.0);

  // Directional scale: (length times the scale factor, 1, 1)
  glyph->DirectionalScaleOn();
  vtkDataArray* directionalScale = GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetPointData()->GetArray(vtkVectorFieldGlyph::GetDirectionalScaleArrayName());
  CHECK_NOT_NULL(directionalScale);
  CHECK_DOUBLE_TOLERANCE(directionalScale->GetComponent(3, 0), 2.0 * 4.0, 1e-12);
  CHECK_DOUBLE(directionalScale->GetComponent(3, 1), 1.0);
  CHECK_DOUBLE(directionalScale->GetComponent(3, 2), 1.0);
  // Flat glyphs are scaled uniformly
  glyph->FlatOn();
  CHECK_NULL(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort)->GetPointData()->GetArray(vtkVectorFieldGlyph::GetDirectionalScaleArrayName()));
  glyph->FlatOff();
  glyph->DirectionalScaleOff();

  // Projection to the glyph plane (the plane need not be orthonormal): the in-plane part of
  // the orientation vectors, in an array of their own, next to the original vectors
  vtkNew<vtkMatrix4x4> plane;
  plane->SetElement(0, 0, 0.5);
  plane->SetElement(1, 1, 0.5);
  glyph->SetGlyphPlane(plane);
  glyph->ProjectToPlaneOn();
  CHECK_STD_STRING(glyph->GetRenderedOrientationArrayName(), std::string("Vector") + vtkVectorFieldGlyph::GetProjectedArrayNameSuffix());
  vtkPolyData* glyphPoints = GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort);
  vtkDataArray* projected = glyphPoints->GetPointData()->GetArray(glyph->GetRenderedOrientationArrayName().c_str());
  CHECK_NOT_NULL(projected);
  double vector[3] = { 0.0, 0.0, 0.0 };
  projected->GetTuple(5, vector);
  CHECK_DOUBLE_TOLERANCE(vector[0], 1.0, 1e-12);
  CHECK_DOUBLE_TOLERANCE(vector[1], 2.0, 1e-12);
  CHECK_DOUBLE_TOLERANCE(vector[2], 0.0, 1e-12);
  CHECK_DOUBLE(glyphPoints->GetPointData()->GetArray("Vector")->GetComponent(5, 2), 3.0);
  // Glyphs that are scaled by the vectors that orient them are scaled by the projected ones
  glyph->SetScaleArrayName("Vector");
  CHECK_STD_STRING(glyph->GetRenderedScaleArrayName(), glyph->GetRenderedOrientationArrayName());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestShape()
{
  std::cout << "Glyph shape" << std::endl;
  vtkNew<vtkVectorFieldGlyph> glyph;
  // The shape is there without an input, one unit long along +X (a sphere is a polygonal
  // approximation, a little shorter)
  for (int glyphType = 0; glyphType < vtkVectorFieldGlyph::Glyph_Last; ++glyphType)
  {
    glyph->SetGlyphType(glyphType);
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    GetOutput(glyph, vtkVectorFieldGlyph::GlyphShapePort)->GetBounds(bounds);
    CHECK_DOUBLE_TOLERANCE(bounds[1] - bounds[0], 1.0, 0.01);
  }
  // Directional flat glyphs start at the point and lie in the XY plane
  glyph->SetGlyphType(vtkVectorFieldGlyph::GlyphArrow);
  glyph->FlatOn();
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  GetOutput(glyph, vtkVectorFieldGlyph::GlyphShapePort)->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[0], 0.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 1.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(bounds[4], 0.0, 1e-12);
  CHECK_DOUBLE_TOLERANCE(bounds[5], 0.0, 1e-12);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// The geometry is placed and sized as vtkGlyph3D places and sizes the glyphs, which follows the
/// same rules as vtkGlyph3DMapper on the GPU, for the scale modes that both support
int TestGeometryMatchesGlyph3D()
{
  std::cout << "Glyph geometry" << std::endl;
  vtkSmartPointer<vtkPolyData> points = CreatePoints();
  vtkNew<vtkVectorFieldGlyph> glyph;
  glyph->SetInputData(points);
  glyph->SetOrientationArrayName("Vector");
  glyph->GenerateGeometryOn();
  // Off, the geometry is not computed
  glyph->GenerateGeometryOff();
  CHECK_INT(static_cast<int>(GetOutput(glyph, vtkVectorFieldGlyph::GlyphGeometryPort)->GetNumberOfPoints()), 0);
  glyph->GenerateGeometryOn();

  struct Case
  {
    const char* Description;
    const char* ScaleArrayName;
    int ScaleMode;
    double ScaleFactor;
  };
  const Case cases[] = {
    { "no scale array", "", vtkVectorFieldGlyph::ScaleNone, 3.0 },
    { "by a scalar", "Scalar", vtkVectorFieldGlyph::ScaleByMagnitude, 0.5 },
    { "by the magnitude of the orientation vectors", "Vector", vtkVectorFieldGlyph::ScaleByMagnitude, 2.0 },
    { "by the components of the orientation vectors", "Vector", vtkVectorFieldGlyph::ScaleByComponents, 2.0 },
    { "reversed", "Vector", vtkVectorFieldGlyph::ScaleByMagnitude, -2.0 },
  };
  for (const Case& testCase : cases)
  {
    glyph->SetScaleArrayName(testCase.ScaleArrayName);
    glyph->SetScaleMode(testCase.ScaleMode);
    glyph->SetScaleFactor(testCase.ScaleFactor);
    vtkPolyData* geometry = GetOutput(glyph, vtkVectorFieldGlyph::GlyphGeometryPort);

    vtkNew<vtkGlyph3D> reference;
    reference->SetInputData(GetOutput(glyph, vtkVectorFieldGlyph::GlyphPointsPort));
    reference->SetSourceData(GetOutput(glyph, vtkVectorFieldGlyph::GlyphShapePort));
    reference->OrientOn();
    reference->SetVectorModeToUseVector();
    reference->SetInputArrayToProcess(1, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "Vector");
    reference->SetScaleFactor(testCase.ScaleFactor);
    switch (testCase.ScaleMode)
    {
      case vtkVectorFieldGlyph::ScaleByMagnitude:
        if (std::string(testCase.ScaleArrayName) == "Scalar")
        {
          reference->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "Scalar");
          reference->SetScaleModeToScaleByScalar();
        }
        else
        {
          reference->SetScaleModeToScaleByVector();
        }
        break;
      case vtkVectorFieldGlyph::ScaleByComponents: reference->SetScaleModeToScaleByVectorComponents(); break;
      default: reference->SetScaleModeToDataScalingOff(); break;
    }
    reference->Update();
    double largestDistance = GetLargestPointDistance(geometry, reference->GetOutput());
    std::cout << "  Scaled " << testCase.Description << ": largest distance from vtkGlyph3D " << largestDistance << std::endl;
    // The first point has a zero scalar: vtkGlyph3D and vtkGlyph3DMapper shrink it to almost
    // nothing. vtkGlyph3D stores single precision points, about 1e-5 mm apart at 90 mm.
    CHECK_BOOL(largestDistance < 1e-4, true);
    CHECK_INT(static_cast<int>(geometry->GetNumberOfPolys()), static_cast<int>(10 * GetOutput(glyph, vtkVectorFieldGlyph::GlyphShapePort)->GetNumberOfPolys()));
    // Each glyph carries the point data of its point, for coloring
    CHECK_NOT_NULL(geometry->GetPointData()->GetArray("Scalar"));
  }

  // A separate scale array, which vtkGlyph3D cannot scale by: the arrow along the vector at the
  // point, as long as the scale array is
  glyph->SetScaleArrayName("Size");
  glyph->SetScaleMode(vtkVectorFieldGlyph::ScaleByMagnitude);
  glyph->SetScaleFactor(1.0);
  glyph->SetRegionArrayName("Inside");
  glyph->ThresholdEnabledOn();
  glyph->SetThresholdArrayName("Scalar");
  glyph->SetThresholdRange(2.0, 2.0);
  vtkPolyData* geometry = GetOutput(glyph, vtkVectorFieldGlyph::GlyphGeometryPort);
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  geometry->GetBounds(bounds);
  // The third point, at x = 20, pointing along +Y, with a size of 3
  CHECK_DOUBLE_TOLERANCE(bounds[2], 0.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(bounds[3], 3.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(0.5 * (bounds[0] + bounds[1]), 20.0, 1e-6);

  // Directional scale: only the length follows the scale array, the thickness stays
  glyph->DirectionalScaleOn();
  geometry = GetOutput(glyph, vtkVectorFieldGlyph::GlyphGeometryPort);
  double directionalBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  geometry->GetBounds(directionalBounds);
  CHECK_DOUBLE_TOLERANCE(directionalBounds[3], 3.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(directionalBounds[1] - directionalBounds[0], (bounds[1] - bounds[0]) / 3.0, 1e-6);

  // The mapper gets the same settings
  vtkNew<vtkGlyph3DMapper> mapper;
  glyph->ConfigureMapper(mapper);
  CHECK_INT(mapper->GetScaleMode(), vtkGlyph3DMapper::SCALE_BY_COMPONENTS);
  CHECK_DOUBLE(mapper->GetScaleFactor(), 1.0);
  CHECK_STD_STRING(GetMapperArrayName(mapper, vtkGlyph3DMapper::SCALE), vtkVectorFieldGlyph::GetDirectionalScaleArrayName());
  CHECK_BOOL(mapper->GetOrient(), true);
  CHECK_STD_STRING(GetMapperArrayName(mapper, vtkGlyph3DMapper::ORIENTATION), "Vector");
  CHECK_POINTER(mapper->GetInputConnection(1, 0), glyph->GetOutputPort(vtkVectorFieldGlyph::GlyphShapePort));
  glyph->DirectionalScaleOff();
  glyph->ConfigureMapper(mapper);
  CHECK_INT(mapper->GetScaleMode(), vtkGlyph3DMapper::SCALE_BY_MAGNITUDE);
  CHECK_STD_STRING(GetMapperArrayName(mapper, vtkGlyph3DMapper::SCALE), "Size");
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestFlatGlyphs()
{
  std::cout << "Flat glyphs" << std::endl;
  vtkSmartPointer<vtkPolyData> points = CreatePoints();
  vtkNew<vtkVectorFieldGlyph> glyph;
  glyph->SetInputData(points);
  glyph->SetOrientationArrayName("Vector");
  glyph->SetScaleArrayName("Vector");
  glyph->FlatOn();
  glyph->GenerateGeometryOn();
  // A sagittal plane at x = 30, 0.5 mm pixels: its X axis along A, its Y axis along S
  vtkNew<vtkMatrix4x4> plane;
  plane->Zero();
  plane->SetElement(1, 0, 0.5);
  plane->SetElement(2, 1, 0.5);
  plane->SetElement(0, 2, 0.5);
  plane->SetElement(0, 3, 30.0);
  plane->SetElement(3, 3, 1.0);
  glyph->SetGlyphPlane(plane);
  glyph->ProjectToPlaneOn();
  glyph->ThresholdEnabledOn();
  glyph->SetThresholdArrayName("Scalar");
  // The fourth point, at x = 30, which points along -A, two long
  glyph->SetThresholdRange(3.0, 3.0);
  vtkPolyData* geometry = GetOutput(glyph, vtkVectorFieldGlyph::GlyphGeometryPort);
  CHECK_BOOL(geometry->GetNumberOfPoints() > 0, true);
  double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  geometry->GetBounds(bounds);
  // In the plane, from the point along -A, as long as the vector
  CHECK_DOUBLE_TOLERANCE(bounds[0], 30.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 30.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(bounds[2], -2.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(bounds[3], 0.0, 1e-6);

  // A vector out of the plane: the glyph shows its in-plane part, and stays in the plane.
  // The sixth point, at x = 50, (1, 2, 3); the plane is moved to it.
  plane->SetElement(0, 3, 50.0);
  glyph->SetGlyphPlane(plane);
  glyph->SetThresholdRange(5.0, 5.0);
  geometry = GetOutput(glyph, vtkVectorFieldGlyph::GlyphGeometryPort);
  geometry->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[0], 50.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 50.0, 1e-9);
  // From the point to (2, 3) in the plane, which is sqrt(13) long
  CHECK_DOUBLE_TOLERANCE(bounds[3], 2.0, 0.1);
  CHECK_DOUBLE_TOLERANCE(bounds[5], 3.0, 0.1);

  // Scaled uniformly: by the magnitude even if the scale mode is by components, and not
  // directionally
  glyph->SetScaleMode(vtkVectorFieldGlyph::ScaleByComponents);
  glyph->DirectionalScaleOn();
  double uniformBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  GetOutput(glyph, vtkVectorFieldGlyph::GlyphGeometryPort)->GetBounds(uniformBounds);
  for (int index = 0; index < 6; ++index)
  {
    CHECK_DOUBLE_TOLERANCE(uniformBounds[index], bounds[index], 1e-9);
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestModified()
{
  std::cout << "Settings that only the mapper uses" << std::endl;
  vtkSmartPointer<vtkPolyData> points = CreatePoints();
  vtkNew<vtkVectorFieldGlyph> glyph;
  glyph->SetInputData(points);
  glyph->Update();
  // Dragging the scale slider does not select the points again
  vtkMTimeType mTime = glyph->GetMTime();
  glyph->SetScaleFactor(5.0);
  glyph->SetScaleArrayName("Size");
  glyph->SetOrientationArrayName("Vector");
  CHECK_INT(static_cast<int>(glyph->GetMTime()), static_cast<int>(mTime));
  // unless the outputs depend on them
  glyph->GenerateGeometryOn();
  mTime = glyph->GetMTime();
  glyph->SetScaleFactor(6.0);
  CHECK_BOOL(glyph->GetMTime() > mTime, true);

  // A copy has every setting
  vtkNew<vtkVectorFieldGlyph> copy;
  copy->CopySettings(glyph);
  CHECK_DOUBLE(copy->GetScaleFactor(), 6.0);
  CHECK_STD_STRING(copy->GetScaleArrayName(), "Size");
  CHECK_STD_STRING(copy->GetOrientationArrayName(), "Vector");
  CHECK_BOOL(copy->GetGenerateGeometry(), true);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkVectorFieldGlyphTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestPointSelection());
  CHECK_EXIT_SUCCESS(TestGlyphArrays());
  CHECK_EXIT_SUCCESS(TestShape());
  CHECK_EXIT_SUCCESS(TestGeometryMatchesGlyph3D());
  CHECK_EXIT_SUCCESS(TestFlatGlyphs());
  CHECK_EXIT_SUCCESS(TestModified());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
