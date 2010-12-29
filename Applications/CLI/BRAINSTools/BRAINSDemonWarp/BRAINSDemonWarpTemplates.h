#ifndef __BRAINSDemonWarpTemplates_h
#define __BRAINSDemonWarpTemplates_h

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include "itkImage.h"
#include "itkIndex.h"
#include "itkSize.h"
#include "itkExceptionObject.h"
#include "BRAINSDemonWarpCommonLibWin32Header.h"
#include "GenericTransformImage.h"
#include "VBRAINSDemonWarp.h"
#include "BRAINSDemonWarp.h"

#include "itkSymmetricForcesDemonsRegistrationFunction.h"
#include "itkFastSymmetricForcesDemonsRegistrationFunction.h"

#include "itkPDEDeformableRegistrationFilter.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkSymmetricForcesDemonsRegistrationFilter.h"
#include "itkLogDomainDeformableRegistrationFilter.h"
#include "itkLogDomainDemonsRegistrationFilter.h"
#include "itkFastSymmetricForcesDemonsRegistrationFilter.h"
#include "itkDiffeomorphicDemonsRegistrationFilter.h"
#include "itkDiffeomorphicDemonsRegistrationWithMaskFilter.h"
#include "itkVectorDiffeomorphicDemonsRegistrationFilter.h"
#include "itkSymmetricLogDomainDemonsRegistrationFilter.h"
#include "itkESMDemonsRegistrationWithMaskFunction.h"
#include "itkArray.h"
#include "itkIO.h"
#if defined( USE_DEBUG_IMAGE_VIEWER )
#  include "DebugImageViewerClient.h"
BRAINSDemonWarpCommonLib_EXPORT extern DebugImageViewerClient DebugImageDisplaySender;
#endif

#include "VcommandIterationupdate.h"
#include "itkLargestForegroundFilledMaskImageFilter.h"

#include "commandIterationupdate.h"
#include "ReadMask.h"
#include "itkImageMaskSpatialObject.h"

BRAINSDemonWarpCommonLib_EXPORT extern void PrintDataTypeStrings(void);

BRAINSDemonWarpCommonLib_EXPORT extern int CompareNoCase(const std::string & s, const std::string & s2);

