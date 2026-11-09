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

// Fixtures shared by the vector field tests: meshes and images carrying an analytic field, so that samples can be
// compared with the true field.

#ifndef __vtkVectorFieldTestingHelpers_h
#define __vtkVectorFieldTestingHelpers_h

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAppendFilter.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkUnstructuredGrid.h>

// STD includes
#include <cmath>

namespace vtkVectorFieldTesting
{

/// A field that varies linearly, which every interpolation scheme used here reproduces
/// exactly inside the cells.
inline void LinearField(const double position[3], double vector[3])
{
  vector[0] = 1.0 + 0.02 * position[0];
  vector[1] = 0.5 - 0.01 * position[1];
  vector[2] = 0.03 * position[2];
}

/// A smooth field that is not linear, for checking how closely a fit approximates it.
inline void SmoothField(const double position[3], double vector[3])
{
  vector[0] = std::sin(position[0] / 30.0);
  vector[1] = std::cos(position[1] / 30.0);
  vector[2] = 0.02 * position[2];
}

/// Add a 3-component point data array to a data set, with the given field at each point.
inline void AddPointField(vtkDataSet* dataSet, const char* name, void (*field)(const double[3], double[3]))
{
  vtkNew<vtkDoubleArray> vectors;
  vectors->SetName(name);
  vectors->SetNumberOfComponents(3);
  vectors->SetNumberOfTuples(dataSet->GetNumberOfPoints());
  for (vtkIdType pointIndex = 0; pointIndex < dataSet->GetNumberOfPoints(); ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    dataSet->GetPoint(pointIndex, position);
    double vector[3] = { 0.0, 0.0, 0.0 };
    field(position, vector);
    vectors->SetTuple(pointIndex, vector);
  }
  dataSet->GetPointData()->AddArray(vectors);
}

/// Add a 3-component cell data array to a data set, with the given field at the center of
/// each cell (the mean of its points).
inline void AddCellField(vtkDataSet* dataSet, const char* name, void (*field)(const double[3], double[3]))
{
  vtkNew<vtkDoubleArray> vectors;
  vectors->SetName(name);
  vectors->SetNumberOfComponents(3);
  vectors->SetNumberOfTuples(dataSet->GetNumberOfCells());
  for (vtkIdType cellIndex = 0; cellIndex < dataSet->GetNumberOfCells(); ++cellIndex)
  {
    vtkCell* cell = dataSet->GetCell(cellIndex);
    double center[3] = { 0.0, 0.0, 0.0 };
    for (vtkIdType cellPointIndex = 0; cellPointIndex < cell->GetNumberOfPoints(); ++cellPointIndex)
    {
      double position[3] = { 0.0, 0.0, 0.0 };
      dataSet->GetPoint(cell->GetPointId(cellPointIndex), position);
      for (int axis = 0; axis < 3; ++axis)
      {
        center[axis] += position[axis] / cell->GetNumberOfPoints();
      }
    }
    double vector[3] = { 0.0, 0.0, 0.0 };
    field(center, vector);
    vectors->SetTuple(cellIndex, vector);
  }
  dataSet->GetCellData()->AddArray(vectors);
}

/// A scalar that varies linearly, for an array that a field is colored by.
inline double LinearScalar(const double position[3])
{
  return 2.0 + 0.1 * position[0] - 0.05 * position[2];
}

/// A smooth scalar that is not linear, for checking how closely a fit approximates it.
inline double SmoothScalar(const double position[3])
{
  return 3.0 + std::sin(position[0] / 30.0) + 0.5 * std::cos(position[2] / 25.0);
}

/// Add a 1-component point data array to a data set, with the given value at each point.
inline void AddPointScalar(vtkDataSet* dataSet, const char* name, double (*scalar)(const double[3]))
{
  vtkNew<vtkDoubleArray> values;
  values->SetName(name);
  values->SetNumberOfTuples(dataSet->GetNumberOfPoints());
  for (vtkIdType pointIndex = 0; pointIndex < dataSet->GetNumberOfPoints(); ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    dataSet->GetPoint(pointIndex, position);
    values->SetValue(pointIndex, scalar(position));
  }
  dataSet->GetPointData()->AddArray(values);
}

/// Add a 1-component cell data array to a data set, with the given value at the center of
/// each cell (the mean of its points).
inline void AddCellScalar(vtkDataSet* dataSet, const char* name, double (*scalar)(const double[3]))
{
  vtkNew<vtkDoubleArray> values;
  values->SetName(name);
  values->SetNumberOfTuples(dataSet->GetNumberOfCells());
  for (vtkIdType cellIndex = 0; cellIndex < dataSet->GetNumberOfCells(); ++cellIndex)
  {
    vtkCell* cell = dataSet->GetCell(cellIndex);
    double center[3] = { 0.0, 0.0, 0.0 };
    for (vtkIdType cellPointIndex = 0; cellPointIndex < cell->GetNumberOfPoints(); ++cellPointIndex)
    {
      double position[3] = { 0.0, 0.0, 0.0 };
      dataSet->GetPoint(cell->GetPointId(cellPointIndex), position);
      for (int axis = 0; axis < 3; ++axis)
      {
        center[axis] += position[axis] / cell->GetNumberOfPoints();
      }
    }
    values->SetValue(cellIndex, scalar(center));
  }
  dataSet->GetCellData()->AddArray(values);
}

/// A volumetric mesh: a cube from -halfSizeMm to halfSizeMm along each axis, divided into
/// voxel cells.
inline vtkSmartPointer<vtkUnstructuredGrid> CreateVolumetricMesh(int numberOfCellsPerAxis = 10, double halfSizeMm = 50.0)
{
  vtkNew<vtkImageData> image;
  image->SetDimensions(numberOfCellsPerAxis + 1, numberOfCellsPerAxis + 1, numberOfCellsPerAxis + 1);
  double spacingMm = 2.0 * halfSizeMm / numberOfCellsPerAxis;
  image->SetSpacing(spacingMm, spacingMm, spacingMm);
  image->SetOrigin(-halfSizeMm, -halfSizeMm, -halfSizeMm);
  vtkNew<vtkAppendFilter> toUnstructuredGrid;
  toUnstructuredGrid->SetInputData(image);
  toUnstructuredGrid->Update();
  vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  grid->DeepCopy(toUnstructuredGrid->GetOutput());
  return grid;
}

/// A surface mesh: a sphere of the given radius, centered at the origin.
inline vtkSmartPointer<vtkPolyData> CreateSurfaceMesh(double radiusMm = 50.0, int resolution = 40)
{
  vtkNew<vtkSphereSource> sphere;
  sphere->SetRadius(radiusMm);
  sphere->SetThetaResolution(resolution);
  sphere->SetPhiResolution(resolution);
  sphere->Update();
  vtkSmartPointer<vtkPolyData> surface = vtkSmartPointer<vtkPolyData>::New();
  surface->DeepCopy(sphere->GetOutput());
  // Only the arrays that a test adds
  surface->GetPointData()->Initialize();
  return surface;
}

/// A 3-component image holding the field at the voxels of a lattice over a cube from -halfSizeMm to halfSizeMm (RAS).
/// Spacing and origin are left at their defaults: the caller places the image (see AddVolumeWithFieldDisplay()).
inline vtkSmartPointer<vtkImageData> CreateVectorImage(void (*field)(const double[3], double[3]), int numberOfVoxelsPerAxis = 11, double halfSizeMm = 50.0)
{
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetDimensions(numberOfVoxelsPerAxis, numberOfVoxelsPerAxis, numberOfVoxelsPerAxis);
  image->AllocateScalars(VTK_DOUBLE, 3);
  double spacingMm = 2.0 * halfSizeMm / (numberOfVoxelsPerAxis - 1);
  for (int k = 0; k < numberOfVoxelsPerAxis; ++k)
  {
    for (int j = 0; j < numberOfVoxelsPerAxis; ++j)
    {
      for (int i = 0; i < numberOfVoxelsPerAxis; ++i)
      {
        double position[3] = { -halfSizeMm + i * spacingMm, -halfSizeMm + j * spacingMm, -halfSizeMm + k * spacingMm };
        double vector[3] = { 0.0, 0.0, 0.0 };
        field(position, vector);
        for (int component = 0; component < 3; ++component)
        {
          image->SetScalarComponentFromDouble(i, j, k, component, vector[component]);
        }
      }
    }
  }
  return image;
}

/// Distance between two positions.
inline double Distance(const double a[3], const double b[3])
{
  return std::sqrt(vtkMath::Distance2BetweenPoints(a, b));
}

/// The output of a connection, updated, as the given type of data set. nullptr if there is
/// no connection, or the output is not of that type.
template <class DataSetType = vtkDataSet>
DataSetType* UpdateConnection(vtkAlgorithmOutput* connection)
{
  if (!connection)
  {
    return nullptr;
  }
  connection->GetProducer()->Update();
  return DataSetType::SafeDownCast(connection->GetProducer()->GetOutputDataObject(connection->GetIndex()));
}

} // namespace vtkVectorFieldTesting

#endif
