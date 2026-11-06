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
// compared with the true field, and the nodes that show them.

#ifndef __vtkMRMLVectorFieldTestingHelpers_h
#define __vtkMRMLVectorFieldTestingHelpers_h

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkVectorFieldSampler.h"
#include "vtkMRMLVectorVolumeNode.h"

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAppendFilter.h>
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
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVector.h>

// STD includes
#include <cmath>
#include <vector>

namespace vtkMRMLVectorFieldTesting
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

/// Save a scene as XML and load that into another scene, as saving and loading a scene file
/// does. Returns false if either failed.
inline bool ReloadScene(vtkMRMLScene* scene, vtkMRMLScene* reloadedScene)
{
  scene->SetSaveToXMLString(1);
  if (!scene->Commit())
  {
    return false;
  }
  reloadedScene->SetLoadFromXMLString(1);
  reloadedScene->SetSceneXMLString(scene->GetSceneXMLString());
  return (reloadedScene->Import() != 0);
}

/// A vector field display node for a node of the scene. It samples on a regular lattice:
/// the checks of the tests were written for that pattern.
inline vtkMRMLVectorFieldDisplayNode* AddFieldDisplayNode(vtkMRMLScene* scene, vtkMRMLDisplayableNode* displayableNode)
{
  vtkMRMLVectorFieldDisplayNode* displayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorFieldDisplayNode"));
  displayableNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  return displayNode;
}

/// A model node in the scene, with the given mesh and a vector field display node.
inline vtkMRMLVectorFieldDisplayNode* AddModelWithFieldDisplay(vtkMRMLScene* scene, vtkPointSet* mesh)
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObserveMesh(mesh);
  return AddFieldDisplayNode(scene, modelNode);
}

/// A vector volume node with a vector field display node, its image placed over a cube from -halfSizeMm to halfSizeMm
/// as CreateVectorImage() lays the voxels out (volume geometry is the node's, not the image's).
inline vtkMRMLVectorFieldDisplayNode* AddVolumeWithFieldDisplay(vtkMRMLScene* scene, vtkImageData* image, double halfSizeMm = 50.0)
{
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  volumeNode->SetAndObserveImageData(image);
  int dimensions[3] = { 2, 2, 2 };
  image->GetDimensions(dimensions);
  double spacingMm = 2.0 * halfSizeMm / (dimensions[0] - 1);
  volumeNode->SetSpacing(spacingMm, spacingMm, spacingMm);
  volumeNode->SetOrigin(-halfSizeMm, -halfSizeMm, -halfSizeMm);
  return AddFieldDisplayNode(scene, volumeNode);
}

//----------------------------------------------------------------------------
/// A slice view sampler configured by the display node; nullptr if the field cannot be resampled in a slice.
inline vtkSmartPointer<vtkVectorFieldSampler> CreateSliceFieldSampler(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  vtkSmartPointer<vtkVectorFieldSampler> sampler = vtkSmartPointer<vtkVectorFieldSampler>::New();
  return (displayNode->UpdateSliceFieldSampler(sampler) ? sampler : nullptr);
}

/// A point list node in the scene with the given control points: where a field is sampled,
/// or streamlines start from.
inline vtkMRMLMarkupsFiducialNode* AddPointList(vtkMRMLScene* scene, const std::vector<vtkVector3d>& positions)
{
  vtkMRMLMarkupsFiducialNode* pointListNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
  for (const vtkVector3d& position : positions)
  {
    pointListNode->AddControlPoint(position);
  }
  return pointListNode;
}

/// A region of interest in the scene: an axis-aligned box with the given center and half sizes.
inline vtkMRMLMarkupsROINode* AddROI(vtkMRMLScene* scene, const vtkVector3d& center, const vtkVector3d& radius)
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsROINode"));
  roiNode->SetXYZ(center[0], center[1], center[2]);
  roiNode->SetRadiusXYZ(radius[0], radius[1], radius[2]);
  return roiNode;
}

/// A linear transform node in the scene that moves everything by the given offset.
inline vtkMRMLLinearTransformNode* AddTranslation(vtkMRMLScene* scene, double x, double y, double z)
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
  vtkNew<vtkTransform> translation;
  translation->Translate(x, y, z);
  transformNode->SetMatrixTransformToParent(translation->GetMatrix());
  return transformNode;
}

/// A non-linear transform node that moves everything by offsetX along X
/// (a thin-plate spline reproduces the affine part of its landmark displacements exactly).
inline vtkMRMLTransformNode* AddNonLinearTranslation(vtkMRMLScene* scene, double offsetX)
{
  vtkNew<vtkPoints> sourceLandmarks;
  vtkNew<vtkPoints> targetLandmarks;
  const double landmarks[5][3] = { { 0, 0, 0 }, { 100, 0, 0 }, { 0, 100, 0 }, { 0, 0, 100 }, { 100, 100, 100 } };
  for (const double* landmark : landmarks)
  {
    sourceLandmarks->InsertNextPoint(landmark);
    targetLandmarks->InsertNextPoint(landmark[0] + offsetX, landmark[1], landmark[2]);
  }
  vtkNew<vtkThinPlateSplineTransform> thinPlateSpline;
  thinPlateSpline->SetBasisToR();
  thinPlateSpline->SetSourceLandmarks(sourceLandmarks);
  thinPlateSpline->SetTargetLandmarks(targetLandmarks);
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  transformNode->SetAndObserveTransformToParent(thinPlateSpline);
  return transformNode;
}

} // namespace vtkMRMLVectorFieldTesting

#endif
