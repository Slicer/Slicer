
#include <iostream>
#include <algorithm>
#include <string>
#include <itkMetaDataObject.h>
#include <itkImage.h>
#include <itkVector.h>
#include <itkVectorImage.h>

#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkNrrdImageIO.h>

#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>

#include <vcl_algorithm.h>

#include "DWICompareCLP.h"

namespace
{
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
  typename FileReaderType::Pointer firstReader = FileReaderType::New();
  typename FileReaderType::Pointer secondReader = FileReaderType::New();
  firstReader->SetFileName( inputVolume1.c_str() );
  firstReader->Update();
  secondReader->SetFileName( inputVolume2.c_str() );
  secondReader->Update();

  typedef itk::MetaDataDictionary DictionaryType;
  const DictionaryType & firstDictionary = firstReader->GetMetaDataDictionary();
  const DictionaryType & secondDictionary = secondReader->GetMetaDataDictionary();

  typedef itk::MetaDataObject<std::string> MetaDataStringType;

  DictionaryType::ConstIterator itr = firstDictionary.Begin();
  DictionaryType::ConstIterator end = firstDictionary.End();

  bool failure = false;

  // If the angle between two gradients differs more than this value they are
  // considered to be non-colinear
  double gradientToleranceForSameness = 1;
  float  bValueTolerance = 0.5;

  while( itr != end )
    {
    if( itr->first == "DWMRI_b-value" )
      {
      std::string firstBValueString;
      std::string secondBValueString;
      itk::ExposeMetaData<std::string>(firstDictionary, itr->first, firstBValueString);
      if( !secondDictionary.HasKey(itr->first) )
        {
        std::cout << "Key " << itr->first << " doesn't exist in 2nd image" << std::endl;
        return EXIT_FAILURE;
        }

      itk::ExposeMetaData<std::string>(secondDictionary, itr->first, secondBValueString);

      std::istringstream iss(secondBValueString);
      int                firstBValue;
      int                secondBValue;
      iss >> firstBValue;
      iss.str(secondBValueString);
      iss.clear();
      iss >> secondBValue;

      if( abs(firstBValue - secondBValue) / firstBValue > bValueTolerance )
        {
        std::cout << "firstBValue String != secondBValueString! "
                  << firstBValueString << "!=" << secondBValueString << std::endl;
        return EXIT_FAILURE;
        }
      }
    else if( itr->first.find("DWMRI_gradient") != std::string::npos )
      {
      std::string firstGradientValueString;
      itk::ExposeMetaData<std::string>(firstDictionary, itr->first, firstGradientValueString);

      vnl_vector_fixed<double, 3> firstGradientVector;
      std::istringstream          iss(firstGradientValueString);
      iss >> firstGradientVector[0] >> firstGradientVector[1] >> firstGradientVector[2];
      firstGradientVector.normalize();

      if( !secondDictionary.HasKey(itr->first) )
        {
        std::cout << "Key " << itr->first << " doesn't exist in 2nd image" << std::endl;
        return EXIT_FAILURE;
        }

      std::string secondGradientValueString;
      itk::ExposeMetaData<std::string>(secondDictionary, itr->first, secondGradientValueString);

      vnl_vector_fixed<double, 3> secondGradientVector;
      iss.str(secondGradientValueString);
      iss.clear();
      iss >> secondGradientVector[0] >> secondGradientVector[1] >> secondGradientVector[2];
      secondGradientVector.normalize();

      if( firstGradientValueString != secondGradientValueString )
        {
        // Check to see if gradients are colinear within tolerance
        double gradientDot = dot_product(firstGradientVector, secondGradientVector);

        double magnitudesProduct = secondGradientVector.magnitude() * firstGradientVector.magnitude();
        double sendToArcCos = gradientDot / magnitudesProduct;

        sendToArcCos = ( sendToArcCos > 1 ) ? 1 : sendToArcCos;
        // Avoid numerical precision problems
        sendToArcCos = ( sendToArcCos < -1 ) ? -1 : sendToArcCos;
        // Avoid numerical precision problems

        const double gradientAngle = vcl_abs( vcl_acos(sendToArcCos) * 180.0 * vnl_math::one_over_pi);

        double gradientMinAngle = vcl_min( gradientAngle, vcl_abs(180.0 - gradientAngle) );
        if( gradientMinAngle > gradientToleranceForSameness )
          {
          std::cout << "GradientValueStrings don't match! " << firstGradientValueString
                    << " != " << secondGradientValueString << std::endl;
          failure = true;
          }
        }
      }
    else if( itr->first.find("NRRD_measurement frame") != std::string::npos )
      {
      std::vector<std::vector<double> > firstMeasurementFrameValue(3);
      for( unsigned int i = 0; i < 3; i++ )
        {
        firstMeasurementFrameValue[i].resize(3);
        }

      itk::ExposeMetaData<std::vector<std::vector<double> > >(firstDictionary, itr->first, firstMeasurementFrameValue);

      if( !secondDictionary.HasKey(itr->first) )
        {
        std::cout << "Key " << itr->first << " doesn't exist in 2nd image" << std::endl;
        return EXIT_FAILURE;
        }

      std::vector<std::vector<double> > secondMeasurementFrameValue(3);
      for( unsigned int i = 0; i < 3; i++ )
        {
        secondMeasurementFrameValue[i].resize(3);
        }

      itk::ExposeMetaData<std::vector<std::vector<double> > >(secondDictionary, itr->first, secondMeasurementFrameValue);
      for( unsigned int i = 0; i < 3; i++ )
        {
        for( unsigned int j = 0; j < 3; j++ )
          {
          if( firstMeasurementFrameValue[i][j] != secondMeasurementFrameValue[i][j] )
            {
            std::cout << "Measurement frames do not match!" << std::endl;
            return EXIT_FAILURE;
            }
          }
        }

      }

    ++itr;
    }

  if( failure )
    {
    return EXIT_FAILURE;
    }
  else
    {
    return EXIT_SUCCESS;
    }
}

void GetImageType(std::string fileName,
                  itk::ImageIOBase::IOPixelType & pixelType,
                  itk::ImageIOBase::IOComponentType & componentType)
{
  typedef itk::Image<short, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader =
    itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName(fileName.c_str() );
  imageReader->UpdateOutputInformation();

  pixelType = imageReader->GetImageIO()->GetPixelType();
  componentType = imageReader->GetImageIO()->GetComponentType();
}

}

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    // itk::GetImageType (inputVolume1, pixelType, componentType);
    GetImageType(inputVolume1, pixelType, componentType);

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
