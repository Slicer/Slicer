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

// Checks the geometry that vtkVectorFieldGrid builds from a sampled lattice: the lines, their
// gaps, the deformation, the undeformed copy, the region clip, the tubes and the fill.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkVectorFieldGrid.h"
#include "vtkVectorFieldSampler.h"

// VTK includes
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

namespace
{

const char InsideArrayName[] = "Inside";

//----------------------------------------------------------------------------
/// A lattice of sizeX by sizeY points, 10 mm apart in the z = 0 plane, with the vector (1,0,0)
/// and magnitude 1 at every point, all of it inside and valid, and a quad for each lattice cell
vtkSmartPointer<vtkPolyData> CreateLattice(int sizeX, int sizeY)
{
  vtkSmartPointer<vtkPolyData> lattice = vtkSmartPointer<vtkPolyData>::New();
  vtkNew<vtkPoints> points;
  vtkNew<vtkDoubleArray> vectors;
  vectors->SetName(vtkVectorFieldSampler::GetVectorArrayName());
  vectors->SetNumberOfComponents(3);
  vtkNew<vtkDoubleArray> magnitudes;
  magnitudes->SetName(vtkVectorFieldSampler::GetMagnitudeArrayName());
  vtkNew<vtkUnsignedCharArray> valid;
  valid->SetName(vtkVectorFieldSampler::GetValidSampleArrayName());
  vtkNew<vtkDoubleArray> inside;
  inside->SetName(InsideArrayName);
  for (int j = 0; j < sizeY; ++j)
  {
    for (int i = 0; i < sizeX; ++i)
    {
      points->InsertNextPoint(10.0 * i, 10.0 * j, 0.0);
      vectors->InsertNextTuple3(1.0, 0.0, 0.0);
      magnitudes->InsertNextValue(1.0);
      valid->InsertNextValue(1);
      inside->InsertNextValue(1.0);
    }
  }
  lattice->SetPoints(points);
  lattice->GetPointData()->AddArray(vectors);
  lattice->GetPointData()->AddArray(magnitudes);
  lattice->GetPointData()->AddArray(valid);
  lattice->GetPointData()->AddArray(inside);

  vtkNew<vtkCellArray> quads;
  for (int j = 0; j + 1 < sizeY; ++j)
  {
    for (int i = 0; i + 1 < sizeX; ++i)
    {
      vtkIdType quad[4] = { j * sizeX + i, j * sizeX + i + 1, (j + 1) * sizeX + i + 1, (j + 1) * sizeX + i };
      quads->InsertNextCell(4, quad);
    }
  }
  lattice->SetPolys(quads);

  vtkNew<vtkIntArray> latticeSize;
  latticeSize->SetName(vtkVectorFieldSampler::GetLatticeSizeArrayName());
  latticeSize->InsertNextValue(sizeX);
  latticeSize->InsertNextValue(sizeY);
  latticeSize->InsertNextValue(1);
  lattice->GetFieldData()->AddArray(latticeSize);
  return lattice;
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkVectorFieldGrid> CreateGrid(vtkPolyData* lattice)
{
  vtkSmartPointer<vtkVectorFieldGrid> grid = vtkSmartPointer<vtkVectorFieldGrid>::New();
  grid->SetInputData(lattice);
  grid->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkVectorFieldSampler::GetVectorArrayName());
  return grid;
}

//----------------------------------------------------------------------------
int TestLines()
{
  vtkSmartPointer<vtkPolyData> lattice = CreateLattice(3, 3);
  vtkSmartPointer<vtkVectorFieldGrid> grid = CreateGrid(lattice);
  grid->Update();
  // A line through each row and each column; none along z, which has a single sample
  CHECK_INT(grid->GetOutput()->GetNumberOfLines(), 6);
  CHECK_INT(grid->GetOutput()->GetNumberOfPolys(), 0);

  // Every second row and column
  grid->SetSubdivision(2);
  grid->Update();
  CHECK_INT(grid->GetOutput()->GetNumberOfLines(), 4);

  // A line stops at a sample that is not valid and starts again after it
  vtkSmartPointer<vtkPolyData> row = CreateLattice(5, 1);
  vtkUnsignedCharArray::SafeDownCast(row->GetPointData()->GetArray(vtkVectorFieldSampler::GetValidSampleArrayName()))->SetValue(2, 0);
  vtkSmartPointer<vtkVectorFieldGrid> rowGrid = CreateGrid(row);
  rowGrid->Update();
  CHECK_INT(rowGrid->GetOutput()->GetNumberOfLines(), 2);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestDeformation()
{
  vtkSmartPointer<vtkPolyData> lattice = CreateLattice(3, 3);
  vtkSmartPointer<vtkVectorFieldGrid> grid = CreateGrid(lattice);
  double bounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  grid->SetScaleFactor(2.0);
  grid->Update();
  grid->GetOutput()->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[0], 2.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 22.0, 1e-9);

  // A negative factor deforms the grid the opposite way
  grid->SetScaleFactor(-2.0);
  grid->Update();
  grid->GetOutput()->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[0], -2.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 18.0, 1e-9);

