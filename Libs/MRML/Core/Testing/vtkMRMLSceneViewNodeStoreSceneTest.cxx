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
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

namespace
{

void populateScene(vtkMRMLScene* scene);
int store();
int storeAndRestore();
int storeAndRemoveVolume();
int storeTwice();
int storeAndRestoreTwice();
int storeTwiceAndRemoveVolume();
int references();
int storePerformance();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneViewNodeStoreSceneTest(int vtkNotUsed(argc),
                                       char * vtkNotUsed(argv)[] )
{
  CHECK_EXIT_SUCCESS(store());
  CHECK_EXIT_SUCCESS(storeAndRestore());
  CHECK_EXIT_SUCCESS(storeAndRemoveVolume());
  CHECK_EXIT_SUCCESS(storeTwice());
  CHECK_EXIT_SUCCESS(storeAndRestoreTwice());
  CHECK_EXIT_SUCCESS(storeTwiceAndRemoveVolume());
  CHECK_EXIT_SUCCESS(references());
  CHECK_EXIT_SUCCESS(storePerformance());
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
void populateScene(vtkMRMLScene* scene)
{
  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  volumeNode->SetScene(scene);
  scene->AddNode(volumeNode.GetPointer());
  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
}

//---------------------------------------------------------------------------
int store()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  CHECK_NOT_NULL(sceneViewNode->GetStoredScene()->GetNodeByID("vtkMRMLScalarVolumeNode1"));

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeAndRestore()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  vtkMRMLNode* volumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");

  sceneViewNode->StoreScene();

  sceneViewNode->RestoreScene();

  vtkMRMLNode* restoredVolumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  // Restoring the volume should re-use the same node.
  CHECK_POINTER(restoredVolumeNode, volumeNode);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeAndRemoveVolume()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  // Remove node from the scene to see if it gets restored.
  vtkMRMLNode* volumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  scene->RemoveNode(volumeNode);

  // The following instantiates new scalar volume nodes. Doing so will ensure
  // that the space that was allocated for 'vtkMRMLScalarVolumeNode1' won't be
  // available again for the restored node.
  vtkNew<vtkCollection> allocatedSpace;
  allocatedSpace->AddItem(vtkNew<vtkMRMLScalarVolumeNode>().GetPointer());
  allocatedSpace->AddItem(vtkNew<vtkMRMLScalarVolumeNode>().GetPointer());
  allocatedSpace->AddItem(vtkNew<vtkMRMLScalarVolumeNode>().GetPointer());
  allocatedSpace->AddItem(vtkNew<vtkMRMLScalarVolumeNode>().GetPointer());
  (void)allocatedSpace;

  // TODO: We expect errors here because of https://github.com/Slicer/Slicer/issues/2816 is not resolved.
  // Once that bug is fixed, RestoreScene() should not throw any errors, and so the
  // TESTING_OUTPUT_ASSERT_ERRORS_BEGIN/END macros should be removed.
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  sceneViewNode->RestoreScene();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // Make sure the node has been restored.
  vtkMRMLNode* restoredVolumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  CHECK_NOT_NULL(restoredVolumeNode);
  CHECK_POINTER_DIFFERENT(restoredVolumeNode, volumeNode)

  sceneViewNode->RestoreScene();

  // Make sure the node has been restored.
  vtkMRMLNode* rerestoredVolumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  CHECK_NOT_NULL(rerestoredVolumeNode);
  CHECK_POINTER(rerestoredVolumeNode, restoredVolumeNode)

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeTwice()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  // Empty scene view nodes until "stored"
  int defaultNodes = sceneViewNode->GetStoredScene() ?
    sceneViewNode->GetStoredScene()->GetNumberOfNodes() : 0;
  CHECK_INT(defaultNodes, 0);

  sceneViewNode->StoreScene();

  // a storage node gets added
  int nodeCount = sceneViewNode->GetStoredScene()->GetNumberOfNodes();
  CHECK_INT(nodeCount, 3);

  sceneViewNode->StoreScene();

  int newNodeCount = sceneViewNode->GetStoredScene()->GetNumberOfNodes();
  CHECK_INT(newNodeCount, nodeCount);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeAndRestoreTwice()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  sceneViewNode->StoreScene();

  sceneViewNode->RestoreScene();
  sceneViewNode->RestoreScene();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storeTwiceAndRemoveVolume()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  sceneViewNode->StoreScene();

  vtkMRMLNode* node = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  scene->RemoveNode(node);

  // TODO: We expect errors here because of https://github.com/Slicer/Slicer/issues/2816 is not resolved.
  // Once that bug is fixed, RestoreScene() should not throw any errors, and so the
  // TESTING_OUTPUT_ASSERT_ERRORS_BEGIN/END macros should be removed.
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  sceneViewNode->RestoreScene();
  sceneViewNode->RestoreScene();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int references()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  vtkMRMLNode* volumeNode =
    scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  vtkSmartPointer<vtkCollection> sceneReferencedNodes;
  sceneReferencedNodes.TakeReference(
    scene->GetReferencedNodes(volumeNode));
  CHECK_INT(sceneReferencedNodes->GetNumberOfItems(), 2);

  sceneViewNode->StoreScene();

  vtkMRMLNode* sceneViewVolumeNode =
    sceneViewNode->GetStoredScene()->GetNodeByID("vtkMRMLScalarVolumeNode1");
  vtkMRMLNode* sceneViewVolumeDisplayNode =
    sceneViewNode->GetStoredScene()->GetNodeByID("vtkMRMLScalarVolumeDisplayNode1");

  vtkSmartPointer<vtkCollection> referencedNodes;
  referencedNodes.TakeReference(scene->GetReferencedNodes(volumeNode));
  vtkSmartPointer<vtkCollection> sceneViewReferencedNodes;
  sceneViewReferencedNodes.TakeReference(
    sceneViewNode->GetStoredScene()->GetReferencedNodes(sceneViewVolumeNode));

  // Number of references in scene view node scene should be the same as the
  // main scene reference count (+1 for new storage node).
  CHECK_INT(sceneViewReferencedNodes->GetNumberOfItems(), 3);
  CHECK_POINTER(sceneViewReferencedNodes->GetItemAsObject(0), sceneViewVolumeNode);
  CHECK_POINTER(sceneViewReferencedNodes->GetItemAsObject(1), sceneViewVolumeDisplayNode);
  CHECK_INT(sceneViewNode->GetStoredScene()->GetNumberOfNodeReferences(), 2);
  CHECK_POINTER(sceneViewNode->GetStoredScene()->GetNthReferencingNode(0), sceneViewVolumeNode);

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int storePerformance()
{
  // This test is for performance
  vtkNew<vtkMRMLScene> scene;
  const int displayNodePairCount = 100;

  for (int i = 0; i < displayNodePairCount; ++i)
    {
    populateScene(scene.GetPointer());
    }

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  vtkNew<vtkTimerLog> timer;
  timer->StartTimer();
  sceneViewNode->StoreScene();
  timer->StopTimer();

  std::cout<< "<DartMeasurement name=\"vtkMRMLSceneViewNode-StorePerformance-"
           << displayNodePairCount <<"\" type=\"numeric/double\">"
           << timer->GetElapsedTime() << "</DartMeasurement>" << std::endl;

  return EXIT_SUCCESS;
}

} // end of anonymous namespace
