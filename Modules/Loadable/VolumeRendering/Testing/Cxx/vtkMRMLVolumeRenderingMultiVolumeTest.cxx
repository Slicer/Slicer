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
#include <vtkMRMLCPURayCastVolumeRenderingDisplayNode.h>
#include <vtkMRMLVolumeRenderingDisplayableManager.h>
#include <vtkSlicerVolumeRenderingLogic.h>

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkImageDifference.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVersion.h>
#include <vtkWindowToImageFilter.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
void SetupRenderer(vtkRenderWindow* renderWindow, vtkRenderer* renderer)
{
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindow->SetSize(600, 600);
  renderWindow->SetMultiSamples(0); // Ensure to have the same test image everywhere

  renderWindow->AddRenderer(renderer);
  renderWindow->SetInteractor(renderWindowInteractor.GetPointer());
}

//----------------------------------------------------------------------------
void SetupScene(vtkRenderer* renderer, vtkMRMLScene* scene, vtkMRMLApplicationLogic* applicationLogic, vtkMRMLDisplayableManagerGroup* displayableManagerGroup)
{
  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  if (applicationLogic != nullptr)
    {
    applicationLogic->SetMRMLScene(scene);
    }

  // Add ViewNode
  vtkNew<vtkMRMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());

  displayableManagerGroup->SetRenderer(renderer);
  displayableManagerGroup->SetMRMLDisplayableNode(viewNode.GetPointer());

  vtkNew<vtkMRMLVolumeRenderingDisplayableManager> vrDisplayableManager;
  vrDisplayableManager->SetMRMLApplicationLogic(applicationLogic);
  displayableManagerGroup->AddDisplayableManager(vrDisplayableManager.GetPointer());

  displayableManagerGroup->GetInteractor()->Initialize();
}

//----------------------------------------------------------------------------
void SetupImageData(vtkImageData* imageData)
{
  const int dimX = 3;
  const int dimY = 3;
  const int dimZ = 3;
  imageData->SetDimensions(dimX, dimY, dimZ);
  imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  unsigned char* ptr = reinterpret_cast<unsigned char*>(
    imageData->GetScalarPointer(0,0,0));
  for (int z = 0; z < dimZ; ++z)
    {
    for (int y = 0; y < dimY; ++y)
      {
      for (int x = 0; x < dimX; ++x)
        {
        *(ptr++) = static_cast<unsigned char>(rand() % 256);
        }
      }
    }
}

//----------------------------------------------------------------------------
void SetupVolumeNode(vtkMRMLScene* scene, vtkMRMLScalarVolumeNode* volumeNode)
{
  vtkNew<vtkImageData> imageData;
  SetupImageData(imageData.GetPointer());
  volumeNode->SetAndObserveImageData(imageData.GetPointer());
  scene->AddNode(volumeNode);

  vtkNew<vtkMRMLVolumePropertyNode> volumePropertyNode;
  scene->AddNode(volumePropertyNode.GetPointer());

  vtkNew<vtkMRMLCPURayCastVolumeRenderingDisplayNode> vrDisplayNode;
  vrDisplayNode->SetAndObserveVolumePropertyNodeID(volumePropertyNode->GetID());
  scene->AddNode(vrDisplayNode.GetPointer());
  volumeNode->AddAndObserveDisplayNodeID(vrDisplayNode->GetID());
}

char TestCopyImageDataEventLog[] =
"# StreamVersion 1\n"
"EnterEvent 585 173 0 0 0 0 0\n"
"KeyPressEvent 585 173 0 0 0 0 0\n"
"KeyPressEvent 560 178 0 0 0 0 0\n"
"KeyPressEvent 536 185 0 0 0 0 0\n";

class vtkChangeImageCallback : public vtkCommand
{
public:
  static vtkChangeImageCallback *New(){return new vtkChangeImageCallback;}
  vtkChangeImageCallback();
  void Execute(vtkObject *caller, unsigned long, void*) override;

  enum ChangeImageBehavior
  {
    DeepCopyImage,
    SetAndObserveImage
  };

  // Attributes
  vtkMRMLScalarVolumeNode* VolumeNode{nullptr};
  int ChangeImageBehavior{DeepCopyImage};
  vtkSmartPointer<vtkCollection> ImageDataCollection;
  int CurrentImageData{-1};
  vtkRenderWindowInteractor* Interactor{nullptr};
};

