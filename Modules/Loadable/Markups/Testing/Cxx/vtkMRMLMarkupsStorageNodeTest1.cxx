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
#include "vtkMRMLMarkupsLineNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"
#include "vtkPolyData.h"

// VTK includes
#include <vtkNew.h>

int vtkMRMLMarkupsStorageNodeTest1(int vtkNotUsed(argc), char * vtkNotUsed(argv)[] )
{
  vtkNew<vtkMRMLMarkupsStorageNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_INT(node1, CoordinateSystem, 0);
  TEST_SET_GET_INT(node1, CoordinateSystem, 1);

  // manually test invalid values
  int invalidValues[3] = { -1, 2, 100 };
  for (int i = 0; i < 3; i++)
    {
    node1->SetCoordinateSystem(invalidValues[i]);
    if (node1->GetCoordinateSystem() == invalidValues[i])
      {
      std::cerr << "Failed to detect invalid coordinate system of -1" << std::endl;
      return EXIT_FAILURE;
      }
    }

  // test valid values
  node1->UseLPSOn();
  CHECK_STD_STRING(node1->GetCoordinateSystemAsString(), "LPS");

  // reset coordinate system flag to RAS
  node1->UseRASOn();
  CHECK_STD_STRING(node1->GetCoordinateSystemAsString(), "RAS");

  return EXIT_SUCCESS;
}
