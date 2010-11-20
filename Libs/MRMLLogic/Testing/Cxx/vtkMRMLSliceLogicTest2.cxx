/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/
// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
//#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkMultiThreader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointData.h>

// STD includes
#include <cstdlib>
#include <iostream>

int vtkMRMLSliceLogicTest2(int argc, char * argv [] )
{
//  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(1);
  /*
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }
  */

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer< vtkMRMLSliceLogic > sliceLogic = vtkSmartPointer< vtkMRMLSliceLogic >::New();
  sliceLogic->SetName("Green");
  sliceLogic->SetMRMLScene(scene);
  
  //vtkSmartPointer< vtkMRMLSliceNode > sliceNode = vtkSmartPointer< vtkMRMLSliceNode >::New();
  //sliceLogic->SetSliceNode(sliceNode);
  
  //vtkSmartPointer<vtkMRMLSliceCompositeNode> sliceCompositeNode =
  //  vtkSmartPointer<vtkMRMLSliceCompositeNode>::New();
  /// Oddities: sliceCompositeNode must be added to the scene
  //scene->AddNode(sliceCompositeNode);
  //sliceLogic->SetSliceCompositeNode(sliceCompositeNode);
  vtkMRMLSliceNode* sliceNode =sliceLogic->GetSliceNode();
  vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode(); 
  
  vtkSmartPointer<vtkMRMLSliceLayerLogic> sliceLayerLogic = 
    vtkSmartPointer<vtkMRMLSliceLayerLogic>::New();
  
  sliceLogic->SetBackgroundLayer(sliceLayerLogic);

  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> displayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
  vtkSmartPointer<vtkMRMLScalarVolumeNode> scalarNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode> storageNode = vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode>::New();
  
  displayNode->SetAutoWindowLevel(true);
  displayNode->SetInterpolate(false);
  
  storageNode->SetFileName(argv[1]);
  if (storageNode->SupportedFileType(argv[1]) == 0)
    {
    return EXIT_FAILURE;
    }
  scalarNode->SetName("foo");
  scalarNode->SetScene(scene);
  displayNode->SetScene(scene);
  //vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
  //displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
  //colorLogic->Delete();
  scene->AddNode(storageNode);
  scene->AddNode(displayNode);
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
  scalarNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(scalarNode);
  storageNode->ReadData(scalarNode);

  vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::New();
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode);
  colorNode->Delete();
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
  
  //sliceLayerLogic->SetVolumeNode(scalarNode);
  sliceCompositeNode->SetBackgroundVolumeID(scalarNode->GetID());
  sliceLogic->GetImageData();
  // Not sure why sliceLayerLogic->GetVolumeDisplayNode() is different from displayNode
  vtkMRMLScalarVolumeDisplayNode* displayNode2 = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(sliceLayerLogic->GetVolumeDisplayNode());
  
  vtkImageData* imageData = vtkImageData::New();
  imageData->DeepCopy(displayNode2->GetInput());
  vtkImageMapToWindowLevelColors* mapToWindow = vtkImageMapToWindowLevelColors::New();
  mapToWindow->SetInput(imageData);
  mapToWindow->Update();
  imageData->Modified();
  //displayNode2->GetInput()->SetScalarComponentFromFloat(0, 0, 0, 0, 10.);
  mapToWindow->Update();
  
  for (int i = 0; i < 10; ++i)
    {
    vtkTimerLog* timerLog = vtkTimerLog::New();
    timerLog->StartTimer();
    displayNode->Modified();
    timerLog->StopTimer();
    std::cout << "vtkMRMLDisplayNode::Modified(): " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    timerLog->StartTimer();
    sliceLayerLogic->UpdateTransforms();
    timerLog->StopTimer();
    std::cout << "vtkMRMLSliceLayerLogic::UpdateTransforms(): " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    timerLog->StartTimer();
    sliceLogic->UpdatePipeline();
    timerLog->StopTimer();
    std::cout << "vtkMRMLSliceLogic::UpdatePipeline(): " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    timerLog->StartTimer();
    sliceLayerLogic->UpdateImageDisplay();
    sliceLayerLogic->GetImageData();
    timerLog->StopTimer();
    std::cout << "vtkMRMLSliceLayerLogic::UpdateImageData(): " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    timerLog->Delete();
    }

  vtkImageViewer2* viewer = vtkImageViewer2::New();
  viewer->SetInput(sliceLogic->GetImageData());
  
  // Renderer, RenderWindow and Interactor
  //vtkRenderer* rr = vtkRenderer::New();
  //vtkRenderWindow* rw = vtkRenderWindow::New();
  vtkRenderWindow* rw = viewer->GetRenderWindow();
  rw->SetSize(600, 600);
  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere
  
  vtkRenderWindowInteractor* ri = vtkRenderWindowInteractor::New();
  viewer->SetupInteractor(ri);
  
  rw->Render();
  if (argc > 2 && std::string(argv[2]) == "-I")
    {
    ri->Start();
    }

  viewer->Delete();
  //rr->Delete();
  //rw->Delete();
  ri->Delete();
  return EXIT_SUCCESS;
}

