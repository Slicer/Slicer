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
#  pragma warning ( disable : 4786 )
#endif

//  This program was modified from
// Insight/Examples/Registration/ImageRegistration8.cxx
//  and is an improved replacement for the old (and defective)
//  brains2/src/iplProg/MutualRegistration program.

//  Image registration using the VersorRigid3DTransform and the
//  MattesMutualInformationImageToImageMetric is the logical
//  thing to insist on (with ITK) when seeking rigid 3D registrations.

#include "BRAINSFitIGTPrimary.h"

#ifdef USE_DEBUG_IMAGE_VIEWER
#  include "DebugImageViewerClient.h"
#endif

#include "BRAINSFitHelper.h"
#include "BRAINSFitIGTPrimaryCLP.h"

// Check that ITK was compiled with correct flags set:
#ifndef ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE
#  error                                                                  \
  "Results will not be correct if ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE is turned off"
#endif
#ifndef ITK_USE_ORIENTED_IMAGE_DIRECTION
#  error                                                                  \
  "Results will not be correct if ITK_USE_ORIENTED_IMAGE_DIRECTION is turned off"
#endif
#ifndef ITK_USE_TRANSFORM_IO_FACTORIES
#  error                                                                  \
  "BRAINSFitIGT Requires ITK_USE_TRANSFORM_IO_FACTORIES to be on, please rebuild ITK."
#endif

#if ( ITK_VERSION_MAJOR < 3  )
#  if ( ITK_VERSION_MINOR < 15 )
#    if ( ITK_VERSION_PATCH < 0 )
#      warning                                                                \
  "A version of ITK greater than 3.15.0 is needed in order to have the cross platform transform patches included."
#      warning                                                        \
  "Use at your own risk with versions less than 3.15.0."
#      warning                                                                \
  "Version 3.15.0 has fixed several issues related to inconsistent rounding on different platforms, and inconsistent definition of the center of voxels."
#    endif
#  endif
#endif

#include "GenericTransformImage.h"

#include "itkGridImageSource.h"

typedef float PixelType;
// Dimension and MaxInputDimension comes from an enum at the start of
// itkMultiModal3DMutualRegistrationHelper.h

typedef itk::Image< PixelType, Dimension > FixedVolumeType;
typedef itk::Image< PixelType, Dimension > MovingVolumeType;

typedef itk::Image< PixelType,
                    MaxInputDimension >                    InputImageType;

typedef itk::ImageFileReader< InputImageType >
FixedVolumeReaderType;
typedef itk::ImageFileReader< InputImageType >
MovingVolumeReaderType;

typedef AffineTransformType::Pointer
AffineTransformPointer;

typedef itk::Vector< double, Dimension > VectorType;

// This function deciphers the BackgroundFillValueString and returns a double
// precision number based on the requested value
double GetBackgroundFillValueFromString(
  const std::string & BackgroundFillValueString)
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
    BackgroundFillValue =
      static_cast< double >( atof( BackgroundFillValueString.c_str() ) );
    }
  return BackgroundFillValue;
}

