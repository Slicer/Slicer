#if 0
#ifndef __BRAINSROIAUTOUTILS_H__
#define __BRAINSROIAUTOUTILS_H__

#include "itkBRAINSROIAutoImageFilter.h"


template <class InputVolumeType, class OutputVolumeType>
typename OutputVolumeType::Pointer DoROIAUTOImage(typename InputVolumeType::Pointer & originalVolume,
    const double otsuPercentileThreshold, const int closingSize, const double thresholdCorrectionFactor = 1.0)
{
  typedef itk::BRAINSROIAutoImageFilter<InputVolumeType,OutputVolumeType> ROIAutoType;
  typename ROIAutoType::Pointer  ROIFilter=ROIAutoType::New();
  ROIFilter->SetInput(originalVolume);
  ROIFilter->SetOtsuPercentileThreshold(otsuPercentileThreshold);
  ROIFilter->SetClosingSize(closingSize);
  ROIFilter->SetThresholdCorrectionFactor(thresholdCorrectionFactor);
  ROIFilter->SetDilateSize(0);
  ROIFilter->Update();
  return ROIFilter->GetOutput();
}


template <class InputVolumeType>
ImageMaskPointer DoROIAUTO(typename InputVolumeType::Pointer & originalVolume,
  const double otsuPercentileThreshold, const int closingSize, const double thresholdCorrectionFactor = 1.0)
{
  typedef itk::BRAINSROIAutoImageFilter<InputVolumeType,itk::Image<unsigned char,3> > ROIAutoType;
  typename ROIAutoType::Pointer  ROIFilter=ROIAutoType::New();
  ROIFilter->SetInput(originalVolume);
  ROIFilter->SetOtsuPercentileThreshold(otsuPercentileThreshold);
  ROIFilter->SetClosingSize(closingSize);
  ROIFilter->SetThresholdCorrectionFactor(thresholdCorrectionFactor);
  ROIFilter->SetDilateSize(0);
  ROIFilter->Update();
  return ROIFilter->GetSpatialObjectROI();
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
  typedef itk::ImageMaskSpatialObject<UCHARIMAGE::ImageDimension> ImageMaskSpatialObjectType;
  typename ImageMaskSpatialObjectType::Pointer mask = ImageMaskSpatialObjectType::New();
  mask->SetImage( castFilter->GetOutput() );
  mask->ComputeObjectToWorldTransform();

  ImageMaskPointer resultMaskPointer = dynamic_cast< ImageMaskSpatialObjectType * >( mask.GetPointer() );
  return resultMaskPointer;
}

#endif
#endif //__BRAINSROIAUTOUTILS_H__
