/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile$
 *  Language:  C++
 *  Date:      $Date: 2007-08-31 11:20:20 -0500 (Fri, 31 Aug 2007) $
 *  Version:   $Revision: 10358 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

/**
  * Hans J. Johnson @ The University of Iowa
  * This program is a standalone version of a program for masking and clipping
  *images
  * using the ROIAUTO method that seems to work well for brain images.
  */

#if defined( _MSC_VER )
#  pragma warning ( disable : 4786 )
#endif
#include "itkIO.h"

#include "itkMultiplyImageFilter.h"
#include "itkLargestForegroundFilledMaskImageFilter.h"

#include "itkImageMaskSpatialObject.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkBRAINSROIAutoImageFilter.h"
#include "BRAINSROIAutoCLP.h"

typedef itk::Image< signed short, 3 >  VolumeImageType;
typedef itk::Image< unsigned char, 3 > VolumeMaskType;
typedef itk::SpatialObject< 3 >        SOImageMaskType;

/**
  * This file contains utility functions that are common to a few of the
  *BRAINSFit Programs.
  */

template< typename PixelType >
void
BRAINSROIAUTOWriteOutputVolume(VolumeImageType::Pointer image,
                               VolumeMaskType::Pointer mask,
                               std::string & fileName)
{
  typedef typename itk::Image< PixelType, VolumeImageType::ImageDimension > WriteOutImageType;

  typedef typename itk::MultiplyImageFilter< VolumeMaskType, VolumeImageType, WriteOutImageType > MultiplierType;

  typename MultiplierType::Pointer clipper = MultiplierType::New();

  clipper->SetInput1(mask);
  clipper->SetInput2(image);
  clipper->Update();

  typename WriteOutImageType::Pointer temp = clipper->GetOutput();
  itkUtil::WriteImage< WriteOutImageType >(temp, fileName);
}

int main(int argc, char *argv[])
{
  PARSE_ARGS;
  if ( inputVolume == "" )
    {
    std::cerr << argv[0] << ": Missing required --inputVolume parameter"
              << std::cerr;
    exit(1);
    }
  VolumeImageType::Pointer ImageInput =
    itkUtil::ReadImage< VolumeImageType >(inputVolume);

  typedef itk::BRAINSROIAutoImageFilter< VolumeImageType, VolumeMaskType > ROIAutoType;
  ROIAutoType::Pointer ROIFilter = ROIAutoType::New();
  ROIFilter->SetInput(ImageInput);
  ROIFilter->SetOtsuPercentileThreshold(otsuPercentileThreshold);
  ROIFilter->SetClosingSize(closingSize);
  ROIFilter->SetThresholdCorrectionFactor(thresholdCorrectionFactor);
  ROIFilter->SetDilateSize(ROIAutoDilateSize);
  ROIFilter->Update();
  SOImageMaskType::Pointer maskWrapper = ROIFilter->GetSpatialObjectROI();
  VolumeMaskType::Pointer  MaskImage = ROIFilter->GetOutput();

  if ( outputROIMaskVolume != "" )
    {
    itkUtil::WriteImage< VolumeMaskType >(MaskImage, outputROIMaskVolume);
    }

  if ( outputClippedVolumeROI != "" )
    {
    //      std::cout << "=========== resampledImage :\n" <<
    // resampledImage->GetDirection() << std::endl;
    // Set in PARSEARGS const bool scaleOutputValues=false;//TODO: Make this a
    // command line parameter
    if ( outputVolumePixelType == "float" )
      {
      BRAINSROIAUTOWriteOutputVolume< float >(ImageInput, MaskImage, outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "short" )
      {
      BRAINSROIAUTOWriteOutputVolume< signed short >(ImageInput, MaskImage, outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "ushort" )
      {
      BRAINSROIAUTOWriteOutputVolume< unsigned short >(ImageInput, MaskImage, outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "int" )
      {
      BRAINSROIAUTOWriteOutputVolume< signed int >(ImageInput, MaskImage, outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "uint" )
      {
      BRAINSROIAUTOWriteOutputVolume< unsigned int >(ImageInput, MaskImage, outputClippedVolumeROI);
      }
    else if ( outputVolumePixelType == "uchar" )
      {
      BRAINSROIAUTOWriteOutputVolume< unsigned char >(ImageInput, MaskImage, outputClippedVolumeROI);
      }
    }
  return 0;
}
