/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRML includes
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkOrientedBSplineTransform.h>
#include <vtkTransform.h>

// STL includes
#include <sstream>

static const double TOLERANCE = 1e-4;
static const double DISPLACEMENT_SCALE = 0.63;

//----------------------------------------------------------------------------
void CreateBSplineVtk2(vtkOrientedBSplineTransform* bsplineTransform,
  double origin[3], double spacing[3], double direction[3][3], double dims[3],
  const double bulkMatrix[3][3], const double bulkOffset[3])
{
  vtkNew<vtkImageData> bsplineCoefficients;
  bsplineCoefficients->SetExtent(0, dims[0] - 1, 0, dims[1] - 1, 0, dims[2] - 1);
  bsplineCoefficients->SetOrigin(origin);
  bsplineCoefficients->SetSpacing(spacing);
  bsplineCoefficients->AllocateScalars(VTK_DOUBLE, 3);
  bsplineCoefficients->GetPointData()->GetScalars()->Fill(0);

  vtkNew<vtkMatrix4x4> bulkTransform;
  vtkNew<vtkMatrix4x4> gridOrientation;
  for (int row = 0; row < 3; row++)
    {
    for (int col = 0; col < 3; col++)
      {
      bulkTransform->SetElement(row, col, bulkMatrix[row][col]);
      gridOrientation->SetElement(row, col, direction[row][col]);
      }
    bulkTransform->SetElement(row, 3, bulkOffset[row]);
    }

  bsplineTransform->SetGridDirectionMatrix(gridOrientation.GetPointer());
  bsplineTransform->SetCoefficientData(bsplineCoefficients.GetPointer());
  bsplineTransform->SetBulkTransformMatrix(bulkTransform.GetPointer());

  bsplineTransform->SetBorderModeToZero();
  bsplineTransform->SetDisplacementScale(DISPLACEMENT_SCALE);
}

//----------------------------------------------------------------------------
struct PointOrientation
{
  PointOrientation() = default;
  PointOrientation(const PointOrientation& po)
  {
    this->Position = po.Position;
    this->Orientation->DeepCopy(po.Orientation);
  };
  vtkVector3d Position;
  vtkNew<vtkMatrix3x3> Orientation;
};

