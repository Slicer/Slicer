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

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkPluginFilterWatcher.h"

#include "LabelMapSmoothingCLP.h"

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

// Image Types
  typedef  unsigned int  InputPixelType;
  typedef  float  InternalPixelType;
  typedef  unsigned int  OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< InternalPixelType, 3 >   InternalImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

// Filter Types
  typedef itk::BinaryThresholdImageFilter< InputImageType, InputImageType > InputThresholdType;
  typedef itk::AntiAliasBinaryImageFilter<InputImageType, InternalImageType> AntiAliasType;
  typedef itk::DiscreteGaussianImageFilter<InternalImageType, InternalImageType >  GaussianType;
  typedef itk::BinaryThresholdImageFilter< InternalImageType, OutputImageType > OutputThresholdType;

// I/O Types
  typedef itk::ImageFileReader< InputImageType >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

// Instances
  ReaderType::Pointer reader = ReaderType::New();
  InputThresholdType::Pointer inputThresholder = InputThresholdType::New();
  AntiAliasType::Pointer antiAliasFilter = AntiAliasType::New();
  GaussianType::Pointer gaussianFilter = GaussianType::New();
  OutputThresholdType::Pointer outputThresholder = OutputThresholdType::New();
  WriterType::Pointer writer = WriterType::New();

// Watchers
  itk::PluginFilterWatcher AntiAliasWatcher(antiAliasFilter, "Anti Alias Image Filter", CLPProcessInformation, 2.0/3.0, 0.0);
  itk::PluginFilterWatcher GaussianWatcher(gaussianFilter, "Gaussian Image Filter", CLPProcessInformation, 1.0/3.0, 2.0/3.0);

  reader->SetFileName (inputVolume.c_str());

  // Choose a label to smooth.  All others will be ignored.  If no
  // label is selected by the user, the maximum label in the image is chosen by default.
  if( labelToSmooth == -1 )
    {
    typedef itk::MinimumMaximumImageFilter< InputImageType > MinMaxType;
    MinMaxType::Pointer minMaxCalculator = MinMaxType::New();
    minMaxCalculator->SetInput( reader->GetOutput() );
    minMaxCalculator->Update();
    labelToSmooth = minMaxCalculator->GetMaximum();
    }

  inputThresholder->SetInput( reader->GetOutput() );
  inputThresholder->SetInsideValue( 1 );
  inputThresholder->SetOutsideValue( 0 );
  inputThresholder->SetLowerThreshold( labelToSmooth );
  inputThresholder->SetUpperThreshold( labelToSmooth );

  antiAliasFilter->SetInput( inputThresholder->GetOutput() );  
  antiAliasFilter->SetMaximumRMSError( maxRMSError );
  antiAliasFilter->SetNumberOfIterations( numberOfIterations );
  antiAliasFilter->SetNumberOfLayers( 2 );

  gaussianFilter->SetInput( antiAliasFilter->GetOutput() );
  gaussianFilter->SetVariance( gaussianSigma * gaussianSigma );

  outputThresholder->SetInput( gaussianFilter->GetOutput() );
  outputThresholder->SetLowerThreshold( 0 );
  outputThresholder->SetInsideValue( 1 );
  outputThresholder->SetOutsideValue( 0 );

  writer->SetInput( outputThresholder->GetOutput() );
  writer->SetFileName( outputVolume.c_str() );
  writer->Update();

  return EXIT_SUCCESS;
}