  // The undeformed lines alone are where the samples are, on an output port of their own
  grid->SetScaleFactor(2.0);
  grid->DeformedLinesVisibilityOff();
  grid->UndeformedLinesVisibilityOn();
  grid->Update();
  vtkPolyData* undeformedLines = vtkPolyData::SafeDownCast(grid->GetOutputDataObject(vtkVectorFieldGrid::UndeformedLinesPort));
  CHECK_NOT_NULL(undeformedLines);
  CHECK_INT(grid->GetOutput()->GetNumberOfPoints(), 0);
  CHECK_INT(undeformedLines->GetNumberOfLines(), 6);
  undeformedLines->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[0], 0.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 20.0, 1e-9);

  // Both: each on its own port
  grid->DeformedLinesVisibilityOn();
  grid->Update();
  undeformedLines = vtkPolyData::SafeDownCast(grid->GetOutputDataObject(vtkVectorFieldGrid::UndeformedLinesPort));
  CHECK_INT(grid->GetOutput()->GetNumberOfLines(), 6);
  CHECK_INT(undeformedLines->GetNumberOfLines(), 6);
  grid->GetOutput()->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 22.0, 1e-9);
  undeformedLines->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 20.0, 1e-9);

  // Nothing to draw
  grid->DeformedLinesVisibilityOff();
  grid->UndeformedLinesVisibilityOff();
  grid->Update();
  CHECK_INT(grid->GetOutput()->GetNumberOfPoints(), 0);
  undeformedLines = vtkPolyData::SafeDownCast(grid->GetOutputDataObject(vtkVectorFieldGrid::UndeformedLinesPort));
  CHECK_INT(undeformedLines->GetNumberOfPoints(), 0);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestClipTubesAndFill()
{
  vtkSmartPointer<vtkPolyData> lattice = CreateLattice(3, 3);
  // The last column (x = 20) is outside
  vtkDataArray* inside = lattice->GetPointData()->GetArray(InsideArrayName);
  for (int j = 0; j < 3; ++j)
  {
    inside->SetComponent(j * 3 + 2, 0, 0.0);
  }
  vtkSmartPointer<vtkVectorFieldGrid> grid = CreateGrid(lattice);
  grid->SetScaleFactor(0.0);
  double bounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  // Not cut without a clip array
  grid->Update();
  grid->GetOutput()->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 20.0, 1e-9);

  // Cut halfway to the outside samples
  grid->SetClipArrayName(InsideArrayName);
  grid->Update();
  grid->GetOutput()->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 15.0, 1e-9);
  CHECK_INT(grid->GetOutput()->GetNumberOfPolys(), 0);

  // Tubes around the lines
  grid->SetLineDiameter(2.0);
  grid->Update();
  CHECK_INT(grid->GetOutput()->GetNumberOfLines(), 0);
  CHECK_BOOL(grid->GetOutput()->GetNumberOfPolys() + grid->GetOutput()->GetNumberOfStrips() > 0, true);
  grid->GetOutput()->GetBounds(bounds);
  // The tube radius is 1, but the extent depends on how the sides of the tube are oriented
  CHECK_BOOL(bounds[5] > 0.9 && bounds[5] < 1.0 + 1e-6, true);
  CHECK_BOOL(bounds[4] < -0.9 && bounds[4] > -1.0 - 1e-6, true);

  // The fill alone, cut the same way as the lines
  grid->SetLineDiameter(0.0);
  grid->DeformedLinesVisibilityOff();
  grid->FillOn();
  grid->Update();
  CHECK_INT(grid->GetOutput()->GetNumberOfLines(), 0);
  CHECK_BOOL(grid->GetOutput()->GetNumberOfPolys() > 0, true);
  grid->GetOutput()->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 15.0, 1e-9);

  // The fill is deformed as the lines are, and drawn with them
  grid->SetClipArrayName("");
  grid->SetScaleFactor(1.0);
  grid->DeformedLinesVisibilityOn();
  grid->Update();
  CHECK_INT(grid->GetOutput()->GetNumberOfLines(), 6);
  CHECK_INT(grid->GetOutput()->GetNumberOfPolys(), 4);
  grid->GetOutput()->GetBounds(bounds);
  CHECK_DOUBLE_TOLERANCE(bounds[0], 1.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(bounds[1], 21.0, 1e-9);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkVectorFieldGridTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestLines());
  CHECK_EXIT_SUCCESS(TestDeformation());
  CHECK_EXIT_SUCCESS(TestClipTubesAndFill());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
