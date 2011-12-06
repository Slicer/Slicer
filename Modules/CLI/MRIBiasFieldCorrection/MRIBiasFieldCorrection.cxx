/*=========================================================================

  Program:   MRI Bias Field Correction
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/CLI/MRIBiasFieldCorrection/MRIBiasFieldCorrection.cxx $
  Language:  C++
  Date:      $Date: 2010-02-10 13:33:12 -0400 (Tue, 2 Feb 2010) $
  Version:   $Revision: 1 $
  Author:    $Sylvain Jaume (MIT)$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details

==========================================================================*/
#include "itkImageFileWriter.h"
#include "itkShrinkImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkN3MRIBiasFieldCorrectionImageFilter.h"
#include "itkN4MRIBiasFieldCorrectionImageFilter.h"

#include "itkImageToVTKImageFilter.h"

#include "vtkImageClip.h"
#include "vtkImageResample.h"
#include "vtkImageAccumulate.h"
#include "vtkImageGaussianSmooth.h"

#include "itkPluginUtilities.h"
#include "MRIBiasFieldCorrectionCLP.h"

#define MRIBiasFieldCorrection_DebugMacro(msg) std::cout << __LINE__ \
                                                         << " MRIBiasFieldCorrection " << msg << std::endl;

#define MRIBiasFieldCorrection_ErrorMacro(msg) std::cout << __LINE__ \
                                                         << " MRIBiasFieldCorrection ERROR: " << msg << std::endl;

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()

