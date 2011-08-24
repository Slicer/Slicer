#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include <algorithm>
#include <string>
#include <itkMetaDataObject.h>
#include <itkImage.h>
#include <itkVector.h>
#include <itkVectorImage.h>

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkNrrdImageIO.h>

#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>

#include "DiffusionWeightedTestCLP.h"

#define DIMENSION 3

template <class PixelType>
int DoIt( int argc, char * argv[], PixelType )
{

  PARSE_ARGS;

  typedef itk::VectorImage<PixelType, DIMENSION>       DiffusionImageType;
  typedef itk::Image<PixelType, DIMENSION>             ScalarImageType;
  typedef double                                       PixelTypeDouble;
  typedef itk::VectorImage<PixelTypeDouble, DIMENSION> DoubleDiffusionImageType;
  typedef itk::Image<PixelTypeDouble, DIMENSION>       ScalarDoubleImageType;
  typedef itk::CovariantVector<double, DIMENSION>      CovariantVectorType;
  std::vector<CovariantVectorType> diffusionDirections;

  typedef itk::ImageFileReader<DiffusionImageType> FileReaderType;
  typename FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetFileName( inputVolume.c_str() );
  reader->Update();

  typedef itk::MetaDataDictionary DictionaryType;
  const DictionaryType & dictionary = reader->GetMetaDataDictionary();

  typedef itk::MetaDataObject<std::string> MetaDataStringType;

  DictionaryType::ConstIterator itr = dictionary.Begin();
  DictionaryType::ConstIterator end = dictionary.End();

  while( itr != end )
    {
    if( itr->first == "DWMRI_b-value" )
      {
      std::string bValueString;
      itk::ExposeMetaData<std::string>(dictionary, itr->first, bValueString);
      std::cout << "DWMRI_b-value(string): " << bValueString << std::endl;
      }
    else if( itr->first.find("DWMRI_gradient") != std::string::npos )
      {
      std::string gradientValueString;
      itk::ExposeMetaData<std::string>(dictionary, itr->first, gradientValueString);
      std::cout << "DWMRI_gradient(string): " << gradientValueString << std::endl;
      }
    else if( itr->first.find("NRRD_measurement frame") != std::string::npos )
      {
      std::vector<std::vector<double> > measurementFrameValue(3);
      for( unsigned int i = 0; i < 3; i++ )
        {
        measurementFrameValue[i].resize(3);
        }

      itk::ExposeMetaData<std::vector<std::vector<double> > >(dictionary, itr->first, measurementFrameValue);

      std::cout << itr->first << ": " << std::endl;
      for( unsigned int i = 0; i < 3; i++ )
        {
        for( unsigned int j = 0; j < 3; j++ )
          {
          std::cout << measurementFrameValue[i][j] << " ";
          }
        std::cout << std::endl;
        }

      std::cout << std::endl;
      }
    else
      {
      std::cout << itr->first << std::endl;
      }

    ++itr;
    }

  typedef itk::ImageFileWriter<DiffusionImageType> WriterType;
  typename WriterType::Pointer nrrdWriter = WriterType::New();
  nrrdWriter->SetInput( reader->GetOutput() );
  nrrdWriter->SetFileName( outputVolume.c_str() );
  nrrdWriter->UseCompressionOn();
  try
    {
    nrrdWriter->Update();
    std::cout << "Done writting the volume" << std::endl;
    }
  catch( itk::ExceptionObject e )
    {
    std::cout << e << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(inputVolume, pixelType, componentType);

    // This filter handles all types

    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt( argc, argv, static_cast<char>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0) );
        break;
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0) );
        break;
      case itk::ImageIOBase::LONG:
        return DoIt( argc, argv, static_cast<long>(0) );
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0) );
        // std::cout << "FLOAT type not currently supported." << std::endl;
        break;
      case itk::ImageIOBase::DOUBLE:
        std::cout << "DOUBLE type not currently supported." << std::endl;
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
