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

// Markups includes
#include "vtkMRMLMarkupsFiducialDisplayNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"
#include "vtkSlicerMarkupsLogic.h"
#include "vtkSlicerSceneViewsModuleLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include "vtkPolyData.h"

int vtkMarkupsAnnotationSceneTest(int argc, char * argv[] )
{
  // Test reading in a Slicer4 MRML scene with legacy annotation nodes (created using Slicer-4.1.1):
  //
  // --- All Annotations
  //  |--- Fiducials List
  //  | |--- F
  //  | |--- F_1
  //  | |--- F_2
  //  |--- Ruler list
  //  | |--- M
  //  | |--- M_1
  //  |--- ROI list
  //  | |--- R
  //  | |--- R_1
  //  | |--- R_2
  //  |--- List
  //    |--- F_4
  //    |--- F_3
  //

  // get the file name
  std::string fileName;
  if (argc > 1)
    {
    fileName = std::string(argv[1]);
    }
  else
    {
    std::cerr << "No mrml scene file name given!" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Using scene file name " << fileName.c_str() << std::endl;

  // set up the scene
  vtkNew<vtkMRMLScene> scene;
  scene->SetURL(fileName.c_str());

  // logic - handle registration of nodes
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene.GetPointer());
  vtkNew<vtkSlicerMarkupsLogic> markupsLogic;
  markupsLogic->SetMRMLScene(scene.GetPointer());
  vtkNew<vtkSlicerAnnotationModuleLogic> annotationLogic;
  annotationLogic->SetMRMLScene(scene.GetPointer());
  vtkNew<vtkSlicerSceneViewsModuleLogic> sceneViewsModuleLogic;
  sceneViewsModuleLogic->SetMRMLScene(scene.GetPointer());

  // read in the scene
  std::cerr << "Reading scene from file: " << fileName.c_str() << std::endl;

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();
  bool readSuccess = (scene->Connect() != 0);
  TESTING_OUTPUT_ASSERT_ERRORS(0);
  TESTING_OUTPUT_ASSERT_WARNINGS(2);  // warning about view tag/layout name, because we read an old scene
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();

  CHECK_BOOL(readSuccess, true);

  // Check that all annotation nodes are converted
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode"), 0);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLAnnotationROINode"), 0);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLAnnotationRulerNode"), 0);

  // Check if markups node are created
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLMarkupsFiducialNode"), 2);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLMarkupsROINode"), 3);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLMarkupsLineNode"), 2);

  // check the control point positions
  vtkMRMLMarkupsFiducialNode *mfnode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->GetFirstNodeByName("Fiducials List"));
  CHECK_NOT_NULL(mfnode);

  double expectedPosition[3][3] =
    {
    { 137.232, 0.0, 85.9229},
    { 92.4384, 1.0, 54.1599 },
    { 86.039, 89.2857, 0.0 }
    };

  for (int i = 0; i < 3; i++)
    {
    double actualPosition[3] = { 0.0, 0.0, 0.0 };
    mfnode->GetNthControlPointPosition(i, actualPosition);
    double diff = vtkMath::Distance2BetweenPoints(expectedPosition[i], actualPosition);
    if (diff > 0.01)
      {
      std::cerr << "Failed to parse the first fiducial location, expected: "
        << expectedPosition[i][0] << ","
        << expectedPosition[i][1] << ","
        << expectedPosition[i][2] << ","
        << "\nbut got : "
        << actualPosition[0] << ","
        << actualPosition[1] << ","
        << actualPosition[2]
        << "\n\tdiff = " << diff << std::endl;
      return EXIT_FAILURE;
      }
  }

  //
  // check display nodes
  //
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLAnnotationPointDisplayNode"), 0);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLAnnotationLineDisplayNode"), 0);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLAnnotationTextDisplayNode"), 0);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLMarkupsFiducialDisplayNode"), 2);

  vtkMRMLMarkupsFiducialDisplayNode *markupsFiducialDisplayNode1 = vtkMRMLMarkupsFiducialDisplayNode::SafeDownCast(mfnode->GetDisplayNode());
  CHECK_NOT_NULL(markupsFiducialDisplayNode1);

  double* color = markupsFiducialDisplayNode1->GetColor();
  double expectedColor[3] = { 0.862745, 0.960784, 0.0784314 };
  double diff = vtkMath::Distance2BetweenPoints(color, expectedColor);
  if (diff > 0.01)
    {
    std::cerr << "Failed to get color on first display node, expected: "
              << expectedColor[0] << ","
              << expectedColor[1] << ","
              << expectedColor[2] << " but got "
              << color[0] << ","
              << color[1] << ","
              << color[2] << ", diff = " << diff << std::endl;
    return EXIT_FAILURE;
    }

  double* selectedColor = markupsFiducialDisplayNode1->GetSelectedColor();
  double expectedSelectedColor[3] = { 0.2667, 0.6745, 0.3922 };
  diff = vtkMath::Distance2BetweenPoints(selectedColor, expectedSelectedColor);
  if (diff > 0.01)
    {
    std::cerr << "Failed to get selected color on first display node, expected: "
      << expectedSelectedColor[0] << ","
      << expectedSelectedColor[1] << ","
      << expectedSelectedColor[2] << " but got "
      << selectedColor[0] << ","
      << selectedColor[1] << ","
      << selectedColor[2] << ", diff = " << diff << std::endl;
    return EXIT_FAILURE;
    }

  //
  // check storage nodes
  //
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLAnnotationStorageNode"), 0);

  return EXIT_SUCCESS;
}
