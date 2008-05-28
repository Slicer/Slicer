#include <iostream>
#include "HelloSlicerCLP.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiscreteGaussianImageFilter.h"

int main(int argc, char * argv [])
{
  PARSE_ARGS;
  std::cout << "Hello Slicer!" << std::endl;
  typedef itk::Image< short,  3 >   ImageType;
  typedef itk::ImageFileReader< ImageType  >  ReaderType;
  typedef itk::ImageFileWriter< ImageType >  WriterType;
  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( helloSlicerInputVolume.c_str() );
  writer->SetFileName (helloSlicerOutputVolume.c_str());

  typedef itk::DiscreteGaussianImageFilter <ImageType, ImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();

  try {  
    filter->SetInput(reader->GetOutput());
    filter->SetVariance(variance);
    writer->SetInput(filter->GetOutput());
    writer->Update();
  }

  catch (itk::ExceptionObject &excep)
  {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