//----------------------------------------------------------------------------
int TestMarkupOrientation(std::vector<PointOrientation> originalOrientations, vtkMRMLMarkupsFiducialNode* markupsNode)
{
  markupsNode->RemoveAllControlPoints();

  for (auto pointOrientation : originalOrientations)
    {
    int index = markupsNode->AddControlPoint(pointOrientation.Position);
    double* matrix = pointOrientation.Orientation->GetData();
    markupsNode->SetNthControlPointOrientationMatrix(index, matrix);

    // ------------------
    // Check that the data in the orientation matrix returned as a vtkMatrix3x3 matches the expected data.
    vtkNew<vtkMatrix3x3> currentControlPointOrientation;
    markupsNode->GetNthControlPointOrientationMatrix(index, currentControlPointOrientation);
    for (int i = 0; i < 9; ++i)
      {
      CHECK_DOUBLE_TOLERANCE(currentControlPointOrientation->GetData()[i], pointOrientation.Orientation->GetData()[i], TOLERANCE);
      }

    // ------------------
    // Check that the matrix orientation in world coordinates matches the expected values
    vtkNew<vtkGeneralTransform> transfromFromNodeToWorld;
    vtkMRMLTransformNode::GetTransformBetweenNodes(markupsNode->GetParentTransformNode(), nullptr, transfromFromNodeToWorld);

    double xAxisActual_Node[3] = { 1.0, 0.0, 0.0 };
    pointOrientation.Orientation->MultiplyPoint(xAxisActual_Node, xAxisActual_Node);

    double yAxisActual_Node[3] = { 0.0, 1.0, 0.0 };
    pointOrientation.Orientation->MultiplyPoint(yAxisActual_Node, yAxisActual_Node);

    double zAxisActual_Node[3] = { 0.0, 0.0, 1.0 };
    pointOrientation.Orientation->MultiplyPoint(zAxisActual_Node, zAxisActual_Node);

    double xAxisActual_World[3] = { 1.0, 0.0, 0.0 };
    transfromFromNodeToWorld->TransformVectorAtPoint(pointOrientation.Position.GetData(), xAxisActual_Node, xAxisActual_World);

    double yAxisActual_World[3] = { 0.0, 1.0, 0.0 };
    transfromFromNodeToWorld->TransformVectorAtPoint(pointOrientation.Position.GetData(), yAxisActual_Node, yAxisActual_World);

    double zAxisActual_World[3] = { 0.0, 0.0, 1.0 };
    transfromFromNodeToWorld->TransformVectorAtPoint(pointOrientation.Position.GetData(), zAxisActual_Node, zAxisActual_World);

    vtkNew<vtkMatrix3x3> controlPointOrientation_World;
    markupsNode->GetNthControlPointOrientationMatrixWorld(index, controlPointOrientation_World->GetData());

    double normal_World[3] = { 0.0 };
    markupsNode->GetNthControlPointNormalWorld(index, normal_World);

    double normalActual_World[3] = { 0.0 };
    std::copy_n(zAxisActual_World, 3, normalActual_World);

    for (int i = 0; i < 3; ++i)
      {
      CHECK_DOUBLE_TOLERANCE(controlPointOrientation_World->GetElement(i, 0), xAxisActual_World[i], TOLERANCE);
      CHECK_DOUBLE_TOLERANCE(controlPointOrientation_World->GetElement(i, 1), yAxisActual_World[i], TOLERANCE);
      CHECK_DOUBLE_TOLERANCE(controlPointOrientation_World->GetElement(i, 2), zAxisActual_World[i], TOLERANCE);
      CHECK_DOUBLE_TOLERANCE(normal_World[i], normalActual_World[i], TOLERANCE);
      }
    }

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsNodeTest6(int, char* [])
{

  std::cout << "Testing vtkMarkupsNode orientation transformation" << std::endl;
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLMarkupsFiducialNode> markupsNode;
  scene->AddNode(markupsNode);

  std::vector<PointOrientation> pointOrientation;

  PointOrientation po0;
  po0.Position = vtkVector3d(0.0, 0.0, 0.0);
  pointOrientation.push_back(po0);

  PointOrientation po1;
  po1.Position = vtkVector3d(10.0, 20.0, 30.0);
  double po1Orientation[9] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };
  po1.Orientation->DeepCopy(po1Orientation);
  pointOrientation.push_back(po1);

  PointOrientation po2;
  po2.Position = vtkVector3d(-1.23, -9.87, -51.8);
  double po2Orientation[9] = { 0.0, -1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0 };
  po2.Orientation->DeepCopy(po2Orientation);
  pointOrientation.push_back(po2);

  PointOrientation po3;
  po3.Position = vtkVector3d(96.0, 93.0, -35.0);
  double po3Orientation[9] = { -0.216468, 0.0449462, 0.97525, -0.323356, - 0.945858, - 0.0281809, 0.921185, -0.321455, 0.219281};
  po3.Orientation->DeepCopy(po3Orientation);
  pointOrientation.push_back(po3);

  PointOrientation po4;
  po4.Position = vtkVector3d(-96.2611, 24.1876, 60.1228);
  double po4Orientation[9] = { 0.157376, 0.946914, 0.280335, 0.986736, -0.139335, -0.0832936, -0.0398113, 0.289725, -0.956282 };
  po4.Orientation->DeepCopy(po4Orientation);
  pointOrientation.push_back(po4);

  PointOrientation po5;
  po5.Position = vtkVector3d(113.133, -23.2094, 11.3227);
  double po5Orientation[9] = { -0.167427, -0.635573, -0.753667, -0.826077, 0.507694, -0.244629, 0.538113, 0.581629, -0.610034 };
  po5.Orientation->DeepCopy(po5Orientation);
  pointOrientation.push_back(po5);

  std::cout << "--------------------------------------------" << std::endl;
  std::cout << "Testing orientation with no parent transform" << std::endl;
  CHECK_EXIT_SUCCESS(TestMarkupOrientation(pointOrientation, markupsNode));
  std::cout << "Success." << std::endl;

  std::cout << "--------------------------------------------" << std::endl;
  std::cout << "Testing orientation with linear transform" << std::endl;

  vtkNew<vtkMRMLTransformNode> linearTransformNode;
  scene->AddNode(linearTransformNode);

  vtkNew<vtkTransform> linearTransform;
  linearTransform->RotateX(15);
  linearTransform->RotateY(31);
  linearTransform->RotateZ(26);
  linearTransform->Translate(5.0, 89.0, 124.0);
  linearTransform->Scale(0.5, 1.0, 1.5);
  linearTransformNode->SetMatrixTransformToParent(linearTransform->GetMatrix());
  markupsNode->SetAndObserveTransformNodeID(linearTransformNode->GetID());

  CHECK_EXIT_SUCCESS(TestMarkupOrientation(pointOrientation, markupsNode));
  std::cout << "Success." << std::endl;

  std::cout << "--------------------------------------------" << std::endl;
  std::cout << "Testing orientation with b-spline transform" << std::endl;

  vtkNew<vtkOrientedBSplineTransform> bSplineTransform;
  double bSplineOrigin[3] = { -100, -100, -100 };
  double bSplineSpacing[3] = { 100, 100, 100 };
  double bSplineDirection[3][3] = { {0.92128500, -0.36017075, -0.146666625}, {0.31722386, 0.91417248, -0.25230478}, {0.22495105, 0.18591857, 0.95646814} };
  double bSplineDims[3] = { 7,8,7 };
  const double bSplineBulkMatrix[3][3] = { { 0.7, 0.2, 0.1 }, { 0.1, 0.8, 0.1 }, { 0.05, 0.2, 0.9 } };
  const double bSplineBulkOffset[3] = { -5, 3, 6 };
  CreateBSplineVtk2(bSplineTransform, bSplineOrigin, bSplineSpacing, bSplineDirection, bSplineDims, bSplineBulkMatrix, bSplineBulkOffset);

  vtkNew<vtkMRMLTransformNode> bSplineTransformNode;
  scene->AddNode(bSplineTransformNode);
  bSplineTransformNode->SetAndObserveTransformToParent(bSplineTransform);
  markupsNode->SetAndObserveTransformNodeID(bSplineTransformNode->GetID());

  CHECK_EXIT_SUCCESS(TestMarkupOrientation(pointOrientation, markupsNode));
  std::cout << "Success." << std::endl;

  return EXIT_SUCCESS;
}
