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
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneEventRecorder.h"
#include "vtkMRMLSceneViewNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
int vtkMRMLSceneViewNodeEventsTest(
  int vtkNotUsed(argc), char * vtkNotUsed(argv)[])
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLCameraNode> cameraNode;
  scene->AddNode(cameraNode.GetPointer());
  vtkNew<vtkMRMLInteractionNode> interactionNode;
  interactionNode->SetPlaceModePersistence(0);
  scene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  sceneViewNode->SetScene(scene.GetPointer());
  sceneViewNode->StoreScene();

  // Remove the cameraNode but keep the interaction node (singleton)
  scene->Clear(0);
  // Change the interaction node
  interactionNode->SetPlaceModePersistence(1);

  if (scene->GetNumberOfNodesByClass("vtkMRMLCameraNode") != 0 ||
      scene->GetNumberOfNodesByClass("vtkMRMLInteractionNode") != 1)
    {
    std::cerr << "Camera node not removed after vtkMRMLScene::Clear()"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLSceneEventRecorder> callback;
  scene->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  // Add the camera node back
  // The following (high level) happens in the scene:
  // vtkMRMLScene::StartState(RestoreState);
  // vtkMRMLScene::AddNode(savedCameraNode);
  // vtkMRMLScene::EndState(RestoreState);
  sceneViewNode->RestoreScene();

  if (scene->GetNumberOfNodesByClass("vtkMRMLCameraNode") != 1 ||
      scene->GetNumberOfNodesByClass("vtkMRMLInteractionNode") != 1)
    {
    std::cerr << "Camera or interaction nodes not restored " << std::endl;
    return EXIT_FAILURE;
    }

  if (callback->CalledEvents.size() != 6 ||
      callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartRestoreEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::NodeAboutToBeAddedEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::NodeAddedEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndRestoreEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::StartRestoreEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::NodeAboutToBeAddedEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::NodeAddedEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::EndRestoreEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
