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
#include <vtkMRMLVolumeRenderingDisplayNode.h>
#include <vtkSlicerVolumeRenderingLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
bool testDefaultRenderingMethod(const std::string& moduleShareDirectory);
bool testPresets(const std::string &moduleShareDirectory);

//----------------------------------------------------------------------------
int vtkSlicerVolumeRenderingLogicTest(int argc, char* argv[])
{
  if (argc != 2)
    {
    std::cout << "Missing moduleShareDirectory argument !" << std::endl;
    return EXIT_FAILURE;
    }
  std::string moduleShareDirectory(argv[1]);

  bool res = true;
  res = testDefaultRenderingMethod(moduleShareDirectory) && res;
  res = testPresets(moduleShareDirectory) && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
bool testDefaultRenderingMethod(const std::string& moduleShareDirectory)
{
  vtkNew<vtkSlicerVolumeRenderingLogic> logic;
  logic->SetModuleShareDirectory(moduleShareDirectory);

  vtkMRMLVolumeRenderingDisplayNode* displayNode =
    logic->CreateVolumeRenderingDisplayNode();
  if (logic->GetDefaultRenderingMethod() != 0 ||
      displayNode != 0)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with vtkSlicerVolumeRenderingLogic::CreateVolumeRenderingDisplayNode()"
              << std::endl << "displayNode:" << displayNode << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  logic->SetMRMLScene(scene.GetPointer());
  displayNode = logic->CreateVolumeRenderingDisplayNode();
  if (displayNode == 0 ||
      !displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"))
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with vtkSlicerVolumeRenderingLogic::CreateVolumeRenderingDisplayNode()"
              << std::endl << "displayNode:" << displayNode << " ("
              << (displayNode ? displayNode->GetClassName() : "none") << ")" << std::endl;
    return false;
    }
  displayNode->Delete();

  logic->SetDefaultRenderingMethod("vtkMRMLGPURayCastVolumeRenderingDisplayNode");
  displayNode = logic->CreateVolumeRenderingDisplayNode();
  if (displayNode == 0 ||
      !displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with vtkSlicerVolumeRenderingLogic::CreateVolumeRenderingDisplayNode()"
              << std::endl << "displayNode:" << displayNode << " ("
              << (displayNode ? displayNode->GetClassName() : "none") << ")" << std::endl;
    return false;
    }
  displayNode->Delete();

  return true;
}

//----------------------------------------------------------------------------
bool testPresets(const std::string& moduleShareDirectory)
{
  vtkNew<vtkSlicerVolumeRenderingLogic> logic;
  logic->SetModuleShareDirectory(moduleShareDirectory);

  if (logic->GetPresetsScene() == 0)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with vtkSlicerVolumeRenderingLogic::GetPresetsScene()" << std::endl;
    return false;
    }

  const char* presetName = "MR-Default";
  if (logic->GetPresetByName(presetName) == 0)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with vtkSlicerVolumeRenderingLogic::GetPresetByName()\n"
              << " - Failed to load preset '" << presetName << "'"
              << std::endl;
    return false;
    }

  return true;
}
