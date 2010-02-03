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

#include "MRIBiasFieldCorrectionCLP.h"

#define MRIBiasFieldCorrection_DebugMacro(msg) \
  std::cout << __LINE__ << " MRIBiasFieldCorrection " << msg << std::endl;

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()

namespace {} // end of anonymous namespace


//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  MRIBiasFieldCorrection_DebugMacro("Start");

  PARSE_ARGS;

  MRIBiasFieldCorrection_DebugMacro("Processed arguments");

  try
  {
    typedef float                                  InputPixelType;
    typedef itk::Image<InputPixelType,3>           InputImageType;
    typedef itk::ImageFileReader<InputImageType>   ReaderType;

    typedef unsigned char                          MaskPixelType;
    typedef itk::Image<MaskPixelType,3>            MaskType;
    typedef itk::ImageFileReader<MaskType>         MaskReaderType;

    typedef float                                  OutputPixelType;
    typedef itk::Image<OutputPixelType, 3>         OutputImageType;
    typedef itk::ImageFileWriter<OutputImageType>  WriterType;

    MRIBiasFieldCorrection_DebugMacro("Read image");

    ReaderType::Pointer ImageReader = ReaderType::New();
    ImageReader->SetFileName(InputImageFileName.c_str());
    ImageReader->Update();
    InputImageType::Pointer inputImage = ImageReader->GetOutput();

    MRIBiasFieldCorrection_DebugMacro("Read mask");

    MaskReaderType::Pointer MaskReader = MaskReaderType::New();
    MaskReader->SetFileName(InputMaskFileName.c_str());
    MaskReader->Update();
    MaskType::Pointer inputMask = MaskReader->GetOutput();

    MRIBiasFieldCorrection_DebugMacro("create output image");

    typedef itk::ShrinkImageFilter<InputImageType, InputImageType>
      ShrinkerType;

    ShrinkerType::Pointer shrinker = ShrinkerType::New();
    shrinker->SetInput( inputImage );
    shrinker->SetShrinkFactors( 1 );

    MRIBiasFieldCorrection_DebugMacro("");

    typedef itk::BinaryThresholdImageFilter<InputImageType, MaskType>
      mFilterType;

    mFilterType::Pointer mfilter = mFilterType::New();
    mfilter->SetInput( inputImage );

    MRIBiasFieldCorrection_DebugMacro("");

    mfilter->SetLowerThreshold(1);
    mfilter->SetOutsideValue(0);
    mfilter->SetInsideValue(1);
    mfilter->UpdateLargestPossibleRegion();

    MRIBiasFieldCorrection_DebugMacro("");

    MaskType::Pointer maskImage = mfilter->GetOutput();

    typedef itk::ShrinkImageFilter<MaskType, MaskType> MaskShrinkerType;

    MaskShrinkerType::Pointer maskShrinker = MaskShrinkerType::New();
    maskShrinker->SetInput( maskImage );
    maskShrinker->SetShrinkFactors( 1 );

    MRIBiasFieldCorrection_DebugMacro("");
    //shrink factor: "<<this->MRIBiasFieldCorrectionNode->GetShrink

    //shrinker->SetShrinkFactors( this->MRIBiasFieldCorrectionNode->GetShrink());
    //maskshrinker->SetShrinkFactors(this->MRIBiasFieldCorrectionNode->GetShrink());

    MRIBiasFieldCorrection_DebugMacro("");

    shrinker->Update();
    shrinker->UpdateLargestPossibleRegion();

    maskShrinker->Update();
    maskShrinker->UpdateLargestPossibleRegion();

    MRIBiasFieldCorrection_DebugMacro("");

    typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType, MaskType,
            InputImageType> CorrecterType;

    CorrecterType::Pointer correcter = CorrecterType::New();
    correcter->SetInput( shrinker->GetOutput() );
    correcter->SetMaskImage( maskShrinker->GetOutput() );

    MRIBiasFieldCorrection_DebugMacro("MaxNumberOfIterations");
    correcter->SetMaximumNumberOfIterations(MaxNumberOfIterations);

    MRIBiasFieldCorrection_DebugMacro("NumberOfFittingLevels");
    correcter->SetNumberOfFittingLevels(NumberOfFittingLevels);

    MRIBiasFieldCorrection_DebugMacro("WienerFilterNoise");
    correcter->SetWeinerFilterNoise(WienerFilterNoise);

    MRIBiasFieldCorrection_DebugMacro("FullWidthAtHalfMaximum");
    correcter->SetBiasFieldFullWidthAtHalfMaximum(FullWidthAtHalfMaximum);

    MRIBiasFieldCorrection_DebugMacro("Convergence Threshold");
    correcter->SetConvergenceThreshold(ConvergenceThreshold);

    MRIBiasFieldCorrection_DebugMacro("");
    correcter->Update();

    MRIBiasFieldCorrection_DebugMacro("");
    typedef itk::BSplineControlPointImageFilter<
    CorrecterType::BiasFieldControlPointLatticeType,
    CorrecterType::ScalarImageType> BSplinerType;

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
    logField->SetRegions(
    bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
    logField->SetDirection( bspliner->GetOutput()->GetDirection() );
    logField->Allocate();

    MRIBiasFieldCorrection_DebugMacro("");
    itk::ImageRegionIterator<CorrecterType::ScalarImageType> ItB(
    bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<InputImageType> ItF( logField,
    logField->GetLargestPossibleRegion() );

    for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
      ItF.Set( ItB.Get()[0] );
    }

    typedef itk::ExpImageFilter<InputImageType, InputImageType> ExpFilterType;

    ExpFilterType::Pointer expFilter = ExpFilterType::New();
    expFilter->SetInput( logField );
    expFilter->Update();

    typedef itk::DivideImageFilter<InputImageType, InputImageType,
            InputImageType> DividerType;
    DividerType::Pointer divider = DividerType::New();
    divider->SetInput1( inputImage );
    divider->SetInput2( expFilter->GetOutput() );
    divider->Update();

    MRIBiasFieldCorrection_DebugMacro("Write " <<
        OutputImageFileName);

    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(OutputImageFileName.c_str());
    writer->SetInput(divider->GetOutput());
    writer->Update();

    MRIBiasFieldCorrection_DebugMacro("Wrote output image");
  }
  catch( itk::ExceptionObject &excep )
  {
    std::cerr << argv[0] << " : Exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
  }
