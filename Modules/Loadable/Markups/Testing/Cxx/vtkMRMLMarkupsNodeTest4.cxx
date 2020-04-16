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
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsPlaneNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkIndent.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkRegularPolygonSource.h>
#include <vtkTestingOutputWindow.h>
#include <vtkTransform.h>

// STL includes
#include <sstream>

static const double EPSILON = 1e-5;

bool ComparePlane(double xAxisExpected_World[3], double yAxisExpected_World[3], double zAxisExpected_World[3],
  double originExpected_World[3], vtkMRMLMarkupsPlaneNode* planeNode, double epsilon)
{
  double xAxisActual_World[3] = { 0.0 };
  double yAxisActual_World[3] = { 0.0 };
  double zAxisActual_World[3] = { 0.0 };
  planeNode->GetAxesWorld(xAxisActual_World, yAxisActual_World, zAxisActual_World);

  if (vtkMath::Dot(xAxisExpected_World, xAxisActual_World) < 1.0 - epsilon)
    {
    return false;
    }
  if (vtkMath::Dot(yAxisExpected_World, yAxisActual_World) < 1.0 - epsilon)
    {
    return false;
    }
  if (vtkMath::Dot(zAxisExpected_World, zAxisActual_World) < 1.0 - epsilon)
    {
    return false;
    }

  double originActual_World[3] = { 0.0 };
  planeNode->GetOriginWorld(originActual_World);
  double originDifference_World[3] = { 0.0 };
  vtkMath::Subtract(originExpected_World, originActual_World, originDifference_World);

  if (vtkMath::Norm(originDifference_World) > epsilon)
    {
    return false;
    }

  return true;
}

int vtkMRMLMarkupsNodeTest4(int , char * [] )
{
  std::cout << "Testing vtkMRMLMarkupsPlaneNode" << std::endl;
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLMarkupsPlaneNode> planeNode;
  scene->AddNode(planeNode);

  double xAxis_World[3] = { 0.0, 0.0, 1.0 };
  double yAxis_World[3] = { -1.0, 0.0, 0.0 };
  double zAxis_World[3] = { 0.0, -1.0, 0.0 };
  double origin_World[3] = { 50.0, 150.0, 200.0 };

  /////////////
  std::cout << "Test set axes/origin" << std::endl;
  planeNode->SetAxesWorld(xAxis_World, yAxis_World, zAxis_World);
  planeNode->SetOriginWorld(origin_World);
  CHECK_BOOL(ComparePlane(xAxis_World, yAxis_World, zAxis_World, origin_World, planeNode, EPSILON), true);

  /////////////
  std::cout << "Test set axes/origin with plane offset" << std::endl;
  vtkNew<vtkTransform> planeToPlaneOffset;
  planeToPlaneOffset->Translate(1.0, 2.0, 3.0);
  planeToPlaneOffset->RotateX(50.0);
  planeToPlaneOffset->RotateY(12.0);
  planeToPlaneOffset->RotateZ(5.0);
  planeToPlaneOffset->Translate(5.0, 3.0, 10.0);
  planeNode->GetPlaneToPlaneOffsetMatrix()->DeepCopy(planeToPlaneOffset->GetMatrix());
  planeNode->SetAxesWorld(xAxis_World, yAxis_World, zAxis_World);
  planeNode->SetOriginWorld(origin_World);
  CHECK_BOOL(ComparePlane(xAxis_World, yAxis_World, zAxis_World, origin_World, planeNode, EPSILON), true);

  /////////////
  std::cout << "Test set axes/origin with plane offset and transform node" << std::endl;
  vtkNew<vtkMRMLLinearTransformNode> transformNode;
  scene->AddNode(transformNode);
  planeNode->SetAndObserveTransformNodeID(transformNode->GetID());

  vtkNew<vtkTransform> localToWorldTransform;
  localToWorldTransform->Translate(30.0, 60.0, 90.0);
  localToWorldTransform->RotateZ(30.0);
  localToWorldTransform->RotateY(60.0);
  localToWorldTransform->RotateX(90.0);
  localToWorldTransform->Translate(90.0, 60.0, 30.0);
  transformNode->SetMatrixTransformToParent(localToWorldTransform->GetMatrix());
  planeNode->SetAxesWorld(xAxis_World, yAxis_World, zAxis_World);
  planeNode->SetOriginWorld(origin_World);
  CHECK_BOOL(ComparePlane(xAxis_World, yAxis_World, zAxis_World, origin_World, planeNode, EPSILON), true);

  /////////////
  std::cout << "Test set norm with plane offset and transform node" << std::endl;
  double expectedNormal_World[3] = { 1.0, -3.0, 5.0 };
  vtkMath::Normalize(expectedNormal_World);
  planeNode->SetNormalWorld(expectedNormal_World);
  double actualNormal_World[3] = { 0.0 };
  planeNode->GetNormalWorld(actualNormal_World);
  double normalDifference_World[3] = { 0.0 };
  vtkMath::Subtract(actualNormal_World, expectedNormal_World, normalDifference_World);
  CHECK_DOUBLE_TOLERANCE(vtkMath::Norm(normalDifference_World), 0.0, EPSILON);

  /////////////
  std::cout << "Test set origin with plane offset and transform node" << std::endl;
  double expectedOrigin_World[3] = { -123.0, 456.0, -789.0 };
  planeNode->SetOriginWorld(expectedOrigin_World);
  double actualOrigin_World[3] = { 0.0 };
  planeNode->GetOriginWorld(actualOrigin_World);
  double originDifference_World[3] = { 0.0 };
  vtkMath::Subtract(actualOrigin_World, expectedOrigin_World, originDifference_World);
  CHECK_DOUBLE_TOLERANCE(vtkMath::Norm(originDifference_World), 0.0, EPSILON);

  std::cout << "Success." << std::endl;

  return EXIT_SUCCESS;
}
