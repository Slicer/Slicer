/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: Resample.cxx,v $
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

// Resample a series
//   Usage: ResampleScalarVolume --spacing xSpacing,ySpacing,zSpacing
//                               InputDirectory OutputDirectory
//
//
//   Example: ResampleScalarVolume --spacing 0,0,1.5 CT.mhd CTResample.mhd
//            will read a volume CT.mhd and create a
//            new volume called CTResample.mhd directory. The new series
//            will have the same x,y spacing as the input series, but
//            will have a z-spacing of 1.5.
//
// Description:
// ResampleScalarVolume resamples a volume with user-specified
// spacing. The number of slices in
// the output series may be larger or smaller due to changes in the
// z-spacing. To retain the spacing for a given dimension, specify 0.
//
// The program progresses as follows:
// 1) Read the input series
// 2) Resample the series according to the user specified x-y-z
//    spacing.
// 3) Write the new series
//

#include "itkPluginUtilities.h"

#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkBSplineInterpolateImageFunction.h"

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"

#include "ResampleScalarVolumeCLP.h"

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
  const unsigned int InputDimension = 3;
  const unsigned int OutputDimension = 3;

  typedef T PixelType;

  typedef itk::Image<PixelType, InputDimension>
  InputImageType;
  typedef itk::Image<PixelType, OutputDimension>
  OutputImageType;
  typedef itk::ImageFileReader<InputImageType>
  ReaderType;
  typedef itk::IdentityTransform<double, InputDimension>
  TransformType;
  typedef itk::LinearInterpolateImageFunction<InputImageType, double>
  LinearInterpolatorType;
  typedef itk::NearestNeighborInterpolateImageFunction<InputImageType, double>
  NearestNeighborInterpolatorType;
  typedef itk::BSplineInterpolateImageFunction<InputImageType, double>
  BSplineInterpolatorType;
#define RADIUS 3

  typedef itk::WindowedSincInterpolateImageFunction<InputImageType, RADIUS,
                                                    itk::Function::HammingWindowFunction<RADIUS> >
   HammingInterpolatorType;
  typedef itk::WindowedSincInterpolateImageFunction<InputImageType, RADIUS,
                                                    itk::Function::CosineWindowFunction<RADIUS> >
   CosineInterpolatorType;
  typedef itk::WindowedSincInterpolateImageFunction<InputImageType, RADIUS,
                                                    itk::Function::WelchWindowFunction<RADIUS> >
   WelchInterpolatorType;
  typedef itk::WindowedSincInterpolateImageFunction<InputImageType, RADIUS,
                                                    itk::Function::LanczosWindowFunction<RADIUS> >
   LanczosInterpolatorType;
  typedef itk::WindowedSincInterpolateImageFunction<InputImageType, RADIUS,
                                                    itk::Function::BlackmanWindowFunction<RADIUS> >
   BlackmanInterpolatorType;

  typedef itk::ResampleImageFilter<InputImageType, InputImageType>
  ResampleFilterType;
  typedef itk::ImageFileWriter<OutputImageType>
  FileWriterType;

// //////////////////////////////////////////////
// 1) Read the input series

  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( InputVolume.c_str() );

  try
    {
    reader->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << "Exception thrown while reading the input file" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }

// //////////////////////////////////////////////
// 2) Resample the series
  typename LinearInterpolatorType::Pointer linearInterpolator = LinearInterpolatorType::New();
  typename NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
  typename BSplineInterpolatorType::Pointer bsplineInterpolator = BSplineInterpolatorType::New();
  typename HammingInterpolatorType::Pointer hammingInterpolator = HammingInterpolatorType::New();
  typename CosineInterpolatorType::Pointer cosineInterpolator = CosineInterpolatorType::New();
  typename WelchInterpolatorType::Pointer welchInterpolator = WelchInterpolatorType::New();
  typename LanczosInterpolatorType::Pointer lanczosInterpolator = LanczosInterpolatorType::New();
  typename BlackmanInterpolatorType::Pointer blackmanInterpolator = BlackmanInterpolatorType::New();

  typename TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();

  const typename InputImageType::SpacingType& inputSpacing =
    reader->GetOutput()->GetSpacing();
  const typename InputImageType::RegionType& inputRegion =
    reader->GetOutput()->GetLargestPossibleRegion();
  const typename InputImageType::SizeType& inputSize =
    inputRegion.GetSize();

  // Compute the size of the output. The user specifies a spacing on
  // the command line. If the spacing is 0, the input spacing will be
  // used. The size (#of pixels) in the output is recomputed using
  // the ratio of the input and output sizes.
  typename InputImageType::SpacingType outputSpacing;
  outputSpacing[0] = outputPixelSpacing[0];
  outputSpacing[1] = outputPixelSpacing[1];
  outputSpacing[2] = outputPixelSpacing[2];
  for( unsigned int i = 0; i < 3; i++ )
    {
    if( outputSpacing[i] == 0.0 )
      {
      outputSpacing[i] = inputSpacing[i];
      }
    }
  typename InputImageType::SizeType   outputSize;
  typedef typename InputImageType::SizeType::SizeValueType SizeValueType;
  outputSize[0] = static_cast<SizeValueType>(inputSize[0] * inputSpacing[0] / outputSpacing[0] + .5);
  outputSize[1] = static_cast<SizeValueType>(inputSize[1] * inputSpacing[1] / outputSpacing[1] + .5);
  outputSize[2] = static_cast<SizeValueType>(inputSize[2] * inputSpacing[2] / outputSpacing[2] + .5);

  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  itk::PluginFilterWatcher watcher(resampler, "Resample Volume",
                                   CLPProcessInformation);

  resampler->SetInput( reader->GetOutput() );
  resampler->SetTransform( transform );
  if( interpolationType == "linear" )
    {
    resampler->SetInterpolator( linearInterpolator );
    }
  else if( interpolationType == "nearestNeighbor" )
    {
    resampler->SetInterpolator( nearestNeighborInterpolator );
    }
  else if( interpolationType == "bspline" )
    {
    resampler->SetInterpolator( bsplineInterpolator );
    }
  else if( interpolationType == "hamming" )
    {
    resampler->SetInterpolator( hammingInterpolator );
    }
  else if( interpolationType == "cosine" )
    {
    resampler->SetInterpolator( cosineInterpolator );
    }
  else if( interpolationType == "welch" )
    {
    resampler->SetInterpolator( welchInterpolator );
    }
  else if( interpolationType == "lanczos" )
    {
    resampler->SetInterpolator( lanczosInterpolator );
    }
  else if( interpolationType == "blackman" )
    {
    resampler->SetInterpolator( blackmanInterpolator );
    }
  else
    {
    resampler->SetInterpolator( linearInterpolator );
    }

  resampler->SetOutputOrigin( reader->GetOutput()->GetOrigin() );
  resampler->SetOutputSpacing( outputSpacing );
  resampler->SetOutputDirection( reader->GetOutput()->GetDirection() );
  resampler->SetSize( outputSize );
  resampler->Update();

// //////////////////////////////////////////////
// 5) Write the new DICOM series

  typename FileWriterType::Pointer seriesWriter = FileWriterType::New();
  seriesWriter->SetInput( resampler->GetOutput() );
  seriesWriter->SetFileName( OutputVolume.c_str() );
  seriesWriter->SetUseCompression(1);
  try
    {
    seriesWriter->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << "Exception thrown while writing the series " << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }
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

    // This filter handles all types

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
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
