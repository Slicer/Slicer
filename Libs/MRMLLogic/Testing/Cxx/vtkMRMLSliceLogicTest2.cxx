/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceLayerLogic.h>

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes
#include <vtkImageResliceMask.h>
#include <vtkImageViewer2.h>
#include <vtkMultiThreader.h>
#include <vtkPointData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

// STD includes
#include <cstdlib>
#include <iostream>

int vtkMRMLSliceLogicTest2(int argc, char * argv [] )
{
//  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(1);
  
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer< vtkMRMLSliceLogic > sliceLogic = vtkSmartPointer< vtkMRMLSliceLogic >::New();
  sliceLogic->SetName("Green");
  sliceLogic->SetMRMLScene(scene);
  
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

  for (int i = 0; i < 10; ++i)
    {
    vtkSmartPointer<vtkTimerLog> timerLog = vtkSmartPointer<vtkTimerLog>::New();
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
    }
  
  // Duplicate the pipeline of vtkMRMLScalarVolumeDisplayNode
  vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
  imageData->DeepCopy(displayNode2->GetInputImageData());
  vtkSmartPointer<vtkImageResliceMask> reslice = vtkSmartPointer<vtkImageResliceMask>::New();
  reslice->SetBackgroundColor(0, 0, 0, 0); // only first two are used
  reslice->AutoCropOutputOff();
  reslice->SetOptimization(1);
  reslice->SetOutputOrigin( 0, 0, 0 );
  reslice->SetOutputSpacing( 1, 1, 1 );
  reslice->SetOutputDimensionality( 3 );
  int dimensions[3];
  sliceNode->GetDimensions(dimensions);
  
  reslice->SetOutputExtent( 0, dimensions[0]-1,
                            0, dimensions[1]-1,
                            0, dimensions[2]-1);
  reslice->SetInput(imageData);
  //reslice->SetResliceTransform(sliceLayerLogic->GetXYToIJKTransform());
  vtkSmartPointer<vtkImageMapToWindowLevelColors> mapToWindow = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
  mapToWindow->SetInput(reslice->GetOutput());
  
  vtkSmartPointer<vtkImageMapToColors> mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  mapToColors->SetOutputFormatToRGB();
  if (colorNode->GetLookupTable() == 0)
    {
    return EXIT_FAILURE;
    }
  mapToColors->SetLookupTable(colorNode->GetLookupTable());
  mapToColors->SetInput(mapToWindow->GetOutput());
  //mapToWindow->Update();
  
  vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
  threshold->SetOutputScalarTypeToUnsignedChar();
  threshold->SetInput(reslice->GetOutput());
  threshold->ThresholdBetween( 1, 0 ); 
  threshold->ReplaceInOn();
  threshold->SetInValue(255);
  threshold->ReplaceOutOn();
  threshold->SetOutValue(255); 
  
  vtkSmartPointer<vtkImageCast> resliceAlphaCast = vtkSmartPointer<vtkImageCast>::New();
  resliceAlphaCast->SetInput(reslice->GetBackgroundMask());
  resliceAlphaCast->SetOutputScalarTypeToUnsignedChar();
  
  vtkSmartPointer<vtkImageLogic> alphaLogic = vtkSmartPointer<vtkImageLogic>::New();
  alphaLogic->SetOperationToAnd();
  alphaLogic->SetOutputTrueValue(255);
  alphaLogic->SetInput1(threshold->GetOutput());
  alphaLogic->SetInput2(resliceAlphaCast->GetOutput());
  
  vtkSmartPointer<vtkImageAppendComponents> appendComponents = vtkSmartPointer<vtkImageAppendComponents>::New();
  appendComponents->RemoveAllInputs();
  appendComponents->SetInputConnection(0, mapToColors->GetOutput()->GetProducerPort() );
  appendComponents->AddInputConnection(0, alphaLogic->GetOutput()->GetProducerPort() );
  
  //displayNode2->GetInput()->SetScalarComponentFromFloat(0, 0, 0, 0, 10.);
  vtkSmartPointer<vtkTimerLog> timerLog = vtkSmartPointer<vtkTimerLog>::New();
  timerLog->StartTimer();
  appendComponents->Update();
  timerLog->StopTimer();
  std::cout << "vtkMRMLScalarVolumeDisplayNode::pipeline: " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
  for (int i = 0; i < 4; ++i)
    {
    imageData->Modified();
    timerLog->StartTimer();
    appendComponents->Update();
    timerLog->StopTimer();
    std::cout << "vtkMRMLScalarVolumeDisplayNode::pipeline updated: " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    
    timerLog->StartTimer();
    mapToWindow->SetWindow(mapToWindow->GetWindow() + 1);
    appendComponents->Update();
    timerLog->StopTimer();
    std::cout << "vtkMRMLScalarVolumeDisplayNode::window updated: " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    
    timerLog->StartTimer();
    threshold->SetOutValue(threshold->GetOutValue()-1);
    appendComponents->Update();
    timerLog->StopTimer();
    std::cout << "vtkMRMLScalarVolumeDisplayNode::threshold updated: " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    
    timerLog->StartTimer();
    alphaLogic->SetOutputTrueValue(alphaLogic->GetOutputTrueValue()-1);
    appendComponents->Update();
    timerLog->StopTimer();
    std::cout << "vtkMRMLScalarVolumeDisplayNode::alpha updated: " << timerLog->GetElapsedTime() << " fps: " << 1. / timerLog->GetElapsedTime() << std::endl;
    }
  vtkSmartPointer<vtkImageViewer2> viewer = vtkSmartPointer<vtkImageViewer2>::New();
  viewer->SetInput(sliceLogic->GetImageData());
  //viewer->SetInput(appendComponents->GetOutput());
  
  // Renderer, RenderWindow and Interactor
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

  ri->Delete();

  return EXIT_SUCCESS;
}

