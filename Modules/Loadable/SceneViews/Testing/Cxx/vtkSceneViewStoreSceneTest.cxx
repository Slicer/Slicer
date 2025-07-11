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
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

// Sequences logic includes
#include <vtkSlicerSequencesLogic.h>

// SceneView logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

namespace
{

void populateScene(vtkMRMLScene* scene);
int store(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int storeAndRestore(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int storeAndRemoveVolume(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int storeTwice(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int storeAndRestoreTwice(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int storeTwiceAndRemoveVolume(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int references(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int storePerformance(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkSceneViewStoreSceneTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
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

  CHECK_EXIT_SUCCESS(store(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(storeAndRestore(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(storeAndRemoveVolume(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(storeTwice(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(storeAndRestoreTwice(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(storeTwiceAndRemoveVolume(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(references(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(storePerformance(scene, sceneViewLogic));
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
void populateScene(vtkMRMLScene* scene)
{
  scene->Clear();

  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  scene->AddNode(displayNode);

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  volumeNode->SetScene(scene);
  scene->AddNode(volumeNode);
  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
}

//---------------------------------------------------------------------------
int store(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  populateScene(scene);

  sceneViewLogic->CreateSceneView("SceneView1");

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeAndRestore(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  populateScene(scene);

  vtkMRMLNode* volumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");

  sceneViewLogic->CreateSceneView("SceneView1");
  sceneViewLogic->RestoreSceneView("SceneView1");

  vtkMRMLNode* restoredVolumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  // Restoring the volume should reuse the same node.
  CHECK_POINTER(restoredVolumeNode, volumeNode);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeAndRemoveVolume(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  populateScene(scene);

  sceneViewLogic->CreateSceneView("SceneView1");

  // Remove node from the scene to see if it gets restored.
  vtkMRMLNode* volumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  scene->RemoveNode(volumeNode);

  sceneViewLogic->RestoreSceneView("SceneView1");

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeTwice(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  populateScene(scene);

  sceneViewLogic->CreateSceneView("SceneView1");
  sceneViewLogic->CreateSceneView("SceneView2");
  CHECK_INT(sceneViewLogic->GetNumberOfSceneViews(), 2);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeAndRestoreTwice(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  populateScene(scene);

  sceneViewLogic->CreateSceneView("SceneView1");
  sceneViewLogic->CreateSceneView("SceneView2");

  sceneViewLogic->RestoreSceneView("SceneView1");
  sceneViewLogic->RestoreSceneView("SceneView2");

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeTwiceAndRemoveVolume(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  populateScene(scene);

  sceneViewLogic->CreateSceneView("SceneView1");
  sceneViewLogic->CreateSceneView("SceneView2");

  vtkMRMLNode* node = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  scene->RemoveNode(node);

  sceneViewLogic->RestoreSceneView("SceneView1");
  sceneViewLogic->RestoreSceneView("SceneView2");

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int references(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  populateScene(scene);

  vtkMRMLNode* volumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  vtkSmartPointer<vtkCollection> sceneReferencedNodes;
  sceneReferencedNodes.TakeReference(scene->GetReferencedNodes(volumeNode));
  CHECK_INT(sceneReferencedNodes->GetNumberOfItems(), 2);

  sceneViewLogic->CreateSceneView("SceneView1");
  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(scene->GetReferencedNodes(volumeNode));

  sceneViewLogic->RestoreSceneView("SceneView1");
  vtkSmartPointer<vtkCollection> rereferencedNodes;
  rereferencedNodes.TakeReference(scene->GetReferencedNodes(volumeNode));

  CHECK_INT(referencedNodes->GetNumberOfItems(), rereferencedNodes->GetNumberOfItems());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storePerformance(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  // This test is for performance
  const int displayNodePairCount = 100;

  for (int i = 0; i < displayNodePairCount; ++i)
  {
    populateScene(scene);
  }

  vtkNew<vtkTimerLog> timer;
  timer->StartTimer();
  sceneViewLogic->CreateSceneView("SceneView1");
  timer->StopTimer();

  std::cout << "<DartMeasurement name=\"vtkMRMLSceneViewNode-StorePerformance-" << displayNodePairCount << "\" type=\"numeric/double\">" << timer->GetElapsedTime()
            << "</DartMeasurement>" << std::endl;

  return EXIT_SUCCESS;
}

} // end of anonymous namespace
