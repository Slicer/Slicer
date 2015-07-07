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
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

namespace
{

void populateScene(vtkMRMLScene* scene);
bool store();
bool storeAndRestore();
bool storeAndRemoveVolume();
bool storeTwice();
bool storeAndRestoreTwice();
bool storeTwiceAndRemoveVolume();
bool references();
bool storePerformance();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneViewNodeStoreSceneTest(int vtkNotUsed(argc),
                                       char * vtkNotUsed(argv)[] )
{
  if (!store())
    {
    std::cerr << "store call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (!storeAndRestore())
    {
    std::cerr << "storeAndRestore call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (!storeAndRemoveVolume())
    {
    std::cerr << "storeAndRemoveVolume call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (!storeTwice())
    {
    std::cerr << "storeTwice call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (!storeAndRestoreTwice())
    {
    std::cerr << "storeAndRestoreTwice call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (!storeTwiceAndRemoveVolume())
    {
    std::cerr << "storeTwiceAndRemoveVolume call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (!references())
    {
    std::cerr << "references call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  if (!storePerformance())
    {
    std::cerr << "updateNodeIDs call not successful." << std::endl;
    return EXIT_FAILURE;
    }

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
bool store()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();

  if (sceneViewNode->GetStoredScene()->GetNodeByID("vtkMRMLScalarVolumeNode1") == 0)
    {
    std::cout << "vtkMRMLSceneViewNode::StoreScene() failed, expected no volume node" << std::endl;
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool storeAndRestore()
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
  if (restoredVolumeNode != volumeNode)
    {
    std::cout << __LINE__ << ": vtkMRMLSceneViewNode::RestoreScene() failed"
              << std::endl;
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool storeAndRemoveVolume()
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

  sceneViewNode->RestoreScene();

  // Make sure the node has been restored.
  vtkMRMLNode* restoredVolumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  if (restoredVolumeNode == 0 ||
      restoredVolumeNode == volumeNode)
    {
    std::cout << __LINE__ << ": vtkMRMLSceneViewNode::RestoreScene() failed"
              << std::endl;
    return false;
    }

  sceneViewNode->RestoreScene();

  // Make sure the node has been restored.
  vtkMRMLNode* rerestoredVolumeNode = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  if (rerestoredVolumeNode == 0 ||
      rerestoredVolumeNode != restoredVolumeNode)
    {
    std::cout << __LINE__ << ": vtkMRMLSceneViewNode::RestoreScene() failed"
              << std::endl;
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool storeTwice()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  // Empty scene view nodes until "stored"
  int defaultNodes = sceneViewNode->GetStoredScene() ?
    sceneViewNode->GetStoredScene()->GetNumberOfNodes() : 0;
  if (defaultNodes != 0)
    {
    std::cout << __LINE__ << ": vtkMRMLSceneViewNode::vtkMRMLSceneViewNode()"
              << " failed, default nodes != 0: "
              << defaultNodes
              << std::endl;
    return false;
    }
  sceneViewNode->StoreScene();

  int nodeCount = sceneViewNode->GetStoredScene()->GetNumberOfNodes();
  // a storage node gets added
  if (nodeCount != 3)
    {
    std::cout << __LINE__ << ": vtkMRMLSceneViewNode::StoreScene() failed"
              << ", node count " << nodeCount << " != 3"
              << std::endl;
    return false;
    }
  sceneViewNode->StoreScene();

  int newNodeCount = sceneViewNode->GetStoredScene()->GetNumberOfNodes();
  if (newNodeCount != nodeCount)
    {
    std::cout << __LINE__ << ": vtkMRMLSceneViewNode::StoreScene() failed"
              << ", new node count " << newNodeCount
              << " != node count " << nodeCount
              << std::endl;
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool storeAndRestoreTwice()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();
  sceneViewNode->StoreScene();

  sceneViewNode->RestoreScene();
  sceneViewNode->RestoreScene();

  return true;
}

//---------------------------------------------------------------------------
bool storeTwiceAndRemoveVolume()
{
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
  scene->AddNode(sceneViewNode.GetPointer());

  sceneViewNode->StoreScene();
  sceneViewNode->StoreScene();
  vtkMRMLNode* node = scene->GetNodeByID("vtkMRMLScalarVolumeNode1");
  scene->RemoveNode(node);
  sceneViewNode->RestoreScene();
  sceneViewNode->RestoreScene();

  return true;
}

//---------------------------------------------------------------------------
bool references()
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
  if (sceneReferencedNodes->GetNumberOfItems() != 2)
    {
    std::cout << __LINE__ << ": vtkMRMLScene::GetReferencedNodes() failed. \n"
              << sceneReferencedNodes->GetNumberOfItems() << " items != 2"
              << std::endl;
    return false;
    }

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
  if (sceneViewReferencedNodes->GetNumberOfItems() != 3 ||
      sceneViewReferencedNodes->GetItemAsObject(0) != sceneViewVolumeNode ||
      sceneViewReferencedNodes->GetItemAsObject(1) != sceneViewVolumeDisplayNode ||
      sceneViewNode->GetStoredScene()->GetNumberOfNodeReferences() != 2 ||
      sceneViewNode->GetStoredScene()->GetNthReferencingNode(0) != sceneViewVolumeNode)
    {
    std::cout << __LINE__ << ": vtkMRMLSceneViewNode::StoreScene() failed." << std::endl
              << sceneViewReferencedNodes->GetNumberOfItems() << " items (3 expected) "
              << sceneViewVolumeNode << ": "
              << sceneViewReferencedNodes->GetItemAsObject(0) << ", "
              << sceneViewReferencedNodes->GetItemAsObject(1) << std::endl;
    unsigned int referencingNodesSize = sceneViewNode->GetStoredScene()->GetNumberOfNodeReferences();
    std::cout << "Referencing nodes: " << referencingNodesSize << "(2 expected) ";
    for (unsigned int i = 0; i < referencingNodesSize; ++i)
      {
      std::cout << sceneViewNode->GetStoredScene()->GetNthReferencingNode(i) << "("
                << sceneViewNode->GetStoredScene()->GetNthReferencingNode(i)->GetID() << ") ";
      }
    std::cout << std::endl;
    std::cout << referencedNodes->GetNumberOfItems() << " items in scene for "
              << volumeNode << ": "
              << referencedNodes->GetItemAsObject(0) << ", "
              << referencedNodes->GetItemAsObject(1) << std::endl;
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool storePerformance()
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
  return true;
}

} // end of anonymous namespace
