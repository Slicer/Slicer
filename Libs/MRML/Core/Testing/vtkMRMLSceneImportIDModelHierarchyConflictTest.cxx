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
#include "vtkMRMLCoreTestingUtilities.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <vtkNew.h>
#include <vtkPolyData.h>

using namespace vtkMRMLCoreTestingUtilities;

//---------------------------------------------------------------------------
bool ImportIDModelHierarchyConflictTest();
bool ImportModelHierarchyTwiceTest();

//---------------------------------------------------------------------------
int vtkMRMLSceneImportIDModelHierarchyConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  bool res = true;
  res = res && ImportIDModelHierarchyConflictTest();
  res = res && ImportModelHierarchyTwiceTest();
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
// The test makes sure the model hierarchy nodes correctly support node ID
// conflict. There are 2 steps in this test
// a) populates a scene with a model with flat hierarchy node
// b) and imports a similar scene into the existing scene.
bool ImportIDModelHierarchyConflictTest()
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

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 2)

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelNode1", modelNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelDisplayNode1", modelDisplayNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelNode->GetDisplayNode() / modelDisplayNode",
              modelNode->GetDisplayNode(),
              modelDisplayNode.GetPointer())
      )
    {
    return false;
    }

  // does the display node point to the correct polydata?
  if (modelDisplayNode->GetInputPolyData() !=
      modelNode->GetPolyData())
    {
    std::cerr << "Line " << __LINE__
              << " - Model display node and model node point to different poly data:"
              <<" \n\tmodelNode->GetPolyData(): " <<  modelNode->GetPolyData()
              << "\n\tmodelDisplayNode->GetInputPolyData(): " << modelDisplayNode->GetInputPolyData()
              << std::endl;
    return false;
    }

  // add a model hierarchy node
  vtkNew<vtkMRMLModelDisplayNode> hierachyDisplayNode;
  scene->AddNode(hierachyDisplayNode.GetPointer());

  vtkNew<vtkMRMLModelHierarchyNode> hierarchyNode;
  scene->AddNode(hierarchyNode.GetPointer());

  hierarchyNode->SetAndObserveDisplayNodeID(hierachyDisplayNode->GetID());
  hierarchyNode->SetAssociatedNodeID(modelNode->GetID());

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 4))
    {
    return false;
    }

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

  scene->Import();

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

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 8)

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelNode1", modelNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelDisplayNode1", modelDisplayNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelNode->GetDisplayNode() / modelDisplayNode",
              modelNode->GetDisplayNode(),
              modelDisplayNode.GetPointer())
      )
    {
    return false;
    }


  vtkMRMLModelNode* modelNode2 = vtkMRMLModelNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLModelNode2"));

  if (!CheckNodeIdAndName(
        __LINE__, modelNode2, "vtkMRMLModelNode2", "New Model1")

      ||!CheckNodeIdAndName(
        __LINE__, modelNode2->GetDisplayNode(),
        "vtkMRMLModelDisplayNode3", "New Display 1")
      )
    {
    return EXIT_FAILURE;
    }

  // check that the hierarchies point to the right display nodes
  vtkMRMLModelHierarchyNode *hierarchyNode2 =
      vtkMRMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelHierarchyNode2"));

  vtkMRMLModelDisplayNode* modelDisplayNode2 =
      vtkMRMLModelDisplayNode::SafeDownCast(modelNode2->GetDisplayNode());

  if (!CheckNotNull(
        __LINE__,
        "GetNodeByID(\"vtkMRMLModelHierarchyNode2\")", hierarchyNode2)

      ||!CheckString(
        __LINE__, "hierarchyNode2->GetDisplayNodeID()",
        hierarchyNode2->GetDisplayNodeID(), "vtkMRMLModelDisplayNode4")

      ||!CheckString(
        __LINE__, "hierarchyNode2->GetAssociatedNodeID()",
        hierarchyNode2->GetAssociatedNodeID(), "vtkMRMLModelNode2")
      )
    {
    return false;
    }

  // check that the model nodes and model display nodes point to the right poly data
  if (!CheckNull(
        __LINE__,
        "Import failed: new model node should have null polydata\n"
        "modelNode2->GetPolyData()",
        modelNode2->GetPolyData())

      ||!CheckNull(
        __LINE__,
        "Import failed: new model node's display node should have null polydata\n"
        "modelDisplayNode2->GetInputPolyData()",
        modelDisplayNode2->GetInputPolyData())

      ||!CheckNotNull(
        __LINE__,
        "Import failed: original model node should not have null polydata\n"
        "modelNode->GetPolyData()",
        modelNode->GetPolyData())

      ||!CheckNotNull(
        __LINE__,
        "Import failed: original model display node should not have null polydata\n"
        "modelDisplayNode->GetInputPolyData()",
        modelDisplayNode->GetInputPolyData()
        )

      ||!CheckPointer(
        __LINE__,
        "Import failed: original model node and display node don't have the same poly data\n"
        "modelNode->GetPolyData() / modelDisplayNode->GetInputPolyData()",
        modelNode->GetPolyData(),
        modelDisplayNode->GetInputPolyData()
        )
      )
    {
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool ImportModelHierarchyTwiceTest()
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

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 4)

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelNode1", modelNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelDisplayNode1", hierachyDisplayNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelHierarchyNode1", modelHierarchyNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLHierarchyNode1", hierarchyNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelHierarchyNode->GetDisplayNode() / hierachyDisplayNode",
              modelHierarchyNode->GetDisplayNode(),
              hierachyDisplayNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelHierarchyNode->GetAssociatedNode() / modelNode",
              modelHierarchyNode->GetAssociatedNode(),
              modelNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelHierarchyNode->GetParentNode() / hierarchyNode",
              modelHierarchyNode->GetParentNode(),
              hierarchyNode.GetPointer())
      )
    {
    return false;
    }

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
  scene->Import();

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

  if (!CheckInt(
        __LINE__, "GetNumberOfNodes",
        scene->GetNumberOfNodes(), 8)

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelNode1", modelNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelDisplayNode1", hierachyDisplayNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLModelHierarchyNode1", modelHierarchyNode.GetPointer())

      ||!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLHierarchyNode1", hierarchyNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelHierarchyNode->GetDisplayNode() / hierachyDisplayNode",
              modelHierarchyNode->GetDisplayNode(),
              hierachyDisplayNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelHierarchyNode->GetAssociatedNode() / modelNode",
              modelHierarchyNode->GetAssociatedNode(),
              modelNode.GetPointer())

      ||!CheckPointer(
              __LINE__, "modelHierarchyNode->GetParentNode() / hierarchyNode",
              modelHierarchyNode->GetParentNode(),
              hierarchyNode.GetPointer())
      )
    {
    return false;
    }


  vtkMRMLModelHierarchyNode* modelHierarchyNode2 =
      vtkMRMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelHierarchyNode2"));

  vtkMRMLHierarchyNode* hierarchyNode2 =
      vtkMRMLHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLHierarchyNode2"));

  if (!CheckNotNull(
        __LINE__,
        "GetNodeByID(\"vtkMRMLModelHierarchyNode2\")", modelHierarchyNode2)

      ||!CheckNotNull(
        __LINE__,
        "GetNodeByID(\"vtkMRMLHierarchyNode2\")", hierarchyNode2)

      ||!CheckPointer(
        __LINE__, "modelHierarchyNode2->GetParentNode() / hierarchyNode2",
        modelHierarchyNode2->GetParentNode(), hierarchyNode2)
      )
    {
    return false;
    }

  return true;
}
