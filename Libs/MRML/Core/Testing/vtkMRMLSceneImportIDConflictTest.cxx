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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRML includes
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <vtkNew.h>

//---------------------------------------------------------------------------
int vtkMRMLSceneImportIDConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  vtkNew<vtkMRMLScene> scene;

  // Add displayable node
  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

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
              << "display node: " << modelNode->GetDisplayNode()
              << std::endl;
    return EXIT_FAILURE;
    }

  const char scene1XML[] =
    "<MRML  version=\"18916\" userTags=\"\">"
    "  <Model id=\"vtkMRMLModelNode1\" name=\"New Model1\" displayNodeRef=\"vtkMRMLModelDisplayNode1\" ></Model>"
    "  <ModelDisplay id=\"vtkMRMLModelDisplayNode1\" name=\"New Display 1\" ></ModelDisplay>"
    "  <ModelDisplay id=\"vtkMRMLModelDisplayNode2\" name=\"New Display 2\" ></ModelDisplay>"
    "  <Model id=\"vtkMRMLModelNode2\" name=\"New Model2\" displayNodeRef=\"vtkMRMLModelDisplayNode2\" ></Model>"
    "</MRML>"
    ;

  scene->SetSceneXMLString(scene1XML);
  scene->SetLoadFromXMLString(1);
  // When importing the scene, there is conflict between the existing nodes
  // and added nodes. New IDs are set by Import to the added nodes.
  scene->Import();
  vtkMRMLModelNode* modelNode2 = vtkMRMLModelNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLModelNode3"));

  if (scene->GetNumberOfNodes() != 6 ||
      scene->GetNodeByID("vtkMRMLModelNode1") != modelNode.GetPointer() ||
      scene->GetNodeByID("vtkMRMLModelDisplayNode1") != modelDisplayNode.GetPointer() ||
      modelNode->GetDisplayNode() != modelDisplayNode.GetPointer() ||
      modelNode2 == 0 ||
      strcmp(modelNode2->GetID(), "vtkMRMLModelNode3") != 0 ||
      strcmp(modelNode2->GetName(), "New Model1") != 0 ||
      modelNode2->GetDisplayNode() == 0 ||
      strcmp(modelNode2->GetDisplayNode()->GetID(), "vtkMRMLModelDisplayNode3") != 0 ||
      strcmp(modelNode2->GetDisplayNode()->GetName(), "New Display 1") != 0)
    {
    std::cerr << "Failed to import scene: "
              << "number of nodes: " << scene->GetNumberOfNodes() << std::endl
              << "model #2: " << modelNode2  << std::endl;
    return EXIT_FAILURE;
    }
  vtkMRMLModelNode* modelNode3 = vtkMRMLModelNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLModelNode2"));
  if (modelNode3 == 0 ||
      strcmp(modelNode3->GetID(), "vtkMRMLModelNode2") != 0 ||
      strcmp(modelNode3->GetName(), "New Model2") != 0 ||
      modelNode3->GetDisplayNode() == 0 ||
      strcmp(modelNode3->GetDisplayNode()->GetID(), "vtkMRMLModelDisplayNode2") != 0 ||
      strcmp(modelNode3->GetDisplayNode()->GetName(), "New Display 2") != 0)
    {
    std::cerr << "Failed to import scene - part2: "
              << "model #3: " << modelNode3 << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
