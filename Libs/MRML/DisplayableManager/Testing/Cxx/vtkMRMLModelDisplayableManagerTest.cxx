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

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelDisplayableManager.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkWindowToImageFilter.h>

// STD includes

const char vtkMRMLModelDisplayableManagerTest1EventLog[] =
"# StreamVersion 1\n";

//----------------------------------------------------------------------------
int vtkMRMLModelDisplayableManagerTest(int argc, char* argv[])
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetSize(600, 600);
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere

  renderWindow->AddRenderer(renderer.GetPointer());
  renderWindow->SetInteractor(renderWindowInteractor.GetPointer());

  // Set Interactor Style
  //vtkNew<vtkMRMLThreeDViewInteractorStyle> iStyle;
  //renderWindowInteractor->SetInteractorStyle(iStyle.GetPointer());

  // move back far enough to see the reformat widgets
  //renderer->GetActiveCamera()->SetPosition(0,0,-500.);

  // MRML scene
  vtkMRMLScene* scene = vtkMRMLScene::New();

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);

  // Add ViewNode
  vtkNew<vtkMRMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  vtkNew<vtkMRMLDisplayableManagerGroup> displayableManagerGroup;
  displayableManagerGroup->SetRenderer(renderer.GetPointer());
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode.GetPointer());

  vtkNew<vtkMRMLModelDisplayableManager> vrDisplayableManager;
  vrDisplayableManager->SetMRMLApplicationLogic(applicationLogic);
  displayableManagerGroup->AddDisplayableManager(vrDisplayableManager.GetPointer());
  displayableManagerGroup->GetInteractor()->Initialize();

  vtkNew<vtkMRMLModelNode> modelNode;
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetRadius(10.);
  sphereSource->Update();
  modelNode->SetPolyDataConnection(sphereSource->GetOutputPort());

  scene->AddNode(modelNode.GetPointer());

  vtkNew<vtkMRMLModelDisplayNode> modelDisplayNode;
  scene->AddNode(modelDisplayNode.GetPointer());

  modelNode->AddAndObserveDisplayNodeID(modelDisplayNode->GetID());

  // TODO: Automatically move the camera (simulating movements)
  // to have a good screenshot.
  renderer->SetBackground(0, 169. / 255, 79. /255);
  renderer->SetBackground2(0, 83. / 255, 155. /255);
  renderer->SetGradientBackground(true);
  renderer->ResetCamera();

  // Event recorder
  bool disableReplay = false, record = false, screenshot = false;
  for (int i = 0; i < argc; i++)
    {
    disableReplay |= (strcmp("--DisableReplay", argv[i]) == 0);
    record        |= (strcmp("--Record", argv[i]) == 0);
    screenshot    |= (strcmp("--Screenshot", argv[i]) == 0);
    }
  vtkNew<vtkInteractorEventRecorder> recorder;
  recorder->SetInteractor(displayableManagerGroup->GetInteractor());
  if (!disableReplay)
    {
    if (record)
      {
      std::cout << "Recording ..." << std::endl;
      recorder->SetFileName("vtkInteractorEventRecorder.log");
      recorder->On();
      recorder->Record();
      }
    else
      {
      // Play
      recorder->ReadFromInputStringOn();
      recorder->SetInputString(vtkMRMLModelDisplayableManagerTest1EventLog);
      recorder->Play();
      }
    }

  int retval = vtkRegressionTestImageThreshold(renderWindow.GetPointer(), 85.0);
  if ( record || retval == vtkRegressionTester::DO_INTERACTOR)
    {
    displayableManagerGroup->GetInteractor()->Initialize();
    displayableManagerGroup->GetInteractor()->Start();
    }

  if (record || screenshot)
    {
    vtkNew<vtkWindowToImageFilter> windowToImageFilter;
    windowToImageFilter->SetInput(renderWindow.GetPointer());
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 1)
    windowToImageFilter->SetScale(1, 1); //set the resolution of the output image
#else
    windowToImageFilter->SetMagnification(1); //set the resolution of the output image
#endif
    windowToImageFilter->Update();

    vtkNew<vtkTesting> testHelper;
    testHelper->AddArguments(argc, const_cast<const char **>(argv));

    vtkStdString screenshootFilename = testHelper->GetDataRoot();
    screenshootFilename += "/Baseline/vtkMRMLCameraDisplayableManagerTest1.png";
    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName(screenshootFilename.c_str());
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
    std::cout << "Saved screenshot: " << screenshootFilename << std::endl;
    }

  vrDisplayableManager->SetMRMLApplicationLogic(nullptr);
  applicationLogic->Delete();
  scene->Delete();

  return !retval;
}

