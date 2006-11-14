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

#include "itkGrayscaleFillholeImageFilter.h"

#include "GrayscaleFillHoleImageFilterCLP.h"

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

  // define the fillhole filter
  typedef itk::GrayscaleFillholeImageFilter<
                            InputImageType, 
                            OutputImageType >  FillholeFilterType;


  // Creation of Reader and Writer filters
  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer  = WriterType::New();
  
  // Create the filter
  FillholeFilterType::Pointer  fillhole = FillholeFilterType::New();
  itk::PluginFilterWatcher watcher(fillhole, "Fill Hole",
    CLPProcessInformation);

  // Setup the input and output files
  reader->SetFileName( inputVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );
  
  // Setup the fillhole method
  fillhole->SetInput(  reader->GetOutput() );
  
  // Write the output
  writer->SetInput( fillhole->GetOutput() );
  writer->Update();

  // Output the number of iterations used
  std::cout << "Fillhole took " << fillhole->GetNumberOfIterationsUsed() << " iterations." << std::endl;
  reader->GetOutput()->Print(std::cout);
  fillhole->GetOutput()->Print(std::cout);
  return 0;

}

