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
#include <vtkMRMLVolumePropertyNode.h>

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//----------------------------------------------------------------------------
int testDefaultRenderingMethod(const std::string& moduleShareDirectory);
int testPresets(const std::string& moduleShareDirectory);

//----------------------------------------------------------------------------
int vtkSlicerVolumeRenderingLogicTest(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cout << "Missing moduleShare Directory argument !" << std::endl;
    return EXIT_FAILURE;
  }
  std::string moduleShareDirectory(argv[1]);

  CHECK_EXIT_SUCCESS(testDefaultRenderingMethod(moduleShareDirectory));
  CHECK_EXIT_SUCCESS(testPresets(moduleShareDirectory));
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int testDefaultRenderingMethod(const std::string& moduleShareDirectory)
{
  vtkNew<vtkSlicerVolumeRenderingLogic> logic;
  logic->SetModuleShareDirectory(moduleShareDirectory);

  vtkMRMLVolumeRenderingDisplayNode* displayNode = logic->CreateVolumeRenderingDisplayNode();
  CHECK_NULL(logic->GetDefaultRenderingMethod());
  CHECK_NULL(displayNode);

  vtkNew<vtkMRMLScene> scene;
  logic->SetMRMLScene(scene);
  displayNode = logic->CreateVolumeRenderingDisplayNode();
  CHECK_NOT_NULL(displayNode);
  CHECK_BOOL(displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"), true);
  displayNode->Delete();

  logic->SetDefaultRenderingMethod("vtkMRMLCPURayCastVolumeRenderingDisplayNode");
  displayNode = logic->CreateVolumeRenderingDisplayNode();
  CHECK_NOT_NULL(displayNode);
  CHECK_BOOL(displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"), true);
  displayNode->Delete();

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int testPresets(const std::string& moduleShareDirectory)
{
  vtkNew<vtkSlicerVolumeRenderingLogic> logic;
  logic->SetModuleShareDirectory(moduleShareDirectory);

  // Default presets
  CHECK_NOT_NULL(logic->GetPresetsScene());
  CHECK_NOT_NULL(logic->GetPresetByName("MR-Default"));

  // Add new preset
  {
    CHECK_NULL(logic->GetPresetByName("MyNewPreset"));
    vtkNew<vtkMRMLVolumePropertyNode> newPreset;
    newPreset->SetName("MyNewPreset");
    CHECK_NOT_NULL(logic->AddPreset(newPreset));
    CHECK_NOT_NULL(logic->GetPresetByName("MyNewPreset"));
  }

  // Add new preset with icon
  {
    vtkNew<vtkImageData> iconImage;
    iconImage->SetDimensions(128, 128, 1);
    iconImage->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    vtkNew<vtkMRMLVolumePropertyNode> newPresetWithIcon;
    newPresetWithIcon->SetName("MyNewPresetWithIcon");
    CHECK_NOT_NULL(logic->AddPreset(newPresetWithIcon, iconImage));
    // Check the added preset
    vtkMRMLNode* addedPresetWithIcon = logic->GetPresetByName("MyNewPresetWithIcon");
    CHECK_NOT_NULL(addedPresetWithIcon);
    vtkMRMLVolumeNode* iconNode = vtkMRMLVolumeNode::SafeDownCast(
      addedPresetWithIcon->GetNodeReference(vtkSlicerVolumeRenderingLogic::GetIconVolumeReferenceRole()));
    CHECK_NOT_NULL(iconNode);
    CHECK_POINTER_DIFFERENT(iconNode->GetImageData(), iconImage);
    // Check that the icon has the same content
    int* dimensions = iconNode->GetImageData()->GetDimensions();
    CHECK_INT(dimensions[0], 128);
    CHECK_INT(dimensions[1], 128);
    CHECK_INT(dimensions[2], 1);
  }

  // Add new preset with icon from a custom presets scene
  {
    // Create custom presets scene (it is normally read from a scene file)
    vtkNew<vtkMRMLScene> customPresetsScene;
    vtkNew<vtkMRMLVolumePropertyNode> newPresetWithIcon;
    newPresetWithIcon->SetName("MyNewPresetWithIcon");
    customPresetsScene->AddNode(newPresetWithIcon);
    vtkNew<vtkMRMLScalarVolumeNode> iconNode;
    vtkNew<vtkImageData> iconImage;
    iconImage->SetDimensions(128, 128, 1);
    iconImage->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    iconNode->SetAndObserveImageData(iconImage);
    customPresetsScene->AddNode(iconNode);
    newPresetWithIcon->SetNodeReferenceID(vtkSlicerVolumeRenderingLogic::GetIconVolumeReferenceRole(),
                                          iconNode->GetID());

    // Add the preset
    CHECK_NOT_NULL(logic->AddPreset(newPresetWithIcon));
    // Check hat the preset is added
    vtkMRMLNode* addedPresetWithIcon = logic->GetPresetByName("MyNewPresetWithIcon");
    CHECK_NOT_NULL(addedPresetWithIcon);
    // Check that the input preset node is still in the input scene
    CHECK_POINTER(newPresetWithIcon->GetScene(), customPresetsScene);

    // Check if icon is added and it is a different instance with the same content
    vtkMRMLVolumeNode* addedIconNode = vtkMRMLVolumeNode::SafeDownCast(
      addedPresetWithIcon->GetNodeReference(vtkSlicerVolumeRenderingLogic::GetIconVolumeReferenceRole()));
    // Check that the icon is valid
    CHECK_NOT_NULL(addedIconNode);
    CHECK_NOT_NULL(addedIconNode->GetImageData());
    // Check that the icon has the same content
    int* dimensions = addedIconNode->GetImageData()->GetDimensions();
    CHECK_INT(dimensions[0], 128);
    CHECK_INT(dimensions[1], 128);
    CHECK_INT(dimensions[2], 1);
    // Check that the icon is an independent copy (not the same instance as the input)
    CHECK_POINTER_DIFFERENT(addedIconNode, iconNode);
    CHECK_POINTER_DIFFERENT(addedIconNode->GetImageData(), iconImage);
  }

  return EXIT_SUCCESS;
}
