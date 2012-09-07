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
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <vtkNew.h>
#include <vtkPolyData.h>

//---------------------------------------------------------------------------
int vtkMRMLSceneImportIDModelHierarchyConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  vtkNew<vtkMRMLScene> scene;

  // Add model node
  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

  // add poly data
  vtkNew<vtkPolyData> polyData;
  modelNode.GetPointer()->SetAndObservePolyData(polyData.GetPointer());
  std::cout << "Polydata pointer = " << polyData.GetPointer() << std::endl;

  // Add display node
  vtkNew<vtkMRMLModelDisplayNode> modelDisplayNode;
  scene->AddNode(modelDisplayNode.GetPointer());
  modelNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());

  if (scene->GetNumberOfNodes() != 2 ||
      (strcmp("vtkMRMLModelNode1", modelNode->GetID()) != 0) ||
      (strcmp("vtkMRMLModelDisplayNode1", modelDisplayNode->GetID()) != 0) ||
      modelNode->GetDisplayNode() != modelDisplayNode.GetPointer())
    {
    std::cerr << "Failed to add node into the scene: "
              << "model id: " << modelNode->GetID()<< " "
              << "model display id: " << modelDisplayNode->GetID()<< " "
              << "display node: " << modelNode.GetPointer()->GetDisplayNode()
              << std::endl;
    return EXIT_FAILURE;
    }
  // does the display node point to the correct polydata?
  std::cout<< "Model display node poly data pointer = "
           << modelDisplayNode.GetPointer()->GetInputPolyData() << std::endl;
  if (modelDisplayNode.GetPointer()->GetInputPolyData() !=
      modelNode.GetPointer()->GetPolyData())
    {
    std::cerr << "Model display node and model node point to different poly data:"
              <<" model poly data = " <<  modelNode.GetPointer()->GetPolyData()
              << ", display node polydata = " << modelDisplayNode.GetPointer()->GetInputPolyData()
              << std::endl;
    return EXIT_FAILURE;
    }

  // add a model hierarchy node
  vtkNew<vtkMRMLModelDisplayNode> hierachyDisplayNode;
  scene->AddNode(hierachyDisplayNode.GetPointer());
  vtkNew<vtkMRMLModelHierarchyNode> hierarchyNode;
  scene->AddNode(hierarchyNode.GetPointer());
  hierarchyNode->SetAndObserveDisplayNodeID(hierachyDisplayNode.GetPointer()->GetID());
  hierarchyNode->SetAssociatedNodeID(modelNode.GetPointer()->GetID());

  std::cout << "Starting scene has " << scene->GetNumberOfNodes() << " nodes" << std::endl;
  /// at this point the following node id should be in the scene
  /// vtkMRMLModelNode1 with valid polydata
  /// vtkMRMLModelDisplayNode1 with valid polydata from vtkMRMLModelNode1
  /// vtkMRMLModelDisplayNode2 with null polydata
  /// vtkMRMLModelHierarchyNode1 (pointing to vtkMRMLModelDisplayNode2 and vtkMRMLModelNode1)

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
  /// vtkMRMLModelNode1 with valid polydata
  /// vtkMRMLModelDisplayNode1 with valid polydata from vtkMRMLModelNode1
  /// vtkMRMLModelDisplayNode2 with null polydata
  /// vtkMRMLModelHierarchyNode1 (pointing to vtkMRMLModelDisplayNode2 and vtkMRMLModelNode1)
  /// vtkMRMLModelNodel2 (imported scene vtkMRMLModelNode1, pointing to vtkMRMLModelDisplayNode3, null polydata)
  /// vtkMRMLModelDisplayNode3 (imported scene vtkMRMLModelDisplayNode1, null polydata)
  /// vtkMRMLModelDisplayNode4 (imported scene vtkMRMLModelDisplayNode2, null polydata)
  /// vtkMRMLModelHierarchyNode2 (imported scene vtkMRMLModelHierarchyNode1,pointing to vtkMRMLModelDisplayNode4, vtkMRMLModelNodel2)

  scene->Import();

  if (scene->GetNumberOfNodes() != 8 ||
      scene->GetNodeByID("vtkMRMLModelNode1") != modelNode.GetPointer() ||
      scene->GetNodeByID("vtkMRMLModelDisplayNode1") != modelDisplayNode.GetPointer() ||
      modelNode->GetDisplayNode() != modelDisplayNode.GetPointer())
    {
    std::cerr << "Failed to import scene "
              << "number of nodes: " << scene->GetNumberOfNodes()
              << ", scene model node1 "  << scene->GetNodeByID("vtkMRMLModelNode1")
              << " != original model node" <<  modelNode.GetPointer()
              << " or scene model display node 1 " << scene->GetNodeByID("vtkMRMLModelDisplayNode1")
              << " != original model dipslay node " << modelDisplayNode.GetPointer()
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLModelNode* modelNode2 = vtkMRMLModelNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLModelNode2"));
  if (modelNode2 == 0 ||
      strcmp(modelNode2->GetID(), "vtkMRMLModelNode2") != 0 ||
      strcmp(modelNode2->GetName(), "New Model1") != 0 ||
      modelNode2->GetDisplayNode() == 0 ||
      strcmp(modelNode2->GetDisplayNode()->GetID(), "vtkMRMLModelDisplayNode3") != 0 ||
      strcmp(modelNode2->GetDisplayNode()->GetName(), "New Display 1") != 0)
    {
    std::cerr << "Failed to import scene: "
              << "model #2: " << modelNode2
              << ", model node 2 display node: " << modelNode2->GetDisplayNode();
    if (modelNode2)
      {
      std::cerr << " model 2 id " << modelNode2->GetID()
                << " != vtkMRMLModelNode2";
      if (modelNode2->GetDisplayNode())
          {
          std::cerr << " model node 2's display node id " << modelNode2->GetDisplayNode()->GetID()
                    << " !=  vtkMRMLModelDisplayNode3";
          }
      }
    std::cerr << std::endl;
    return EXIT_FAILURE;
    }

  // check that the hierarchies point to the right display nodes
  vtkMRMLModelHierarchyNode *hierarchyNode2 = vtkMRMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelHierarchyNode2"));
  if (!hierarchyNode2)
    {
    std::cerr << "Import failure: imported hierarchy node is not using id vtkMRMLModelHierarchyNode2" << std::endl;
    return EXIT_FAILURE;
    }
  if (!hierarchyNode2->GetDisplayNodeID())
    {
    std::cerr << "Import failure: imported hierarchy node does not have a display node" << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(hierarchyNode2->GetDisplayNodeID(), "vtkMRMLModelDisplayNode4") != 0)
    {
    std::cerr << "Import failure: new model hierarchy should point to new display node with id vtkMRMLModelDisplayNode4, instead points to "
              << hierarchyNode2->GetDisplayNodeID() << std::endl;
    return EXIT_FAILURE;
    }
  // check that the hierarchies point to the right model nodes
  if (!hierarchyNode2->GetAssociatedNodeID())
    {
    std::cerr << "Import failure: imported hierarchy node does not have an associated node" << std::endl;
    return EXIT_FAILURE;
    }
  if (strcmp(hierarchyNode2->GetAssociatedNodeID(), "vtkMRMLModelNode2") != 0)
    {
    std::cerr << "Import failure: new model hierarchy shoudl point to new model node with id vtkMRMLModelNode2, instead points to "
              << hierarchyNode2->GetAssociatedNodeID() << std::endl;
    return EXIT_FAILURE;
    }

  // check that the model nodes and model display nodes point to the right poly data
  if (modelNode2->GetPolyData() != NULL)
    {
    std::cerr << "Import failed: new model node should have null polydata: "
              << modelNode2->GetPolyData()
              << std::endl;
    }
  if (vtkMRMLModelDisplayNode::SafeDownCast(modelNode2->GetDisplayNode())
      ->GetInputPolyData() != NULL)
    {
    std::cerr << "Import failed: new model node's display node should have null polydata: "
              << vtkMRMLModelDisplayNode::SafeDownCast(modelNode2->GetDisplayNode())
                   ->GetInputPolyData()
              << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Imported model poly data and display node poly data are properly null" << std::endl;

  if (modelNode->GetPolyData() == NULL)
    {
    std::cerr << "Import failed: original model node should not have null polydata"
              << std::endl;
    return EXIT_FAILURE;
    }
  if (vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode())->GetInputPolyData() == NULL)
    {
    std::cerr << "Import failed: original model display node should not have null polydata"
              << std::endl;
    return EXIT_FAILURE;
    }
  if (modelNode->GetPolyData() != vtkMRMLModelDisplayNode::SafeDownCast(
        modelNode->GetDisplayNode())->GetInputPolyData())
    {
    std::cerr << "Import failed: original model node and display node don't have the same poly data"
              << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
