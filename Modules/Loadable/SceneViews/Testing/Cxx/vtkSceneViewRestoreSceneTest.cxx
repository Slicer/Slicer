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
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// Sequences logic includes
#include <vtkSlicerSequencesLogic.h>

// SceneView logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

namespace
{
int test_SceneView_SaveAndRestore(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);
int test_SceneView_SingletonNode(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic);

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkSceneViewRestoreSceneTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
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

  CHECK_EXIT_SUCCESS(test_SceneView_SaveAndRestore(scene, sceneViewLogic));
  CHECK_EXIT_SUCCESS(test_SceneView_SingletonNode(scene, sceneViewLogic));
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
int test_SceneView_SaveAndRestore(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  scene->Clear(1);

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  scene->AddNode(volumeNode);
  volumeNode->CreateDefaultDisplayNodes();

  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode);
  modelNode->CreateDefaultDisplayNodes();

  const char* sceneView1Name = "SceneView1";
  volumeNode->SetDisplayVisibility(true);
  modelNode->SetDisplayVisibility(true);
  sceneViewLogic->CreateSceneView(sceneView1Name, "First scene view");
  CHECK_BOOL(volumeNode->GetDisplayNode()->GetVisibility(), true);
  CHECK_BOOL(modelNode->GetDisplayNode()->GetVisibility(), true);

  const char* sceneView2Name = "SceneView2";
  volumeNode->SetDisplayVisibility(false);
  modelNode->SetDisplayVisibility(true);
  sceneViewLogic->CreateSceneView(sceneView2Name, "Second scene view");
  CHECK_BOOL(volumeNode->GetDisplayNode()->GetVisibility(), false);
  CHECK_BOOL(modelNode->GetDisplayNode()->GetVisibility(), true);

  const char* sceneView3Name = "SceneView3";
  volumeNode->SetDisplayVisibility(false);
  modelNode->SetDisplayVisibility(false);
  sceneViewLogic->CreateSceneView(sceneView3Name, "Third scene view");
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

//---------------------------------------------------------------------------
int test_SceneView_SingletonNode(vtkMRMLScene* scene, vtkSlicerSceneViewsModuleLogic* sceneViewLogic)
{
  scene->Clear(1);

  // Add singleton node
  vtkNew<vtkMRMLScalarVolumeNode> singletonNode;
  singletonNode->SetSingletonTag("Singleton");
  vtkMRMLNode* addedNode = scene->AddNode(singletonNode);
  CHECK_POINTER(singletonNode, addedNode);

  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(10, 10, 10);
  imageData->AllocateScalars(VTK_CHAR, 1);
  singletonNode->SetAndObserveImageData(imageData);

  std::vector<vtkMRMLNode*> savedNodes;
  savedNodes.push_back(singletonNode);

  // Save scene view with specific nodes
  std::string sceneView1Name = "SceneView10x10x10";
  sceneViewLogic->CreateSceneView(savedNodes, sceneView1Name);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[0], 10);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[1], 10);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[2], 10);

  // Change singleton node
  std::string sceneView2Name = "SceneView20x20x20";
  singletonNode->GetImageData()->SetDimensions(20, 20, 20);
  sceneViewLogic->CreateSceneView(savedNodes, sceneView2Name);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[0], 20);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[1], 20);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[2], 20);

  // Restore scene view
  sceneViewLogic->RestoreSceneView(sceneView1Name);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[0], 10);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[1], 10);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[2], 10);

  sceneViewLogic->RestoreSceneView(sceneView2Name);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[0], 20);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[1], 20);
  CHECK_INT(singletonNode->GetImageData()->GetDimensions()[2], 20);

  return EXIT_SUCCESS;
}

} // end of anonymous namespace
