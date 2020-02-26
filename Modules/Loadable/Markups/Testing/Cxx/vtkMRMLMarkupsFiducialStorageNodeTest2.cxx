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
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"
#include "vtkPolyData.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

int vtkMRMLMarkupsFiducialStorageNodeTest2(int argc, char * argv[] )
{
  // Test reading in a Slicer3 .fcsv file
  vtkNew<vtkMRMLMarkupsFiducialStorageNode> node1;
  vtkNew<vtkMRMLMarkupsFiducialNode> markupsFiducialNode;
  vtkNew<vtkMRMLMarkupsDisplayNode> displayNode;

  // get the file name
  std::string fileName = std::string("midsag.fcsv");
  if (argc > 1)
    {
    fileName = std::string(argv[1]);
    }
  std::cout << "Using file name " << fileName.c_str() << std::endl;
  node1->SetFileName(fileName.c_str());

  // set up a scene
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  scene->AddNode(markupsFiducialNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());
  markupsFiducialNode->SetAndObserveStorageNodeID(node1->GetID());
  markupsFiducialNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  //
  // test read
  //
  std::cout << "Reading from " << node1->GetFileName() << std::endl;

  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  int retval = node1->ReadData(markupsFiducialNode.GetPointer());
  TESTING_OUTPUT_ASSERT_WARNINGS(1); // Expected warning: Have an unversioned file, assuming Slicer 3 format .fcsv
  TESTING_OUTPUT_ASSERT_WARNINGS_END();
  if (!retval)
    {
    std::cerr << "Failed to read into Markups fiducial node from Slicer3 Fiducials file " << node1->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "\nMarkup read from file = " << std::endl;
  vtkIndent indent;
  markupsFiducialNode->PrintSelf(std::cout, indent);
  std::cout << std::endl;

  // test values on the first markup
  double inputPoint[3] = {12.5, -93.5, 7.5};
  double outputPoint[3];
  markupsFiducialNode->GetNthFiducialPosition(0, outputPoint);
  double diff = fabs(outputPoint[0] - inputPoint[0]) + fabs(outputPoint[1] - inputPoint[1]) + fabs(outputPoint[2] - inputPoint[2]);
  if (diff > 0.1)
    {
    std::cerr << "After reading in, expected markup point "
              << inputPoint[0] << "," << inputPoint[1] << "," << inputPoint[2]
              << " but got a diff of " << diff << " from read in point "
              << outputPoint[0] << "," << outputPoint[1] << "," << outputPoint[2]
              << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
