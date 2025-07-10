/*=========================================================================

  Program:   Slicer
  Language:  C++
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkImageFileWriter.h"
#include "itkPluginUtilities.h"

#include "itkGrayscaleGrindPeakImageFilter.h"

#include "GrayscaleGrindPeakImageFilterCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class T>
int DoIt( int argc, char* argv[], T )
{
  PARSE_ARGS;

  //
  //  The following code defines the input and output pixel types and their
  //  associated image types.
  //
  const unsigned int Dimension = 3;

  typedef T InputPixelType;
  typedef T OutputPixelType;

  typedef itk::Image<InputPixelType,  Dimension> InputImageType;
  typedef itk::Image<OutputPixelType, Dimension> OutputImageType;

  // readers/writers
  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  // define the grindpeak filter
  typedef itk::GrayscaleGrindPeakImageFilter<
    InputImageType,
    OutputImageType>  GrindPeakFilterType;

  // Creation of Reader and Writer filters
  typename ReaderType::Pointer reader = ReaderType::New();
  typename WriterType::Pointer writer  = WriterType::New();

  // Create the filter
  typename GrindPeakFilterType::Pointer  grindpeak = GrindPeakFilterType::New();
  itk::PluginFilterWatcher watcher(grindpeak, "Grid Peak",
                                   CLPProcessInformation);

  // Setup the input and output files
  reader->SetFileName( inputVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );

  // Setup the grindpeak method
  grindpeak->SetInput(  reader->GetOutput() );

  // Write the output
  writer->SetInput( grindpeak->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;

}

} // end of anonymous namespace

int main( int argc, char* argv[] )
{

  PARSE_ARGS;

  itk::IOPixelEnum     pixelType;
  itk::IOComponentEnum componentType;

  try
  {
    itk::GetImageType(inputVolume, pixelType, componentType);

    // This filter handles all types

    switch( componentType )
    {
      case itk::IOComponentEnum::UCHAR:
      case itk::IOComponentEnum::CHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::IOComponentEnum::USHORT:
      case itk::IOComponentEnum::SHORT:
        return DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::IOComponentEnum::UINT:
      case itk::IOComponentEnum::INT:
        return DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::IOComponentEnum::ULONG:
      case itk::IOComponentEnum::LONG:
        return DoIt( argc, argv, static_cast<long>(0) );
        break;
      case itk::IOComponentEnum::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0) );
        break;
      case itk::IOComponentEnum::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0) );
        break;
      case itk::IOComponentEnum::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
    }
  }
  catch( itk::ExceptionObject& excep )
  {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
