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
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

// VTK includes
#include <vtkIndent.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// STL includes
#include <sstream>

int vtkMRMLMarkupsNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLMarkupsNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_BOOLEAN(node1, Locked);

  node1->SetMarkupLabelFormat(std::string("%N-%d"));
  node1->SetName("testingname");
  std::string formatTest = node1->ReplaceListNameInMarkupLabelFormat();
  CHECK_STD_STRING(formatTest, "testingname-%d");

  vtkNew<vtkMRMLMeasurement> measurement1;
  measurement1->SetName("Diameter");
  measurement1->SetValue(15.0);
  measurement1->SetUnits("mm2");
  node1->AddMeasurement(measurement1);
  CHECK_INT(node1->GetNumberOfMeasurements(), 1);
  CHECK_STRING(node1->GetNthMeasurement(0)->GetName(), "Diameter");

  node1->SetNthMeasurement(0, "Radius", 11.1, "cm");
  CHECK_STRING(node1->GetNthMeasurement(0)->GetName(), "Radius");

  node1->RemoveNthMeasurement(0);
  CHECK_INT(node1->GetNumberOfMeasurements(), 0);

  node1->SetNthMeasurement(0, "Cross-section area", 15.2, "mm2");
  node1->SetNthMeasurement(1, "Volume", 1.3, "mm3");
  node1->SetNthMeasurement(2, "Length", 25.4, "mm");
  CHECK_INT(node1->GetNumberOfMeasurements(), 3);

  node1->RemoveNthMeasurement(0);
  CHECK_INT(node1->GetNumberOfMeasurements(), 2);
  CHECK_STRING(node1->GetNthMeasurement(0)->GetName(), "Volume");
  CHECK_STRING(node1->GetNthMeasurement(1)->GetName(), "Length");

  //
  // test methods with no markups
  //

  std::cout << "Removing all markups" << std::endl;
  node1->RemoveAllControlPoints();
  CHECK_INT(node1->GetNumberOfControlPoints(), 0);

  // MarkupExists
  std::cout << "Checking if markup exists in empty markups node" << std::endl;
  for (int m = -1; m < 3; m++)
    {
    CHECK_BOOL(node1->ControlPointExists(m), false);
    }

  // Get Nth Markup
  vtkMRMLMarkupsNode::ControlPoint *markup;
  for (int n = -1; n < 3; n++)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    markup = node1->GetNthControlPoint(n);
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    CHECK_NULL(markup);
    }

  // AddMarkupWithNPoints
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->AddNControlPoints(-1);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_INT(node1->GetNumberOfControlPoints(), 0);

  node1->AddNControlPoints(0);
  CHECK_INT(node1->GetNumberOfControlPoints(), 0);

  // RemoveMarkup
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->RemoveNthControlPoint(-1);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->RemoveNthControlPoint(0);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  CHECK_INT(node1->GetNumberOfControlPoints(), 0);

  //
  // now add some markups
  //
  std::cout << "Adding markups with points" << std::endl;
  node1->AddNControlPoints(1);
  node1->AddNControlPoints(2);
  node1->AddNControlPoints(3);
  node1->AddNControlPoints(6);
  CHECK_INT(node1->GetNumberOfControlPoints(), 12);

  /// Associated node ids
  node1->SetNthControlPointAssociatedNodeID(2, std::string("vtkMRMLScalarVolumeNode1"));
  node1->Print(std::cout);
  CHECK_STD_STRING(node1->GetNthControlPointAssociatedNodeID(2), "vtkMRMLScalarVolumeNode1");

  //
  // ID
  //
  std::cout << "IDs:" << std::endl;
  for (int n = 0; n < node1->GetNumberOfControlPoints(); ++n)
    {
    std::cout << n << ": id = " << node1->GetNthControlPointID(n).c_str() << std::endl;
    }
  // reset one
  std::string oldID = node1->GetNthControlPointID(0);
  CHECK_BOOL(node1->ResetNthControlPointID(0), true);
  std::cout << "After resetting 0th markup id from " << oldID.c_str() << ", new one is " << node1->GetNthControlPointID(0).c_str() << std::endl;

  //
  // orientation
  //
  std::cout << "Orientations:" << std::endl;
  double orientation[4];
  for (int n = 0; n < node1->GetNumberOfControlPoints(); ++n)
    {
    node1->GetNthControlPointOrientation(n, orientation);
    std::cout << n << ": orientation = "
              << orientation[0] << ","
              << orientation[1] << ","
              << orientation[2] << ","
              << orientation[3] << std::endl;
    // test for default
    if (orientation[0] != 0.0 ||
        orientation[1] != 0.0 ||
        orientation[2] != 0.0 ||
        orientation[3] != 1.0)
      {
      std::cerr << "Incorrect default orientation for markup " << n
                << "! Expected 0.0, 0.0, 0.0, 1.0," << std::endl;
      return EXIT_FAILURE;
      }
    }
  double testOrientation[4] = {0.5, 1.0, 0.0, 0.0};
  node1->SetNthControlPointOrientation(0,
                                 testOrientation[0], testOrientation[1],
                                 testOrientation[2], testOrientation[3]);
  double newOrientation[4];
  node1->GetNthControlPointOrientation(0, newOrientation);
  for (int r = 0; r < 4; r++)
    {
    if (newOrientation[r] != testOrientation[r])
      {
      std::cerr << "Failed to set orientation! "
                << "Expected: "
                << testOrientation[0] << ", "
                << testOrientation[1] << ", "
                << testOrientation[2] << ", "
                << testOrientation[3]
                << " but got: "
                << newOrientation[0] << ", "
                << newOrientation[1] << ", "
                << newOrientation[2] << ", "
                << newOrientation[3] << std::endl;
      return EXIT_FAILURE;
      }
    }

  testOrientation[0] = 0.333;
  node1->SetNthControlPointOrientationFromArray(0, testOrientation);
  node1->GetNthControlPointOrientation(0, newOrientation);
  for (int r = 0; r < 4; r++)
    {
    if (fabs(newOrientation[r] - testOrientation[r]) > 1e-5)
      {
      std::cerr << "Failed to set orientation from array! "
                << "Expected: "
                << testOrientation[0] << ", "
                << testOrientation[1] << ", "
                << testOrientation[2] << ", "
                << testOrientation[3]
                << " but got: "
                << newOrientation[0] << ", "
                << newOrientation[1] << ", "
                << newOrientation[2] << ", "
                << newOrientation[3] << std::endl;
      return EXIT_FAILURE;
      }
    }

  testOrientation[0] = 0.111;
  testOrientation[1] = 0.0;
  testOrientation[2] = 1.0;
  testOrientation[3] = 0.0;
  double *orientationPointer = testOrientation;
  node1->SetNthControlPointOrientationFromPointer(0, orientationPointer);
  node1->GetNthControlPointOrientation(0, newOrientation);
  for (int r = 0; r < 4; r++)
    {
    if (fabs(newOrientation[r] - testOrientation[r]) > 1e-5)
      {
      std::cerr << "Failed to set orientation from pointer! "
                << "Expected: "
                << testOrientation[0] << ", "
                << testOrientation[1] << ", "
                << testOrientation[2] << ", "
                << testOrientation[3]
                << " but got: "
                << newOrientation[0] << ", "
                << newOrientation[1] << ", "
                << newOrientation[2] << ", "
                << newOrientation[3] << std::endl;
      return EXIT_FAILURE;
      }
    }

  //
  // Selected/Visib
  //

  CHECK_BOOL(node1->GetNthControlPointSelected(1), true);
  node1->SetNthControlPointSelected(1, false);
  CHECK_BOOL(node1->GetNthControlPointSelected(1), false);

  CHECK_BOOL(node1->GetNthControlPointVisibility(1), true);
  node1->SetNthControlPointVisibility(1,false);
  CHECK_BOOL(node1->GetNthControlPointVisibility(1), false);

  //
  // Label
  //
  node1->SetNthControlPointLabel(1, std::string("TestLabel"));
  CHECK_STD_STRING(node1->GetNthControlPointLabel(1), "TestLabel");

  //
  // test methods with markups
  //
  double p0[3];
  p0[0] = 0.99;
  p0[1] = 1.33;
  p0[2] = -9.0;
  node1->SetNthControlPointPosition(2, p0[0], p0[1], p0[2]);
  vtkVector3d p1 = node1->GetNthControlPointPositionVector(2);
  if (p1.GetX() != p0[0] ||
      p1.GetY() != p0[1] ||
      p1.GetZ() != p0[2])
    {
    std::cerr << "Failed to get back markup 2 point 1 via vector, expected "
              << p0[0] << "," << p0[1] << "," << p0[2] << ", but got "
              << p1.GetX() << "," << p1.GetY() << "," << p1.GetZ() << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Get markup point vector: " << p1.GetX() << "," << p1.GetY() << "," << p1.GetZ() << std::endl;
    }
  double p3[3];
  node1->GetNthControlPointPosition(2,p3);
  if (p3[0] != p0[0] ||
      p3[1] != p0[1] ||
      p3[2] != p0[2])
    {
    std::cerr << "Failed to get back markup 2 point 1 via array, expected "
              << p0[0] << "," << p0[1] << "," << p0[2] << ", but got "
              << p3[0] << "," << p3[1] << "," << p3[2] << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Get markup point array: " << p3[0] << "," << p3[1] << "," << p3[2] << std::endl;
    }

  vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
  double offset[3] = {-10.0, 5.5, 0.22};
  mat->SetElement(0,3,offset[0]);
  mat->SetElement(1,3,offset[1]);
  mat->SetElement(2,3,offset[2]);
  std::cout << "Using transform matrix:" << std::endl;
  mat->Print(std::cout);
  double expectedPos[3];
  expectedPos[0] = p3[0] + offset[0];
  expectedPos[1] = p3[1] + offset[1];
  expectedPos[2] = p3[2] + offset[2];
  node1->ApplyTransformMatrix(mat);
  double outputPoint[3];
  node1->GetNthControlPointPosition(2,outputPoint);
  std::cout << "Input Point = " << p3[0] << "," << p3[1] << "," << p3[2] << std::endl;
  std::cout << "Offset = " << offset[0] << "," << offset[1] << "," << offset[2] << std::endl;
  std::cout << "Expected Output = " << expectedPos[0] << "," << expectedPos[1] << "," << expectedPos[2] << std::endl;
  std::cout << "Output Point = " << outputPoint[0] << "," << outputPoint[1] << "," << outputPoint[2] << std::endl;
  double diff = sqrt(vtkMath::Distance2BetweenPoints(expectedPos, outputPoint));
  if (diff > 0.1)
    {
    std::cerr << "Difference between expected and output too large: " << diff << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "\tdiff = " << diff << std::endl;
    }

  // test WriteCLI
  vtkIndent indent;
  int numMarkups = node1->GetNumberOfControlPoints();
  for (int m = 0; m < numMarkups; m++)
    {
    // make sure all are selected so all will be passed
    node1->SetNthControlPointSelected(m, true);
    }
  std::cout << "\nTesting WriteCLI call on markups node with "
            << numMarkups<< " markups." << std::endl;
  node1->PrintSelf(std::cout, indent);
  std::vector<std::string> commandLine1;
  std::string prefix = "point";
  node1->WriteCLI(commandLine1, prefix, vtkMRMLStorageNode::CoordinateSystemRAS);
  std::cout << "Wrote RAS points to CLI as:" << std::endl;
  for (unsigned int i = 0; i < commandLine1.size(); ++i)
    {
    std::cout << commandLine1[i].c_str() << std::endl;
    }
  CHECK_INT(commandLine1.size(), numMarkups * 2);

  std::vector<std::string> commandLine2;
  node1->WriteCLI(commandLine2, prefix, vtkMRMLStorageNode::CoordinateSystemLPS);
  std::cout << "Wrote LPS points to CLI as:" << std::endl;;
  for (unsigned int i = 0; i < commandLine2.size(); ++i)
    {
    std::cout << commandLine2[i].c_str() << std::endl;
    }
  CHECK_INT(commandLine2.size(), numMarkups * 2);

  // single point test
  std::vector<std::string> commandLine3;
  node1->WriteCLI(commandLine3, prefix, vtkMRMLStorageNode::CoordinateSystemRAS, 0);
  std::cout << "Wrote single RAS markup to CLI (command line size "
            << commandLine3.size() << ") :" << std::endl;
  for (unsigned int i = 0; i < commandLine3.size(); ++i)
    {
    std::cout << commandLine3[i].c_str() << std::endl;
    }
  CHECK_INT(commandLine3.size(), 2);

  return EXIT_SUCCESS;
}