//----------------------------------------------------------------------------
vtkChangeImageCallback::vtkChangeImageCallback()
  : ImageDataCollection(vtkSmartPointer<vtkCollection>::New())
{
  const int imageCount = 5;
  for (int i = 0; i < imageCount; ++i)
    {
    vtkNew<vtkImageData> imageData;
    SetupImageData(imageData.GetPointer());
    this->ImageDataCollection->AddItem(imageData.GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkChangeImageCallback::Execute(vtkObject* caller, unsigned long , void* )
{
  vtkMRMLDisplayableManagerGroup* group =
    vtkMRMLDisplayableManagerGroup::SafeDownCast(caller);
  if (group)
    {
    if (this->Interactor)
      {
      this->Interactor->Render();
      }
    return;
    }
  assert(vtkRenderWindowInteractor::SafeDownCast(caller));
  this->CurrentImageData = (this->CurrentImageData + 1)
    % this->ImageDataCollection->GetNumberOfItems();
  vtkImageData* newImageData = vtkImageData::SafeDownCast(
    this->ImageDataCollection->GetItemAsObject(this->CurrentImageData));
  switch (this->ChangeImageBehavior)
    {
    case DeepCopyImage:
      this->VolumeNode->GetImageData()->DeepCopy(newImageData);
      break;
    case SetAndObserveImage:
      this->VolumeNode->SetAndObserveImageData(newImageData);
      break;
    }
}

//----------------------------------------------------------------------------
bool TestChangeImageData(int copyBehavior, vtkImageData* screenShot)
{
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  SetupRenderer(renderWindow.GetPointer(), renderer.GetPointer());

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  vtkNew<vtkMRMLDisplayableManagerGroup> displayableManagerGroup;
  SetupScene(renderer.GetPointer(), scene.GetPointer(),
             applicationLogic.GetPointer(),
             displayableManagerGroup.GetPointer());

  // Make sure the images (default image in volume node and the ones in
  // changeImageCallback )are the same for each TestChangeImageData call.
  srand(1);

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  SetupVolumeNode(scene.GetPointer(), volumeNode.GetPointer());

  vtkNew<vtkChangeImageCallback> changeImageCallback;
  changeImageCallback->VolumeNode = volumeNode.GetPointer();
  changeImageCallback->Interactor = renderWindow->GetInteractor();
  changeImageCallback->ChangeImageBehavior = copyBehavior;

  renderer->ResetCamera();

  renderWindow->GetInteractor()->AddObserver(
    vtkCommand::KeyPressEvent, changeImageCallback.GetPointer());
  displayableManagerGroup->AddObserver(
    vtkCommand::UpdateEvent, changeImageCallback.GetPointer());

  vtkNew<vtkInteractorEventRecorder> recorder;
  recorder->SetInteractor(renderWindow->GetInteractor());
  recorder->ReadFromInputStringOn();
  recorder->SetInputString(TestCopyImageDataEventLog);
  recorder->Play();

  if (screenShot)
    {
    vtkNew<vtkWindowToImageFilter> windowToImageFilter;
    windowToImageFilter->SetInput(renderWindow.GetPointer());
    windowToImageFilter->Update();
    screenShot->DeepCopy(windowToImageFilter->GetOutput());
    }
  //renderWindow->GetInteractor()->Start();

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeRenderingMultiVolumeTest(int vtkNotUsed(argc),
                                          char* vtkNotUsed(argv)[])
{
  bool res = true;
  // Save the output image of the Test and compare each output to make sure
  // the different copy techniques work the same.
  vtkNew<vtkImageData> referenceScreenShot;
  res = TestChangeImageData(vtkChangeImageCallback::DeepCopyImage,
                            referenceScreenShot.GetPointer()) && res;
  vtkNew<vtkCollection> screenShots;
  vtkNew<vtkImageData> screenShot;
  res = TestChangeImageData(vtkChangeImageCallback::SetAndObserveImage,
                            screenShot.GetPointer()) && res;
  screenShots->AddItem(screenShot.GetPointer());
  if (!res)
    {
    std::cout << __LINE__ << ": TestChangeImageData failed" << std::endl;
    return EXIT_FAILURE;
    }
  for (int i = 0; i < screenShots->GetNumberOfItems(); ++i)
    {
    vtkImageData* screenShotImage = vtkImageData::SafeDownCast(
      screenShots->GetItemAsObject(i));

    vtkNew<vtkImageDifference> diff;
    diff->SetInputData(referenceScreenShot.GetPointer());
    diff->SetImageData(screenShotImage);
    diff->Update();
    double error = diff->GetThresholdedError();
    if (error > 0)
      {
      std::cout << __LINE__ << ": TestChangeImageData failed to give similar "
                << "result. Got a thresholded error of " << error << std::endl;
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}
