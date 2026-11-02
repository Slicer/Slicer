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

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLVectorVolumeNode.h"

// vtkAddon includes
#include <vtkScatteredBSplineTransform.h>

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkOrientedBSplineTransform.h>
#include <vtkOrientedGridTransform.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>

// ITK includes
#include <itkFactoryRegistration.h>

// STD includes
#include <iostream>
#include <string>
#include <vector>

namespace
{

//-----------------------------------------------------------------------------
/// A displacement that is linear in the position, which both a grid and a B-spline reproduce
void LinearDisplacement(const double position[3], double displacement[3])
{
  displacement[0] = 2.0 + 0.05 * position[0] - 0.02 * position[1];
  displacement[1] = -1.0 + 0.03 * position[2];
  displacement[2] = 0.5 + 0.01 * position[0] + 0.04 * position[1];
}

//-----------------------------------------------------------------------------
/// Largest distance between where the transform maps the positions and where the linear
/// displacement moves them
double GetLargestError(vtkAbstractTransform* transform, const std::vector<vtkVector3d>& positions)
{
  double largestError = 0.0;
  for (const vtkVector3d& position : positions)
  {
    double displacement[3] = { 0.0, 0.0, 0.0 };
    LinearDisplacement(position.GetData(), displacement);
    double expected[3] = { position[0] + displacement[0], position[1] + displacement[1], position[2] + displacement[2] };
    double actual[3] = { 0.0, 0.0, 0.0 };
    transform->TransformPoint(position.GetData(), actual);
    largestError = std::max(largestError, std::sqrt(vtkMath::Distance2BetweenPoints(expected, actual)));
  }
  return largestError;
}

//-----------------------------------------------------------------------------
/// A volume of 11 x 11 x 11 voxels, 2 mm apart, rotated about the S axis
vtkMRMLScalarVolumeNode* AddReferenceVolume(vtkMRMLScene* scene)
{
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode", "Reference"));
  vtkNew<vtkImageData> image;
  image->SetDimensions(11, 11, 11);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  image->GetPointData()->GetScalars()->Fill(0);
  volumeNode->SetAndObserveImageData(image);
  vtkNew<vtkTransform> ijkToRas;
  ijkToRas->Translate(-8.0, -12.0, -10.0);
  ijkToRas->RotateZ(20.0);
  ijkToRas->Scale(2.0, 2.0, 2.0);
  volumeNode->SetIJKToRASMatrix(ijkToRas->GetMatrix());
  return volumeNode;
}

//-----------------------------------------------------------------------------
/// Positions well inside the reference volume, between its voxels too
std::vector<vtkVector3d> GetPositionsInsideVolume(vtkMRMLVolumeNode* volumeNode)
{
  vtkNew<vtkMatrix4x4> ijkToRas;
  volumeNode->GetIJKToRASMatrix(ijkToRas);
  std::vector<vtkVector3d> positions;
  for (double i = 2.0; i <= 8.0; i += 1.5)
  {
    for (double j = 2.0; j <= 8.0; j += 2.5)
    {
      for (double k = 2.0; k <= 8.0; k += 3.0)
      {
        double ijk[4] = { i, j, k, 1.0 };
        double ras[4] = { 0.0, 0.0, 0.0, 1.0 };
        ijkToRas->MultiplyPoint(ijk, ras);
        positions.emplace_back(ras[0], ras[1], ras[2]);
      }
    }
  }
  return positions;
}

} // namespace

