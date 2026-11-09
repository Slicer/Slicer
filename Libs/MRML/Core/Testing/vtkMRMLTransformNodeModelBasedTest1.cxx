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

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"

// vtkAddon includes
#include <vtkMeshDisplacementTransform.h>
#include <vtkScatteredBSplineTransform.h>

// VTK includes
#include <vtkAppendFilter.h>
#include <vtkBSplineTransform.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVector.h>
#include <vtkWarpVector.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{

//----------------------------------------------------------------------------
/// A field that varies linearly, which every interpolation scheme used here reproduces
/// exactly inside the cells.
void LinearField(const double position[3], double vector[3])
{
  vector[0] = 1.0 + 0.02 * position[0];
  vector[1] = 0.5 - 0.01 * position[1];
  vector[2] = 0.03 * position[2];
}

//----------------------------------------------------------------------------
/// A smooth field that is not linear, for checking how closely a fit approximates it.
void SmoothField(const double position[3], double vector[3])
{
  vector[0] = std::sin(position[0] / 30.0);
  vector[1] = std::cos(position[1] / 30.0);
  vector[2] = 0.02 * position[2];
}

//----------------------------------------------------------------------------
/// Add a 3-component point data array to a data set, with the given field at each point.
void AddPointField(vtkDataSet* dataSet, const char* name, void (*field)(const double[3], double[3]))
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

//----------------------------------------------------------------------------
/// A volumetric mesh: a cube from -halfSizeMm to halfSizeMm along each axis, divided into
/// voxel cells.
vtkSmartPointer<vtkUnstructuredGrid> CreateVolumetricMesh(int numberOfCellsPerAxis = 10, double halfSizeMm = 50.0)
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

//----------------------------------------------------------------------------
/// A surface mesh: a sphere of the given radius, centered at the origin.
vtkSmartPointer<vtkPolyData> CreateSurfaceMesh(double radiusMm = 50.0, int resolution = 40)
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

//----------------------------------------------------------------------------
/// Distance between two positions.
double Distance(const double a[3], const double b[3])
{
  return std::sqrt(vtkMath::Distance2BetweenPoints(a, b));
}

//----------------------------------------------------------------------------
/// Save a scene with its data files into a directory and load it into another scene.
/// Returns false if either failed.
bool ReloadScene(vtkMRMLScene* scene, vtkMRMLScene* reloadedScene, const std::string& directory)
{
  vtksys::SystemTools::RemoveADirectory(directory);
  vtksys::SystemTools::MakeDirectory(directory);
  if (!scene->SaveSceneToSlicerDataBundleDirectory(directory.c_str()))
  {
    return false;
  }
  std::string sceneFileName = directory + "/" + vtksys::SystemTools::GetFilenameWithoutExtension(directory) + ".mrml";
  reloadedScene->SetURL(sceneFileName.c_str());
  return (reloadedScene->Import() != 0);
}

//----------------------------------------------------------------------------
/// The model-based transforms (vtkScatteredBSplineTransform, vtkMeshDisplacementTransform) of a transform.
std::vector<vtkAbstractTransform*> GetModelBasedTransforms(vtkAbstractTransform* transform)
{
  std::vector<vtkAbstractTransform*> modelBasedTransforms;
  vtkNew<vtkCollection> components;
  vtkMRMLTransformNode::FlattenGeneralTransform(components, transform);
  for (int index = 0; index < components->GetNumberOfItems(); ++index)
  {
    vtkObject* component = components->GetItemAsObject(index);
    if (vtkScatteredBSplineTransform::SafeDownCast(component) || vtkMeshDisplacementTransform::SafeDownCast(component))
    {
      modelBasedTransforms.push_back(vtkAbstractTransform::SafeDownCast(component));
    }
  }
  return modelBasedTransforms;
}

//----------------------------------------------------------------------------
/// The mesh that a model-based transform is computed from.
vtkPointSet* GetInputPointSet(vtkAbstractTransform* transform)
{
  transform->Update();
  if (vtkScatteredBSplineTransform* bsplineTransform = vtkScatteredBSplineTransform::SafeDownCast(transform))
  {
    return bsplineTransform->GetInputPointSet();
  }
  if (vtkMeshDisplacementTransform* meshTransform = vtkMeshDisplacementTransform::SafeDownCast(transform))
  {
    return meshTransform->GetInputPointSet();
  }
  return nullptr;
}

