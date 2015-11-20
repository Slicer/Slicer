/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRML includes
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"

// Annotations includes
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// STD includes
#include <vtkNew.h>

bool ImportTwiceTest(bool verbose);

//---------------------------------------------------------------------------
int vtkSlicerAnnotationModuleLogicImportSceneTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  bool verbose = true;
  bool res = true;
  res = ImportTwiceTest(verbose) && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
// The tests makes sure the annotation hierarchy nodes can be loaded twice.
// a) populates a scene with an annotation
// b) and imports a similar scene into the existing scene.
bool ImportTwiceTest(bool verbose)
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());

  vtkNew<vtkMRMLInteractionNode> interactionNode;
  scene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkSlicerAnnotationModuleLogic > logic;
  logic->SetMRMLScene(scene.GetPointer());

  logic->AddHierarchy();
  vtkNew<vtkMRMLAnnotationRulerNode> rnode;
  rnode->Initialize(scene.GetPointer());

  std::cout << "Starting scene has " << scene->GetNumberOfNodes() << " nodes" << std::endl;
  /// At this point the following node id should be in the scene:
  ///   - vtkMRMLAnnotationHierarchyNode
  ///       - vtkMRMLAnnotationHierarchyNode -> vtkMRMLFiducialAnnotationNode -> vtkMRMLFiducialAnnotationDisplayNode
  ///            -
  ///   - vtkMRMLModelHierarchyNode1 (pointing to vtkMRMLModelDisplayNode1 and vtkMRMLModelNode1)
  ///   - vtkMRMLModelHierarchyNode2 (parent of vtkMRMLModelHierarchyNode1)
  vtkMRMLAnnotationRulerNode* ruler1 = vtkMRMLAnnotationRulerNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLAnnotationRulerNode1"));
  double controlPoint1[3]={34,56,78};
  double controlPoint2[3]={134,156,178};
  ruler1->SetControlPointWorldCoordinates(0,controlPoint1, 0, 1);
  ruler1->SetControlPointWorldCoordinates(1,controlPoint2, 0, 1);
  vtkMRMLAnnotationHierarchyNode* rulerHierarchy1 = vtkMRMLAnnotationHierarchyNode::SafeDownCast(
    vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
      scene.GetPointer(),
      ruler1->GetID()));
  vtkMRMLAnnotationHierarchyNode* listNode1 = vtkMRMLAnnotationHierarchyNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLAnnotationHierarchyNode2"));
  vtkMRMLAnnotationHierarchyNode* allAnnotationsNode1 = vtkMRMLAnnotationHierarchyNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLAnnotationHierarchyNode1"));
  if (rulerHierarchy1->GetParentNode() != listNode1)
    {
    std::cerr << __LINE__ << ": Import failed." << std::endl;
    return false;
    }
  if (listNode1->GetParentNode() != allAnnotationsNode1)
    {
    std::cerr << __LINE__ << ": Import failed." << std::endl;
    return false;
    }

  // Save
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  if (verbose)
    {
    std::cout << xmlScene << std::endl;
    }

  // Load same scene into scene
  scene->SetSceneXMLString(xmlScene);
  scene->SetLoadFromXMLString(1);
  scene->Import();

  if (verbose)
    {
    scene->SetSaveToXMLString(1);
    scene->Commit();
    xmlScene = scene->GetSceneXMLString();
    std::cout << "Scene after import: " << std::endl;
    std::cout << xmlScene << std::endl;
    }

  vtkMRMLAnnotationRulerNode* ruler2 = vtkMRMLAnnotationRulerNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLAnnotationRulerNode2"));
  vtkMRMLAnnotationHierarchyNode* rulerHierarchy2 = vtkMRMLAnnotationHierarchyNode::SafeDownCast(
    vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(
      scene.GetPointer(),
      ruler2->GetID()));
  vtkMRMLAnnotationHierarchyNode* listNode2 = vtkMRMLAnnotationHierarchyNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLAnnotationHierarchyNode5"));
  vtkMRMLAnnotationHierarchyNode* allAnnotationsNode2 = vtkMRMLAnnotationHierarchyNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLAnnotationHierarchyNode4"));

  if (rulerHierarchy2->GetParentNode() != listNode2)
    {
    std::cerr << __LINE__ << ": Import failed." << std::endl;
    return false;
    }
  if (listNode2->GetParentNode() != allAnnotationsNode2)
    {
    std::cerr << __LINE__ << ": Import failed." << std::endl;
    return false;
    }
  return true;
}
