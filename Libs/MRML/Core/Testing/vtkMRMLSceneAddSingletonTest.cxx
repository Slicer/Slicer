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
#include "vtkMRMLModelNode.h"
#include "vtkMRMLParser.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneViewNode.h"

// STD includes
#include <vtkNew.h>

//---------------------------------------------------------------------------
int vtkMRMLSceneAddSingletonTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  vtkNew<vtkMRMLScene> scene;

  // Add singleton node
  vtkNew<vtkMRMLModelNode> singleton1;
  singleton1->SetSingletonTag("Singleton");
  vtkMRMLNode* addedNode = scene->AddNode(singleton1.GetPointer());

  if (addedNode != singleton1.GetPointer() ||
      singleton1->GetScene() != scene.GetPointer() ||
      (strcmp("vtkMRMLModelNodeSingleton", singleton1->GetID()) != 0) ||
      (strcmp("Model", singleton1->GetName()) != 0) ||
      scene->GetNumberOfNodes() != 1 ||
      scene->GetNodeByID("vtkMRMLModelNodeSingleton") != singleton1.GetPointer())
    {
    std::cerr << "Failed to add singleton into the scene: "
              << "node: " << singleton1.GetPointer() << " "
              << "added: " << addedNode << " "
              << "scene: " << singleton1->GetScene() << " "
              << "id: " << singleton1->GetID() << " "
              << "name: " << singleton1->GetName() << " "
              << "#nodes: " << scene->GetNumberOfNodes() << " "
              << scene->GetNodeByID("vtkMRMLSliceNodeSingleton") << std::endl;
    return EXIT_FAILURE;
    }

  // AddNode of a new singleton shouldn't add an instance but copy tthe node
  // in the existing singleton
  vtkNew<vtkMRMLModelNode> singleton1Copy;
  singleton1Copy->SetSingletonTag("Singleton");
  addedNode = scene->AddNode(singleton1Copy.GetPointer());

  if (addedNode != singleton1.GetPointer() ||
      singleton1Copy->GetScene() != 0 ||
      singleton1Copy->GetID() != 0 ||
      singleton1Copy->GetName() != 0 ||
      (strcmp("vtkMRMLModelNodeSingleton", singleton1->GetID()) != 0) ||
      (strcmp("Model", singleton1->GetName()) != 0) ||
      scene->GetNumberOfNodes() != 1 ||
      scene->GetNodeByID("vtkMRMLModelNodeSingleton") != singleton1.GetPointer())
    {
    std::cerr << "Failed to add singleton into the scene: "
              << "node: " << singleton1.GetPointer() << " "
              << "added: " << addedNode << " "
              << "scene: " << singleton1->GetScene() << " "
              << "id: " << singleton1->GetID() << " "
              << "name: " << singleton1->GetName() << " "
              << "#nodes: " << scene->GetNumberOfNodes() << " "
              << scene->GetNodeByID("vtkMRMLSliceNodeSingleton");
    return EXIT_FAILURE;
    }

  // Add a different singleton.
  vtkNew<vtkMRMLModelNode> singleton2;
  singleton2->SetSingletonTag("Singleton2");
  addedNode = scene->AddNode(singleton2.GetPointer());

  if (addedNode != singleton2.GetPointer() ||
      singleton2->GetScene() != scene.GetPointer() ||
      (strcmp("vtkMRMLModelNodeSingleton2", singleton2->GetID()) != 0) ||
      (strcmp("Model_1", singleton2->GetName()) != 0) ||
      scene->GetNumberOfNodes() != 2 ||
      scene->GetNodeByID("vtkMRMLModelNodeSingleton2") != singleton2.GetPointer())
    {
    std::cerr << "Failed to add singleton into the scene: "
              << "node: " << singleton2.GetPointer() << " "
              << "added: " << addedNode << " "
              << "scene: " << singleton1->GetScene() << " "
              << "id: " << singleton1->GetID() << " "
              << "name: " << singleton1->GetName() << " "
              << "#nodes: " << scene->GetNumberOfNodes() << " "
              << scene->GetNodeByID("vtkMRMLSliceNodeSingleton2") << std::endl;
    return EXIT_FAILURE;
    }

  const char sceneXML[] =
    "<MRML  version=\"18916\" userTags=\"\">"
    "<SceneView id=\"vtkMRMLSceneSnapshotNode1\" name=\"sceneView\">"
    "  <Model id=\"vtkMRMLModelNodeSingleton\" name=\"Restored Model\" ></Model>"
    "  <Model id=\"vtkMRMLModelNode1\"      name=\"Old Model\" ></Model>"
    "  <Model id=\"vtkMRMLModelNodeSingleton1\" name=\"Restored Model2\" ></Model>"
    "</SceneView>"
    "</MRML>"
    ;

  vtkNew<vtkMRMLScene> tempScene;
  vtkNew<vtkMRMLSceneViewNode> registerNode;
  tempScene->RegisterNodeClass(registerNode.GetPointer());

  vtkNew<vtkMRMLParser> parser;
  parser->SetMRMLScene(tempScene.GetPointer());
  parser->Parse(sceneXML);

  vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(
    tempScene->GetFirstNodeByName("sceneView"));
  sceneViewNode->GetStoredScene()->InitTraversal();

  // Test singleton loading/restoring.
  vtkMRMLNode* restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNextNodeByClass("vtkMRMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  if (scene->GetNumberOfNodes() != 2 ||
      strcmp(restoredSingleton1->GetID(), "vtkMRMLModelNodeSingleton") != 0 ||
      scene->GetNodeByID(restoredSingleton1->GetID()) != singleton1.GetPointer() ||
      strcmp(singleton1->GetName(), "Restored Model") != 0 ||
      addedNode != singleton1.GetPointer())
    {
    std::cerr << "Failed to add singleton into the scene: "
              << "node: " << singleton1.GetPointer() << " "
              << "added: " << addedNode << " "
              << "scene: " << singleton1->GetScene() << " "
              << "id: " << singleton1->GetID() << " "
              << "name: " << singleton1->GetName() << " "
              << "#nodes: " << scene->GetNumberOfNodes() << " "
              << scene->GetNodeByID("vtkMRMLSliceNodeSingleton");
    return EXIT_FAILURE;
    }

  // Test compatibility with Slicer 3 scenes.
  restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNextNodeByClass("vtkMRMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  if (scene->GetNumberOfNodes() != 2 ||
      strcmp(restoredSingleton1->GetID(), "vtkMRMLModelNode1") != 0 ||
      scene->GetNodeByID(restoredSingleton1->GetID()) != singleton1.GetPointer() ||
      strcmp(singleton1->GetName(), "Old Model") != 0 ||
      addedNode == restoredSingleton1)
    {
    std::cerr << "Failed to add singleton into the scene: "
              << "node: " << singleton1.GetPointer() << " "
              << "added: " << addedNode << " "
              << "scene: " << singleton1->GetScene() << " "
              << "id: " << singleton1->GetID() << " "
              << "name: " << singleton1->GetName() << " "
              << "#nodes: " << scene->GetNumberOfNodes() << " "
              << scene->GetNodeByID("vtkMRMLModelNode1");
    return EXIT_FAILURE;
    }

  // Test odd node ID. There is no reason why it could happen, but there is no
  // reason why it shouldn't be supported.
  restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNextNodeByClass("vtkMRMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  if (scene->GetNumberOfNodes() != 2 ||
      strcmp(restoredSingleton1->GetID(), "vtkMRMLModelNodeSingleton1") != 0 ||
      scene->GetNodeByID(restoredSingleton1->GetID()) != singleton1.GetPointer() ||
      strcmp(singleton1->GetName(), "Restored Model2") != 0 ||
      addedNode != singleton1.GetPointer())
    {
    std::cerr << "Failed to add singleton into the scene: "
              << "node: " << singleton1.GetPointer() << " "
              << "added: " << addedNode << " "
              << "scene: " << singleton1->GetScene() << " "
              << "id: " << singleton1->GetID() << " "
              << "name: " << singleton1->GetName() << " "
              << "#nodes: " << scene->GetNumberOfNodes() << " "
              << scene->GetNodeByID("vtkMRMLSliceNodeSingleton1");
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
