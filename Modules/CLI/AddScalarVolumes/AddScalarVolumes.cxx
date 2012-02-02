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
#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkConstrainedValueAdditionImageFilter.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkPluginUtilities.h"
#include "AddScalarVolumesCLP.h"

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

  typedef    T InputPixelType;
  typedef    T OutputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;

  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  typedef itk::BSplineInterpolateImageFunction<InputImageType>                                       Interpolator;
  typedef itk::ResampleImageFilter<InputImageType, OutputImageType>                                  ResampleType;
  typedef itk::ConstrainedValueAdditionImageFilter<InputImageType, OutputImageType, OutputImageType> FilterType;

  typename ReaderType::Pointer reader1 = ReaderType::New();
  itk::PluginFilterWatcher watchReader1(reader1, "Read Volume 1",
                                        CLPProcessInformation);

  typename ReaderType::Pointer reader2 = ReaderType::New();
  itk::PluginFilterWatcher watchReader2(reader2,
                                        "Read Volume 2",
                                        CLPProcessInformation);
  reader1->SetFileName( inputVolume1.c_str() );
  reader2->SetFileName( inputVolume2.c_str() );
  reader2->ReleaseDataFlagOn();

  reader1->Update();
  reader2->Update();

  typename Interpolator::Pointer interp = Interpolator::New();
  interp->SetInputImage(reader2->GetOutput() );
  interp->SetSplineOrder(order);

  typename ResampleType::Pointer resample = ResampleType::New();
  resample->SetInput(reader2->GetOutput() );
  resample->SetOutputParametersFromImage(reader1->GetOutput() );
  resample->SetInterpolator( interp );
  resample->SetDefaultPixelValue( 0 );
  resample->ReleaseDataFlagOn();

  itk::PluginFilterWatcher watchResample(resample, "Resampling",
                                         CLPProcessInformation);

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1( reader1->GetOutput() );
  filter->SetInput2( resample->GetOutput() );

  itk::PluginFilterWatcher watchFilter(filter, "Adding",
                                       CLPProcessInformation);

  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
  writer->SetFileName( outputVolume.c_str() );
  writer->SetInput( filter->GetOutput() );
  writer->SetUseCompression(1);
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
    itk::GetImageType(inputVolume1, pixelType, componentType);

    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt( argc, argv, static_cast<char>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0) );
        break;
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0) );
        break;
      case itk::ImageIOBase::LONG:
        return DoIt( argc, argv, static_cast<long>(0) );
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0) );
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0) );
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }

    // This filter handles all types on input, but only produces
    // signed types

    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
