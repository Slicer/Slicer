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
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkMultiThreader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointData.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageCast.h>
#include <vtkImageBlend.h>
#include <vtkColorTransferFunction.h>
// STD includes
#include <cstdlib>
#include <iostream>

vtkMRMLScalarVolumeNode* loadVolume2(const char* volume, vtkMRMLScene* scene)
{
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> displayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
  vtkSmartPointer<vtkMRMLScalarVolumeNode> scalarNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode> storageNode = vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode>::New();

  displayNode->SetAutoWindowLevel(false);
  displayNode->SetInterpolate(false);

  storageNode->SetFileName(volume);
  if (storageNode->SupportedFileType(volume) == 0)
    {
    return 0;
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

  return scalarNode.GetPointer();
}

int vtkMRMLSliceLogicTest4(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  input_image " << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  vtkMRMLScalarVolumeNode* scalarNode = loadVolume2(argv[1], scene);
  if (scalarNode == 0)
    {
    std::cerr << "Not a valid volume: " << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  vtkImageData* img = scalarNode->GetImageData();
  
  vtkSmartPointer<vtkImageReslice> resliceMask = vtkSmartPointer<vtkImageReslice>::New();
  resliceMask->SetOutputExtent(0, 400, 0, 500, 0, 0);
  resliceMask->SetInput(img);

  vtkSmartPointer<vtkImageMapToColors> colors = vtkSmartPointer<vtkImageMapToColors>::New();
  colors->SetInput(resliceMask->GetOutput());
  vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
  ctf->AddRGBPoint(0, 1., 0., 0.);
  colors->SetLookupTable(ctf);

  vtkSmartPointer<vtkImageAppendComponents> append = vtkSmartPointer<vtkImageAppendComponents>::New();
  append->SetInputConnection(0 ,colors->GetOutput()->GetProducerPort());

  //vtkImageBlend* blend = vtkImageBlend::New();
  //blend->AddInput(append->GetOutput());

  colors->Update();
  append->Update();
  append->Update();
  append->Update();
  colors->Update();
  //append->Update();
  //blend->Update();
  //append->Update();
  resliceMask->SetOutputExtent(0, 400, 0, 300, 0, 0);
  colors->Update();
  
  return EXIT_SUCCESS;
}
