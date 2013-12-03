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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceLogic.h>

// MRML includes
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkNew.h>

//----------------------------------------------------------------------------
int vtkMRMLSliceViewDisplayableManagerFactoryTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere
  renderWindow->AddRenderer(renderer.GetPointer());
  renderWindow->SetInteractor(renderWindowInteractor.GetPointer());

  // MRML scene
  vtkNew<vtkMRMLScene> scene;

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene.GetPointer());

  // Add ViewNode
  vtkMRMLSliceNode * viewNode = vtkMRMLSliceNode::New();
  viewNode->SetLayoutName("Red");
  scene->AddNode(viewNode);
  viewNode->Delete();

  vtkMRMLSliceLogic * sliceLogic = vtkMRMLSliceLogic::New();
  sliceLogic->SetName("Red");
  sliceLogic->SetMRMLScene(scene.GetPointer());
  sliceLogic->SetSliceNode(viewNode);
  sliceLogic->UpdateSliceNode();
  sliceLogic->UpdateSliceNodeFromLayout();
  sliceLogic->UpdateSliceCompositeNode();

  // Factory
  vtkMRMLSliceViewDisplayableManagerFactory * factory =
    vtkMRMLSliceViewDisplayableManagerFactory::GetInstance();

  // Check if GetRegisteredDisplayableManagerCount returns 0
  if (factory->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  factory->RegisterDisplayableManager("vtkMRMLModelSliceDisplayableManager");

  // Check if GetRegisteredDisplayableManagerCount returns 1
  if (factory->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLDisplayableManagerGroup * displayableManagerGroup =
    factory->InstantiateDisplayableManagers(renderer.GetPointer());

  if (!displayableManagerGroup)
    {
    std::cerr << "Failed to instantiate Displayable Managers using "
        << "InstantiateDisplayableManagers" << std::endl;
    return EXIT_FAILURE;
    }

  // Check if GetDisplayableManagerCount returns 2
  if (displayableManagerGroup->GetDisplayableManagerCount() != 1)
    {
    std::cerr << "Check displayableManagerGroup->GetDisplayableManagerCount()" << std::endl;
    std::cerr << "Expected DisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current DisplayableManagerCount:"
      << displayableManagerGroup->GetDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // Assign ViewNode
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode);

  displayableManagerGroup->Delete();
  sliceLogic->Delete();

  return EXIT_SUCCESS;
}
