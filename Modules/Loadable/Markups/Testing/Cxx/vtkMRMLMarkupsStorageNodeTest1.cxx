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
#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"
#include "vtkPolyData.h"

// VTK includes
#include <vtkNew.h>


int vtkMRMLMarkupsStorageNodeTest1(int vtkNotUsed(argc), char * vtkNotUsed(argv)[] )
{
  vtkNew<vtkMRMLMarkupsStorageNode> node1;

  EXERCISE_BASIC_STORAGE_MRML_METHODS( vtkMRMLMarkupsStorageNode, node1 );

  /// Test coordinate system, use valid range as this macro will fail if the CoordinateSystem var isn't set to the passed invalid values
  TEST_SET_GET_INT_RANGE(node1, CoordinateSystem, 1, 1);

  // manually test invalid values
  node1->SetCoordinateSystem(-1);
  if (node1->GetCoordinateSystem() == -1)
    {
    std::cerr << "Failed to detect invalid coordinate system of -1" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetCoordinateSystem(100);
  if (node1->GetCoordinateSystem() == 100)
    {
    std::cerr << "Failed to detect invalid coordinate system of 100" << std::endl;
    return EXIT_FAILURE;
    }

  // test valid values
  node1->UseLPSOn();
  if (node1->GetCoordinateSystemAsString().compare("LPS") != 0)
    {
    std::cerr << "Failed to set coordinate system to LPS "
              << vtkMRMLMarkupsStorageNode::LPS
              << ", int flag = " << node1->GetCoordinateSystem()
              << ", string = " << node1->GetCoordinateSystemAsString().c_str()
              << std::endl;
    return EXIT_FAILURE;
    }

  node1->UseIJKOn();
  if (node1->GetCoordinateSystemAsString().compare("IJK") != 0)
    {
    std::cerr << "Failed to set coordinate system to IJK "
              << vtkMRMLMarkupsStorageNode::IJK
              << ", int flag = " << node1->GetCoordinateSystem()
              << ", string = " << node1->GetCoordinateSystemAsString().c_str()
              << std::endl;
    return EXIT_FAILURE;
    }

  // reset coordinate system flag to RAS
  node1->UseRASOn();
 if (node1->GetCoordinateSystemAsString().compare("RAS") != 0)
    {
    std::cerr << "Failed to set coordinate system to RAS "
              << vtkMRMLMarkupsStorageNode::RAS
              << ", int flag = " << node1->GetCoordinateSystem()
              << ", string = " << node1->GetCoordinateSystemAsString().c_str()
              << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
