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
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// Sequences logic includes
#include <vtkSlicerSequencesLogic.h>

// SceneView logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

//---------------------------------------------------------------------------
int vtkSceneViewEventsTest(
  int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLApplicationLogic> appLogic;

  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  appLogic->SetMRMLScene(scene);
  sequencesLogic->SetMRMLScene(scene);
  sequencesLogic->SetMRMLApplicationLogic(appLogic);
  appLogic->SetModuleLogic("Sequences", sequencesLogic);

  vtkNew<vtkSlicerSceneViewsModuleLogic> sceneViewLogic;
  sceneViewLogic->SetMRMLScene(scene);
  sceneViewLogic->SetMRMLApplicationLogic(appLogic);
  appLogic->SetModuleLogic("SceneViews", sceneViewLogic);


  vtkNew<vtkMRMLCameraNode> cameraNode;
  scene->AddNode(cameraNode.GetPointer());

  sceneViewLogic->CreateSceneView("SceneView1");

  // Change the camera node
  cameraNode->SetPosition(1.0, 2.0, 3.0);

  // Add observer to the camera node
  int cameraModifiedEventCount = 0;
  vtkNew<vtkCallbackCommand> cameraNodeObserver;
  cameraNodeObserver->SetCallback(
    [](vtkObject* caller, unsigned long eid, void* clientData, void* callData)
    {
      int* cameraModifiedEventCount = static_cast<int*>(clientData);
      (*cameraModifiedEventCount)++;
    });
  cameraNodeObserver->SetClientData(&cameraModifiedEventCount);
  cameraNode->AddObserver(vtkCommand::ModifiedEvent, cameraNodeObserver.GetPointer());

  sceneViewLogic->RestoreSceneView("SceneView1");

  CHECK_INT(cameraModifiedEventCount, 1);

  return EXIT_SUCCESS;
}
