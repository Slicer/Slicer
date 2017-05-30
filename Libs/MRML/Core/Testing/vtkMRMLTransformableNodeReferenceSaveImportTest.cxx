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
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

namespace
{

void populateScene(vtkMRMLScene* scene);
bool import();

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLTransformableNodeReferenceSaveImportTest(int vtkNotUsed(argc),
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
void populateScene(vtkMRMLScene* scene)
{

  vtkNew<vtkMRMLScalarVolumeNode> trnsformableableNode;
  scene->AddNode(trnsformableableNode.GetPointer());

  vtkNew<vtkMRMLLinearTransformNode> trnsformNode;
  scene->AddNode(trnsformNode.GetPointer());

  trnsformableableNode->SetAndObserveTransformNodeID(trnsformNode->GetID());

}

//---------------------------------------------------------------------------
bool import()
{
  // Save a scene containing a viewnode and a sceneview node.
  vtkNew<vtkMRMLScene> scene;
  populateScene(scene.GetPointer());

  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  std::cout << xmlScene << std::endl;

  // Simulate another scene
  vtkNew<vtkMRMLScene> scene2;

  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene);
  scene2->Import();

  // Check transform node IDs
  vtkMRMLNode* trnsformNode =
    scene2->GetFirstNodeByClass("vtkMRMLLinearTransformNode");
  if (!trnsformNode || strcmp(trnsformNode->GetID(), "vtkMRMLLinearTransformNode1") != 0
      )
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    transform node ID is " << trnsformNode->GetID()
              << " instead of vtkMRMLLinearTransformNode1." << std::endl;

    return false;
    }

  // Check references
  vtkMRMLTransformableNode* trnsformableNode =
    vtkMRMLTransformableNode::SafeDownCast(scene2->GetFirstNodeByClass("vtkMRMLTransformableNode"));

  if (strcmp(trnsformableNode->GetTransformNodeID(),
             trnsformNode->GetID()) != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    Transformable node references are not updated. "
              << "Transform node ID reference is "
              << trnsformableNode->GetTransformNodeID()
              << " instead of " << trnsformNode->GetID() << std::endl;
    return false;
    }

  return true;
}

}