namespace
{
// --------------------------------------------------------------------------
int L1BiasFieldCorrection( int argc, char *argv[],
                           itk::Image<float, 3>::Pointer inputImage,
                           itk::Image<float, 3>::Pointer outputImage )
{
  MRIBiasFieldCorrection_DebugMacro("L1BiasFieldCorrection start");

  PARSE_ARGS;

  try
    {
    typedef itk::Image<float, 3> InputImageType;
    typedef itk::Image<float, 3> OutputImageType;

    MRIBiasFieldCorrection_DebugMacro("AlgorithmType " << AlgorithmType);

    const InputImageType::PointType inputOrigin = inputImage->GetOrigin();

    const InputImageType::SpacingType inputSpacing =
      inputImage->GetSpacing();

    const InputImageType::DirectionType inputDirection =
      inputImage->GetDirection();

    MRIBiasFieldCorrection_DebugMacro("inputOrigin " << inputOrigin);
    MRIBiasFieldCorrection_DebugMacro("inputSpacing " << inputSpacing);
    MRIBiasFieldCorrection_DebugMacro("inputDirection\n" << inputDirection);

    typedef itk::ImageToVTKImageFilter<InputImageType> ConvertITKtoVTKType;

    ConvertITKtoVTKType::Pointer convertITKtoVTK =
      ConvertITKtoVTKType::New();

    convertITKtoVTK->SetInput( inputImage );
    convertITKtoVTK->Update();

    int dim[3], outDim[3] = {16, 16, 16};
    int wholeExtent[6], clipExtent[6];

    double origin[3], spacing[3];
    double bounds[6], range[2];

    convertITKtoVTK->GetOutput()->GetDimensions(dim);
    convertITKtoVTK->GetOutput()->GetWholeExtent(wholeExtent);

    convertITKtoVTK->GetOutput()->GetOrigin(origin);
    convertITKtoVTK->GetOutput()->GetSpacing(spacing);

    convertITKtoVTK->GetOutput()->GetBounds(bounds);
    convertITKtoVTK->GetOutput()->GetScalarRange(range);

    MRIBiasFieldCorrection_DebugMacro("inputImage dim "
                                      << dim[0] << " " << dim[1] << " " << dim[2]
                                      << " wholeExtent "
                                      << wholeExtent[0] << " " << wholeExtent[1] << " "
                                      << wholeExtent[2] << " " << wholeExtent[3] << " "
                                      << wholeExtent[4] << " " << wholeExtent[5]
                                      << " origin "
                                      << origin[0] << " " << origin[1] << " " << origin[2] << " "
                                      << " spacing "
                                      << spacing[0] << " " << spacing[1] << " " << spacing[2] << " "
                                      << " bounds "
                                      << bounds[0] << " " << bounds[1] << " " << bounds[2] << " "
                                      << bounds[3] << " " << bounds[4] << " " << bounds[5]
                                      << " range " << range[0] << " " << range[1]);

    if( dim[0] <= 0 || dim[1] <= 0 || dim[2] <= 0 )
      {
      MRIBiasFieldCorrection_ErrorMacro("inputImage dim "
                                        << dim[0] << " " << dim[1] << " " << dim[2]);
      return 0;
      }

    if( wholeExtent[0] >= wholeExtent[1] ||
        wholeExtent[2] >= wholeExtent[3] ||
        wholeExtent[4] >= wholeExtent[5] )
      {
      MRIBiasFieldCorrection_ErrorMacro("inputImage wholeExtent "
                                        << wholeExtent[0] << " " << wholeExtent[1] << " "
                                        << wholeExtent[2] << " " << wholeExtent[3] << " "
                                        << wholeExtent[4] << " " << wholeExtent[5]);
      return 0;
      }

    if( spacing[0] <= 0.0 || spacing[1] <= 0.0 || spacing[2] <= 0.0 )
      {
      MRIBiasFieldCorrection_ErrorMacro("inputImage spacing "
                                        << spacing[0] << " " << spacing[1] << " " << spacing[2]);
      return 0;
      }

    if( bounds[0] >= bounds[1] ||
        bounds[2] >= bounds[3] ||
        bounds[4] >= bounds[5] )
      {
      MRIBiasFieldCorrection_ErrorMacro("inputImage bounds "
                                        << bounds[0] << " " << bounds[1] << " " << bounds[2] << " "
                                        << bounds[3] << " " << bounds[4] << " " << bounds[5]);
      return 0;
      }

    // itk::PluginFilterWatcher watchShrinker(shrinker, "Shrink image",
    // CLPProcessInformation);

    if( range[1] - range[0] <= 1.0 )
      {
      MRIBiasFieldCorrection_ErrorMacro("range " << range[0] << " "
                                                 << range[1]);
      return EXIT_FAILURE;
      }

    double resampleFactorX = 128.0 / static_cast<double>(dim[0]);
    double resampleFactorY = 128.0 / static_cast<double>(dim[1]);
    double resampleFactorZ = 128.0 / static_cast<double>(dim[2]);

    double lengthX = bounds[1] - bounds[0];
    double lengthY = bounds[3] - bounds[2];
    double lengthZ = bounds[5] - bounds[4];

    double offsetX = 0.02 * lengthX;
    double offsetY = 0.02 * lengthY;
    double offsetZ = 0.02 * lengthZ;

    bounds[0] += offsetX;
    bounds[2] += offsetY;
    bounds[4] += offsetZ;

    bounds[1] -= offsetX;
    bounds[3] -= offsetY;
    bounds[5] -= offsetZ;

    double outLengthX = bounds[1] - bounds[0];
    double outLengthY = bounds[3] - bounds[2];
    double outLengthZ = bounds[5] - bounds[4];

    double steps[3] =
      {
      outLengthX / outDim[0],
      outLengthY / outDim[1],
      outLengthZ / outDim[2]
      };

    // kernel radius
    double radius = 5.0;
    double x, y, z;
    double mean[3], stdDev[3];

    double startX = bounds[0] + 0.5 * steps[0] - origin[0];
    double startY = bounds[2] + 0.5 * steps[1] - origin[1];
    double startZ = bounds[4] + 0.5 * steps[2] - origin[2];

    double factorX = static_cast<double>(dim[0]);
    double factorY = static_cast<double>(dim[1]);
    double factorZ = static_cast<double>(dim[2]);

    factorX /= static_cast<double>(outDim[0]);
    factorY /= static_cast<double>(outDim[1]);
    factorZ /= static_cast<double>(outDim[2]);

    MRIBiasFieldCorrection_DebugMacro(
      "dim " << dim[0] << " " << dim[1] << " " << dim[2] << " "
             << "outDim " << outDim[0] << " " << outDim[1] << " " << outDim[2]);

    double outSpacing[3] =
      {
      spacing[0] * factorX,
      spacing[1] * factorY,
      spacing[2] * factorZ
      };

    double outOrigin[3] =
      {
      origin[0] + 0.5 * outSpacing[0],
      origin[1] + 0.5 * outSpacing[1],
      origin[2] + 0.5 * outSpacing[2]
      };

    MRIBiasFieldCorrection_DebugMacro(
      "factor " << factorX << " " << factorY << " " << factorZ
                << " outSpacing "
                << outSpacing[0] << " " << outSpacing[1] << " " << outSpacing[2]);

    vtkImageData *meanImage = vtkImageData::New();
    meanImage->SetDimensions(outDim);
    meanImage->SetOrigin(outOrigin);
    meanImage->SetSpacing(outSpacing);
    meanImage->SetScalarTypeToDouble();
    meanImage->AllocateScalars();
    meanImage->Update();

    vtkImageData *stdDevImage = vtkImageData::New();
    stdDevImage->SetDimensions(outDim);
    stdDevImage->SetOrigin(outOrigin);
    stdDevImage->SetSpacing(outSpacing);
    stdDevImage->SetScalarTypeToDouble();
    stdDevImage->AllocateScalars();
    stdDevImage->Update();

    double *ptrMean = static_cast<double *>(meanImage->GetScalarPointer() );
    double *ptrStdDev = static_cast<double *>
      (stdDevImage->GetScalarPointer() );

    double progress;
    double progressFactor = 100.0
      / static_cast<double>( wholeExtent[5] - wholeExtent[4] );

    progressFactor = progressFactor;
    for( int k = 0; k < outDim[2]; k++ )
      {
      z = startZ + k * steps[2];

      clipExtent[4] = static_cast<int>( (z - radius) / spacing[2] + 0.5);
      clipExtent[5] = static_cast<int>( (z + radius) / spacing[2] + 0.5);

      if( clipExtent[4] < wholeExtent[4] )
        {
        clipExtent[4] = wholeExtent[4];
        }
      if( clipExtent[5] < wholeExtent[4] )
        {
        clipExtent[5] = wholeExtent[4];
        }

      if( clipExtent[4] > wholeExtent[5] )
        {
        clipExtent[4] = wholeExtent[5];
        }
      if( clipExtent[5] > wholeExtent[5] )
        {
        clipExtent[5] = wholeExtent[5];
        }

      progress = clipExtent[5] - wholeExtent[4];

      MRIBiasFieldCorrection_DebugMacro("clipExtent[4 5] " << clipExtent[4]
                                                           << " " << clipExtent[5] << " progress "
                                                           << static_cast<int>(progress * progressFactor) << "%");

      if( clipExtent[4] >= clipExtent[5] )
        {
        MRIBiasFieldCorrection_ErrorMacro("clipExtent[4] "
                                          << clipExtent[4] << " clipExtent[5] " << clipExtent[5]);
        meanImage->Delete();
        stdDevImage->Delete();
        return 0;
        }
      for( int j = 0; j < outDim[1]; j++ )
        {
        y = startY + j * steps[1];

        clipExtent[2] = static_cast<int>( (y - radius) / spacing[1] + 0.5);
        clipExtent[3] = static_cast<int>( (y + radius) / spacing[1] + 0.5);

        if( clipExtent[2] < wholeExtent[2] )
          {
          clipExtent[2] = wholeExtent[2];
          }
        if( clipExtent[3] < wholeExtent[2] )
          {
          clipExtent[3] = wholeExtent[2];
          }

        if( clipExtent[2] > wholeExtent[3] )
          {
          clipExtent[2] = wholeExtent[3];
          }
        if( clipExtent[3] > wholeExtent[3] )
          {
          clipExtent[3] = wholeExtent[3];
          }

        if( clipExtent[2] >= clipExtent[3] )
          {
          MRIBiasFieldCorrection_ErrorMacro("clipExtent[2] "
                                            << clipExtent[2] << " clipExtent[3] " << clipExtent[3]);
          meanImage->Delete();
          stdDevImage->Delete();
          return 0;
          }
        for( int i = 0; i < outDim[0]; i++ )
          {
          x = startX + i * steps[0];

          clipExtent[0] = static_cast<int>( (x - radius) / spacing[0] + 0.5);
          clipExtent[1] = static_cast<int>( (x + radius) / spacing[0] + 0.5);

          if( clipExtent[0] < wholeExtent[0] )
            {
            clipExtent[0] = wholeExtent[0];
            }
          if( clipExtent[1] < wholeExtent[0] )
            {
            clipExtent[1] = wholeExtent[0];
            }

          if( clipExtent[0] > wholeExtent[1] )
            {
            clipExtent[0] = wholeExtent[1];
            }
          if( clipExtent[1] > wholeExtent[1] )
            {
            clipExtent[1] = wholeExtent[1];
            }

          if( 0 )
            {
            MRIBiasFieldCorrection_DebugMacro("clipExtent "
                                              << clipExtent[0] << " " << clipExtent[1] << " "
                                              << clipExtent[2] << " " << clipExtent[3] << " "
                                              << clipExtent[4] << " " << clipExtent[5]
                                              << " numPts "
                                              << (clipExtent[1] - clipExtent[0] + 1)
                                              * (clipExtent[3] - clipExtent[2] + 1)
                                              * (clipExtent[5] - clipExtent[4] + 1) );
            }

          if( clipExtent[0] >= clipExtent[1] )
            {
            MRIBiasFieldCorrection_ErrorMacro("clipExtent[0] "
                                              << clipExtent[0] << " clipExtent[1] " << clipExtent[1]);
            meanImage->Delete();
            stdDevImage->Delete();
            return 0;
            }

          vtkImageClip *imageClip = vtkImageClip::New();
          imageClip->SetInput(convertITKtoVTK->GetOutput() );
          imageClip->SetOutputWholeExtent(clipExtent);
          imageClip->ClipDataOn();
          imageClip->Update();

          imageClip->GetOutput()->GetScalarRange(range);

          if( 0 )
            {
            MRIBiasFieldCorrection_DebugMacro("imageClip range "
                                              << range[0] << " " << range[1]);
            }

          if( range[1] - range[0] <= 1.0 )
            {
            MRIBiasFieldCorrection_ErrorMacro("range " << range[0] << " "
                                                       << range[1]);
            imageClip->Delete();
            return 0;
            }

          double scale = 256.0 / (range[1] - range[0]);

          vtkImageAccumulate *imageAccumulate = vtkImageAccumulate::New();
          imageAccumulate->SetInput(imageClip->GetOutput() );
          imageClip->Delete();
          imageAccumulate->SetComponentExtent(0, 255, 0, 0, 0, 0);
          imageAccumulate->SetComponentOrigin(range[0], 0.0, 0.0);
          imageAccumulate->SetComponentSpacing(scale, 1.0, 1.0);
          imageAccumulate->IgnoreZeroOn();
          imageAccumulate->Update();

          imageAccumulate->GetMean(mean);
          imageAccumulate->GetStandardDeviation(stdDev);
          imageAccumulate->Delete();

          if( 0 )
            {
            MRIBiasFieldCorrection_DebugMacro("mean " << mean[0]
                                                      << " stdDev " << stdDev[0]);
            }

          *ptrMean = mean[0];
          *ptrStdDev = stdDev[0];

          ptrMean++;
          ptrStdDev++;
          }
        }
      }

    int intFactor = static_cast<int>(factorZ + 0.5);

    while( intFactor /= 2 )
      {
      outSpacing[0] *= 0.5;
      outSpacing[1] *= 0.5;
      outSpacing[2] *= 0.5;

      outOrigin[0] = origin[0] + 0.5 * outSpacing[0];
      outOrigin[1] = origin[1] + 0.5 * outSpacing[1];
      outOrigin[2] = origin[2] + 0.5 * outSpacing[2];

      outDim[0] *= 2;
      outDim[1] *= 2;
      outDim[2] *= 2;

      MRIBiasFieldCorrection_DebugMacro("intFactor " << intFactor
                                                     << " outDim "
                                                     << outDim[0] << " " << outDim[1] << " " << outDim[2]
                                                     << " outOrigin "
                                                     << outOrigin[0] << " " << outOrigin[1] << " " << outOrigin[2]
                                                     << " outSpacing "
                                                     << outSpacing[0] << " " << outSpacing[1] << " " << outSpacing[2]);

      vtkImageReslice *meanReslice = vtkImageReslice::New();
      meanReslice->SetInput(meanImage);
      meanImage->Delete();
      meanReslice->SetOutputExtent(0, outDim[0] - 1, 0, outDim[1] - 1, 0,
                                   outDim[2] - 1);
      meanReslice->SetOutputSpacing(outSpacing);
      meanReslice->SetOutputOrigin(outOrigin);
      meanReslice->Update();

      vtkImageGaussianSmooth *meanGaussian = vtkImageGaussianSmooth::New();
      meanGaussian->SetInput(meanReslice->GetOutput() );
      meanReslice->Delete();
      meanGaussian->Update();

      meanImage = meanGaussian->GetOutput();
      meanImage->Register(NULL);
      meanGaussian->Delete();

      vtkImageReslice *stdDevReslice = vtkImageReslice::New();
      stdDevReslice->SetInput(stdDevImage);
      stdDevImage->Delete();
      stdDevReslice->SetOutputExtent(0, outDim[0] - 1, 0, outDim[1] - 1, 0,
                                     outDim[2] - 1);
      stdDevReslice->SetOutputSpacing(outSpacing);
      stdDevReslice->SetOutputOrigin(outOrigin);
      stdDevReslice->Update();

      vtkImageGaussianSmooth *stdDevGaussian =
        vtkImageGaussianSmooth::New();
      stdDevGaussian->SetInput(stdDevReslice->GetOutput() );
      stdDevReslice->Delete();
      stdDevGaussian->Update();

      stdDevImage = stdDevGaussian->GetOutput();
      stdDevImage->Register(NULL);
      stdDevGaussian->Delete();
      }

    vtkImageReslice *meanReslice = vtkImageReslice::New();
    meanReslice->SetInput(meanImage);
    meanImage->Delete();
    meanReslice->SetOutputExtent(wholeExtent);
    meanReslice->SetOutputSpacing(spacing);
    meanReslice->SetOutputOrigin(origin);
    meanReslice->Update();

    int meanDim[3];
    meanReslice->GetOutput()->GetDimensions(meanDim);

    MRIBiasFieldCorrection_DebugMacro("meanDim "
                                      << meanDim[0] << " " << meanDim[1] << " " << meanDim[2]);

    if( meanDim[0] != dim[0] ||
        meanDim[1] != dim[1] ||
        meanDim[2] != dim[2] )
      {
      MRIBiasFieldCorrection_ErrorMacro("meanDim "
                                        << meanDim[0] << " " << meanDim[1] << " " << meanDim[2]
                                        << " != dim " << dim[0] << " " << dim[1] << " " << dim[2]);
      return 0;
      }

    vtkImageResample *imageResample = vtkImageResample::New();
    imageResample = vtkImageResample::New();
    imageResample->SetInput(meanReslice->GetOutput() );
    imageResample->SetAxisMagnificationFactor(0, resampleFactorX);
    imageResample->SetAxisMagnificationFactor(1, resampleFactorY);
    imageResample->SetAxisMagnificationFactor(2, resampleFactorZ);
    imageResample->Update();

    vtkImageReslice *stdDevReslice = vtkImageReslice::New();
    stdDevReslice->SetInput(stdDevImage);
    stdDevImage->Delete();
    stdDevReslice->SetOutputExtent(wholeExtent);
    stdDevReslice->SetOutputSpacing(spacing);
    stdDevReslice->SetOutputOrigin(origin);
    stdDevReslice->Update();

    int stdDevDim[3];
    stdDevReslice->GetOutput()->GetDimensions(stdDevDim);

    MRIBiasFieldCorrection_DebugMacro("stdDevDim "
                                      << stdDevDim[0] << " " << stdDevDim[1] << " " << stdDevDim[2]);

    if( stdDevDim[0] != dim[0] ||
        stdDevDim[1] != dim[1] ||
        stdDevDim[2] != dim[2] )
      {
      MRIBiasFieldCorrection_ErrorMacro("stdDevDim "
                                        << stdDevDim[0] << " " << stdDevDim[1] << " " << stdDevDim[2]
                                        << " != dim " << dim[0] << " " << dim[1] << " " << dim[2]);
      return 0;
      }

    imageResample = vtkImageResample::New();
    imageResample->SetInput(stdDevReslice->GetOutput() );
    imageResample->SetAxisMagnificationFactor(0, resampleFactorX);
    imageResample->SetAxisMagnificationFactor(1, resampleFactorY);
    imageResample->SetAxisMagnificationFactor(2, resampleFactorZ);
    imageResample->Update();

    double *ptrInputImage = static_cast<double *>
      (convertITKtoVTK->GetOutput()->GetScalarPointer() );

    double *ptrMeanReslice = static_cast<double *>
      (meanReslice->GetOutput()->GetScalarPointer() );

    double *ptrStdDevReslice = static_cast<double *>
      (stdDevReslice->GetOutput()->GetScalarPointer() );

    int numPts = convertITKtoVTK->GetOutput()->GetNumberOfPoints();
    MRIBiasFieldCorrection_DebugMacro("numPts " << numPts);

    MRIBiasFieldCorrection_DebugMacro("inputImage "
                                      << convertITKtoVTK->GetOutput()->GetScalarTypeAsString() );

    MRIBiasFieldCorrection_DebugMacro("meanReslice "
                                      << meanReslice->GetOutput()->GetScalarTypeAsString() );

    MRIBiasFieldCorrection_DebugMacro("stdDevReslice "
                                      << meanReslice->GetOutput()->GetScalarTypeAsString() );

    MRIBiasFieldCorrection_DebugMacro("ImageRegionIterator");

    itk::ImageRegionIterator<OutputImageType> ItOut( outputImage,
                                                     outputImage->GetLargestPossibleRegion() );
    for( ItOut.GoToBegin(); !ItOut.IsAtEnd(); ++ItOut, ++ptrInputImage,
         ++ptrMeanReslice, ++ptrStdDevReslice )
      {
      if( fabs(*ptrStdDevReslice) > 1e-12 )
        {
        ItOut.Set( (*ptrInputImage - *ptrMeanReslice)
                   / *ptrStdDevReslice );
        }
      else
        {
        ItOut.Set( *ptrInputImage - *ptrMeanReslice );
        }

      if( ItOut.Get() < -5.0 )
        {
        ItOut.Set(-5.0);
        }
      else if( ItOut.Get() > 5.0 )
        {
        ItOut.Set(5.0);
        }
      }

    MRIBiasFieldCorrection_DebugMacro("Created output image");
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << " : Exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------
int N3BiasFieldCorrection( int argc, char *argv[],
                           itk::Image<float, 3>::Pointer inputImage,
                           itk::Image<float, 3>::Pointer outputImage )
{
  MRIBiasFieldCorrection_DebugMacro("N3BiasFieldCorrection start");

  PARSE_ARGS;

  try
    {
    typedef itk::Image<float, 3> InputImageType;
    typedef itk::Image<float, 3> OutputImageType;

    typedef unsigned char                  MaskPixelType;
    typedef itk::Image<MaskPixelType, 3>   MaskType;
    typedef itk::ImageFileReader<MaskType> MaskReaderType;

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
    maskReader->SetFileName( InputMask.c_str() );
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

    typedef itk::BinaryThresholdImageFilter<InputImageType, MaskType>
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

    typedef itk::ShrinkImageFilter<MaskType, MaskType> MaskShrinkerType;

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
    bspliner->SetInput( correcter->GetLogBiasFieldControlPointLattice() );
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

    typedef itk::ExpImageFilter<InputImageType, InputImageType>
    ExpFilterType;

    ExpFilterType::Pointer expFilter = ExpFilterType::New();
    expFilter->SetInput( logField );
    expFilter->Update();

    typedef itk::DivideImageFilter<InputImageType, InputImageType,
                                   InputImageType> DividerType;

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
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << " : Exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------
int N4BiasFieldCorrection( int argc, char *argv[],
                           itk::Image<float, 3>::Pointer inputImage,
                           itk::Image<float, 3>::Pointer outputImage )
{
  MRIBiasFieldCorrection_DebugMacro("N4BiasFieldCorrection start");

  PARSE_ARGS;

  try
    {
    typedef itk::Image<float, 3> InputImageType;
    typedef itk::Image<float, 3> OutputImageType;

    typedef unsigned char                  MaskPixelType;
    typedef itk::Image<MaskPixelType, 3>   MaskType;
    typedef itk::ImageFileReader<MaskType> MaskReaderType;

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
    maskReader->SetFileName( InputMask.c_str() );
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

    typedef itk::BinaryThresholdImageFilter<InputImageType, MaskType>
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

    typedef itk::ShrinkImageFilter<MaskType, MaskType> MaskShrinkerType;

    MaskShrinkerType::Pointer maskShrinker = MaskShrinkerType::New();
    maskShrinker->SetInput( maskImage );
    maskShrinker->SetShrinkFactors( ShrinkFactor );

    maskShrinker->Update();
    maskShrinker->UpdateLargestPossibleRegion();

    MRIBiasFieldCorrection_DebugMacro(
      "N4MRIBiasFieldCorrectionImageFilter");

    typedef itk::N4MRIBiasFieldCorrectionImageFilter<InputImageType,
                                                     MaskType, InputImageType> CorrecterType;

    CorrecterType::Pointer correcter = CorrecterType::New();
    correcter->SetInput( shrinker->GetOutput() );
    correcter->SetMaskImage( maskShrinker->GetOutput() );

    CorrecterType::VariableSizeArrayType maximumNumberOfIterations(NumberOfFittingLevels);
    for( int i = 0; i < NumberOfFittingLevels; i++ )
      {
      maximumNumberOfIterations[i] = MaximumNumberOfIterations;
      }

    correcter->SetMaximumNumberOfIterations( maximumNumberOfIterations );
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
    bspliner->SetInput( correcter->GetLogBiasFieldControlPointLattice() );
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

    typedef itk::ExpImageFilter<InputImageType, InputImageType>
    ExpFilterType;

    ExpFilterType::Pointer expFilter = ExpFilterType::New();
    expFilter->SetInput( logField );
    expFilter->Update();

    typedef itk::DivideImageFilter<InputImageType, InputImageType,
                                   InputImageType> DividerType;

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
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << " : Exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------
template <class Tin, class Tout>
int DoIt(int argc, char *argv[], Tin, Tout)
{
  PARSE_ARGS;

  typedef Tin  InputPixelType;
  typedef Tout OutputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;

  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  typedef itk::CastImageFilter<InputImageType, OutputImageType>
  CastFilterType;

  typename ReaderType::Pointer reader = ReaderType::New();

  itk::PluginFilterWatcher watchReader(reader, "Read image",
                                       CLPProcessInformation);

  reader->SetFileName( InputImage.c_str() );
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
  outputImage->SetRegions(inputImage->GetLargestPossibleRegion().GetSize() );
  outputImage->SetDirection( inputImage->GetDirection() );
  outputImage->Allocate();

  if( AlgorithmType == "N4" )
    {
    N4BiasFieldCorrection(argc, argv, castFilter->GetOutput(), outputImage);
    }
  else if( AlgorithmType == "N3" )
    {
    N3BiasFieldCorrection(argc, argv, castFilter->GetOutput(), outputImage);
    }
  else
    {
    MRIBiasFieldCorrection_ErrorMacro("Algorithm type not implemented: "
                                      << AlgorithmType);
    return EXIT_FAILURE;
    }

  typename WriterType::Pointer writer = WriterType::New();

  itk::PluginFilterWatcher watchWriter( writer, "Write Volume",
                                        CLPProcessInformation );

  writer->SetFileName( OutputImage.c_str() );
  writer->SetInput( outputImage );
  writer->Update();

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

// ----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(InputImage, pixelType, componentType);

    float dummy = 0.0;

    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
      case itk::ImageIOBase::CHAR:

        return DoIt( argc, argv, static_cast<char>(0), dummy);

      case itk::ImageIOBase::USHORT:
      case itk::ImageIOBase::SHORT:

        return DoIt( argc, argv, static_cast<short>(0), dummy);

      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:

        return DoIt( argc, argv, static_cast<int>(0), dummy);

      case itk::ImageIOBase::ULONG:
      case itk::ImageIOBase::LONG:

        return DoIt( argc, argv, static_cast<long>(0), dummy);

      case itk::ImageIOBase::FLOAT:

        return DoIt( argc, argv, static_cast<float>(0), dummy);

      case itk::ImageIOBase::DOUBLE:

        return DoIt( argc, argv, static_cast<double>(0), dummy);

      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:

      default:
        std::cout << "Unknown component type " << componentType
                  << std::endl;
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
