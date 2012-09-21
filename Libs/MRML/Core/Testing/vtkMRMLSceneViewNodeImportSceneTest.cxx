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
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

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

  vtkNew<vtkMRMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());

  displayNode->AddViewNodeID(viewNode->GetID());

  if (saveInSceneView)
    {
    vtkNew<vtkMRMLSceneViewNode> sceneViewNode;
    scene->AddNode(sceneViewNode.GetPointer());

    sceneViewNode->StoreScene();
    /*
    vtkNew<vtkMRMLHierarchyNode> topLevelHierarchyNode;
    scene->AddNode(topLevelHierarchyNode.GetPointer());

    vtkNew<vtkMRMLHierarchyNode> sceneViewHierarchyNode;
    scene->AddNode(sceneViewHierarchyNode.GetPointer());

    sceneViewHierarchyNode->SetParentNodeID(topLevelHieararchyNode->GetID());
    sceneViewHierarchyNode->SetAssociatedNodeID(sceneViewNode);
    */
    }

  scene->RemoveNode(displayNode.GetPointer());
}

//---------------------------------------------------------------------------
bool import()
{
  // Save a scene containing a viewnode and a sceneview node.
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer(), true);

  // scene
  //   + vtkMRMLViewNode1
  //   + vtkMRMLSceneViewNode1
  //       + vtkMRMLViewNode1
  //       + vtkMRMLScalarVolumeDisplayNode1
  //            -> ref: vtkMRMLViewNode1

  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  std::cout << xmlScene << std::endl;

  // Simulate another scene
  vtkNew<vtkMRMLScene> scene2;
  populateScene(scene2.GetPointer(), false);

  // scene2
  //   + vtkMRMLViewNode1

  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene);
  scene2->Import();

  // scene2
  //   + vtkMRMLViewNode1 (original)
  //   + vtkMRMLViewNode2 (imported)
  //   + vtkMRMLSceneViewNode1 (imported)
  //       + vtkMRMLViewNode2
  //       + vtkMRMLScalarVolumeDisplayNode1
  //            -> ref: vtkMRMLViewNode2


  // Check scene node IDs
  vtkMRMLNode* viewNode = scene2->GetNthNodeByClass(0, "vtkMRMLViewNode");
  vtkMRMLNode* viewNode2 = scene2->GetNthNodeByClass(1, "vtkMRMLViewNode");
  vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(
    scene2->GetNthNodeByClass(0, "vtkMRMLSceneViewNode"));
  if (!viewNode || !viewNode2 || !sceneViewNode
      || strcmp(viewNode->GetID(), "vtkMRMLViewNode1") != 0
      || strcmp(viewNode2->GetID(), "vtkMRMLViewNode2") != 0
      || strcmp(sceneViewNode->GetID(), "vtkMRMLSceneViewNode1") != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    Original view node ID is " << viewNode->GetID()
              << " instead of vtkMRMLViewNode1." << std::endl
              << "    Imported view node ID is " << viewNode2->GetID()
              << " instead of vtkMRMLViewNode2." << std::endl
              << "    Scene view node ID is " << sceneViewNode->GetID()
              << " instead of vtkMRMLSceneViewNode." << std::endl;
    return false;
    }

  // Check sceneViewNode node IDs.
  vtkMRMLNode* sceneViewViewNode =
    sceneViewNode->GetNodes()->GetNthNodeByClass(0, "vtkMRMLViewNode");
  vtkMRMLDisplayNode* sceneViewDisplayNode = vtkMRMLDisplayNode::SafeDownCast(
    sceneViewNode->GetNodes()->GetNthNodeByClass(
      0, "vtkMRMLScalarVolumeDisplayNode"));
  if (sceneViewNode->GetNodes()->GetNumberOfNodes() != 2
      || !sceneViewViewNode || !sceneViewDisplayNode
      || strcmp(sceneViewViewNode->GetID(), "vtkMRMLViewNode2") != 0
      || strcmp(sceneViewDisplayNode->GetID(),
                "vtkMRMLScalarVolumeDisplayNode1") != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    Scene view View node ID is " << sceneViewViewNode->GetID()
              << " instead of vtkMRMLViewNode2." << std::endl
              << "    Scene view Display node ID is " << sceneViewDisplayNode->GetID()
              << " instead of vtkMRMLScalarVolumeDisplayNode1." << std::endl;
    return false;
    }

  // Check references
  if (strcmp(sceneViewDisplayNode->GetNthViewNodeID(0),
             sceneViewViewNode->GetID()) != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    Display node references are not updated. "
              << "Display node view node ID is " 
              << sceneViewDisplayNode->GetNthViewNodeID(0)
              << " instead of " << sceneViewViewNode->GetID() << std::endl;
    return false;
    }

  return true;
}

}
