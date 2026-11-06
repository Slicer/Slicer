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

// Checks the fits vtkMRMLVectorFieldPipeline makes to draw a field between its points: the vector fit, its reach around
// the points, the fit of the coloring values, and that the fits are only set up again when their input changes.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLVectorFieldPipeline.h"

// vtkAddon includes
#include <vtkScatteredBSplineTransform.h>

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

// STD includes
#include <cmath>

namespace
{

//----------------------------------------------------------------------------
/// Points on a 5 x 5 x 5 lattice, 10 mm apart, with the vector (1 + 0.01 x, 0, 0) and the
/// value x / 10 at each of them
vtkSmartPointer<vtkPolyData> CreatePoints()
{
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPoints> points;
  vtkNew<vtkDoubleArray> vectors;
  vectors->SetName("Vector");
  vectors->SetNumberOfComponents(3);
  vtkNew<vtkDoubleArray> values;
  values->SetName("Value");
  for (int k = 0; k < 5; ++k)
  {
    for (int j = 0; j < 5; ++j)
    {
      for (int i = 0; i < 5; ++i)
      {
        double x = 10.0 * i;
        points->InsertNextPoint(x, 10.0 * j, 10.0 * k);
        vectors->InsertNextTuple3(1.0 + 0.01 * x, 0.0, 0.0);
        values->InsertNextValue(x / 10.0);
      }
    }
  }
  polyData->SetPoints(points);
  polyData->GetPointData()->AddArray(vectors);
  polyData->GetPointData()->AddArray(values);
  return polyData;
}

//----------------------------------------------------------------------------
int TestVectorFit()
{
  std::cout << "Fit of the vectors" << std::endl;
  vtkSmartPointer<vtkPolyData> points = CreatePoints();
  vtkNew<vtkMRMLVectorFieldPipeline> interpolation;

  // Nothing to fit
  CHECK_NULL(interpolation->UpdateVectorFit(points, "NoSuchArray", 0.0));
  CHECK_NULL(interpolation->UpdateVectorFit(points, "Value", 0.0));
  CHECK_NULL(interpolation->UpdateVectorFit(nullptr, "Vector", 0.0));

  // Automatic distance: a tenth of the diagonal of the bounds (40 mm along each axis)
  vtkScatteredBSplineTransform* fit = interpolation->UpdateVectorFit(points, "Vector", 0.0);
  CHECK_NOT_NULL(fit);
  CHECK_DOUBLE_TOLERANCE(interpolation->GetFittedMaximumDistance(), 0.1 * std::sqrt(3.0) * 40.0, 1e-9);
  // The fit reaches that far beyond the outermost points
  double gridBounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  fit->GetGridBounds(gridBounds);
  CHECK_DOUBLE_TOLERANCE(gridBounds[0], -interpolation->GetFittedMaximumDistance(), 1e-9);
  CHECK_DOUBLE_TOLERANCE(gridBounds[1], 40.0 + interpolation->GetFittedMaximumDistance(), 1e-9);
  // A distance that is set
  CHECK_POINTER(interpolation->UpdateVectorFit(points, "Vector", 5.0), fit);
  CHECK_DOUBLE(interpolation->GetFittedMaximumDistance(), 5.0);

  // It reproduces the vectors between the points
  fit->Update();
  double position[3] = { 15.0, 15.0, 15.0 };
  double moved[3] = { 0.0, 0.0, 0.0 };
  fit->TransformPoint(position, moved);
  CHECK_DOUBLE_TOLERANCE(moved[0] - position[0], 1.15, 0.01);
  CHECK_DOUBLE_TOLERANCE(moved[1] - position[1], 0.0, 0.01);

  // Setting the same input again does not make it fit again
  vtkMTimeType fitMTime = fit->GetMTime();
  interpolation->UpdateVectorFit(points, "Vector", 5.0);
  CHECK_INT(static_cast<int>(fit->GetMTime()), static_cast<int>(fitMTime));
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestColorFit()
{
  std::cout << "Fit of the colors" << std::endl;
  vtkSmartPointer<vtkPolyData> points = CreatePoints();
  vtkNew<vtkMRMLVectorFieldPipeline> interpolation;
  vtkDataArray* values = points->GetPointData()->GetArray("Value");

  // On the lattice of the vectors, so there is no fit of the colors without one of them
  CHECK_NULL(interpolation->UpdateColorFit(points, values, -1));
  vtkScatteredBSplineTransform* vectorFit = interpolation->UpdateVectorFit(points, "Vector", 0.0);
  vtkScatteredBSplineTransform* colorFit = interpolation->UpdateColorFit(points, values, -1);
  CHECK_NOT_NULL(colorFit);
  double vectorBounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  double colorBounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  vectorFit->GetGridBounds(vectorBounds);
  colorFit->GetGridBounds(colorBounds);
  CHECK_DOUBLE(colorBounds[1], vectorBounds[1]);

  // The value is the first component of the displacement
  colorFit->Update();
  double position[3] = { 25.0, 10.0, 10.0 };
  double moved[3] = { 0.0, 0.0, 0.0 };
  colorFit->TransformPoint(position, moved);
  CHECK_DOUBLE_TOLERANCE(moved[0] - position[0], 2.5, 0.01);

  // The values are only copied again when they change
  vtkMTimeType colorMTime = colorFit->GetMTime();
  interpolation->UpdateColorFit(points, values, -1);
  CHECK_INT(static_cast<int>(colorFit->GetMTime()), static_cast<int>(colorMTime));
  values->SetComponent(0, 0, 7.0);
  values->Modified();
  interpolation->UpdateColorFit(points, values, -1);
  CHECK_BOOL(colorFit->GetMTime() > colorMTime, true);

  // Values that do not belong to the points
  vtkNew<vtkDoubleArray> tooFewValues;
  tooFewValues->SetNumberOfValues(3);
  CHECK_NULL(interpolation->UpdateColorFit(points, tooFewValues, -1));
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldPipelineTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestVectorFit());
  CHECK_EXIT_SUCCESS(TestColorFit());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