//----------------------------------------------------------------------------
/// Largest distance between where the transform maps the given positions and where the
/// field says they move to.
double GetLargestError(vtkAbstractTransform* transform, const std::vector<vtkVector3d>& positions, void (*field)(const double[3], double[3]))
{
  double largestError = 0.0;
  for (const vtkVector3d& position : positions)
  {
    double displacement[3] = { 0.0, 0.0, 0.0 };
    field(position.GetData(), displacement);
    double expected[3] = { position[0] + displacement[0], position[1] + displacement[1], position[2] + displacement[2] };
    double actual[3] = { 0.0, 0.0, 0.0 };
    transform->TransformPoint(position.GetData(), actual);
    largestError = std::max(largestError, Distance(expected, actual));
  }
  return largestError;
}

//----------------------------------------------------------------------------
/// Distance that the transform moves a position
double GetMotion(vtkAbstractTransform* transform, double x, double y, double z)
{
  double position[3] = { x, y, z };
  double moved[3] = { 0.0, 0.0, 0.0 };
  transform->TransformPoint(position, moved);
  return Distance(position, moved);
}

//----------------------------------------------------------------------------
struct EventCounter
{
  int Count{ 0 };
  static void Callback(vtkObject*, unsigned long, void* clientData, void*) { ++static_cast<EventCounter*>(clientData)->Count; }
};

