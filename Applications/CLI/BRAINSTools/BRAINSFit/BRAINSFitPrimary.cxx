/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date: 2007-08-31 11:20:20 -0500 (Fri, 31 Aug 2007) $
  Version:   $Revision: 10358 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

  =========================================================================*/
#if defined( _MSC_VER )
#pragma warning ( disable : 4786 )
#endif

//  This program was modified from
// Insight/Examples/Registration/ImageRegistration8.cxx
//  and is an improved replacement for the old (and defective)
//  brains2/src/iplProg/MutualRegistration program.

//  Image registration using the VersorRigid3DTransform and the
//  MattesMutualInformationImageToImageMetric is the logical
//  thing to insist on (with ITK) when seeking rigid 3D registrations.

#include "BRAINSFitPrimary.h"

#ifdef USE_DEBUG_IMAGE_VIEWER
#include "DebugImageViewerClient.h"
#endif

#include "BRAINSFitHelper.h"

#include "BRAINSFitPrimaryCLP.h"


// Check that ITK was compiled with correct flags set:
#ifndef ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE
#error \
  "Results will not be correct if ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE is turned off"
#endif
#ifndef ITK_USE_ORIENTED_IMAGE_DIRECTION
#error \
  "Results will not be correct if ITK_USE_ORIENTED_IMAGE_DIRECTION is turned off"
#endif
#ifndef ITK_USE_TRANSFORM_IO_FACTORIES
#error \
  "BRAINSFit Requires ITK_USE_TRANSFORM_IO_FACTORIES to be on, please rebuild ITK."
#endif

#if (ITK_VERSION_MAJOR < 3  )
#if (ITK_VERSION_MINOR < 15 )
#if (ITK_VERSION_PATCH < 0 )
  #warning \
  "A version of ITK greater than 3.15.0 is needed in order to have the cross platform transform patches included."
  #warning \
  "Use at your own risk with versions less than 3.15.0."
  #warning \
  "Version 3.15.0 has fixed several issues related to inconsistent rounding on different platforms, and inconsistent definition of the center of voxels."
#endif
#endif
#endif

typedef float PixelType;
// Dimension and MaxInputDimension comes from an enum at the start of itkMultiModal3DMutualRegistrationHelper.h

typedef itk::Image<PixelType, Dimension>                            FixedVolumeType;
typedef itk::Image<PixelType, Dimension>                            MovingVolumeType;

typedef itk::Image<PixelType,
  MaxInputDimension>                    InputImageType;

typedef itk::ImageFileReader<InputImageType>
FixedVolumeReaderType;
typedef itk::ImageFileReader<InputImageType>
MovingVolumeReaderType;


typedef AffineTransformType::Pointer
AffineTransformPointer;

typedef itk::Vector<double, Dimension> VectorType;

// This function deciphers the BackgroundFillValueString and returns a double
// precision number based on the requested value
double GetBackgroundFillValueFromString(
  const std::string &BackgroundFillValueString )
{
  const std::string BIGNEGText("BIGNEG");
  const std::string NaNText("NaN");
  double            BackgroundFillValue = 0.0;

  if ( BackgroundFillValueString == BIGNEGText )
    {
    union {
      unsigned int i_val[2];
      double d_val;
      } FourByteHolder;
    FourByteHolder.i_val[0] = 0xF000F000;
    FourByteHolder.i_val[1] = 0xF000F000;
    BackgroundFillValue = FourByteHolder.d_val;
    }
  else if ( BackgroundFillValueString == NaNText )
    {
    union {
      unsigned int i_val[2];
      double d_val;
      } FourByteHolder;
    FourByteHolder.i_val[0] = 0xFFFFFFFF;
    FourByteHolder.i_val[1] = 0xFFFFFFFF;
    BackgroundFillValue = FourByteHolder.d_val;
    }
  else
    {
    BackgroundFillValue
      = static_cast<double>( atof( BackgroundFillValueString.c_str() ) );
    }
  return BackgroundFillValue;
}


