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
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkSlicerMarkupsLogic.h"

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

  // read in the scene
  int readVal = scene->Connect();
  if (readVal == 0)
    {
    std::cerr << "Failed to load scene from file "
              << fileName.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  //
  // test that we have two markup fiducials and no annotation fiducials
  //
  int numAnnotationNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode");
  int numMarkupsNodes = scene->GetNumberOfNodesByClass("vtkMRMLMarkupsFiducialNode");
  if (numAnnotationNodes != 0 ||
      numMarkupsNodes != 2)
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
  if (nodes.size() != 2)
    {
    std::cerr << "Failed to get the markup fiducial nodes from the read in scene, colleciton size = " << nodes.size() << std::endl;
    return EXIT_FAILURE;
    }

  // check the control point positions
  vtkMRMLMarkupsFiducialNode *mfnode = vtkMRMLMarkupsFiducialNode::SafeDownCast(nodes[0]);
  if (!mfnode)
    {
    std::cerr << "Failed to get first markups fiducial node" << std::endl;
    return EXIT_FAILURE;
    }
  double annotationPosition[3] = {137.232, 0.0, 85.9229};
  double markupPosition[3];
  mfnode->GetNthFiducialPosition(0, markupPosition);
  if (annotationPosition[0] != markupPosition[0] ||
      annotationPosition[1] != markupPosition[1] ||
      annotationPosition[2] != markupPosition[2])
    {
    std::cerr << "Failed to parse the first fiducial location, expected "
              << annotationPosition[0] << ","
              << annotationPosition[1] << ","
              << annotationPosition[2] << " but got "
              << markupPosition[0] << ","
              << markupPosition[1] << ","
              << markupPosition[2] << std::endl;
    return EXIT_FAILURE;
    }
  mfnode = vtkMRMLMarkupsFiducialNode::SafeDownCast(nodes[1]);
  if (!mfnode)
    {
    std::cerr << "Failed to get second markups fiducial node" << std::endl;
    return EXIT_FAILURE;
    }
  annotationPosition[0] = 92.4384;
  annotationPosition[1] = 1.0;
  annotationPosition[2] = 54.1599;
  double markupPosition2[3] = {0.0, 0.0, 0.0};
  mfnode->GetNthFiducialPosition(0, markupPosition2);
  if (annotationPosition[0] != markupPosition2[0] ||
      annotationPosition[1] != markupPosition2[1] ||
      annotationPosition[2] != markupPosition2[2])
    {
    std::cerr << "Failed to parse the second fiducial location, expected "
              << annotationPosition[0] << ","
              << annotationPosition[1] << ","
              << annotationPosition[2] << " but got "
              << markupPosition2[0] << ","
              << markupPosition2[1] << ","
              << markupPosition2[2] << std::endl;
    return EXIT_FAILURE;
    }
  //
  // check display nodes
  //
  int numAnnotationPointDisplayNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationPointDisplayNode");
  int numMarkupsDisplayNodes = scene->GetNumberOfNodesByClass("vtkMRMLMarkupsDisplayNode");
  if (numAnnotationPointDisplayNodes != 0 ||
      numMarkupsDisplayNodes != 2)
    {
    std::cerr << "Failed to translate annotation point display nodes into markup display nodes, have "
              << numAnnotationPointDisplayNodes << " annotation point display nodes and "
              << numMarkupsDisplayNodes << " mark up display nodes"
              << std::endl;
    return EXIT_FAILURE;
    }
  double inputColor1[3] = {0.862745, 0.960784, 0.0784314};
  double inputColor2[3] = {0.737255, 0.14902, 0.631373};
  vtkMRMLNode *mrmlNode1 = scene->GetNodeByID("vtkMRMLAnnotationPointDisplayNode1");
  if (!mrmlNode1)
    {
    std::cerr << "Failed to get node vtkMRMLAnnotationPointDisplayNode1" << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLMarkupsDisplayNode *markupsDisplayNode1 = vtkMRMLMarkupsDisplayNode::SafeDownCast(mrmlNode1);
  if (!markupsDisplayNode1)
    {
    std::cerr << "Failed to cast node with id vtkMRMLAnnotationPointDisplayNode1 to the expected markups display node" << std::endl;
    return EXIT_FAILURE;
    }
  double *outputColor1 = markupsDisplayNode1->GetColor();
  if (outputColor1[0] != inputColor1[0] ||
      outputColor1[1] != inputColor1[1] ||
      outputColor1[2] != inputColor1[2])
    {
    std::cerr << "Failed to get correct color on first display node, expected: "
              << inputColor1[0] << ","
              << inputColor1[1] << ","
              << inputColor1[2] << " but got "
              << outputColor1[0] << ","
              << outputColor1[1] << ","
              << outputColor1[2] << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLNode *mrmlNode2 = scene->GetNodeByID("vtkMRMLAnnotationPointDisplayNode2");
  if (!mrmlNode2)
    {
    std::cerr << "Failed to get node vtkMRMLAnnotationPointDisplayNode1" << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLMarkupsDisplayNode *markupsDisplayNode2 = vtkMRMLMarkupsDisplayNode::SafeDownCast(mrmlNode2);
  if (!markupsDisplayNode2)
    {
    std::cerr << "Failed to cast node with id vtkMRMLAnnotationPointDisplayNode2 to the expected markups display node" << std::endl;
    return EXIT_FAILURE;
    }
  double *outputColor2 = markupsDisplayNode2->GetColor();
  if (outputColor2[0] != inputColor2[0] ||
      outputColor2[1] != inputColor2[1] ||
      outputColor2[2] != inputColor2[2])
    {
    std::cerr << "Failed to get correct color on first display node, expected: "
              << inputColor2[0] << ","
              << inputColor2[1] << ","
              << inputColor2[2] << " but got "
              << outputColor2[0] << ","
              << outputColor2[1] << ","
              << outputColor2[2] << std::endl;
    return EXIT_FAILURE;
    }

  double inputGlyphScale2 = 5.0;
  double outputGlyphScale2 = markupsDisplayNode2->GetGlyphScale();
  if (inputGlyphScale2 != outputGlyphScale2)
    {
    std::cerr << "Failed to read in expected glyph scale of "
              << inputGlyphScale2 << ", got: "
              << outputGlyphScale2 << std::endl;
    return EXIT_FAILURE;
    }
  //
  // check storage nodes
  //

  return EXIT_SUCCESS;
}
