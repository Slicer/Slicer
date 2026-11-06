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
#include "vtkMeshDisplacementTransform.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVectorFieldTestingHelpers.h"

// vtkAddon includes
#include <vtkScatteredBSplineTransform.h>

// VTK includes
#include <vtkBSplineTransform.h>
#include <vtkCallbackCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkWarpVector.h>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

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
int TestBSpline()
{
  std::cout << "B-spline fit" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  // Not a scattered transform: an ordinary one, with a file of its own
  CHECK_NULL(transformNode->GetScatteredTransform());
  CHECK_BOOL(vtkMRMLTransformNode::IsScatteredTransform(transformNode->GetTransformToParent()), false);
  vtkSmartPointer<vtkMRMLStorageNode> ordinaryStorageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take(transformNode->CreateDefaultStorageNode());
  CHECK_NOT_NULL(ordinaryStorageNode);

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
  CHECK_POINTER(transformNode->GetScatteredTransform(), bsplineTransform.GetPointer());
  // Computed from the model, so not stored in a file of its own
  CHECK_NULL(transformNode->CreateDefaultStorageNode());
  // No model yet: the identity, quietly
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 10.0, -20.0, 30.0), 0.0, 1e-9);

  int countBefore = transformModified.Count;
  transformNode->SetAndObserveDisplacementModelNode(modelNode);
  CHECK_POINTER(transformNode->GetDisplacementModelNode(), modelNode);
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
  // A spacing that has more digits than a stream writes by default
  bsplineTransform->SetControlPointSpacing(20.0 / 3.0, 25.0, 30.0);
  bsplineTransform->SetMaximumNumberOfLevels(5);
  bsplineTransform->SetBorderMode(VTK_BSPLINE_ZERO_AT_BORDER);
  bsplineTransform->SetDisplacementScale(1.5);
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene), true);
  vtkMRMLTransformNode* reloaded = vtkMRMLTransformNode::SafeDownCast(reloadedScene->GetNodeByID(transformNode->GetID()));
  CHECK_NOT_NULL(reloaded);
  CHECK_NOT_NULL(reloaded->GetDisplacementModelNode());
  vtkScatteredBSplineTransform* reloadedTransform = vtkScatteredBSplineTransform::SafeDownCast(reloaded->GetScatteredTransform());
  CHECK_NOT_NULL(reloadedTransform);
  CHECK_STRING(reloadedTransform->GetDisplacementArrayName(), "Other");
  CHECK_DOUBLE(reloadedTransform->GetGridBoundsMargin(), 15.0);
  CHECK_DOUBLE(reloadedTransform->GetControlPointSpacing()[0], 20.0 / 3.0);
  CHECK_DOUBLE(reloadedTransform->GetControlPointSpacing()[1], 25.0);
  CHECK_DOUBLE(reloadedTransform->GetMinimumControlPointSpacing(), 1.0);
  CHECK_INT(reloadedTransform->GetMaximumNumberOfLevels(), 5);
  CHECK_DOUBLE(reloadedTransform->GetTolerance(), 0.001);
  CHECK_BOOL(reloadedTransform->GetUseLinearApproximation(), true);
  CHECK_BOOL(reloadedTransform->GetAutomaticGridBounds(), true);
  CHECK_INT(reloadedTransform->GetBorderMode(), VTK_BSPLINE_ZERO_AT_BORDER);
  CHECK_DOUBLE(reloadedTransform->GetDisplacementScale(), 1.5);
  CHECK_NULL(reloaded->GetStorageNode());
  // The mesh is not saved in a string, so give the reloaded model the same one
  reloaded->GetDisplacementModelNode()->SetAndObserveMesh(newSurface);
  CHECK_POINTER(reloadedTransform->GetInputPointSet(), newSurface.GetPointer());
  reloadedTransform->SetBorderMode(VTK_BSPLINE_EDGE);
  reloadedTransform->SetDisplacementScale(1.0);
  reloadedTransform->SetMaximumNumberOfLevels(8);
  CHECK_BOOL(GetLargestError(reloaded->GetTransformToParent(), std::vector<vtkVector3d>(positions.begin(), positions.end() - 1), LinearField) < 0.01, true);

  // Inverted: the transform to parent is computed from the scattered transform, which is still
  // the one of the node, follows the model, and is saved in its direction
  transformNode->Inverse();
  CHECK_POINTER(transformNode->GetScatteredTransform(), bsplineTransform.GetPointer());
  CHECK_BOOL(transformNode->IsScatteredTransformFromParent(), true);
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
  CHECK_NULL(transformNode->CreateDefaultStorageNode());
  {
    vtkNew<vtkMRMLScene> invertedScene;
    CHECK_BOOL(ReloadScene(scene, invertedScene), true);
    vtkMRMLTransformNode* invertedReloaded = vtkMRMLTransformNode::SafeDownCast(invertedScene->GetNodeByID(transformNode->GetID()));
    CHECK_NOT_NULL(invertedReloaded);
    CHECK_NOT_NULL(vtkScatteredBSplineTransform::SafeDownCast(invertedReloaded->GetScatteredTransform()));
    CHECK_BOOL(invertedReloaded->IsScatteredTransformFromParent(), true);
  }
  transformNode->Inverse();
  CHECK_BOOL(transformNode->IsScatteredTransformFromParent(), false);

  // Copied with the node
  vtkNew<vtkMRMLTransformNode> copiedNode;
  copiedNode->CopyContent(transformNode);
  vtkScatteredBSplineTransform* copiedTransform = vtkScatteredBSplineTransform::SafeDownCast(copiedNode->GetScatteredTransform());
  CHECK_NOT_NULL(copiedTransform);
  CHECK_BOOL(copiedTransform != bsplineTransform.GetPointer(), true);
  CHECK_DOUBLE(copiedTransform->GetGridBoundsMargin(), 15.0);

  // Without its model, it is the identity again
  countBefore = transformModified.Count;
  transformNode->SetAndObserveDisplacementModelNode(nullptr);
  CHECK_BOOL(transformModified.Count > countBefore, true);
  CHECK_NULL(bsplineTransform->GetInputPointSet());
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 20.0, -10.0, 5.0), 0.0, 1e-9);

  // Also fitted right away when the transform is set after the model
  vtkMRMLTransformNode* modelFirstNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  vtkMRMLModelNode* firstModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  firstModelNode->SetAndObserveMesh(surface);
  modelFirstNode->SetAndObserveDisplacementModelNode(firstModelNode);
  vtkNew<vtkScatteredBSplineTransform> modelFirstTransform;
  modelFirstTransform->SetDisplacementArrayName("Displacement");
  modelFirstNode->SetAndObserveTransformToParent(modelFirstTransform);
  CHECK_POINTER(modelFirstTransform->GetInputPointSet(), surface.GetPointer());
  CHECK_BOOL(modelFirstTransform->GetNumberOfLevels() > 0, true);

  // An ordinary transform is not changed by a displacement model
  vtkMRMLTransformNode* linearNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  vtkNew<vtkTransform> translation;
  translation->Translate(1.0, 2.0, 3.0);
  linearNode->SetAndObserveTransformToParent(translation);
  linearNode->SetAndObserveDisplacementModelNode(modelNode);
  CHECK_NULL(linearNode->GetScatteredTransform());
  CHECK_POINTER(linearNode->GetTransformToParent(), translation.GetPointer());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// A transform node with the displacements of a model used as they are
