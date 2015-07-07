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

void populateScene(vtkMRMLScene* scene, bool saveInSceneView);
bool import();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneViewNodeImportSceneTest(int vtkNotUsed(argc),
                                       char * vtkNotUsed(argv)[] )
{
  if (!import())
    {
    std::cerr << "import call not successful." << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
void populateScene(vtkMRMLScene* scene, bool saveInSceneView)
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
}

//---------------------------------------------------------------------------
bool import()
{
  // Save a scene containing a viewnode and a sceneview node.
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer(), true);

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
  populateScene(scene2.GetPointer(), false);

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
    scene2->GetNthNodeByClass(0, "vtkMRMLScalarVolumeDisplayNode");
  vtkMRMLNode* displayNode2 = scene2->GetNthNodeByClass(1, "vtkMRMLScalarVolumeDisplayNode");
  vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(
    scene2->GetNthNodeByClass(0, "vtkMRMLSceneViewNode"));
  if (!displayNode || !displayNode2 || !sceneViewNode
      || strcmp(displayNode->GetID(), "vtkMRMLScalarVolumeDisplayNode1") != 0
      || strcmp(displayNode2->GetID(), "vtkMRMLScalarVolumeDisplayNode2") != 0
      || strcmp(sceneViewNode->GetID(), "vtkMRMLSceneViewNode1") != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    Original display node ID is " << displayNode->GetID()
              << " instead of vtkMRMLScalarVolumeDisplayNode1." << std::endl
              << "    Imported display node ID is " << displayNode2->GetID()
              << " instead of vtkMRMLScalarVolumeDisplayNode2." << std::endl
              << "    Scene view node ID is " << sceneViewNode->GetID()
              << " instead of vtkMRMLSceneViewNode." << std::endl;
    return false;
    }

  // Check sceneViewNode node IDs.
  vtkMRMLNode* sceneViewDisplayNode =
    sceneViewNode->GetStoredScene()->GetNthNodeByClass(0, "vtkMRMLScalarVolumeDisplayNode");
  vtkMRMLDisplayableNode* sceneViewDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast(
    sceneViewNode->GetStoredScene()->GetNthNodeByClass(
      0, "vtkMRMLScalarVolumeNode"));
  if (sceneViewNode->GetStoredScene()->GetNumberOfNodes() != 3
      || !sceneViewDisplayNode || !sceneViewDisplayableNode
      || strcmp(sceneViewDisplayNode->GetID(), "vtkMRMLScalarVolumeDisplayNode2") != 0
      || strcmp(sceneViewDisplayableNode->GetID(),
                "vtkMRMLScalarVolumeNode1") != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    Scene view Display node ID is " << sceneViewDisplayNode->GetID()
              << " instead of vtkMRMLScalarVolumeDisplayNode2." << std::endl
              << "    Scene view Displayable node ID is " << sceneViewDisplayableNode->GetID()
              << " instead of vtkMRMLScalarVolumeNode1." << std::endl;
    return false;
    }

  // Check references
  if (strcmp(sceneViewDisplayableNode->GetNthDisplayNodeID(0),
             sceneViewDisplayNode->GetID()) != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    Displayable node references are not updated. "
              << "Displayable node display node ID is "
              << sceneViewDisplayableNode->GetNthDisplayNodeID(0)
              << " instead of " << sceneViewDisplayNode->GetID() << std::endl;
    return false;
    }

  return true;
}

}
