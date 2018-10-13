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
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneEventRecorder.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>

// STD includes
#include <iostream>

//---------------------------------------------------------------------------
int vtkMRMLSceneBatchProcessTest(
  int vtkNotUsed(argc), char * vtkNotUsed(argv) [] )
{
  /*
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }
  */
  // Instantiate scene
  vtkNew<vtkMRMLScene> scene;

  // Configure mrml event callback
  vtkNew<vtkMRMLSceneEventRecorder> callback;
  scene->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  //---------------------------------------------------------------------------
  // BatchProcess
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  scene->StartState(vtkMRMLScene::BatchProcessState);

  if (callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }

  callback->CalledEvents.clear();

  // Fires:
  // 1) EndBatchProcessEvent
  scene->EndState(vtkMRMLScene::BatchProcessState);

  if (callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // Nested BatchProcess
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  scene->StartState(vtkMRMLScene::BatchProcessState);
  scene->StartState(vtkMRMLScene::BatchProcessState);

  if (scene->IsBatchProcessing() != true||
      callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  scene->EndState(vtkMRMLScene::BatchProcessState);
  if (scene->IsBatchProcessing() != true ||
      callback->CalledEvents.size() != 0)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  // Fires:
  // 2) EndBatchProcessEvent
  scene->EndState(vtkMRMLScene::BatchProcessState);

  if (scene->IsBatchProcessing() != false ||
      callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // Import
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartImportProcessEvent
  scene->StartState(vtkMRMLScene::ImportState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsImporting() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartImportEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::StartBatchProcessEvent] >
      callback->LastEventMTime[vtkMRMLScene::StartImportEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::StartImportEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 3) NodeAboutToBeAddedEvent
  // 4) NodeAddedEvent
  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

  if (scene->IsBatchProcessing() != true ||
      scene->IsImporting() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::NodeAboutToBeAddedEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::NodeAddedEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::NodeAboutToBeAddedEvent] >
      callback->LastEventMTime[vtkMRMLScene::NodeAddedEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::NodeAboutToBeAddedEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::NodeAddedEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 5) EndImportProcessEvent
  // 6) EndBatchProcessEvent
  scene->EndState(vtkMRMLScene::ImportState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsImporting() != false ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::EndImportEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::EndImportEvent] >
      callback->LastEventMTime[vtkMRMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::EndImportEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // BatchProcess + Import
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartImportProcessEvent
  scene->StartState(vtkMRMLScene::BatchProcessState);
  scene->StartState(vtkMRMLScene::ImportState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsImporting() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartImportEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::StartBatchProcessEvent] >
      callback->LastEventMTime[vtkMRMLScene::StartImportEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::StartImportEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 3) EndImportProcessEvent
  // 4) EndBatchProcessEvent
  scene->EndState(vtkMRMLScene::ImportState);
  if (scene->IsBatchProcessing() != true ||
      scene->IsImporting() != false ||
      callback->CalledEvents.size() != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndImportEvent] != 1)
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::EndImportEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  scene->EndState(vtkMRMLScene::BatchProcessState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsImporting() != false ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::EndImportEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::EndImportEvent] >
      callback->LastEventMTime[vtkMRMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::EndImportEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // Close
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartCloseProcessEvent
  scene->StartState(vtkMRMLScene::CloseState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsClosing() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::StartCloseEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::StartCloseEvent] >
      callback->LastEventMTime[vtkMRMLScene::StartBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::StartCloseEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 3) NodeAboutToBeRemovedEvent
  // 4) NodeRemovedEvent
  scene->RemoveNode(modelNode.GetPointer());

  if (scene->IsBatchProcessing() != true ||
      scene->IsClosing() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::NodeAboutToBeRemovedEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::NodeRemovedEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::NodeAboutToBeRemovedEvent] >
      callback->LastEventMTime[vtkMRMLScene::NodeRemovedEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::NodeAboutToBeRemovedEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::NodeRemovedEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 5) EndCloseProcessEvent
  // 6) EndBatchProcessEvent
  scene->EndState(vtkMRMLScene::CloseState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsClosing() != false ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::EndCloseEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::EndCloseEvent] >
      callback->LastEventMTime[vtkMRMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::EndCloseEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  //---------------------------------------------------------------------------
  // Restore within Restore
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartRestoreProcessEvent
  scene->StartState(vtkMRMLScene::RestoreState);
  scene->StartState(vtkMRMLScene::RestoreState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsRestoring() != true ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartRestoreEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::StartBatchProcessEvent] >
      callback->LastEventMTime[vtkMRMLScene::StartRestoreEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::StartRestoreEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();


  // 5) EndRestoreProcessEvent
  // 6) EndBatchProcessEvent
  scene->EndState(vtkMRMLScene::RestoreState);
  scene->EndState(vtkMRMLScene::RestoreState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsRestoring() != false ||
      callback->CalledEvents.size() != 2 ||
      callback->CalledEvents[vtkMRMLScene::EndRestoreEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::EndRestoreEvent] >
      callback->LastEventMTime[vtkMRMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::EndRestoreEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();


  //---------------------------------------------------------------------------
  // Import within Restore
  //---------------------------------------------------------------------------
  // Fires:
  // 1) StartBatchProcessEvent
  // 2) StartRestoreProcessEvent
  // 3) StartImportProcessEvent
  scene->StartState(vtkMRMLScene::RestoreState);
  scene->StartState(vtkMRMLScene::ImportState);

  if (scene->IsBatchProcessing() != true ||
      scene->IsRestoring() != true ||
      scene->IsImporting() != true ||
      callback->CalledEvents.size() != 3 ||
      callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartRestoreEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::StartImportEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::StartBatchProcessEvent] >
      callback->LastEventMTime[vtkMRMLScene::StartRestoreEvent] ||
      callback->LastEventMTime[vtkMRMLScene::StartRestoreEvent] >
      callback->LastEventMTime[vtkMRMLScene::StartImportEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::StartBatchProcessEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::StartRestoreEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::StartImportEvent] << " "
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  // 4) EndImportProcessEvent
  // 5) EndRestoreProcessEvent
  // 6) EndBatchProcessEvent
  scene->EndState(vtkMRMLScene::ImportState);
  scene->EndState(vtkMRMLScene::RestoreState);

  if (scene->IsBatchProcessing() != false ||
      scene->IsRestoring() != false ||
      scene->IsImporting() != false ||
      callback->CalledEvents.size() != 3 ||
      callback->CalledEvents[vtkMRMLScene::EndImportEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndRestoreEvent] != 1 ||
      callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent] != 1 ||
      callback->LastEventMTime[vtkMRMLScene::EndImportEvent] >
      callback->LastEventMTime[vtkMRMLScene::EndRestoreEvent] ||
      callback->LastEventMTime[vtkMRMLScene::EndRestoreEvent] >
      callback->LastEventMTime[vtkMRMLScene::EndBatchProcessEvent])
    {
    std::cerr << "Wrong fired events: "
              << callback->CalledEvents.size() << " event(s) fired." << std::endl
              << callback->CalledEvents[vtkMRMLScene::EndImportEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::EndRestoreEvent] << " "
              << callback->CalledEvents[vtkMRMLScene::EndBatchProcessEvent]
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->CalledEvents.clear();

  return EXIT_SUCCESS;
}
