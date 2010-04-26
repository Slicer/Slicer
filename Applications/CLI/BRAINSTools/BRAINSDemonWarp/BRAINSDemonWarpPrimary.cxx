#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include "itkImage.h"
#include "itkIndex.h"
#include "itkSize.h"
#include "itkExceptionObject.h"
//#include "itkBrains2MaskImageIOFactory.h"
#include "GenericTransformImage.h"
#include "BRAINSDemonWarp.h"
#include "BRAINSDemonWarpPrimaryCLP.h"

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
#include "itkSymmetricLogDomainDemonsRegistrationFilter.h"
#include "itkESMDemonsRegistrationWithMaskFunction.h"
#include "itkArray.h"
#include "itkIO.h"
#ifdef USE_DEBUG_IMAGE_VIEWER
#include "DebugImageViewerClient.h"
#endif

#include "itkLargestForegroundFilledMaskImageFilter.h"

#include "commandIterationupdate.h"
#include "ReadMask.h"
#include "itkImageMaskSpatialObject.h"


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
  itk::Index<3> checkerboardPatternSubdivisions;
  bool forceCoronalZeroOrigin;
  bool outputNormalized;
  bool outputDebug;
  std::string maskProcessingMode;
  std::string fixedBinaryVolume;
  std::string movingBinaryVolume;
  int lowerThresholdForBOBF;
  int upperThresholdForBOBF;
  int backgroundFillValue;
  itk::Index<3> seedForBOBF;
  itk::Index<3> neighborhoodForBOBF;
  itk::Size<3> medianFilterSize;
  std::string movingLandmarks;
  std::string fixedLandmarks;
  std::string initializeWithDeformationField;
  std::string initializeWithFourier;
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
  typedef itk::FixedArray<unsigned int, 3> ShrinkFactorsType;

  /** IterationArray type. */
  typedef itk::Array<unsigned int> IterationsArrayType;
  unsigned long numberOfHistogramLevels;
  unsigned long numberOfMatchPoints;
  unsigned short numberOfLevels;
  ShrinkFactorsType theMovingImageShrinkFactors;
  ShrinkFactorsType theFixedImageShrinkFactors;
  IterationsArrayType numberOfIterations;
};

// This function prints the valid pixel types.
void PrintDataTypeStrings(void)
{
  // Prints the Input and output data type strings.
  std::cout << "uchar" << std::endl;
  std::cout << "short" << std::endl;
  std::cout << "ushort" << std::endl;
  std::cout << "int" << std::endl;
  std::cout << "float" << std::endl;
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes
  // too many segments in one object file for the intel compiler
  std::cout << "uint" << std::endl;
  std::cout << "double" << std::endl;
#endif
}

// This function compares strings ignoring case.
int CompareNoCase( const std::string & s, const std::string & s2 )
{
  // Compare strings.
  std::string::const_iterator p = s.begin();
  std::string::const_iterator p2 = s2.begin();

  while ( p != s.end() && p2 != s2.end() )
    {
    if ( toupper(*p) != toupper(*p2) )
      {
      return ( toupper(*p) < toupper(*p2) ) ? -1 : 1;
      }
    p++;
    p2++;
    }

  return ( s2.size() == s.size() ) ? 0 : ( s.size() < s2.size() ) ? -1 : 1;
}

