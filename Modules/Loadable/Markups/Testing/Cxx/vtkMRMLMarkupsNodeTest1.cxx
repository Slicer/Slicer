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

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS( vtkMRMLMarkupsNode, node1 );

  TEST_SET_GET_BOOLEAN(node1, Locked);

  node1->SetMarkupLabelFormat(std::string("%N-%d"));
  node1->SetName("testingname");
  std::string formatTest = node1->ReplaceListNameInMarkupLabelFormat();
  if (formatTest.compare("testingname-%d") != 0)
    {
    std::cerr << "Failed to add list name to the format, "
              << "expected 'testingname-%d' but got " << formatTest.c_str()
              << std::endl;
    return EXIT_FAILURE;
    }

  node1->AddText("testing");

  int numTexts = node1->GetNumberOfTexts();
  if (numTexts != 1)
    {
    std::cerr << " Failed to add text 'testing', got number of texts " << numTexts << std::endl;
    return EXIT_FAILURE;
    }
  else { std::cout << "Added text, have " << numTexts << std::endl; }

  vtkStdString str = node1->GetText(0);
  if (str.compare("testing") != 0)
    {
    std::cerr << " Failed to add text 'testing', got text 0 " << str.c_str() << std::endl;
    return EXIT_FAILURE;
    }
  else { std::cout << "Got text " << str.c_str() << std::endl; }

  node1->SetText(0, "New string");
  str = node1->GetText(0);
  if (str.compare("New string") != 0)
    {
    std::cerr << " Failed to set text 0 to 'New string', got text 0 " << str.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  node1->DeleteText(0);
  numTexts = node1->GetNumberOfTexts();
  if (numTexts != 0)
    {
    std::cerr << " Failed to delete text 0, got number of texts " << numTexts << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Removing all markups" << std::endl;
  node1->RemoveAllMarkups();

  //
  // test methods with no markups
  //

  // MarkupExists
  std::cout << "Checking if markup exists in empty markups node" << std::endl;
  for (int m = -1; m < 3; m++)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    if (node1->MarkupExists(m))
      {
      std::cerr << "Says that markup " << m << " exists, it shouldn't" << std::endl;
      return EXIT_FAILURE;
      }
    else
      {
      std::cout << "\t" << m << " doesn't exist, good!" << std::endl;
      }
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    }

  // Get Number of Markups
  int numMarkups = node1->GetNumberOfMarkups();
  if (numMarkups != 0)
    {
    std::cerr << "Number of markups should be zero, got " << numMarkups << std::endl;
    return EXIT_FAILURE;
    }

  // Point Exists in Markup
  for (int p = -1; p < 3; p++)
    {
    for (int m = -1; m < 3; m++)
      {
      TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
      if (node1->PointExistsInMarkup(p, m))
        {
        std::cerr << "Says that point " << p << " in markup " <<  m << " exists, it shouldn't" << std::endl;
        return EXIT_FAILURE;
        }
      else
        {
        std::cout << "Points exists in markup: success on point " << p << " in markup " <<  m << std::endl;
        }
      TESTING_OUTPUT_ASSERT_ERRORS_END();
      }
    }

  // Get Number Of Points in Markup
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  int numPoints = node1->GetNumberOfPointsInNthMarkup(0);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (numPoints != 0)
    {
    std::cerr << "For empty markup 0, got " << numPoints << " instead of 0" << std::endl;
    return EXIT_FAILURE;
    }
  else { std::cout << "pass 1" << std::endl; }

  // Get Nth Markup
  Markup *markup;
  for (int n = -1; n < 3; n++)
    {
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    markup = node1->GetNthMarkup(n);
    TESTING_OUTPUT_ASSERT_ERRORS_END();
    if (markup)
      {
      if (markup->points.size() != 0)
        {
        std::cerr << "For empty markup " << n << ", got " << markup->points.size() << " instead of 0" << std::endl;
        return EXIT_FAILURE;
        }
      else { std::cout << "pass get nth markup " << n << std::endl; }
      }
    else { std::cout << "empty nth markup " << n << std::endl; }
    }

  // AddMarkupWithNPoints
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->AddMarkupWithNPoints(-1);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  numMarkups = node1->GetNumberOfMarkups();
  if (numMarkups != 0)
    {
    std::cerr << "Tried adding invalid number of points, -1, expected 0 markups, but got " << numMarkups << std::endl;
    return EXIT_FAILURE;
    }
  else { std::cout << "pass add markup with -1 points" << std::endl; }

  node1->AddMarkupWithNPoints(0);
  numMarkups = node1->GetNumberOfMarkups();
  if (numMarkups != 1)
    {
    std::cerr << "Tried adding markup with 0 points, expected 1 markups, but got " << numMarkups << std::endl;
    return EXIT_FAILURE;
    }
  else { std::cout << "pass add markup with 0 points" << std::endl; }

  // RemoveMarkup
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->RemoveMarkup(-1);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  std::cout << "Removed markup -1" << std::endl;
  node1->RemoveMarkup(0);
  std::cout << "Removed markup 0" << std::endl;

  numMarkups = node1->GetNumberOfMarkups();
  if (numMarkups != 0)
    {
    std::cerr << "Tried to delete markup 0, but still have " << numMarkups << " markups" << std::endl;
    return EXIT_FAILURE;
    }
  else { std::cout << "pass get number of markups with 0 markups" << std::endl; }

  //
  // now add some markups
  //
  std::cout << "Adding markups with points" << std::endl;
  node1->AddMarkupWithNPoints(1);
  node1->AddMarkupWithNPoints(2);
  node1->AddMarkupWithNPoints(3);
  node1->AddMarkupWithNPoints(6);

  numMarkups = node1->GetNumberOfMarkups();
  if (numMarkups != 4)
    {
    std::cerr << "Tried adding 4 markups, but have " << numMarkups << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Added four markups, have " << numMarkups << " markups" << std::endl;
    }

  numPoints = node1->GetNumberOfPointsInNthMarkup(2);
  if (numPoints != 3)
    {
    std::cerr << "Had made a 3 point markup at n = 2, got " << numPoints << " points instead" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Made a 3 point markup at n = 2, got " << numPoints << " back" << std::endl;
    }

  /// Associated node ids
  node1->SetNthMarkupAssociatedNodeID(2, std::string("vtkMRMLScalarVolumeNode1"));

  node1->Print(std::cout);

  std::string id = node1->GetNthMarkupAssociatedNodeID(2);
  if (id.compare("vtkMRMLScalarVolumeNode1") != 0)
    {
    std::cerr << "Failed to get back expected id of vtkMRMLScalarVolumeNode1,"
              << " got '" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Got back associated node id of " << id.c_str() << std::endl;
    }

  //
  // ID
  //
  std::cout << "IDs:" << std::endl;
  for (int n = 0; n < node1->GetNumberOfMarkups(); ++n)
    {
    std::cout << n << ": id = " << node1->GetNthMarkupID(n).c_str() << std::endl;
    }
  // reset one
  std::string oldID = node1->GetNthMarkupID(0);
  bool retval = node1->ResetNthMarkupID(0);
  if (!retval)
    {
    std::cerr << "Failed to reset 0th markup id from " << oldID.c_str() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "After resetting 0th markup id from " << oldID.c_str() << ", new one is " << node1->GetNthMarkupID(0).c_str() << std::endl;

  //
  // orientation
  //
  std::cout << "Orientations:" << std::endl;
  double orientation[4];
  for (int n = 0; n < node1->GetNumberOfMarkups(); ++n)
    {
    node1->GetNthMarkupOrientation(n, orientation);
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
  node1->SetNthMarkupOrientation(0,
                                 testOrientation[0], testOrientation[1],
                                 testOrientation[2], testOrientation[3]);
  double newOrientation[4];
  node1->GetNthMarkupOrientation(0, newOrientation);
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
  node1->SetNthMarkupOrientationFromArray(0, testOrientation);
  node1->GetNthMarkupOrientation(0, newOrientation);
  for (int r = 0; r < 4; r++)
    {
    if (newOrientation[r] != testOrientation[r])
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
  node1->SetNthMarkupOrientationFromPointer(0, orientationPointer);
  node1->GetNthMarkupOrientation(0, newOrientation);
  for (int r = 0; r < 4; r++)
    {
    if (newOrientation[r] != testOrientation[r])
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
  bool sel = node1->GetNthMarkupSelected();
  if (!sel)
    {
    std::cerr << "Error: markup should be selected by default" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetNthMarkupSelected(1, false);
  if (node1->GetNthMarkupSelected(1))
    {
    std::cerr << "Error: markup 1 should be unselected" << std::endl;
    return EXIT_FAILURE;
    }
  bool visib = node1->GetNthMarkupVisibility();
  if (!visib)
    {
    std::cerr << "Error: markup should be visib by default" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetNthMarkupVisibility(1,false);
  if (node1->GetNthMarkupVisibility(1))
    {
    std::cerr << "Error: markup 1 should be invisible" << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Label
  //
  node1->SetNthMarkupLabel(1, std::string("TestLabel"));
  std::string labelTest = node1->GetNthMarkupLabel(1);
  if (labelTest.compare("TestLabel") != 0)
    {
    std::cerr << "Failed to set first labe to TestLabel, got '" << labelTest.c_str() << "'" << std::endl;
    }
  else
    {
    std::cout << "Set 1st label to " << labelTest.c_str() << std::endl;
    }
  //
  // test methods with markups
  //
  double p0[3];
  p0[0] = 0.99;
  p0[1] = 1.33;
  p0[2] = -9.0;
  node1->SetMarkupPoint(2, 1, p0[0], p0[1], p0[2]);
  vtkVector3d p1 = node1->GetMarkupPointVector(2,1);
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
  node1->GetMarkupPoint(2,1,p3);
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
  node1->GetMarkupPoint(2,1,outputPoint);
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
  numMarkups = node1->GetNumberOfMarkups();
  unsigned int expectedCommandLineSize = 0;
  for (int m = 0; m < numMarkups; m++)
    {
    expectedCommandLineSize += 2*node1->GetNumberOfPointsInNthMarkup(m);
    // make sure all are selected so all will be passed
    node1->SetNthMarkupSelected(m, true);
    }
  std::cout << "\nTesting WriteCLI call on markups node with "
            << numMarkups<< " markups." << std::endl;
  node1->PrintSelf(std::cout, indent);
  std::vector<std::string> commandLine1;
  std::string prefix = "point";
  node1->WriteCLI(commandLine1, prefix, 0);
  std::cout << "Wrote RAS points to CLI as:" << std::endl;
  for (unsigned int i = 0; i < commandLine1.size(); ++i)
    {
    std::cout << commandLine1[i].c_str() << std::endl;
    }
  if (commandLine1.size() != expectedCommandLineSize)
    {
    std::cerr << "Incorrect number of command line args, for "
              << expectedCommandLineSize << " markup points, got "
              << commandLine1.size() << std::endl;
    return EXIT_FAILURE;
    }

  std::vector<std::string> commandLine2;
  node1->WriteCLI(commandLine2, prefix, 1);
  std::cout << "Wrote LPS points to CLI as:" << std::endl;;
  for (unsigned int i = 0; i < commandLine2.size(); ++i)
    {
    std::cout << commandLine2[i].c_str() << std::endl;
    }
  if (commandLine2.size() != expectedCommandLineSize)
    {
    std::cerr << "Incorrect number of command line args, for "
              << expectedCommandLineSize << " markup points, got "
              << commandLine2.size() << std::endl;
    return EXIT_FAILURE;
    }
  // single point test
  std::vector<std::string> commandLine3;
  unsigned int multipleFalseExpectedSize = 2*node1->GetNumberOfPointsInNthMarkup(0);
  node1->WriteCLI(commandLine3, prefix, 0, 0);
  std::cout << "Wrote single RAS markup to CLI (command line size "
            << commandLine3.size() << ") :" << std::endl;
  for (unsigned int i = 0; i < commandLine3.size(); ++i)
    {
    std::cout << commandLine3[i].c_str() << std::endl;
    }
  if (commandLine3.size() != multipleFalseExpectedSize)
    {
    std::cerr << "Too many markups written (" << commandLine3.size()
              << ") when multiple flag set to false was passed for " << numMarkups << " markups!"
              << " Expected to see " << multipleFalseExpectedSize << " points." << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
