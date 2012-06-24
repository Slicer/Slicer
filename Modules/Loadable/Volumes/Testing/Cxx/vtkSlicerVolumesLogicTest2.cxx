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
#include <vtkDataSetAttributes.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtkSmartPointer.h>

// ITK includes
#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkImageFileWriter.h"

#include <sstream>
#include <vector>
#include <string>

//-----------------------------------------------------------------------------
bool isImageDataValid(vtkImageData* imageData)
{
  if (!imageData)
    {
    return false;
    }
  imageData->GetProducerPort();
  vtkInformation* info = imageData->GetPipelineInformation();
  if (!info)
    {
    return false;
    }
  vtkInformation *scalarInfo = vtkDataObject::GetActiveFieldInformation(info,
    vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  if (!scalarInfo)
    {
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
int main( int argc, char * argv[] )
{

  if(argc<2)
    {
    std::cerr << "Usage: <testName> <temporary dir path>" << std::endl;
    return EXIT_FAILURE;
    }

  typedef itk::Image<itk::RGBPixel<unsigned char> > ImageType;
  typedef itk::ImageFileWriter<ImageType> WriterType;

  vtkSmartPointer<vtkStringArray> fileNameList = vtkSmartPointer<vtkStringArray>::New();
  // create two RGB images and save as PNG
  for(int i=0;i<5;i++)
    {

    std::ostringstream sstr;
    sstr << argv[argc-1] << "/rgb_test_image" << i << ".png";
    std::cout << sstr.str() << std::endl;
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
    std::cout << "Will write test file to " << sstr.str() << std::endl;
    w->SetFileName(sstr.str().c_str());
    w->Update();
    }



  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVolumesLogic> logic;

  logic->SetMRMLScene(scene.GetPointer());

  vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorVolume =
    vtkMRMLVectorVolumeNode::SafeDownCast(
      logic->AddArchetypeVolume(argv[argc-1], "rgbVolume", 0, fileNameList));

  if (!vectorVolume ||
      !isImageDataValid(vectorVolume->GetImageData()))
    {
    std::cerr << "Failed to load RGB image." << std::endl;
    if (vectorVolume && vectorVolume->GetImageData())
      {
      vectorVolume->GetImageData()->Print(std::cerr);
      }
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
