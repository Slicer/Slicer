/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

//  

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPluginFilterWatcher.h"

#include "itkHistogramMatchingImageFilter.h"

#include "HistogramMatchingCLP.h"

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  //
  //  The following code defines the input and output pixel types and their
  //  associated image types.
  //
  const unsigned int Dimension = 3;
  
  typedef short           InputPixelType;
  typedef short           OutputPixelType;
  typedef short           WritePixelType;

  typedef itk::Image< InputPixelType,  Dimension >   InputImageType;
  typedef itk::Image< OutputPixelType, Dimension >   OutputImageType;


  // readers/writers
  typedef itk::ImageFileReader< InputImageType  >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  // define the histogram matching
  typedef itk::HistogramMatchingImageFilter<
                            InputImageType, 
                            OutputImageType >  FilterType;


  // Creation of Reader and Writer filters
  ReaderType::Pointer reader1 = ReaderType::New();
  ReaderType::Pointer reader2 = ReaderType::New();
  WriterType::Pointer writer  = WriterType::New();
  
  // Create the filter
  FilterType::Pointer  filter = FilterType::New();
  itk::PluginFilterWatcher watcher(filter, "Match Histogram",
    CLPProcessInformation);

  // Setup the input and output files
  reader1->SetFileName( inputVolume.c_str() );
  reader2->SetFileName( referenceVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );
  
  // Setup the filter
  filter->SetInput                    ( reader1->GetOutput() );
  filter->SetReferenceImage           ( reader2->GetOutput() );
  filter->SetNumberOfHistogramLevels  ( numberOfHistogramLevels );
  filter->SetNumberOfMatchPoints      ( numberOfHistogramLevels );
  filter->SetThresholdAtMeanIntensity ( thresholdAtMeanIntensity );

  // Write the output
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return 0;

}

