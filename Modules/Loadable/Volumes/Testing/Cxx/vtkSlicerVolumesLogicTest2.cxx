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

// Volumes logic
#include "vtkSlicerVolumesLogic.h"

// MRML includes
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkDataSetAttributes.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtkSmartPointer.h>
#include <vtkTrivialProducer.h>
#include <vtkStreamingDemandDrivenPipeline.h>

// ITK includes
#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkImageFileWriter.h"

// STD includes
#include <sstream>
#include <vector>
#include <string>

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

//-----------------------------------------------------------------------------
bool isImageDataValid(vtkAlgorithmOutput* imageDataConnection)
{
  if (!imageDataConnection ||
      !imageDataConnection->GetProducer())
    {
    std::cout << "No image data port" << std::endl;
    return false;
    }
  imageDataConnection->GetProducer()->Update();
  vtkInformation* info =
    imageDataConnection->GetProducer()->GetOutputInformation(0);
  if (!info)
    {
    std::cout << "No output information" << std::endl;
    return false;
    }
  vtkInformation *scalarInfo = vtkDataObject::GetActiveFieldInformation(info,
    vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  if (!scalarInfo)
    {
    std::cout << "No scalar information" << std::endl;
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
int main( int argc, char * argv[] )
{
  itk::itkFactoryRegistration();

  if(argc<2)
    {
    std::cerr << "Usage: <testName> <temporary dir path>" << std::endl;
    return EXIT_FAILURE;
    }

  typedef itk::Image<itk::RGBPixel<unsigned char>,3 > ImageType;
  typedef itk::ImageFileWriter<ImageType> WriterType;

  vtkSmartPointer<vtkStringArray> fileNameList = vtkSmartPointer<vtkStringArray>::New();
  // create two RGB images and save as PNG
  for(int i=0;i<5;i++)
    {

    std::ostringstream sstr;
    sstr << argv[argc-1] << "/rgb_test_image" << i << ".png";
    fileNameList->InsertNextValue(sstr.str().c_str());
    ImageType::Pointer im1 = ImageType::New();

    ImageType::SizeType size;
    ImageType::RegionType region;
    size[0] = 10; size[1] = 10; size[2] = 10;

    region.SetSize(size);
    im1->SetRegions(region);
    im1->Allocate();

    WriterType::Pointer w = WriterType::New();
    w->SetInput(im1);
    w->SetFileName(sstr.str().c_str());
    w->Update();
    }



  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVolumesLogic> logic;

  logic->SetMRMLScene(scene.GetPointer());

  vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorVolume =
    vtkMRMLVectorVolumeNode::SafeDownCast(
      logic->AddArchetypeVolume(fileNameList->GetValue(0), "rgbVolume", 0, fileNameList));

  if (!vectorVolume ||
      !isImageDataValid(vectorVolume->GetImageDataConnection()))
    {
    std::cerr << "Failed to load RGB image." << std::endl;
    return EXIT_FAILURE;
    }

  if (vectorVolume && vectorVolume->GetImageData())
    {
    vectorVolume->GetImageData()->Print(std::cerr);
    }

  return EXIT_SUCCESS;
}
