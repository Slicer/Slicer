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
#include "vtkMRMLCoreTestingMacros.h"
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

  CHECK_POINTER(singleton1.GetPointer(), addedNode);
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkMRMLModelNodeSingleton"));
  CHECK_POINTER(singleton1->GetScene(), scene.GetPointer());
  CHECK_STRING(singleton1->GetID(), "vtkMRMLModelNodeSingleton");
  CHECK_STRING(singleton1->GetName(), "Model");
  CHECK_INT(scene->GetNumberOfNodes(), 1);

  // AddNode of a new singleton shouldn't add an instance but copy the node
  // in the existing singleton
  vtkNew<vtkMRMLModelNode> singleton1Copy;
  singleton1Copy->SetSingletonTag("Singleton");
  addedNode = scene->AddNode(singleton1Copy.GetPointer());

  CHECK_NULL(singleton1Copy->GetScene());
  CHECK_NULL(singleton1Copy->GetID());
  CHECK_NULL(singleton1Copy->GetName());
  CHECK_POINTER(singleton1.GetPointer(), addedNode);
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkMRMLModelNodeSingleton"));
  CHECK_STRING(singleton1->GetID(), "vtkMRMLModelNodeSingleton");
  CHECK_STRING(singleton1->GetName(), "Model");
  CHECK_INT(scene->GetNumberOfNodes(), 1);

  // Add a different singleton.
  vtkNew<vtkMRMLModelNode> singleton2;
  singleton2->SetSingletonTag("Singleton2");
  addedNode = scene->AddNode(singleton2.GetPointer());

  CHECK_POINTER(singleton2.GetPointer(), addedNode);
  CHECK_POINTER(singleton2.GetPointer(), scene->GetNodeByID("vtkMRMLModelNodeSingleton2"));
  CHECK_POINTER(singleton2->GetScene(), scene.GetPointer());
  CHECK_STRING(singleton2->GetID(), "vtkMRMLModelNodeSingleton2");
  CHECK_STRING(singleton2->GetName(), "Model_1");
  CHECK_INT(scene->GetNumberOfNodes(), 2);

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

  // Test singleton loading/restoring.
  vtkMRMLNode* restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNthNodeByClass(0, "vtkMRMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  CHECK_STRING(restoredSingleton1->GetID(), "vtkMRMLModelNodeSingleton");
  CHECK_POINTER(singleton1.GetPointer(), addedNode);
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkMRMLModelNodeSingleton"));
  CHECK_STRING(singleton1->GetName(), "Restored Model");
  CHECK_INT(scene->GetNumberOfNodes(), 2);

  // Test compatibility with Slicer 3 scenes.
  std::string singleton1ID = singleton1->GetID();
  restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNthNodeByClass(1, "vtkMRMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  CHECK_POINTER_DIFFERENT(restoredSingleton1, addedNode);
  CHECK_STRING(restoredSingleton1->GetID(), "vtkMRMLModelNode1");
  // The node ID of singleton1 is kept (the node ID of restoredSingleton1 is changed and
  // nodes that are imported along with this singleton are notified about the ID change)
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkMRMLModelNodeSingleton"));
  CHECK_STRING(singleton1->GetName(), "Old Model");
  CHECK_INT(scene->GetNumberOfNodes(), 2);

  // Test odd node ID. There is no reason why it could happen, but there is no
  // reason why it shouldn't be supported.
  restoredSingleton1 =
    sceneViewNode->GetStoredScene()->GetNthNodeByClass(2, "vtkMRMLModelNode");
  restoredSingleton1->SetSingletonTag("Singleton");
  restoredSingleton1->SetAddToScene(1);
  addedNode = scene->AddNode(restoredSingleton1);

  CHECK_POINTER_DIFFERENT(restoredSingleton1, addedNode);
  CHECK_STRING(restoredSingleton1->GetID(), "vtkMRMLModelNodeSingleton1");
  CHECK_POINTER(singleton1.GetPointer(), addedNode);
  // The node ID of singleton1 is kept (the node ID of restoredSingleton1 is changed and
  // nodes that are imported along with this singleton are notified about the ID change)
  CHECK_POINTER(singleton1.GetPointer(), scene->GetNodeByID("vtkMRMLModelNodeSingleton"));
  CHECK_STRING(singleton1->GetName(), "Restored Model2");
  CHECK_INT(scene->GetNumberOfNodes(), 2);

  ////////////////////////////
  // Check node references of imported singleton and regular nodes

  scene->Clear(true);

  // SingletonNodeA: singleton node referencing another singleton node and a regular node
  // vtkMRMLScriptedModuleNode1: regular node referencing a singleton node and another regular node
  const char scene1XML[] =
    "<MRML>"
    "<ScriptedModule id=\"vtkMRMLScriptedModuleNodeSingletonA\" name=\"Scene1SingletonNodeA\" singletonTag=\"SingletonA\""
      " references=\"ReferenceB:vtkMRMLScriptedModuleNodeSingletonB;Reference1:vtkMRMLScriptedModuleNode1;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkMRMLScriptedModuleNodeSingletonB\" name=\"Scene1SingletonNodeB\" singletonTag=\"SingletonB\""
      " references=\"Reference1:vtkMRMLScriptedModuleNode2;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkMRMLScriptedModuleNode1\" name=\"Scene1RegularNode1\""
      " references=\"Reference2:vtkMRMLScriptedModuleNode2;ReferenceA:vtkMRMLScriptedModuleNodeSingletonA;ReferenceB:vtkMRMLScriptedModuleNodeSingletonB;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkMRMLScriptedModuleNode2\" name=\"Scene1RegularNode2\" > </ScriptedModule>"
    "</MRML>";

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(scene1XML);
  scene->Import();  // adds Subject Hierarchy Node

  CHECK_INT(scene->GetNumberOfNodes(), 5);

  vtkMRMLNode* scene1SingletonA = scene->GetNodeByID("vtkMRMLScriptedModuleNodeSingletonA");
  vtkMRMLNode* scene1SingletonB = scene->GetNodeByID("vtkMRMLScriptedModuleNodeSingletonB");
  vtkMRMLNode* scene1Regular1 = scene->GetNodeByID("vtkMRMLScriptedModuleNode1");
  vtkMRMLNode* scene1Regular2 = scene->GetNodeByID("vtkMRMLScriptedModuleNode2");

  // Check node contents
  CHECK_NOT_NULL(scene1SingletonA);
  CHECK_NOT_NULL(scene1SingletonB);
  CHECK_NOT_NULL(scene1Regular1);
  CHECK_NOT_NULL(scene1Regular2);
  CHECK_STRING(scene1SingletonA->GetName(), "Scene1SingletonNodeA");
  CHECK_STRING(scene1SingletonB->GetName(), "Scene1SingletonNodeB");
  CHECK_STRING(scene1Regular1->GetName(), "Scene1RegularNode1");
  CHECK_STRING(scene1Regular2->GetName(), "Scene1RegularNode2");

  // Check node references
  CHECK_POINTER(scene1SingletonA->GetNodeReference("ReferenceB"), scene1SingletonB);
  CHECK_POINTER(scene1SingletonA->GetNodeReference("Reference1"), scene1Regular1);
  CHECK_POINTER(scene1Regular1->GetNodeReference("ReferenceA"), scene1SingletonA);
  CHECK_POINTER(scene1Regular1->GetNodeReference("ReferenceB"), scene1SingletonB);
  CHECK_POINTER(scene1Regular1->GetNodeReference("Reference2"), scene1Regular2);

  // SingletonNodeA: singleton node referencing another singleton node and a regular node
  // SingletonNodeB: singleton node, ID clash with a node in scene 1
  // vtkMRMLScriptedModuleNode1: regular node referencing a singleton node and another regular node; ID clash with a node in scene 1
  const char scene2XML[] =
    "<MRML>"
    "<ScriptedModule id=\"vtkMRMLScriptedModuleNodeSingletonA\" name=\"Scene2SingletonNodeA\" singletonTag=\"SingletonA\""
      " references=\"ReferenceB:vtkMRMLScriptedModuleNodeSingletonB;Reference1:vtkMRMLScriptedModuleNode1;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkMRMLScriptedModuleNodeSingletonXB\" name=\"Scene2SingletonNodeB\" singletonTag=\"SingletonB\""
      " references=\"Reference1:vtkMRMLScriptedModuleNodeX2;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkMRMLScriptedModuleNode1\" name=\"Scene2RegularNode1\""
      " references=\"Reference2:vtkMRMLScriptedModuleNodeX2;ReferenceA:vtkMRMLScriptedModuleNodeSingletonA;ReferenceB:vtkMRMLScriptedModuleNodeSingletonXB;\" > </ScriptedModule>"
    "<ScriptedModule id=\"vtkMRMLScriptedModuleNodeX2\" name=\"Scene2RegularNode2\" > </ScriptedModule>"
    "</MRML>";

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(scene2XML);
  scene->Import();  // adds Subject Hierarchy Node

  CHECK_INT(scene->GetNumberOfNodes(), 7);

  vtkMRMLNode* scene2SingletonA = scene->GetNodeByID("vtkMRMLScriptedModuleNodeSingletonA");
  vtkMRMLNode* scene2SingletonB = scene->GetNodeByID("vtkMRMLScriptedModuleNodeSingletonB"); // ID changed (singleton is matched based on singleton tag)
  vtkMRMLNode* scene2Regular1 = scene->GetNodeByID("vtkMRMLScriptedModuleNode3"); // ID changed (due to clash with a regular node in scene1)
  vtkMRMLNode* scene2Regular2 = scene->GetNodeByID("vtkMRMLScriptedModuleNodeX2");

  CHECK_NULL(scene->GetNodeByID("vtkMRMLScriptedModuleNodeSingletonXB"));
  CHECK_NULL(scene->GetNodeByID("vtkMRMLScriptedModuleNode4"));
  CHECK_NOT_NULL(scene2SingletonA);
  CHECK_NOT_NULL(scene2SingletonB);
  CHECK_NOT_NULL(scene2Regular1);
  CHECK_NOT_NULL(scene2Regular2);

  // Check contents of newly imported nodes
  CHECK_STRING(scene2SingletonA->GetName(), "Scene2SingletonNodeA");
  CHECK_STRING(scene2SingletonB->GetName(), "Scene2SingletonNodeB");
  CHECK_STRING(scene2Regular1->GetName(), "Scene2RegularNode1");
  CHECK_STRING(scene2Regular2->GetName(), "Scene2RegularNode2");

  // Check that singleton ID is kept but contents is overwritten
  CHECK_STRING(scene1SingletonA->GetID(), "vtkMRMLScriptedModuleNodeSingletonA"); // same
  CHECK_STRING(scene1SingletonA->GetName(), "Scene2SingletonNodeA"); // changed
  CHECK_STRING(scene1SingletonB->GetID(), "vtkMRMLScriptedModuleNodeSingletonB"); // same
  CHECK_STRING(scene1SingletonB->GetName(), "Scene2SingletonNodeB"); // changed
  // Check that singleton node pointers remained the same
  CHECK_POINTER(scene1SingletonA, scene2SingletonA);
  CHECK_POINTER(scene1SingletonB, scene2SingletonB);
  // Check that regular nodes have not changed
  CHECK_STRING(scene1Regular1->GetID(), "vtkMRMLScriptedModuleNode1"); // same
  CHECK_STRING(scene1Regular1->GetName(), "Scene1RegularNode1"); // same
  CHECK_STRING(scene1Regular2->GetID(), "vtkMRMLScriptedModuleNode2"); // same
  CHECK_STRING(scene1Regular2->GetName(), "Scene1RegularNode2"); // same

  // Check that node references in regular nodes did not change
  CHECK_POINTER(scene1Regular1->GetNodeReference("ReferenceB"), scene1SingletonB);
  CHECK_POINTER(scene1Regular1->GetNodeReference("Reference2"), scene1Regular2);

  // Check that new node references are correct (same test repeated as with scene1 before the second scene import)
  CHECK_POINTER(scene2SingletonA->GetNodeReference("ReferenceB"), scene2SingletonB);
  CHECK_POINTER(scene2SingletonA->GetNodeReference("Reference1"), scene2Regular1);
  CHECK_POINTER(scene2Regular1->GetNodeReference("ReferenceA"), scene2SingletonA);
  CHECK_POINTER(scene2Regular1->GetNodeReference("ReferenceB"), scene2SingletonB);
  CHECK_POINTER(scene2Regular1->GetNodeReference("Reference2"), scene2Regular2);

  return EXIT_SUCCESS;
}
