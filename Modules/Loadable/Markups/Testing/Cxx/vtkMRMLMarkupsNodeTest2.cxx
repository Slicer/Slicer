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
#include "vtkMRMLMarkupsNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// test copy and swap
int vtkMRMLMarkupsNodeTest2(int , char * [] )
{
  vtkNew<vtkMRMLMarkupsNode> node1;
  vtkIndent indent;

  // now try with some data
  vtkMRMLMarkupsNode::ControlPoint markup1, markup2;
  markup2.Label = std::string("Markup number two");
  markup2.Description = std::string("Description for two");
  markup2.AssociatedNodeID = std::string("vtkMRMLVolumeNode333");
  markup2.Selected = false;
  markup2.Locked = true;
  markup2.Visibility = false;
  markup2.Position[0] = 2.0;
  markup2.Position[1] = -5.0;
  markup2.Position[2] = 15.6;

  markup1 = markup2;

  if (markup1.Position[0] != markup2.Position[0] ||
      markup1.Position[1] != markup2.Position[1] ||
      markup1.Position[2] != markup2.Position[2])
    {
    std::cerr << "After copy, point 0 doesn't match in markups" << std::endl;
    return EXIT_FAILURE;
    }

  // test swap
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->SwapControlPoints(-1,100);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  std::cout << "Adding a markup with 1 point" << std::endl;
  node1->AddNControlPoints(1);
  double pos0[3];
  pos0[0] = 3.0;
  pos0[1] = 5.5;
  pos0[2] = -2.6;
  node1->SetNthControlPointPositionFromArray(0, pos0);

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->SwapControlPoints(-1,100);
  node1->SwapControlPoints(-1,1);
  node1->SwapControlPoints(1,-1);
  node1->SwapControlPoints(1,100);
  node1->SwapControlPoints(100,1);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  std::cout << "Adding another markup with 1 point" << std::endl;
  node1->AddNControlPoints(1);
  double pos1[3];
  pos1[0] = -3.9;
  pos1[1] = 15.5;
  pos1[2] = 2.666;
  node1->SetNthControlPointPositionFromArray(1, pos1);

  std::cout << "Swapping markups 0 and 1, num markups = " << node1->GetNumberOfControlPoints() << std::endl;
  node1->PrintSelf(std::cout, indent);

  node1->SwapControlPoints(0, 1);

  double pos0New[3], pos1New[3];
  node1->GetNthControlPointPosition(0, pos0New);
  if (pos0New[0] != pos1[0] ||
      pos0New[1] != pos1[1] ||
      pos0New[2] != pos1[2])
    {
    std::cerr << "Swap failed, point 0 expected: "
              << pos1[0] << ", " << pos1[1] << ", " << pos1[2]
              << ", but got:"
              << pos0New[0] << ", " << pos0New[1] << ", " << pos0New[2]
              << std::endl;
    return EXIT_FAILURE;
    }
  node1->GetNthControlPointPosition(1, pos1New);
  if (pos1New[0] != pos0[0] ||
      pos1New[1] != pos0[1] ||
      pos1New[2] != pos0[2])
    {
    std::cerr << "Swap failed, point 1 expected: "
              << pos0[0] << ", " << pos0[1] << ", " << pos0[2]
              << ", but got:"
              << pos1New[0] << ", " << pos1New[1] << ", " << pos1New[2]
              << std::endl;
    return EXIT_FAILURE;
    }

  // double check that can change a swapped point without changing the other
  // one
  pos1New[0] = pos1New[0] * 0.33;
  pos1New[1] = pos1New[1] * 100.5;
  pos1New[2] = pos1New[2] * -10.67;
  node1->SetNthControlPointPositionFromArray(1, pos1New);
  node1->GetNthControlPointPosition(0, pos0New);
  if (pos0New[0] != pos1[0] ||
      pos0New[1] != pos1[1] ||
      pos0New[2] != pos1[2])
    {
    std::cerr << "Swap failed after changing point 1, point 0 expected: "
              << pos1[0] << ", " << pos1[1] << ", " << pos1[2]
              << ", but got:"
              << pos0New[0] << ", " << pos0New[1] << ", " << pos0New[2]
              << std::endl;
    return EXIT_FAILURE;
    }

  // Check if ID returned is valid
  if (node1->GetNumberOfControlPoints() > 0)
    {
    vtkMRMLMarkupsNode::ControlPoint* markup = node1->GetNthControlPoint(0);
    const char* markupID = markup->ID.c_str();
    int markupIndex = node1->GetNthControlPointIndexByID(markupID);
    if (node1->GetNthControlPointByID(markupID) != markup)
      {
      std::cerr << "Get Markup by ID failed" << std::endl;
      return EXIT_FAILURE;
      }
    if (markupIndex != 0)
      {
      std::cerr << "Get Markup index by ID failed, returned "
                << markupIndex << ", expecting 0" << std::endl;
      return EXIT_FAILURE;
      }
    }

  // Check returned value with a nullptr ID
  vtkMRMLMarkupsNode::ControlPoint* markupNull = node1->GetNthControlPointByID(nullptr);
  int indexNull = node1->GetNthControlPointIndexByID(nullptr);
  if (markupNull)
    {
    std::cerr << "Get Markup by ID with nullptr parameters failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (indexNull >= 0)
    {
    std::cerr << "Get Markup index by ID with nullptr parameters failed" << std::endl;
    return EXIT_FAILURE;
    }

  // Check returned value with an invalid ID
  vtkMRMLMarkupsNode::ControlPoint* markupInvalid = node1->GetNthControlPointByID("Invalid");
  int indexInvalid = node1->GetNthControlPointIndexByID("Invalid");
  if (markupInvalid)
    {
    std::cerr << "Get Markup by ID with invalid ID failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (indexInvalid >= 0)
    {
    std::cerr << "Get Markup index by ID with invalid ID failed" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
