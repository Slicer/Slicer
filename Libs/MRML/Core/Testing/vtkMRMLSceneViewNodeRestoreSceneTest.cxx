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
#include "vtkMRMLSceneViewNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

namespace
{

vtkMRMLScene* createScene();
int restoreEditAndRestore();
int removeRestoreEditAndRestore();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneViewNodeRestoreSceneTest(int vtkNotUsed(argc),
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

  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  volumeNode->SetScene(scene);
  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(volumeNode.GetPointer());

  return scene;
}

//---------------------------------------------------------------------------
int restoreEditAndRestore()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  sceneViewNode->RestoreScene();

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLScalarVolumeNode1"));
  volumeNode->SetAndObserveDisplayNodeID("vtkMRMLScalarVolumeDisplayNode2");

  sceneViewNode->RestoreScene();

  CHECK_STRING(volumeNode->GetDisplayNodeID(), "vtkMRMLScalarVolumeDisplayNode1");

  return EXIT_SUCCESS;
}


//---------------------------------------------------------------------------
int removeRestoreEditAndRestore()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLScalarVolumeNode1"));
  scene->RemoveNode(volumeNode);

  // TODO: We expect errors here because of https://github.com/Slicer/Slicer/issues/2816 is not resolved.
  // Once that bug is fixed, RestoreScene() should not throw any errors, and so the
  // TESTING_OUTPUT_ASSERT_ERRORS_BEGIN/END macros should be removed.
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  sceneViewNode->RestoreScene();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  vtkMRMLScalarVolumeNode* restoredVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLScalarVolumeNode1"));
  restoredVolumeNode->SetAndObserveDisplayNodeID("vtkMRMLScalarVolumeDisplayNode2");

  sceneViewNode->RestoreScene();

  CHECK_NOT_NULL(restoredVolumeNode->GetDisplayNodeID());
  CHECK_STRING(restoredVolumeNode->GetDisplayNodeID(), "vtkMRMLScalarVolumeDisplayNode1");

  return EXIT_SUCCESS;
}

} // end of anonymous namespace
