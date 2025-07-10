#include "itkImageFileWriter.h"

#include "itkSmoothingRecursiveGaussianImageFilter.h"

#include "itkPluginUtilities.h"

#include "GaussianBlurImageFilterCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class T>
int DoIt( int argc, char* argv[], T )
{
  PARSE_ARGS;

  typedef    T InputPixelType;
  typedef    T OutputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;

  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  typedef itk::SmoothingRecursiveGaussianImageFilter<
    InputImageType, OutputImageType>  FilterType;

  typename ReaderType::Pointer reader = ReaderType::New();

  reader->SetFileName( inputVolume.c_str() );

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput( reader->GetOutput() );
  filter->SetSigma( sigma );

  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputVolume.c_str() );
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char* argv[] )
{
  PARSE_ARGS;

  itk::IOPixelEnum     pixelType;
  itk::IOComponentEnum componentType;

  try
  {
    itk::GetImageType(inputVolume, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types
    switch( componentType )
    {
      case itk::IOComponentEnum::UCHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::IOComponentEnum::CHAR:
        return DoIt( argc, argv, static_cast<char>(0) );
        break;
      case itk::IOComponentEnum::USHORT:
        return DoIt( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::IOComponentEnum::SHORT:
        return DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::IOComponentEnum::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0) );
        break;
      case itk::IOComponentEnum::INT:
        return DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::IOComponentEnum::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0) );
        break;
      case itk::IOComponentEnum::LONG:
        return DoIt( argc, argv, static_cast<long>(0) );
        break;
      case itk::IOComponentEnum::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0) );
        break;
      case itk::IOComponentEnum::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0) );
        break;
      case itk::IOComponentEnum::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
    }
  }

  catch ( itk::ExceptionObject& excep )
  {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
