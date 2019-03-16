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

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// MRML includes
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <iostream>

//---------------------------------------------------------------------------
/// vtkMRMLTestLogic records what methods of vtkMRMLAbstractLogic are called
/// when vtkMRMLScene fires events.
class vtkMRMLTestLogic: public vtkMRMLAbstractLogic
{
public:
  vtkTypeMacro(vtkMRMLTestLogic, vtkMRMLAbstractLogic);
  static vtkMRMLTestLogic *New();

  void SetMRMLSceneInternal(vtkMRMLScene* scene) override;
  void UnobserveMRMLScene() override;
  void ObserveMRMLScene() override;
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;

  void OnMRMLSceneStartBatchProcess() override;
  void OnMRMLSceneEndBatchProcess() override;
  void OnMRMLSceneStartClose() override;
  void OnMRMLSceneEndClose() override;
  void OnMRMLSceneStartImport() override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneStartRestore() override;
  void OnMRMLSceneEndRestore() override;
  void OnMRMLSceneNew() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* nodeAdded) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* nodeRemoved) override;

  enum MethodType{
    Set = 0,
    Unobserve,
    Observe,
    Register,
    Update
  };
  /// CalledMethods stores how many times a method is called.
  /// Methods are identified using vtkMRMLTestLogic::MethodType
  /// or vtkMRMLScene::SceneEventType
  std::map<unsigned long, int> CalledMethods;
protected:
  vtkMRMLTestLogic() = default;
  ~vtkMRMLTestLogic() override = default;
};

vtkStandardNewMacro(vtkMRMLTestLogic);

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::SetMRMLSceneInternal(vtkMRMLScene* scene)
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLTestLogic::Set];

  // Listen to all the events
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::StartBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartCloseEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartImportEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndImportEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::StartRestoreEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::EndRestoreEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  this->SetAndObserveMRMLSceneEventsInternal(scene, sceneEvents.GetPointer());
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::UnobserveMRMLScene()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLTestLogic::Unobserve];
  this->Superclass::UnobserveMRMLScene();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::ObserveMRMLScene()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLTestLogic::Observe];
  this->Superclass::ObserveMRMLScene();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::RegisterNodes()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLTestLogic::Register];
  this->Superclass::RegisterNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::UpdateFromMRMLScene()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLTestLogic::Update];
  this->Superclass::UpdateFromMRMLScene();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneStartBatchProcess()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::StartBatchProcessEvent];
  this->Superclass::OnMRMLSceneStartBatchProcess();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneEndBatchProcess()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::EndBatchProcessEvent];
  this->Superclass::OnMRMLSceneEndBatchProcess();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneStartClose()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::StartCloseEvent];
  this->Superclass::OnMRMLSceneStartClose();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneEndClose()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::EndCloseEvent];
  this->Superclass::OnMRMLSceneEndClose();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneStartImport()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::StartImportEvent];
  this->Superclass::OnMRMLSceneStartImport();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneEndImport()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::EndImportEvent];
  this->Superclass::OnMRMLSceneEndImport();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneStartRestore()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::StartRestoreEvent];
  this->Superclass::OnMRMLSceneStartRestore();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneEndRestore()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::EndRestoreEvent];
  this->Superclass::OnMRMLSceneEndRestore();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneNew()
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::NewSceneEvent];
  this->Superclass::OnMRMLSceneNew();
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* nodeAdded)
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::NodeAddedEvent];
  this->Superclass::OnMRMLSceneNodeAdded(nodeAdded);
}

//---------------------------------------------------------------------------
void vtkMRMLTestLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* nodeRemoved)
{
  std::cout << __FUNCTION__ << std::endl;
  ++this->CalledMethods[vtkMRMLScene::NodeRemovedEvent];
  this->Superclass::OnMRMLSceneNodeRemoved(nodeRemoved);
}