template <class ImageType>
typename ImageType::Pointer ExtractImage(
  typename InputImageType::Pointer & inputImage,
  unsigned int InputImageTimeIndex)
{
  typedef typename itk::ExtractImageFilter<InputImageType,
    ImageType> ExtractImageFilterType;
  typename ExtractImageFilterType::Pointer extractImageFilter
    = ExtractImageFilterType::New();

  // fixedVolumeReader->GetOutput();
  InputImageType::RegionType inputRegion = inputImage->GetLargestPossibleRegion();
  InputImageType::SizeType   inputSize = inputRegion.GetSize();
  inputSize[3] = 0;
  inputRegion.SetSize(inputSize);

  InputImageType::IndexType inputIndex = inputRegion.GetIndex();
  inputIndex[0] = 0;
  inputIndex[1] = 0;
  inputIndex[2] = 0;
  inputIndex[3] = InputImageTimeIndex;
  inputRegion.SetIndex(inputIndex);
  extractImageFilter->SetExtractionRegion( inputRegion );
  extractImageFilter->SetInput(inputImage);

  try
    {
    extractImageFilter->Update();
    }
  catch (... )
    {
    std::cout << "Error while extracting a time indexed fixed image."
              << std::endl;
    throw;
    }
  typename ImageType::Pointer extractImage = extractImageFilter->GetOutput();
  //  std::cerr << "Extract fixed image origin" << extractImage->GetOrigin() <<
  // std::endl;

  return extractImage;
}

template <class ImageType>
typename ImageType::Pointer DoMedian(typename ImageType::Pointer & input,
  typename ImageType::SizeType indexRadius)
{
  typedef typename itk::MedianImageFilter<ImageType,
    ImageType> MedianFilterType;
  typename MedianFilterType::Pointer medianFilter = MedianFilterType::New();

  medianFilter->SetRadius( indexRadius );
  medianFilter->SetInput( input );
  medianFilter->Update();
  typename ImageType::Pointer result = medianFilter->GetOutput();
  return result;
}



#ifdef USE_DEBUG_IMAGE_VIEWER
/*************************
 * Have a global variable to
 * add debugging information.
*/
DebugImageViewerClient DebugImageDisplaySender;
#endif

