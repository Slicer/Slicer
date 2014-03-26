/*=========================================================================

This plugin is based on the Insight Toolkit example
ImageReadDicomWrite. It has been modified for GenerateCLP style
command line processing and additional features have been added.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConnectedComponentImageFilter.h"

#include "ConnectedComponentCLP.h"
#include "itkPluginUtilities.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class Tin>
int DoIt( int argc, char * argv[])
{
  PARSE_ARGS;

  typedef Tin     InputPixelType;

  typedef itk::Image<InputPixelType, 3>                        InputImageType;

  typedef itk::ImageFileReader<InputImageType>                    ReaderType;
  typedef itk::ImageFileWriter<InputImageType>                    WriterType;

  typename InputImageType::Pointer image;
  typename ReaderType::Pointer  reader = ReaderType::New();

  typedef itk::ConnectedComponentImageFilter<InputImageType, InputImageType>  FilterType;

  typename ReaderType::Pointer reader1 = ReaderType::New();
  itk::PluginFilterWatcher watchReader1(reader1, "Read Volume", CLPProcessInformation);

  reader1->SetFileName( InputVolume.c_str() );

  typename FilterType::Pointer filter = FilterType::New();
  itk::PluginFilterWatcher watchFilter(filter,   "Processing", CLPProcessInformation);

  filter->SetInput(reader1->GetOutput());

  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
  writer->SetFileName( OutputVolume.c_str() );
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char* argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(InputVolume, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types

    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt<unsigned char>( argc, argv);
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt<char>( argc, argv);
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt<unsigned short>( argc, argv);
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt<short>( argc, argv);
        break;
      case itk::ImageIOBase::UINT:
        return DoIt<unsigned int>( argc, argv);
        break;
      case itk::ImageIOBase::INT:
        return DoIt<int>( argc, argv);
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt<unsigned long>( argc, argv);
        break;
      case itk::ImageIOBase::LONG:
        return DoIt<long>( argc, argv);
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
