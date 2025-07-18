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
#include <vtkCacheManager.h>
#include <vtkDataIOManager.h>
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// VTK includes

#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// Sequences logic includes
#include <vtkSlicerSequencesLogic.h>

// SceneView logic includes
#include <vtkSlicerSceneViewsModuleLogic.h>

// ITK includes
#include <itkFactoryRegistration.h>

namespace
{

int populateScene(vtkMRMLScene* scene)
{
  scene->Clear(1);

  vtkNew<vtkMRMLModelNode> displayableNode;
  const vtkNew<vtkPolyData> polyData;
  displayableNode->SetAndObserveMesh(polyData);
  scene->AddNode(displayableNode);

  vtkNew<vtkMRMLModelDisplayNode> displayNode;
  scene->AddNode(displayNode);

  displayableNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkSceneViewImportSceneTest(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Missing temporary directory argument" << std::endl;
    return EXIT_FAILURE;
  }

  const char* const tempDir = argv[1];
  if (!tempDir)
  {
    std::cerr << "Missing temporary directory argument" << std::endl;
    return EXIT_FAILURE;
  }

  itk::itkFactoryRegistration();

  // Save a scene containing a viewnode and a sceneview node.
  vtkNew<vtkMRMLScene> scene;
  scene->SetDataIOManager(vtkNew<vtkDataIOManager>());
  scene->GetDataIOManager()->SetCacheManager(vtkNew<vtkCacheManager>());
  scene->GetDataIOManager()->GetCacheManager()->SetRemoteCacheDirectory(tempDir);

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

  CHECK_EXIT_SUCCESS(populateScene(scene));

  vtkNew<vtkImageData> screenshot;
  screenshot->SetDimensions(100, 100, 1);
  screenshot->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

  CHECK_INT(sceneViewLogic->GetNumberOfSceneViews(), 0);
  sceneViewLogic->CreateSceneView("SceneView1", "", 0, screenshot);
  CHECK_INT(sceneViewLogic->GetNumberOfSceneViews(), 1);

  // Save the scene
  const std::string fileName = std::string(tempDir) + "/vtkSceneViewNodeImportSceneTest.mrb";
  CHECK_BOOL(scene->WriteToMRB(fileName.c_str()), true);

  scene->Clear(1);
  CHECK_BOOL(scene->ReadFromMRB(fileName.c_str()), true);
  std::cout << fileName << std::endl;

  CHECK_INT(sceneViewLogic->GetNumberOfSceneViews(), 1);

  return EXIT_SUCCESS;
}