//---------------------------------------------------------------------------
int vtkMRMLAbstractLogicSceneEventsTest(
  int vtkNotUsed(argc), char * vtkNotUsed(argv)[] )
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLTestLogic* testLogic = vtkMRMLTestLogic::New();

  //---------------------------------------------------------------------------
  // SetMRMLScene
  //---------------------------------------------------------------------------
  testLogic->SetMRMLScene(scene.GetPointer());

  if (testLogic->GetMRMLScene() != scene.GetPointer() ||
      testLogic->CalledMethods.size() != 4 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Set] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Observe] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Register] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLTestLogic::Set] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Observe] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Register] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Update]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // SetMRMLScene(other scene)
  //---------------------------------------------------------------------------
  vtkNew<vtkMRMLScene> scene2;
  testLogic->SetMRMLScene(scene2.GetPointer());

  if (testLogic->GetMRMLScene() != scene2.GetPointer() ||
      testLogic->CalledMethods.size() != 5 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Unobserve] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Set] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Observe] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Register] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLTestLogic::Unobserve] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Set] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Observe] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Register] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Update]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // SetMRMLScene(0)
  //---------------------------------------------------------------------------
  testLogic->SetMRMLScene(nullptr);

  if (testLogic->GetMRMLScene() != nullptr ||
      testLogic->CalledMethods.size() != 2 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Unobserve] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Set] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLTestLogic::Unobserve] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Set]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // SetMRMLScene(scene)
  //---------------------------------------------------------------------------
  testLogic->SetMRMLScene(scene.GetPointer());

  if (testLogic->GetMRMLScene() != scene.GetPointer() ||
      testLogic->CalledMethods.size() != 4 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Set] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Observe] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Register] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLTestLogic::Set] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Observe] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Register] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Update]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // Import
  //---------------------------------------------------------------------------
  scene->StartState(vtkMRMLScene::ImportState);

  if (testLogic->CalledMethods.size() != 2 ||
      testLogic->CalledMethods[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
      testLogic->CalledMethods[vtkMRMLScene::StartImportEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLScene::StartBatchProcessEvent] << " "
              << testLogic->CalledMethods[vtkMRMLScene::StartImportEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  // Add node into the scene during the import state
  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  scene->AddNode(volumeNode.GetPointer());

  if (testLogic->CalledMethods.size() != 1 ||
      testLogic->CalledMethods[vtkMRMLScene::NodeAddedEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLScene::NodeAddedEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  // end of the import
  scene->EndState(vtkMRMLScene::ImportState);

  if (testLogic->CalledMethods.size() != 3 ||
      testLogic->CalledMethods[vtkMRMLScene::EndImportEvent] != 1 ||
      testLogic->CalledMethods[vtkMRMLScene::EndBatchProcessEvent] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLScene::EndBatchProcessEvent] << " "
              << testLogic->CalledMethods[vtkMRMLScene::EndImportEvent] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Update]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // Add node (outside of batch processing)
  //---------------------------------------------------------------------------
  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());
  if (testLogic->CalledMethods.size() != 1 ||
      testLogic->CalledMethods[vtkMRMLScene::NodeAddedEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLScene::NodeAddedEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // Close
  //---------------------------------------------------------------------------
  scene->Clear(false);
  if (testLogic->CalledMethods.size() != 6 ||
      testLogic->CalledMethods[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
      testLogic->CalledMethods[vtkMRMLScene::StartCloseEvent] != 1 ||
      testLogic->CalledMethods[vtkMRMLScene::NodeRemovedEvent] != 2 ||
      testLogic->CalledMethods[vtkMRMLScene::EndCloseEvent] != 1 ||
      testLogic->CalledMethods[vtkMRMLScene::EndBatchProcessEvent] != 1 ||
      testLogic->CalledMethods[vtkMRMLTestLogic::Update] != 1)
    {
    std::cerr << "Wrong fired events: "
              << testLogic->CalledMethods.size() << " event(s) fired." << std::endl
              << testLogic->CalledMethods[vtkMRMLScene::StartBatchProcessEvent] << " "
              << testLogic->CalledMethods[vtkMRMLScene::StartCloseEvent] << " "
              << testLogic->CalledMethods[vtkMRMLScene::NodeRemovedEvent] << " "
              << testLogic->CalledMethods[vtkMRMLScene::EndCloseEvent] << " "
              << testLogic->CalledMethods[vtkMRMLScene::EndBatchProcessEvent] << " "
              << testLogic->CalledMethods[vtkMRMLTestLogic::Update] << " "
              << std::endl;
    return EXIT_FAILURE;
    }
  testLogic->CalledMethods.clear();

  //---------------------------------------------------------------------------
  // Add node again (outside of batch processing)
  //---------------------------------------------------------------------------
  scene->AddNode(modelNode.GetPointer());
  scene->AddNode(volumeNode.GetPointer());

  testLogic->Delete();

  return EXIT_SUCCESS;
}

