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
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkIndent.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkOrientedBSplineTransform.h>
#include <vtkPointData.h>
#include <vtkRegularPolygonSource.h>
#include <vtkTestingOutputWindow.h>
#include <vtkTransform.h>

// STL includes
#include <sstream>

static const double EPSILON = 1e-4;

//----------------------------------------------------------------------------
bool CompareROI(double xAxisExpected_World[3], double yAxisExpected_World[3], double zAxisExpected_World[3],
  double originExpected_World[3], double sizeExpected_World[3], vtkMRMLMarkupsROINode* roiNode, double epsilon)
{
  double xAxisActual_World[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetXAxisWorld(xAxisActual_World);

  double yAxisActual_World[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetYAxisWorld(yAxisActual_World);

  double zAxisActual_World[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetZAxisWorld(zAxisActual_World);

  if (vtkMath::Dot(xAxisExpected_World, xAxisActual_World) < 1.0 - epsilon)
    {
    std::cerr << "X-axis: expected: ["
      << xAxisExpected_World[0] << ", "
      << xAxisExpected_World[1] << ", "
      << xAxisExpected_World[2] << ", "
      << "] got: ["
      << xAxisActual_World[0] << ", "
      << xAxisActual_World[1] << ", "
      << xAxisActual_World[2] << ", "
      << "]" << std::endl;
    return false;
    }
  if (vtkMath::Dot(yAxisExpected_World, yAxisActual_World) < 1.0 - epsilon)
    {
    std::cerr << "Y-axis: expected: ["
      << yAxisExpected_World[0] << ", "
      << yAxisExpected_World[1] << ", "
      << yAxisExpected_World[2] << ", "
      << "] got: ["
      << yAxisActual_World[0] << ", "
      << yAxisActual_World[1] << ", "
      << yAxisActual_World[2] << ", "
      << "]" << std::endl;
    return false;
    }
  if (vtkMath::Dot(zAxisExpected_World, zAxisActual_World) < 1.0 - epsilon)
    {
    std::cerr << "Z-axis: expected: ["
      << zAxisExpected_World[0] << ", "
      << zAxisExpected_World[1] << ", "
      << zAxisExpected_World[2]
      << "] got: ["
      << zAxisActual_World[0] << ", "
      << zAxisActual_World[1] << ", "
      << zAxisActual_World[2]
      << "]" << std::endl;
    return false;
    }

  double originActual_World[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetCenterWorld(originActual_World);

  double originDifference_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Subtract(originExpected_World, originActual_World, originDifference_World);
  if (vtkMath::Norm(originDifference_World) > epsilon)
    {
    std::cerr << "Center: expected: ["
      << originExpected_World[0] << ", "
      << originExpected_World[1] << ", "
      << originExpected_World[2]
      << "] got: ["
      << originActual_World[0] << ", "
      << originActual_World[1] << ", "
      << originActual_World[2]
      << "]" << std::endl;
    return false;
    }

  double sizeActual_World[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSizeWorld(sizeActual_World);

  double sizeDifference_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Subtract(sizeExpected_World, sizeActual_World, sizeDifference_World);
  if (std::abs(sizeDifference_World[0]) > EPSILON
    || std::abs(sizeDifference_World[1]) > EPSILON
    || std::abs(sizeDifference_World[2]) > EPSILON
    )
  {
    std::cerr << "Size: expected: ["
      << sizeExpected_World[0] << ", "
      << sizeExpected_World[1] << ", "
      << sizeExpected_World[2]
      << "] got: ["
      << sizeActual_World[0] << ", "
      << sizeActual_World[1] << ", "
      << sizeActual_World[2]
      << "]" << std::endl;
    return false;
  }

  return true;
}

double DisplacementScale = 0.63;

//----------------------------------------------------------------------------
void CreateBSplineVtk(vtkOrientedBSplineTransform* bsplineTransform,
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
  bsplineTransform->SetDisplacementScale(DisplacementScale);
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsNodeTest5(int , char * [])
{
  std::cout << "Testing vtkMRMLMarkupsROINode" << std::endl;
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLMarkupsROINode> roiNode;
  scene->AddNode(roiNode);

  vtkNew<vtkTransform> roiToLocal;
  roiToLocal->RotateX(55);
  roiToLocal->RotateY(12);
  roiToLocal->RotateZ(32);
  roiToLocal->Translate(50, 150, 200);

  double origin[3] = { 0.0, 0.0, 0.0 };
  double xAxis[3] = { 1.0, 0.0, 0.0 };
  double yAxis[3] = { 0.0, 1.0, 0.0 };
  double zAxis[3] = { 0.0, 0.0, 1.0 };

  double xAxis_Node[3] = { 1.0, 0.0, 0.0 };
  double yAxis_Node[3] = { 0.0, 1.0, 0.0 };
  double zAxis_Node[3] = { 0.0, 0.0, 1.0 };
  double origin_Node[3] = { 0.0, 0.0, 0.0 };

  double size_World[3] = { 5.0, 5.0, 5.0 };

  roiToLocal->TransformVectorAtPoint(origin, xAxis, xAxis_Node);
  roiToLocal->TransformVectorAtPoint(origin, yAxis, yAxis_Node);
  roiToLocal->TransformVectorAtPoint(origin, zAxis, zAxis_Node);
  roiToLocal->TransformPoint(origin, origin_Node);

  /////////////
  std::cout << "Test set axes/origin" << std::endl;
  roiNode->GetObjectToNodeMatrix()->DeepCopy(roiToLocal->GetMatrix());
  roiNode->SetSizeWorld(size_World);

  CHECK_BOOL(CompareROI(xAxis_Node, yAxis_Node, zAxis_Node, origin_Node, size_World, roiNode, EPSILON), true);

  ///////////////
  std::cout << "Test set axes/origin and linear transform node" << std::endl;

  vtkNew<vtkMRMLLinearTransformNode> transformNode;
  scene->AddNode(transformNode);
  roiNode->SetAndObserveTransformNodeID(transformNode->GetID());

  vtkNew<vtkTransform> linearTransform;
  linearTransform->Translate(30.0, 60.0, 90.0);
  linearTransform->RotateZ(30.0);
  linearTransform->RotateY(60.0);
  linearTransform->RotateX(90.0);
  linearTransform->Translate(90.0, 60.0, 30.0);
  linearTransform->Scale(5.0, 12.0, 1.0);
  transformNode->SetMatrixTransformToParent(linearTransform->GetMatrix());

  double xAxis_World[3] = { 1.0, 0.0, 0.0 };
  double yAxis_World[3] = { 0.0, 1.0, 0.0 };
  double zAxis_World[3] = { 0.0, 0.0, 1.0 };
  double origin_World[3] = { 0.0, 0.0, 0.0 };

  vtkNew<vtkMatrix4x4> matrix;
  vtkMRMLMarkupsROINode::GenerateOrthogonalMatrix(xAxis_Node, yAxis_Node, zAxis_Node, origin_Node, matrix, linearTransform);
  for (int i = 0; i < 3; ++i)
    {
    xAxis_World[i] = matrix->GetElement(i, 0);
    yAxis_World[i] = matrix->GetElement(i, 1);
    zAxis_World[i] = matrix->GetElement(i, 2);
    origin_World[i] = matrix->GetElement(i, 3);
    }
  roiNode->SetSizeWorld(size_World);

  CHECK_BOOL(CompareROI(xAxis_World, yAxis_World, zAxis_World, origin_World, size_World, roiNode, EPSILON), true);

  ///////////////
  std::cout << "Test harden linear transform" << std::endl;
  roiNode->HardenTransform();
  CHECK_BOOL(CompareROI(xAxis_World, yAxis_World, zAxis_World, origin_World, size_World, roiNode, EPSILON), true);

  ///////////////
  std::cout << "Test b-spline transform" << std::endl;

  vtkNew<vtkOrientedBSplineTransform> bSplineTransform;
  double bSplineSpacing[3] = { 100, 100, 100 };
  double bSplineDirection[3][3] = { {0.92128500, -0.36017075, -0.146666625}, {0.31722386, 0.91417248, -0.25230478}, {0.22495105, 0.18591857, 0.95646814} };
  double bSplineDims[3] = { 7,8,7 };
  const double bSplineBulkMatrix[3][3] = { { 0.7, 0.2, 0.1 }, { 0.1, 0.8, 0.1 }, { 0.05, 0.2, 0.9 } };
  const double bSplineBulkOffset[3] = { -5, 3, 6 };
  CreateBSplineVtk(bSplineTransform, origin, bSplineSpacing, bSplineDirection, bSplineDims, bSplineBulkMatrix, bSplineBulkOffset);

  vtkNew<vtkMRMLBSplineTransformNode> bSplineTransformNode;
  scene->AddNode(bSplineTransformNode);
  bSplineTransformNode->SetAndObserveTransformToParent(bSplineTransform);
  roiNode->GetObjectToNodeMatrix()->Identity();
  roiNode->SetCenter(0.0, 0.0, 0.0);
  roiNode->SetAndObserveTransformNodeID(bSplineTransformNode->GetID());
  roiNode->SetSizeWorld(size_World);

  vtkMRMLMarkupsROINode::GenerateOrthogonalMatrix(xAxis, yAxis, zAxis, origin, matrix, bSplineTransform, false);
  for (int i = 0; i < 3; ++i)
    {
    xAxis_World[i] = matrix->GetElement(i, 0);
    yAxis_World[i] = matrix->GetElement(i, 1);
    zAxis_World[i] = matrix->GetElement(i, 2);
    origin_World[i] = matrix->GetElement(i, 3);
    }

  CHECK_BOOL(CompareROI(xAxis_World, yAxis_World, zAxis_World, origin_World, size_World, roiNode, EPSILON), true);

  ///////////////
  std::cout << "Test harden b-spline transform" << std::endl;
  roiNode->HardenTransform();

  CHECK_BOOL(CompareROI(xAxis_World, yAxis_World, zAxis_World, origin_World, size_World, roiNode, EPSILON), true);

  std::cout << "Success." << std::endl;

  return EXIT_SUCCESS;
}
