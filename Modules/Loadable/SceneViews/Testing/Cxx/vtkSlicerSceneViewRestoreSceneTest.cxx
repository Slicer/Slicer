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
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// Sequences logic includes
#include <vtkSlicerSequencesLogic.h>

// SceneView logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

namespace
{

vtkMRMLScene* createScene();
int restoreEditAndRestore();
int removeRestoreEditAndRestore();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkSlicerSceneViewRestoreSceneTest(int vtkNotUsed(argc),
                                         char * vtkNotUsed(argv)[] )
{
  CHECK_EXIT_SUCCESS(restoreEditAndRestore());
  CHECK_EXIT_SUCCESS(removeRestoreEditAndRestore());
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
vtkMRMLScene* createScene()
{
  vtkMRMLScene* scene = vtkMRMLScene::New();

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  scene->AddNode(volumeNode);
  volumeNode->CreateDefaultDisplayNodes();


  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode);
  modelNode->CreateDefaultDisplayNodes();


  return scene;
}

//---------------------------------------------------------------------------
int restoreEditAndRestore()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  return EXIT_SUCCESS;
}


//---------------------------------------------------------------------------
int removeRestoreEditAndRestore()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene);

  vtkNew<vtkSlicerSequencesLogic> sequencesLogic;
  sequencesLogic->SetMRMLScene(scene);
  sequencesLogic->SetMRMLApplicationLogic(applicationLogic);
  applicationLogic->SetModuleLogic("Sequences", sequencesLogic);

  vtkNew<vtkSlicerSceneViewsModuleLogic> sceneViewLogic;
  sceneViewLogic->SetMRMLScene(scene);
  sceneViewLogic->SetMRMLApplicationLogic(applicationLogic);
  applicationLogic->SetModuleLogic("SceneViews", sceneViewLogic);


  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLScalarVolumeNode1"));
  volumeNode->CreateDefaultDisplayNodes();
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLModelNode1"));

  const char* sceneView1Name = "SceneView1";
  volumeNode->SetDisplayVisibility(true);
  modelNode->SetDisplayVisibility(true);
  sceneViewLogic->CreateSceneView(sceneView1Name, "First scene view", 0, nullptr);
  CHECK_BOOL(volumeNode->GetDisplayNode()->GetVisibility(), true);
  CHECK_BOOL(modelNode->GetDisplayNode()->GetVisibility(), true);

  const char* sceneView2Name = "SceneView2";
  volumeNode->SetDisplayVisibility(false);
  modelNode->SetDisplayVisibility(true);
  sceneViewLogic->CreateSceneView(sceneView2Name, "Second scene view", 0, nullptr);
  CHECK_BOOL(volumeNode->GetDisplayNode()->GetVisibility(), false);
  CHECK_BOOL(modelNode->GetDisplayNode()->GetVisibility(), true);

  const char* sceneView3Name = "SceneView3";
  volumeNode->SetDisplayVisibility(false);
  modelNode->SetDisplayVisibility(false);
  sceneViewLogic->CreateSceneView(sceneView3Name, "Third scene view", 0, nullptr);
  CHECK_BOOL(volumeNode->GetDisplayNode()->GetVisibility(), false);
  CHECK_BOOL(modelNode->GetDisplayNode()->GetVisibility(), false);

  // Markups node not included in the scene views.
  vtkNew<vtkMRMLMarkupsFiducialNode> fiducialNode;
  scene->AddNode(fiducialNode);
  fiducialNode->CreateDefaultDisplayNodes();
  fiducialNode->SetDisplayVisibility(true);

  // Scene view 1: Volume and model nodes visible
  sceneViewLogic->RestoreSceneView(sceneView1Name);
  CHECK_BOOL(volumeNode->GetDisplayNode()->GetVisibility(), true);
  CHECK_BOOL(modelNode->GetDisplayNode()->GetVisibility(), true);
  CHECK_BOOL(fiducialNode->GetDisplayNode()->GetVisibility(), true);

  // Scene view 2: Volume node not visible, model node visible
  sceneViewLogic->RestoreSceneView(sceneView2Name);
  CHECK_BOOL(volumeNode->GetDisplayNode()->GetVisibility(), false);
  CHECK_BOOL(modelNode->GetDisplayNode()->GetVisibility(), true);
  CHECK_BOOL(fiducialNode->GetDisplayNode()->GetVisibility(), true);

  // Scene view 3: Volume and model nodes not visible
  sceneViewLogic->RestoreSceneView(sceneView3Name);
  CHECK_BOOL(volumeNode->GetDisplayNode()->GetVisibility(), false);
  CHECK_BOOL(modelNode->GetDisplayNode()->GetVisibility(), false);
  CHECK_BOOL(fiducialNode->GetDisplayNode()->GetVisibility(), true);

  // Markups node not included in the scene views.
  // Should be unaffected by scene view.
  fiducialNode->SetDisplayVisibility(false);
  sceneViewLogic->RestoreSceneView(sceneView1Name);
  CHECK_BOOL(fiducialNode->GetDisplayNode()->GetVisibility(), false);
  sceneViewLogic->RestoreSceneView(sceneView2Name);
  CHECK_BOOL(fiducialNode->GetDisplayNode()->GetVisibility(), false);
  sceneViewLogic->RestoreSceneView(sceneView3Name);
  CHECK_BOOL(fiducialNode->GetDisplayNode()->GetVisibility(), false);

  return EXIT_SUCCESS;
}

} // end of anonymous namespace
