#include "itkLargestForegroundFilledMaskImageFilter.h"
#include "itkIO.h"
#include <string>

int
main(int argc, char **argv)
{
  if ( argc < 3 )
    {
    std::cerr << "TestLargestForegrounFilledMaskImageFilter <input image> <output image>"
              << std::endl;
    exit(1);
    }
  typedef itk::Image< float, 3 > ImageType;
  typedef itk::LargestForegroundFilledMaskImageFilter< ImageType >
  FilterType;
  std::string         inputname(argv[1]);
  std::string         outputname(argv[2]);
  ImageType::Pointer  image = itkUtil::ReadImage< ImageType >(inputname);
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(image);
  filter->Update();
  ImageType::Pointer outputImage = filter->GetOutput();
  itkUtil::WriteImage< ImageType >(outputImage, outputname);
  return 0;
}
