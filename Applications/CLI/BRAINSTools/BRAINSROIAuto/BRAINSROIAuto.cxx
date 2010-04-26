/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date: 2007-08-31 11:20:20 -0500 (Fri, 31 Aug 2007) $
  Version:   $Revision: 10358 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

  =========================================================================*/

/**
 * Hans J. Johnson @ The University of Iowa
 * This program is a standalone version of a program for masking and clipping images
 * using the ROIAUTO method that seems to work well for brain images.
 */

#if defined( _MSC_VER )
#  pragma warning ( disable : 4786 )
#endif
#include "itkIO.h"
#include "itkMultiplyImageFilter.h"
#include "BRAINSROIAutoCLP.h"
#include "itkLargestForegroundFilledMaskImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageMaskSpatialObject.h"
#include "itkImageRegionIteratorWithIndex.h"

typedef itk::Image<signed short, 3> ImageType;
typedef itk::Image<unsigned char, 3> MaskImageType;

/**
 * This file contains utility functions that are common to a few of the BRAINSFit Programs.
 */

typedef itk::SpatialObject<3>  ImageMaskType;
typedef ImageMaskType::Pointer ImageMaskPointer;

template <class VolumeType>
ImageMaskPointer DoROIAUTO(typename VolumeType::Pointer & extractedVolume,
  const double otsuPercentileThreshold, const int closingSize, const double thresholdCorrectionFactor = 1.0)
{
//   tempMaskImage = FindLargestForgroundFilledMask<VolumeType>(
//                           extractedVolume,
//                           otsuPercentileThreshold,
//                           closingSize);
  typedef itk::LargestForegroundFilledMaskImageFilter<VolumeType> LFFMaskFilterType;
  typename LFFMaskFilterType::Pointer LFF = LFFMaskFilterType::New();
  LFF->SetInput(extractedVolume);
  LFF->SetOtsuPercentileThreshold(otsuPercentileThreshold);
  LFF->SetClosingSize(closingSize);
  LFF->SetThresholdCorrectionFactor(thresholdCorrectionFactor);
  //  LFF->Update();
  
  typedef unsigned char                     NewPixelType;
  typedef itk::Image<NewPixelType, VolumeType::ImageDimension> NewImageType;
  typedef itk::CastImageFilter<VolumeType, NewImageType> CastImageFilter;
  typename CastImageFilter::Pointer castFilter = CastImageFilter::New();
  castFilter->SetInput( LFF->GetOutput() );
  castFilter->Update( );

  // save mask image to output variable
  // tempMaskImage = castFilter->GetOutput();
  // tempMaskImage->DisconnectPipeline();

  // convert mask image to mask
  typedef itk::ImageMaskSpatialObject<VolumeType::ImageDimension> ImageMaskSpatialObjectType;
  typename ImageMaskSpatialObjectType::Pointer mask = ImageMaskSpatialObjectType::New();
  mask->SetImage( castFilter->GetOutput() );
  mask->ComputeObjectToWorldTransform();

  ImageMaskPointer resultMaskPointer = dynamic_cast< ImageMaskType * >( mask.GetPointer() );
  return resultMaskPointer;
}

template <typename PixelType>
void
WriteOutputVolume(ImageType::Pointer image,
                  MaskImageType::Pointer mask,
                  std::string &fileName)
{
  typedef typename itk::Image<PixelType,ImageType::ImageDimension> WriteOutImageType;

  typedef typename itk::MultiplyImageFilter<MaskImageType,ImageType,WriteOutImageType> MultiplierType;

  typename MultiplierType::Pointer clipper=MultiplierType::New();

  clipper->SetInput1(mask);
  clipper->SetInput2(image);
  clipper->Update();

  typename WriteOutImageType::Pointer temp=clipper->GetOutput();
  itkUtil::WriteImage<WriteOutImageType>(temp, fileName);
}

int main( int argc, char *argv[] )
{
  PARSE_ARGS;
  if(inputVolume == "")
    {
    std::cerr << argv[0] << ": Missing required --inputVolume parameter"
             << std::cerr;
    exit(1);
    }
  ImageType::Pointer ImageInput=
    itkUtil::ReadImage<ImageType>(inputVolume);

  ImageMaskPointer maskWrapper = DoROIAUTO<ImageType>(ImageInput,
                                                      otsuPercentileThreshold,
                                                      closingSize,
                                                      thresholdCorrectionFactor);
  // The reference ImageMask is not getting set properly, so:
  typedef itk::ImageMaskSpatialObject<MaskImageType::ImageDimension> ImageMaskSpatialObjectType;
  ImageMaskSpatialObjectType::Pointer fixedImageMask( 
                                                     dynamic_cast< ImageMaskSpatialObjectType * >( maskWrapper.GetPointer() ));
  MaskImageType::Pointer MaskImage = const_cast<MaskImageType *>( fixedImageMask->GetImage() );

  if(outputROIMaskVolume != "" )
    {
    itkUtil::WriteImage<MaskImageType>(MaskImage,outputROIMaskVolume);
    }

  if ( outputClippedVolumeROI != "")
    {
    //      std::cout << "=========== resampledImage :\n" <<
    // resampledImage->GetDirection() << std::endl;
    // Set in PARSEARGS const bool scaleOutputValues=false;//TODO: Make this a
    // command line parameter
    if ( outputVolumePixelType == "float" )
      {
      WriteOutputVolume<float>(ImageInput,MaskImage,outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "short" )
      {
      WriteOutputVolume<signed short>(ImageInput,MaskImage,outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "ushort" )
      {
      WriteOutputVolume<unsigned short>(ImageInput,MaskImage,outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "int" )
      {
      WriteOutputVolume<signed int>(ImageInput,MaskImage,outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "uint" )
      {
      WriteOutputVolume<unsigned int>(ImageInput,MaskImage,outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "uchar" )
      {
      WriteOutputVolume<unsigned char>(ImageInput,MaskImage,outputClippedVolumeROI);
      }
    }

  return 0;
}
