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

// VolumeRendering includes
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLVolumePropertyStorageNode.h>
#include <vtkSlicerVolumeRenderingLogic.h>

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTestingOutputWindow.h>

//----------------------------------------------------------------------------
int testAddVolumePropertyFromFile(const std::string &temporaryDirectory);

//----------------------------------------------------------------------------
int vtkSlicerVolumeRenderingLogicAddFromFileTest(int argc, char* argv[])
{
  if (argc != 2)
    {
    std::cout << "Missing temporary directory argument !" << std::endl;
    return EXIT_FAILURE;
    }
  std::string temporaryDirectory(argv[1]);

  CHECK_EXIT_SUCCESS(testAddVolumePropertyFromFile(temporaryDirectory));
  
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int testAddVolumePropertyFromFile(const std::string& temporaryDirectory)
{
  vtkNew<vtkSlicerVolumeRenderingLogic> logic;

  std::cout << "temporaryDirectory = " << temporaryDirectory.c_str() << std::endl;

 
  // write out a defaults file
  vtkNew<vtkMRMLVolumePropertyNode> defaultVolumePropertyNode;
  vtkNew<vtkMRMLVolumePropertyStorageNode> volumePropertyStorageNode;

  // set up the temporary file name
  std::vector<std::string> components;
  components.push_back(temporaryDirectory);
  components.emplace_back("VolumeRenderingLogicVolumeProperty.vp");
  std::string fileName = vtksys::SystemTools::JoinPath(components);
  std::cout << "fileName = " << fileName.c_str() << std::endl;

  volumePropertyStorageNode->SetFileName(fileName.c_str());
  CHECK_INT(volumePropertyStorageNode->WriteData(defaultVolumePropertyNode.GetPointer()), 1);
  std::cout << "\tfile written okay" << std::endl;

  // try reading without a scene
  vtkMRMLVolumePropertyNode *vpNode = logic->AddVolumePropertyFromFile(fileName.c_str());
  CHECK_NULL(vpNode);

  // set the scene
  vtkNew<vtkMRMLScene> scene;
  logic->SetMRMLScene(scene.GetPointer());

  // null file name
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vpNode = logic->AddVolumePropertyFromFile(nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(vpNode);

  // empty file name
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vpNode = logic->AddVolumePropertyFromFile("");
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(vpNode);

  // read it back in
  vpNode = logic->AddVolumePropertyFromFile(fileName.c_str());
  CHECK_NOT_NULL(vpNode);

  std::cout << "Test passed!" << std::endl;
  return EXIT_SUCCESS;
}
