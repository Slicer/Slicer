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

vtkMRMLScene* createScene();
bool store();
bool storeAndRestore();
bool storeAndRemoveVolume();
bool storeTwice();
bool storeAndRestoreTwice();
bool storeTwiceAndRemoveVolume();

//---------------------------------------------------------------------------
int vtkMRMLSceneViewNodeStoreSceneTest(int vtkNotUsed(argc),
                                       char * vtkNotUsed(argv)[] )
{
  if (store() != EXIT_SUCCESS)
    {
    std::cerr << "store call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (storeAndRestore() != EXIT_SUCCESS)
    {
    std::cerr << "storeAndRestore call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (storeAndRemoveVolume() != EXIT_SUCCESS)
    {
    std::cerr << "storeAndRemoveVolume call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (storeTwice() != EXIT_SUCCESS)
    {
    std::cerr << "storeTwice call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (storeAndRestoreTwice() != EXIT_SUCCESS)
    {
    std::cerr << "storeAndRestoreTwice call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (storeTwiceAndRemoveVolume() != EXIT_SUCCESS)
    {
    std::cerr << "storeTwiceAndRemoveVolume call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Scene View Node Store Scene Test succeeded" << std::endl;
  return EXIT_SUCCESS;
}

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
bool store()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool storeAndRestore()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();
  sceneViewNode->RestoreScene();
  
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool storeAndRemoveVolume()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();
  vtkMRMLNode* node = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  scene->RemoveNode(node);
  sceneViewNode->RestoreScene();
  sceneViewNode->RestoreScene();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool storeTwice()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();
  sceneViewNode->StoreScene();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool storeAndRestoreTwice()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();
  sceneViewNode->StoreScene();

  sceneViewNode->RestoreScene();
  sceneViewNode->RestoreScene();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool storeTwiceAndRemoveVolume()
{
  vtkSmartPointer<vtkMRMLScene> scene;
  scene.TakeReference(createScene());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();
  sceneViewNode->StoreScene();
  vtkMRMLNode* node = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  scene->RemoveNode(node);
  sceneViewNode->RestoreScene();
  sceneViewNode->RestoreScene();

  return EXIT_SUCCESS;
}
