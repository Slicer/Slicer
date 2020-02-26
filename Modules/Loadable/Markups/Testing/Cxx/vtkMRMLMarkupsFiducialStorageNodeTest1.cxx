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
#include "vtkMRMLMarkupsFiducialDisplayNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"
#include "vtkPolyData.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// STD includes
#include <algorithm>

int vtkMRMLMarkupsFiducialStorageNodeTest1(int argc, char * argv[] )
{
  vtkNew<vtkMRMLMarkupsFiducialStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  vtkNew<vtkMRMLMarkupsFiducialNode> markupsNode;
  vtkNew<vtkMRMLMarkupsFiducialDisplayNode> displayNode;

  // get the file name
  std::string fileName = std::string("testMarkupsStorageNode.fcsv");
  if (argc > 1)
    {
    fileName = std::string(argv[1]);
    }
  std::cout << "Using file name " << fileName.c_str() << std::endl;

  // set up a scene
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  scene->AddNode(markupsNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());
  markupsNode->SetAndObserveStorageNodeID(node1->GetID());
  markupsNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  // add a markup with one point with non default values
  int index =  markupsNode->AddNControlPoints(1);
  double orientation[4] = {0.2, 1.0, 0.0, 0.0};
  markupsNode->SetNthControlPointOrientationFromArray(index, orientation);
  markupsNode->ResetNthControlPointID(index);
  std::string associatedNodeID = std::string("testingAssociatedID");
  markupsNode->SetNthControlPointAssociatedNodeID(index,associatedNodeID);
  markupsNode->SetNthControlPointSelected(index, 0);
  markupsNode->SetNthControlPointVisibility(index, 0);
  markupsNode->SetNthControlPointLocked(index, 1);

  std::string label = std::string("Testing label");
  markupsNode->SetNthControlPointLabel(index, label);
  std::string desc = std::string("description with spaces");
  markupsNode->SetNthControlPointDescription(index, desc);
  double inputPoint[3] = {-9.9, 1.1, 0.87};
  markupsNode->SetNthControlPointPositionFromArray(index, inputPoint);

  // and add a markup with 1 point, default values
  markupsNode->AddNControlPoints(1);
  // and another one unsetting the label
  index = markupsNode->AddNControlPoints(1);
  markupsNode->SetNthControlPointLabel(index,"");

  // add another one with a label and description that have commas in them
  int commaIndex = markupsNode->AddNControlPoints(1);
  markupsNode->SetNthControlPointLabel(commaIndex, "Label, commas, two");
  markupsNode->SetNthControlPointDescription(commaIndex, "Description one comma \"and two quotes\", for more testing");
  // add another one with a label and description with complex combos of commas and quotes
  int quotesIndex = markupsNode->AddNControlPoints(1);
  markupsNode->SetNthControlPointLabel(quotesIndex, "Label with end quotes \"around the last phrase\"");
  markupsNode->SetNthControlPointDescription(quotesIndex, "\"Description fully quoted\"");

  //
  // test write
  //

  std::cout << "\nWriting this markup to file:" << std::endl;
  vtkIndent indent;
  markupsNode->PrintSelf(std::cout, indent);
  std::cout << std::endl;

  node1->SetFileName(fileName.c_str());
  std::cout << "Writing " << node1->GetFileName() << std::endl;
  int retval = node1->WriteData(markupsNode.GetPointer());
  CHECK_BOOL(retval, true);

  //
  // test read
  //
  vtkNew<vtkMRMLMarkupsFiducialStorageNode> snode2;
  vtkNew<vtkMRMLMarkupsFiducialNode> markupsNode2;
  vtkNew<vtkMRMLScene> scene2;
  scene2->AddNode(snode2.GetPointer());
  scene2->AddNode(markupsNode2.GetPointer());
  markupsNode2->SetAndObserveStorageNodeID(snode2->GetID());
  snode2->SetFileName(node1->GetFileName());

  std::cout << "Reading from " << snode2->GetFileName() << std::endl;

  retval = snode2->ReadData(markupsNode2.GetPointer());
  CHECK_BOOL(retval, true);

  std::cout << "\nMarkup read from file = " << std::endl;
  markupsNode2->PrintSelf(std::cout, indent);
  std::cout << std::endl;

  // test values on the first markup
  index = 0;
  double newOrientation[4];
  markupsNode2->GetNthControlPointOrientation(index, newOrientation);
  for (int r = 0; r < 4; r++)
    {
    CHECK_DOUBLE_TOLERANCE(newOrientation[r], orientation[r], 1e-3);
    }
  CHECK_STD_STRING(markupsNode2->GetNthControlPointAssociatedNodeID(index), associatedNodeID);
  CHECK_BOOL(markupsNode2->GetNthControlPointSelected(index), false);
  CHECK_BOOL(markupsNode2->GetNthControlPointVisibility(index), false);
  CHECK_BOOL(markupsNode2->GetNthControlPointLocked(index), true);
  CHECK_STD_STRING(markupsNode2->GetNthControlPointLabel(index), label);
  CHECK_STD_STRING(markupsNode2->GetNthControlPointDescription(index), desc);

  double outputPoint[3];
  markupsNode2->GetNthControlPointPosition(index, outputPoint);
  CHECK_DOUBLE_TOLERANCE(outputPoint[0], inputPoint[0], 0.1);
  CHECK_DOUBLE_TOLERANCE(outputPoint[1], inputPoint[1], 0.1);
  CHECK_DOUBLE_TOLERANCE(outputPoint[2], inputPoint[2], 0.1);

  // test the unset label on the third markup
  index = 2;
  CHECK_STD_STRING(markupsNode2->GetNthControlPointLabel(index), "");

  // now read it again with a display node defined
  vtkNew<vtkMRMLMarkupsFiducialDisplayNode> displayNode2;
  scene2->AddNode(displayNode2.GetPointer());
  markupsNode2->SetAndObserveDisplayNodeID(displayNode2->GetID());
  std::cout << "Added display node, re-reading from "
            << snode2->GetFileName() << std::endl;
  retval = snode2->ReadData(markupsNode2.GetPointer());
  CHECK_BOOL(retval, true);

  //
  // test with RAS coordinate system
  node1->UseRASOn();
  std::cout << "Writing file in RAS coordinate system:\n"
            << node1->GetFileName() << std::endl;
  retval = node1->WriteData(markupsNode.GetPointer());
  CHECK_BOOL(retval, true);

  // read it in after clearing out the test data
  // Set to use LPS to verify that not this hint but the coordinate system that
  // is specified in the file is taken into account.
  snode2->UseLPSOn();
  markupsNode2->RemoveAllControlPoints();
  std::cout << "Reading file that uses RAS coordinate system from " << snode2->GetFileName() << std::endl;

  retval = snode2->ReadData(markupsNode2.GetPointer());
  CHECK_BOOL(retval, true);
  CHECK_BOOL(snode2->GetUseRAS(), true);

  std::cout << "\nMarkups specified in RAS read from file = " << std::endl;
  markupsNode2->PrintSelf(std::cout, indent);

  // check the point coordinates are correct when stored in files in RAS coordinate system
  double outputPointLoadedFromRASFile[3];
  index = 0;
  markupsNode2->GetNthControlPointPosition(index, outputPointLoadedFromRASFile);
  CHECK_DOUBLE_TOLERANCE(outputPointLoadedFromRASFile[0], inputPoint[0], 0.1);
  CHECK_DOUBLE_TOLERANCE(outputPointLoadedFromRASFile[1], inputPoint[1], 0.1);
  CHECK_DOUBLE_TOLERANCE(outputPointLoadedFromRASFile[2], inputPoint[2], 0.1);

  // check for commas in the markup label and description
  std::string labelWithCommas = markupsNode2->GetNthControlPointLabel(commaIndex);
  int numCommas = std::count(labelWithCommas.begin(), labelWithCommas.end(), ',');
  CHECK_INT(numCommas, 2);
  std::string descriptionWithCommasAndQuotes = markupsNode2->GetNthControlPointDescription(commaIndex);
  numCommas = std::count(descriptionWithCommasAndQuotes.begin(),
                         descriptionWithCommasAndQuotes.end(),
                         ',');
  CHECK_INT(numCommas, 1);
  int numQuotes = std::count(descriptionWithCommasAndQuotes.begin(),
                            descriptionWithCommasAndQuotes.end(),
                            '"');
  CHECK_INT(numQuotes, 2);

  // check ending quoted label
  std::string labelWithQuotes = markupsNode2->GetNthControlPointLabel(quotesIndex);
  numQuotes = std::count(labelWithQuotes.begin(), labelWithQuotes.end(), '"');
  CHECK_INT(numQuotes, 2);

  // check fully quoted description
  std::string descWithQuotes = markupsNode2->GetNthControlPointDescription(quotesIndex);
  numQuotes = std::count(descWithQuotes.begin(), descWithQuotes.end(), '"');
  CHECK_INT(numQuotes, 2);

  return EXIT_SUCCESS;
}