int TestMesh()
{
  std::cout << "Mesh" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  vtkNew<vtkMeshDisplacementTransform> meshTransform;
  meshTransform->SetDisplacementArrayName("Displacement");
  transformNode->SetAndObserveTransformToParent(meshTransform);
  CHECK_POINTER(transformNode->GetScatteredTransform(), meshTransform.GetPointer());
  CHECK_NULL(transformNode->CreateDefaultStorageNode());

  // A surface: the model itself, under the transform, is moved exactly as a warp moves it
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0);
  AddPointField(surface, "Displacement", SmoothField);
  vtkMRMLModelNode* surfaceModelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  surfaceModelNode->SetAndObserveMesh(surface);
  transformNode->SetAndObserveDisplacementModelNode(surfaceModelNode);
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
  transformNode->SetAndObserveDisplacementModelNode(volumeModelNode);
  std::vector<vtkVector3d> positions = { vtkVector3d(10.0, -5.0, 3.0), vtkVector3d(-33.3, 41.2, 7.7), vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(49.0, -49.0, 12.5) };
  double largestError = GetLargestError(transformNode->GetTransformToParent(), positions, LinearField);
  std::cout << "  largest error inside the cells: " << largestError << std::endl;
  CHECK_BOOL(largestError < 1e-6, true);
  // Outside the cells: not moved
  CHECK_DOUBLE_TOLERANCE(GetMotion(transformNode->GetTransformToParent(), 80.0, 0.0, 0.0), 0.0, 1e-9);
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

  // Saved and loaded
  vtkNew<vtkMRMLScene> reloadedScene;
  CHECK_BOOL(ReloadScene(scene, reloadedScene), true);
  vtkMRMLTransformNode* reloaded = vtkMRMLTransformNode::SafeDownCast(reloadedScene->GetNodeByID(transformNode->GetID()));
  CHECK_NOT_NULL(reloaded);
  vtkMeshDisplacementTransform* reloadedTransform = vtkMeshDisplacementTransform::SafeDownCast(reloaded->GetScatteredTransform());
  CHECK_NOT_NULL(reloadedTransform);
  CHECK_STRING(reloadedTransform->GetDisplacementArrayName(), "Displacement");
  CHECK_DOUBLE(reloadedTransform->GetDisplacementScale(), 2.0);
  CHECK_NOT_NULL(reloaded->GetDisplacementModelNode());
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkMRMLTransformNodeScatteredTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestBSpline());
  CHECK_EXIT_SUCCESS(TestMesh());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
