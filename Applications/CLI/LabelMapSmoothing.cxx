 /*=========================================================================
  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: OtsuThresholdImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2006-11-15 07:00:22 -0500 (Wed, 15 Nov 2006) $
  Version:   $Revision: 1591 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkAntiAliasBinaryImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkMinimumMaximumImageFilter.h"
#include "itkLabelStatisticsImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkPasteImageFilter.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkPluginFilterWatcher.h"

#include "LabelMapSmoothingCLP.h"

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  unsigned int numberOfLayers = 2;
  unsigned int boundingBoxPadding = vnl_math_max((unsigned int)(std::ceil(4.0*gaussianSigma)),numberOfLayers);

// Image Types
  const unsigned short ImageDimension = 3;
  typedef itk::Image< float, ImageDimension >  FloatImageType;
  typedef itk::Image< unsigned char, ImageDimension >    UCharImageType;

// Filter Types
  typedef itk::BinaryThresholdImageFilter< UCharImageType, UCharImageType > InputThresholdType;
  typedef itk::MinimumMaximumImageFilter< UCharImageType > MinMaxType;
  typedef itk::LabelStatisticsImageFilter< UCharImageType, UCharImageType > LabelStatisticsType;
  typedef itk::RegionOfInterestImageFilter< UCharImageType, UCharImageType > ExtracterType;
  typedef itk::AntiAliasBinaryImageFilter< UCharImageType, FloatImageType > AntiAliasType;
  typedef itk::DiscreteGaussianImageFilter< FloatImageType, FloatImageType >  GaussianType;
  typedef itk::BinaryThresholdImageFilter< FloatImageType, UCharImageType > OutputThresholdType;
  typedef itk::PasteImageFilter< UCharImageType, UCharImageType, UCharImageType> PasteType;

// I/O Types
  typedef itk::ImageFileReader< UCharImageType >  ReaderType;
  typedef itk::ImageFileWriter< UCharImageType >  WriterType;

// Instances
  ReaderType::Pointer reader = ReaderType::New();
  InputThresholdType::Pointer inputThresholder = InputThresholdType::New();
  MinMaxType::Pointer minMaxCalculator = MinMaxType::New();
  LabelStatisticsType::Pointer labelStatisticsFilter = LabelStatisticsType::New();
  ExtracterType::Pointer extracter = ExtracterType::New();
  AntiAliasType::Pointer antiAliasFilter = AntiAliasType::New();
  GaussianType::Pointer gaussianFilter = GaussianType::New();
  OutputThresholdType::Pointer outputThresholder = OutputThresholdType::New();
  PasteType::Pointer paster = PasteType::New();
  WriterType::Pointer writer = WriterType::New();

// Watchers
  itk::PluginFilterWatcher AntiAliasWatcher(antiAliasFilter, "Anti Alias Image Filter", CLPProcessInformation, 2.0/3.0, 0.0);
  itk::PluginFilterWatcher GaussianWatcher(gaussianFilter, "Gaussian Image Filter", CLPProcessInformation, 1.0/3.0, 2.0/3.0);

  try
    {

  reader->SetFileName (inputVolume.c_str());
  reader->Update();

  // Choose a label to smooth.  All others will be ignored.  
  // If the chosen label is greater than the largest label in the
  // image or lower than the smallest, the label will be set to the
  // closest one.
  // If no label is selected by the user, the maximum label in the image is chosen by default.
  minMaxCalculator->SetInput( reader->GetOutput() );
  minMaxCalculator->ReleaseDataFlagOn();
  minMaxCalculator->Update();
  
  if( labelToSmooth == -1 )
    {
    labelToSmooth = minMaxCalculator->GetMaximum();
    }
  else if( labelToSmooth > minMaxCalculator->GetMaximum() )
    {
    labelToSmooth = minMaxCalculator->GetMaximum();
    }
  else if( labelToSmooth < minMaxCalculator->GetMinimum() )
    {
    labelToSmooth = minMaxCalculator->GetMinimum();
    }


  inputThresholder->SetInput( reader->GetOutput() );
  inputThresholder->SetInsideValue( 1 );
  inputThresholder->SetOutsideValue( 0 );
  inputThresholder->SetLowerThreshold( labelToSmooth );
  inputThresholder->SetUpperThreshold( labelToSmooth );
  inputThresholder->ReleaseDataFlagOn();
  inputThresholder->Update();

  // Find the bounding box of the desired label.
  labelStatisticsFilter->SetInput( inputThresholder->GetOutput() );
  labelStatisticsFilter->SetLabelInput( inputThresholder->GetOutput() );
  //labelStatisticsFilter->ReleaseDataFlagOn();
  labelStatisticsFilter->Update();
  LabelStatisticsType::BoundingBoxType boundingBox = labelStatisticsFilter->GetBoundingBox( 1 );
  // Extend the bounding box in each direction to ensure that the
  // cropping of the image does not affect the final result.
  // Make sure the new bounding box does not extend outside of the
  // original image size.
  UCharImageType::SizeType imageSize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
  UCharImageType::RegionType::SizeType regionSize;
  UCharImageType::IndexType regionIndex;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    boundingBox[2*i] = vnl_math_max(0,(int)(boundingBox[2*i]-boundingBoxPadding));
    boundingBox[2*i+1] = vnl_math_min((int)(imageSize[i]-1),(int)(boundingBox[2*i+1]+boundingBoxPadding));
    regionIndex[i] = boundingBox[2*i];
    regionSize[i] = boundingBox[2*i+1]-boundingBox[2*i]+1;
    }

  // Create a region corresponding to the bounding box.
  UCharImageType::RegionType boundingRegion;
  boundingRegion.SetSize( regionSize );
  boundingRegion.SetIndex( regionIndex );

  // Extract the region from the image.
  extracter->SetInput( inputThresholder->GetOutput() );
  extracter->SetRegionOfInterest ( boundingRegion );
  extracter->ReleaseDataFlagOn();
  extracter->Update();

  FloatImageType::Pointer antiAliasImage;
  {
  antiAliasFilter->SetInput( extracter->GetOutput() );  
  antiAliasFilter->SetMaximumRMSError( maxRMSError );
  antiAliasFilter->SetNumberOfIterations( numberOfIterations );
  antiAliasFilter->SetNumberOfLayers( numberOfLayers );
  antiAliasFilter->ReleaseDataFlagOn();
  antiAliasFilter->Update();
  
  antiAliasImage = antiAliasFilter->GetOutput();
  antiAliasImage->DisconnectPipeline();
  }
  
  gaussianFilter->SetInput( antiAliasImage );
  gaussianFilter->SetVariance( gaussianSigma * gaussianSigma );
  gaussianFilter->ReleaseDataFlagOn();
  gaussianFilter->Update();

  outputThresholder->SetInput( gaussianFilter->GetOutput() );
  outputThresholder->SetLowerThreshold( 0 );
  outputThresholder->SetInsideValue( 1 );
  outputThresholder->SetOutsideValue( 0 );
  outputThresholder->ReleaseDataFlagOn();
  outputThresholder->Update();

  // Paste the smoothed result back into the original thresholded
  // image.  This is done rather than creating a new volume to save
  // memory.  Since the bounding box ensures that the entire region of
  // the inputThresholder image is included, the pasting will cover
  // all foreground regions.
  paster->SetDestinationImage( inputThresholder->GetOutput() );
  paster->SetSourceImage( outputThresholder->GetOutput() );
  paster->SetSourceRegion( outputThresholder->GetOutput()->GetRequestedRegion() );
  paster->SetDestinationIndex( regionIndex );
  paster->ReleaseDataFlagOn();
  paster->Update();

  writer->SetInput( paster->GetOutput() );
  writer->SetFileName( outputVolume.c_str() );
  writer->Update();


    }
  catch( itk::ExceptionObject & exc ) 
    { 
    std::cout << "ExceptionObject caught !" << std::endl; 
    std::cout << exc << std::endl; 
    return EXIT_FAILURE;
    } 
  catch(std::exception & exc)
    {
    std::cout << "ExceptionObject caught !" << std::endl; 
    std::cout << exc.what() << std::endl; 
    return EXIT_FAILURE;
    }


  return EXIT_SUCCESS;
}