BRAINSDemonWarpCommonLib_EXPORT extern int BRAINSResamplePrimary(int argc, char *argv[]);
BRAINSDemonWarpCommonLib_EXPORT extern void ProcessOutputType_uchar(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void ProcessOutputType_short(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void ProcessOutputType_ushort(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void ProcessOutputType_int(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void ProcessOutputType_uint(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void ProcessOutputType_float(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void ProcessOutputType_double(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void VectorProcessOutputType_uchar(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void VectorProcessOutputType_short(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void VectorProcessOutputType_ushort(
  struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void VectorProcessOutputType_int(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void VectorProcessOutputType_uint(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void VectorProcessOutputType_float(struct BRAINSDemonWarpAppParameters & command);

BRAINSDemonWarpCommonLib_EXPORT extern void VectorProcessOutputType_double(
  struct BRAINSDemonWarpAppParameters & command);

struct BRAINSDemonWarpAppParameters {
  std::string movingVolume;
  std::string fixedVolume;
  std::string outputVolume;
  std::string outputDeformationFieldVolume;
  std::string inputPixelType;
  std::string outputPixelType;
  std::string outputDisplacementFieldPrefix;
  std::string outputCheckerboardVolume;
  std::string registrationFilterType;
  itk::Index< 3 > checkerboardPatternSubdivisions;
  bool outputNormalized;
  bool outputDebug;
  std::string maskProcessingMode;
  std::string fixedBinaryVolume;
  std::string movingBinaryVolume;
  int lowerThresholdForBOBF;
  int upperThresholdForBOBF;
  int backgroundFillValue;
  itk::Index< 3 > seedForBOBF;
  itk::Index< 3 > neighborhoodForBOBF;
  itk::Size< 3 > medianFilterSize;
  /*/Not yet implemented
    bool forceCoronalZeroOrigin;
    std::string movingLandmarks;
    std::string fixedLandmarks;
    std::string initializeWithFourier;
    */
  std::string initializeWithDeformationField;
  std::string initializeWithTransform;
  unsigned int numberOfBCHApproximationTerms;

  /** Smoothing sigma for the deformation field at each iteration.*/
  float smoothDeformationFieldSigma;

  /** Maximum lengthof an update vector. */
  float maxStepLength;

  /** Type of gradient used for computing the demons force. */
  unsigned int gradientType;

  /** Smoothing sigma for the update field at each iteration. */
  float smoothingUp;

  /** Intensity_histogram_matching. */
  bool histogramMatch;

  /** ShrinkFactors type. */
  typedef itk::FixedArray< unsigned int, 3 > ShrinkFactorsType;

  /** IterationArray type. */
  typedef itk::Array< unsigned int > IterationsArrayType;
  unsigned long numberOfHistogramLevels;
  unsigned long numberOfMatchPoints;
  unsigned short numberOfLevels;
  ShrinkFactorsType theMovingImageShrinkFactors;
  ShrinkFactorsType theFixedImageShrinkFactors;
  IterationsArrayType numberOfIterations;
  // VECTORPARAMS
  std::vector< std::string > vectorMovingVolume;
  std::vector< std::string > vectorFixedVolume;
  bool makeBOBF;
  typedef itk::Array< float > WeightFactorsType;
  WeightFactorsType weightFactors;
  std::string interpolationMode;
};

// This function calls the Thirion registration filter setting all the
// parameters.
template< class InPixelType, class OutPixelType >
void ThirionFunction(const struct BRAINSDemonWarpAppParameters & command)
{
  const int dims = 3;

  typedef itk::Image< InPixelType, dims >                ImageType;
  typedef itk::Image< float, dims >                      TRealImage;
  typedef itk::Image< OutPixelType, dims >               OutputImageType;
  typedef itk::Image< itk::Vector< float, dims >, dims > TDeformationField;
  typedef itk::Image< itk::Vector< float, dims >, dims > TVelocityField;

  typedef unsigned char                                     MaskPixelType;
  typedef itk::Image< MaskPixelType, dims >                 MaskImageType;
  typedef itk::CastImageFilter< TRealImage, MaskImageType > CastImageFilter;

  typedef itk::ImageMaskSpatialObject< dims >                           ImageMaskSpatialObjectType;
  typedef itk::SpatialObject< GenericTransformImageNS::SpaceDimension > SpatialObjectType;

  //
  // If optional landmark files given, will use landmark registration to
  // generate
  // a deformation field to prime the thirion demons registration.

  typedef typename itk::BRAINSDemonWarp
  < ImageType,
    TRealImage,
    OutputImageType
  > AppType;
  typename  AppType::Pointer app = AppType::New ();

  // Set up the diffeomorphic demons filter with mask

  if ( command.outputDebug )
    {
    std::cout << command.registrationFilterType
              << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
              << std::endl;
    }

  typename CommandIterationUpdate< float, 3 >::Pointer observer;
  if ( command.outputDebug )
    {
    observer = CommandIterationUpdate< float, 3 >::New();
    }
  if ( command.registrationFilterType == "SymmetricLogDemons" || command.registrationFilterType == "LogDemons" )
    {
    app->SetLogDomain(true);
    typedef typename itk::LogDomainDeformableRegistrationFilter< TRealImage, TRealImage, TVelocityField >
    BaseRegistrationFilterType;
    typename BaseRegistrationFilterType::Pointer filter;

    if ( command.registrationFilterType == "LogDemons" )
      {
      typedef typename itk::LogDomainDemonsRegistrationFilter< TRealImage, TRealImage, TVelocityField >
      ActualRegistrationFilterType;
      typedef typename ActualRegistrationFilterType::GradientType GradientType;

      typename ActualRegistrationFilterType::Pointer actualfilter =
        ActualRegistrationFilterType::New();

      actualfilter->SetMaximumUpdateStepLength(command.maxStepLength);
      actualfilter->SetUseGradientType( static_cast< GradientType >( command.gradientType ) );
      actualfilter->SetNumberOfBCHApproximationTerms(command.numberOfBCHApproximationTerms);
      filter = actualfilter;
      }
    if ( command.registrationFilterType == "SymmetricLogDemons" )
      {
      typedef typename itk::SymmetricLogDomainDemonsRegistrationFilter< TRealImage, TRealImage, TVelocityField >
      ActualRegistrationFilterType;
      typename ActualRegistrationFilterType::Pointer actualfilter = ActualRegistrationFilterType::New();
      typedef  typename ActualRegistrationFilterType::GradientType GradientType;

      actualfilter->SetMaximumUpdateStepLength(command.maxStepLength);
      actualfilter->SetUseGradientType( static_cast< GradientType >( command.gradientType ) );
      actualfilter->SetNumberOfBCHApproximationTerms(command.numberOfBCHApproximationTerms);
      filter = actualfilter;
      }
    if ( command.smoothDeformationFieldSigma > 0.1 )
      {
      if ( command.outputDebug )
        {
        std::cout << " Smoothing is on ....." << std::endl;
        }
      filter->SmoothVelocityFieldOn();
      filter->SetStandardDeviations(command.smoothDeformationFieldSigma);
      }
    else
      {
      filter->SmoothVelocityFieldOff();
      }
    if ( command.smoothingUp > 0.1 )
      {
      if ( command.outputDebug )
        {
        std::cout << " Smoothing at update....." << std::endl;
        }
      filter->SmoothUpdateFieldOn();
      filter->SetUpdateFieldStandardDeviations(command.smoothingUp);
      }
    else
      {
      filter->SmoothUpdateFieldOff();
      }
    if ( command.outputDebug )
      {
      filter->AddObserver(itk::IterationEvent(), observer);
      }

    app->SetLDDRegistrationFilter(filter);
    }

  else
    {
    // Set up the demons filter
    typedef typename itk::PDEDeformableRegistrationFilter< TRealImage, TRealImage,
                                                           TDeformationField >
    BaseRegistrationFilterType;
    // BaseRegistrationFilterType::Pointer filter =
    //   BaseRegistrationFilterType::New();
    typename BaseRegistrationFilterType::Pointer filter;

    if ( command.registrationFilterType == "Demons" )
      {
      typedef typename itk::DemonsRegistrationFilter< TRealImage, TRealImage,
                                                      TDeformationField >
      ActualRegistrationFilterType;
      ActualRegistrationFilterType::Pointer actualfilter =
        ActualRegistrationFilterType::New();
      // TODO:  Review this value setting with Insight Journal Diffeomorphic
      // implementation.
      // actualfilter->SetMaximumUpdateStepLength( command.maxStepLength );
      // NOTE: GRADIENT TYPE NOT AVAILABLE IN DemonsRegistrationFilter
      // typedef ActualRegistrationFilterType::GradientType GradientType;
      // actualfilter->SetUseGradientType(
      // static_cast<GradientType>(command.gradientType) );
      // actualfilter->SetUseMovingImageGradient(true);
      filter = actualfilter;
      }
    else if ( command.registrationFilterType == "Diffeomorphic" )
      {
      typedef typename itk::DiffeomorphicDemonsRegistrationWithMaskFilter<
        TRealImage, TRealImage,
        TDeformationField >   ActualRegistrationFilterType;
      typename ActualRegistrationFilterType::Pointer actualfilter =
        ActualRegistrationFilterType::New();

      typedef  typename ActualRegistrationFilterType::GradientType GradientType;
      actualfilter->SetMaximumUpdateStepLength(command.maxStepLength);
      actualfilter->SetUseGradientType( static_cast< GradientType >( command.
                                                                     gradientType ) );
      // It would be preferable that this would be part of the "Application"
      // TODO:  Move this bit of data into the application portion.
      if ( command.maskProcessingMode == "ROIAUTO" )
        {
        if ( ( command.fixedBinaryVolume != "" )
             || ( command.movingBinaryVolume != "" ) )
          {
          std::cout
          <<
          "ERROR:  Can not specify mask file names when ROIAUTO is used for the maskProcessingMode"
          << std::endl;
          exit(-1);
          }
        std::cout << "Diffeomorphic with autogenerated Mask!!!!!!!"
                  << std::endl;
        typename  TRealImage::Pointer movingBinaryVolumeImage;
        typename  TRealImage::Pointer fixedBinaryVolumeImage;
        const double otsuPercentileThreshold = 0.01;
        const int    closingSize = 7;
        // typedef itk::Image<signed long, dims> LargeIntegerImage;

        typename  TRealImage::Pointer fixedVolume =
          itkUtil::ReadImage< TRealImage >( command.fixedVolume.c_str () );
        //       fixedBinaryVolumeImage =
        // FindLargestForgroundFilledMask<TRealImage>(
        //       fixedVolume,
        //       otsuPercentileThreshold,
        //       closingSize);
        typedef itk::LargestForegroundFilledMaskImageFilter< TRealImage > LFFMaskFilterType;
        LFFMaskFilterType::Pointer LFF = LFFMaskFilterType::New();
        LFF->SetInput(fixedVolume);
        LFF->SetOtsuPercentileThreshold(otsuPercentileThreshold);
        LFF->SetClosingSize(closingSize);
        LFF->Update();
        fixedBinaryVolumeImage = LFF->GetOutput();

        typename CastImageFilter::Pointer castFixedMaskImage = CastImageFilter::New();
        castFixedMaskImage->SetInput(fixedBinaryVolumeImage);
        castFixedMaskImage->Update();

        //      typename MaskImageType::Pointer fm =
        // castFixedMaskImage->GetOutput();
        //      itkUtil::WriteImage<MaskImageType>(fm,
        // "fixedMaskImage.nii.gz");

        // convert mask image to mask
        typename ImageMaskSpatialObjectType::Pointer fixedMask = ImageMaskSpatialObjectType::New();
        fixedMask->SetImage( castFixedMaskImage->GetOutput() );
        fixedMask->ComputeObjectToWorldTransform();

        typename  TRealImage::Pointer movingVolume =
          itkUtil::ReadImage< TRealImage >( command.movingVolume.c_str () );
        //         movingBinaryVolumeImage =
        // FindLargestForgroundFilledMask<TRealImage>(
        //         movingVolume,
        //         otsuPercentileThreshold,
        //         closingSize);
        LFF->SetInput(movingVolume);
        LFF->SetOtsuPercentileThreshold(otsuPercentileThreshold);
        LFF->SetClosingSize(closingSize);
        LFF->Update();
        movingBinaryVolumeImage = LFF->GetOutput();

        typename CastImageFilter::Pointer castMovingMaskImage = CastImageFilter::New();
        castMovingMaskImage->SetInput(movingBinaryVolumeImage);
        castMovingMaskImage->Update();

        //      typename MaskImageType::Pointer mm =
        // castMovingMaskImage->GetOutput();
        //      itkUtil::WriteImage<MaskImageType>(mm,
        // "movingMaskImage.nii.gz");

        // convert mask image to mask
        typename ImageMaskSpatialObjectType::Pointer movingMask = ImageMaskSpatialObjectType::New();
        movingMask->SetImage( castMovingMaskImage->GetOutput() );
        movingMask->ComputeObjectToWorldTransform();

        actualfilter->SetFixedImageMask( dynamic_cast< SpatialObjectType * >( fixedMask.GetPointer() ) );
        actualfilter->SetMovingImageMask( dynamic_cast< SpatialObjectType * >( movingMask.GetPointer() ) );
        }
      else if ( command.maskProcessingMode == "ROI" )
        {
        if ( ( command.fixedBinaryVolume == "" )
             || ( command.movingBinaryVolume == "" ) )
          {
          std::cout
          <<
          "ERROR:  Must specify mask file names when ROI is used for the maskProcessingMode"
          << std::endl;
          exit(-1);
          }
        std::cout << "Diffeomorphic with Mask!!!!!!!" << std::endl;
        typename  TRealImage::Pointer fixedVolume =
          itkUtil::ReadImage< TRealImage >( command.fixedVolume.c_str () );
        typename  TRealImage::Pointer movingVolume =
          itkUtil::ReadImage< TRealImage >( command.movingVolume.c_str () );

        SpatialObjectType::Pointer fixedMask = ReadImageMask< SpatialObjectType, dims >(
          command.fixedBinaryVolume,
          fixedVolume);
        SpatialObjectType::Pointer movingMask = ReadImageMask< SpatialObjectType, dims >(
          command.movingBinaryVolume,
          movingVolume);
        actualfilter->SetFixedImageMask(fixedMask);
        actualfilter->SetMovingImageMask(movingMask);
        }
      filter = actualfilter;
      }
    else if ( command.registrationFilterType == "FastSymmetricForces" )
      {
      // s <- s + u (ITK basic implementation)
      typedef typename itk::FastSymmetricForcesDemonsRegistrationFilter<
        TRealImage, TRealImage, TDeformationField > ActualRegistrationFilterType;
      typedef typename ActualRegistrationFilterType::GradientType GradientType;
      typename ActualRegistrationFilterType::Pointer actualfilter =
        ActualRegistrationFilterType::New();
      // TODO:  Review this value setting.
      actualfilter->SetMaximumUpdateStepLength(command.maxStepLength);
      actualfilter->SetUseGradientType( static_cast< GradientType >( command.
                                                                     gradientType ) );
      filter = actualfilter;
      }
    /*
    else if(command.registrationFilterType == "UseFirstOrderExpOn Diffeomorphic Registration")
    {
    //TODO:  Review this value setting with Insight Journal Diffeomorphic implementation.
    // s <- s o (Id + u) (Diffeomorphic demons)
    // This is simply a crude diffeomorphic demons
    // where the exponential is computed in 0 iteration
    typedef typename itk::DiffeomorphicDemonsRegistrationFilter  < TRealImage, TRealImage, TDeformationField>
    ActualRegistrationFilterType;
    typedef typename ActualRegistrationFilterType::GradientType GradientType;
    ActualRegistrationFilterType::Pointer actualfilter = ActualRegistrationFilterType::New();
    //TODO:  HACK: Make sure that MaxLength and GradientTypes are set.
    actualfilter->SetMaximumUpdateStepLength( command.maxStepLength );
    actualfilter->SetUseGradientType( static_cast<GradientType>(command.gradientType) );
    actualfilter->UseFirstOrderExpOn();
    filter = actualfilter;
    }
    */
    else
      {
      std::cerr << "Unknown Registration Filter type: "
                << command.registrationFilterType << std::endl;
      std::cerr.flush();
      throw;
      }

    // TODO:  Review this value setting with Insight Journal Diffeomorphic
    // implementation.
    if ( command.smoothDeformationFieldSigma > 0.1 )
      {
      if ( command.outputDebug )
        {
        std::cout << " Smoothing is on ....." << std::endl;
        }
      filter->SmoothDeformationFieldOn();
      filter->SetStandardDeviations(command.smoothDeformationFieldSigma);
      }
    else
      {
      filter->SmoothDeformationFieldOff();
      }
    if ( command.smoothingUp > 0.1 )
      {
      if ( command.outputDebug )
        {
        std::cout << " Smoothing at update....." << std::endl;
        }
      filter->SmoothUpdateFieldOn();
      filter->SetUpdateFieldStandardDeviations(command.smoothingUp);
      }
    else
      {
      filter->SmoothUpdateFieldOff();
      }
    if ( command.outputDebug )
      {
      filter->AddObserver(itk::IterationEvent(), observer);
      }

    app->SetRegistrationFilter(filter);
    }
  /*NOT YET IMPLEMENTED
    if ( command.fixedLandmarks != "none"
      && command.fixedLandmarks != ""
      && command.movingLandmarks != "none"
      && command.movingLandmarks != "" )
      {
      app->SetMovingLandmarkFilename(command.movingLandmarks);
      app->SetFixedLandmarkFilename(command.fixedLandmarks);
    if ( command.initializeWithFourier != "" )
      {
      app->SetInitialCoefficientFilename( command.initializeWithFourier.c_str () );
      }
      }
      */
  if ( command.initializeWithDeformationField != "" )
    {
    app->SetInitialDeformationFieldFilename(
      command.initializeWithDeformationField.c_str () );
    }
  if ( command.initializeWithTransform != "" )
    {
    app->SetInitialTransformFilename( command.initializeWithTransform.c_str () );
    }

  app->SetTheMovingImageFilename ( command.movingVolume.c_str () );
  app->SetTheFixedImageFilename ( command.fixedVolume.c_str () );
  if ( command.outputDebug )
    {
    typename  TRealImage::Pointer fixedVolume =
      itkUtil::ReadImage< TRealImage >( command.fixedVolume.c_str () );
    typename  TRealImage::Pointer movingVolume =
      itkUtil::ReadImage< TRealImage >( command.movingVolume.c_str () );
    observer->SetMovingImage(movingVolume);
    observer->SetFixedImage(fixedVolume);
    }
  app->SetWarpedImageName ( command.outputVolume.c_str () );
  app->SetMedianFilterSize(command.medianFilterSize);

  // Set the other optional arguments if specified by the user.
  if ( command.outputDisplacementFieldPrefix != "" )
    {
    app->SetDisplacementBaseName ( command.outputDisplacementFieldPrefix.c_str () );
    }
  if ( command.outputDeformationFieldVolume != "" )
    {
    app->SetDeformationFieldOutputName (
      command.outputDeformationFieldVolume.c_str () );
    }

  if ( command.outputCheckerboardVolume != "" )
    {
    app->SetCheckerBoardFilename ( command.outputCheckerboardVolume.c_str () );
    unsigned int array[3] = {
      command.checkerboardPatternSubdivisions[0],
      command.checkerboardPatternSubdivisions[1],
      command.checkerboardPatternSubdivisions[2]
      };
    app->SetCheckerBoardPattern (array);
    }

  if ( command.outputNormalized )
    {
    std::string normalize = "ON";   // TODO:  SetOutNormalized should be a
                                    // boolean
                                    // not a string.
    app->SetOutNormalized ( normalize.c_str () );
    }

  if ( command.outputDebug )
    {
    bool debug = true;
    app->SetOutDebug (debug);   // TODO:  SetOutDebug should be a boolean not a
                                // string.
    }

  app->SetTheMovingImageShrinkFactors(command.theMovingImageShrinkFactors);
  app->SetTheFixedImageShrinkFactors(command.theFixedImageShrinkFactors);

  app->SetUseHistogramMatching(command.histogramMatch);
  if ( app->GetUseHistogramMatching() )
    {
    if ( command.outputDebug )
      {
      std::cout << " Use Histogram Matching....." << std::endl;
      }
    app->SetNumberOfHistogramLevels(command.numberOfHistogramLevels);
    app->SetNumberOfMatchPoints(command.numberOfMatchPoints);
    }

  app->SetNumberOfLevels(command.numberOfLevels);
  app->SetNumberOfIterations(command.numberOfIterations);
  app->SetInterpolationMode(command.interpolationMode);

  if ( ( command.maskProcessingMode == "NOMASK" )
       && ( ( command.fixedBinaryVolume != "" )
            || ( command.movingBinaryVolume != "" ) ) )
    {
    std::cout
    <<
    "ERROR:  Can not specify mask file names when the default of NOMASK is used for the maskProcessingMode"
    << std::endl;
    exit(-1);
    }
  // If making BOBF option is specified Initialize its parameters
  if ( command.maskProcessingMode == "BOBF" )
    {
    if ( ( command.fixedBinaryVolume == "" )
         || ( command.movingBinaryVolume == "" ) )
      {
      std::cout
      <<
      "Error: If BOBF option is set for maskProcessingMode then the fixed mask name and moving mask file name should be specified. \n";
      exit(-1);
      }

    app->SetFixedBinaryVolume ( command.fixedBinaryVolume.c_str () );
    app->SetMovingBinaryVolume ( command.movingBinaryVolume.c_str () );
    app->SetLower (command.lowerThresholdForBOBF);
    app->SetUpper (command.upperThresholdForBOBF);
    typename ImageType::SizeType radius;
    radius[0] = command.neighborhoodForBOBF[0];   // Radius along X
    radius[1] = command.neighborhoodForBOBF[1];   // Radius along Y
    radius[2] = command.neighborhoodForBOBF[2];   // Radius along Z
    app->SetRadius (radius);
    typename ImageType::IndexType seed;
    seed[0] = command.seedForBOBF[0];   // Seed in X dimension;
    seed[1] = command.seedForBOBF[1];   // Seed in Y dimension;
    seed[2] = command.seedForBOBF[2];   // Seed in Z dimension;
    app->SetSeed (seed);
    }
  if ( command.outputDebug )
    {
    std::cout << "Setting Default PixelValue: "
              << command.backgroundFillValue << "." << std::endl;
    }
  app->SetDefaultPixelValue (command.backgroundFillValue);
  if ( command.outputDebug )
    {
    std::cout << "Running Thirion Registration" << std::endl;
    }
  try
    {
    app->Execute ();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Caught an ITK exception: " << std::endl;
    std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
    throw err;
    }
  catch ( ... )
    {
    std::
    cout << "Caught a non-ITK exception " << __FILE__ << " " << __LINE__
         << std::endl;
    }

  return;
}

// This function calls the Thirion registration filter setting all the
// parameters.
template< class InPixelType, class OutPixelType >
void ProcessAppType(const struct BRAINSDemonWarpAppParameters & command)
{
  ThirionFunction< InPixelType, OutPixelType >(command);
}

// This function processes the output data type.
template< class PixelType >
void
ProcessOutputType(struct BRAINSDemonWarpAppParameters & command)
{
  if ( command.outputPixelType != "" )
    {
    // process the string for the data type
    if ( CompareNoCase ( command.outputPixelType, std::string ("uchar") ) == 0 )
      {
      ProcessAppType< PixelType, unsigned char >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("short") ) == 0 )
      {
      ProcessAppType< PixelType, short >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("ushort") ) == 0 )
      {
      ProcessAppType< PixelType, unsigned short >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("int") ) == 0 )
      {
      ProcessAppType< PixelType, int >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("float") ) == 0 )
      {
      ProcessAppType< PixelType, float >(command);
      }
#ifdef _USE_UNCOMMON_TYPES  // This is commented out because it causes too many
                            // segments in one object file for the intel
                            // compiler
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("uint") ) == 0 )
      {
      ProcessAppType< PixelType, unsigned int >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("double") ) == 0 )
      {
      ProcessAppType< PixelType, double >(command);
      }
#endif
    else
      {
      std::cout
      << "Error. Invalid data type for -outtype!  Use one of these:"
      << std::endl;
      PrintDataTypeStrings ();
      exit (-1);
      }
    }
  else
    {
    ProcessAppType< PixelType, float >(command);
    }
}

template< class InPixelType, class OutPixelType >
void VectorThirionFunction(const struct BRAINSDemonWarpAppParameters & command)
{
  const int dims = 3;

  typedef itk::Image< InPixelType, dims >                ImageType;
  typedef itk::Image< float, dims >                      TRealImage;
  typedef itk::VectorImage< float, dims >                TVectorImage;
  typedef itk::Image< OutPixelType, dims >               OutputImageType;
  typedef itk::Image< itk::Vector< float, dims >, dims > TDeformationField;
  //
  // If optional landmark files given, will use landmark registration to
  // generate
  // a deformation field to prime the thirion demons registration.

  typedef typename itk::VBRAINSDemonWarp
  < ImageType,
    TRealImage,
    OutputImageType
  > AppType;
  typename  AppType::Pointer app = AppType::New ();

  // Set up the demons filter
  typedef typename itk::PDEDeformableRegistrationFilter< TRealImage, TRealImage,
                                                         TDeformationField >
  BaseRegistrationFilterType;
  // BaseRegistrationFilterType::Pointer filter =
  //   BaseRegistrationFilterType::New();
  typename BaseRegistrationFilterType::Pointer filter;
  if ( command.outputDebug )
    {
    std::cout << command.registrationFilterType
              << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
              << std::endl;
    }

  if ( command.registrationFilterType == "Demons" )
    {
    if ( command.vectorMovingVolume.size() == 1 )
      {
      typedef typename itk::DemonsRegistrationFilter< TRealImage, TRealImage,
                                                      TDeformationField >
      ActualRegistrationFilterType;
      ActualRegistrationFilterType::Pointer actualfilter =
        ActualRegistrationFilterType::New();
      filter = actualfilter;
      }
    else
      {
      std::cout << "Thirion demons does not support multi-input images!"
                << std::endl;
      exit(-1);
      }
    }

  else if ( command.registrationFilterType == "Diffeomorphic" )
    {
    //    std::cout << "Use Diffeomorphic Registration" << std::endl;
    if ( command.vectorMovingVolume.size() == 1 )
      {
      typedef typename itk::DiffeomorphicDemonsRegistrationFilter< TRealImage,
                                                                   TRealImage,
                                                                   TDeformationField >
      ActualRegistrationFilterType;
      typedef  typename ActualRegistrationFilterType::GradientType GradientType;
      typename ActualRegistrationFilterType::Pointer actualfilter =
        ActualRegistrationFilterType::New();
      // TODO:  Review this value setting with Insight Journal Diffeomorphic
      // implementation.
      actualfilter->SetMaximumUpdateStepLength(command.maxStepLength);
      actualfilter->SetUseGradientType( static_cast< GradientType >( command.
                                                                     gradientType ) );
      filter = actualfilter;
      }
    else
      {
      typedef typename itk::VectorDiffeomorphicDemonsRegistrationFilter<
        TVectorImage, TVectorImage, TDeformationField >
      ActualRegistrationFilterType;
      typedef  typename ActualRegistrationFilterType::GradientType GradientType;
      typename ActualRegistrationFilterType::Pointer VDDfilter =
        ActualRegistrationFilterType::New();
      // TODO:  Review this value setting with Insight Journal Diffeomorphic
      // implementation.
      VDDfilter->SetMaximumUpdateStepLength(command.maxStepLength);
      VDDfilter->SetUseGradientType( static_cast< GradientType >( command.
                                                                  gradientType ) );
      if ( command.smoothDeformationFieldSigma > 0.1 )
        {
        if ( command.outputDebug )
          {
          std::cout << " Smoothing is on ....." << std::endl;
          }
        VDDfilter->SmoothDeformationFieldOn();
        VDDfilter->SetStandardDeviations(command.smoothDeformationFieldSigma);
        }
      else
        {
        VDDfilter->SmoothDeformationFieldOff();
        }
      if ( command.smoothingUp > 0.1 )
        {
        if ( command.outputDebug )
          {
          std::cout << " Smoothing at update....." << std::endl;
          }
        VDDfilter->SmoothUpdateFieldOn();
        VDDfilter->SetUpdateFieldStandardDeviations(command.smoothingUp);
        }
      else
        {
        VDDfilter->SmoothUpdateFieldOff();
        }
      if ( command.outputDebug )
        {
        typename VCommandIterationUpdate< float,
                                          3 >::Pointer observer =
          VCommandIterationUpdate< float, 3 >::New();
        VDDfilter->AddObserver(itk::IterationEvent(), observer);
        }

      app->SetVectorRegistrationFilter(VDDfilter);
      }
    }
  else if ( command.registrationFilterType == "FastSymmetricForces" )
    {
    // s <- s + u (ITK basic implementation)
    if ( command.vectorMovingVolume.size() == 1 )
      {
      typedef typename itk::FastSymmetricForcesDemonsRegistrationFilter<
        TRealImage, TRealImage, TDeformationField > ActualRegistrationFilterType;
      typedef typename ActualRegistrationFilterType::GradientType GradientType;
      typename ActualRegistrationFilterType::Pointer actualfilter =
        ActualRegistrationFilterType::New();
      // TODO:  Review this value setting.
      actualfilter->SetMaximumUpdateStepLength(command.maxStepLength);
      actualfilter->SetUseGradientType( static_cast< GradientType >( command.
                                                                     gradientType ) );
      filter = actualfilter;
      }
    else
      {
      std::cout
      << "FastSymmetricForces demons does not support multi-input images!"
      << std::endl;
      exit(-1);
      }
    }
  else
    {
    std::cerr << "Unknown Registration Filter type: "
              << command.registrationFilterType << std::endl;
    std::cerr.flush();
    throw;
    }

  // TODO:  Review this value setting with Insight Journal Diffeomorphic
  // implementation.
  if ( command.vectorMovingVolume.size() == 1 )
    {
    if ( command.smoothDeformationFieldSigma > 0.1 )
      {
      if ( command.outputDebug )
        {
        std::cout << " Smoothing is on ....." << std::endl;
        }
      filter->SmoothDeformationFieldOn();
      filter->SetStandardDeviations(command.smoothDeformationFieldSigma);
      }
    else
      {
      filter->SmoothDeformationFieldOff();
      }
    if ( command.smoothingUp > 0.1 )
      {
      if ( command.outputDebug )
        {
        std::cout << " Smoothing at update....." << std::endl;
        }
      filter->SmoothUpdateFieldOn();
      filter->SetUpdateFieldStandardDeviations(command.smoothingUp);
      }
    else
      {
      filter->SmoothUpdateFieldOff();
      }
    if ( command.outputDebug )
      {
      typename VCommandIterationUpdate< float,
                                        3 >::Pointer observer =
        VCommandIterationUpdate< float, 3 >::New();
      filter->AddObserver(itk::IterationEvent(), observer);
      }

    app->SetRegistrationFilter(filter);
    }

  /* NOT YET IMPLEMENTED
  if ( command.fixedLandmarks != "none"
    && command.fixedLandmarks != ""
    && command.movingLandmarks != "none"
    && command.movingLandmarks != "" )
    {
    app->SetMovingLandmarkFilename(command.movingLandmarks);
    app->SetFixedLandmarkFilename(command.fixedLandmarks);
    }
  if ( command.initializeWithFourier != "" )
    {
    app->SetInitialCoefficientFilename( command.initializeWithFourier.c_str () );
    }
   app->SetForceCoronalZeroOrigin (command.forceCoronalZeroOrigin);
*/
  if ( command.initializeWithDeformationField != "" )
    {
    app->SetInitialDeformationFieldFilename(
      command.initializeWithDeformationField.c_str () );
    }
  if ( command.initializeWithTransform != "" )
    {
    app->SetInitialTransformFilename( command.initializeWithTransform.c_str () );
    }

  std::vector< std::string > fixedImageNames = command.vectorFixedVolume;
  std::vector< std::string > movingImageNames = command.vectorMovingVolume;

  app->SetTheFixedImageFilename(fixedImageNames);
  app->SetTheMovingImageFilename(movingImageNames);
  app->SetWarpedImageName ( command.outputVolume.c_str () );
  app->SetInterpolationMode(command.interpolationMode);
  app->SetMedianFilterSize(command.medianFilterSize);

  // Set the other optional arguments if specified by the user.
  if ( command.outputDisplacementFieldPrefix != "" )
    {
    app->SetDisplacementBaseName ( command.outputDisplacementFieldPrefix.c_str () );
    }
  if ( command.outputDeformationFieldVolume != "" )
    {
    app->SetDeformationFieldOutputName (
      command.outputDeformationFieldVolume.c_str () );
    }

  if ( command.outputCheckerboardVolume != "" )
    {
    app->SetCheckerBoardFilename ( command.outputCheckerboardVolume.c_str () );
    unsigned int array[3] = {
      command.checkerboardPatternSubdivisions[0],
      command.checkerboardPatternSubdivisions[1],
      command.checkerboardPatternSubdivisions[2]
      };
    app->SetCheckerBoardPattern (array);
    }

  if ( command.outputNormalized )
    {
    std::string normalize = "ON";   // TODO:  SetOutNormalized should be a
                                    // boolean
                                    // not a string.
    app->SetOutNormalized ( normalize.c_str () );
    }

  if ( command.outputDebug )
    {
    bool debug = true;
    app->SetOutDebug (debug);   // TODO:  SetOutDebug should be a boolean not a
                                // string.
    }

  app->SetTheMovingImageShrinkFactors(command.theMovingImageShrinkFactors);
  app->SetTheFixedImageShrinkFactors(command.theFixedImageShrinkFactors);

  app->SetUseHistogramMatching(command.histogramMatch);
  if ( app->GetUseHistogramMatching() )
    {
    if ( command.outputDebug )
      {
      std::cout << " Use Histogram Matching....." << std::endl;
      }
    app->SetNumberOfHistogramLevels(command.numberOfHistogramLevels);
    app->SetNumberOfMatchPoints(command.numberOfMatchPoints);
    }

  app->SetNumberOfLevels(command.numberOfLevels);
  app->SetNumberOfIterations(command.numberOfIterations);
  app->SetInterpolationMode(command.interpolationMode);

  app->SetWeightFactors(command.weightFactors);

  // If making BOBF option is specified Initialize its parameters
  if ( command.makeBOBF )
    {
    if ( ( command.fixedBinaryVolume == "" )
         || ( command.movingBinaryVolume == "" ) )
      {
      std::cout
      <<
      "Error: If BOBF option is set then the fixed mask name and moving mask file name should be specified. \n";
      exit(-1);
      }

    app->SetFixedBinaryVolume ( command.fixedBinaryVolume.c_str () );
    app->SetMovingBinaryVolume ( command.movingBinaryVolume.c_str () );
    app->SetLower (command.lowerThresholdForBOBF);
    app->SetUpper (command.upperThresholdForBOBF);
    typename ImageType::SizeType radius;
    radius[0] = command.neighborhoodForBOBF[0];   // Radius along X
    radius[1] = command.neighborhoodForBOBF[1];   // Radius along Y
    radius[2] = command.neighborhoodForBOBF[2];   // Radius along Z
    app->SetRadius (radius);
    typename ImageType::IndexType seed;
    seed[0] = command.seedForBOBF[0];   // Seed in X dimension;
    seed[1] = command.seedForBOBF[1];   // Seed in Y dimension;
    seed[2] = command.seedForBOBF[2];   // Seed in Z dimension;
    app->SetSeed (seed);
    }
  if ( command.outputDebug )
    {
    std::cout << "Setting Default PixelValue: "
              << command.backgroundFillValue << "." << std::endl;
    }
  app->SetDefaultPixelValue (command.backgroundFillValue);
  if ( command.outputDebug )
    {
    std::cout << "Running Thirion Registration" << std::endl;
    }
  try
    {
    app->Execute ();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Caught an ITK exception: " << std::endl;
    std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
    throw err;
    }
  catch ( ... )
    {
    std::cout << "Caught a non-ITK exception " << __FILE__ << " "
              << __LINE__ << std::endl;
    }

  return;
}

// This function calls the Thirion registration filter setting all the
// parameters.
template< class InPixelType, class OutPixelType >
void VectorProcessAppType(const struct BRAINSDemonWarpAppParameters & command)
{
  VectorThirionFunction< InPixelType, OutPixelType >(command);
}

// This function processes the output data type.
template< class PixelType >
void
VectorProcssOutputType(struct BRAINSDemonWarpAppParameters & command)
{
  if ( command.outputPixelType != "" )
    {
    // process the string for the data type
    if ( CompareNoCase ( command.outputPixelType, std::string ("uchar") ) == 0 )
      {
      VectorProcessAppType< PixelType, unsigned char >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType, std::string ("short") ) == 0 )
      {
      VectorProcessAppType< PixelType, short >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType, std::string ("ushort") ) == 0 )
      {
      VectorProcessAppType< PixelType, unsigned short >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType, std::string ("int") ) == 0 )
      {
      VectorProcessAppType< PixelType, int >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType, std::string ("float") ) == 0 )
      {
      VectorProcessAppType< PixelType, float >(command);
      }
#ifdef _USE_UNCOMMON_TYPES  // This is commented out because it causes too many
                            // segments in one object file for the intel
                            // compiler
    else if ( CompareNoCase ( command.outputPixelType, std::string ("uint") ) == 0 )
      {
      VectorProcessAppType< PixelType, unsigned int >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType, std::string ("double") ) == 0 )
      {
      VectorProcessAppType< PixelType, double >(command);
      }
#endif
    else
      {
      std::cout
      << "Error. Invalid data type for -outtype!  Use one of these:"
      << std::endl;
      PrintDataTypeStrings ();
      exit (-1);
      }
    }
  else
    {
    VectorProcessAppType< PixelType, float >(command);
    }
}