int BRAINSFitPrimary( int argc, char *argv[] )
{
  PARSE_ARGS;
  RegisterBrains2MaskFactory();

#ifdef USE_DEBUG_IMAGE_VIEWER
  if(UseDebugImageViewer) 
    {
    DebugImageDisplaySender.SetEnabled(UseDebugImageViewer);
    }
#endif

  if ( debugNumberOfThreads > 0 ) // NOTE: Default is -1, which then uses the
                                  // ITK default.
    {
    itk::MultiThreader::SetGlobalMaximumNumberOfThreads(debugNumberOfThreads);
    }
  if ( ( initializeTransformMode != "Off" ) && ( initialTransform.size() > 0 ) )
    {
    std::cout
   << "FATAL ERROR:  initializeTransformMode and initialTransform can not be used at the same time!"
   << std::endl;
    exit(-1);
    }
  std::vector<int> zeroOrigin(3, 0);

  //Verify that the spline grid sizes are greater than 3
    {
    bool validGridSize=true;
    for( unsigned int sgs=0; sgs< splineGridSize.size(); sgs++)
      {
      if(splineGridSize[sgs] < 3)
        {
        validGridSize=false;
        std::cout << "splineGridSize["<<sgs<<"]= "<< splineGridSize[sgs]
          << " is invalid.  There must be at lest 3 divisions in each dimension of the image." << std::endl;
        }
      }
    if(validGridSize == false)
      {
      exit(-1);
      }
    }

  //  const bool explicitOriginsFlag(fixedVolumeOriginArg.isSet() &&
  //                                 movingVolumeOriginArg.isSet());

  if ( outputTransform.size() == 0
       && strippedOutputTransform.size() == 0
       && outputVolume.size() == 0 )
    {
    std::cout << "Error:  user requested neither outputTransform,"
              << " nor strippedOutputTransform,"
              << " nor outputVolume." << std::endl;
    return 2;
    }

  if(numberOfIterations.size() != transformType.size())
    {
    if(numberOfIterations.size() != 1)
      {
      std::cerr << "The numberOfIterations array must match the transformType length" <<std::endl;
      exit(-1);
      }
    else
      {
      // replicate throughout
      const int numberOf = numberOfIterations[0];
      for (unsigned int i=1; i<transformType.size(); i++)
        {
        numberOfIterations.push_back(numberOf);
        }
      }
    }
  if(minimumStepSize.size() != transformType.size())
    {
    if(minimumStepSize.size() != 1)
      {
      std::cerr << "The minimumStepSize array must match the transformType length" <<std::endl;
      exit(-1);
      }
    else
      {
      // replicate throughout
      const double stepSize = minimumStepSize[0];
      for (unsigned int i=1; i<transformType.size(); i++)
        {
        minimumStepSize.push_back(stepSize);
        }
      }
    }

  //Need to ensure that the order of transforms is from smallest to largest.
  itk::ValidateTransformRankOrdering(transformType);

  // Extracting a timeIndex cube from the fixed image goes here....
  // Also MedianFilter
  FixedVolumeType::Pointer  extractFixedVolume;
  MovingVolumeType::Pointer extractMovingVolume;
  InputImageType::Pointer
  OriginalFixedVolume ( itkUtil::ReadImage<InputImageType>(fixedVolume) );

  std::cerr << "Original Fixed image origin"
            << OriginalFixedVolume->GetOrigin() << std::endl;
  // fixedVolumeTimeIndex lets lets us treat 3D as 4D.
  /***********************
   * Acquire Fixed Image Index
   **********************/
  extractFixedVolume = ExtractImage<FixedVolumeType>(OriginalFixedVolume,
                                                     fixedVolumeTimeIndex);
  // Extracting a timeIndex cube from the moving image goes here....

  InputImageType::Pointer OriginalMovingVolume (
    itkUtil::ReadImage<InputImageType>(movingVolume) );
  // This default lets us treat 3D as 4D.
  // const unsigned int movingVolumeTimeIndex;

  /***********************
   * Acquire Moving Image Index
   **********************/
  extractMovingVolume = ExtractImage<MovingVolumeType>(OriginalMovingVolume,
                                                       movingVolumeTimeIndex);

#ifdef USE_DEBUG_IMAGE_VIEWER
  if(DebugImageDisplaySender.Enabled())
    {
    DebugImageDisplaySender.SendImage<itk::Image<float, 3> >(extractFixedVolume,0);
    DebugImageDisplaySender.SendImage<itk::Image<float, 3> >(extractMovingVolume,1);
    }
#endif

  // get median filter radius.
  // const unsigned int MedianFilterRadius =
  // command.GetValueAsInt(MedianFilterRadiusText, IntegerText);
  // Median Filter images if requested.
  if ( medianFilterSize[0] > 0 || medianFilterSize[1] > 0
       || medianFilterSize[2] > 0 )
    {
    FixedVolumeType::SizeType indexRadius;
    indexRadius[0] = static_cast<long int>( medianFilterSize[0] ); // radius along x
    indexRadius[1] = static_cast<long int>( medianFilterSize[1] ); // radius along y
    indexRadius[2] = static_cast<long int>( medianFilterSize[2] ); // radius along z
    // DEBUG
    std::cout << "Median radius  " << indexRadius << std::endl;
    extractFixedVolume = DoMedian<FixedVolumeType>(extractFixedVolume,
                                                   indexRadius);
    extractMovingVolume = DoMedian<MovingVolumeType>(extractMovingVolume,
                                                     indexRadius);
    }

  //
  // If masks are associated with the images, then read them into the correct
  // orientation.
  // if they've been defined assign the masks...
  //
  ImageMaskPointer fixedMask = NULL;
  ImageMaskPointer movingMask = NULL;
    {
    if (maskProcessingMode == "NOMASK")
      {
      if ( ( fixedBinaryVolume != "" )
        || ( movingBinaryVolume != "" ) )
        {
        std::cout
          << "ERROR:  Can not specify mask file names when the default of NOMASK is used for the maskProcessingMode"
          << std::endl;
        exit(-1);
        }
      }
    else if ( maskProcessingMode == "ROIAUTO" )
      {
      if ( ( fixedBinaryVolume != "" )
        || ( movingBinaryVolume != "" ) )
        {
        std::cout
          << "ERROR:  Can not specify mask file names when ROIAUTO is used for the maskProcessingMode"
          << std::endl;
        exit(-1);
        }
      const double otsuPercentileThreshold = 0.01;
      const int    closingSize = 9;
        {
        fixedMask = DoROIAUTO<FixedVolumeType>(extractFixedVolume,
          otsuPercentileThreshold,
          closingSize);
        }
        {
        movingMask = DoROIAUTO<MovingVolumeType>(extractMovingVolume,
          otsuPercentileThreshold,
          closingSize);
        }
      }
    else if ( maskProcessingMode == "ROI" )
      {
      if ( ( fixedBinaryVolume == "" )
        || ( movingBinaryVolume == "" ) )
        {
        std::cout
          <<
          "ERROR:  Must specify mask file names when ROI is used for the maskProcessingMode"
          << std::endl;
        exit(-1);
        }
      fixedMask = ReadImageMask<SpatialObjectType, Dimension>(
        fixedBinaryVolume,
        extractFixedVolume.GetPointer() );
      movingMask = ReadImageMask<SpatialObjectType, Dimension>(
        movingBinaryVolume,
        extractMovingVolume.GetPointer() );
      }
      { //Write out some debugging information if requested
      typedef itk::Image<unsigned char, 3> MaskImageType;
      typedef itk::ImageMaskSpatialObject<MaskImageType::ImageDimension> ImageMaskSpatialObjectType;
      if( ( !fixedMask.IsNull() ) && ( outputFixedVolumeROI != "" ) )
        {
        ImageMaskSpatialObjectType::Pointer fixedImageMask(
          dynamic_cast< ImageMaskSpatialObjectType * >( fixedMask.GetPointer() ));
        MaskImageType::Pointer tempOutputFixedVolumeROI = const_cast<MaskImageType *>( fixedImageMask->GetImage() );
        itkUtil::WriteImage<MaskImageType>(tempOutputFixedVolumeROI,outputFixedVolumeROI);
        }
      if( ( !movingMask.IsNull() ) && ( outputMovingVolumeROI != "" ) )
        {
        ImageMaskSpatialObjectType::Pointer movingImageMask(
          dynamic_cast< ImageMaskSpatialObjectType * >( movingMask.GetPointer() ));
        MaskImageType::Pointer tempOutputMovingVolumeROI = const_cast<MaskImageType *>( movingImageMask->GetImage() );
        itkUtil::WriteImage<MaskImageType>(tempOutputMovingVolumeROI,outputMovingVolumeROI);
        }
      }
    }
  // This default fills the background with zeros
  //  const double BackgroundFillValue =
  //  GetBackgroundFillValueFromString(command.GetValueAsString(BackgroundFillValueText,
  //  FloatCodeText));

  //  const std::string patientID(command.GetValueAsString(patientIDText,
  // IdStringText));
  //  const std::string studyID(command.GetValueAsString(patientIDText,
  // IdStringText));
  // Apparently when you register one transform, you need to register all your
  // transforms.
  //
  itk::TransformFactory<VersorRigid3DTransformType>::RegisterTransform();
  itk::TransformFactory<ScaleVersor3DTransformType>::RegisterTransform();
  itk::TransformFactory<ScaleSkewVersor3DTransformType>::RegisterTransform();
  itk::TransformFactory<AffineTransformType>::RegisterTransform();
  itk::TransformFactory<BSplineTransformType>::RegisterTransform();

  GenericTransformType::Pointer currentGenericTransform=itk::ReadTransformFromDisk(initialTransform);


  FixedVolumeType::Pointer resampledImage;
  /*
   *  Everything prior to this point is preprocessing
   *  Start Processing
   *
   */
  int actualIterations = 0;
  int permittedIterations = 0;
  //int allLevelsIterations=0;


  {
  typedef itk::BRAINSFitHelper HelperType;
  HelperType::Pointer myHelper=HelperType::New();
  myHelper->SetTransformType(transformType);
  myHelper->SetFixedVolume(extractFixedVolume);
  myHelper->SetMovingVolume(extractMovingVolume);
  myHelper->SetHistogramMatch(histogramMatch);
  myHelper->SetNumberOfMatchPoints(numberOfMatchPoints);
  myHelper->SetFixedBinaryVolume(fixedMask);
  myHelper->SetMovingBinaryVolume(movingMask);
  myHelper->SetPermitParameterVariation(permitParameterVariation);
  myHelper->SetNumberOfSamples(numberOfSamples);
  myHelper->SetNumberOfHistogramBins(numberOfHistogramBins);
  myHelper->SetNumberOfIterations(numberOfIterations);
  myHelper->SetMaximumStepLength(maximumStepSize);
  myHelper->SetMinimumStepLength(minimumStepSize);
  myHelper->SetRelaxationFactor(relaxationFactor);
  myHelper->SetTranslationScale(translationScale);
  myHelper->SetReproportionScale(reproportionScale);
  myHelper->SetSkewScale(skewScale);
  myHelper->SetUseExplicitPDFDerivativesMode(useExplicitPDFDerivativesMode);
  myHelper->SetUseCachingOfBSplineWeightsMode(useCachingOfBSplineWeightsMode);
  myHelper->SetBackgroundFillValue(backgroundFillValue);
  myHelper->SetInitializeTransformMode(initializeTransformMode);
  myHelper->SetMaskInferiorCutOffFromCenter(maskInferiorCutOffFromCenter);
  myHelper->SetCurrentGenericTransform(currentGenericTransform);
  myHelper->SetUseWindowedSinc(useWindowedSinc);
  myHelper->SetSplineGridSize(splineGridSize);
  myHelper->SetCostFunctionConvergenceFactor(costFunctionConvergenceFactor);
  myHelper->SetProjectedGradientTolerance(projectedGradientTolerance);
  myHelper->SetMaxBSplineDisplacement(maxBSplineDisplacement);
  myHelper->SetDisplayDeformedImage(UseDebugImageViewer);
  myHelper->SetPromptUserAfterDisplay(PromptAfterImageSend);
  myHelper->SetDebugLevel(debugLevel);
  if(debugLevel > 7 )
    {
    myHelper->PrintCommandLine(true,"BF");
    }
  myHelper->StartRegistration();
  currentGenericTransform=myHelper->GetCurrentGenericTransform();
  MovingVolumeType::ConstPointer preprocessedMovingVolume = myHelper->GetPreprocessedMovingVolume();


{
  typedef itk::ResampleImageFilter<FixedVolumeType,MovingVolumeType,double> ResampleFilterType;
  ResampleFilterType::Pointer resampler = ResampleFilterType::New();

  typedef itk::Transform<double,3,3> GenericTransformType;
  resampler->SetTransform( currentGenericTransform );
  resampler->SetInput( preprocessedMovingVolume );
  // Remember:  the Data is Moving's, the shape is Fixed's.
  resampler->SetOutputParametersFromImage(extractFixedVolume);
  resampler->SetDefaultPixelValue( backgroundFillValue );

  if ( useWindowedSinc == true )
    {
    typedef itk::ConstantBoundaryCondition<MovingVolumeType>
    BoundaryConditionType;
    static const unsigned int WindowedSincHammingWindowRadius = 5;
    typedef itk::Function::HammingWindowFunction<
      WindowedSincHammingWindowRadius, double, double> WindowFunctionType;
    typedef itk::WindowedSincInterpolateImageFunction<
      MovingVolumeType,
      WindowedSincHammingWindowRadius,
      WindowFunctionType,
      BoundaryConditionType,
      double>    WindowedSincInterpolatorType;

    WindowedSincInterpolatorType::Pointer interpolator
      = WindowedSincInterpolatorType::New();
    resampler->SetInterpolator( interpolator );
    }
  else     // Default to LINEAR_INTERP
    {
    // NOTE: Linear is the default.  resampler->SetInterpolator( interpolator );
    }
  resampler->Update();       //  Explicit Update() required here.
  FixedVolumeType::Pointer ResampledImage = resampler->GetOutput();
  resampledImage=ResampledImage;
}
  //resampledImage=myHelper->GetResampledImage();
  //HelperType::LINEAR_INTERP);
  actualIterations=myHelper->GetActualNumberOfIterations();
  permittedIterations=myHelper->GetPermittedNumberOfIterations();
  //allLevelsIterations=myHelper->GetAccumulatedNumberOfIterationsForAllLevels();
  }
  /*
   *  At this point we can save the resampled image.
   */

  if ( outputVolume.size() > 0 )
    {
    //      std::cout << "=========== resampledImage :\n" <<
    // resampledImage->GetDirection() << std::endl;
    // Set in PARSEARGS const bool scaleOutputValues=false;//TODO: Make this a
    // command line parameter
    if ( outputVolumePixelType == "float" )
      {
      // itkUtil::WriteCastImage<itk::Image<float, FixedVolumeType::ImageDimension>, FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image<float,
        FixedVolumeType::ImageDimension> WriteOutImageType;
      WriteOutImageType::Pointer CastImage
        = ( scaleOutputValues == true ) ?
          ( itkUtil::PreserveCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) ) :
          ( itkUtil::TypeCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) );
      itkUtil::WriteImage<WriteOutImageType>(CastImage, outputVolume);
      }
    else if ( outputVolumePixelType == "short" )
      {
      // itkUtil::WriteCastImage<itk::Image<signed short, FixedVolumeType::ImageDimension>, FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image<signed short,
        FixedVolumeType::ImageDimension> WriteOutImageType;
      WriteOutImageType::Pointer CastImage
        = ( scaleOutputValues == true ) ?
          ( itkUtil::PreserveCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) ) :
          ( itkUtil::TypeCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) );
      itkUtil::WriteImage<WriteOutImageType>(CastImage, outputVolume);
      }
    else if ( outputVolumePixelType == "ushort" )
      {
      // itkUtil::WriteCastImage<itk::Image<unsigned short, FixedVolumeType::ImageDimension>, FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image<unsigned short,
        FixedVolumeType::ImageDimension> WriteOutImageType;
      WriteOutImageType::Pointer CastImage
        = ( scaleOutputValues == true ) ?
          ( itkUtil::PreserveCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) ) :
          ( itkUtil::TypeCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) );
      itkUtil::WriteImage<WriteOutImageType>(CastImage, outputVolume);
      }
    else if ( outputVolumePixelType == "int" )
      {
      // itkUtil::WriteCastImage<itk::Image<signed int, FixedVolumeType::ImageDimension>, FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image<signed int,
        FixedVolumeType::ImageDimension> WriteOutImageType;
      WriteOutImageType::Pointer CastImage
        = ( scaleOutputValues == true ) ?
          ( itkUtil::PreserveCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) ) :
          ( itkUtil::TypeCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) );
      itkUtil::WriteImage<WriteOutImageType>(CastImage, outputVolume);
      }
    else if ( outputVolumePixelType == "uint" )
      {
      // itkUtil::WriteCastImage<itk::Image<unsigned int, FixedVolumeType::ImageDimension>, FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image<unsigned int,
        FixedVolumeType::ImageDimension> WriteOutImageType;
      WriteOutImageType::Pointer CastImage
        = ( scaleOutputValues == true ) ?
          ( itkUtil::PreserveCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) ) :
          ( itkUtil::TypeCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) );
      itkUtil::WriteImage<WriteOutImageType>(CastImage, outputVolume);
      }
