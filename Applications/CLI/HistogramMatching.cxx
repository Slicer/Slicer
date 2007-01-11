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
#include "itkPluginUtilities.h"

#include "itkHistogramMatchingImageFilter.h"

#include "HistogramMatchingCLP.h"

template<class T> int DoIt( int argc, char * argv[], T )
{
  PARSE_ARGS;

  const unsigned int Dimension = 3;
  
  typedef T InputPixelType;
  typedef T OutputPixelType;
  typedef T WritePixelType;

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
  typename ReaderType::Pointer reader1 = ReaderType::New();
  typename ReaderType::Pointer reader2 = ReaderType::New();
  typename WriterType::Pointer writer  = WriterType::New();
  
  // Create the filter
  typename FilterType::Pointer  filter = FilterType::New();
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
  filter->SetNumberOfMatchPoints      ( numberOfMatchPoints );
  filter->SetThresholdAtMeanIntensity ( thresholdAtMeanIntensity );

  // Write the output
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;

}

int main( int argc, char * argv[] )
{
  
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType (inputVolume, pixelType, componentType);

    // This filter handles all types
    
    switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
      case itk::ImageIOBase::CHAR:
      case itk::ImageIOBase::USHORT:
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0));
        break;
      case itk::ImageIOBase::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0));
        break;
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0));
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0));
        break;
      case itk::ImageIOBase::LONG:
        return DoIt( argc, argv, static_cast<long>(0));
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0));
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0));
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
