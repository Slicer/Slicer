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
bool restoreEditAndRestore();
bool removeRestoreEditAndRestore();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneViewNodeRestoreSceneTest(int vtkNotUsed(argc),
                                         char * vtkNotUsed(argv)[] )
{
  bool res = true;
  res = restoreEditAndRestore() && res;
  res = removeRestoreEditAndRestore() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
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
bool restoreEditAndRestore()
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

  if (strcmp(volumeNode->GetDisplayNodeID(), "vtkMRMLScalarVolumeDisplayNode1") != 0)
    {
    std::cout << __LINE__ << "vtkMRMLSceneViewNode::RestoreScene failed"
              << std::endl;
    return false;
    }
  return true;
}


//---------------------------------------------------------------------------
bool removeRestoreEditAndRestore()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLScalarVolumeNode1"));
  scene->RemoveNode(volumeNode);

  sceneViewNode->RestoreScene();

  vtkMRMLScalarVolumeNode* restoredVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLScalarVolumeNode1"));
  restoredVolumeNode->SetAndObserveDisplayNodeID("vtkMRMLScalarVolumeDisplayNode2");

  sceneViewNode->RestoreScene();

  if (restoredVolumeNode->GetDisplayNodeID() == 0 ||
      strcmp(restoredVolumeNode->GetDisplayNodeID(), "vtkMRMLScalarVolumeDisplayNode1") != 0)
    {
    std::cout << __LINE__ << ": vtkMRMLSceneViewNode::RestoreScene failed"
              << std::endl;
    return false;
    }
  return true;
}

} // end of anonymous namespace
