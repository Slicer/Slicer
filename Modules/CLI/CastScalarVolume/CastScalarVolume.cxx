/*=========================================================================

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkImageFileWriter.h"

#include "itkCastImageFilter.h"

#include "itkPluginUtilities.h"
#include "CastScalarVolumeCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class Tin, class Tout>
int DoIt( int argc, char * argv[], Tin, Tout )
{

  PARSE_ARGS;

  typedef    Tin InputPixelType;
  typedef Tout   OutputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;

  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  typedef itk::CastImageFilter<
    InputImageType, OutputImageType>  FilterType;

  typename ReaderType::Pointer reader1 = ReaderType::New();
  itk::PluginFilterWatcher watchReader1(reader1, "Read Volume",
                                        CLPProcessInformation);

  reader1->SetFileName( InputVolume.c_str() );

  typename FilterType::Pointer filter = FilterType::New();
  itk::PluginFilterWatcher watchFilter(filter,
                                       "Cast image",
                                       CLPProcessInformation);

  filter->SetInput( 0, reader1->GetOutput() );

  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
  writer->SetFileName( OutputVolume.c_str() );
  writer->SetInput( filter->GetOutput() );
  writer->SetUseCompression(1);
  writer->Update();
  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(InputVolume, pixelType, componentType);

    // This filter handles all types on input
    switch( componentType )
      {
      case itk::ImageIOBase::CHAR:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<char>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::SHORT:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<short>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::INT:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<int>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::LONG:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<long>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::UCHAR:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<unsigned char>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::USHORT:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<unsigned short>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::UINT:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<unsigned int>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::ULONG:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<unsigned long>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::FLOAT:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<float>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::DOUBLE:
        if( Type == std::string("Char") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<char>(0) );
          }
        else if( Type == std::string("UnsignedChar") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<unsigned char>(0) );
          }
        else if( Type == std::string("Short") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<short>(0) );
          }
        else if( Type == std::string("UnsignedShort") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<unsigned short>(0) );
          }
        else if( Type == std::string("Int") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<int>(0) );
          }
        else if( Type == std::string("UnsignedInt") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<unsigned int>(0) );
          }
        else if( Type == std::string("Long") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<long>(0) );
          }
        else if( Type == std::string("UnsignedLong") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<unsigned long>(0) );
          }
        else if( Type == std::string("Float") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<float>(0) );
          }
        else if( Type == std::string("Double") )
          {
          return DoIt( argc, argv, static_cast<double>(0), static_cast<double>(0) );
          }
        else
          {
          std::cout << "Unknown type to which to cast input volume: " << Type.c_str() << std::endl;
          return EXIT_FAILURE;
          }
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "Unknown component type " << componentType << std::endl;
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
