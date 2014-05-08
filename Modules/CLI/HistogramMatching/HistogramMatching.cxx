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

#include "itkImageFileWriter.h"
#include "itkPluginUtilities.h"

#include "itkHistogramMatchingImageFilter.h"

#include "HistogramMatchingCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class T>
int DoIt( int argc, char * argv[], T )
{
  PARSE_ARGS;

  const unsigned int Dimension = 3;

  typedef T InputPixelType;
  typedef T OutputPixelType;

  typedef itk::Image<InputPixelType,  Dimension> InputImageType;
  typedef itk::Image<OutputPixelType, Dimension> OutputImageType;

  // readers/writers
  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  // define the histogram matching
  typedef itk::HistogramMatchingImageFilter<
    InputImageType,
    OutputImageType, InputPixelType>  FilterType;

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
  writer->SetUseCompression(1);

  // Setup the filter
  filter->SetInput( reader1->GetOutput() );
  filter->SetReferenceImage( reader2->GetOutput() );
  filter->SetNumberOfHistogramLevels( numberOfHistogramLevels );
  filter->SetNumberOfMatchPoints( numberOfMatchPoints );
  filter->SetThresholdAtMeanIntensity( thresholdAtMeanIntensity );

  // Write the output
  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;

}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(inputVolume, pixelType, componentType);

    // This filter handles all types
    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt<unsigned char>( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt<char>( argc, argv, static_cast<char>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt<unsigned short>( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt<short>( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        return DoIt<unsigned int>( argc, argv, static_cast<unsigned int>(0) );
        break;
      case itk::ImageIOBase::INT:
        return DoIt<int>( argc, argv, static_cast<int>(0) );
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt<unsigned long>( argc, argv, static_cast<unsigned long>(0) );
        break;
/* A bug in ITK prevents this from working with ITK Review Statistics turned on. */
#if defined USE_REVIEW_STATISTICS
      case itk::ImageIOBase::LONG:
        return DoIt<long>( argc, argv, static_cast<long>(0) );
        break;
#endif
      case itk::ImageIOBase::FLOAT:
        return DoIt<float>( argc, argv, static_cast<float>(0) );
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt<double>( argc, argv, static_cast<double>(0) );
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }

  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