#if 0
    else if ( outputVolumePixelType == "char" )
      {
      // itkUtil::WriteCastImage<itk::Image<signed char, FixedVolumeType::ImageDimension>, FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image<char,
        FixedVolumeType::ImageDimension> WriteOutImageType;
      WriteOutImageType::Pointer CastImage
        = ( scaleOutputValues == true ) ?
          ( itkUtil::PreserveCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) ) :
          ( itkUtil::TypeCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) );
      itkUtil::WriteImage<WriteOutImageType>(CastImage, outputVolume);
      }
#endif
    else if ( outputVolumePixelType == "uchar" )
      {
      // itkUtil::WriteCastImage<itk::Image<unsigned char, FixedVolumeType::ImageDimension>, FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image<unsigned char,
        FixedVolumeType::ImageDimension> WriteOutImageType;
      WriteOutImageType::Pointer CastImage
        = ( scaleOutputValues == true ) ?
          ( itkUtil::PreserveCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) ) :
          ( itkUtil::TypeCast<FixedVolumeType,
             WriteOutImageType>(resampledImage) );
      itkUtil::WriteImage<WriteOutImageType>(CastImage, outputVolume);
      }
    }

// GREG:  BRAINSFit currently does not determine if the registrations have not converged before reaching their maximum number of iterations.  Currently transforms are always written out, under the assumption that the registraiton converged.  We need to figure out how to determine if the registrations did not converge (i.e. maximum number of iterations were reached), and then not write out the transforms, unless explicitly demanded to write them out from a command line flag.
// GREG:  We should write a test, and document what the expected behaviors are when a multi-level registration is requested (Rigid,ScaleSkew,Affine), and one of the first types does not converge.

    if ( actualIterations + 1 >= permittedIterations )
      {
      if (writeTransformOnFailure == false) // taken right off the command line.
        {
        std::cout << "actualIterations: " << actualIterations << std::endl;
        std::cout << "permittedIterations: " << permittedIterations << std::endl;
        return failureExitCode; // taken right off the command line.
        }
      }

  /*const int write_status=*/
    WriteBothTransformsToDisk(currentGenericTransform,outputTransform,strippedOutputTransform);

  if ( actualIterations + 1 >= permittedIterations )
    {
    std::cout << "actualIterations: " << actualIterations << std::endl;
    std::cout << "permittedIterations: " << permittedIterations << std::endl;
    return failureExitCode; // taken right off the command line.
    }

  return 0;
}
