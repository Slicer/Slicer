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
// MRMLLogic includes

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageAppendComponents.h>
#include <vtkNew.h>
#include <vtkVersion.h>

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

//-----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* loadVolume2(const char* volume, vtkMRMLScene* scene)
{
  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  vtkNew<vtkMRMLScalarVolumeNode> scalarNode;
  vtkNew<vtkMRMLVolumeArchetypeStorageNode> storageNode;

  displayNode->SetAutoWindowLevel(false);
  displayNode->SetInterpolate(false);

  storageNode->SetFileName(volume);
  if (storageNode->SupportedFileType(volume) == 0)
    {
    return nullptr;
    }
  scalarNode->SetName("foo");
  scalarNode->SetScene(scene);
  displayNode->SetScene(scene);
  //vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
  //displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
  //colorLogic->Delete();
  scene->AddNode(storageNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
  scalarNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(scalarNode.GetPointer());
  storageNode->ReadData(scalarNode.GetPointer());

  vtkNew<vtkMRMLColorTableNode> colorNode;
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode.GetPointer());

  displayNode->SetAndObserveColorNodeID(colorNode->GetID());

  return scalarNode.GetPointer();
}

//-----------------------------------------------------------------------------
int vtkMRMLSliceLogicTest4(int argc, char * argv [] )
{
  itk::itkFactoryRegistration();

  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  input_image " << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLScene> scene;

  vtkMRMLScalarVolumeNode* scalarNode = loadVolume2(argv[1], scene.GetPointer());
  if (scalarNode == nullptr)
    {
    std::cerr << "Not a valid volume: " << argv[1] << std::endl;
    return EXIT_FAILURE;
    }

  vtkImageData* img = scalarNode->GetImageData();

  vtkNew<vtkImageReslice> resliceMask;
  resliceMask->SetOutputExtent(0, 400, 0, 500, 0, 0);
  resliceMask->SetInputData(img);

  vtkNew<vtkImageMapToColors> colors;
  colors->SetInputConnection(resliceMask->GetOutputPort());
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->AddRGBPoint(0, 1., 0., 0.);
  colors->SetLookupTable(ctf.GetPointer());

  vtkNew<vtkImageAppendComponents> append;
  append->SetInputConnection(0, colors->GetOutputPort());

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
