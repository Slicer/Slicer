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
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <vtkNew.h>
#include <vtkPolyData.h>

using namespace vtkMRMLCoreTestingUtilities;

//---------------------------------------------------------------------------
int ImportIDModelHierarchyConflictTest();
int ImportModelHierarchyTwiceTest();

//---------------------------------------------------------------------------
int vtkMRMLSceneImportIDModelHierarchyConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  bool res = true;
  res = res && (ImportIDModelHierarchyConflictTest() == EXIT_SUCCESS);
  res = res && (ImportModelHierarchyTwiceTest() == EXIT_SUCCESS);
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
// The test makes sure the model hierarchy nodes correctly support node ID
// conflict. There are 2 steps in this test
// a) populates a scene with a model with flat hierarchy node
// b) and imports a similar scene into the existing scene.
int ImportIDModelHierarchyConflictTest()
{
  vtkNew<vtkMRMLScene> scene;

  // Add model node
  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

  // add poly data
  vtkNew<vtkPolyData> polyData;
  modelNode->SetAndObservePolyData(polyData.GetPointer());

  // Add display node
  vtkNew<vtkMRMLModelDisplayNode> modelDisplayNode;
  scene->AddNode(modelDisplayNode.GetPointer());
  modelNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());

  CHECK_INT(scene->GetNumberOfNodes(), 2);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelDisplayNode1", modelDisplayNode.GetPointer());
  CHECK_POINTER(modelNode->GetDisplayNode(), modelDisplayNode.GetPointer());

  // does the display node point to the correct polydata?
  CHECK_POINTER(modelDisplayNode->GetInputPolyData(), modelNode->GetPolyData());

  // add a model hierarchy node
  vtkNew<vtkMRMLModelDisplayNode> hierachyDisplayNode;
  scene->AddNode(hierachyDisplayNode.GetPointer());

  vtkNew<vtkMRMLModelHierarchyNode> hierarchyNode;
  scene->AddNode(hierarchyNode.GetPointer());

  hierarchyNode->SetAndObserveDisplayNodeID(hierachyDisplayNode->GetID());
  hierarchyNode->SetAssociatedNodeID(modelNode->GetID());

  CHECK_INT(scene->GetNumberOfNodes(), 4);

  // Note about vtkMRMLModelHierarchyNode reference role attribute names:
  //
  // For DisplayNodeID:
  //  - displayNodeID -> preferred
  //  - displayNodeRef -> deprecated
  //
  // For AssociatedNodeID:
  //  - associatedNodeRef -> preferred
  //  - modelNodeRef and modelNodeID -> deprecated

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkMRMLModelNode1  (valid polydata)
  //    |          |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode1
  //    |
  //    |---- vtkMRMLModelDisplayNode1 (valid polydata)
  //    |
  //    |---- vtkMRMLModelDisplayNode2 (null polydata)
  //    |
  //    |---- vtkMRMLModelHierarchyNode1
  //               |-- ref [displayNodeID] to vtkMRMLModelDisplayNode2
  //               |-- ref [associatedNodeRef] to vtkMRMLModelNode1


  // Here is the scene that will be imported:
  //
  //  Scene
  //    |---- vtkMRMLModelNode1  (null polydata / New Model1)
  //    |          |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode1
  //    |
  //    |---- vtkMRMLModelDisplayNode1 (null polydata / New Display 1)
  //    |
  //    |---- vtkMRMLModelDisplayNode2 (null polydata / New Display 2)
  //    |
  //    |---- vtkMRMLModelHierarchyNode1  (null polydata / vtkMRMLModelHierarchyNode1)
  //               |-- ref [displayNodeID] to vtkMRMLModelDisplayNode2
  //               |-- ref [associatedNodeRef] to vtkMRMLModelNode1


  const char scene1XML[] =
    "<MRML  version=\"18916\" userTags=\"\">"
    "  <Model id=\"vtkMRMLModelNode1\" name=\"New Model1\" displayNodeRef=\"vtkMRMLModelDisplayNode1\" ></Model>"
    "  <ModelDisplay id=\"vtkMRMLModelDisplayNode1\" name=\"New Display 1\" ></ModelDisplay>"
    "  <ModelDisplay id=\"vtkMRMLModelDisplayNode2\" name=\"New Display 2\" ></ModelDisplay>"
    "  <ModelHierarchy id=\"vtkMRMLModelHierarchyNode1\" name=\"vtkMRMLModelHierarchyNode1\"  hideFromEditors=\"true\"  selectable=\"true\" displayNodeRef=\"vtkMRMLModelDisplayNode2\"  expanded=\"true\" modelNodeRef=\"vtkMRMLModelNode1\"></ModelHierarchy>"
    "</MRML>"
    ;

  scene->SetSceneXMLString(scene1XML);
  scene->SetLoadFromXMLString(1);

  // When importing the scene, there is conflict between the existing nodes
  // and added nodes. New IDs are set by Import to the added nodes.
  // The node ids in the scene after a proper import should be

  scene->Import();  // adds Subject Hierarchy Node

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkMRMLSubjectHierarchyNode1
  //    |---- vtkMRMLModelNode1  (valid polydata)
  //    |          |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode1
  //    |
  //    |---- vtkMRMLModelDisplayNode1 (valid polydata)
  //    |
  //    |---- vtkMRMLModelDisplayNode2 (null polydata)
  //    |
  //    |---- vtkMRMLModelHierarchyNode1
  //    |          |-- ref [displayNodeID] to vtkMRMLModelDisplayNode2
  //    |          |-- ref [associatedNodeRef] to vtkMRMLModelNode1
  //    |
  //    |---- vtkMRMLModelNode2  (null polydata / New Model1)             [was vtkMRMLModelNode1]
  //    |          |-- ref [displayNodeRef] to vtkMRMLModelDisplayNode3
  //    |
  //    |---- vtkMRMLModelDisplayNode3 (null polydata / New Display 1)    [was vtkMRMLModelDisplayNode1]
  //    |
  //    |---- vtkMRMLModelDisplayNode4 (null polydata / New Display 2)    [was vtkMRMLModelDisplayNode2]
  //    |
  //    |---- vtkMRMLModelHierarchyNode2  (null polydata / vtkMRMLModelHierarchyNode1) [was vtkMRMLModelHierarchyNode1]
  //               |-- ref [displayNodeID] to vtkMRMLModelDisplayNode4
  //               |-- ref [associatedNodeRef] to vtkMRMLModelNode2

  CHECK_INT(scene->GetNumberOfNodes(), 9);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelDisplayNode1", modelDisplayNode.GetPointer());
  CHECK_POINTER(modelNode->GetDisplayNode(), modelDisplayNode.GetPointer());

  vtkMRMLModelNode* modelNode2 = vtkMRMLModelNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLModelNode2"));

  CHECK_NODE_ID_AND_NAME(modelNode2, "vtkMRMLModelNode2", "New Model1");
  CHECK_NODE_ID_AND_NAME(modelNode2->GetDisplayNode(), "vtkMRMLModelDisplayNode3", "New Display 1");

  // check that the hierarchies point to the right display nodes
  vtkMRMLModelHierarchyNode *hierarchyNode2 =
      vtkMRMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelHierarchyNode2"));

  vtkMRMLModelDisplayNode* modelDisplayNode2 =
      vtkMRMLModelDisplayNode::SafeDownCast(modelNode2->GetDisplayNode());

  CHECK_NOT_NULL(hierarchyNode2);
  CHECK_STRING(hierarchyNode2->GetDisplayNodeID(), "vtkMRMLModelDisplayNode4");
  CHECK_STRING(hierarchyNode2->GetAssociatedNodeID(), "vtkMRMLModelNode2");

  // check that the model nodes and model display nodes point to the right poly data
  CHECK_NULL(modelNode2->GetPolyData()); // new model node should have null polydata
  CHECK_NULL(modelDisplayNode2->GetInputPolyData()); // new model node's display node should have null polydata
  CHECK_NOT_NULL(modelNode->GetPolyData()); // original model node should not have null polydata
  CHECK_NOT_NULL(modelDisplayNode->GetInputPolyData()); // original model display node should not have null polydata
  CHECK_POINTER(modelNode->GetPolyData(), modelDisplayNode->GetInputPolyData()); // original model node and display node don't have the same poly data

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int ImportModelHierarchyTwiceTest()
{
  vtkNew<vtkMRMLScene> scene;

  // Add model node
  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());
  vtkNew<vtkMRMLModelDisplayNode> hierachyDisplayNode;
  scene->AddNode(hierachyDisplayNode.GetPointer());

  // Add a model hierarchy node
  vtkNew<vtkMRMLModelHierarchyNode> modelHierarchyNode;
  scene->AddNode(modelHierarchyNode.GetPointer());
  modelHierarchyNode->SetAndObserveDisplayNodeID(hierachyDisplayNode->GetID());
  modelHierarchyNode->SetAssociatedNodeID(modelNode->GetID());

  vtkNew<vtkMRMLHierarchyNode> hierarchyNode;
  scene->AddNode(hierarchyNode.GetPointer());
  modelHierarchyNode->SetParentNodeID(hierarchyNode->GetID());

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkMRMLModelNode1
  //    |
  //    |---- vtkMRMLModelDisplayNode1
  //    |
  //    |---- vtkMRMLModelHierarchyNode1
  //    |          |-- ref [displayNodeID] to vtkMRMLModelDisplayNode1
  //    |          |-- ref [associatedNodeRef] to vtkMRMLModelNode1
  //    |          |-- ref [parentNodeRef] to vtkMRMLHierarchyNode1
  //    |
  //    |---- vtkMRMLHierarchyNode1

  CHECK_INT(scene->GetNumberOfNodes(), 4);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelDisplayNode1", hierachyDisplayNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelHierarchyNode1", modelHierarchyNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLHierarchyNode1", hierarchyNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetDisplayNode(), hierachyDisplayNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetAssociatedNode(), modelNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetParentNode(), hierarchyNode.GetPointer());

  //
  // Save
  //

  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
