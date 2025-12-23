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
# pragma warning(disable : 4786)
#endif

#include "itkImageFileWriter.h"
#include "itkPluginUtilities.h"

#include "itkGrayscaleFillholeImageFilter.h"

#include "GrayscaleFillHoleImageFilterCLP.h"

// STD includes
#include <iostream>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class T>
int DoIt(int argc, char* argv[], T)
{
  PARSE_ARGS;

  //
  //  The following code defines the input and output pixel types and their
  //  associated image types.
  //
  const unsigned int Dimension = 3;

  typedef T InputPixelType;
  typedef T OutputPixelType;

  typedef itk::Image<InputPixelType, Dimension> InputImageType;
  typedef itk::Image<OutputPixelType, Dimension> OutputImageType;

  // readers/writers
  typedef itk::ImageFileReader<InputImageType> ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  // define the fillhole filter
  typedef itk::GrayscaleFillholeImageFilter<InputImageType, OutputImageType> FillholeFilterType;

  // Creation of Reader and Writer filters
  typename ReaderType::Pointer reader = ReaderType::New();
  typename WriterType::Pointer writer = WriterType::New();

  // Create the filter
  typename FillholeFilterType::Pointer fillhole = FillholeFilterType::New();
  itk::PluginFilterWatcher watcher(fillhole, "Fill Hole", CLPProcessInformation);

  // Setup the input and output files
  reader->SetFileName(inputVolume.c_str());
  writer->SetFileName(outputVolume.c_str());

  // Setup the fillhole method
  fillhole->SetInput(reader->GetOutput());

  // Write the output
  writer->SetInput(fillhole->GetOutput());
  writer->Update();

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main(int argc, char* argv[])
{

  PARSE_ARGS;

  itk::IOPixelEnum pixelType;
  itk::IOComponentEnum componentType;

  try
  {
    itk::GetImageType(inputVolume, pixelType, componentType);

    // This filter handles all types
    switch (componentType)
    {
      case itk::IOComponentEnum::UCHAR: return DoIt(argc, argv, static_cast<unsigned char>(0)); break;
      case itk::IOComponentEnum::CHAR: return DoIt(argc, argv, static_cast<char>(0)); break;
      case itk::IOComponentEnum::USHORT: return DoIt(argc, argv, static_cast<unsigned short>(0)); break;
      case itk::IOComponentEnum::SHORT: return DoIt(argc, argv, static_cast<short>(0)); break;
      case itk::IOComponentEnum::UINT: return DoIt(argc, argv, static_cast<unsigned int>(0)); break;
      case itk::IOComponentEnum::INT: return DoIt(argc, argv, static_cast<int>(0)); break;
      case itk::IOComponentEnum::ULONG: return DoIt(argc, argv, static_cast<unsigned long>(0)); break;
      case itk::IOComponentEnum::LONG: return DoIt(argc, argv, static_cast<long>(0)); break;
      case itk::IOComponentEnum::FLOAT: return DoIt(argc, argv, static_cast<float>(0)); break;
      case itk::IOComponentEnum::DOUBLE: return DoIt(argc, argv, static_cast<double>(0)); break;
      case itk::IOComponentEnum::UNKNOWNCOMPONENTTYPE:
      default: std::cout << "unknown component type" << std::endl; break;
    }
  }

  catch (itk::ExceptionObject& excep)
  {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