//----------------------------------------------------------------------------
/// A transform node with a B-spline fitted to the displacements of a model
int TestBSpline(const std::string& tempDir)
{
  std::cout << "B-spline fit" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));

  // Displacements at the points of a sphere, 50 mm in radius
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0);
  AddPointField(surface, "Displacement", LinearField);
  AddPointField(surface, "Other", SmoothField);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObserveMesh(surface);

  EventCounter transformModified;
  vtkNew<vtkCallbackCommand> callback;
  callback->SetCallback(EventCounter::Callback);
  callback->SetClientData(&transformModified);
  transformNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, callback);

  vtkNew<vtkScatteredBSplineTransform> bsplineTransform;
  bsplineTransform->SetDisplacementArrayName("Displacement");
  bsplineTransform->UseLinearApproximationOn();
  bsplineTransform->SetGridBoundsMargin(15.0);
  bsplineTransform->SetMinimumControlPointSpacing(1.0);
  bsplineTransform->SetTolerance(0.001);
  transformNode->SetAndObserveTransformToParent(bsplineTransform);
  // Saved in a file, like any other transform
  vtkSmartPointer<vtkMRMLStorageNode> storageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take(transformNode->CreateDefaultStorageNode());
  CHECK_NOT_NULL(storageNode);
  // No model yet: the identity, quietly
  CHECK_NULL(transformNode->GetDisplacementModelNode(bsplineTransform));
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 10.0, -20.0, 30.0), 0.0, 1e-9);

  int countBefore = transformModified.Count;
  transformNode->SetAndObserveDisplacementModelNode(bsplineTransform, modelNode);
  CHECK_POINTER(transformNode->GetDisplacementModelNode(bsplineTransform), modelNode);
  // Also found from its inverse, which is the transform from parent
  CHECK_POINTER(transformNode->GetDisplacementModelNode(transformNode->GetTransformFromParent()), modelNode);
  CHECK_POINTER(bsplineTransform->GetInputPointSet(), surface.GetPointer());
  CHECK_BOOL(transformModified.Count > countBefore, true);
  // Fitted right away, not the first time it is used, which may be in an event handler
  CHECK_BOOL(bsplineTransform->GetNumberOfLevels() > 0, true);

  // A linear field is reproduced exactly: at the points, between them, and beyond them within the
  // margin
  std::vector<vtkVector3d> positions;
  for (vtkIdType pointIndex = 0; pointIndex < surface->GetNumberOfPoints(); pointIndex += 37)
  {
    double* p = surface->GetPoint(pointIndex);
    positions.emplace_back(p[0], p[1], p[2]);
  }
  positions.emplace_back(0.0, 0.0, 0.0);
  positions.emplace_back(20.0, -10.0, 5.0);
  positions.emplace_back(60.0, 0.0, 0.0);
  double largestError = GetLargestError(transformNode->GetTransformToParent(), positions, LinearField);
  std::cout << "  largest error of a linear field: " << largestError << std::endl;
  CHECK_BOOL(largestError < 0.01, true);

  // A node under the transform is warped by it
  vtkMRMLModelNode* warpedModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  warpedModelNode->SetAndObserveTransformNodeID(transformNode->GetID());
  vtkNew<vtkGeneralTransform> warpedToWorld;
  warpedModelNode->GetParentTransformNode()->GetTransformToWorld(warpedToWorld);
  CHECK_BOOL(GetLargestError(warpedToWorld, positions, LinearField) < 0.01, true);

  // A parameter of the transform is a change of the transform
  countBefore = transformModified.Count;
  bsplineTransform->SetDisplacementArrayName("Other");
  CHECK_BOOL(transformModified.Count > countBefore, true);
  CHECK_BOOL(GetLargestError(transformNode->GetTransformToParent(), std::vector<vtkVector3d>(positions.begin(), positions.end() - 3), SmoothField) < 0.05, true);

  // A mesh that is replaced is used at once
  vtkSmartPointer<vtkPolyData> newSurface = CreateSurfaceMesh(50.0);
  AddPointField(newSurface, "Other", LinearField);
  countBefore = transformModified.Count;
  modelNode->SetAndObserveMesh(newSurface);
  CHECK_BOOL(transformModified.Count > countBefore, true);
  CHECK_POINTER(bsplineTransform->GetInputPointSet(), newSurface.GetPointer());
  CHECK_BOOL(GetLargestError(transformNode->GetTransformToParent(), positions, LinearField) < 0.01, true);
  // So is a mesh that is changed in place, which makes the transform newer without it saying so
  countBefore = transformModified.Count;
  newSurface->GetPointData()->GetArray("Other")->Fill(0.0);
  newSurface->GetPointData()->GetArray("Other")->Modified();
  newSurface->Modified();
  CHECK_BOOL(transformModified.Count > countBefore, true);
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 20.0, -10.0, 5.0), 0.0, 1e-6);
  AddPointField(newSurface, "Other", LinearField);
  newSurface->Modified();

  // A mesh without the array: the identity, quietly
  bsplineTransform->SetDisplacementArrayName("NoSuchArray");
  CHECK_NULL(bsplineTransform->GetInputPointSet());
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 20.0, -10.0, 5.0), 0.0, 1e-9);
  bsplineTransform->SetDisplacementArrayName("Other");
  CHECK_POINTER(bsplineTransform->GetInputPointSet(), newSurface.GetPointer());

  // Saved and loaded: the model, the transform type and its parameters; the transform is fitted again from them
  // A spacing that has more digits than a stream writes by default, and grid bounds, which are saved in LPS
  bsplineTransform->SetControlPointSpacing(20.0 / 3.0, 25.0, 30.0);
  bsplineTransform->SetMaximumNumberOfLevels(5);
  bsplineTransform->SetBorderMode(VTK_BSPLINE_ZERO_AT_BORDER);
  bsplineTransform->SetDisplacementScale(1.5);
  bsplineTransform->SetBorderFalloffDistance(12.0);
  bsplineTransform->AutomaticBorderFalloffDistanceOn();
  bsplineTransform->SetGridBounds(-70.0, 60.0, -65.0, 55.0, -50.0, 80.0);
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene, tempDir + "/ModelBasedBSpline"), true);
  vtkMRMLTransformNode* reloaded = vtkMRMLTransformNode::SafeDownCast(reloadedScene->GetNodeByID(transformNode->GetID()));
  CHECK_NOT_NULL(reloaded);
  CHECK_NOT_NULL(reloaded->GetStorageNode());
  vtkScatteredBSplineTransform* reloadedTransform = vtkScatteredBSplineTransform::SafeDownCast(reloaded->GetTransformToParent());
  CHECK_NOT_NULL(reloadedTransform);
  vtkMRMLModelNode* reloadedModelNode = reloaded->GetDisplacementModelNode(reloadedTransform);
  CHECK_NOT_NULL(reloadedModelNode);
  CHECK_STRING(reloadedModelNode->GetID(), modelNode->GetID());
  CHECK_STRING(reloadedTransform->GetDisplacementArrayName(), "Other");
  CHECK_DOUBLE(reloadedTransform->GetGridBoundsMargin(), 15.0);
  CHECK_DOUBLE(reloadedTransform->GetBorderFalloffDistance(), 12.0);
  CHECK_BOOL(reloadedTransform->GetAutomaticBorderFalloffDistance(), true);
  CHECK_DOUBLE(reloadedTransform->GetControlPointSpacing()[0], 20.0 / 3.0);
  CHECK_DOUBLE(reloadedTransform->GetControlPointSpacing()[1], 25.0);
  CHECK_DOUBLE(reloadedTransform->GetMinimumControlPointSpacing(), 1.0);
  CHECK_INT(reloadedTransform->GetMaximumNumberOfLevels(), 5);
  CHECK_DOUBLE(reloadedTransform->GetTolerance(), 0.001);
  CHECK_BOOL(reloadedTransform->GetUseLinearApproximation(), true);
  CHECK_BOOL(reloadedTransform->GetAutomaticGridBounds(), true);
  for (int boundIndex = 0; boundIndex < 6; ++boundIndex)
  {
    CHECK_DOUBLE(reloadedTransform->GetGridBounds()[boundIndex], bsplineTransform->GetGridBounds()[boundIndex]);
  }
  CHECK_INT(reloadedTransform->GetBorderMode(), VTK_BSPLINE_ZERO_AT_BORDER);
  CHECK_DOUBLE(reloadedTransform->GetDisplacementScale(), 1.5);
  // Computed from the mesh of the reloaded model, so it is the same transform
  CHECK_POINTER(reloadedTransform->GetInputPointSet(), reloadedModelNode->GetMesh());
  CHECK_DOUBLE_TOLERANCE(GetMotion(reloadedTransform, 20.0, -10.0, 5.0), GetMotion(bsplineTransform, 20.0, -10.0, 5.0), 1e-6);
  reloadedTransform->SetBorderMode(VTK_BSPLINE_EDGE);
  reloadedTransform->SetDisplacementScale(1.0);
  reloadedTransform->SetMaximumNumberOfLevels(8);
  // Without falloff, so that the linear field is followed inside the sphere as well
  reloadedTransform->AutomaticBorderFalloffDistanceOff();
  reloadedTransform->SetBorderFalloffDistance(0.0);
  CHECK_BOOL(GetLargestError(reloaded->GetTransformToParent(), std::vector<vtkVector3d>(positions.begin(), positions.end() - 1), LinearField) < 0.01, true);

  // Inverted: the transform to parent is computed from the model-based transform, which is still
  // the one of the node, follows the model, and is saved in its direction
  transformNode->Inverse();
  CHECK_POINTER(transformNode->GetTransformFromParent(), bsplineTransform.GetPointer());
  CHECK_POINTER(transformNode->GetDisplacementModelNode(bsplineTransform), modelNode);
  {
    double position[3] = { 20.0, -10.0, 5.0 };
    double moved[3] = { 0.0, 0.0, 0.0 };
    double back[3] = { 0.0, 0.0, 0.0 };
    bsplineTransform->TransformPoint(position, moved);
    transformNode->GetTransformToParent()->TransformPoint(moved, back);
    CHECK_DOUBLE_TOLERANCE(Distance(position, back), 0.0, 0.01);
  }
  countBefore = transformModified.Count;
  modelNode->SetAndObserveMesh(surface);
  CHECK_BOOL(transformModified.Count > countBefore, true);
  CHECK_POINTER(bsplineTransform->GetInputPointSet(), surface.GetPointer());
  modelNode->SetAndObserveMesh(newSurface);
  {
    vtkNew<vtkMRMLScene> invertedScene;
    CHECK_BOOL(ReloadScene(scene, invertedScene, tempDir + "/ModelBasedBSplineInverted"), true);
    vtkMRMLTransformNode* invertedReloaded = vtkMRMLTransformNode::SafeDownCast(invertedScene->GetNodeByID(transformNode->GetID()));
    CHECK_NOT_NULL(invertedReloaded);
    vtkAbstractTransform* invertedReloadedTransform = invertedReloaded->GetTransformFromParentAs("vtkScatteredBSplineTransform", false, true);
    CHECK_NOT_NULL(invertedReloadedTransform);
    CHECK_NOT_NULL(invertedReloaded->GetDisplacementModelNode(invertedReloadedTransform));
    CHECK_POINTER(GetInputPointSet(invertedReloadedTransform), invertedReloaded->GetDisplacementModelNode(invertedReloadedTransform)->GetMesh());
  }
  transformNode->Inverse();
  CHECK_POINTER(transformNode->GetTransformToParent(), bsplineTransform.GetPointer());

  // Copied with the node, also which model it is computed from
  vtkMRMLTransformNode* copiedNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  copiedNode->Copy(transformNode);
  vtkScatteredBSplineTransform* copiedTransform = vtkScatteredBSplineTransform::SafeDownCast(copiedNode->GetTransformToParent());
  CHECK_NOT_NULL(copiedTransform);
  CHECK_BOOL(copiedTransform != bsplineTransform.GetPointer(), true);
  CHECK_DOUBLE(copiedTransform->GetGridBoundsMargin(), 15.0);
  CHECK_POINTER(copiedNode->GetDisplacementModelNode(copiedTransform), modelNode);

  // Without its model, it is the identity again
  countBefore = transformModified.Count;
  transformNode->SetAndObserveDisplacementModelNode(bsplineTransform, nullptr);
  CHECK_BOOL(transformModified.Count > countBefore, true);
  CHECK_NULL(bsplineTransform->GetInputPointSet());
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 20.0, -10.0, 5.0), 0.0, 1e-9);

  // Set with its model in one call: fitted right away
  vtkMRMLTransformNode* modelFirstNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  vtkMRMLModelNode* firstModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  firstModelNode->SetAndObserveMesh(surface);
  vtkNew<vtkScatteredBSplineTransform> modelFirstTransform;
  modelFirstTransform->SetDisplacementArrayName("Displacement");
  modelFirstNode->SetAndObserveModelBasedTransformToParent(modelFirstTransform, firstModelNode);
  CHECK_POINTER(modelFirstTransform->GetInputPointSet(), surface.GetPointer());
  CHECK_BOOL(modelFirstTransform->GetNumberOfLevels() > 0, true);
  // Another transform replaces it, and the model goes with it
  vtkNew<vtkScatteredBSplineTransform> replacingTransform;
  modelFirstNode->SetAndObserveTransformToParent(replacingTransform);
  CHECK_NULL(modelFirstNode->GetDisplacementModelNode(replacingTransform));

  // An ordinary transform has no displacement model
  vtkMRMLTransformNode* linearNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  vtkNew<vtkTransform> translation;
  translation->Translate(1.0, 2.0, 3.0);
  linearNode->SetAndObserveTransformToParent(translation);
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  linearNode->SetAndObserveDisplacementModelNode(translation, modelNode);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(linearNode->GetDisplacementModelNode(translation));
  CHECK_POINTER(linearNode->GetTransformToParent(), translation.GetPointer());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// A transform node with the displacements of a model used as they are
