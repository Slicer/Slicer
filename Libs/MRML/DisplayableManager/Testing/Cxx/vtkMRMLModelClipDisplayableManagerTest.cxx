/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLClipNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelDisplayableManager.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkMatrix4x4.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>

//----------------------------------------------------------------------------
int vtkMRMLModelClipDisplayableManagerTest(int argc, char* argv[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(0, 169. / 255, 79. / 255);
  renderer->SetBackground2(0, 83. / 255, 155. / 255);
  renderer->SetGradientBackground(true);
  renderer->ResetCamera();

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(600, 600);
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere
  renderWindow->AddRenderer(renderer);

  const vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetInteractor(renderWindowInteractor);

  // MRML scene
  vtkNew<vtkMRMLScene> scene;

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene);

  // Add ViewNode
  const vtkNew<vtkMRMLViewNode> viewNode;
  scene->AddNode(viewNode);

  vtkNew<vtkMRMLDisplayableManagerGroup> displayableManagerGroup;
  displayableManagerGroup->SetRenderer(renderer);
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode);

  vtkNew<vtkMRMLModelDisplayableManager> displayableManager;
  displayableManager->SetMRMLApplicationLogic(applicationLogic);
  displayableManagerGroup->AddDisplayableManager(displayableManager);
  displayableManagerGroup->GetInteractor()->Initialize();

  vtkNew<vtkTransform> sliceToRAS;
  sliceToRAS->RotateX(125);
  sliceToRAS->RotateY(20);

  vtkNew<vtkMRMLSliceNode> sliceNode;
  sliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS->GetMatrix());
  sliceNode->UpdateMatrices();
  scene->AddNode(sliceNode);

  vtkNew<vtkMRMLClipNode> clipNode;
  scene->AddNode(clipNode);
  clipNode->SetAndObserveClippingNodeID(sliceNode->GetID());

  vtkNew<vtkMRMLModelNode> modelNode;
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetRadius(10.);
  sphereSource->Update();
  modelNode->SetPolyDataConnection(sphereSource->GetOutputPort());
  scene->AddNode(modelNode);

  vtkNew<vtkMRMLModelDisplayNode> modelDisplayNode;
  modelDisplayNode->SetAndObserveClipNodeID(clipNode->GetID());
  modelDisplayNode->ClippingOn();
  modelDisplayNode->ClippingCapSurfaceOn();
  modelDisplayNode->ClippingOutlineOn();
  modelDisplayNode->SetClippingCapOpacity(0.6);
  scene->AddNode(modelDisplayNode);
  modelNode->AddAndObserveDisplayNodeID(modelDisplayNode->GetID());

  const int retval = vtkRegressionTestImage(renderWindow);
  return retval > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
