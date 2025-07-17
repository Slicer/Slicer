/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or https://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include "itkCurvatureAnisotropicDiffusionImageFilter.h"

#include "itkPluginUtilities.h"
#include "CurvatureAnisotropicDiffusionCLP.h"

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

  typedef float InputPixelType;
  typedef T OutputPixelType;

  typedef itk::Image<InputPixelType, 3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;

  typedef itk::ImageFileReader<InputImageType> ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  typedef itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InputImageType> FilterType;
  typedef itk::CastImageFilter<InputImageType, OutputImageType> CastType;

  const typename ReaderType::Pointer reader = ReaderType::New();
  const itk::PluginFilterWatcher watchReader(reader, "Read Volume", CLPProcessInformation);

  reader->SetFileName(inputVolume.c_str());

  const typename FilterType::Pointer filter = FilterType::New();
  const itk::PluginFilterWatcher watchFilter(filter, "Curvature Anisotropic Diffusion", CLPProcessInformation);

  filter->SetInput(reader->GetOutput());
  filter->UseImageSpacingOn();
  filter->SetNumberOfIterations(numberOfIterations);
  filter->SetTimeStep(timeStep);
  filter->SetConductanceParameter(conductance);

  const typename CastType::Pointer cast = CastType::New();
  cast->SetInput(filter->GetOutput());

  const typename WriterType::Pointer writer = WriterType::New();
  const itk::PluginFilterWatcher watchWriter(writer, "Write Volume", CLPProcessInformation);
  writer->SetFileName(outputVolume.c_str());
  writer->SetInput(cast->GetOutput());
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
