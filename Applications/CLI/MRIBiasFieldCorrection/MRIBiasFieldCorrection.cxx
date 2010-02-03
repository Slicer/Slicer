/*=========================================================================

  Program:   MRI Bias Field Correction
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Applications/CLI/MRIBiasFieldCorrection/MRIBiasFieldCorrection.cxx $
  Language:  C++
  Date:      $Date: 2010-02-10 13:33:12 -0400 (Tue, 2 Feb 2010) $
  Version:   $Revision: 1 $
  Author:    $Sylvain Jaume (MIT)$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details

==========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <string>
#include <iostream>

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkCastImageFilter.h"
#include "itkShrinkImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkN3MRIBiasFieldCorrectionImageFilter.h"
#include "itkBSplineControlPointImageFilter.h"
#include "itkExpImageFilter.h"

#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"

#include "itkPluginUtilities.h"
#include "MRIBiasFieldCorrectionCLP.h"

#define MRIBiasFieldCorrection_DebugMacro(msg) \
  std::cout << __LINE__ << " MRIBiasFieldCorrection " << msg << std::endl;

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()

namespace
{
  //--------------------------------------------------------------------------
  int BiasFieldCorrection( int argc, char *argv[],
      itk::Image< float, 3 >::Pointer inputImage,
      itk::Image< float, 3 >::Pointer outputImage )
  {
    MRIBiasFieldCorrection_DebugMacro("BiasFieldCorrection start");

    PARSE_ARGS;

    try
    {
      typedef itk::Image< float, 3 >             InputImageType;
      typedef itk::Image< float, 3 >             OutputImageType;

      typedef unsigned char                      MaskPixelType;
      typedef itk::Image< MaskPixelType, 3 >     MaskType;
      typedef itk::ImageFileReader< MaskType >   MaskReaderType;

      MRIBiasFieldCorrection_DebugMacro("AlgorithmType " << AlgorithmType);

      MRIBiasFieldCorrection_DebugMacro("ShrinkFactor " << ShrinkFactor);

      MRIBiasFieldCorrection_DebugMacro("MaximumNumberOfIterations "
        << MaximumNumberOfIterations);

      MRIBiasFieldCorrection_DebugMacro("NumberOfFittingLevels "
        << NumberOfFittingLevels);

      MRIBiasFieldCorrection_DebugMacro("WienerFilterNoise "
        << WienerFilterNoise);

      MRIBiasFieldCorrection_DebugMacro("FullWidthAtHalfMaximum "
        << FullWidthAtHalfMaximum);

      MRIBiasFieldCorrection_DebugMacro("ConvergenceThreshold "
        << ConvergenceThreshold);

      MRIBiasFieldCorrection_DebugMacro("MaskReader");

      MaskReaderType::Pointer maskReader = MaskReaderType::New();
      maskReader->SetFileName( InputMaskFileName.c_str() );
      maskReader->Update();

      MaskType::Pointer inputMask = maskReader->GetOutput();

      MRIBiasFieldCorrection_DebugMacro("ShrinkImageFilter");

      typedef itk::ShrinkImageFilter<InputImageType, InputImageType>
        ShrinkerType;

      ShrinkerType::Pointer shrinker = ShrinkerType::New();

      itk::PluginFilterWatcher watchShrinker(shrinker, "Shrink image",
        CLPProcessInformation);

      shrinker->SetInput( inputImage );
      shrinker->SetShrinkFactors( ShrinkFactor );
      shrinker->Update();
      shrinker->UpdateLargestPossibleRegion();

      MRIBiasFieldCorrection_DebugMacro("BinaryThresholdImageFilter");

      typedef itk::BinaryThresholdImageFilter< InputImageType, MaskType >
        ThresholdImageFilterType;

      ThresholdImageFilterType::Pointer thresholdImageFilter =
        ThresholdImageFilterType::New();

      thresholdImageFilter->SetInput( inputImage );
      thresholdImageFilter->SetLowerThreshold(1);
      thresholdImageFilter->SetOutsideValue(0);
      thresholdImageFilter->SetInsideValue(1);
      thresholdImageFilter->UpdateLargestPossibleRegion();

      MaskType::Pointer maskImage =
        thresholdImageFilter->GetOutput();

      typedef itk::ShrinkImageFilter< MaskType, MaskType > MaskShrinkerType;

      MaskShrinkerType::Pointer maskShrinker = MaskShrinkerType::New();
      maskShrinker->SetInput( maskImage );
      maskShrinker->SetShrinkFactors( ShrinkFactor );

      maskShrinker->Update();
      maskShrinker->UpdateLargestPossibleRegion();

      MRIBiasFieldCorrection_DebugMacro(
          "N3MRIBiasFieldCorrectionImageFilter");

      typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType,
              MaskType, InputImageType> CorrecterType;

      CorrecterType::Pointer correcter = CorrecterType::New();
      correcter->SetInput( shrinker->GetOutput() );
      correcter->SetMaskImage( maskShrinker->GetOutput() );

      correcter->SetMaximumNumberOfIterations( MaximumNumberOfIterations );
      correcter->SetNumberOfFittingLevels( NumberOfFittingLevels );
      correcter->SetWeinerFilterNoise( WienerFilterNoise );
      correcter->SetBiasFieldFullWidthAtHalfMaximum( FullWidthAtHalfMaximum );
      correcter->SetConvergenceThreshold( ConvergenceThreshold );
      correcter->Update();

      MRIBiasFieldCorrection_DebugMacro("BSplineControlPointImageFilter");

      typedef CorrecterType::BiasFieldControlPointLatticeType
        PointType;
      typedef CorrecterType::ScalarImageType ScalarImageType;

      typedef itk::BSplineControlPointImageFilter<PointType, ScalarImageType>
        BSplinerType;

      BSplinerType::Pointer bspliner = BSplinerType::New();
      bspliner->SetInput( correcter->GetBiasFieldControlPointLattice() );
      bspliner->SetSplineOrder( correcter->GetSplineOrder() );
      bspliner->SetSize( inputImage->GetLargestPossibleRegion().GetSize() );
      bspliner->SetOrigin( inputImage->GetOrigin() );
      bspliner->SetDirection( inputImage->GetDirection() );
      bspliner->SetSpacing( inputImage->GetSpacing() );
      bspliner->Update();

      MRIBiasFieldCorrection_DebugMacro("logField");

      InputImageType::Pointer logField = InputImageType::New();
      logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
      logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
      logField->SetRegions( bspliner->GetOutput()->GetLargestPossibleRegion().
          GetSize() );
      logField->SetDirection( bspliner->GetOutput()->GetDirection() );
      logField->Allocate();

      MRIBiasFieldCorrection_DebugMacro("ImageRegionIterator");

      itk::ImageRegionIterator<ScalarImageType> ItB( bspliner->GetOutput(),
          bspliner->GetOutput()->GetLargestPossibleRegion() );

      itk::ImageRegionIterator<InputImageType> ItF( logField,
          logField->GetLargestPossibleRegion() );

      for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
      {
        ItF.Set( ItB.Get()[0] );
      }

      typedef itk::ExpImageFilter< InputImageType, InputImageType >
        ExpFilterType;

      ExpFilterType::Pointer expFilter = ExpFilterType::New();
      expFilter->SetInput( logField );
      expFilter->Update();

      typedef itk::DivideImageFilter< InputImageType, InputImageType,
              InputImageType > DividerType;

      DividerType::Pointer divider = DividerType::New();
      divider->SetInput1( inputImage );
      divider->SetInput2( expFilter->GetOutput() );
      divider->Update();

      MRIBiasFieldCorrection_DebugMacro("ImageRegionIterator");

      itk::ImageRegionIterator<InputImageType> ItIn( divider->GetOutput(),
        divider->GetOutput()->GetLargestPossibleRegion() );

      itk::ImageRegionIterator<OutputImageType> ItOut( outputImage,
        outputImage->GetLargestPossibleRegion() );

      for( ItIn.GoToBegin(), ItOut.GoToBegin(); !ItIn.IsAtEnd(); ++ItIn,
          ++ItOut )
      {
        ItOut.Set( ItIn.Get() );
      }

      MRIBiasFieldCorrection_DebugMacro("Created output image");
    }
    catch( itk::ExceptionObject &excep )
    {
      std::cerr << argv[0] << " : Exception caught!" << std::endl;
      std::cerr << excep << std::endl;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  //--------------------------------------------------------------------------
  template<class Tin, class Tout> int DoIt(int argc, char *argv[], Tin, Tout)
  {
    PARSE_ARGS;

    typedef Tin  InputPixelType;
    typedef Tout OutputPixelType;

    typedef itk::Image< InputPixelType,  3 >   InputImageType;
    typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

    typedef itk::ImageFileReader< InputImageType >  ReaderType;
    typedef itk::ImageFileWriter< OutputImageType > WriterType;

    typedef itk::CastImageFilter< InputImageType, OutputImageType >
      CastFilterType;

    typename ReaderType::Pointer reader = ReaderType::New();

    itk::PluginFilterWatcher watchReader(reader, "Read image",
        CLPProcessInformation);

    reader->SetFileName( InputImageFileName.c_str() );
    reader->Update();

    typename InputImageType::Pointer inputImage = reader->GetOutput();

    typename CastFilterType::Pointer castFilter = CastFilterType::New();

    itk::PluginFilterWatcher watchFilter(castFilter, "Cast image",
        CLPProcessInformation);

    castFilter->SetInput( 0, inputImage );
    castFilter->Update();

    typename OutputImageType::Pointer outputImage = OutputImageType::New();

    outputImage->SetOrigin( inputImage->GetOrigin() );
    outputImage->SetSpacing( inputImage->GetSpacing() );
    outputImage->SetRegions(inputImage->GetLargestPossibleRegion().GetSize());
    outputImage->SetDirection( inputImage->GetDirection() );
    outputImage->Allocate();

    BiasFieldCorrection(argc, argv, castFilter->GetOutput(),
        outputImage );

    typename WriterType::Pointer writer = WriterType::New();

    itk::PluginFilterWatcher watchWriter( writer, "Write Volume",
        CLPProcessInformation );

    writer->SetFileName( OutputImageFileName.c_str() );
    writer->SetInput( outputImage );
    writer->Update();

    return EXIT_SUCCESS;
  }

} // end of anonymous namespace


//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
  {
    itk::GetImageType (InputImageFileName, pixelType, componentType);

    float tmp;

    switch (componentType)
    {
      case itk::ImageIOBase::UCHAR:
      case itk::ImageIOBase::CHAR:

        return DoIt( argc, argv, static_cast<char>(0), tmp);

      case itk::ImageIOBase::USHORT:
      case itk::ImageIOBase::SHORT:

        return DoIt( argc, argv, static_cast<short>(0), tmp);

      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:

        return DoIt( argc, argv, static_cast<int>(0), tmp);

      case itk::ImageIOBase::ULONG:
      case itk::ImageIOBase::LONG:

        return DoIt( argc, argv, static_cast<long>(0), tmp);

      case itk::ImageIOBase::FLOAT:

          return DoIt( argc, argv, static_cast<float>(0), tmp);

      case itk::ImageIOBase::DOUBLE:

          return DoIt( argc, argv, static_cast<double>(0), tmp);

      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:

      default:
          std::cout << "Unknown component type " << componentType
            << std::endl;
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
