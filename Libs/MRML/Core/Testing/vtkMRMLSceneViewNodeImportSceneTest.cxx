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

int populateScene(vtkMRMLScene* scene, bool saveInSceneView)
{
  vtkNew<vtkMRMLSceneViewNode> sceneViewtoRegister;
  scene->RegisterNodeClass(sceneViewtoRegister.GetPointer());

  vtkNew<vtkMRMLScalarVolumeNode> displayableNode;
  scene->AddNode(displayableNode.GetPointer());

  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());

  displayableNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  if (saveInSceneView)
    {
    vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
    scene->AddNode(sceneViewNode.GetPointer());

    sceneViewNode->StoreScene();
    }

  scene->RemoveNode(displayableNode.GetPointer());
  return EXIT_SUCCESS;
}

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneViewNodeImportSceneTest(int vtkNotUsed(argc),
                                       char * vtkNotUsed(argv)[] )
{
  // Save a scene containing a viewnode and a sceneview node.
  vtkNew<vtkMRMLScene> scene;
  CHECK_EXIT_SUCCESS(populateScene(scene.GetPointer(), true));

  // scene
  //   + vtkMRMLScalarVolumeDisplayNode1
  //   + vtkMRMLSceneViewNode1
  //       + vtkMRMLScalarVolumeNode1
  //            -> ref: vtkMRMLScalarVolumeDisplayNode1
  //       + vtkMRMLScalarVolumeDisplayNode1

  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  std::cout << xmlScene << std::endl;

  // Simulate another scene
  vtkNew<vtkMRMLScene> scene2;
  CHECK_EXIT_SUCCESS(populateScene(scene2.GetPointer(), false));

  // scene2
  //   + vtkMRMLScalarVolumeDisplayNode1

  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene);
  scene2->Import();

  // scene2
  //   + vtkMRMLScalarVolumeDisplayNode1 (original)
  //   + vtkMRMLScalarVolumeDisplayNode2 (imported)
  //   + vtkMRMLSceneViewNode1 (imported)
  //       + vtkMRMLScalarVolumeNode1
  //            -> ref: vtkMRMLScalarVolumeDisplayNode2
  //       + vtkMRMLScalarVolumeDisplayNode2

  // Check scene node IDs
  vtkMRMLNode* displayNode =
    scene2->GetFirstNodeByClass("vtkMRMLScalarVolumeDisplayNode");
  vtkMRMLNode* displayNode2 = scene2->GetNthNodeByClass(1, "vtkMRMLScalarVolumeDisplayNode");
  vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(
    scene2->GetFirstNodeByClass("vtkMRMLSceneViewNode"));

  CHECK_NOT_NULL(displayNode);
  CHECK_NOT_NULL(displayNode2);
  CHECK_NOT_NULL(sceneViewNode);
  CHECK_STRING(displayNode->GetID(), "vtkMRMLScalarVolumeDisplayNode1");
  CHECK_STRING(displayNode2->GetID(), "vtkMRMLScalarVolumeDisplayNode2");
  CHECK_STRING(sceneViewNode->GetID(), "vtkMRMLSceneViewNode1");

  // Check sceneViewNode node IDs.
  vtkMRMLNode* sceneViewDisplayNode =
    sceneViewNode->GetStoredScene()->GetFirstNodeByClass("vtkMRMLScalarVolumeDisplayNode");
  vtkMRMLDisplayableNode* sceneViewDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast(
    sceneViewNode->GetStoredScene()->GetFirstNodeByClass("vtkMRMLScalarVolumeNode"));

  CHECK_INT(sceneViewNode->GetStoredScene()->GetNumberOfNodes(), 3);
  CHECK_NOT_NULL(sceneViewDisplayNode);
  CHECK_NOT_NULL(sceneViewDisplayableNode);
  CHECK_STRING(sceneViewDisplayNode->GetID(), "vtkMRMLScalarVolumeDisplayNode2");
  CHECK_STRING(sceneViewDisplayableNode->GetID(), "vtkMRMLScalarVolumeNode1");

  // Check references
  CHECK_STRING(sceneViewDisplayableNode->GetNthDisplayNodeID(0), sceneViewDisplayNode->GetID());

  return EXIT_SUCCESS;
}
