#ifndef __itkIO_h
#define __itkIO_h

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkOrientImageFilter.h"
#include "itkSpatialOrientation.h"
#include "itkSpatialOrientationAdapter.h"
#include "itkAnalyzeImageIO.h"
#include "itkMetaDataObject.h"
#include "itkImageRegionIterator.h"
#include "itkThinPlateR2LogRSplineKernelTransform.h"
#include "itkResampleImageFilter.h"
#include "itkImageDuplicator.h"
#include "Imgmath.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkGDCMImageIO.h"

namespace itkUtil
{
typedef itk::SpatialOrientationAdapter SOAdapterType;
typedef SOAdapterType::DirectionType   DirectionType;

/**
  *
  *
  *
  */
/** read an image using ITK -- image-based template */
template< typename TImage >
typename TImage::Pointer ReadImage(const std::string fileName)
{
  typename TImage::Pointer image;
  std::string               extension = itksys::SystemTools::GetFilenameLastExtension(fileName);
  itk::GDCMImageIO::Pointer dicomIO = itk::GDCMImageIO::New();
  if ( dicomIO->CanReadFile( fileName.c_str() ) || ( itksys::SystemTools::LowerCase(extension) == ".dcm" ) )
    {
    std::string dicomDir = itksys::SystemTools::GetParentDirectory( fileName.c_str() );

    itk::GDCMSeriesFileNames::Pointer FileNameGenerator = itk::GDCMSeriesFileNames::New();
    FileNameGenerator->SetUseSeriesDetails(true);
    FileNameGenerator->SetDirectory(dicomDir);
    typedef const std::vector< std::string > ContainerType;
    const ContainerType & seriesUIDs = FileNameGenerator->GetSeriesUIDs();

    typedef typename itk::ImageSeriesReader< TImage > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileNames( FileNameGenerator->GetFileNames(seriesUIDs[0]) );
    reader->SetImageIO(dicomIO);
    try
      {
      reader->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cout << "Caught an exception: " << std::endl;
      std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
      throw err;
      }
    catch ( ... )
      {
      std::cout << "Error while reading in image for patient " << fileName << std::endl;
      throw;
      }
    image = reader->GetOutput();
    image->DisconnectPipeline();
    reader->ReleaseDataFlagOn();
    }
  else
    {
    typedef itk::ImageFileReader< TImage > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( fileName.c_str() );
    try
      {
      reader->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cout << "Caught an exception: " << std::endl;
      std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
      throw err;
      }
    catch ( ... )
      {
      std::cout << "Error while reading in image" << fileName << std::endl;
      throw;
      }
    image = reader->GetOutput();
    image->DisconnectPipeline();
    reader->ReleaseDataFlagOn();
    }
  return image;
}

/**
  *
  *
  *
  */

template< class ImageType1, class ImageType2 >
bool
ImagePhysicalDimensionsAreIdentical(typename ImageType1::Pointer & inputImage1,
                                    typename ImageType2::Pointer & inputImage2)
{
  bool same = true;

  same &= ( inputImage1->GetDirection() == inputImage2->GetDirection() );
  same &= ( inputImage1->GetSpacing() == inputImage2->GetSpacing() );
  same &= ( inputImage1->GetOrigin() == inputImage2->GetOrigin() );
  return same;
}

template< class ImageType >
typename ImageType::Pointer
OrientImage(typename ImageType::ConstPointer & inputImage,
            itk::SpatialOrientation::ValidCoordinateOrientationFlags orient)
{
  typename itk::OrientImageFilter< ImageType, ImageType >::Pointer orienter =
    itk::OrientImageFilter< ImageType, ImageType >::New();

  orienter->SetDesiredCoordinateOrientation(orient);
  orienter->UseImageDirectionOn();
  orienter->SetInput(inputImage);
  orienter->Update();
  typename ImageType::Pointer returnval =
    orienter->GetOutput();
  returnval->DisconnectPipeline();
  orienter->ReleaseDataFlagOn();
  return returnval;
}

template< class ImageType >
typename ImageType::Pointer
OrientImage(typename ImageType::ConstPointer & inputImage,
            const typename ImageType::DirectionType & dirCosines)
{
  return OrientImage< ImageType >
           ( inputImage,
           SOAdapterType().FromDirectionCosines(
             dirCosines) );
}

template< class ImageType >
typename ImageType::Pointer
OrientImage(typename ImageType::Pointer & inputImage,
            const typename ImageType::DirectionType & dirCosines)
{
  typename ImageType::ConstPointer constImg(inputImage);
  return OrientImage< ImageType >
           ( constImg,
           SOAdapterType().FromDirectionCosines(
             dirCosines) );
}

template< class ImageType >
typename ImageType::Pointer
OrientImage(typename ImageType::Pointer & inputImage,
            itk::SpatialOrientation::ValidCoordinateOrientationFlags orient)
{
  typename ImageType::ConstPointer constImg(inputImage);
  return OrientImage< ImageType >(constImg, orient);
}

template< class ImageType >
typename ImageType::Pointer
ReadImageAndOrient(const std::string & filename,
                   itk::SpatialOrientation::ValidCoordinateOrientationFlags orient)
{
  typename ImageType::Pointer img =
    ReadImage< ImageType >(filename);
  typename ImageType::ConstPointer constImg(img);
  typename ImageType::Pointer image = itkUtil::OrientImage< ImageType >(constImg,
                                                                        orient);
  return image;
}

template< class ImageType >
typename ImageType::Pointer
ReadImageAndOrient(const std::string & filename,
                   const DirectionType & dir)
{
  return ReadImageAndOrient< ImageType >
           ( filename,
           SOAdapterType().FromDirectionCosines(dir) );
}

template< typename  TReadImageType >
typename TReadImageType::Pointer ReadImageCoronal(const std::string & fileName)
{
  DirectionType CORdir = SOAdapterType().ToDirectionCosines
                           (itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP);

  return ReadImageAndOrient< TReadImageType >(fileName, CORdir);
}

template< class ImageType >
void
WriteImage(typename ImageType::Pointer & image,
           const std::string & filename)
{
  typedef itk::ImageFileWriter< ImageType > WriterType;
  typename  WriterType::Pointer writer = WriterType::New();
  writer->UseCompressionOn();
  writer->SetFileName( filename.c_str() );
  writer->SetInput(image);
  try
    {
    writer->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    throw;
    }
}

/**
  *
  *
  * @author hjohnson (6/4/2008)
  *
  * @param InputImageType
  * @param OutputImageType
  * @param input
  *
  * @return typename OutputImageType::Pointer
  */
template< class InputImageType, class OutputImageType >
typename OutputImageType::Pointer
TypeCast(const typename InputImageType::Pointer & input)
{
  typedef itk::CastImageFilter< InputImageType,
                                OutputImageType > CastToRealFilterType;
  typename CastToRealFilterType::Pointer toReal = CastToRealFilterType::New();
  toReal->SetInput(input);
  toReal->Update();
  return toReal->GetOutput();
}

/**
  *   \author Hans J. Johnson
  *   Converts images from one type to
  *  another with explicit min and max values. NOTE:  The original
  *  range of the image is determined explicitly from the data,
  *  and then linearly scaled into the range specified.
  * \param image --The input image to convert and scale
  * \param OuputMin --The required minimum value of the output
  *        image
  * \param OutputMax -- The required maximum value of the output
  *        image
  * \return A new image of the specified type and scale.
  */
template< class InputImageType, class OutputImageType >
typename OutputImageType::Pointer
ScaleAndCast(const typename InputImageType::Pointer & image,
             const typename OutputImageType::PixelType OutputMin,
             const typename OutputImageType::PixelType OutputMax)
{
  typedef itk::RescaleIntensityImageFilter< InputImageType,
                                            OutputImageType > R2CRescaleFilterType;
  typename R2CRescaleFilterType::Pointer RealToProbMapCast =
    R2CRescaleFilterType::New();
  RealToProbMapCast->SetOutputMinimum(OutputMin);
  RealToProbMapCast->SetOutputMaximum(OutputMax);
  RealToProbMapCast->SetInput(image);
  try
    {
    RealToProbMapCast->Update();
    }
  catch ( itk::ExceptionObject & e )
    {
    std::cerr << "Exception in Image cast." << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    exit(-1);
    }
  typename OutputImageType::Pointer returnScaled = RealToProbMapCast->GetOutput();
  return returnScaled;
}

/**
  * This function will do a type cast if the OutputImageType
  * intensity range is larger than the input image type range.
  * If the OutputImageType range is smaller, then a Scaling will
  * occur.
  *
  * @author hjohnson (6/4/2008)
  *
  * @param InputImageType
  * @param OutputImageType
  * @param image
  *
  * @return typename OutputImageType::Pointer
  */
template< class InputImageType, class OutputImageType >
typename OutputImageType::Pointer
PreserveCast(const typename InputImageType::Pointer image)
{
  const typename InputImageType::PixelType inputmin =
    itk::NumericTraits< typename InputImageType::PixelType >::min();
  const typename InputImageType::PixelType inputmax =
    itk::NumericTraits< typename InputImageType::PixelType >::max();
  const typename OutputImageType::PixelType outputmin =
    itk::NumericTraits< typename OutputImageType::PixelType >::min();
  const typename OutputImageType::PixelType outputmax =
    itk::NumericTraits< typename OutputImageType::PixelType >::max();
  if ( ( inputmin >= outputmin ) && ( inputmax <= outputmax ) )
    {
    return TypeCast< InputImageType, OutputImageType >(image);
    }
  else
    {
    return ScaleAndCast< InputImageType, OutputImageType >(image,
                                                           outputmin,
                                                           outputmax);
    }
}

template< class ImageType >
typename ImageType::Pointer
CopyImage(const typename ImageType::Pointer & input)
{
  typedef itk::ImageDuplicator< ImageType > ImageDupeType;
  typename ImageDupeType::Pointer MyDuplicator = ImageDupeType::New();
  MyDuplicator->SetInputImage(input);
  MyDuplicator->Update();
  return MyDuplicator->GetOutput();
}
}

#endif
