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
  // should be invalid if scene is not set
  CHECK_STD_STRING(id, "");

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  int fidIndex = logic1->AddFiducial();
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  // should be invalid if scene is not set
  CHECK_INT(fidIndex, -1);

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  int sliceIntersectionVisibility = logic1->GetSliceIntersectionsVisibility();
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  // should be invalid if scene is not set
  CHECK_INT(sliceIntersectionVisibility, -1);
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
  CHECK_STD_STRING_DIFFERENT(id, "");

  vtkMRMLNode *mrmlNode = scene->GetNodeByID(id.c_str());
  CHECK_NOT_NULL(mrmlNode);
  char *name = mrmlNode->GetName();
  CHECK_NOT_NULL(name);
  CHECK_STRING(testName, name);

  std::cout << "Added a new markup node to the scene, id = '" << id.c_str() << "', name = '" << name << "'" <<  std::endl;
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  CHECK_NOT_NULL(markupsNode);

  // test the list stuff
  logic1->SetAllMarkupsVisibility(nullptr, true);
  logic1->ToggleAllMarkupsVisibility(nullptr);
  logic1->SetAllMarkupsLocked(nullptr, false);
  logic1->ToggleAllMarkupsLocked(nullptr);
  logic1->SetAllMarkupsSelected(nullptr, true);
  logic1->ToggleAllMarkupsSelected(nullptr);

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
  markupsNode->AddNControlPoints(5);
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
  vtkMRMLMarkupsDisplayNode* defaultDisplayNode = logic1->GetDefaultMarkupsDisplayNode();
  CHECK_NOT_NULL(defaultDisplayNode);

  // make a test display node and reset it to defaults
  int originalGlyphType = defaultDisplayNode->GetGlyphType();
  double originalGlyphScale = defaultDisplayNode->GetGlyphScale();
  double originalTextScale = defaultDisplayNode->GetTextScale();
  int glyphType = 3;
  displayNode->SetGlyphType(glyphType);
  double textScale = 3.33;
  displayNode->SetTextScale(textScale);
  double glyphScale = 0.33;
  displayNode->SetGlyphScale(glyphScale);

  // reset the display node to defaults
  logic1->SetDisplayNodeToDefaults(displayNode);
  // check that the logic didn't change
  CHECK_INT(defaultDisplayNode->GetGlyphType(), originalGlyphType);
  // check that the display node is changed
  CHECK_INT(displayNode->GetGlyphType(), originalGlyphType);
  CHECK_DOUBLE(displayNode->GetGlyphScale(), originalGlyphScale);
  CHECK_DOUBLE(displayNode->GetTextScale(), originalTextScale);

  // test without a selection node
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  fidIndex = logic1->AddFiducial(5.0, 6.0, -7.0);
  TESTING_OUTPUT_ASSERT_ERRORS_END(); // error is expected to be reported due to lack of selection node
  CHECK_INT(fidIndex, -1);

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(logic1->StartPlaceMode(0), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END(); // error is expected to be reported due to lack of selection node

  // add a selection node
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene);

  CHECK_BOOL(logic1->StartPlaceMode(1), true);

  // test adding a fiducial to an active list - no app logic
  fidIndex = logic1->AddFiducial(-1.1, 100.0, 500.0);
  CHECK_BOOL(fidIndex >= 0, true);
  std::cout << "Added a fid to the active fid list, index = " << fidIndex << std::endl;

  // adding with app logic
  logic1->SetMRMLApplicationLogic(applicationLogic.GetPointer());
  fidIndex = logic1->AddFiducial(-11, 10.0, 50.0);
  CHECK_BOOL(fidIndex >= 0, true);
  std::cout << "Added a fid to the active fid list, index = " << fidIndex << std::endl;

  // test the renaming
  std::string activeListID = logic1->GetActiveListID();
  mrmlNode = scene->GetNodeByID(activeListID.c_str());
  CHECK_NOT_NULL(mrmlNode);
  vtkMRMLMarkupsNode *activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  CHECK_NOT_NULL(activeMarkupsNode);

  std::cout << "Before renaming:" << std::endl;
  for (int i = 0; i < activeMarkupsNode->GetNumberOfControlPoints(); ++i)
    {
    std::cout << "Markup " << i << " label = "
              << activeMarkupsNode->GetNthControlPointLabel(i).c_str() << std::endl;
    }

  activeMarkupsNode->SetName("RenamingTest");
  activeMarkupsNode->SetMarkupLabelFormat("T %d %N");
  logic1->RenameAllMarkupsFromCurrentFormat(activeMarkupsNode);
  std::string newLabel = activeMarkupsNode->GetNthControlPointLabel(0);
  std::string expectedLabel = std::string("T 1 RenamingTest");
  CHECK_STD_STRING(newLabel, expectedLabel);

  std::cout << "After renaming:" << std::endl;
  for (int i = 0; i < activeMarkupsNode->GetNumberOfControlPoints(); ++i)
    {
    std::cout << "Markup " << i << " label = "
              << activeMarkupsNode->GetNthControlPointLabel(i).c_str() << std::endl;
    }

  // test setting active list id
  std::string newID = logic1->AddNewFiducialNode("New list", scene);
  activeListID = logic1->GetActiveListID();
  CHECK_STD_STRING(activeListID, newID);

  // set the old one active
  logic1->SetActiveListID(activeMarkupsNode);
  activeListID = logic1->GetActiveListID();
  CHECK_STD_STRING(activeListID, activeMarkupsNode->GetID());

  sliceIntersectionVisibility = logic1->GetSliceIntersectionsVisibility();
  CHECK_INT(sliceIntersectionVisibility, 0);

  logic1->SetSliceIntersectionsVisibility(true);
  sliceIntersectionVisibility = logic1->GetSliceIntersectionsVisibility();
  CHECK_INT(sliceIntersectionVisibility, 0);

  // now add a slice composite node
  vtkNew<vtkMRMLSliceCompositeNode> compNode;
  scene->AddNode(compNode.GetPointer());
  logic1->SetSliceIntersectionsVisibility(true);
  sliceIntersectionVisibility = logic1->GetSliceIntersectionsVisibility();
  CHECK_INT(sliceIntersectionVisibility, 1);

  return EXIT_SUCCESS;
}