//  std::cerr << xmlScene << std::endl;

  // Load same scene into scene
  scene->SetSceneXMLString(xmlScene);
  scene->SetLoadFromXMLString(1);
  scene->Import();  // adds Subject Hierarchy Node

  // At this point the scene should be:
  //
  //  Scene
  //    |---- vtkMRMLSubjectHierarchyNode1
  //    |---- vtkMRMLModelNode1
  //    |
  //    |---- vtkMRMLModelDisplayNode1
  //    |
  //    |---- vtkMRMLModelHierarchyNode1
  //    |          |-- ref [displayNodeID] to vtkMRMLModelDisplayNode1
  //    |          |-- ref [associatedNodeRef] to vtkMRMLModelNode1
  //    |          |-- ref [parentNodeRef] to vtkMRMLHierarchyNode1
  //    |
  //    |---- vtkMRMLHierarchyNode1
  //    |
  //    |---- vtkMRMLModelNode2                                         [was vtkMRMLModelNode1]
  //    |
  //    |---- vtkMRMLModelDisplayNode2                                  [was vtkMRMLModelDisplayNode1]
  //    |
  //    |---- vtkMRMLModelHierarchyNode2
  //    |          |-- ref [displayNodeID] to vtkMRMLModelDisplayNode2
  //    |          |-- ref [associatedNodeRef] to vtkMRMLModelNode2
  //    |          |-- ref [parentNodeRef] to vtkMRMLHierarchyNode2
  //    |
  //    |---- vtkMRMLHierarchyNode2                                     [was vtkMRMLHierarchyNode1]

  CHECK_INT(scene->GetNumberOfNodes(), 9);
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelNode1", modelNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelDisplayNode1", hierachyDisplayNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLModelHierarchyNode1", modelHierarchyNode.GetPointer());
  CHECK_NODE_IN_SCENE_BY_ID(scene.GetPointer(),"vtkMRMLHierarchyNode1", hierarchyNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetDisplayNode(), hierachyDisplayNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetAssociatedNode(), modelNode.GetPointer());
  CHECK_POINTER(modelHierarchyNode->GetParentNode(), hierarchyNode.GetPointer());

  vtkMRMLModelHierarchyNode* modelHierarchyNode2 =
      vtkMRMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelHierarchyNode2"));

  vtkMRMLHierarchyNode* hierarchyNode2 =
      vtkMRMLHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLHierarchyNode2"));

  CHECK_NOT_NULL(modelHierarchyNode2);
  CHECK_NOT_NULL(hierarchyNode2);
  CHECK_POINTER(modelHierarchyNode2->GetParentNode(), hierarchyNode2);

  return EXIT_SUCCESS;
}
