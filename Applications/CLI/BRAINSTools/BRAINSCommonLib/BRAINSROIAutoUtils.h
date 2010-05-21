#ifndef __BRAINSROIAUTOUTILS_H__
#define __BRAINSROIAUTOUTILS_H__

/**
 *\author Hans J. Johnson
 *This is a set of utility functions to help with identifying common tissue
 * Regions in an image.
 */

//HACK:  This should really be a class with multiple member variables to hold the different types of requested outputs.
//

#include "itkImageMaskSpatialObject.h"
#include "itkLargestForegroundFilledMaskImageFilter.h"
#include "itkCastImageFilter.h"


typedef itk::SpatialObject<3>  SpatialObjectType;
typedef SpatialObjectType::Pointer ImageMaskPointer;


template <class InputVolumeType, class OutputVolumeType>
typename OutputVolumeType::Pointer DoROIAUTOImage(typename InputVolumeType::Pointer & originalVolume,
    const double otsuPercentileThreshold, const int closingSize, const double thresholdCorrectionFactor = 1.0)
{
  typedef itk::CastImageFilter<InputVolumeType, OutputVolumeType> CastImageFilter;
  typename CastImageFilter::Pointer castFilter = CastImageFilter::New();
    {
    typedef itk::LargestForegroundFilledMaskImageFilter<InputVolumeType> LFFMaskFilterType;
    typename LFFMaskFilterType::Pointer LFF = LFFMaskFilterType::New();
    LFF->SetInput(originalVolume);
    LFF->SetOtsuPercentileThreshold(otsuPercentileThreshold);
    LFF->SetClosingSize(closingSize);
    LFF->SetThresholdCorrectionFactor(thresholdCorrectionFactor);
    LFF->Update();
    castFilter->SetInput( LFF->GetOutput() );
    }
  castFilter->Update( );
  return castFilter->GetOutput();
}

template <class InputVolumeType>
ImageMaskPointer ConvertImageToSpatialObjectMask(typename InputVolumeType::Pointer & volumeImageMask)
{
  typedef itk::Image<unsigned char, 3> UCHARIMAGE;

  typedef itk::CastImageFilter<InputVolumeType, UCHARIMAGE> CastImageFilter;
  typename CastImageFilter::Pointer castFilter = CastImageFilter::New();
  castFilter->SetInput( volumeImageMask );
  castFilter->Update( );

  // convert mask image to mask
  typedef itk::ImageMaskSpatialObject<itk::Image<unsigned char,3>::ImageDimension> ImageMaskSpatialObjectType;
  typename ImageMaskSpatialObjectType::Pointer mask = ImageMaskSpatialObjectType::New();
  mask->SetImage( castFilter->GetOutput() );
  mask->ComputeObjectToWorldTransform();

  ImageMaskPointer resultMaskPointer = dynamic_cast< ImageMaskSpatialObjectType * >( mask.GetPointer() );
  return resultMaskPointer;
}


template <class InputVolumeType>
ImageMaskPointer DoROIAUTO(typename InputVolumeType::Pointer & originalVolume,
  const double otsuPercentileThreshold, const int closingSize, const double thresholdCorrectionFactor = 1.0)
{
  typedef unsigned char                     NewPixelType;
  typedef itk::Image<NewPixelType, InputVolumeType::ImageDimension> NewImageType;
  typename NewImageType::Pointer ucharImage=DoROIAUTOImage<InputVolumeType,NewImageType>(originalVolume,otsuPercentileThreshold,closingSize,thresholdCorrectionFactor);

  ImageMaskPointer resultMaskPointer = ConvertImageToSpatialObjectMask<NewImageType>(ucharImage);
  return resultMaskPointer;
}

#endif //__BRAINSROIAUTOUTILS_H__