int TestMesh(const std::string& tempDir)
{
  std::cout << "Mesh" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  vtkNew<vtkMeshDisplacementTransform> meshTransform;
  meshTransform->SetDisplacementArrayName("Displacement");
  transformNode->SetAndObserveTransformToParent(meshTransform);

  // A surface: the model itself, under the transform, is moved exactly as a warp moves it
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0);
  AddPointField(surface, "Displacement", SmoothField);
  vtkMRMLModelNode* surfaceModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  surfaceModelNode->SetAndObserveMesh(surface);
  transformNode->SetAndObserveDisplacementModelNode(meshTransform, surfaceModelNode);
  CHECK_POINTER(meshTransform->GetInputPointSet(), surface.GetPointer());
  surfaceModelNode->SetAndObserveTransformNodeID(transformNode->GetID());
  vtkNew<vtkGeneralTransform> surfaceToWorld;
  surfaceModelNode->GetParentTransformNode()->GetTransformToWorld(surfaceToWorld);
  vtkNew<vtkTransformFilter> transformedSurface;
  transformedSurface->SetInputData(surface);
  transformedSurface->SetTransform(surfaceToWorld);
  transformedSurface->Update();
  surface->GetPointData()->SetActiveVectors("Displacement");
  vtkNew<vtkWarpVector> warpedSurface;
  warpedSurface->SetInputData(surface);
  warpedSurface->Update();
  vtkPointSet* transformed = vtkPointSet::SafeDownCast(transformedSurface->GetOutput());
  vtkPointSet* warped = vtkPointSet::SafeDownCast(warpedSurface->GetOutput());
  double largestDifference = 0.0;
  for (vtkIdType pointIndex = 0; pointIndex < surface->GetNumberOfPoints(); ++pointIndex)
  {
    largestDifference = std::max(largestDifference, Distance(transformed->GetPoint(pointIndex), warped->GetPoint(pointIndex)));
  }
  std::cout << "  largest difference from the warped surface: " << largestDifference << std::endl;
  CHECK_BOOL(largestDifference < 1e-4, true);
  surfaceModelNode->SetAndObserveTransformNodeID(nullptr);
  // A surface has no volume: a position that is not on it is not moved
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 10.0, -5.0, 3.0), 0.0, 1e-9);

  // A volumetric mesh: interpolated in its cells, which reproduces a linear field exactly
  vtkSmartPointer<vtkUnstructuredGrid> volume = CreateVolumetricMesh(10, 50.0);
  AddPointField(volume, "Displacement", LinearField);
  vtkMRMLModelNode* volumeModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  volumeModelNode->SetAndObserveMesh(volume);
  transformNode->SetAndObserveDisplacementModelNode(meshTransform, volumeModelNode);
  std::vector<vtkVector3d> positions = { vtkVector3d(10.0, -5.0, 3.0), vtkVector3d(-33.3, 41.2, 7.7), vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(49.0, -49.0, 12.5) };
  double largestError = GetLargestError(transformNode->GetTransformToParent(), positions, LinearField);
  std::cout << "  largest error inside the cells: " << largestError << std::endl;
  CHECK_BOOL(largestError < 1e-6, true);
  // Outside the cells: not moved
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 80.0, 0.0, 0.0), 0.0, 1e-9);
  // With a border falloff distance: moved by the displacement at the closest point of the mesh,
  // (50, 0, 0), weighted by the distance: half of it halfway to the falloff distance
  meshTransform->SetBorderFalloffDistance(60.0);
  {
    double closestPoint[3] = { 50.0, 0.0, 0.0 };
    double closestDisplacement[3] = { 0.0, 0.0, 0.0 };
    LinearField(closestPoint, closestDisplacement);
    CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 80.0, 0.0, 0.0), 0.5 * vtkMath::Norm(closestDisplacement), 1e-6);
  }
  // The inverse maps the moved positions back
  double inside[3] = { 10.0, -5.0, 3.0 };
  double moved[3] = { 0.0, 0.0, 0.0 };
  double back[3] = { 0.0, 0.0, 0.0 };
  transformNode->GetTransformToParent()->TransformPoint(inside, moved);
  transformNode->GetTransformFromParent()->TransformPoint(moved, back);
  CHECK_DOUBLE_TOLERANCE(Distance(inside, back), 0.0, 0.01);
  // Scaled
  meshTransform->SetDisplacementScale(2.0);
  {
    double displacement[3] = { 0.0, 0.0, 0.0 };
    LinearField(inside, displacement);
    transformNode->GetTransformToParent()->TransformPoint(inside, moved);
    for (int axis = 0; axis < 3; ++axis)
    {
      CHECK_DOUBLE_TOLERANCE(moved[axis], inside[axis] + 2.0 * displacement[axis], 1e-6);
    }
  }
  // A mesh that is changed in place is used at once, and everything transformed by it is told
  EventCounter transformModified;
  vtkNew<vtkCallbackCommand> callback;
  callback->SetCallback(EventCounter::Callback);
  callback->SetClientData(&transformModified);
  transformNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, callback);
  volume->GetPointData()->GetArray("Displacement")->Fill(0.0);
  volume->GetPointData()->GetArray("Displacement")->Modified();
  volume->Modified();
  CHECK_BOOL(transformModified.Count > 0, true);
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 10.0, -5.0, 3.0), 0.0, 1e-9);

  // Saved and loaded, the automatic falloff setting as well
  meshTransform->AutomaticBorderFalloffDistanceOn();
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene, tempDir + "/ModelBasedMesh"), true);
  vtkMRMLTransformNode* reloaded = vtkMRMLTransformNode::SafeDownCast(reloadedScene->GetNodeByID(transformNode->GetID()));
  CHECK_NOT_NULL(reloaded);
  vtkMeshDisplacementTransform* reloadedTransform = vtkMeshDisplacementTransform::SafeDownCast(reloaded->GetTransformToParent());
  CHECK_NOT_NULL(reloadedTransform);
  CHECK_STRING(reloadedTransform->GetDisplacementArrayName(), "Displacement");
  CHECK_DOUBLE(reloadedTransform->GetDisplacementScale(), 2.0);
  CHECK_DOUBLE(reloadedTransform->GetBorderFalloffDistance(), 60.0);
  CHECK_BOOL(reloadedTransform->GetAutomaticBorderFalloffDistance(), true);
  vtkMRMLModelNode* reloadedModelNode = reloaded->GetDisplacementModelNode(reloadedTransform);
  CHECK_NOT_NULL(reloadedModelNode);
  CHECK_STRING(reloadedModelNode->GetID(), volumeModelNode->GetID());
  CHECK_POINTER(reloadedTransform->GetInputPointSet(), reloadedModelNode->GetMesh());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Largest distance between where the transform of a node to world maps the positions and where they are expected.
