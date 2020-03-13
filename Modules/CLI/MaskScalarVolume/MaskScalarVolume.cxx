/*=========================================================================

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkImageFileWriter.h"

#include "itkMaskImageFilter.h"
#include "itkThresholdImageFilter.h"

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"

#include "itkPluginUtilities.h"
#include "MaskScalarVolumeCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class T>
int DoIt( int argc, char * argv[] )
{

  PARSE_ARGS;

  typedef    T InputPixelType;
  typedef    T OutputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;

  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  typedef itk::NearestNeighborInterpolateImageFunction<InputImageType> Interpolator;
  typedef itk::ResampleImageFilter<InputImageType, OutputImageType>    ResampleType;
  typedef itk::MaskImageFilter<
    InputImageType, InputImageType, OutputImageType>  FilterType;

  typedef itk::ThresholdImageFilter<
    InputImageType>  ThresholdFilterType;

  typename ReaderType::Pointer reader1 = ReaderType::New();
  itk::PluginFilterWatcher watchReader1(reader1, "Read Input Volume",
                                        CLPProcessInformation);

  typename ReaderType::Pointer reader2 = ReaderType::New();
  itk::PluginFilterWatcher watchReader2(reader2,
                                        "Read Mask Volume",
                                        CLPProcessInformation);
  reader1->SetFileName( InputVolume.c_str() );
  reader2->SetFileName( MaskVolume.c_str() );
  reader2->ReleaseDataFlagOn();

  reader1->Update();
  reader2->Update();

  // have to threshold the mask volume
  typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  itk::PluginFilterWatcher watchThresholdFilter(thresholdFilter,
                                                "Threshold Image",
                                                CLPProcessInformation);

  thresholdFilter->SetInput(0, reader2->GetOutput() );
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->ThresholdOutside(Label, Label);
  thresholdFilter->ReleaseDataFlagOn();

  typename Interpolator::Pointer interp = Interpolator::New();
  interp->SetInputImage(thresholdFilter->GetOutput() );

  typename ResampleType::Pointer resample = ResampleType::New();
  resample->SetInput(thresholdFilter->GetOutput() );
  resample->SetOutputParametersFromImage(reader1->GetOutput() );
  resample->SetInterpolator( interp );
  resample->SetDefaultPixelValue( 0 );
  resample->ReleaseDataFlagOn();

  itk::PluginFilterWatcher watchResample(resample, "Resampling",
                                         CLPProcessInformation);

  typename FilterType::Pointer filter = FilterType::New();
  itk::PluginFilterWatcher watchFilter(filter,
                                       "Masking",
                                       CLPProcessInformation);

  filter->SetInput( 0, reader1->GetOutput() );
  filter->SetInput( 1, resample->GetOutput() );
  filter->SetOutsideValue( Replace );

  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
  writer->SetFileName( OutputVolume.c_str() );
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
    itk::GetImageType(InputVolume, pixelType, componentType);

    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt<unsigned char>( argc, argv);
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt<char>( argc, argv );
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt<unsigned short>( argc, argv );
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt<short>( argc, argv );
        break;
      case itk::ImageIOBase::UINT:
        return DoIt<unsigned int>( argc, argv );
        break;
      case itk::ImageIOBase::INT:
        return DoIt<int>( argc, argv );
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt<unsigned long>( argc, argv );
        break;
      case itk::ImageIOBase::LONG:
        return DoIt<long>( argc, argv );
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt<float>( argc, argv );
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt<double>( argc, argv );
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
