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

static void PrintLabels(vtkMRMLMarkupsNode *m)
{
  if (!m)
    {
    return;
    }
  for (int i = 0; i < m->GetNumberOfMarkups(); ++i)
    {
    std::cout << i << ": " << m->GetNthMarkupLabel(i).c_str() << std::endl;
    }
}

int vtkSlicerMarkupsLogicTest2(int , char * [] )
{
  vtkNew<vtkSlicerMarkupsLogic> logic1;

  // Test moving markups between lists
  vtkSmartPointer<vtkMRMLMarkupsNode> source = vtkSmartPointer<vtkMRMLMarkupsNode>::New();
  vtkSmartPointer<vtkMRMLMarkupsNode> dest = vtkSmartPointer<vtkMRMLMarkupsNode>::New();

  // null cases
  if (logic1->MoveNthMarkupToNewListAtIndex(0, NULL, NULL, 0))
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: Failed to return false when passed null lists" << std::endl;
    return EXIT_FAILURE;
    }
  if (logic1->MoveNthMarkupToNewListAtIndex(0, source, NULL, 0))
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: Failed to return false when passed null destination list" << std::endl;
    return EXIT_FAILURE;
    }
  if (logic1->MoveNthMarkupToNewListAtIndex(0, NULL, dest, 0))
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: Failed to return false when passed null source list" << std::endl;
    return EXIT_FAILURE;
    }

  // empty lists
  if (logic1->MoveNthMarkupToNewListAtIndex(0, source, dest, 0))
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: Failed to return false when passed emtpy source and dest list" << std::endl;
    return EXIT_FAILURE;
    }

  // dest list smaller than source list
  source->AddMarkupWithNPoints(1);
  source->SetNthMarkupLabel(0,"S1");
  source->SetNthMarkupDescription(0,"To be moved");
  double x =  5.5;
  double y = -1.2;
  double z = 10.6;
  source->SetMarkupPoint(0, 0, x, y, z);

  if (!logic1->MoveNthMarkupToNewListAtIndex(0, source, dest, 0))
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: Failed to move fid from list of size 1 to list of size 0" << std::endl;
    return EXIT_FAILURE;
    }
  if (source->GetNumberOfMarkups() != 0)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: failed to move fiducial out of source list, list size = "
              << source->GetNumberOfMarkups()  << std::endl;
    return EXIT_FAILURE;
    }
  if (dest->GetNumberOfMarkups() != 1)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: failed to move fiducial into dest list, list size = "
              << dest->GetNumberOfMarkups() << std::endl;
    return EXIT_FAILURE;
    }
  std::string movedLabel = dest->GetNthMarkupLabel(0);
  std::string movedDescription = dest->GetNthMarkupDescription(0);
  double movedPoint[3];
  dest->GetMarkupPoint(0, 0, movedPoint);
  if (movedLabel.compare("S1") != 0 ||
      movedDescription.compare("To be moved") != 0 ||
      movedPoint[0] != x ||
      movedPoint[1] != y ||
      movedPoint[2] != z)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: moved fiducial doesn't match source, label = " << movedLabel.c_str()
              << ", description = " << movedDescription.c_str()
              << ", point = " << movedPoint[0] << ", " << movedPoint[1] << ", " << movedPoint[2]
              << ". Expected 'S1', 'To be moved', " << x << ", " << y << ", " << z
              << std::endl;
    return EXIT_FAILURE;
    }

  // dest list larger than source list
  source->AddMarkupWithNPoints(1);
  source->SetNthMarkupLabel(0, "S2");
  source->AddMarkupWithNPoints(1);
  source->SetNthMarkupLabel(1, "S3");
  source->AddMarkupWithNPoints(1);
  source->SetNthMarkupLabel(2, "S4");

  dest->AddMarkupWithNPoints(1);
  dest->SetNthMarkupLabel(1, "D1");
  dest->AddMarkupWithNPoints(1);
  dest->SetNthMarkupLabel(2, "D2");
  dest->AddMarkupWithNPoints(1);
  dest->SetNthMarkupLabel(3, "D3");
  dest->AddMarkupWithNPoints(1);
  dest->SetNthMarkupLabel(4, "D4");
  dest->AddMarkupWithNPoints(1);
  dest->SetNthMarkupLabel(5, "D5");
  int destStartSize = dest->GetNumberOfMarkups();
  // source should now be S2, S3, S4
  // dest should be S1, D1, D2, D3, D4, D5
  std::cout << "Starting more move testing, source = " << std::endl;
  vtkIndent indent;
  PrintLabels(source);
  std::cout << "\tdest, start size = " << destStartSize << std::endl;
  PrintLabels(dest);

  if (!logic1->MoveNthMarkupToNewListAtIndex(1, source, dest, 2))
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: Failed to move fid from index 0 to 2" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "After moving S3 to dest list, expect S1, D1, S3, D2, D3, D4, D5:" << std::endl;
  PrintLabels(dest);
  std::cout << "\t and source should be S2, S4" << std::endl;
  PrintLabels(source);
  // test it
  if (dest->GetNumberOfMarkups() != destStartSize + 1)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: "
              << " destination list is wrong size, got "
              << dest->GetNumberOfMarkups() << " != "
              << destStartSize + 1 << std::endl;
    return EXIT_FAILURE;
    }
  std::string newLabel = dest->GetNthMarkupLabel(2);
  if (newLabel.compare("S3") != 0)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: "
              << " after moving to index 2, new label "
              << newLabel.c_str() << " != " << " 'S3'" << std::endl;
    return EXIT_FAILURE;
    }
  // make sure that the next markup was moved down
  newLabel = dest->GetNthMarkupLabel(3);
  if (newLabel.compare("D2") != 0)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: "
              << "after moving to index 2, new label at index 3 "
              << newLabel.c_str() << " != " << " 'D2'" << std::endl;
    return EXIT_FAILURE;
    }
  // on the source list make sure that the next one was moved up
  newLabel = source->GetNthMarkupLabel(1);
  if (newLabel.compare("S4") != 0)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: "
              << "after moving a markup from the source list, expected that the new 0th label would be S4, but got "
              << newLabel.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  // add one to the beginning
  if (!logic1->MoveNthMarkupToNewListAtIndex(1, source, dest, 0))
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: Failed to move fid from index 1 to 0" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "After moving S4 to the start of the dest list, expect S4, S1, D1, S3, D2, D3, D4, D5:" << std::endl;
  PrintLabels(dest);
  if (dest->GetNumberOfMarkups() != destStartSize + 2)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: "
              << " destination list is wrong size, got "
              << dest->GetNumberOfMarkups() << " != "
              << destStartSize + 2 << std::endl;
    return EXIT_FAILURE;
    }
  newLabel = dest->GetNthMarkupLabel(0);
  if (newLabel.compare("S4") != 0)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: after moving to index 0, new label '" << newLabel.c_str() << "' != " << " 'S4'" << std::endl;
    return EXIT_FAILURE;
    }
  // check that last one is expected D5
  newLabel = dest->GetNthMarkupLabel(7);
  if (newLabel.compare("D5") != 0)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: after moving to index 0, new label at index 7 '" << newLabel.c_str() << "' != " << " 'D5'" << std::endl;
    return EXIT_FAILURE;
    }

  // and move to the end of the list
  if (!logic1->MoveNthMarkupToNewListAtIndex(0, source, dest, 8))
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: Failed to move fid from index 0 to 8" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "After moving S2 to destination list, source list should be empty:" << std::endl;
  PrintLabels(source);
  std::cout << "\t and destination should be S4, S1, D1, S3, D2, D3, D4, D5, S2:" << std::endl;
  PrintLabels(dest);
  if (source->GetNumberOfMarkups() != 0)
    {
    std::cerr << "Source list is not empty: size = "
              << source->GetNumberOfMarkups() << std::endl;
    return EXIT_FAILURE;
    }
  if (dest->GetNumberOfMarkups() != destStartSize + 3)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: "
              << "destination list is wrong size, got "
              << dest->GetNumberOfMarkups() << " != "
              << destStartSize + 3 << std::endl;
    return EXIT_FAILURE;
    }
  // check that index 8 has the rigth label in dest
  newLabel = dest->GetNthMarkupLabel(8);
  if (newLabel.compare("S2") != 0)
    {
    std::cerr << "MoveNthMarkupToNewListAtIndex: "
              << "after moving to index 8, new label '"
              << newLabel.c_str() << "' != " << " 'S2'" << std::endl;
    return EXIT_FAILURE;
    }

  // cleanup

  return EXIT_SUCCESS;
}