// This function calls the Thirion registration filter setting all the
// parameters.
template <class InPixelType, class OutPixelType>
void ThirionFunction (const struct BRAINSDemonWarpAppParameters & command)
{
  const int dims = 3;

  typedef itk::Image<InPixelType, dims>              ImageType;
  typedef itk::Image<float, dims>                    TRealImage;
  typedef itk::Image<OutPixelType, dims>             OutputImageType;
  typedef itk::Image<itk::Vector<float, dims>, dims> TDeformationField;
  typedef itk::Image<itk::Vector<float, dims>, dims> TVelocityField;

  typedef unsigned char                     MaskPixelType;
  typedef itk::Image<MaskPixelType, dims>   MaskImageType;
  typedef itk::CastImageFilter<TRealImage, MaskImageType> CastImageFilter;

  typedef itk::ImageMaskSpatialObject<dims> ImageMaskSpatialObjectType;
  typedef itk::SpatialObject<GenericTransformImageNS::SpaceDimension>  SpatialObjectType;

  //
  // If optional landmark files given, will use landmark registration to
  // generate
  // a deformation field to prime the thirion demons registration.
  // Need to explicitly register the B2MaskIOFactory
  itk::Brains2MaskImageIOFactory::RegisterOneFactory ();

  typedef typename itk::BRAINSDemonWarp
    <ImageType,
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

  typename CommandIterationUpdate<float,3>::Pointer observer;
  if ( command.outputDebug )
    {
     observer = CommandIterationUpdate<float, 3>::New();
    }
  if(command.registrationFilterType == "SymmetricLogDemons" || command.registrationFilterType == "LogDemons")
    {
    app->SetLogDomain(true);
    typedef typename itk::LogDomainDeformableRegistrationFilter<TRealImage,TRealImage, TVelocityField>
      BaseRegistrationFilterType;
    typename BaseRegistrationFilterType::Pointer filter;

    if(command.registrationFilterType == "LogDemons")
      {
      typedef typename itk::LogDomainDemonsRegistrationFilter < TRealImage, TRealImage, TVelocityField>
        ActualRegistrationFilterType;
      typedef typename ActualRegistrationFilterType::GradientType GradientType;

      typename ActualRegistrationFilterType::Pointer actualfilter
        = ActualRegistrationFilterType::New();

      actualfilter->SetMaximumUpdateStepLength( command.maxStepLength );
      actualfilter->SetUseGradientType(static_cast<GradientType>(command.gradientType) );
      actualfilter->SetNumberOfBCHApproximationTerms(command.numberOfBCHApproximationTerms);
      filter = actualfilter;
      }
    if(command.registrationFilterType == "SymmetricLogDemons")
      {
      typedef typename itk::SymmetricLogDomainDemonsRegistrationFilter<TRealImage,TRealImage, TVelocityField>
        ActualRegistrationFilterType;
      typename ActualRegistrationFilterType::Pointer actualfilter = ActualRegistrationFilterType::New();
      typedef  typename ActualRegistrationFilterType::GradientType GradientType;

      actualfilter->SetMaximumUpdateStepLength( command.maxStepLength );
      actualfilter->SetUseGradientType(static_cast<GradientType>(command.gradientType) );
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
      filter->SetStandardDeviations( command.smoothDeformationFieldSigma );
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
      filter->SetUpdateFieldStandardDeviations( command.smoothingUp );
      }
    else
      {
      filter->SmoothUpdateFieldOff();
      }
    if ( command.outputDebug )
      {
      filter->AddObserver( itk::IterationEvent(), observer );
      }

    app->SetLDDRegistrationFilter(filter);
    }

  else
    {
    // Set up the demons filter
    typedef typename itk::PDEDeformableRegistrationFilter<TRealImage, TRealImage,
      TDeformationField>
      BaseRegistrationFilterType;
    // BaseRegistrationFilterType::Pointer filter =
    //   BaseRegistrationFilterType::New();
    typename BaseRegistrationFilterType::Pointer filter;

    if ( command.registrationFilterType == "Demons" )
      {
      typedef typename itk::DemonsRegistrationFilter<TRealImage, TRealImage,
        TDeformationField>
        ActualRegistrationFilterType;
      ActualRegistrationFilterType::Pointer actualfilter
        = ActualRegistrationFilterType::New();
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
        TDeformationField>   ActualRegistrationFilterType;
      typename ActualRegistrationFilterType::Pointer actualfilter
        =   ActualRegistrationFilterType::New();

      typedef  typename ActualRegistrationFilterType::GradientType GradientType;
      actualfilter->SetMaximumUpdateStepLength(command.maxStepLength );
      actualfilter->SetUseGradientType( static_cast<GradientType>( command.
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

        typename  TRealImage::Pointer fixedVolume
          = itkUtil::ReadImage<TRealImage>( command.fixedVolume.c_str () );
        //       fixedBinaryVolumeImage = FindLargestForgroundFilledMask<TRealImage>(
        //       fixedVolume,
        //       otsuPercentileThreshold,
        //       closingSize);
        typedef itk::LargestForegroundFilledMaskImageFilter<TRealImage> LFFMaskFilterType;
        LFFMaskFilterType::Pointer LFF = LFFMaskFilterType::New();
        LFF->SetInput(fixedVolume);
        LFF->SetOtsuPercentileThreshold(otsuPercentileThreshold);
        LFF->SetClosingSize(closingSize);
        LFF->Update();
        fixedBinaryVolumeImage = LFF->GetOutput();

        typename CastImageFilter::Pointer castFixedMaskImage = CastImageFilter::New();
        castFixedMaskImage->SetInput( fixedBinaryVolumeImage );
        castFixedMaskImage->Update( );

        //      typename MaskImageType::Pointer fm = castFixedMaskImage->GetOutput();
        //      itkUtil::WriteImage<MaskImageType>(fm, "fixedMaskImage.nii.gz");

        // convert mask image to mask
        typename ImageMaskSpatialObjectType::Pointer fixedMask = ImageMaskSpatialObjectType::New();
        fixedMask->SetImage( castFixedMaskImage->GetOutput() );
        fixedMask->ComputeObjectToWorldTransform();

        typename  TRealImage::Pointer movingVolume
          = itkUtil::ReadImage<TRealImage>( command.movingVolume.c_str () );
        //         movingBinaryVolumeImage = FindLargestForgroundFilledMask<TRealImage>(
        //         movingVolume,
        //         otsuPercentileThreshold,
        //         closingSize);
        LFF->SetInput(movingVolume);
        LFF->SetOtsuPercentileThreshold(otsuPercentileThreshold);
        LFF->SetClosingSize(closingSize);
        LFF->Update();
        movingBinaryVolumeImage = LFF->GetOutput();

        typename CastImageFilter::Pointer castMovingMaskImage = CastImageFilter::New();
        castMovingMaskImage->SetInput( movingBinaryVolumeImage );
        castMovingMaskImage->Update( );

        //      typename MaskImageType::Pointer mm = castMovingMaskImage->GetOutput();
        //      itkUtil::WriteImage<MaskImageType>(mm, "movingMaskImage.nii.gz");

        // convert mask image to mask
        typename ImageMaskSpatialObjectType::Pointer movingMask = ImageMaskSpatialObjectType::New();
        movingMask->SetImage( castMovingMaskImage->GetOutput() );
        movingMask->ComputeObjectToWorldTransform();

        actualfilter->SetFixedImageMask(dynamic_cast< SpatialObjectType * >( fixedMask.GetPointer()));
        actualfilter->SetMovingImageMask(dynamic_cast< SpatialObjectType * >( movingMask.GetPointer()));
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
        typename  TRealImage::Pointer fixedVolume
          = itkUtil::ReadImage<TRealImage>( command.fixedVolume.c_str () );
        typename  TRealImage::Pointer movingVolume
          = itkUtil::ReadImage<TRealImage>( command.movingVolume.c_str () );

        SpatialObjectType::Pointer fixedMask = ReadImageMask<SpatialObjectType, dims>(
                                                                                      command.fixedBinaryVolume,
                                                                                      fixedVolume);
        SpatialObjectType::Pointer movingMask = ReadImageMask<SpatialObjectType, dims>(
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
      TRealImage, TRealImage, TDeformationField> ActualRegistrationFilterType;
      typedef typename ActualRegistrationFilterType::GradientType GradientType;
      typename ActualRegistrationFilterType::Pointer actualfilter
        = ActualRegistrationFilterType::New();
      // TODO:  Review this value setting.
      actualfilter->SetMaximumUpdateStepLength( command.maxStepLength );
      actualfilter->SetUseGradientType( static_cast<GradientType>( command.
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
      filter->SetStandardDeviations( command.smoothDeformationFieldSigma );
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
      filter->SetUpdateFieldStandardDeviations( command.smoothingUp );
      }
    else
      {
      filter->SmoothUpdateFieldOff();
      }
    if ( command.outputDebug )
      {
      filter->AddObserver( itk::IterationEvent(), observer );
      }

    app->SetRegistrationFilter(filter);
    }

  if ( command.fixedLandmarks != "none"
       && command.fixedLandmarks != ""
       && command.movingLandmarks != "none"
       && command.movingLandmarks != "" )
    {
    app->SetMovingLandmarkFilename(command.movingLandmarks);
    app->SetFixedLandmarkFilename(command.fixedLandmarks);
    }
  if ( command.initializeWithDeformationField != "" )
    {
    app->SetInitialDeformationFieldFilename
      (command.initializeWithDeformationField.c_str () );
    }
  if ( command.initializeWithFourier != "" )
    {
    app->SetInitialCoefficientFilename(command.initializeWithFourier.c_str () );
    }
  if ( command.initializeWithTransform != "" )
    {
    app->SetInitialTransformFilename( command.initializeWithTransform.c_str () );
    }

  app->SetTheMovingImageFilename ( command.movingVolume.c_str () );
  app->SetTheFixedImageFilename ( command.fixedVolume.c_str () );
  if(command.outputDebug)
    {
    typename  TRealImage::Pointer fixedVolume
      = itkUtil::ReadImage<TRealImage>( command.fixedVolume.c_str () );
    typename  TRealImage::Pointer movingVolume
      = itkUtil::ReadImage<TRealImage>( command.movingVolume.c_str () );
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
    unsigned int array[3]
      = { command.checkerboardPatternSubdivisions[0],
          command.checkerboardPatternSubdivisions[1],
          command.checkerboardPatternSubdivisions[2]};
    app->SetCheckerBoardPattern (array);
    }

  app->SetForceCoronalZeroOrigin (command.forceCoronalZeroOrigin);
  if ( command.outputNormalized )
    {
    std::string normalize = "ON"; // TODO:  SetOutNormalized should be a boolean
                                  // not a string.
    app->SetOutNormalized ( normalize.c_str () );
    }

  if ( command.outputDebug )
    {
    bool debug = true;
    app->SetOutDebug (debug); // TODO:  SetOutDebug should be a boolean not a
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
    radius[0] = command.neighborhoodForBOBF[0]; // Radius along X
    radius[1] = command.neighborhoodForBOBF[1]; // Radius along Y
    radius[2] = command.neighborhoodForBOBF[2]; // Radius along Z
    app->SetRadius (radius);
    typename ImageType::IndexType seed;
    seed[0] = command.seedForBOBF[0]; // Seed in X dimension;
    seed[1] = command.seedForBOBF[1]; // Seed in Y dimension;
    seed[2] = command.seedForBOBF[2]; // Seed in Z dimension;
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
  catch (... )
    {
    std::
      cout << "Caught a non-ITK exception " << __FILE__ << " " << __LINE__
           << std::endl;
    }

  return;
}

// This function calls the Thirion registration filter setting all the
// parameters.
template <class InPixelType, class OutPixelType>
void ProcessAppType (const struct BRAINSDemonWarpAppParameters & command)
{
  ThirionFunction<InPixelType, OutPixelType>(command);
}

// This function processes the output data type.
template <class PixelType>
void
ProcessOutputType (struct BRAINSDemonWarpAppParameters & command)
{
  if ( command.outputPixelType != "" )
    {
    // process the string for the data type
    if ( CompareNoCase ( command.outputPixelType, std::string ("uchar") ) == 0 )
      {
      ProcessAppType<PixelType, unsigned char>(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("short") ) == 0 )
      {
      ProcessAppType<PixelType, short>(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("ushort") ) == 0 )
      {
      ProcessAppType<PixelType, unsigned short>(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("int") ) == 0 )
      {
      ProcessAppType<PixelType, int>(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("float") ) == 0 )
      {
      ProcessAppType<PixelType, float>(command);
      }
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes too many
                           // segments in one object file for the intel compiler
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("uint") ) == 0 )
      {
      ProcessAppType<PixelType, unsigned int>(command);
      }
    else if ( CompareNoCase ( command.outputPixelType,
                              std::string ("double") ) == 0 )
      {
      ProcessAppType<PixelType, double>(command);
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
    ProcessAppType<PixelType, float>(command);
    }
}

#ifdef USE_DEBUG_IMAGE_VIEWER
/*************************
 * Have a global variable to
 * add debugging information.
 */
DebugImageViewerClient DebugImageDisplaySender;
#endif

int BRAINSDemonWarpPrimary(int argc, char *argv[])
{
  // Apparently when you register one transform, you need to register all your
  // transforms.
  //
  itk::TransformFactory<VersorRigid3DTransformType>::RegisterTransform();
  itk::TransformFactory<ScaleVersor3DTransformType>::RegisterTransform();
  itk::TransformFactory<ScaleSkewVersor3DTransformType>::RegisterTransform();
  itk::TransformFactory<AffineTransformType>::RegisterTransform();
  itk::TransformFactory<BSplineTransformType>::RegisterTransform();
  struct BRAINSDemonWarpAppParameters command;

  {
  PARSE_ARGS;
#ifdef USE_DEBUG_IMAGE_VIEWER
  DebugImageDisplaySender.SetEnabled(UseDebugImageViewer);
  DebugImageDisplaySender.SetPromptUser(PromptAfterImageSend);
#endif

  command.registrationFilterType = registrationFilterType;
  command.movingVolume = movingVolume;
  command.fixedVolume = fixedVolume;
  command.outputVolume = outputVolume;
  command.outputDeformationFieldVolume = outputDeformationFieldVolume;
  command.inputPixelType = inputPixelType;
  command.outputPixelType = outputPixelType;
  command.outputDisplacementFieldPrefix = outputDisplacementFieldPrefix;
  command.outputCheckerboardVolume = outputCheckerboardVolume;
  command.forceCoronalZeroOrigin = forceCoronalZeroOrigin;
  command.outputNormalized = outputNormalized;
  command.outputDebug = outputDebug;
  command.maskProcessingMode = maskProcessingMode;
  command.fixedBinaryVolume = fixedBinaryVolume;
  command.movingBinaryVolume = movingBinaryVolume;
  command.lowerThresholdForBOBF = lowerThresholdForBOBF;
  command.upperThresholdForBOBF = upperThresholdForBOBF;
  command.backgroundFillValue = backgroundFillValue;
  command.movingLandmarks = movingLandmarks;
  command.fixedLandmarks = fixedLandmarks;
  command.initializeWithDeformationField = initializeWithDeformationField;
  command.initializeWithFourier = initializeWithFourier;
  command.initializeWithTransform = initializeWithTransform;

  command.histogramMatch = histogramMatch;
  command.numberOfHistogramLevels = numberOfHistogramBins;
  command.numberOfMatchPoints = numberOfMatchPoints;
  command.numberOfLevels = numberOfPyramidLevels;
  command.numberOfIterations.SetSize(numberOfPyramidLevels);

  command.maxStepLength = maxStepLength;
  command.gradientType = gradientType;
  command.smoothDeformationFieldSigma = smoothDeformationFieldSigma;
  command.smoothingUp = smoothingUp;
  command.numberOfBCHApproximationTerms = numberOfBCHApproximationTerms;

  for ( int i = 0; i < numberOfPyramidLevels; i++ )
    {
    command.numberOfIterations[i] = arrayOfPyramidLevelIterations[i];
    }

  for ( int i = 0; i < 3; i++ )
    {
    command.theMovingImageShrinkFactors[i] = minimumMovingPyramid[i];
    command.theFixedImageShrinkFactors[i] = minimumFixedPyramid[i];
    }

  for ( int i = 0; i < 3; i++ )
    {
    command.checkerboardPatternSubdivisions[i]
      = checkerboardPatternSubdivisions[i];
    command.seedForBOBF[i] = seedForBOBF[i];
    command.neighborhoodForBOBF[i] = neighborhoodForBOBF[i];
    command.medianFilterSize[i] = medianFilterSize[i];
    }

  if ( outputPixelType == "CopyInputPixelType" )
    {
    command.outputPixelType = inputPixelType;
    }
  }
  //  bool debug=true;
  if ( command.outputDebug )
    {
    std::cout
      << "                   movingVolume: " << command.movingVolume
      << std::endl
      << "                    fixedVolume: " << command.fixedVolume
      << std::endl
      << "                   outputVolume: " << command.outputVolume
      << std::endl
      << "   outputDeformationFieldVolume: "
      << command.outputDeformationFieldVolume << std::endl
      << "                 inputPixelType: " << command.inputPixelType
      << std::endl
      << "                outputPixelType: " << command.outputPixelType
      << std::endl
      << "  outputDisplacementFieldPrefix: "
      << command.outputDisplacementFieldPrefix << std::endl
      << "       outputCheckerboardVolume: "
      << command.outputCheckerboardVolume << std::endl
      << "checkerboardPatternSubdivisions: "
      << command.checkerboardPatternSubdivisions << std::endl
      << "               outputNormalized: " << command.outputNormalized
      << std::endl
      << "                    outputDebug: " << command.outputDebug
      << std::endl
      << "             maskProcessingMode: " << command.maskProcessingMode
      << std::endl
      << "              fixedBinaryVolume: " << command.fixedBinaryVolume
      << std::endl
      << "             movingBinaryVolume: " << command.movingBinaryVolume
      << std::endl
      << "          lowerThresholdForBOBF: "
      << command.lowerThresholdForBOBF << std::endl
      << "          upperThresholdForBOBF: "
      << command.upperThresholdForBOBF << std::endl
      << "            backgroundFillValue: " << command.backgroundFillValue
      << std::endl
      << "                    seedForBOBF: " << command.seedForBOBF
      << std::endl
      << "            neighborhoodForBOBF: " << command.neighborhoodForBOBF
      << std::endl
      << "               medianFilterSize: " << command.medianFilterSize
      << std::endl
      << "        movingLandmarks: " << command.movingLandmarks << std::endl
      << "         fixedLandmarks: " << command.fixedLandmarks << std::endl
      << "  initializeWithDeformationField: "
      << command.initializeWithDeformationField  << std::endl
      << "     initializeWithFourier: " << command.initializeWithFourier
      << std::endl
      << "       initializeWithTransform: "
      << command.initializeWithTransform << std::endl
      << "                    gradientType: " << command.gradientType
      << std::endl
      << "                   maxStepLength: " << command.maxStepLength
      << std::endl
      << "     smoothDeformationFieldSigma: "
      << command.smoothDeformationFieldSigma << std::endl
      << "                     smoothingUp: " << command.smoothingUp
      << std::endl
      << "                   histogramMatch: " << command.histogramMatch
      << std::endl
      << "                histogram levels: "
      << command.numberOfHistogramLevels << std::endl
      << "                 matching points: " << command.numberOfMatchPoints
      << std::endl
      << "   numberOfBCHApproximationTerms: " << command.numberOfBCHApproximationTerms
      << std::endl
      ;
    }

  bool violated = false;
  if ( command.movingVolume.size() == 0 )
    {
    violated = true; std::cout << "  --movingVolume Required! "  << std::endl;
    }
  if ( command.fixedVolume.size() == 0 )
    {
    violated = true; std::cout << "  --fixedVolume Required! "  << std::endl;
    }

  if ( ( command.checkerboardPatternSubdivisions[0] == 0 )
       || ( command.checkerboardPatternSubdivisions[1] == 0 )
       || ( command.checkerboardPatternSubdivisions[2] == 0 ) )
    {
    std::cout
      <<
      "Invalid Patameters. The value of checkboardPatternSubdivisions should not be zero!"
      << std::endl;
    exit(-1);
    }

  //   if (command.outputVolume.size() == 0)
  //   {
  //   violated = true; std::cout << "  --outputVolume Required! "  << std::endl;
  //   }
  // if (outputDeformationFieldVolume.size() == 0) { violated = true; std::cout
  // << "  --outputDeformationFieldVolume Required! "  << std::endl; }
  // if (registrationParameters.size() == 0) { violated = true; std::cout << "
  //  --registrationParameters Required! "  << std::endl; }
  // if (inputPixelType.size() == 0) { violated = true; std::cout << "
  //  --inputPixelType Required! "  << std::endl; }
  if ( violated )
    {
    exit(1);
    }

  // Test if the input data type is valid
  if ( command.inputPixelType != "" )
    {
    // check to see if valid type
    if ( ( CompareNoCase( command.inputPixelType.c_str(), std::string("uchar" ) ) )
         && ( CompareNoCase( command.inputPixelType.c_str(), std::string("short" ) ) )
         && ( CompareNoCase( command.inputPixelType.c_str(),
                             std::string("ushort") ) )
         && ( CompareNoCase( command.inputPixelType.c_str(), std::string("int"   ) ) )
         && ( CompareNoCase( command.inputPixelType.c_str(), std::string("float" ) ) )
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes too many
                           // segments in one object file for the intel compiler
         &&
         ( CompareNoCase( command.inputPixelType.c_str(), std::string("uint"  ) ) )
         && ( CompareNoCase( command.inputPixelType.c_str(),
                             std::string("double") ) )
#endif
         )
      {
      std::cout
        << "Error. Invalid data type string specified with --inputPixelType!"
        << std::endl;
      std::cout << "Use one of the following:" << std::endl;
      PrintDataTypeStrings();
      exit(-1);
      }
    }

  if ( command.outputPixelType != "" )
    {
    // check to see if valid type
    if ( ( CompareNoCase( command.outputPixelType.c_str(),
                          std::string("uchar" ) ) )
         &&            ( CompareNoCase( command.outputPixelType.c_str(),
                                        std::string("SHORT") ) )
         && ( CompareNoCase( command.outputPixelType.c_str(),
                             std::string("ushort") ) )
         && ( CompareNoCase( command.outputPixelType.c_str(), std::string("int"   ) ) )
         && ( CompareNoCase( command.outputPixelType.c_str(),
                             std::string("float" ) ) )
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes too many
                           // segments in one object file for the intel compiler
         &&
         ( CompareNoCase( command.outputPixelType.c_str(), std::string("uint"  ) ) )
         && ( CompareNoCase( command.outputPixelType.c_str(),
                             std::string("double") ) )
#endif
         )
      {
      std::cout
        << "Error. Invalid data type string specified with --outputPixelType!"
        << std::endl;
      std::cout << "Use one of the following:" << std::endl;
      PrintDataTypeStrings();
      exit(-1);
      }
    }

  // Call the process output data type function based on the input data type.

  if ( CompareNoCase ( command.inputPixelType, std::string ("uchar") ) == 0 )
    {
    ProcessOutputType<unsigned char>(command);
    }
  else if ( CompareNoCase ( command.inputPixelType,
                            std::string ("short") ) == 0 )
    {
    ProcessOutputType<short>(command);
    }
  else if ( CompareNoCase ( command.inputPixelType,
                            std::string ("ushort") ) == 0 )
    {
    ProcessOutputType<unsigned short>(command);
    }
  else if ( CompareNoCase ( command.inputPixelType, std::string ("int") ) == 0 )
    {
    ProcessOutputType<int>(command);
    }
  else if ( CompareNoCase ( command.inputPixelType,
                            std::string ("float") ) == 0 )
    {
    ProcessOutputType<float>(command);
    }
#ifdef _USE_UNCOMMON_TYPES // This is commented out because it causes too many
                           // segments in one object file for the intel compiler
  else if ( CompareNoCase ( command.inputPixelType, std::string ("uint") ) == 0 )
    {
    ProcessOutputType<unsigned int>(command);
    }
  else if ( CompareNoCase ( command.inputPixelType,
                            std::string ("double") ) == 0 )
    {
    ProcessOutputType<double>(command);
    }
#endif
  else
    {
    std::cout
      << "Error. Invalid data type for --inputPixelType!  Use one of these:"
      << std::endl;
    PrintDataTypeStrings ();
    exit (-1);
    }
  return 0;
}
