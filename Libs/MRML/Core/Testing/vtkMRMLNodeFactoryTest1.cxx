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
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNodeFactory.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

namespace
{
void countEvent(vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  (*reinterpret_cast<int*>(clientData))++;
}
} // namespace

//---------------------------------------------------------------------------
int TestFactory()
{
  vtkNew<vtkMRMLNodeFactory> factory;
  CHECK_INT(factory->GetNumberOfRegisteredNodeClasses(), 0);
  CHECK_INT(factory->GetNumberOfRegisteredAbstractNodeClasses(), 0);
  CHECK_NULL(factory->GetClassNameByTag("Model"));
  CHECK_NULL(factory->GetTagByClassName("vtkMRMLModelNode"));
  CHECK_BOOL(factory->IsNodeClassRegistered("vtkMRMLModelNode"), false);
  CHECK_STD_STRING(factory->GetTypeDisplayNameByClassName("vtkMRMLModelNode"), "");

  int registeredEvents = 0;
  vtkNew<vtkCallbackCommand> callback;
  callback->SetCallback(countEvent);
  callback->SetClientData(&registeredEvents);
  factory->AddObserver(vtkMRMLNodeFactory::NodeClassRegisteredEvent, callback);

  vtkNew<vtkMRMLModelNode> modelPrototype;
  factory->RegisterNodeClass(modelPrototype);
  CHECK_INT(registeredEvents, 1);
  CHECK_INT(factory->GetNumberOfRegisteredNodeClasses(), 1);
  CHECK_POINTER(factory->GetNthRegisteredNodeClass(0), modelPrototype.GetPointer());
  CHECK_STRING(factory->GetClassNameByTag("Model"), "vtkMRMLModelNode");
  CHECK_STRING(factory->GetTagByClassName("vtkMRMLModelNode"), "Model");
  CHECK_BOOL(factory->IsNodeClassRegistered("vtkMRMLModelNode"), true);
  CHECK_STD_STRING(factory->GetTypeDisplayNameByClassName("vtkMRMLModelNode"), modelPrototype->GetTypeDisplayName());

  // Registering the same prototype with the same tag again is a no-op
  factory->RegisterNodeClass(modelPrototype);
  CHECK_INT(registeredEvents, 1);
  CHECK_INT(factory->GetNumberOfRegisteredNodeClasses(), 1);

  vtkSmartPointer<vtkMRMLNode> createdNode = vtkSmartPointer<vtkMRMLNode>::Take(factory->CreateNodeByClass("vtkMRMLModelNode"));
  CHECK_NOT_NULL(createdNode);
  CHECK_BOOL(createdNode->IsA("vtkMRMLModelNode"), true);
  CHECK_BOOL(createdNode.GetPointer() != modelPrototype.GetPointer(), true);

  // Legacy tag registration (backward compatibility)
  factory->RegisterNodeClass(modelPrototype, "LegacyModel");
  CHECK_INT(registeredEvents, 2);
  CHECK_INT(factory->GetNumberOfRegisteredNodeClasses(), 2);
  CHECK_STRING(factory->GetClassNameByTag("LegacyModel"), "vtkMRMLModelNode");

  // Abstract classes
  factory->RegisterAbstractNodeClass("vtkMRMLVolumeNode", "Volume");
  CHECK_INT(registeredEvents, 3);
  CHECK_INT(factory->GetNumberOfRegisteredAbstractNodeClasses(), 1);
  CHECK_STD_STRING(factory->GetNthRegisteredAbstractNodeClassName(0), "vtkMRMLVolumeNode");
  CHECK_STD_STRING(factory->GetNthRegisteredAbstractNodeTypeDisplayName(0), "Volume");
  CHECK_STD_STRING(factory->GetTypeDisplayNameByClassName("vtkMRMLVolumeNode"), "Volume");
  // same registration again does not invoke an event
  factory->RegisterAbstractNodeClass("vtkMRMLVolumeNode", "Volume");
  CHECK_INT(registeredEvents, 3);

  // Copy into another factory
  vtkNew<vtkMRMLNodeFactory> otherFactory;
  otherFactory->CopyRegisteredNodeClasses(factory);
  CHECK_INT(otherFactory->GetNumberOfRegisteredNodeClasses(), 2);
  CHECK_INT(otherFactory->GetNumberOfRegisteredAbstractNodeClasses(), 1);
  CHECK_STRING(otherFactory->GetClassNameByTag("LegacyModel"), "vtkMRMLModelNode");
  // the prototype is shared between the factories
  CHECK_POINTER(otherFactory->GetNthRegisteredNodeClass(0), modelPrototype.GetPointer());
  // copying again does not change anything
  otherFactory->CopyRegisteredNodeClasses(factory);
  CHECK_INT(otherFactory->GetNumberOfRegisteredNodeClasses(), 2);

  // Core classes
  vtkNew<vtkMRMLNodeFactory> coreFactory;
  coreFactory->RegisterCoreNodeClasses();
  CHECK_BOOL(coreFactory->GetNumberOfRegisteredNodeClasses() > 50, true);
  CHECK_BOOL(coreFactory->IsNodeClassRegistered("vtkMRMLSequenceNode"), true);
  CHECK_STRING(coreFactory->GetClassNameByTag("Sequence"), "vtkMRMLSequenceNode");
  CHECK_STD_STRING(coreFactory->GetTypeDisplayNameByClassName("vtkMRMLVolumeNode"), "Volume");

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestSceneNodeFactory()
{
  // A scene creates a factory with the core node classes on first use
  vtkNew<vtkMRMLScene> scene1;
  vtkMRMLNodeFactory* factory1 = scene1->GetNodeFactory();
  CHECK_NOT_NULL(factory1);
  CHECK_POINTER(scene1->GetNodeFactory(), factory1);
  CHECK_BOOL(scene1->IsNodeClassRegistered("vtkMRMLModelNode"), true);
  CHECK_STRING(scene1->GetClassNameByTag("Model"), "vtkMRMLModelNode");
  CHECK_STRING(scene1->GetTagByClassName("vtkMRMLModelNode"), "Model");
  CHECK_STD_STRING(scene1->GetTypeDisplayNameByClassName("vtkMRMLVolumeNode"), "Volume");
  CHECK_INT(scene1->GetNumberOfRegisteredNodeClasses(), factory1->GetNumberOfRegisteredNodeClasses());
  CHECK_INT(scene1->GetNumberOfRegisteredAbstractNodeClasses(), factory1->GetNumberOfRegisteredAbstractNodeClasses());
  CHECK_POINTER(scene1->GetNthRegisteredNodeClass(0), factory1->GetNthRegisteredNodeClass(0));
  CHECK_STD_STRING(scene1->GetNthRegisteredAbstractNodeClassName(0), factory1->GetNthRegisteredAbstractNodeClassName(0));
  CHECK_STD_STRING(scene1->GetNthRegisteredAbstractNodeTypeDisplayName(0), factory1->GetNthRegisteredAbstractNodeTypeDisplayName(0));

  vtkSmartPointer<vtkMRMLNode> modelNode = vtkSmartPointer<vtkMRMLNode>::Take(scene1->CreateNodeByClass("vtkMRMLModelNode"));
  CHECK_NOT_NULL(modelNode);
  CHECK_BOOL(modelNode->IsA("vtkMRMLModelNode"), true);

  // Share the factory with another scene
  vtkNew<vtkMRMLScene> scene2;
  scene2->SetNodeFactory(factory1);
  CHECK_POINTER(scene2->GetNodeFactory(), factory1);
  CHECK_BOOL(scene2->IsNodeClassRegistered("vtkMRMLModelNode"), true);

  // A class registered in one scene is available in the other scene,
  // and both scenes notify their observers
  int scene1Events = 0;
  int scene2Events = 0;
  vtkNew<vtkCallbackCommand> callback1;
  callback1->SetCallback(countEvent);
  callback1->SetClientData(&scene1Events);
  scene1->AddObserver(vtkMRMLScene::NodeClassRegisteredEvent, callback1);
  vtkNew<vtkCallbackCommand> callback2;
  callback2->SetCallback(countEvent);
  callback2->SetClientData(&scene2Events);
  scene2->AddObserver(vtkMRMLScene::NodeClassRegisteredEvent, callback2);

  vtkNew<vtkMRMLModelNode> modelPrototype;
  scene2->RegisterNodeClass(modelPrototype, "CustomModelTag");
  CHECK_STRING(scene1->GetClassNameByTag("CustomModelTag"), "vtkMRMLModelNode");
  CHECK_STRING(scene2->GetClassNameByTag("CustomModelTag"), "vtkMRMLModelNode");
  CHECK_INT(scene1Events, 1);
  CHECK_INT(scene2Events, 1);

  scene1->RegisterAbstractNodeClass("vtkMRMLTestAbstractNode", "Test");
  CHECK_STD_STRING(scene2->GetTypeDisplayNameByClassName("vtkMRMLTestAbstractNode"), "Test");
  CHECK_INT(scene1Events, 2);
  CHECK_INT(scene2Events, 2);

  // After the factory is replaced, the scene is not notified about changes of the old factory anymore
  vtkNew<vtkMRMLNodeFactory> factory3;
  scene2->SetNodeFactory(factory3);
  CHECK_INT(scene2Events, 3); // replacing the factory changes the registered classes
  CHECK_POINTER(scene2->GetNodeFactory(), factory3.GetPointer());
  CHECK_NULL(scene2->GetClassNameByTag("CustomModelTag"));
  scene1->RegisterNodeClass(modelPrototype, "AnotherCustomModelTag");
  CHECK_INT(scene1Events, 3);
  CHECK_INT(scene2Events, 3);
  factory3->RegisterNodeClass(modelPrototype);
  CHECK_INT(scene2Events, 4);
  CHECK_INT(scene1Events, 3);

  // CopyRegisteredNodesToScene shares the factory with a scene that does not have a factory yet
  vtkNew<vtkMRMLScene> scene4;
  scene1->CopyRegisteredNodesToScene(scene4);
  CHECK_POINTER(scene4->GetNodeFactory(), factory1);

  // ...and copies the registered classes into a scene that already has its own factory
  vtkNew<vtkMRMLScene> scene5;
  CHECK_NULL(scene5->GetClassNameByTag("CustomModelTag")); // creates the scene's own factory
  vtkMRMLNodeFactory* factory5 = scene5->GetNodeFactory();
  CHECK_BOOL(factory5 != factory1, true);
  scene1->CopyRegisteredNodesToScene(scene5);
  CHECK_POINTER(scene5->GetNodeFactory(), factory5);
  CHECK_STRING(scene5->GetClassNameByTag("CustomModelTag"), "vtkMRMLModelNode");
  CHECK_STD_STRING(scene5->GetTypeDisplayNameByClassName("vtkMRMLTestAbstractNode"), "Test");
  // registration in the copied scene does not affect the source scene
  scene5->RegisterNodeClass(modelPrototype, "Scene5OnlyTag");
  CHECK_NULL(scene1->GetClassNameByTag("Scene5OnlyTag"));

  // The factory outlives the scene that created it
  vtkSmartPointer<vtkMRMLNodeFactory> sharedFactory;
  {
    vtkNew<vtkMRMLScene> temporaryScene;
    sharedFactory = temporaryScene->GetNodeFactory();
  }
  CHECK_BOOL(sharedFactory->IsNodeClassRegistered("vtkMRMLModelNode"), true);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestSequenceNodeFactory()
{
  // The internal scene of a sequence node shares the factory of the scene that contains the sequence node
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
  CHECK_NOT_NULL(sequenceNode);
  CHECK_NOT_NULL(sequenceNode->GetSequenceScene());
  CHECK_POINTER(sequenceNode->GetSequenceScene()->GetNodeFactory(), scene->GetNodeFactory());

  vtkNew<vtkMRMLModelNode> modelNode;
  sequenceNode->SetDataNodeAtValue(modelNode, "0");
  CHECK_INT(sequenceNode->GetNumberOfDataNodes(), 1);

  // A sequence node that is not in a scene creates its own factory
  vtkNew<vtkMRMLSequenceNode> standaloneSequenceNode;
  CHECK_NOT_NULL(standaloneSequenceNode->GetSequenceScene());
  CHECK_BOOL(standaloneSequenceNode->GetSequenceScene()->GetNodeFactory() != scene->GetNodeFactory(), true);
  CHECK_BOOL(standaloneSequenceNode->GetSequenceScene()->IsNodeClassRegistered("vtkMRMLModelNode"), true);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int vtkMRMLNodeFactoryTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  CHECK_EXIT_SUCCESS(TestFactory());
  CHECK_EXIT_SUCCESS(TestSceneNodeFactory());
  CHECK_EXIT_SUCCESS(TestSequenceNodeFactory());
  return EXIT_SUCCESS;
}
