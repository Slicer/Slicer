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
#include "vtkSlicerMarkupsLogic.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

static void PrintLabels(vtkMRMLMarkupsNode *m)
{
  if (!m)
    {
    return;
    }
  for (int i = 0; i < m->GetNumberOfControlPoints(); ++i)
    {
    std::cout << i << ": " << m->GetNthControlPointLabel(i).c_str() << std::endl;
    }
}

int vtkSlicerMarkupsLogicTest2(int , char * [] )
{
  vtkNew<vtkSlicerMarkupsLogic> logic1;

  // Test moving markups between lists
  vtkSmartPointer<vtkMRMLMarkupsNode> source = vtkSmartPointer<vtkMRMLMarkupsNode>::New();
  vtkSmartPointer<vtkMRMLMarkupsNode> dest = vtkSmartPointer<vtkMRMLMarkupsNode>::New();

  // null cases
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  if (logic1->MoveNthControlPointToNewListAtIndex(0, nullptr, nullptr, 0))
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: Failed to return false when passed null lists" << std::endl;
    return EXIT_FAILURE;
    }
  if (logic1->MoveNthControlPointToNewListAtIndex(0, source, nullptr, 0))
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: Failed to return false when passed null destination list" << std::endl;
    return EXIT_FAILURE;
    }
  if (logic1->MoveNthControlPointToNewListAtIndex(0, nullptr, dest, 0))
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: Failed to return false when passed null source list" << std::endl;
    return EXIT_FAILURE;
    }
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // empty lists
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  if (logic1->MoveNthControlPointToNewListAtIndex(0, source, dest, 0))
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: Failed to return false when passed empty source and dest list" << std::endl;
    return EXIT_FAILURE;
    }
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // dest list smaller than source list
  source->AddNControlPoints(1);
  source->SetNthControlPointLabel(0,"S1");
  source->SetNthControlPointDescription(0,"To be moved");
  double x =  5.5;
  double y = -1.2;
  double z = 10.6;
  source->SetNthControlPointPosition(0, x, y, z);

  if (!logic1->MoveNthControlPointToNewListAtIndex(0, source, dest, 0))
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: Failed to move fid from list of size 1 to list of size 0" << std::endl;
    return EXIT_FAILURE;
    }
  if (source->GetNumberOfControlPoints() != 0)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: failed to move fiducial out of source list, list size = "
              << source->GetNumberOfControlPoints()  << std::endl;
    return EXIT_FAILURE;
    }
  if (dest->GetNumberOfControlPoints() != 1)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: failed to move fiducial into dest list, list size = "
              << dest->GetNumberOfControlPoints() << std::endl;
    return EXIT_FAILURE;
    }
  std::string movedLabel = dest->GetNthControlPointLabel(0);
  std::string movedDescription = dest->GetNthControlPointDescription(0);
  double movedPoint[3];
  dest->GetNthControlPointPosition(0, movedPoint);
  if (movedLabel.compare("S1") != 0 ||
      movedDescription.compare("To be moved") != 0 ||
      movedPoint[0] != x ||
      movedPoint[1] != y ||
      movedPoint[2] != z)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: moved fiducial doesn't match source, label = " << movedLabel.c_str()
              << ", description = " << movedDescription.c_str()
              << ", point = " << movedPoint[0] << ", " << movedPoint[1] << ", " << movedPoint[2]
              << ". Expected 'S1', 'To be moved', " << x << ", " << y << ", " << z
              << std::endl;
    return EXIT_FAILURE;
    }

  // dest list larger than source list
  source->AddNControlPoints(1);
  source->SetNthControlPointLabel(0, "S2");
  source->AddNControlPoints(1);
  source->SetNthControlPointLabel(1, "S3");
  source->AddNControlPoints(1);
  source->SetNthControlPointLabel(2, "S4");

  dest->AddNControlPoints(1);
  dest->SetNthControlPointLabel(1, "D1");
  dest->AddNControlPoints(1);
  dest->SetNthControlPointLabel(2, "D2");
  dest->AddNControlPoints(1);
  dest->SetNthControlPointLabel(3, "D3");
  dest->AddNControlPoints(1);
  dest->SetNthControlPointLabel(4, "D4");
  dest->AddNControlPoints(1);
  dest->SetNthControlPointLabel(5, "D5");
  int destStartSize = dest->GetNumberOfControlPoints();
  // source should now be S2, S3, S4
  // dest should be S1, D1, D2, D3, D4, D5
  std::cout << "Starting more move testing, source = " << std::endl;
  vtkIndent indent;
  PrintLabels(source);
  std::cout << "\tdest, start size = " << destStartSize << std::endl;
  PrintLabels(dest);

  if (!logic1->MoveNthControlPointToNewListAtIndex(1, source, dest, 2))
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: Failed to move fid from index 0 to 2" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "After moving S3 to dest list, expect S1, D1, S3, D2, D3, D4, D5:" << std::endl;
  PrintLabels(dest);
  std::cout << "\t and source should be S2, S4" << std::endl;
  PrintLabels(source);
  // test it
  if (dest->GetNumberOfControlPoints() != destStartSize + 1)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: "
              << " destination list is wrong size, got "
              << dest->GetNumberOfControlPoints() << " != "
              << destStartSize + 1 << std::endl;
    return EXIT_FAILURE;
    }
  std::string newLabel = dest->GetNthControlPointLabel(2);
  if (newLabel.compare("S3") != 0)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: "
              << " after moving to index 2, new label "
              << newLabel.c_str() << " != " << " 'S3'" << std::endl;
    return EXIT_FAILURE;
    }
  // make sure that the next markup was moved down
  newLabel = dest->GetNthControlPointLabel(3);
  if (newLabel.compare("D2") != 0)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: "
              << "after moving to index 2, new label at index 3 "
              << newLabel.c_str() << " != " << " 'D2'" << std::endl;
    return EXIT_FAILURE;
    }
  // on the source list make sure that the next one was moved up
  newLabel = source->GetNthControlPointLabel(1);
  if (newLabel.compare("S4") != 0)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: "
              << "after moving a markup from the source list, expected that the new 0th label would be S4, but got "
              << newLabel.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  // add one to the beginning
  if (!logic1->MoveNthControlPointToNewListAtIndex(1, source, dest, 0))
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: Failed to move fid from index 1 to 0" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "After moving S4 to the start of the dest list, expect S4, S1, D1, S3, D2, D3, D4, D5:" << std::endl;
  PrintLabels(dest);
  if (dest->GetNumberOfControlPoints() != destStartSize + 2)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: "
              << " destination list is wrong size, got "
              << dest->GetNumberOfControlPoints() << " != "
              << destStartSize + 2 << std::endl;
    return EXIT_FAILURE;
    }
  newLabel = dest->GetNthControlPointLabel(0);
  if (newLabel.compare("S4") != 0)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: after moving to index 0, new label '" << newLabel.c_str() << "' != " << " 'S4'" << std::endl;
    return EXIT_FAILURE;
    }
  // check that last one is expected D5
  newLabel = dest->GetNthControlPointLabel(7);
  if (newLabel.compare("D5") != 0)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: after moving to index 0, new label at index 7 '" << newLabel.c_str() << "' != " << " 'D5'" << std::endl;
    return EXIT_FAILURE;
    }

  // and move to the end of the list
  if (!logic1->MoveNthControlPointToNewListAtIndex(0, source, dest, 8))
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: Failed to move fid from index 0 to 8" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "After moving S2 to destination list, source list should be empty:" << std::endl;
  PrintLabels(source);
  std::cout << "\t and destination should be S4, S1, D1, S3, D2, D3, D4, D5, S2:" << std::endl;
  PrintLabels(dest);
  if (source->GetNumberOfControlPoints() != 0)
    {
    std::cerr << "Source list is not empty: size = "
              << source->GetNumberOfControlPoints() << std::endl;
    return EXIT_FAILURE;
    }
  if (dest->GetNumberOfControlPoints() != destStartSize + 3)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: "
              << "destination list is wrong size, got "
              << dest->GetNumberOfControlPoints() << " != "
              << destStartSize + 3 << std::endl;
    return EXIT_FAILURE;
    }
  // check that index 8 has the right label in dest
  newLabel = dest->GetNthControlPointLabel(8);
  if (newLabel.compare("S2") != 0)
    {
    std::cerr << "MoveNthControlPointToNewListAtIndex: "
              << "after moving to index 8, new label '"
              << newLabel.c_str() << "' != " << " 'S2'" << std::endl;
    return EXIT_FAILURE;
    }

  // Test copying markup between lists
  // null cases
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  if (logic1->CopyNthControlPointToNewList(0, nullptr, nullptr))
    {
    std::cerr << "CopyNthControlPointToNewList: Failed to return false when passed null lists" << std::endl;
    return EXIT_FAILURE;
    }
  if (logic1->CopyNthControlPointToNewList(0, source, nullptr))
    {
    std::cerr << "CopyNthControlPointToNewList: Failed to return false when passed null destination list" << std::endl;
    return EXIT_FAILURE;
    }
  if (logic1->CopyNthControlPointToNewList(0, nullptr, dest))
    {
    std::cerr << "CopyNthControlPointToNewList: Failed to return false when passed null source list" << std::endl;
    return EXIT_FAILURE;
    }
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  source->AddNControlPoints(1);
  int sourceSize = source->GetNumberOfControlPoints();
  int destSize = dest->GetNumberOfControlPoints();
  if (!logic1->CopyNthControlPointToNewList(0, source, dest))
    {
    std::cerr << "Failed to copy 0th markup to new list" << std::endl;
    return EXIT_FAILURE;
    }
  if (sourceSize != source->GetNumberOfControlPoints())
    {
    std::cerr << "Failed to copy 0th markup to new list, source list expected size was "
              << sourceSize << " but is now " << source->GetNumberOfControlPoints() << std::endl;
    return EXIT_FAILURE;
    }
  if ((destSize + 1) != dest->GetNumberOfControlPoints())
    {
    std::cerr << "Failed to copy 0th markup to new list, destination list expected size was "
              << destSize << " but is now " << dest->GetNumberOfControlPoints() << std::endl;
    return EXIT_FAILURE;
    }
  // cleanup

  return EXIT_SUCCESS;
}
