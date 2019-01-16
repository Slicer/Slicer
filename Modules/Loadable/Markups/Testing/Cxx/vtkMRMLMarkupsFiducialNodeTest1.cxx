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
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLScene.h"
#include "vtkURIHandler.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyData.h>

int vtkMRMLMarkupsFiducialNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLMarkupsFiducialNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  vtkMRMLMarkupsDisplayNode *dispNode = node1->GetMarkupsDisplayNode();
  std::cout << "Get MarkupsDisplayNode returned " << (dispNode ? "valid" : "null") << " pointer" << std::endl;

  // set position
  double inPos[3] = {0.33, 1.55, -99.0};
  int fidIndex = node1->AddFiducial(inPos[0], inPos[1], inPos[2]);
  double outPos[3];
  node1->GetNthFiducialPosition(0, outPos);
  double diff = sqrt(vtkMath::Distance2BetweenPoints(inPos, outPos));
  std::cout << "Diff between AddFiducial and GetNthFiducialPosition = " << diff << std::endl;
  if (diff > 0.1)
    {
    return EXIT_FAILURE;
    }
  inPos[1] = 15.55;
  int fidIndex2 = node1->AddFiducialFromArray(inPos);

  // selected
  node1->SetNthFiducialSelected(fidIndex2, false);
  bool retval = node1->GetNthFiducialSelected(fidIndex2);
  if (retval != false)
    {
    std::cerr << "Error setting/getting selected to false on fid " << fidIndex << std::endl;
    return EXIT_FAILURE;
    }

  // visibility
  node1->SetNthFiducialVisibility(fidIndex2, false);
  retval = node1->GetNthFiducialVisibility(fidIndex2);
  if (retval != false)
    {
    std::cerr << "Error setting/getting visibility to false on fid " << fidIndex << std::endl;
    return EXIT_FAILURE;
    }

  // label
  node1->SetNthFiducialLabel(fidIndex2, std::string("TestingLabelHere"));
  std::string returnLabel = node1->GetNthFiducialLabel(fidIndex2);
  if (returnLabel.compare("TestingLabelHere") != 0)
    {
    std::cerr << "Failure to set/get label for fid " << fidIndex2 << ", got '" << returnLabel.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }

  // associated node id
  std::string inID = "vtkMRMLScalarVolumeNode21";
  node1->SetNthFiducialAssociatedNodeID(fidIndex2, inID.c_str());
  std::string outID = node1->GetNthFiducialAssociatedNodeID(fidIndex2);
  if (outID.compare(inID) != 0)
    {
    std::cerr << "Failed to set fid " << fidIndex2 << " assoc node id to " << inID.c_str() << ", got '" << outID << "'" << std::endl;
    return EXIT_FAILURE;
    }

  // world coords
  double inCoords[4] = {0.4, 10.5, -8, 1.0};
  node1->SetNthFiducialWorldCoordinates(fidIndex2, inCoords);
  double outCoords[4];
  node1->GetNthFiducialWorldCoordinates(fidIndex2, outCoords);
  diff = sqrt(vtkMath::Distance2BetweenPoints(inCoords, outCoords));
  std::cout << "Diff between set world and get world coords = " << diff << std::endl;
  if (diff > 0.1)
    {
    return EXIT_FAILURE;
    }

  // position as array
  double p0[3];
  p0[0] = 0.99;
  p0[1] = 1.33;
  p0[2] = -9.0;
  node1->SetNthFiducialPositionFromArray(fidIndex2, p0);
  double p1[3];
  node1->GetNthFiducialPosition(fidIndex2, p1);
  diff = sqrt(vtkMath::Distance2BetweenPoints(p0, p1));
  std::cout << "Diff between set nth fiducial position array and get = " << diff << std::endl;
  if (diff > 0.1)
    {
    return EXIT_FAILURE;
    }
  // position as points
  p0[1] = -4.5;
  node1->SetNthFiducialPosition(fidIndex2, p0[0], p0[1], p0[2]);
  node1->GetNthFiducialPosition(fidIndex2, p1);
  diff = sqrt(vtkMath::Distance2BetweenPoints(p0, p1));
  std::cout << "Diff between set nth fiducial position and get = " << diff << std::endl;
  if (diff > 0.1)
    {
    return EXIT_FAILURE;
    }

  node1->Print(std::cout);

  return EXIT_SUCCESS;
}