//-----------------------------------------------------------------------------
int vtkSlicerTransformLogicImportTest1(int argc, char* argv[])
{
  itk::itkFactoryRegistration();
  if (argc < 2)
  {
    std::cerr << "Usage: vtkSlicerTransformLogicImportTest1 /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }
  std::string tempDir = argv[1];

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerTransformLogic> logic;
  logic->SetMRMLScene(scene);

  // A transform with a linear displacement, defined as a grid on the reference volume
  vtkMRMLScalarVolumeNode* referenceVolumeNode = AddReferenceVolume(scene);
  std::vector<vtkVector3d> positions = GetPositionsInsideVolume(referenceVolumeNode);
  vtkMRMLVectorVolumeNode* sourceVolumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode", "Displacement"));
  {
    vtkNew<vtkImageData> displacementImage;
    displacementImage->SetDimensions(11, 11, 11);
    displacementImage->AllocateScalars(VTK_FLOAT, 3);
    vtkNew<vtkMatrix4x4> ijkToRas;
    referenceVolumeNode->GetIJKToRASMatrix(ijkToRas);
    for (int k = 0; k < 11; ++k)
    {
      for (int j = 0; j < 11; ++j)
      {
        for (int i = 0; i < 11; ++i)
        {
          double ijk[4] = { double(i), double(j), double(k), 1.0 };
          double ras[4] = { 0.0, 0.0, 0.0, 1.0 };
          ijkToRas->MultiplyPoint(ijk, ras);
          double displacement[3] = { 0.0, 0.0, 0.0 };
          LinearDisplacement(ras, displacement);
          for (int component = 0; component < 3; ++component)
          {
            displacementImage->SetScalarComponentFromDouble(i, j, k, component, displacement[component]);
          }
        }
      }
    }
    sourceVolumeNode->SetAndObserveImageData(displacementImage);
    sourceVolumeNode->SetIJKToRASMatrix(ijkToRas);
  }

  // Import from a vector volume: a grid transform, as transform to parent
  vtkMRMLTransformNode* importedFromVolume = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "FromVolume"));
  CHECK_BOOL(logic->ImportTransformFromDisplacementVolume(importedFromVolume, sourceVolumeNode), true);
  CHECK_NOT_NULL(vtkOrientedGridTransform::SafeDownCast(importedFromVolume->GetTransformToParent()));
  double largestError = GetLargestError(importedFromVolume->GetTransformToParent(), positions);
  std::cout << "Largest error of the transform imported from a volume: " << largestError << std::endl;
  CHECK_BOOL(largestError < 1e-4, true);
  // The voxels are copied: changing the volume afterwards does not change the transform
  sourceVolumeNode->GetImageData()->GetPointData()->GetScalars()->Fill(0.0);
  sourceVolumeNode->GetImageData()->Modified();
  CHECK_BOOL(GetLargestError(importedFromVolume->GetTransformToParent(), positions) < 1e-4, true);

  // Exporting it on the same geometry gives the volume back
  vtkMRMLVolumeNode* exportedVolumeNode = logic->CreateDisplacementVolumeFromTransform(importedFromVolume, referenceVolumeNode, false /*magnitude*/);
  CHECK_NOT_NULL(exportedVolumeNode);
  {
    vtkImageData* exportedImage = exportedVolumeNode->GetImageData();
    double ijk5[3] = { 5.0, 5.0, 5.0 };
    vtkNew<vtkMatrix4x4> ijkToRas;
    referenceVolumeNode->GetIJKToRASMatrix(ijkToRas);
    double ijk[4] = { ijk5[0], ijk5[1], ijk5[2], 1.0 };
    double ras[4] = { 0.0, 0.0, 0.0, 1.0 };
    ijkToRas->MultiplyPoint(ijk, ras);
    double expected[3] = { 0.0, 0.0, 0.0 };
    LinearDisplacement(ras, expected);
    for (int component = 0; component < 3; ++component)
    {
      CHECK_DOUBLE_TOLERANCE(exportedImage->GetScalarComponentAsDouble(5, 5, 5, component), expected[component], 1e-4);
    }
  }

  // A volume under a linear transform is placed where it is shown
  {
    vtkMRMLVectorVolumeNode* movedVolumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(exportedVolumeNode);
    CHECK_NOT_NULL(movedVolumeNode);
    vtkMRMLTransformNode* translation = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "Translation"));
    vtkNew<vtkMatrix4x4> translationMatrix;
    translationMatrix->SetElement(0, 3, 30.0);
    translation->SetMatrixTransformToParent(translationMatrix);
    movedVolumeNode->SetAndObserveTransformNodeID(translation->GetID());
    vtkMRMLTransformNode* importedFromMovedVolume = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "FromMovedVolume"));
    CHECK_BOOL(logic->ImportTransformFromDisplacementVolume(importedFromMovedVolume, movedVolumeNode), true);
    vtkOrientedGridTransform* movedGrid = vtkOrientedGridTransform::SafeDownCast(importedFromMovedVolume->GetTransformToParent());
    CHECK_NOT_NULL(movedGrid);
    vtkOrientedGridTransform* grid = vtkOrientedGridTransform::SafeDownCast(importedFromVolume->GetTransformToParent());
    CHECK_DOUBLE_TOLERANCE(movedGrid->GetDisplacementGrid()->GetOrigin()[0], grid->GetDisplacementGrid()->GetOrigin()[0] + 30.0, 1e-6);

    // Under a rotation, the displacements are rotated with the volume: a point p of the volume
    // displaced by d is shown at R*p, displaced by R*d
    vtkMRMLTransformNode* rotation = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "Rotation"));
    vtkNew<vtkTransform> rotationTransform;
    rotationTransform->RotateZ(90.0);
    rotation->SetMatrixTransformToParent(rotationTransform->GetMatrix());
    movedVolumeNode->SetAndObserveTransformNodeID(rotation->GetID());
    vtkMRMLTransformNode* importedFromRotatedVolume = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "FromRotatedVolume"));
    CHECK_BOOL(logic->ImportTransformFromDisplacementVolume(importedFromRotatedVolume, movedVolumeNode), true);
    vtkNew<vtkMatrix4x4> ijkToRas;
    movedVolumeNode->GetIJKToRASMatrix(ijkToRas);
    for (const vtkVector3d& ijk : { vtkVector3d(5.0, 5.0, 5.0), vtkVector3d(3.0, 6.0, 4.0) })
    {
      double position_IJK[4] = { ijk[0], ijk[1], ijk[2], 1.0 };
      double position_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
      ijkToRas->MultiplyPoint(position_IJK, position_RAS);
      double displacement_RAS[3] = { 0.0, 0.0, 0.0 };
      LinearDisplacement(position_RAS, displacement_RAS);
      double position_World[3] = { 0.0, 0.0, 0.0 };
      rotationTransform->TransformPoint(position_RAS, position_World);
      double expectedDisplacement_World[3] = { 0.0, 0.0, 0.0 };
      rotationTransform->TransformVector(displacement_RAS, expectedDisplacement_World);
      double transformed_World[3] = { 0.0, 0.0, 0.0 };
      importedFromRotatedVolume->GetTransformToParent()->TransformPoint(position_World, transformed_World);
      for (int axis = 0; axis < 3; ++axis)
      {
        CHECK_DOUBLE_TOLERANCE(transformed_World[axis] - position_World[axis], expectedDisplacement_World[axis], 1e-4);
      }
    }
  }

  // Only a vector volume holds displacements
  {
    vtkNew<vtkMRMLVectorVolumeNode> scalarVolumeNode;
    scene->AddNode(scalarVolumeNode);
    scalarVolumeNode->SetAndObserveImageData(referenceVolumeNode->GetImageData());
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    CHECK_BOOL(logic->ImportTransformFromDisplacementVolume(importedFromVolume, scalarVolumeNode), false);
    TESTING_OUTPUT_ASSERT_ERRORS_END();
  }

  // Import from a model: displacements at the points of a sphere, fitted by a B-spline
  vtkNew<vtkSphereSource> sphere;
  sphere->SetRadius(20.0);
  sphere->SetThetaResolution(30);
  sphere->SetPhiResolution(30);
  sphere->Update();
  vtkNew<vtkPolyData> mesh;
  mesh->DeepCopy(sphere->GetOutput());
  vtkNew<vtkDoubleArray> displacements;
  displacements->SetName("Displacement");
  displacements->SetNumberOfComponents(3);
  displacements->SetNumberOfTuples(mesh->GetNumberOfPoints());
  std::vector<vtkVector3d> meshPositions;
  for (vtkIdType pointIndex = 0; pointIndex < mesh->GetNumberOfPoints(); ++pointIndex)
  {
    double* position = mesh->GetPoint(pointIndex);
    double displacement[3] = { 0.0, 0.0, 0.0 };
    LinearDisplacement(position, displacement);
    displacements->SetTypedTuple(pointIndex, displacement);
    if (pointIndex % 23 == 0)
    {
      meshPositions.emplace_back(position[0], position[1], position[2]);
    }
  }
  mesh->GetPointData()->AddArray(displacements);
  meshPositions.emplace_back(0.0, 0.0, 0.0);
  meshPositions.emplace_back(5.0, -8.0, 3.0);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode", "Solid"));
  modelNode->SetAndObserveMesh(mesh);

  // A copy of the fit, as a B-spline transform
  vtkMRMLTransformNode* importedFromModel = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "FromModel"));
  CHECK_BOOL(logic->ImportTransformFromModel(importedFromModel, modelNode, "Displacement", false /*keepLinked*/), true);
  CHECK_NULL(importedFromModel->GetScatteredTransform());
  CHECK_NULL(importedFromModel->GetDisplacementModelNode());
  vtkAbstractTransform* bsplineTransform = importedFromModel->GetTransformToParent();
  CHECK_NOT_NULL(bsplineTransform);
  CHECK_STD_STRING(std::string(bsplineTransform->GetClassName()), "vtkOrientedBSplineTransform");
  largestError = GetLargestError(bsplineTransform, meshPositions);
  std::cout << "Largest error of the transform imported from a model: " << largestError << std::endl;
  CHECK_BOOL(largestError < 0.01, true);
  // A copy: a new mesh does not change it
  {
    vtkNew<vtkPolyData> otherMesh;
    otherMesh->DeepCopy(mesh);
    otherMesh->GetPointData()->GetArray("Displacement")->Fill(0.0);
    modelNode->SetAndObserveMesh(otherMesh);
    CHECK_BOOL(GetLargestError(importedFromModel->GetTransformToParent(), meshPositions) < 0.01, true);
    modelNode->SetAndObserveMesh(mesh);
  }
  // It can be saved and loaded
  {
    vtkNew<vtkMRMLTransformStorageNode> storageNode;
    scene->AddNode(storageNode);
    std::string fileName = tempDir + "/vtkSlicerTransformLogicImportTest1_bspline.h5";
    storageNode->SetFileName(fileName.c_str());
    CHECK_BOOL(storageNode->WriteData(importedFromModel) != 0, true);
    vtkMRMLTransformNode* readNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "Read"));
    CHECK_BOOL(storageNode->ReadData(readNode) != 0, true);
    largestError = GetLargestError(readNode->GetTransformToParent(), meshPositions);
    std::cout << "Largest error of the saved and loaded B-spline transform: " << largestError << std::endl;
    CHECK_BOOL(largestError < 0.01, true);
  }

  // By default the transform stays linked to the model: a scattered transform, which follows it
  vtkMRMLTransformNode* linkedNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode", "Linked"));
  linkedNode->AddDefaultStorageNode();
  CHECK_NOT_NULL(linkedNode->GetStorageNode());
  CHECK_BOOL(logic->ImportTransformFromModel(linkedNode, modelNode, "Displacement"), true);
  vtkScatteredBSplineTransform* scatteredTransform = vtkScatteredBSplineTransform::SafeDownCast(linkedNode->GetScatteredTransform());
  CHECK_NOT_NULL(scatteredTransform);
  CHECK_POINTER(linkedNode->GetDisplacementModelNode(), modelNode);
  CHECK_STRING(scatteredTransform->GetDisplacementArrayName(), "Displacement");
  CHECK_POINTER(scatteredTransform->GetInputPointSet(), static_cast<vtkPointSet*>(mesh.GetPointer()));
  // It is saved with the scene, not in a file of its own
  CHECK_NULL(linkedNode->GetStorageNode());
  // The defaults: a tenth of the diagonal beyond the points, following their linear trend
  double meshBounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  mesh->GetBounds(meshBounds);
  double diagonal = std::sqrt((meshBounds[1] - meshBounds[0]) * (meshBounds[1] - meshBounds[0]) + (meshBounds[3] - meshBounds[2]) * (meshBounds[3] - meshBounds[2])
                              + (meshBounds[5] - meshBounds[4]) * (meshBounds[5] - meshBounds[4]));
  CHECK_DOUBLE_TOLERANCE(scatteredTransform->GetGridBoundsMargin(), 0.1 * diagonal, 1e-9);
  CHECK_BOOL(scatteredTransform->GetUseLinearApproximation(), true);
  CHECK_BOOL(scatteredTransform->GetAutomaticGridBounds(), true);
  CHECK_BOOL(GetLargestError(linkedNode->GetTransformToParent(), meshPositions) < 0.01, true);
  // A new mesh is followed
  {
    vtkNew<vtkPolyData> otherMesh;
    otherMesh->DeepCopy(mesh);
    otherMesh->GetPointData()->GetArray("Displacement")->Fill(0.0);
    modelNode->SetAndObserveMesh(otherMesh);
    double origin[3] = { 0.0, 0.0, 0.0 };
    double moved[3] = { 0.0, 0.0, 0.0 };
    linkedNode->GetTransformToParent()->TransformPoint(origin, moved);
    CHECK_DOUBLE_TOLERANCE(std::sqrt(vtkMath::Distance2BetweenPoints(origin, moved)), 0.0, 1e-6);
    modelNode->SetAndObserveMesh(mesh);
  }
  // Importing a volume into it makes it an ordinary transform again
  CHECK_BOOL(logic->ImportTransformFromDisplacementVolume(linkedNode, vtkMRMLVectorVolumeNode::SafeDownCast(exportedVolumeNode)), true);
  CHECK_NULL(linkedNode->GetScatteredTransform());
  CHECK_NULL(linkedNode->GetDisplacementModelNode());

  // The array has to be there, with 3 components
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(logic->ImportTransformFromModel(importedFromModel, modelNode, "NoSuchArray"), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_BOOL(GetLargestError(importedFromModel->GetTransformToParent(), meshPositions) < 0.01, true);

  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