// This function processes the output data type.
template< class PixelType >
void
VectorProcessOutputType(struct BRAINSDemonWarpAppParameters & command)
{
  if ( command.outputPixelType != "" )
    {
    // process the string for the data type
    if ( CompareNoCase ( command.outputPixelType, std::string ("uchar") ) == 0 )
      {
      VectorProcessAppType< PixelType, unsigned char >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("short") ) == 0 )
      {
      VectorProcessAppType< PixelType, short >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("ushort") ) == 0 )
      {
      VectorProcessAppType< PixelType, unsigned short >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("int") ) == 0 )
      {
      VectorProcessAppType< PixelType, int >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("float") ) == 0 )
      {
      VectorProcessAppType< PixelType, float >(command);
      }
#ifdef _USE_UNCOMMON_TYPES  // This is commented out because it causes too many
                            // segments in one object file for the intel
                            // compiler
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("uint") ) == 0 )
      {
      VectorProcessAppType< PixelType, unsigned int >(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("double") ) == 0 )
      {
      VectorProcessAppType< PixelType, double >(command);
      }
#endif
    else
      {
      std::cout
      << "Error. Invalid data type for -outtype!  Use one of these:"
      << std::endl;
      PrintDataTypeStrings ();
      exit (-1);
      }
    }
  else
    {
    VectorProcessAppType< PixelType, float >(command);
    }
}

#endif // __BRAINSDemonWarpTemplates_h
