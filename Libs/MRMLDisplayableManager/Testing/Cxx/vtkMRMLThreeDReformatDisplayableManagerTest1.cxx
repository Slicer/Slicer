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
#include <vtkMRMLThreeDReformatDisplayableManager.h>
#include <vtkThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowToImageFilter.h>

// STD includes

#include "TestingMacros.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

char vtkMRMLThreeDReformatDisplayableManagerTest1EventLog[] =
"# StreamVersion 1\n";

//----------------------------------------------------------------------------
int vtkMRMLThreeDReformatDisplayableManagerTest1(int argc, char* argv[])
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
  vtkNew<vtkThreeDViewInteractorStyle> iStyle;
  renderWindowInteractor->SetInteractorStyle(iStyle.GetPointer());

  // move back far enough to see the reformat widgets
  renderer->GetActiveCamera()->SetPosition(0,0,-500.);

  // MRML scene
  vtkMRMLScene* scene = vtkMRMLScene::New();

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  // Pass through event handling once without observing the scene
  // allows any dependent nodes to be created
  //applicationLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, 0);
  applicationLogic->SetAndObserveMRMLScene(scene);

  // Add ViewNode
  vtkNew<vtkMRMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  vtkNew<vtkMRMLDisplayableManagerGroup> displayableManagerGroup;
  displayableManagerGroup->SetRenderer(renderer.GetPointer());
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode.GetPointer());

  vtkNew<vtkMRMLThreeDReformatDisplayableManager> reformatDisplayableManager;
  reformatDisplayableManager->SetMRMLApplicationLogic(applicationLogic);
  displayableManagerGroup->AddDisplayableManager(reformatDisplayableManager.GetPointer());

  // Visible when added
  vtkNew<vtkMRMLSliceNode> sliceNodeRed;
  // CRASH if there is no layout
  sliceNodeRed->SetLayoutName("Red");
  // TODO: This color should be taken into account, not the layout name
  sliceNodeRed->SetLayoutColor("red");
  sliceNodeRed->SetWidgetVisible(1);

  scene->AddNode(sliceNodeRed.GetPointer());

  // Locked to Camera
  vtkNew<vtkMRMLSliceNode> sliceNodeYellow;
  sliceNodeYellow->SetLayoutName("Yellow");
  sliceNodeYellow->SetLayoutColor("yellow");
  sliceNodeYellow->SetWidgetVisible(1);
  // TODO: Why it is "Widget"(visible) but "Plane"(locked to camera), it
  // should be consistent.
  sliceNodeYellow->SetPlaneLockedToCamera(1);

  scene->AddNode(sliceNodeYellow.GetPointer());

  // Delayed Visibility
  vtkNew<vtkMRMLSliceNode> sliceNodeGreen;
  sliceNodeGreen->SetLayoutName("Green");
  sliceNodeGreen->SetLayoutColor("green");
  sliceNodeGreen->SetPlaneLockedToCamera(1);
  sliceNodeGreen->SetSliceOffset(-20);

  scene->AddNode(sliceNodeGreen.GetPointer());

  sliceNodeGreen->SetWidgetVisible(1);

  // Deleted slice
  vtkNew<vtkMRMLSliceNode> sliceNodeDeleted;
  sliceNodeDeleted->SetLayoutName("White");
  // TODO: make sure white (SVG color) is supported
  sliceNodeDeleted->SetLayoutColor("white");
  sliceNodeDeleted->SetSliceOffset(300);
  sliceNodeDeleted->SetWidgetVisible(1);

  scene->AddNode(sliceNodeDeleted.GetPointer());
  scene->RemoveNode(sliceNodeDeleted.GetPointer());

  // Imported slice
  vtkNew<vtkMRMLSliceNode> sliceNodeImported;
  // CRASH if there is no layout
  sliceNodeImported->SetLayoutName("Chocolate");
  // TODO: This color should be taken into account, not the layout name
  sliceNodeImported->SetLayoutColor("chocolate");
  sliceNodeImported->SetSliceOffset(10.);
  sliceNodeImported->SetWidgetVisible(1);

  // Simulate a scene loading
  scene->SetIsImporting(1);
  scene->AddNode(sliceNodeImported.GetPointer());
  scene->SetIsImporting(0);

  // Restored slice
  vtkNew<vtkMRMLSliceNode> sliceNodeRestored;
  // CRASH if there is no layout
  sliceNodeRestored->SetLayoutName("Custom");
  // TODO: This color should be taken into account, not the layout name
  sliceNodeRestored->SetLayoutColor("#888888");
  sliceNodeRestored->SetSliceOffset(-10.);
  sliceNodeRestored->SetWidgetVisible(1);

  // Simulate a scene restore
  class vtkAbortCommand: public vtkCommand
  {
  public:
    static vtkAbortCommand *New(){return new vtkAbortCommand;}
    virtual void Execute (vtkObject* vtkNotUsed(caller),
                          unsigned long vtkNotUsed(eventId),
                          void* vtkNotUsed(callData))
    {
    this->SetAbortFlag(1);
    }
  };
  vtkNew<vtkAbortCommand> abortCommand;
  scene->AddObserver(vtkCommand::AnyEvent, abortCommand.GetPointer(), 10000.);
  scene->SetIsImporting(1);
  scene->RemoveObserver(abortCommand.GetPointer());
  
  scene->InvokeEvent(vtkMRMLScene::SceneAboutToBeRestoredEvent, NULL);
  scene->AddNode(sliceNodeRestored.GetPointer());

  scene->AddObserver(vtkCommand::AnyEvent, abortCommand.GetPointer(), 10000.);
  scene->SetIsImporting(0);
  scene->RemoveObserver(abortCommand.GetPointer());

  scene->InvokeEvent(vtkMRMLScene::SceneRestoredEvent, NULL);

  // TODO: Automatically move the camera (simulating movements)
  // to have a good screenshot.

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
      recorder->SetInputString(vtkMRMLThreeDReformatDisplayableManagerTest1EventLog);
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
    VTK_CREATE(vtkWindowToImageFilter, windowToImageFilter) ;
    windowToImageFilter->SetInput(renderWindow.GetPointer());
    windowToImageFilter->SetMagnification(1); //set the resolution of the output image
    windowToImageFilter->Update();

    vtkNew<vtkTesting> testHelper;
    testHelper->AddArguments(argc, const_cast<const char **>(argv));

    vtkStdString screenshootFilename = testHelper->GetDataRoot();
    screenshootFilename += "/Baseline/vtkMRMLCameraDisplayableManagerTest1.png";
    VTK_CREATE(vtkPNGWriter, writer);
    writer->SetFileName(screenshootFilename.c_str());
    writer->SetInput(windowToImageFilter->GetOutput());
    writer->Write();
    std::cout << "Saved screenshot: " << screenshootFilename << std::endl;
    }

  reformatDisplayableManager->SetMRMLApplicationLogic(0);
  applicationLogic->Delete();
  scene->Delete();

  return !retval;
}