template< class ImageType >
typename ImageType::Pointer ExtractImage(
  typename InputImageType::Pointer & inputImage,
  unsigned int InputImageTimeIndex)
{
  typedef typename itk::ExtractImageFilter< InputImageType,
                                            ImageType > ExtractImageFilterType;
  typename ExtractImageFilterType::Pointer extractImageFilter =
    ExtractImageFilterType::New();

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
  extractImageFilter->SetExtractionRegion(inputRegion);
  extractImageFilter->SetInput(inputImage);

  try
    {
    extractImageFilter->Update();
    }
  catch ( ... )
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

template< class ImageType >
typename ImageType::Pointer DoMedian(typename ImageType::Pointer & input,
                                     typename ImageType::SizeType indexRadius)
{
  typedef typename itk::MedianImageFilter< ImageType,
                                           ImageType > MedianFilterType;
  typename MedianFilterType::Pointer medianFilter = MedianFilterType::New();

  medianFilter->SetRadius(indexRadius);
  medianFilter->SetInput(input);
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

int BRAINSFitIGTPrimary(int argc, char *argv[])
{
  PARSE_ARGS;

  itk::AddExtraTransformRegister();

  RegisterBrains2MaskFactory();

#ifdef USE_DEBUG_IMAGE_VIEWER
  if ( UseDebugImageViewer )
    {
    DebugImageDisplaySender.SetEnabled(UseDebugImageViewer);
    }
#endif

  if ( debugNumberOfThreads > 0 ) // NOTE: Default is -1, which then uses the
  // ITK default.
    {
    itk::MultiThreader::SetGlobalMaximumNumberOfThreads(debugNumberOfThreads);
    }
  std::string localInitializeTransformMode = "Off";
  if ( ( ( initialTransform.size() > 0 ) + ( useCenterOfHeadAlign == true )
         + ( useGeometryAlign == true ) + ( useMomentsAlign == true )
         + ( useCenterOfROIAlign == true ) ) > 1 )
    {
    std::cout
    <<
    "ERROR:  Can only specify one of [initialTransform | useCenterOfHeadAlign | useGeometryAlign | useMomentsAlign \
        || useCenterOfROIAlign ]"
    << std::endl;
    }
  if ( useCenterOfHeadAlign == true ) { localInitializeTransformMode = "useCenterOfHeadAlign"; }
  if ( useGeometryAlign == true )     { localInitializeTransformMode = "useGeometryAlign"; }
  if ( useMomentsAlign == true )      { localInitializeTransformMode = "useMomentsAlign"; }
  if ( useCenterOfROIAlign == true )      { localInitializeTransformMode = "useCenterOfROIAlign"; }

  // std::vector<int> zeroOrigin(3, 0);

  // Verify that the spline grid sizes are greater than 3
    {
    bool validGridSize = true;
    for ( unsigned int sgs = 0; sgs < splineGridSize.size(); sgs++ )
      {
      if ( splineGridSize[sgs] < 3 )
        {
        validGridSize = false;
        std::cout << "splineGridSize[" << sgs << "]= " << splineGridSize[sgs]
                  << " is invalid.  There must be at lest 3 divisions in each dimension of the image." << std::endl;
        exit(-1);
        }
      }
    }
  std::vector< std::string > localTransformType;
  // See if the individual boolean registration options are being used.  If any
  // of these are set, then transformType is not used.
  if ( ( useRigid == true ) || ( useScaleVersor3D == true ) || ( useScaleSkewVersor3D == true )
       || ( useAffine == true ) || ( useBSpline == true ) || ( useROIBSpline == true ) )
    {
    localTransformType.resize(0); // Set to zero lenght;
    if ( useRigid == true )             { localTransformType.push_back("Rigid"); }
    if ( useScaleVersor3D == true )     { localTransformType.push_back("ScaleVersor3D"); }
    if ( useScaleSkewVersor3D == true ) { localTransformType.push_back("ScaleSkewVersor3D"); }
    if ( useAffine == true )            { localTransformType.push_back("Affine"); }
    if ( useBSpline == true )           { localTransformType.push_back("BSpline"); }
    if ( useROIBSpline == true )        { localTransformType.push_back("ROIBSpline"); }
    }
  else if ( transformType.size() > 0 )
    {
    localTransformType = transformType;
    }
  else if ( ( ( initialTransform.size() >  0 ) + ( useCenterOfHeadAlign == true )
              + ( useGeometryAlign == true ) + ( useMomentsAlign == true )
              + ( useCenterOfROIAlign == true ) ) > 0 )
    {
    // Only do the initialization phase;
    }
  else
    {
    std::cerr << "ERROR: No registration phases specified to perform!" << std::endl;
    exit(-1);
    }

  // In order to make the Slicer interface work, a few alternate command line
  // options need to available
  std::string localOutputTransform;
  if ( ( linearTransform.size() > 0 && bsplineTransform.size() > 0 )
       || ( linearTransform.size() > 0 && outputTransform.size() > 0 )
       || ( outputTransform.size() > 0 && bsplineTransform.size() > 0 ) )
    {
    std::cout << "Error:  user can only specify one output transform type." << std::endl;
    exit(-1);
    }
  if ( linearTransform.size() > 0 )
    {
    localOutputTransform = linearTransform;
    if ( ( localTransformType.size() > 0 ) && 
         ( ( localTransformType[localTransformType.size() - 1] == "BSpline" ) ||
           ( localTransformType[localTransformType.size() - 1] == "ROIBSpline") ) )
      {
      std::cout << "Error:  Linear transforms can not be used for BSpline registration!" << std::endl;
      exit(-1);
      }
    }
  else if ( bsplineTransform.size() > 0 )
    {
    localOutputTransform = bsplineTransform;
    if ( ( localTransformType.size() > 0 ) && 
         ( localTransformType[localTransformType.size() - 1] != "BSpline" ) &&
         ( localTransformType[localTransformType.size() - 1] != "ROIBSpline" ) ) 
         
      {
      std::cout << "Error:  BSpline registrations require output transform to be of type BSpline!" << std::endl;
      exit(-1);
      }
    else if ( localTransformType.size() == 0 )
      {
      std::cout << "Error:  Initializer only registrations require output transform to be of type Linear!" << std::endl;
      exit(-1);
      }
    }
  else if ( outputTransform.size() > 0 )
    {
    localOutputTransform = outputTransform;
    }

  if ( localOutputTransform.size() == 0
       && strippedOutputTransform.size() == 0
       && outputVolume.size() == 0 )
    {
    std::cout << "Error:  user requested neither localOutputTransform,"
              << " nor strippedOutputTransform,"
              << " nor outputVolume." << std::endl;
    return 2;
    }

  if ( numberOfIterations.size() != localTransformType.size() )
    {
    if ( numberOfIterations.size() != 1 )
      {
      std::cerr << "The numberOfIterations array must match the length of the transformType"
                << "length, or have a single value that is used for all registration phases." << std::endl;
      exit(-1);
      }
    else
      {
      // replicate throughout
      numberOfIterations.resize( localTransformType.size() );
      const int numberOf = numberOfIterations[0];
      for ( unsigned int i = 1; i < localTransformType.size(); i++ )
        {
        numberOfIterations[i] = ( numberOf );
        }
      }
    }
  if ( minimumStepSize.size() != localTransformType.size() )
    {
    if ( minimumStepSize.size() != 1 )
      {
      std::cerr << "The minimumStepSize array must match the localTransformType length" << std::endl;
      exit(-1);
      }
    else
      {
      // replicate throughout
      const double stepSize = minimumStepSize[0];
      for ( unsigned int i = 1; i < localTransformType.size(); i++ )
        {
        minimumStepSize.push_back(stepSize);
        }
      }
    }

  // Need to ensure that the order of transforms is from smallest to largest.
  itk::ValidateTransformRankOrdering(localTransformType);

  FixedVolumeType::Pointer
  extractFixedVolume ( itkUtil::ReadImage< FixedVolumeType >(fixedVolume) );
  MovingVolumeType::Pointer
  extractMovingVolume ( itkUtil::ReadImage< MovingVolumeType >(movingVolume) );

#ifdef USE_DEBUG_IMAGE_VIEWER
  if ( DebugImageDisplaySender.Enabled() )
    {
    DebugImageDisplaySender.SendImage< itk::Image< float, 3 > >(extractFixedVolume, 0);
    DebugImageDisplaySender.SendImage< itk::Image< float, 3 > >(extractMovingVolume, 1);
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
    indexRadius[0] = static_cast< long int >( medianFilterSize[0] ); // radius
                                                                     // along x
    indexRadius[1] = static_cast< long int >( medianFilterSize[1] ); // radius
                                                                     // along y
    indexRadius[2] = static_cast< long int >( medianFilterSize[2] ); // radius
                                                                     // along z
    // DEBUG
    std::cout << "Median radius  " << indexRadius << std::endl;
    extractFixedVolume = DoMedian< FixedVolumeType >(extractFixedVolume,
                                                     indexRadius);
    extractMovingVolume = DoMedian< MovingVolumeType >(extractMovingVolume,
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
    if ( maskProcessingMode == "NOMASK" )
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
        {
        typedef itk::BRAINSROIAutoImageFilter< FixedVolumeType, itk::Image< unsigned char, 3 > > ROIAutoType;
        ROIAutoType::Pointer ROIFilter = ROIAutoType::New();
        ROIFilter->SetInput(extractFixedVolume);
        ROIFilter->SetDilateSize(ROIAutoDilateSize);
        ROIFilter->Update();
        fixedMask = ROIFilter->GetSpatialObjectROI();
        }
        {
        typedef itk::BRAINSROIAutoImageFilter< MovingVolumeType, itk::Image< unsigned char, 3 > > ROIAutoType;
        ROIAutoType::Pointer ROIFilter = ROIAutoType::New();
        ROIFilter->SetInput(extractMovingVolume);
        ROIFilter->SetDilateSize(ROIAutoDilateSize);
        ROIFilter->Update();
        movingMask = ROIFilter->GetSpatialObjectROI();
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
      fixedMask = ReadImageMask< SpatialObjectType, Dimension >(
        fixedBinaryVolume,
        extractFixedVolume.GetPointer() );
      movingMask = ReadImageMask< SpatialObjectType, Dimension >(
        movingBinaryVolume,
        extractMovingVolume.GetPointer() );
      }
      { // Write out some debugging information if requested
      typedef itk::Image< unsigned char, 3 >                               MaskImageType;
      typedef itk::ImageMaskSpatialObject< MaskImageType::ImageDimension > ImageMaskSpatialObjectType;
      if ( ( !fixedMask.IsNull() ) && ( outputFixedVolumeROI != "" ) )
        {
        ImageMaskSpatialObjectType::Pointer fixedImageMask(
          dynamic_cast< ImageMaskSpatialObjectType * >( fixedMask.GetPointer() ) );
        MaskImageType::Pointer tempOutputFixedVolumeROI = const_cast< MaskImageType * >( fixedImageMask->GetImage() );
        itkUtil::WriteImage< MaskImageType >(tempOutputFixedVolumeROI, outputFixedVolumeROI);
        }
      if ( ( !movingMask.IsNull() ) && ( outputMovingVolumeROI != "" ) )
        {
        ImageMaskSpatialObjectType::Pointer movingImageMask(
          dynamic_cast< ImageMaskSpatialObjectType * >( movingMask.GetPointer() ) );
        MaskImageType::Pointer tempOutputMovingVolumeROI = const_cast< MaskImageType * >( movingImageMask->GetImage() );
        itkUtil::WriteImage< MaskImageType >(tempOutputMovingVolumeROI, outputMovingVolumeROI);
        }
      }
    }
  // This default fills the background with zeros
  //  const double BackgroundFillValue =
  //
  //
  //
  //
  // GetBackgroundFillValueFromString(command.GetValueAsString(BackgroundFillValueText,
  //  FloatCodeText));

  // Note itk::ReadTransformFromDisk returns NULL if file name does not exist.
  GenericTransformType::Pointer currentGenericTransform = itk::ReadTransformFromDisk(initialTransform);

  FixedVolumeType::Pointer resampledImage;
  /*
   *  Everything prior to this point is preprocessing
   *  Start Processing
   *
   */
  int actualIterations = 0;
  int permittedIterations = 0;
  // int allLevelsIterations=0;

    {
    typedef itk::BRAINSFitHelper HelperType;
    HelperType::Pointer myHelper = HelperType::New();
    myHelper->SetTransformType(localTransformType);
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
    myHelper->SetInitializeTransformMode(localInitializeTransformMode);
    myHelper->SetMaskInferiorCutOffFromCenter(maskInferiorCutOffFromCenter);
    myHelper->SetCurrentGenericTransform(currentGenericTransform);
    myHelper->SetSplineGridSize(splineGridSize);
    myHelper->SetCostFunctionConvergenceFactor(costFunctionConvergenceFactor);
    myHelper->SetProjectedGradientTolerance(projectedGradientTolerance);
    myHelper->SetMaxBSplineDisplacement(maxBSplineDisplacement);
    myHelper->SetDisplayDeformedImage(UseDebugImageViewer);
    myHelper->SetPromptUserAfterDisplay(PromptAfterImageSend);
    myHelper->SetDebugLevel(debugLevel);
    if ( debugLevel > 7 )
      {
      myHelper->PrintCommandLine(true, "BF");
      }
    myHelper->StartRegistration();
    currentGenericTransform = myHelper->GetCurrentGenericTransform();
    MovingVolumeType::ConstPointer preprocessedMovingVolume = myHelper->GetPreprocessedMovingVolume();

// intraop acquisition is usually in lower resolution -- does it make sense to
// increase the spacing for the resampled volume?
// perhaps this can be done by the user with additional resampling ...
#if 0
    FixedVolumeType::Pointer
      resampleReferenceVolume = FixedVolumeType::New();

    // use spacing from the moving volume, since fixed volume usually has larger
    // slice thickness
    FixedVolumeType::SpacingType originalSpacing = extractFixedVolume->GetSpacing();
    FixedVolumeType::SpacingType refSpacing;
    FixedVolumeType::RegionType  refRegion = extractFixedVolume->GetLargestPossibleRegion();

    FixedVolumeType::SizeType  originalSize = refRegion.GetSize();
    MovingVolumeType::SizeType refSize;

    refSpacing[0] = extractMovingVolume->GetSpacing()[0];
    refSpacing[1] = extractMovingVolume->GetSpacing()[1];
    refSpacing[2] = extractMovingVolume->GetSpacing()[2];

    refSize[0] = originalSize[0] * originalSpacing[0] / refSpacing[0];
    refSize[1] = originalSize[1] * originalSpacing[1] / refSpacing[1];
    refSize[2] = originalSize[2] * originalSpacing[2] / refSpacing[2];

    std::cout << "Setting output spacing to " << refSpacing << std::endl;

    refRegion.SetSize(refSize);

    resampleReferenceVolume->SetRegions(refRegion);
    resampleReferenceVolume->Allocate();
    resampleReferenceVolume->SetDirection( extractFixedVolume->GetDirection() );
    resampleReferenceVolume->SetSpacing(refSpacing);
    resampleReferenceVolume->SetOrigin( extractFixedVolume->GetOrigin() );
#endif //

    const std::vector< GenericTransformType::Pointer > *tfmList = myHelper->GetGenericTransformListPtr();
    if ( tfmList->size() )
      {
      unsigned    currentTransformId = 0, requestedTransformId = 0;
      std::string currentFileName = "";
      if ( localInitializeTransformMode != "Off" )
        {
        if ( initialTransformDebug.size() > 0 )
          {
          std::cout << ( *tfmList )[0] << std::endl;
          itk::WriteTransformToDisk( ( *tfmList )[0], initialTransformDebug );
          }
        currentTransformId++;
        }
      for (; currentTransformId < tfmList->size(); currentTransformId++, requestedTransformId++ )
        {
        if ( localTransformType[requestedTransformId] == "Rigid" && rigidTransformDebug.size() > 0 )
          {
          currentFileName = rigidTransformDebug;
          }
        if ( localTransformType[requestedTransformId] == "ScaleVersor3D" && scaleVersorTransformDebug.size() > 0 )
          {
          currentFileName = scaleVersorTransformDebug;
          }
        if ( localTransformType[requestedTransformId] == "ScaleSkewVersor3D" && scaleSkewVersorTransformDebug.size() >
             0 )
          {
          currentFileName = scaleSkewVersorTransformDebug;
          }
        if ( localTransformType[requestedTransformId] == "Affine" && affineTransformDebug.size() > 0 )
          {
          currentFileName = affineTransformDebug;
          }
        if ( ( localTransformType[requestedTransformId] == "BSpline" || localTransformType[requestedTransformId] ==
               "ROIBSpline" )
             && bsplineTransformDebug.size() > 0 )
          {
          currentFileName = bsplineTransformDebug;
          }

        if ( ( localTransformType[requestedTransformId] == "BSpline"
               || localTransformType[requestedTransformId] == "ROIBSpline" )
             && deformedGridImage.size() > 0
              )
          {
          // generate the label image showing the bspline grid, and apply the
          // bspline transformation
          // ... follow BRAINSCommonLib/GenericTransformImage.cxx
          GenericTransformType const *const        tfm = ( *tfmList )[currentTransformId];
          const BSplineTransformType::ConstPointer bsplineTfm =
            dynamic_cast< BSplineTransformType const *const >( tfm );

          // get transform parameters
          BSplineTransformType::ParametersType fixedParams =
            bsplineTfm->GetFixedParameters();
          BSplineTransformType::ParametersType params =
            bsplineTfm->GetParameters();

          typedef itk::GridImageSource< FixedVolumeType > GridSourceType;
          GridSourceType::Pointer gridImageFilter = GridSourceType::New();

          float                         scale = 255.0;
          float                         gridThickness = 2.;
          FixedVolumeType::SizeType     size;
          FixedVolumeType::SpacingType  spacing;
          GridSourceType::ArrayType     gridSpacing;
          GridSourceType::ArrayType     gridOffset;
          GridSourceType::ArrayType     sigma;
          GridSourceType::BoolArrayType which;

          // Specify image parameters
          spacing = extractMovingVolume->GetSpacing();
          size = extractMovingVolume->GetLargestPossibleRegion().GetSize();

          // Specify grid parameters
          sigma.Fill(1);
          which.Fill(true);
          which[2] = false;

          size[1] = size[1] * spacing[1] / spacing[0];
          size[2] = size[2] * spacing[2] / spacing[0];
          spacing.Fill(spacing[0]);

          sigma[0] = spacing[0] * gridThickness;
          sigma[1] = spacing[1] * gridThickness;
          sigma[2] = spacing[2] * gridThickness;

          gridSpacing[0] = fixedParams[6];
          gridSpacing[1] = fixedParams[7];
          gridSpacing[2] = fixedParams[8];

          gridOffset[0] = gridSpacing[0] / 2.;
          gridOffset[1] = gridSpacing[1] / 2.;
          gridOffset[2] = gridSpacing[2] / 2.;

          // Specify 0th order B-spline function (Box function)
          typedef itk::BSplineKernelFunction< 0 > KernelType;
          KernelType::Pointer kernel = KernelType::New();

          // Set parameters
          gridImageFilter->SetKernelFunction(kernel);

          // Set parameters; spacing and size will be different from those in
          // the original moving image
          gridImageFilter->SetSpacing(spacing);
          gridImageFilter->SetSize(size);
          gridImageFilter->SetOrigin( extractMovingVolume->GetOrigin() );
          gridImageFilter->SetGridSpacing(gridSpacing);
          gridImageFilter->SetGridOffset(gridOffset);
          gridImageFilter->SetDirection( extractMovingVolume->GetDirection() );
          gridImageFilter->SetWhichDimensions(which);
          gridImageFilter->SetSigma(sigma);
          gridImageFilter->SetScale(scale);
          gridImageFilter->Update();

          // resample the grid and save the output
          resampledImage = TransformResample< MovingVolumeType, FixedVolumeType >(
            gridImageFilter->GetOutput(),
            extractFixedVolume,
//          resampleReferenceVolume,
            0,
            GetInterpolatorFromString< MovingVolumeType >( std::string("NearestNeighbor") ),
            currentGenericTransform);

          itkUtil::WriteImage< FixedVolumeType >(resampledImage, deformedGridImage);
          }

        if ( currentFileName.size() == 0 )
          {
          continue;
          }

        itk::WriteTransformToDisk( ( *tfmList )[currentTransformId], currentFileName );
        currentFileName = "";
        }
      }

      {
      // Remember:  the Data is Moving's, the shape is Fixed's.
      resampledImage = TransformResample< MovingVolumeType, FixedVolumeType >(
        preprocessedMovingVolume,
//    resampleReferenceVolume, // fixed volume has large slice thickness
        extractFixedVolume,
        backgroundFillValue,
        GetInterpolatorFromString< MovingVolumeType >(interpolationMode),
        currentGenericTransform);
      }
    actualIterations = myHelper->GetActualNumberOfIterations();
    permittedIterations = myHelper->GetPermittedNumberOfIterations();
    //
    //
    // allLevelsIterations=myHelper->GetAccumulatedNumberOfIterationsForAllLevels();
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
      // itkUtil::WriteCastImage<itk::Image<float,
      // FixedVolumeType::ImageDimension>,
      // FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image< float,
                          FixedVolumeType::ImageDimension > WriteOutImageType;
      WriteOutImageType::Pointer CastImage =
        ( scaleOutputValues == true ) ?
        ( itkUtil::PreserveCast< FixedVolumeType,
                                 WriteOutImageType >(resampledImage) ) :
        ( itkUtil::TypeCast< FixedVolumeType,
                             WriteOutImageType >(resampledImage) );
      itkUtil::WriteImage< WriteOutImageType >(CastImage, outputVolume);
      }
    else if ( outputVolumePixelType == "short" )
      {
      // itkUtil::WriteCastImage<itk::Image<signed short,
      // FixedVolumeType::ImageDimension>,
      // FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image< signed short,
                          FixedVolumeType::ImageDimension > WriteOutImageType;
      WriteOutImageType::Pointer CastImage =
        ( scaleOutputValues == true ) ?
        ( itkUtil::PreserveCast< FixedVolumeType,
                                 WriteOutImageType >(resampledImage) ) :
        ( itkUtil::TypeCast< FixedVolumeType,
                             WriteOutImageType >(resampledImage) );
      itkUtil::WriteImage< WriteOutImageType >(CastImage, outputVolume);
      }
    else if ( outputVolumePixelType == "ushort" )
      {
      // itkUtil::WriteCastImage<itk::Image<unsigned short,
      // FixedVolumeType::ImageDimension>,
      // FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image< unsigned short,
                          FixedVolumeType::ImageDimension > WriteOutImageType;
      WriteOutImageType::Pointer CastImage =
        ( scaleOutputValues == true ) ?
        ( itkUtil::PreserveCast< FixedVolumeType,
                                 WriteOutImageType >(resampledImage) ) :
        ( itkUtil::TypeCast< FixedVolumeType,
                             WriteOutImageType >(resampledImage) );
      itkUtil::WriteImage< WriteOutImageType >(CastImage, outputVolume);
      }
    else if ( outputVolumePixelType == "int" )
      {
      // itkUtil::WriteCastImage<itk::Image<signed int,
      // FixedVolumeType::ImageDimension>,
      // FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image< signed int,
                          FixedVolumeType::ImageDimension > WriteOutImageType;
      WriteOutImageType::Pointer CastImage =
        ( scaleOutputValues == true ) ?
        ( itkUtil::PreserveCast< FixedVolumeType,
                                 WriteOutImageType >(resampledImage) ) :
        ( itkUtil::TypeCast< FixedVolumeType,
                             WriteOutImageType >(resampledImage) );
      itkUtil::WriteImage< WriteOutImageType >(CastImage, outputVolume);
      }
    else if ( outputVolumePixelType == "uint" )
      {
      // itkUtil::WriteCastImage<itk::Image<unsigned int,
      // FixedVolumeType::ImageDimension>,
      // FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image< unsigned int,
                          FixedVolumeType::ImageDimension > WriteOutImageType;
      WriteOutImageType::Pointer CastImage =
        ( scaleOutputValues == true ) ?
        ( itkUtil::PreserveCast< FixedVolumeType,
                                 WriteOutImageType >(resampledImage) ) :
        ( itkUtil::TypeCast< FixedVolumeType,
                             WriteOutImageType >(resampledImage) );
      itkUtil::WriteImage< WriteOutImageType >(CastImage, outputVolume);
      }
#if 0
    else if ( outputVolumePixelType == "char" )
      {
      // itkUtil::WriteCastImage<itk::Image<signed char,
      // FixedVolumeType::ImageDimension>,
      // FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image< char,
                          FixedVolumeType::ImageDimension > WriteOutImageType;
      WriteOutImageType::Pointer CastImage =
        ( scaleOutputValues == true ) ?
        ( itkUtil::PreserveCast< FixedVolumeType,
                                 WriteOutImageType >(resampledImage) ) :
        ( itkUtil::TypeCast< FixedVolumeType,
                             WriteOutImageType >(resampledImage) );
      itkUtil::WriteImage< WriteOutImageType >(CastImage, outputVolume);
      }
#endif
    else if ( outputVolumePixelType == "uchar" )
      {
      // itkUtil::WriteCastImage<itk::Image<unsigned char,
      // FixedVolumeType::ImageDimension>,
      // FixedVolumeType>(resampledImage,outputVolume);
      typedef itk::Image< unsigned char,
                          FixedVolumeType::ImageDimension > WriteOutImageType;
      WriteOutImageType::Pointer CastImage =
        ( scaleOutputValues == true ) ?
        ( itkUtil::PreserveCast< FixedVolumeType,
                                 WriteOutImageType >(resampledImage) ) :
        ( itkUtil::TypeCast< FixedVolumeType,
                             WriteOutImageType >(resampledImage) );
      itkUtil::WriteImage< WriteOutImageType >(CastImage, outputVolume);
      }
    }

#if 0 // HACK:  This does not work properly when only an initializer transform
      // is used, or if the final transform is BSpline.
  // GREG:  BRAINSFitIGT currently does not determine if the registrations have
  // not
  // converged before reaching their maximum number of iterations.  Currently
  // transforms are always written out, under the assumption that the
  // registraiton converged.  We need to figure out how to determine if the
  // registrations did not converge (i.e. maximum number of iterations were
  // reached), and then not write out the transforms, unless explicitly demanded
  // to write them out from a command line flag.
  // GREG:  We should write a test, and document what the expected behaviors are
  // when a multi-level registration is requested (Rigid,ScaleSkew,Affine), and
  // one of the first types does not converge.
  // HACK  This does not work properly until BSpline reports iterations
  // correctly
  if ( actualIterations + 1 >= permittedIterations )
    {
    if ( writeTransformOnFailure == false ) // taken right off the command line.
      {
      std::cout << "actualIterations: " << actualIterations << std::endl;
      std::cout << "permittedIterations: " << permittedIterations << std::endl;
      return failureExitCode; // taken right off the command line.
      }
    }
#endif

  /*const int write_status=*/
  itk::WriteBothTransformsToDisk(currentGenericTransform.GetPointer(),
                                 localOutputTransform, strippedOutputTransform);

#if 0 // HACK:  This does not work properly when only an initializer transform
      // is used, or if the final transform is BSpline.
  if ( actualIterations + 1 >= permittedIterations )
    {
    std::cout << "actualIterations: " << actualIterations << std::endl;
    std::cout << "permittedIterations: " << permittedIterations << std::endl;
    return failureExitCode; // taken right off the command line.
    }
#endif

  return 0;
}
