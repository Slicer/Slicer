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
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkSlicerMarkupsLogic.h"
// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

int vtkSlicerMarkupsLogicTest1(int , char * [] )
{
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
/*vtkNew<vtkMRMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());*/

  vtkNew<vtkSlicerMarkupsLogic> logic1;

  // test without a scene
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  std::string id = logic1->AddNewFiducialNode();
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (!id.empty())
    {
    std::cerr << "Failure to add a new markup node to empty scene, got id of '" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed adding a node to no scene." << std::endl;
    }

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  int fidIndex = logic1->AddFiducial();
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (fidIndex != -1)
    {
    std::cerr << "Failure to add a new fiducial point to empty scene, got fidIndex of '" << fidIndex << "'" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed adding a fiducial point to no scene." << std::endl;
    }

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  int sliceIntersectionVisibility = logic1->GetSliceIntersectionsVisibility();
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (sliceIntersectionVisibility != -1)
    {
    std::cerr << "Failed to get no scene slice intersections visibility of -1, got "
              << sliceIntersectionVisibility << std::endl;
    return EXIT_FAILURE;
    }
  logic1->SetSliceIntersectionsVisibility(true);

  // test with a scene
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  logic1->SetMRMLScene(scene);
  TESTING_OUTPUT_ASSERT_ERRORS(1); // one error is expected to be reported due to lack of selection node
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  const char *testName = "Test node 2";
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  id = logic1->AddNewFiducialNode(testName);
  TESTING_OUTPUT_ASSERT_ERRORS_END(); // error is expected to be reported due to lack of selection node
  if (id.empty())
    {
    std::cerr << "Failure to add a new node to a valid scene, got id of '" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLNode *mrmlNode = scene->GetNodeByID(id.c_str());
  if (!mrmlNode)
    {
    std::cerr << "Failure to add a new node to a valid scene, couldn't find node with id'" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  char *name = mrmlNode->GetName();
  if (!name || strcmp(testName, name) != 0)
    {
    std::cerr << "Failed to set a name on the new node, got node name of '" << (name ? name : "null") << "'" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Added a new markup node to the scene, id = '" << id.c_str() << "', name = '" << name << "'" <<  std::endl;
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (!markupsNode)
    {
    std::cerr << "Failed to get the new node as a markups node" << std::endl;
    return EXIT_FAILURE;
    }
  // test the list stuff
  logic1->SetAllMarkupsVisibility(NULL, true);
  logic1->ToggleAllMarkupsVisibility(NULL);
  logic1->SetAllMarkupsLocked(NULL, false);
  logic1->ToggleAllMarkupsLocked(NULL);
  logic1->SetAllMarkupsSelected(NULL, true);
  logic1->ToggleAllMarkupsSelected(NULL);

  // no points
  logic1->SetAllMarkupsVisibility(markupsNode, false);
  logic1->SetAllMarkupsVisibility(markupsNode, true);
  logic1->ToggleAllMarkupsVisibility(markupsNode);
  logic1->SetAllMarkupsLocked(markupsNode, true);
  logic1->SetAllMarkupsLocked(markupsNode, false);
  logic1->ToggleAllMarkupsLocked(markupsNode);
  logic1->SetAllMarkupsSelected(markupsNode, false);
  logic1->SetAllMarkupsSelected(markupsNode, true);
  logic1->ToggleAllMarkupsSelected(markupsNode);

  // add some points
  markupsNode->AddMarkupWithNPoints(5);
  logic1->SetAllMarkupsVisibility(markupsNode, false);
  logic1->SetAllMarkupsVisibility(markupsNode, true);
  logic1->ToggleAllMarkupsVisibility(markupsNode);
  logic1->SetAllMarkupsLocked(markupsNode, true);
  logic1->SetAllMarkupsLocked(markupsNode, false);
  logic1->ToggleAllMarkupsLocked(markupsNode);
  logic1->SetAllMarkupsSelected(markupsNode, false);
  logic1->SetAllMarkupsSelected(markupsNode, true);
  logic1->ToggleAllMarkupsSelected(markupsNode);

  // test the default display node settings
  vtkSmartPointer<vtkMRMLMarkupsDisplayNode> displayNode = vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::New();
  TEST_SET_GET_INT_RANGE(logic1, DefaultMarkupsDisplayNodeGlyphType, displayNode->GetMinimumGlyphType(), displayNode->GetMaximumGlyphType());
  TEST_SET_GET_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeGlyphScale, 0.0, 10.0);
  TEST_SET_GET_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeTextScale, 0.0, 15.0);
  TEST_SET_GET_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeOpacity, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeColor, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeSelectedColor, 0.0, 1.0);
  TEST_SET_GET_INT_RANGE(logic1, DefaultMarkupsDisplayNodeSliceProjection, 0.0, 4.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeSliceProjectionColor, 0.0, 1.0);
  TEST_SET_GET_DOUBLE(logic1, DefaultMarkupsDisplayNodeSliceProjectionOpacity, 0.0);
  TEST_SET_GET_DOUBLE(logic1, DefaultMarkupsDisplayNodeSliceProjectionOpacity, 1.0);

  // make a test display node and reset it to defaults
  int originalGlyphType = logic1->GetDefaultMarkupsDisplayNodeGlyphType();
  double originalGlyphScale = logic1->GetDefaultMarkupsDisplayNodeGlyphScale();
  double originalTextScale = logic1->GetDefaultMarkupsDisplayNodeTextScale();
  int glyphType = 3;
  displayNode->SetGlyphType(glyphType);
  double textScale = 3.33;
  displayNode->SetTextScale(textScale);
  double glyphScale = 0.33;
  displayNode->SetGlyphScale(glyphScale);

  // reset the display node to defaults
  logic1->SetDisplayNodeToDefaults(displayNode);
  // check that the logic didn't change
  if (logic1->GetDefaultMarkupsDisplayNodeGlyphType() != originalGlyphType)
    {
    std::cerr << "Error resetting display node glyph type to "
              << originalGlyphType
              << ", logic was changed and now have glyph type: "
              << logic1->GetDefaultMarkupsDisplayNodeGlyphType() << std::endl;
    return EXIT_FAILURE;
    }
  // check that the display node is changed
  if (displayNode->GetGlyphType() != originalGlyphType)
    {
    std::cerr << "Error resetting display node glyph type to defaults, "
              << "was expecting  " << originalGlyphType
              << ", but got " << displayNode->GetGlyphType() << std::endl;
    return EXIT_FAILURE;
    }
  if (displayNode->GetGlyphScale() != originalGlyphScale)
    {
    std::cerr << "Error resetting display node glyph scale to defaults, "
              << "was expecting  " << originalGlyphScale
              << ", but got " << displayNode->GetGlyphScale() << std::endl;
    return EXIT_FAILURE;
    }
  if (displayNode->GetTextScale() != originalTextScale)
    {
    std::cerr << "Error resetting display node text scale to defaults,"
              << " was expecting  " << originalTextScale
              << ", but got " << displayNode->GetTextScale() << std::endl;
    return EXIT_FAILURE;
    }

  // test without a selection node
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  fidIndex = logic1->AddFiducial(5.0, 6.0, -7.0);
  TESTING_OUTPUT_ASSERT_ERRORS_END(); // error is expected to be reported due to lack of selection node
  if (fidIndex != -1)
    {
    std::cerr << "Failed on adding a fiducial to the scene with no selection node, expected index of -1, but got: "
              << fidIndex << std::endl;
      return EXIT_FAILURE;
    }

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  if (logic1->StartPlaceMode(0))
    {
    std::cerr << "Failed to fail starting place mode!" << std::endl;
    return EXIT_FAILURE;
    }
  TESTING_OUTPUT_ASSERT_ERRORS_END(); // error is expected to be reported due to lack of selection node

  // add a selection node
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene);

  if (!logic1->StartPlaceMode(1))
    {
    std::cerr << "Failed to start place mode after adding a selection node!"
              << std::endl;
    return EXIT_FAILURE;
    }

  // test adding a fiducial to an active list - no app logic
  fidIndex = logic1->AddFiducial(-1.1, 100.0, 500.0);
  if (fidIndex == -1)
    {
    std::cerr << "Failed to add a fiducial to the active fiducial list in the scene, got index of " << fidIndex << std::endl;
      return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Added a fid to the active fid list, index = " << fidIndex << std::endl;
    }

  // adding with app logic
  logic1->SetMRMLApplicationLogic(applicationLogic.GetPointer());
  fidIndex = logic1->AddFiducial(-11, 10.0, 50.0);
  if (fidIndex == -1)
    {
    std::cerr << "Failed to add a fiducial to the active fiducial list from the app logic, got index of " << fidIndex << std::endl;
      return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Added a fid to the active fid list, index = " << fidIndex << std::endl;
    }

  // test the renaming
  std::string activeListID = logic1->GetActiveListID();
  mrmlNode = scene->GetNodeByID(activeListID.c_str());
  vtkMRMLMarkupsNode *activeMarkupsNode = NULL;
  if (mrmlNode)
    {
    activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!activeMarkupsNode)
    {
    std::cerr << "Failed to get active markups list from id " << activeListID.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Before renaming:" << std::endl;
  for (int i = 0; i < activeMarkupsNode->GetNumberOfMarkups(); ++i)
    {
    std::cout << "Markup " << i << " label = "
              << activeMarkupsNode->GetNthMarkupLabel(i).c_str() << std::endl;
    }
  activeMarkupsNode->SetName("RenamingTest");
  activeMarkupsNode->SetMarkupLabelFormat("T %d %N");
  logic1->RenameAllMarkupsFromCurrentFormat(activeMarkupsNode);
  std::string newLabel = activeMarkupsNode->GetNthMarkupLabel(0);
  std::string expectedLabel = std::string("T 1 RenamingTest");
  if (newLabel.compare(expectedLabel) != 0)
    {
    std::cerr << "After rename, expected " << expectedLabel.c_str()
              << ", but got " << newLabel.c_str() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "After renaming:" << std::endl;
  for (int i = 0; i < activeMarkupsNode->GetNumberOfMarkups(); ++i)
    {
    std::cout << "Markup " << i << " label = "
              << activeMarkupsNode->GetNthMarkupLabel(i).c_str() << std::endl;
    }

  // test setting active list id
  std::string newID = logic1->AddNewFiducialNode("New list", scene);
  activeListID = logic1->GetActiveListID();
  if (activeListID.compare(newID) != 0)
    {
    std::cerr << "Failed to set new fiducial node active. newID = "
              << newID.c_str() << ", active id = "
              << activeListID.c_str() << std::endl;
    return EXIT_FAILURE;
    }
  // set the old one active
  logic1->SetActiveListID(activeMarkupsNode);
  activeListID = logic1->GetActiveListID();
  if (activeListID.compare(activeMarkupsNode->GetID()) != 0)
    {
    std::cerr << "Failed to set old fiducial node active. old id = "
              << activeMarkupsNode->GetID() << ", current active id = "
              << activeListID.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  sliceIntersectionVisibility = logic1->GetSliceIntersectionsVisibility();
  if (sliceIntersectionVisibility != 0)
    {
    std::cerr << "Failed to get no scene slice intersections visibility of 0, got "
              << sliceIntersectionVisibility << std::endl;
    return EXIT_FAILURE;
    }
  logic1->SetSliceIntersectionsVisibility(true);
  sliceIntersectionVisibility = logic1->GetSliceIntersectionsVisibility();
  if (sliceIntersectionVisibility != 0)
    {
    std::cerr << "Failed to get slice intersection visibility of 0"
              << " with no slice composite node, got "
              << sliceIntersectionVisibility << std::endl;
    return EXIT_FAILURE;
    }
  // now add a slice composite node
  vtkNew<vtkMRMLSliceCompositeNode> compNode;
  scene->AddNode(compNode.GetPointer());
  logic1->SetSliceIntersectionsVisibility(true);
  sliceIntersectionVisibility = logic1->GetSliceIntersectionsVisibility();
  if (sliceIntersectionVisibility != 1)
    {
    std::cerr << "Failed to get slice intersection visibility of 1"
              << " with a slice composite node, got "
              << sliceIntersectionVisibility << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