double GetLargestDifference(vtkMRMLTransformNode* transformNode, const std::vector<vtkVector3d>& positions, const std::vector<vtkVector3d>& expectedPositions)
{
  vtkNew<vtkGeneralTransform> transformToWorld;
  transformNode->GetTransformToWorld(transformToWorld);
  double largestDifference = 0.0;
  for (size_t positionIndex = 0; positionIndex < positions.size(); ++positionIndex)
  {
    double moved[3] = { 0.0, 0.0, 0.0 };
    transformToWorld->TransformPoint(positions[positionIndex].GetData(), moved);
    largestDifference = std::max(largestDifference, Distance(moved, expectedPositions[positionIndex].GetData()));
  }
  return largestDifference;
}

//----------------------------------------------------------------------------
/// Check that each model-based transform of a transform node is computed from the expected model: the mesh transform
/// from the volume model, the B-spline fit from the surface model. Models are compared by ID, so that the models of a
/// reloaded scene can be checked too.
int CheckDisplacementModels(vtkMRMLTransformNode* transformNode, const char* volumeModelNodeID, const char* surfaceModelNodeID, int expectedNumberOfModelBasedTransforms)
{
  std::vector<vtkAbstractTransform*> modelBasedTransforms = GetModelBasedTransforms(transformNode->GetTransformToParent());
  CHECK_INT(static_cast<int>(modelBasedTransforms.size()), expectedNumberOfModelBasedTransforms);
  for (vtkAbstractTransform* modelBasedTransform : modelBasedTransforms)
  {
    vtkMRMLModelNode* modelNode = transformNode->GetDisplacementModelNode(modelBasedTransform);
    CHECK_NOT_NULL(modelNode);
    CHECK_STRING(modelNode->GetID(), vtkMeshDisplacementTransform::SafeDownCast(modelBasedTransform) ? volumeModelNodeID : surfaceModelNodeID);
    CHECK_POINTER(GetInputPointSet(modelBasedTransform), modelNode->GetMesh());
  }
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// Model-based transforms in a composite transform: hardened, saved, loaded and split
int TestComposite(const std::string& tempDir)
{
  std::cout << "Composite" << std::endl;
  vtkNew<vtkMRMLScene> scene;

  // Displacements in a volumetric mesh, used as they are, and at the points of a surface, fitted
  vtkSmartPointer<vtkUnstructuredGrid> volume = CreateVolumetricMesh(10, 50.0);
  AddPointField(volume, "Displacement", LinearField);
  vtkMRMLModelNode* volumeModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode", "Volume"));
  volumeModelNode->SetAndObserveMesh(volume);
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0, 20);
  AddPointField(surface, "Displacement", SmoothField);
  vtkMRMLModelNode* surfaceModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode", "Surface"));
  surfaceModelNode->SetAndObserveMesh(surface);

  // A B-spline fit, under an inverted mesh transform
  vtkMRMLTransformNode* meshNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "Mesh"));
  vtkNew<vtkMeshDisplacementTransform> meshTransform;
  meshTransform->SetDisplacementArrayName("Displacement");
  meshTransform->SetBorderFalloffDistance(30.0);
  meshNode->SetAndObserveModelBasedTransformToParent(meshTransform, volumeModelNode);
  meshNode->Inverse();
  CHECK_POINTER(meshNode->GetDisplacementModelNode(meshTransform), volumeModelNode);
  vtkMRMLTransformNode* bsplineNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "BSpline"));
  vtkNew<vtkScatteredBSplineTransform> bsplineTransform;
  bsplineTransform->SetDisplacementArrayName("Displacement");
  bsplineTransform->UseLinearApproximationOn();
  bsplineTransform->SetGridBoundsMargin(15.0);
  bsplineTransform->SetMinimumControlPointSpacing(5.0);
  bsplineTransform->SetTolerance(0.001);
  bsplineNode->SetAndObserveModelBasedTransformToParent(bsplineTransform, surfaceModelNode);
  bsplineNode->SetAndObserveTransformNodeID(meshNode->GetID());

  std::vector<vtkVector3d> positions = { vtkVector3d(10.0, -5.0, 3.0), vtkVector3d(-20.0, 15.0, 7.0), vtkVector3d(30.0, 20.0, -10.0), vtkVector3d(0.0, 0.0, 0.0) };
  std::vector<vtkVector3d> expectedPositions;
  {
    vtkNew<vtkGeneralTransform> transformToWorld;
    bsplineNode->GetTransformToWorld(transformToWorld);
    for (const vtkVector3d& position : positions)
    {
      vtkVector3d moved;
      transformToWorld->TransformPoint(position.GetData(), moved.GetData());
      expectedPositions.push_back(moved);
    }
  }

  // Hardened: one node with both model-based transforms, each computed from its own model
  CHECK_BOOL(bsplineNode->HardenTransform(), true);
  CHECK_NULL(bsplineNode->GetParentTransformNode());
  CHECK_EXIT_SUCCESS(CheckDisplacementModels(bsplineNode, volumeModelNode->GetID(), surfaceModelNode->GetID(), 2));
  CHECK_BOOL(GetLargestDifference(bsplineNode, positions, expectedPositions) < 0.01, true);

  // Copied with the node: each model-based transform with its model
  vtkMRMLTransformNode* copiedNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "Copy"));
  copiedNode->Copy(bsplineNode);
  CHECK_EXIT_SUCCESS(CheckDisplacementModels(copiedNode, volumeModelNode->GetID(), surfaceModelNode->GetID(), 2));
  CHECK_BOOL(GetLargestDifference(copiedNode, positions, expectedPositions) < 0.01, true);

  // Saved and loaded: each is computed from the mesh of its reloaded model
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene, tempDir + "/ModelBasedComposite"), true);
  vtkMRMLTransformNode* reloaded = vtkMRMLTransformNode::SafeDownCast(reloadedScene->GetNodeByID(bsplineNode->GetID()));
  CHECK_NOT_NULL(reloaded);
  CHECK_EXIT_SUCCESS(CheckDisplacementModels(reloaded, volumeModelNode->GetID(), surfaceModelNode->GetID(), 2));
  double largestDifference = GetLargestDifference(reloaded, positions, expectedPositions);
  std::cout << "  largest difference after saving and loading: " << largestDifference << std::endl;
  CHECK_BOOL(largestDifference < 0.01, true);

  // In a text file as well
  vtkNew<vtkMRMLTransformStorageNode> textStorageNode;
  reloadedScene->AddNode(textStorageNode);
  std::string textFileName = tempDir + "/ModelBasedComposite.tfm";
  textStorageNode->SetFileName(textFileName.c_str());
  CHECK_BOOL(textStorageNode->WriteData(reloaded) != 0, true);
  // The file holds the source keys of the model-based transforms, and the node that it is read into references the
  // models with them
  CHECK_BOOL(textStorageNode->ReadData(reloaded) != 0, true);
  CHECK_EXIT_SUCCESS(CheckDisplacementModels(reloaded, volumeModelNode->GetID(), surfaceModelNode->GetID(), 2));
  CHECK_BOOL(GetLargestDifference(reloaded, positions, expectedPositions) < 0.01, true);
  // Another node does not reference the models
  vtkMRMLTransformNode* readNode = vtkMRMLTransformNode::SafeDownCast(reloadedScene->AddNewNodeByClass("vtkMRMLTransformNode", "ReadText"));
  CHECK_BOOL(textStorageNode->ReadData(readNode) != 0, true);
  CHECK_INT(static_cast<int>(GetModelBasedTransforms(readNode->GetTransformToParent()).size()), 2);
  for (vtkAbstractTransform* modelBasedTransform : GetModelBasedTransforms(readNode->GetTransformToParent()))
  {
    CHECK_NULL(readNode->GetDisplacementModelNode(modelBasedTransform));
  }

  // Split: each model-based transform goes to its own node, with its model
  CHECK_BOOL(reloaded->Split() != 0, true);
  int numberOfModelBasedNodes = 0;
  for (vtkMRMLTransformNode* node = reloaded; node; node = node->GetParentTransformNode())
  {
    if (GetModelBasedTransforms(node->GetTransformToParent()).empty())
    {
      continue;
    }
    CHECK_EXIT_SUCCESS(CheckDisplacementModels(node, volumeModelNode->GetID(), surfaceModelNode->GetID(), 1));
    ++numberOfModelBasedNodes;
  }
  CHECK_INT(numberOfModelBasedNodes, 2);
  CHECK_BOOL(GetLargestDifference(reloaded, positions, expectedPositions) < 0.01, true);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkMRMLTransformNodeModelBasedTest1(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }
  const std::string tempDir = vtksys::SystemTools::CollapseFullPath(std::string(argv[1]) + "/vtkMRMLTransformNodeModelBasedTest1");
  vtksys::SystemTools::MakeDirectory(tempDir);
  CHECK_EXIT_SUCCESS(TestBSpline(tempDir));
  CHECK_EXIT_SUCCESS(TestMesh(tempDir));
  CHECK_EXIT_SUCCESS(TestComposite(tempDir));
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
