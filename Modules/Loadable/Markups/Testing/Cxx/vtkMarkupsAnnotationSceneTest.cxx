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
#include "vtkSlicerMarkupsLogic.h"

// Annotation includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"


// VTK includes
#include <vtkNew.h>
#include "vtkPolyData.h"

int vtkMarkupsAnnotationSceneTest(int argc, char * argv[] )
{
  // Test reading in a Slicer4 MRML scene with two Annotation fiducial points

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

  // read in the scene
  int readVal = scene->Connect();
  if (readVal == 0)
    {
    std::cerr << "Failed to load scene from file "
              << fileName.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  // save annotation node values
  double annotationPosition1[3];
  double annotationPosition2[3];
  // the first fiducial display node color and glyph size ar
  // used for the markups list
  double inputColor1[3];
  double inputGlyphScale1;

  std::vector<vtkMRMLNode *> annotationNodes;
  scene->GetNodesByClass("vtkMRMLAnnotationFiducialNode", annotationNodes);
  if (annotationNodes.size() < 1)
    {
    std::cerr << "Failed to get annotation fiducial nodes from read in scene, "
              << "collection size = " << annotationNodes.size() << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLAnnotationFiducialNode *annotNode1 = vtkMRMLAnnotationFiducialNode::SafeDownCast(annotationNodes[0]);
  if (!annotNode1)
    {
    std::cerr << "Failed to get first annotation fiducial node" << std::endl;
    return EXIT_FAILURE;
    }
  annotNode1->GetFiducialCoordinates(annotationPosition1);
  vtkMRMLAnnotationFiducialNode *annotNode2 = vtkMRMLAnnotationFiducialNode::SafeDownCast(annotationNodes[1]);
  if (!annotNode2)
    {
    std::cerr << "Failed to get second annotation fiducial node" << std::endl;
    return EXIT_FAILURE;
    }
  annotNode2->GetFiducialCoordinates(annotationPosition2);

  std::vector<vtkMRMLNode *> annotationDisplayNodes;
  scene->GetNodesByClass("vtkMRMLAnnotationPointDisplayNode", annotationDisplayNodes);
  if (annotationDisplayNodes.size() < 1)
    {
    std::cerr << "Failed to get annotation point display nodes from read in scene, "
              << "collection size = " << annotationDisplayNodes.size() << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = vtkMRMLAnnotationPointDisplayNode::SafeDownCast(annotationDisplayNodes[0]);
  if (!pointDisplayNode)
    {
    std::cerr << "Failed to get annotation point display node" << std::endl;
    return EXIT_FAILURE;
    }
  pointDisplayNode->GetColor(inputColor1);
  inputGlyphScale1 = pointDisplayNode->GetGlyphScale();
  //
  // convert
  //
  markupsLogic->ConvertAnnotationFiducialsToMarkups();

  //
  // test that we have 1 markup fiducial list and no annotation fiducials
  //
  int numAnnotationNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode");
  int numMarkupsNodes = scene->GetNumberOfNodesByClass("vtkMRMLMarkupsFiducialNode");
  if (numAnnotationNodes != 0 ||
      numMarkupsNodes != 1)
    {
    std::cerr << "Failed to translate annotation fiducial nodes "
              << " into markup fiducial nodes, still have "
              << numAnnotationNodes
              << " annotation fiducial nodes in the scene, with "
              << numMarkupsNodes << " markups fiducial nodes." << std::endl;
    return EXIT_FAILURE;
    }
  std::vector<vtkMRMLNode *> nodes;
  scene->GetNodesByClass("vtkMRMLMarkupsFiducialNode", nodes);
  if (nodes.size() != 1)
    {
    std::cerr << "Failed to get the markup fiducial nodes from the read in scene, collection size = " << nodes.size() << std::endl;
    return EXIT_FAILURE;
    }

  // check the control point positions
  vtkMRMLMarkupsFiducialNode *mfnode = vtkMRMLMarkupsFiducialNode::SafeDownCast(nodes[0]);
  if (!mfnode)
    {
    std::cerr << "Failed to get first markups fiducial node" << std::endl;
    return EXIT_FAILURE;
    }

  double markupPosition[3] = {0.0, 0.0, 0.0};
  mfnode->GetNthFiducialPosition(0, markupPosition);
  double diff = vtkMath::Distance2BetweenPoints(annotationPosition1, markupPosition);
  if (diff > 0.01)
    {
    std::cerr << "Failed to parse the first fiducial location, expected: "
              << annotationPosition1[0] << ","
              << annotationPosition1[1] << ","
              << annotationPosition1[2] << "\nbut got: "
              << markupPosition[0] << ","
              << markupPosition[1] << ","
              << markupPosition[2] << "\n\tdiff = " << diff << std::endl;
    return EXIT_FAILURE;
    }

  double markupPosition2[3] = {0.0, 0.0, 0.0};
  mfnode->GetNthFiducialPosition(1, markupPosition2);
  diff = vtkMath::Distance2BetweenPoints(annotationPosition2, markupPosition2);
  if (diff > 0.01)
    {
    std::cerr << "Failed to parse the second fiducial location, expected "
              << annotationPosition2[0] << ","
              << annotationPosition2[1] << ","
              << annotationPosition2[2] << " but got "
              << markupPosition2[0] << ","
              << markupPosition2[1] << ","
              << markupPosition2[2] << "\n\tdiff = " << diff << std::endl;
    return EXIT_FAILURE;
    }
  //
  // check display nodes
  //
  int numAnnotationPointDisplayNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationPointDisplayNode");
  int numMarkupsFiducialDisplayNodes = scene->GetNumberOfNodesByClass("vtkMRMLMarkupsFiducialDisplayNode");
  if (numAnnotationPointDisplayNodes != 0 ||
      numMarkupsFiducialDisplayNodes != 1)
    {
    std::cerr << "Failed to translate annotation point display nodes into markup display nodes, have "
              << numAnnotationPointDisplayNodes << " annotation point display nodes and "
              << numMarkupsFiducialDisplayNodes << " markup fiducial display nodes"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLNode *mrmlNode1 = scene->GetNodeByID("vtkMRMLMarkupsFiducialDisplayNode1");
  if (!mrmlNode1)
    {
    std::cerr << "Failed to get node vtkMRMLMarkupsFiducialDisplayNode1" << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLMarkupsFiducialDisplayNode *markupsFiducialDisplayNode1 = vtkMRMLMarkupsFiducialDisplayNode::SafeDownCast(mrmlNode1);
  if (!markupsFiducialDisplayNode1)
    {
    std::cerr << "Failed to cast node with id vtkMRMLMarkupsFiducialDisplayNode1 to the expected markups fiducial display node" << std::endl;
    return EXIT_FAILURE;
    }
  double *outputColor1 = markupsFiducialDisplayNode1->GetColor();
  diff = vtkMath::Distance2BetweenPoints(inputColor1, outputColor1);
  if (diff > 0.01)
    {
    std::cerr << "Failed to get correct color on first display node, expected: "
              << inputColor1[0] << ","
              << inputColor1[1] << ","
              << inputColor1[2] << " but got "
              << outputColor1[0] << ","
              << outputColor1[1] << ","
              << outputColor1[2] << ", diff = " << diff << std::endl;
    return EXIT_FAILURE;
    }

  double outputGlyphScale1 = markupsFiducialDisplayNode1->GetGlyphScale();
  if (inputGlyphScale1 != outputGlyphScale1)
    {
    std::cerr << "Failed to read in expected glyph scale of "
              << inputGlyphScale1 << ", got: "
              << outputGlyphScale1 << std::endl;
    return EXIT_FAILURE;
    }

  //
  // check storage nodes
  //
  int numAnnotationStorageNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationStorageNode");
  if (numAnnotationStorageNodes != 0)
    {
    std::cerr << "Failed to convert and delete annotation storage nodes, still have "
              << numAnnotationStorageNodes << " in the scene." << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
