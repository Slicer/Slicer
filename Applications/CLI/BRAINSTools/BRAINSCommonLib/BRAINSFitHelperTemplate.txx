#include "itkIO.h"
#ifdef USE_DEBUG_IMAGE_VIEWER
#  include "DebugImageViewerClient.h"
#endif

#include "itkEuler3DTransform.h"
#include "itkCheckerBoardImageFilter.h"
#include "itkOtsuHistogramMatchingImageFilter.h"
#include <fstream>
#include "BRAINSFitHelperTemplate.h"

#include "itkLabelObject.h"
#include "itkStatisticsLabelObject.h"
#include "itkLabelImageToStatisticsLabelMapFilter.h"

#include "itkImageToImageMetric.h"

#include "itkMattesMutualInformationImageToImageMetric.h"

namespace itk
{
  inline
    void
    ValidateTransformRankOrdering(const std::vector< std::string > & transformType)
      {
      // Need to review transform types in the transform type vector to ensure that
      // they are ordered appropriately
      // for processing.  I.e. that they go from low dimensional to high
      // dimensional, and throw an error if
      // B-Spline is before Rigid, or any other non-sensical ordering of the
      // transform types.
      // Rigid=1, ScaleVersor3D=2, ScaleSkewVersor3D=3, Affine=4, and (BSpline or
      // ROIBspline)=5
      unsigned int CurrentTransformRank = 0;

      for ( unsigned int l = 0; l < transformType.size(); l++ )
        {
        if ( transformType[l] == "Rigid" )
          {
          if ( CurrentTransformRank <= 1 )
            {
            CurrentTransformRank = 1;
            }
          else
            {
            std::cerr << "Ordering of transforms does not proceed from\n"
              << "smallest to largest.  Please review settings for transformType.\n"
              << "Rigid < ScaleVersor3D < ScaleSkewVersor3D < Affine < (BSpline | ROIBSpine)" << std::endl;
            exit (-1);
            }
          }
        else if ( transformType[l] == "ScaleVersor3D" )
          {
          if ( CurrentTransformRank <= 2 )
            {
            CurrentTransformRank = 2;
            }
          else
            {
            std::cerr << "Ordering of transforms does not proceed from\n"
              << "smallest to largest.  Please review settings for transformType.\n"
              << "Rigid < ScaleVersor3D < ScaleSkewVersor3D < Affine < (BSpline | ROIBSpline)" << std::endl;
            exit (-1);
            }
          }
        else if ( transformType[l] == "ScaleSkewVersor3D" )
          {
          if ( CurrentTransformRank <= 3 )
            {
            CurrentTransformRank = 3;
            }
          else
            {
            std::cerr << "Ordering of transforms does not proceed from\n"
              << "smallest to largest.  Please review settings for transformType.\n"
              << "Rigid < ScaleVersor3D < ScaleSkewVersor3D < Affine < (BSpline | ROIBSpline)" << std::endl;
            exit (-1);
            }
          }
        else if ( transformType[l] == "Affine" )
          {
          if ( CurrentTransformRank <= 4 )
            {
            CurrentTransformRank = 4;
            }
          else
            {
            std::cerr << "Ordering of transforms does not proceed from\n"
              << "smallest to largest.  Please review settings for transformType.\n"
              << "Rigid < ScaleVersor3D < ScaleSkewVersor3D < Affine < (BSpline | ROIBSpline)" << std::endl;
            exit (-1);
            }
          }
        else if ( transformType[l] == "BSpline" )
          {
          if ( CurrentTransformRank <= 5 )
            {
            CurrentTransformRank = 5;
            }
          else
            {
            std::cerr << "Ordering of transforms does not proceed from\n"
              << "smallest to largest.  Please review settings for transformType.\n"
              << "Rigid < ScaleVersor3D < ScaleSkewVersor3D < Affine < (BSpline | ROIBSpline)" << std::endl;
            exit (-1);
            }
          }
        else if ( transformType[l] == "ROIBSpline" )
          {
          if ( CurrentTransformRank <= 5 )
            {
            CurrentTransformRank = 5;
            }
          else
            {
            std::cerr << "Ordering of transforms does not proceed from\n"
              << "smallest to largest.  Please review settings for transformType.\n"
              << "Rigid < ScaleVersor3D < ScaleSkewVersor3D < Affine < (BSpline | ROIBSpline)" << std::endl;
            exit (-1);
            }
          }

        else
          {
          std::cerr << " ERROR:  Invalid transform type specified for element " << l << " of --transformType: "
            << transformType[l] << std::endl;
          std::cerr << "Ordering of transforms must proceed from\n"
            << "smallest to largest.  Please review settings for transformType.\n"
            << "Rigid < ScaleVersor3D < ScaleSkewVersor3D < Affine < BSpline" << std::endl;
          exit(-1);
          }
        }
      }


  template< class FixedImageType, class MovingImageType, class TransformType,
    class SpecificInitializerType, typename MetricType >
      typename TransformType::Pointer DoCenteredInitialization(
        typename FixedImageType::Pointer & orientedFixedVolume,
        typename MovingImageType::Pointer & orientedMovingVolume,
        ImageMaskPointer & fixedMask,   // NOTE:  This is both input and output variable,  the Mask is updated by this function
        ImageMaskPointer & movingMask,  // NOTE:  This is both input and output variable,  the Mask is updated by this function
        std::string & initializeTransformMode,
        typename MetricType::Pointer & CostMetricObject)
        {
        typename TransformType::Pointer initialITKTransform = TransformType::New();
        initialITKTransform->SetIdentity();

        if ( initializeTransformMode == "useGeometryAlign" )
          {
          // useGeometryAlign assumes objects are center in field of view, with
          // different
          typedef itk::CenteredTransformInitializer< TransformType, FixedImageType,
                  MovingImageType > OrdinaryInitializerType;
          typename OrdinaryInitializerType::Pointer CenteredInitializer =
            OrdinaryInitializerType::New();

          CenteredInitializer->SetFixedImage(orientedFixedVolume);
          CenteredInitializer->SetMovingImage(orientedMovingVolume);
          CenteredInitializer->SetTransform(initialITKTransform);
          CenteredInitializer->GeometryOn();              // Use the image spce center
          CenteredInitializer->InitializeTransform();
          }
        else if ( initializeTransformMode == "useCenterOfHeadAlign"
          || initializeTransformMode == "useCenterOfROIAlign" )
          {
          typedef typename itk::ImageMaskSpatialObject< FixedImageType::ImageDimension > ImageMaskSpatialObjectType;
          typedef itk::Image< unsigned char, 3 >                                          MaskImageType;
          typename MovingImageType::PointType movingCenter;
          typename FixedImageType::PointType fixedCenter;

          if ( initializeTransformMode == "useCenterOfROIAlign" )
            {
            // calculate the centers of each ROI
            typedef itk::StatisticsLabelObject< unsigned char, 3 > LabelObjectType;
            typedef itk::LabelMap< LabelObjectType >               LabelMapType;
            typedef itk::LabelImageToStatisticsLabelMapFilter< MaskImageType, MaskImageType >
              LabelStatisticsFilterType;
            typedef LabelMapType::LabelObjectContainerType LabelObjectContainerType;

            typename ImageMaskSpatialObjectType::Pointer movingImageMask(
              dynamic_cast< ImageMaskSpatialObjectType * >( movingMask.GetPointer() ) );
            typename MaskImageType::Pointer tempOutputMovingVolumeROI =
              const_cast< MaskImageType * >( movingImageMask->GetImage() );

            typename ImageMaskSpatialObjectType::Pointer fixedImageMask(
              dynamic_cast< ImageMaskSpatialObjectType * >( fixedMask.GetPointer() ) );
            typename MaskImageType::Pointer tempOutputFixedVolumeROI =
              const_cast< MaskImageType * >( fixedImageMask->GetImage() );

            LabelStatisticsFilterType::Pointer movingImageToLabel = LabelStatisticsFilterType::New();
            movingImageToLabel->SetInput( movingImageMask->GetImage() );
            movingImageToLabel->SetFeatureImage( movingImageMask->GetImage() );
            movingImageToLabel->SetComputePerimeter(false);
            movingImageToLabel->Update();

            LabelStatisticsFilterType::Pointer fixedImageToLabel = LabelStatisticsFilterType::New();
            fixedImageToLabel->SetInput( fixedImageMask->GetImage() );
            fixedImageToLabel->SetFeatureImage( fixedImageMask->GetImage() );
            fixedImageToLabel->SetComputePerimeter(false);
            fixedImageToLabel->Update();

            LabelObjectType *movingLabel = movingImageToLabel->GetOutput()->GetNthLabelObject(0);
            LabelObjectType *fixedLabel = fixedImageToLabel->GetOutput()->GetNthLabelObject(0);

            LabelObjectType::CentroidType movingCentroid = movingLabel->GetCentroid();
            LabelObjectType::CentroidType fixedCentroid = fixedLabel->GetCentroid();

            movingCenter[0] = movingCentroid[0];
            movingCenter[1] = movingCentroid[1];
            movingCenter[2] = movingCentroid[2];

            fixedCenter[0] = fixedCentroid[0];
            fixedCenter[1] = fixedCentroid[1];
            fixedCenter[2] = fixedCentroid[2];

            // calculate the translation (and rotation?)
            // initialize the transform center using the fixed ROI center
            }
          else // CenterOfHead
            {  //     typename MovingImageType::PointType movingCenter =
            // GetCenterOfBrain<MovingImageType>(orientedMovingVolume);
            //     typename FixedImageType::PointType fixedCenter =
            // GetCenterOfBrain<FixedImageType>(orientedFixedVolume);
            typedef typename itk::FindCenterOfBrainFilter< MovingImageType >
              MovingFindCenterFilter;
            typename MovingFindCenterFilter::Pointer movingFindCenter =
              MovingFindCenterFilter::New();
            movingFindCenter->SetInput(orientedMovingVolume);
            if ( movingMask.IsNotNull() )
              {
              typename ImageMaskSpatialObjectType::Pointer movingImageMask(
                dynamic_cast< ImageMaskSpatialObjectType * >( movingMask.GetPointer() ) );
              typename MaskImageType::Pointer tempOutputMovingVolumeROI =
                const_cast< MaskImageType * >( movingImageMask->GetImage() );
              movingFindCenter->SetImageMask(tempOutputMovingVolumeROI);
              }
            movingFindCenter->Update();
            movingCenter = movingFindCenter->GetCenterOfBrain();
              {
              // convert mask image to mask
              typedef typename itk::ImageMaskSpatialObject< Dimension >
                ImageMaskSpatialObjectType;
              typename ImageMaskSpatialObjectType::Pointer mask =
                ImageMaskSpatialObjectType::New();
              mask->SetImage( movingFindCenter->GetClippedImageMask() );

              typename MaskImageType::Pointer ClippedMask = movingFindCenter->GetClippedImageMask();
              // itkUtil::WriteImage<MaskImageType>( ClippedMask ,
              // std::string("MOVING_MASK.nii.gz"));

              mask->ComputeObjectToWorldTransform();
              typename SpatialObjectType::Pointer p = dynamic_cast< SpatialObjectType * >( mask.GetPointer() );
              if ( p.IsNull() )
                {
                std::cout << "ERROR::" << __FILE__ << " " << __LINE__ << std::endl;
                exit(-1);
                }
              movingMask = p;
              }

            typedef typename itk::FindCenterOfBrainFilter< FixedImageType >
              FixedFindCenterFilter;
            typename FixedFindCenterFilter::Pointer fixedFindCenter =
              FixedFindCenterFilter::New();
            fixedFindCenter->SetInput(orientedFixedVolume);
            if ( fixedMask.IsNotNull() )
              {
              typename ImageMaskSpatialObjectType::Pointer fixedImageMask(
                dynamic_cast< ImageMaskSpatialObjectType * >( fixedMask.GetPointer() ) );
              typename MaskImageType::Pointer tempOutputFixedVolumeROI =
                const_cast< MaskImageType * >( fixedImageMask->GetImage() );
              fixedFindCenter->SetImageMask(tempOutputFixedVolumeROI);
              }
            fixedFindCenter->Update();
            fixedCenter = fixedFindCenter->GetCenterOfBrain();

              {
              // convert mask image to mask
              typedef typename itk::ImageMaskSpatialObject< Dimension > ImageMaskSpatialObjectType;
              typename ImageMaskSpatialObjectType::Pointer mask = ImageMaskSpatialObjectType::New();
              mask->SetImage( fixedFindCenter->GetClippedImageMask() );

              typename MaskImageType::Pointer ClippedMask = fixedFindCenter->GetClippedImageMask();

              mask->ComputeObjectToWorldTransform();
              typename SpatialObjectType::Pointer p = dynamic_cast< SpatialObjectType * >( mask.GetPointer() );
              if ( p.IsNull() )
                {
                std::cout << "ERROR::" << __FILE__ << " " << __LINE__ << std::endl;
                exit(-1);
                }
              fixedMask = p;
              }
            }

          const double movingHeadScaleGuessRatio = 1;
          /*
           *
           *fixedFindCenter->GetHeadSizeEstimate()/movingFindCenter->GetHeadSizeEstimate();
           */

          typename TransformType::InputPointType rotationCenter;
          typename TransformType::OutputVectorType translationVector;
          itk::Vector< double, 3 > scaleValue;

          for ( unsigned int i = 0; i < Dimension; i++ )
            {
            rotationCenter[i]    = fixedCenter[i];
            translationVector[i] = movingCenter[i] - fixedCenter[i];
            scaleValue[i] = movingHeadScaleGuessRatio;
            }
          typedef itk::Euler3DTransform< double > EulerAngle3DTransformType;
          typename EulerAngle3DTransformType::Pointer bestEulerAngles3D = EulerAngle3DTransformType::New();
          bestEulerAngles3D->SetCenter(rotationCenter);
          bestEulerAngles3D->SetTranslation(translationVector);

          typedef itk::Euler3DTransform< double > EulerAngle3DTransformType;
          typename EulerAngle3DTransformType::Pointer currentEulerAngles3D = EulerAngle3DTransformType::New();
          currentEulerAngles3D->SetCenter(rotationCenter);
          currentEulerAngles3D->SetTranslation(translationVector);

          CostMetricObject->SetTransform(currentEulerAngles3D);
          CostMetricObject->Initialize();
          double max_cc = 0.0;
          // void QuickSampleParameterSpace(void)
            {
            currentEulerAngles3D->SetRotation(0, 0, 0);
            // Initialize with current guess;
            max_cc = CostMetricObject->GetValue( currentEulerAngles3D->GetParameters() );
            const double HARange = 12.0;
            const double PARange = 12.0;
            // rough search in neighborhood.
            const double one_degree = 1.0F * vnl_math::pi / 180.0F;
            const double HAStepSize = 3.0 * one_degree;
            const double PAStepSize = 3.0 * one_degree;
            // Quick search just needs to get an approximate angle correct.
              {
              for ( double HA = -HARange * one_degree; HA <= HARange * one_degree; HA += HAStepSize )
                {
                for ( double PA = -PARange * one_degree; PA <= PARange * one_degree; PA += PAStepSize )
                  {
                  currentEulerAngles3D->SetRotation(PA, 0, HA);
                  const double current_cc = CostMetricObject->GetValue( currentEulerAngles3D->GetParameters() );
                  if ( current_cc < max_cc )
                    {
                    max_cc = current_cc;
                    bestEulerAngles3D->SetFixedParameters( currentEulerAngles3D->GetFixedParameters() );
                    bestEulerAngles3D->SetParameters( currentEulerAngles3D->GetParameters() );
                    }
                  // #define DEBUGGING_PRINT_IMAGES
#ifdef DEBUGGING_PRINT_IMAGES
                    {
                    std::cout << "quick search "
                      << " HA= " << ( currentEulerAngles3D->GetParameters()[2] ) * 180.0 / vnl_math::pi
                      << " PA= " << ( currentEulerAngles3D->GetParameters()[0] ) * 180.0 / vnl_math::pi
                      << " cc="  <<  current_cc
                      << std::endl;
                    }
                  if ( 0 )
                    {
                    typedef itk::ResampleImageFilter< FixedImageType, MovingImageType, double > ResampleFilterType;
                    typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();

                    resampler->SetTransform(currentEulerAngles3D);
                    resampler->SetInput(orientedMovingVolume);
                    // Remember:  the Data is Moving's, the shape is Fixed's.
                    resampler->SetOutputParametersFromImage(orientedFixedVolume);
                    resampler->Update();            //  Explicit Update() required
                    // here.
                    typename FixedImageType::Pointer ResampledImage = resampler->GetOutput();

                    typedef itk::CheckerBoardImageFilter< FixedImageType > Checkerfilter;
                    typename Checkerfilter::Pointer checker = Checkerfilter::New ();
                    unsigned int array[3] = { 36, 36, 36 };

                    checker->SetInput1 (orientedFixedVolume);
                    checker->SetInput2 (ResampledImage);
                    checker->SetCheckerPattern (array);
                    try
                      {
                      checker->Update ();
                      }
                    catch ( itk::ExceptionObject & err )
                      {
                      std::cout << "Caught an ITK exception: " << std::endl;
                      std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
                      }
                    char filename[300];
                    sprintf(filename, "%05.2f_%05.2f_%05.2f.nii.gz",
                      ( currentEulerAngles3D->GetParameters()[2] ) * 180 / vnl_math::pi,
                      ( currentEulerAngles3D->GetParameters()[0] ) * 180 / vnl_math::pi, current_cc);

                      {
                      typedef typename itk::ImageFileWriter< FixedImageType > WriterType;
                      typename WriterType::Pointer writer = WriterType::New();
                      writer->UseCompressionOn();
                      writer->SetFileName(filename);
                      writer->SetInput( checker->GetOutput() );
                      try
                        {
                        writer->Update();
                        }
                      catch ( itk::ExceptionObject & err )
                        {
                        std::cout << "Exception Object caught: " << std::endl;
                        std::cout << err << std::endl;
                        throw;
                        }
                      }
                    }
#endif
                  }
                }
              }
            // DEBUGGING_PRINT_IMAGES INFORMATION
#ifdef DEBUGGING_PRINT_IMAGES
              {
              std::cout << "FINAL: quick search "
                << " HA= " << ( bestEulerAngles3D->GetParameters()[2] ) * 180.0 / vnl_math::pi
                << " PA= " << ( bestEulerAngles3D->GetParameters()[0] ) * 180.0 / vnl_math::pi
                << " cc="  <<  max_cc
                << std::endl;
              }
#endif
            }
          typename VersorRigid3DTransformType::Pointer quickSetVersor = VersorRigid3DTransformType::New();
          quickSetVersor->SetCenter( bestEulerAngles3D->GetCenter() );
          quickSetVersor->SetTranslation( bestEulerAngles3D->GetTranslation() );
            {
            itk::Versor< double > localRotation;
            localRotation.Set( bestEulerAngles3D->GetRotationMatrix() );
            quickSetVersor->SetRotation(localRotation);
            }
#ifdef DEBUGGING_PRINT_IMAGES
            {
            typedef itk::ResampleImageFilter< FixedImageType, MovingImageType, double > ResampleFilterType;
            typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();

            resampler->SetTransform(quickSetVersor);
            resampler->SetInput(orientedMovingVolume);
            // Remember:  the Data is Moving's, the shape is Fixed's.
            resampler->SetOutputParametersFromImage(orientedFixedVolume);
            resampler->Update();              //  Explicit Update() required here.
            typename FixedImageType::Pointer ResampledImage = resampler->GetOutput();

            typedef itk::CheckerBoardImageFilter< FixedImageType > Checkerfilter;
            typename Checkerfilter::Pointer checker = Checkerfilter::New ();
            unsigned int array[3] = { 18, 18, 18 };

            checker->SetInput1 (orientedFixedVolume);
            checker->SetInput2 (ResampledImage);
            checker->SetCheckerPattern (array);
            try
              {
              checker->Update ();
              }
            catch ( itk::ExceptionObject & err )
              {
              std::cout << "Caught an ITK exception: " << std::endl;
              std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
              }
            char filename[300];
            sprintf(filename, "FINAL_%05.2f_%05.2f_%05.2f.nii.gz",
              ( bestEulerAngles3D->GetParameters()[2] ) * 180 / vnl_math::pi,
              ( bestEulerAngles3D->GetParameters()[0] ) * 180 / vnl_math::pi, max_cc);

              {
              typedef typename itk::ImageFileWriter< FixedImageType > WriterType;
              typename WriterType::Pointer writer = WriterType::New();
              wirter->UseCompressionOn();
              writer->SetFileName(filename);
              // writer->SetInput(checker->GetOutput());
              writer->SetInput(ResampledImage);
              try
                {
                writer->Update();
                }
              catch ( itk::ExceptionObject & err )
                {
                std::cout << "Exception Object caught: " << std::endl;
                std::cout << err << std::endl;
                throw;
                }
              }
            }
#endif
          AssignRigid::AssignConvertedTransform( initialITKTransform, quickSetVersor.GetPointer() );
          }
        else if ( initializeTransformMode == "useMomentsAlign" )
          {
          // useMomentsAlign assumes that the structures being registered have same
          // amount
          // of mass approximately uniformly distributed.
          typename SpecificInitializerType::Pointer CenteredInitializer =
            SpecificInitializerType::New();

          CenteredInitializer->SetFixedImage(orientedFixedVolume);
          CenteredInitializer->SetMovingImage(orientedMovingVolume);
          CenteredInitializer->SetTransform(initialITKTransform);
          CenteredInitializer->MomentsOn();              // Use intensity center of
          // mass

          CenteredInitializer->InitializeTransform();
          }
        else               // can't happen unless an unimplemented CLP option was
          // added:
          {
          std::cout << "FAILURE:  Improper mode for initializeTransformMode: "
            << initializeTransformMode << std::endl;
          exit(-1);
          }
        std::cout << "Initializing transform with "  << initializeTransformMode
          << " to " << std::endl;
        std::cout << initialITKTransform << std::endl;
        std::cout << "===============================================" << std::endl;
        return initialITKTransform;
        }

  template <class FixedImageType, class MovingImageType>
    BRAINSFitHelperTemplate<FixedImageType,MovingImageType>::BRAINSFitHelperTemplate()
      {
      m_TransformType.resize(1);
      m_TransformType[0] = "Rigid";
      m_FixedVolume = NULL;
      m_MovingVolume = NULL;
      m_FixedBinaryVolume = NULL;
      m_MovingBinaryVolume = NULL;
      m_PermitParameterVariation.resize(0);
      m_NumberOfSamples = 500000;
      m_NumberOfHistogramBins = 50;
      m_HistogramMatch = false;
      m_NumberOfMatchPoints = 10;
      m_NumberOfIterations.resize(1);
      m_NumberOfIterations[0] = 1500;
      m_MaximumStepLength = 0.2;
      m_MinimumStepLength.resize(1);
      m_MinimumStepLength[0] = 0.005;
      m_RelaxationFactor = 0.5;
      m_TranslationScale = 1000.0;
      m_ReproportionScale = 1.0;
      m_SkewScale = 1.0;
      m_BackgroundFillValue = 0.0;
      m_InitializeTransformMode = "Off";
      m_UseExplicitPDFDerivativesMode = "Off";
      m_MaskInferiorCutOffFromCenter = 1000;
      m_CurrentGenericTransform = NULL;
      m_SplineGridSize.resize(3);
      m_SplineGridSize[0] = 14;
      m_SplineGridSize[1] = 10;
      m_SplineGridSize[2] = 12;
      m_MaxBSplineDisplacement = 0.0,
                               m_CostFunctionConvergenceFactor = 1e+9,
                               m_ProjectedGradientTolerance = 1e-5,
                               m_ActualNumberOfIterations = 0;
      m_PermittedNumberOfIterations = 0;
      // m_AccumulatedNumberOfIterationsForAllLevels=0;
      m_DebugLevel = 0;
      m_DisplayDeformedImage = false;
      m_PromptUserAfterDisplay = false;
      m_CostMetricObject = NULL;
      }

  template <class FixedImageType, class MovingImageType>
    template <class TransformType, class OptimizerType, class MetricType>
    void
    BRAINSFitHelperTemplate<FixedImageType, MovingImageType>::
    FitCommonCode(int numberOfIterations,
      double minimumStepLength,
      typename TransformType::Pointer &initialITKTransform)
      {
      // FitCommonCode
      typedef typename itk::MultiModal3DMutualRegistrationHelper
        < TransformType,
      OptimizerType,
      FixedImageType,
      MovingImageType,
      MetricType > MultiModal3DMutualRegistrationHelperType;

      typename MultiModal3DMutualRegistrationHelperType::Pointer
        appMutualRegistration = MultiModal3DMutualRegistrationHelperType::New();

      appMutualRegistration->SetNumberOfHistogramBins(m_NumberOfHistogramBins);

      appMutualRegistration->SetNumberOfIterations( numberOfIterations);

      // TODO:  What do the following two lines really accomplish
      // debug parameter, suppressed from command line
      const bool initialTransformPassThru(false);
      appMutualRegistration->SetInitialTransformPassThruFlag( initialTransformPassThru );
      appMutualRegistration->SetPermitParameterVariation( m_PermitParameterVariation );
      appMutualRegistration->SetNumberOfSamples( m_NumberOfSamples );
      appMutualRegistration->SetRelaxationFactor( m_RelaxationFactor );
      appMutualRegistration->SetMaximumStepLength( m_MaximumStepLength );
      appMutualRegistration->SetMinimumStepLength( minimumStepLength );
      appMutualRegistration->SetTranslationScale( m_TranslationScale );
      appMutualRegistration->SetReproportionScale( m_ReproportionScale );
      appMutualRegistration->SetSkewScale( m_SkewScale );

      appMutualRegistration->SetFixedImage(    m_FixedVolume    );
      appMutualRegistration->SetMovingImage(   m_MovingVolume   );
      appMutualRegistration->SetCostMetricObject ( this->m_CostMetricObject );

      appMutualRegistration->SetBackgroundFillValue(   m_BackgroundFillValue   );

      appMutualRegistration->SetInitialTransform( initialITKTransform );
      appMutualRegistration->SetDisplayDeformedImage(m_DisplayDeformedImage);
      appMutualRegistration->SetPromptUserAfterDisplay(m_PromptUserAfterDisplay);
      appMutualRegistration->SetObserveIterations(m_ObserveIterations);
      /*
       *  At this point appMutualRegistration should be all set to make
       *  an itk pipeline class templated in TransformType etc.
       *  with all its inputs in place;
       */
      // initialize the interconnects between components
      appMutualRegistration->Initialize();

      typename TransformType::Pointer finalTransform;
      try
        {
        appMutualRegistration->Update();
        finalTransform = appMutualRegistration->GetTransform();
        this->m_FinalMetricValue=appMutualRegistration->GetFinalMetricValue();
        this->m_ActualNumberOfIterations = appMutualRegistration->GetActualNumberOfIterations();
        this->m_PermittedNumberOfIterations = numberOfIterations;
        //this->m_AccumulatedNumberOfIterationsForAllLevels += appMutualRegistration->GetActualNumberOfIterations();
        }
      catch( itk::ExceptionObject& err )
        {
        // pass exception to caller
        std::cout << "ERROR OCCURED: " << err << std::endl;
        exit(-1);
        throw err;
        }

      { //Put the transform on the CurrentTransformList
      //Initialize next level of transformations with previous transform result
      this->m_CurrentGenericTransform=finalTransform;
      }
      }

  template <class FixedImageType, class MovingImageType>
    void
    BRAINSFitHelperTemplate<FixedImageType, MovingImageType>::StartRegistration(void)
      {
      typedef itk::MattesMutualInformationImageToImageMetric<FixedImageType,MovingImageType> MattesMutualInformationMetricType;
      unsigned currentTransformId = 0;

      if ( std::string(this->m_InitializeTransformMode) != "Off" )
        {
        m_GenericTransformList.resize(m_TransformType.size() + 1);
        }
      else
        {
        m_GenericTransformList.resize( m_TransformType.size() );
        }

      if ( this->m_DebugLevel > 3 )
        {
        this->PrintSelf(std::cout, 3);
        }
      std::vector< double > localMinimumStepLength( m_TransformType.size() );
      if ( m_MinimumStepLength.size() != m_TransformType.size() )
        {
        if ( m_MinimumStepLength.size() != 1 )
          {
          std::cout << "ERROR:  Wrong number of parameters for MinimumStepLength."
            << "It either needs to be 1 or the same size as TransformType."
            << std::endl;
          exit(-1);
          }
        for ( unsigned int q = 0; q < m_TransformType.size(); q++ )
          {
          localMinimumStepLength[q] = m_MinimumStepLength[0];
          }
        }
      else
        {
        localMinimumStepLength = m_MinimumStepLength;
        }
      std::vector< int > localNumberOfIterations( m_TransformType.size() );
      if ( m_NumberOfIterations.size() != m_TransformType.size() )
        {
        if ( m_NumberOfIterations.size() != 1 )
          {
          std::cout << "ERROR:  Wrong number of parameters for NumberOfIterations."
            << " It either needs to be 1 or the same size as TransformType."
            << std::endl;
          exit(-1);
          }
        for ( unsigned int q = 0; q < m_TransformType.size(); q++ )
          {
          localNumberOfIterations[q] = m_NumberOfIterations[0];
          }
        }
      else
        {
        localNumberOfIterations = m_NumberOfIterations;
        }
      std::string localInitializeTransformMode(this->m_InitializeTransformMode);
      for ( unsigned int currentTransformIndex = 0;
        currentTransformIndex < m_TransformType.size();
        currentTransformIndex++ )
        {
        const std::string currentTransformType(m_TransformType[currentTransformIndex]);
        std::cout << "TranformTypes: "
          << currentTransformType << "(" << currentTransformIndex + 1 << " of " << m_TransformType.size() << ")."
          << std::endl;
        std::cout << std::flush << std::endl;
        }


      // Initialize Transforms
      if ( localInitializeTransformMode != "Off" )
        // Use CenteredVersorTranformInitializer
        {
        typedef VersorRigid3DTransformType TransformType;
        std::cout << "Initializing transform with " << localInitializeTransformMode << std::endl;
        typedef itk::CenteredVersorTransformInitializer< FixedImageType,
                MovingImageType > InitializerType;

        TransformType::Pointer initialITKTransform =
          DoCenteredInitialization< FixedImageType, MovingImageType,
          TransformType, InitializerType, MetricType >(
            m_FixedVolume,
            m_MovingVolume,
            m_FixedBinaryVolume,
            m_MovingBinaryVolume,
            localInitializeTransformMode,this->m_CostMetricObject);
        m_CurrentGenericTransform = initialITKTransform.GetPointer();
        localInitializeTransformMode = "Off";    // Now reset to Off once
        // initialization is done.

        // Now if necessary clip the images based on m_MaskInferiorCutOffFromCenter
        DoCenteredTransformMaskClipping< TransformType,
          FixedImageType::ImageDimension >(
            m_FixedBinaryVolume,
            m_MovingBinaryVolume,
            initialITKTransform,
            m_MaskInferiorCutOffFromCenter);

        { // Write out some debugging information if requested
        typedef itk::Image< unsigned char, 3 >                               MaskImageType;
        typedef itk::ImageMaskSpatialObject< MaskImageType::ImageDimension > ImageMaskSpatialObjectType;
        if ( ( !this->m_FixedBinaryVolume.IsNull() ) && ( m_OutputFixedVolumeROI != "" ) )
          {
          ImageMaskSpatialObjectType::Pointer fixedImageMask(
            dynamic_cast< ImageMaskSpatialObjectType * >( this->m_FixedBinaryVolume.GetPointer() ) );
          MaskImageType::Pointer tempOutputFixedVolumeROI = const_cast< MaskImageType * >( fixedImageMask->GetImage() );
          itkUtil::WriteImage< MaskImageType >(tempOutputFixedVolumeROI, m_OutputFixedVolumeROI);
          }
        if ( ( !this->m_MovingBinaryVolume.IsNull() ) && ( m_OutputMovingVolumeROI != "" ) )
          {
          ImageMaskSpatialObjectType::Pointer movingImageMask(
            dynamic_cast< ImageMaskSpatialObjectType * >( this->m_MovingBinaryVolume.GetPointer() ) );
          MaskImageType::Pointer tempOutputMovingVolumeROI = const_cast< MaskImageType * >( movingImageMask->GetImage() );
          itkUtil::WriteImage< MaskImageType >(tempOutputMovingVolumeROI, m_OutputMovingVolumeROI);
          }
        }

      m_GenericTransformList[currentTransformId++] = initialITKTransform;
        }

      for ( unsigned int currentTransformIndex = 0;
        currentTransformIndex < m_TransformType.size();
        currentTransformIndex++ )
        {
        // m_AccumulatedNumberOfIterationsForAllLevels +=
        // localNumberOfIterations[currentTransformIndex];
        const std::string currentTransformType(m_TransformType[currentTransformIndex]);
        std::cout << "\n\n\n=============================== "
          << "Starting Transform Estimations for "
          << currentTransformType << "(" << currentTransformIndex + 1
          << " of " << m_TransformType.size() << ")."
          << "==============================="
          << std::endl;
        std::cout << std::flush << std::endl;
        //
        // Break into cases on TransformType:
        //
        if ( currentTransformType == "Rigid" )
          {
          //  Choose TransformType for the itk registration class template:
          typedef VersorRigid3DTransformType           TransformType;
          typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
          // const int NumberOfEstimatedParameter = 6;

          //
          // Process the initialITKTransform as VersorRigid3DTransform:
          //
          TransformType::Pointer initialITKTransform = TransformType::New();
          initialITKTransform->SetIdentity();
          if ( m_CurrentGenericTransform.IsNotNull() )
            {
            try
              {
              const std::string transformFileType = m_CurrentGenericTransform->GetNameOfClass();
              if ( transformFileType == "VersorRigid3DTransform" )
                {
                const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< VersorRigid3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( ( transformFileType == "ScaleVersor3DTransform" )
                || ( transformFileType == "ScaleSkewVersor3DTransform" )
                || ( transformFileType == "AffineTransform" ) )
                {
                // CONVERTING TO RIGID TRANSFORM TYPE from other type:
                std::cout << "WARNING:  Extracting Rigid component type from transform." << std::endl;
                VersorRigid3DTransformType::Pointer tempInitializerITKTransform = itk::ComputeRigidTransformFromGeneric(
                  m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform( initialITKTransform, tempInitializerITKTransform.GetPointer() );
                }
              else
                {
                std::cout
                  <<
                  "Unsupported initial transform file -- TransformBase first transform typestring, "
                  << transformFileType
                  << " not equal to required type VersorRigid3DTransform"
                  << std::endl;
                return;
                }
              }
            catch ( itk::ExceptionObject & excp )
              {
              std::cout << "[FAILED]" << std::endl;
              std::cerr
                << "Error while reading the m_CurrentGenericTransform" << std::endl;
              std::cerr << excp << std::endl;
              return;
              }
            }
            {
            // Special optimizations only for the MMI metric
            // that need adjusting based on both the type of metric, and
            // the "dimensionality" of the transform being adjusted.
            typename MattesMutualInformationMetricType::Pointer test_MMICostMetric= dynamic_cast<MattesMutualInformationMetricType *>(this->m_CostMetricObject.GetPointer());
            if(test_MMICostMetric.IsNotNull())
              {
              const bool UseExplicitPDFDerivatives = ( m_UseExplicitPDFDerivativesMode == "ON" || m_UseExplicitPDFDerivativesMode == "AUTO" ) ? true : false;
              test_MMICostMetric->SetUseExplicitPDFDerivatives(UseExplicitPDFDerivatives);
              }
            }

          this->FitCommonCode<TransformType, OptimizerType, MetricType>
            (localNumberOfIterations[currentTransformIndex],
             localMinimumStepLength[currentTransformIndex],
             initialITKTransform);
          localInitializeTransformMode="Off"; //Now turn of the initiallize code to off

          }
        else if ( currentTransformType == "ScaleVersor3D" )
          {
          //  Choose TransformType for the itk registration class template:
          typedef ScaleVersor3DTransformType    TransformType;
          typedef itk::VersorTransformOptimizer OptimizerType;
          // const int NumberOfEstimatedParameter = 9;

          //
          // Process the initialITKTransform as ScaleVersor3DTransform:
          //
          TransformType::Pointer initialITKTransform = TransformType::New();
          initialITKTransform->SetIdentity();
          if ( m_CurrentGenericTransform.IsNotNull() )
            {
            try
              {
              const std::string transformFileType = m_CurrentGenericTransform->GetNameOfClass();
              if ( transformFileType == "VersorRigid3DTransform" )
                {
                const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< VersorRigid3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( transformFileType == "ScaleVersor3DTransform" )
                {
                const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( ( transformFileType == "ScaleSkewVersor3DTransform" )
                || ( transformFileType == "AffineTransform" ) )
                {
                // CONVERTING TO RIGID TRANSFORM TYPE from other type:
                // TODO: we should preserve the Scale components
                std::cout << "WARNING:  Extracting Rigid component type from transform." << std::endl;
                VersorRigid3DTransformType::Pointer tempInitializerITKTransform = itk::ComputeRigidTransformFromGeneric(
                  m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform( initialITKTransform, tempInitializerITKTransform.GetPointer() );
                }
              else          // || transformFileType == "ScaleSkewVersor3DTransform"
                // ||
                // transformFileType == "AffineTransform"
                {
                std::cout
                  <<
                  "Unsupported initial transform file -- TransformBase first transform typestring, "
                  << transformFileType
                  <<
                  " not equal to required type VersorRigid3DTransform OR ScaleVersor3DTransform"
                  << std::endl;
                return;
                }
              }
            catch ( itk::ExceptionObject & excp )
              {
              std::cout << "[FAILED]" << std::endl;
              std::cerr
                << "Error while reading the m_CurrentGenericTransform"
                << std::endl;
              std::cerr << excp << std::endl;
              return;
              }
            }
            {
            // Special optimizations only for the MMI metric
            // that need adjusting based on both the type of metric, and
            // the "dimensionality" of the transform being adjusted.
            typename MattesMutualInformationMetricType::Pointer test_MMICostMetric= dynamic_cast<MattesMutualInformationMetricType *>(this->m_CostMetricObject.GetPointer());
            if(test_MMICostMetric.IsNotNull())
              {
              const bool UseExplicitPDFDerivatives = ( m_UseExplicitPDFDerivativesMode == "ON" || m_UseExplicitPDFDerivativesMode == "AUTO" ) ? true : false;
              test_MMICostMetric->SetUseExplicitPDFDerivatives(UseExplicitPDFDerivatives);
              }
            }
          // #include "FitCommonCode.tmpl"
          this->FitCommonCode<TransformType, OptimizerType, MetricType>
            (localNumberOfIterations[currentTransformIndex],
             localMinimumStepLength[currentTransformIndex],
             initialITKTransform);
          localInitializeTransformMode="Off"; //Now turn of the initiallize code to off
          }
        else if ( currentTransformType == "ScaleSkewVersor3D" )
          {
          //  Choose TransformType for the itk registration class template:
          typedef ScaleSkewVersor3DTransformType TransformType;
          typedef itk::VersorTransformOptimizer  OptimizerType;
          // const int NumberOfEstimatedParameter = 15;

          //
          // Process the initialITKTransform as ScaleSkewVersor3D:
          //
          TransformType::Pointer initialITKTransform = TransformType::New();
          initialITKTransform->SetIdentity();
          if ( m_CurrentGenericTransform.IsNotNull() )
            {
            try
              {
              const std::string transformFileType = m_CurrentGenericTransform->GetNameOfClass();
              if ( transformFileType == "VersorRigid3DTransform" )
                {
                const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< VersorRigid3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( transformFileType == "ScaleVersor3DTransform" )
                {
                const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( transformFileType == "ScaleSkewVersor3DTransform" )
                {
                const ScaleSkewVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleSkewVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( ( transformFileType == "AffineTransform" ) )
                {
                // CONVERTING TO RIGID TRANSFORM TYPE from other type:
                // TODO:  We should really preserve the Scale and Skew components
                std::cout << "WARNING:  Extracting Rigid component type from transform." << std::endl;
                VersorRigid3DTransformType::Pointer tempInitializerITKTransform = itk::ComputeRigidTransformFromGeneric(
                  m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform( initialITKTransform, tempInitializerITKTransform.GetPointer() );
                }
              else          // || transformFileType == "AffineTransform" ||
                // transformFileType
                // == "ScaleVersor3DTransform"
                {
                std::cout
                  <<
                  "Unsupported initial transform file -- TransformBase first transform typestring, "
                  << transformFileType
                  << " not equal to required type VersorRigid3DTransform "
                  << "OR ScaleVersor3DTransform OR ScaleSkewVersor3DTransform"
                  << std::endl;
                return;
                }
              }
            catch ( itk::ExceptionObject & excp )
              {
              std::cout << "[FAILED]" << std::endl;
              std::cerr
                << "Error while reading the m_CurrentGenericTransform"
                << std::endl;
              std::cerr << excp << std::endl;
              return;
              }
            }
            {
            // Special optimizations only for the MMI metric
            // that need adjusting based on both the type of metric, and
            // the "dimensionality" of the transform being adjusted.
            typename MattesMutualInformationMetricType::Pointer test_MMICostMetric= dynamic_cast<MattesMutualInformationMetricType *>(this->m_CostMetricObject.GetPointer());
            if(test_MMICostMetric.IsNotNull())
              {
              const bool UseExplicitPDFDerivatives = ( m_UseExplicitPDFDerivativesMode == "ON" || m_UseExplicitPDFDerivativesMode == "AUTO" ) ? true : false;
              test_MMICostMetric->SetUseExplicitPDFDerivatives(UseExplicitPDFDerivatives);
              }
            }
          // #include "FitCommonCode.tmpl"
          this->FitCommonCode<TransformType, OptimizerType, MetricType>
            (localNumberOfIterations[currentTransformIndex],
             localMinimumStepLength[currentTransformIndex],
             initialITKTransform);
          localInitializeTransformMode="Off"; //Now turn of the initiallize code to off
          }
        else if ( currentTransformType == "Affine" )
          {
          //  Choose TransformType for the itk registration class template:
          typedef itk::AffineTransform< double, Dimension > TransformType;
          typedef itk::RegularStepGradientDescentOptimizer  OptimizerType;
          // const int NumberOfEstimatedParameter = 12;

          //
          // Process the initialITKTransform
          //
          TransformType::Pointer initialITKTransform = TransformType::New();
          initialITKTransform->SetIdentity();
          if ( m_CurrentGenericTransform.IsNotNull() )
            {
            try
              {
              const std::string transformFileType = m_CurrentGenericTransform->GetNameOfClass();
              if ( transformFileType == "VersorRigid3DTransform" )
                {
                const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< VersorRigid3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( transformFileType == "ScaleVersor3DTransform" )
                {
                const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( transformFileType == "ScaleSkewVersor3DTransform" )
                {
                const ScaleSkewVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleSkewVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else if ( transformFileType == "AffineTransform" )
                {
                const AffineTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< AffineTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(initialITKTransform,
                  tempInitializerITKTransform);
                }
              else          //  NO SUCH CASE!!
                {
                std::cout
                  <<
                  "Unsupported initial transform file -- TransformBase first transform typestring, "
                  << transformFileType
                  << " not equal to any recognized type VersorRigid3DTransform OR "
                  << "ScaleVersor3DTransform OR ScaleSkewVersor3DTransform OR AffineTransform"
                  << std::endl;
                return;
                }
              }
            catch ( itk::ExceptionObject & excp )
              {
              std::cout << "[FAILED]" << std::endl;
              std::cerr
                << "Error while reading the m_CurrentGenericTransform"
                << std::endl;
              std::cerr << excp << std::endl;
              return;
              }
            }

            {
            // Special optimizations only for the MMI metric
            // that need adjusting based on both the type of metric, and
            // the "dimensionality" of the transform being adjusted.
            typename MattesMutualInformationMetricType::Pointer test_MMICostMetric= dynamic_cast<MattesMutualInformationMetricType *>(this->m_CostMetricObject.GetPointer());
            if(test_MMICostMetric.IsNotNull())
              {
              const bool UseExplicitPDFDerivatives = ( m_UseExplicitPDFDerivativesMode == "ON" || m_UseExplicitPDFDerivativesMode == "AUTO" ) ? true : false;
              test_MMICostMetric->SetUseExplicitPDFDerivatives(UseExplicitPDFDerivatives);
              }
            }
          // #include "FitCommonCode.tmpl"
          this->FitCommonCode<TransformType, OptimizerType, MetricType>
            (localNumberOfIterations[currentTransformIndex],
             localMinimumStepLength[currentTransformIndex],
             initialITKTransform);
          localInitializeTransformMode="Off"; //Now turn of the initiallize code to off
          }
        else if ( currentTransformType == "BSpline" )
          {
          //
          // Process the bulkAffineTransform for BSpline's BULK
          //
          AffineTransformType::Pointer bulkAffineTransform =
            AffineTransformType::New();
          bulkAffineTransform->SetIdentity();

          typedef itk::Image< float, 3 > RegisterImageType;

          BSplineTransformType::Pointer outputBSplineTransform =
            BSplineTransformType::New();
          outputBSplineTransform->SetIdentity();

          BSplineTransformType::Pointer initialBSplineTransform =
            BSplineTransformType::New();
          initialBSplineTransform->SetIdentity();

            {
            typedef BSplineTransformType::RegionType TransformRegionType;
            typedef TransformRegionType::SizeType    TransformSizeType;
            typedef itk::BSplineDeformableTransformInitializer
              < BSplineTransformType, RegisterImageType > InitializerType;
            InitializerType::Pointer transformInitializer = InitializerType::New();
            transformInitializer->SetTransform(initialBSplineTransform);
            transformInitializer->SetImage(m_FixedVolume);
            TransformSizeType tempGridSize;
            tempGridSize[0] = m_SplineGridSize[0];
            tempGridSize[1] = m_SplineGridSize[1];
            tempGridSize[2] = m_SplineGridSize[2];
            transformInitializer->SetGridSizeInsideTheImage(tempGridSize);
            transformInitializer->InitializeTransform();
            }

          if ( m_CurrentGenericTransform.IsNotNull() )
            {
            try
              {
              const std::string transformFileType = m_CurrentGenericTransform->GetNameOfClass();
              if ( transformFileType == "VersorRigid3DTransform" )
                {
                const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< VersorRigid3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(bulkAffineTransform,
                  tempInitializerITKTransform);
                initialBSplineTransform->SetBulkTransform(bulkAffineTransform);
                }
              else if ( transformFileType == "ScaleVersor3DTransform" )
                {
                const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(bulkAffineTransform,
                  tempInitializerITKTransform);
                initialBSplineTransform->SetBulkTransform(bulkAffineTransform);
                }
              else if ( transformFileType == "ScaleSkewVersor3DTransform" )
                {
                const ScaleSkewVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleSkewVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(bulkAffineTransform,
                  tempInitializerITKTransform);
                initialBSplineTransform->SetBulkTransform(bulkAffineTransform);
                }
              else if ( transformFileType == "AffineTransform" )
                {
                const AffineTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< AffineTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(bulkAffineTransform,
                  tempInitializerITKTransform);
                initialBSplineTransform->SetBulkTransform(bulkAffineTransform);
                }
              else if ( transformFileType == "BSplineDeformableTransform" )
                {
                const BSplineTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< BSplineTransformType const *const >( m_CurrentGenericTransform.GetPointer() );

                initialBSplineTransform->SetBulkTransform(
                  tempInitializerITKTransform->GetBulkTransform() );
                BSplineTransformType::ParametersType tempFixedInitialParameters =
                  tempInitializerITKTransform->GetFixedParameters();
                BSplineTransformType::ParametersType initialFixedParameters =
                  initialBSplineTransform->GetFixedParameters();

                bool checkMatch = true;     // Assume true;
                if ( initialFixedParameters.GetSize() != tempFixedInitialParameters.GetSize() )
                  {
                  checkMatch = false;
                  std::cerr << "ERROR INITILIZATION FIXED PARAMETERS DO NOT MATCH: " << initialFixedParameters.GetSize()
                    << " != " << tempFixedInitialParameters.GetSize() << std::endl;
                  }
                if ( checkMatch )        //  This ramus covers the hypothesis that
                  // the
                  // FixedParameters represent the grid locations of the spline nodes.
                  {
                  for ( unsigned int i = 0; i < initialFixedParameters.GetSize(); i++ )
                    {
                    if ( initialFixedParameters.GetElement(i) != tempFixedInitialParameters.GetElement(i) )
                      {
                      checkMatch = false;
                      std::cerr << "ERROR FIXED PARAMETERS DO NOT MATCH: " << initialFixedParameters.GetElement(i)
                        << " != " << tempFixedInitialParameters.GetElement(i) << std::endl;
                      }
                    }
                  }
                if ( checkMatch )
                  {
                  BSplineTransformType::ParametersType tempInitialParameters =
                    tempInitializerITKTransform->GetParameters();
                  if ( initialBSplineTransform->GetNumberOfParameters() ==
                    tempInitialParameters.Size() )
                    {
                    initialBSplineTransform->SetFixedParameters(
                      tempFixedInitialParameters);
                    initialBSplineTransform->SetParametersByValue(tempInitialParameters);
                    }
                  else
                    {
                    // Error, initializing from wrong size transform parameters;
                    //  Use its bulk transform only?
                    std::cerr
                      << "Trouble using the m_CurrentGenericTransform for initializing a BSPlineDeformableTransform:"
                      << std::endl;
                    std::cerr
                      <<
                      "The initializing BSplineDeformableTransform has a different"
                      << " number of Parameters, than what is required for the requested grid."
                      << std::endl;
                    std::cerr
                      << "BRAINSFit was only able to use the bulk transform that was before it."
                      << std::endl;
                    exit(-1);
                    }
                  }
                else
                  {
                  std::cerr
                    << "ERROR:  initialization BSpline transform does not have the same "
                    << "parameter dimensions as the one currently specified."
                    << std::endl;
                  exit(-1);
                  }
                }
              else
                {
                std::cerr << "ERROR:  Invalid transform initializer type found:  " << transformFileType << std::endl;
                exit(-1);
                }
              }
            catch ( itk::ExceptionObject & excp )
              {
              std::cout << "[FAILED]" << std::endl;
              std::cerr
                << "Error while reading the m_CurrentGenericTransform"
                << std::endl;
              std::cerr << excp << std::endl;
              return;
              }
            }

            {
            // Special optimizations only for the MMI metric
            // that need adjusting based on both the type of metric, and
            // the "dimensionality" of the transform being adjusted.
            typename MattesMutualInformationMetricType::Pointer test_MMICostMetric= dynamic_cast<MattesMutualInformationMetricType *>(this->m_CostMetricObject.GetPointer());
            if(test_MMICostMetric.IsNotNull())
              {
              // As recommended in documentation in
              // itkMattesMutualInformationImageToImageMetric.h
              // "UseExplicitPDFDerivatives = False ... This method is well suited
              // for Transforms with a large number of parameters, such as,
              // BSplineDeformableTransforms."
              const bool UseExplicitPDFDerivatives = ( m_UseExplicitPDFDerivativesMode != "ON" || m_UseExplicitPDFDerivativesMode == "AUTO" ) ? false : true;
              test_MMICostMetric->SetUseExplicitPDFDerivatives(UseExplicitPDFDerivatives);
              }
            }
          outputBSplineTransform =
            DoBSpline< RegisterImageType, SpatialObjectType,
            BSplineTransformType >(
              initialBSplineTransform,
              m_FixedVolume, m_MovingVolume,
              this->m_CostMetricObject.GetPointer(),
              this->m_MaxBSplineDisplacement,
              this->m_CostFunctionConvergenceFactor,
              this->m_ProjectedGradientTolerance,
              this->m_DisplayDeformedImage,
              this->m_PromptUserAfterDisplay);
          if ( outputBSplineTransform.IsNull() )
            {
            std::cout
              << "Error -- the BSpline fit has failed." << std::endl;
            std::cout
              << "Error -- the BSpline fit has failed." << std::endl;

            m_ActualNumberOfIterations = 1;
            m_PermittedNumberOfIterations = 1;
            }
          else
            {
            // Initialize next level of transformations with previous transform
            // result
            // TransformList.clear();
            // TransformList.push_back(finalTransform);
            m_CurrentGenericTransform = outputBSplineTransform;
            // Now turn of the initiallize code to off
            localInitializeTransformMode = "Off";
              {
              // HACK:  The BSpline optimizer does not return the correct iteration values.
              m_ActualNumberOfIterations = 1;
              m_PermittedNumberOfIterations = 3;
              }
            }
          }
        // AF: ROIBSpline use the bounding box of the input label to initialize
        //     the BSpline grid, instead of using the whole image volume. This is
        //     particularly useful when the region of interest is small wrt the image
        else if ( currentTransformType == "ROIBSpline" )
          {
          //
          // Process the bulkAffineTransform for BSpline's BULK
          //

          AffineTransformType::Pointer bulkAffineTransform =
            AffineTransformType::New();
          bulkAffineTransform->SetIdentity();

          typedef itk::Image< float, 3 > RegisterImageType;

          BSplineTransformType::Pointer outputBSplineTransform =
            BSplineTransformType::New();
          outputBSplineTransform->SetIdentity();

          BSplineTransformType::Pointer initialBSplineTransform =
            BSplineTransformType::New();
          initialBSplineTransform->SetIdentity();

            {
            typedef itk::Image< unsigned char, 3 >                               MaskImageType;
            typedef itk::ImageMaskSpatialObject< MaskImageType::ImageDimension > ImageMaskSpatialObjectType;

            typedef BSplineTransformType::RegionType TransformRegionType;
            typedef TransformRegionType::SizeType    TransformSizeType;
            typedef itk::BSplineDeformableTransformInitializer
              < BSplineTransformType, RegisterImageType > InitializerType;

            ImageMaskSpatialObjectType::Pointer fixedImageMask(
              dynamic_cast< ImageMaskSpatialObjectType * >( m_FixedBinaryVolume.GetPointer() ) );
            ImageMaskSpatialObjectType::Pointer movingImageMask(
              dynamic_cast< ImageMaskSpatialObjectType * >( m_MovingBinaryVolume.GetPointer() ) );

            typedef ImageMaskSpatialObjectType::ImageType                            MaskImageType;
            typedef itk::ResampleImageFilter< MaskImageType, MaskImageType, double > ResampleFilterType;
            ResampleFilterType::Pointer resampler = ResampleFilterType::New();
            resampler->SetTransform(m_CurrentGenericTransform);
            resampler->SetInput( movingImageMask->GetImage() );
            resampler->SetOutputParametersFromImage( fixedImageMask->GetImage() );
            resampler->Update();

            typedef itk::AddImageFilter< MaskImageType, MaskImageType > AddFilterType;
            AddFilterType::Pointer adder = AddFilterType::New();
            adder->SetInput1( fixedImageMask->GetImage() );
            adder->SetInput2( resampler->GetOutput() );
            adder->Update();

            /*
             * typedef itk::ImageFileWriter<MaskImageType> WriterType;
             * WriterType::Pointer writer = WriterType::New();
             * writer->SetFileName( "/tmp/jointMask.nrrd" );
             * writer->SetInput( adder->GetOutput() );
             * writer->Update();
             */

            ImageMaskSpatialObjectType::Pointer jointMask = ImageMaskSpatialObjectType::New();
            jointMask->SetImage( adder->GetOutput() );
            jointMask->ComputeObjectToWorldTransform();

            typename FixedImageType::Pointer    roiImage = FixedImageType::New();
            typename FixedImageType::RegionType roiRegion =
              jointMask->GetAxisAlignedBoundingBoxRegion();
            typename FixedImageType::SpacingType roiSpacing =
              m_FixedVolume->GetSpacing();
            /*
             * std::cout << "Image size: " <<
             *    m_FixedVolume->GetBufferedRegion().GetSize() << std::endl;
             * std::cout << "ROI size: " << roiRegion.GetSize() << std::endl;
             * std::cout << "ROI spacing: " << roiSpacing << std::endl;
             * std::cout << "ROI index: " << roiRegion.GetIndex() << std::endl;
             * std::cout << "ROI size in physical space: " <<
             * roiRegion.GetSize()[0]*roiSpacing[0] << " " <<
             * roiRegion.GetSize()[1]*roiSpacing[1] << " " <<
             * roiRegion.GetSize()[2]*roiSpacing[2] << std::endl;
             */

            typename FixedImageType::PointType roiOriginPt;
            typename FixedImageType::IndexType roiOriginIdx;
            roiOriginIdx.Fill(0);
            m_FixedVolume->TransformIndexToPhysicalPoint(roiRegion.GetIndex(), roiOriginPt);
            roiRegion.SetIndex(roiOriginIdx);
            roiImage->SetRegions(roiRegion);
            roiImage->Allocate();
            roiImage->FillBuffer(1.);
            roiImage->SetSpacing(roiSpacing);
            roiImage->SetOrigin(roiOriginPt);
            roiImage->SetDirection( m_FixedVolume->GetDirection() );

            /*
             * std::cout << "ROI origin: " << roiOriginPt << std::endl;
             * typedef itk::ImageFileWriter<FixedImageType> WriterType;
             * WriterType::Pointer writer = WriterType::New();
             * writer->SetFileName( "/tmp/bsplineroi.nrrd" );
             * writer->SetInput( roiImage );
             * writer->Update();
             */

            InitializerType::Pointer transformInitializer = InitializerType::New();
            transformInitializer->SetTransform(initialBSplineTransform);
            transformInitializer->SetImage(roiImage);
            TransformSizeType tempGridSize;
            tempGridSize[0] = m_SplineGridSize[0];
            tempGridSize[1] = m_SplineGridSize[1];
            tempGridSize[2] = m_SplineGridSize[2];
            transformInitializer->SetGridSizeInsideTheImage(tempGridSize);
            transformInitializer->InitializeTransform();
            }

          if ( m_CurrentGenericTransform.IsNotNull() )
            {
            try
              {
              const std::string transformFileType = m_CurrentGenericTransform->GetNameOfClass();
              if ( transformFileType == "VersorRigid3DTransform" )
                {
                const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< VersorRigid3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(bulkAffineTransform,
                  tempInitializerITKTransform);
                initialBSplineTransform->SetBulkTransform(bulkAffineTransform);
                }
              else if ( transformFileType == "ScaleVersor3DTransform" )
                {
                const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(bulkAffineTransform,
                  tempInitializerITKTransform);
                initialBSplineTransform->SetBulkTransform(bulkAffineTransform);
                }
              else if ( transformFileType == "ScaleSkewVersor3DTransform" )
                {
                const ScaleSkewVersor3DTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< ScaleSkewVersor3DTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(bulkAffineTransform,
                  tempInitializerITKTransform);
                initialBSplineTransform->SetBulkTransform(bulkAffineTransform);
                }
              else if ( transformFileType == "AffineTransform" )
                {
                const AffineTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< AffineTransformType const *const >( m_CurrentGenericTransform.GetPointer() );
                AssignRigid::AssignConvertedTransform(bulkAffineTransform,
                  tempInitializerITKTransform);
                initialBSplineTransform->SetBulkTransform(bulkAffineTransform);
                }
              else if ( transformFileType == "BSplineDeformableTransform" )
                {
                const BSplineTransformType::ConstPointer tempInitializerITKTransform =
                  dynamic_cast< BSplineTransformType const *const >( m_CurrentGenericTransform.GetPointer() );

                initialBSplineTransform->SetBulkTransform(
                  tempInitializerITKTransform->GetBulkTransform() );
                BSplineTransformType::ParametersType tempFixedInitialParameters =
                  tempInitializerITKTransform->GetFixedParameters();
                BSplineTransformType::ParametersType initialFixedParameters =
                  initialBSplineTransform->GetFixedParameters();

                bool checkMatch = true;     // Assume true;
                if ( initialFixedParameters.GetSize() != tempFixedInitialParameters.GetSize() )
                  {
                  checkMatch = false;
                  std::cerr << "ERROR INITILIZATION FIXED PARAMETERS DO NOT MATCH: " << initialFixedParameters.GetSize()
                    << " != " << tempFixedInitialParameters.GetSize() << std::endl;
                  }
                if ( checkMatch )        //  This ramus covers the hypothesis that
                  // the
                  // FixedParameters represent the grid locations of the spline nodes.
                  {
                  for ( unsigned int i = 0; i < initialFixedParameters.GetSize(); i++ )
                    {
                    if ( initialFixedParameters.GetElement(i) != tempFixedInitialParameters.GetElement(i) )
                      {
                      checkMatch = false;
                      std::cerr << "ERROR FIXED PARAMETERS DO NOT MATCH: " << initialFixedParameters.GetElement(i)
                        << " != " << tempFixedInitialParameters.GetElement(i) << std::endl;
                      }
                    }
                  }
                if ( checkMatch )
                  {
                  BSplineTransformType::ParametersType tempInitialParameters =
                    tempInitializerITKTransform->GetParameters();
                  if ( initialBSplineTransform->GetNumberOfParameters() ==
                    tempInitialParameters.Size() )
                    {
                    initialBSplineTransform->SetFixedParameters(
                      tempFixedInitialParameters);
                    initialBSplineTransform->SetParametersByValue(tempInitialParameters);
                    }
                  else
                    {
                    // Error, initializing from wrong size transform parameters;
                    //  Use its bulk transform only?
                    std::cerr
                      << "Trouble using the m_CurrentGenericTransform for initializing a BSPlineDeformableTransform:"
                      << std::endl;
                    std::cerr
                      <<
                      "The initializing BSplineDeformableTransform has a different"
                      << " number of Parameters, than what is required for the requested grid."
                      << std::endl;
                    std::cerr
                      << "BRAINSFit was only able to use the bulk transform that was before it."
                      << std::endl;
                    exit(-1);
                    }
                  }
                else
                  {
                  std::cerr
                    << "ERROR:  initialization BSpline transform does not have the same "
                    << "parameter dimensions as the one currently specified."
                    << std::endl;
                  exit(-1);
                  }
                }
              else
                {
                std::cerr << "ERROR:  Invalid transform initializer type found:  " << transformFileType << std::endl;
                exit(-1);
                }
              }
            catch ( itk::ExceptionObject & excp )
              {
              std::cout << "[FAILED]" << std::endl;
              std::cerr
                << "Error while reading the m_CurrentGenericTransform"
                << std::endl;
              std::cerr << excp << std::endl;
              return;
              }
            }

            {
            // Special optimizations only for the MMI metric
            // that need adjusting based on both the type of metric, and
            // the "dimensionality" of the transform being adjusted.
            typename MattesMutualInformationMetricType::Pointer test_MMICostMetric= dynamic_cast<MattesMutualInformationMetricType *>(this->m_CostMetricObject.GetPointer());
            if(test_MMICostMetric.IsNotNull())
              {
              const bool UseExplicitPDFDerivatives = ( m_UseExplicitPDFDerivativesMode != "ON" || m_UseExplicitPDFDerivativesMode == "AUTO" ) ? false : true;
              test_MMICostMetric->SetUseExplicitPDFDerivatives(UseExplicitPDFDerivatives);
              }
            }
          outputBSplineTransform =
            DoBSpline< RegisterImageType, SpatialObjectType,
            BSplineTransformType >(
              initialBSplineTransform,
              m_FixedVolume, m_MovingVolume,
              this->m_CostMetricObject.GetPointer(),
              this->m_MaxBSplineDisplacement,
              this->m_CostFunctionConvergenceFactor,
              this->m_ProjectedGradientTolerance,
              this->m_DisplayDeformedImage,
              this->m_PromptUserAfterDisplay);
          if ( outputBSplineTransform.IsNull() )
            {
            std::cout
              << "Error -- the BSpline fit has failed." << std::endl;
            std::cout
              << "Error -- the BSpline fit has failed." << std::endl;

            m_ActualNumberOfIterations = 1;
            m_PermittedNumberOfIterations = 1;
            }
          else
            {
            // Initialize next level of transformations with previous transform
            // result
            // TransformList.clear();
            // TransformList.push_back(finalTransform);
            m_CurrentGenericTransform = outputBSplineTransform;
            // Now turn of the initiallize code to off
            localInitializeTransformMode = "Off";
              {
              // HACK:  The BSpline optimizer does not return the correct iteration
              // values.
              m_ActualNumberOfIterations = 1;
              m_PermittedNumberOfIterations = 3;
              }
            }
          }

        else
          {
          std::cout
            << "Error choosing what kind of transform to fit \""
            << currentTransformType << "(" << currentTransformIndex + 1 << " of " << m_TransformType.size() << "). "
            << std::endl;
          std::cout << std::flush << std::endl;
          exit(-1);
          return;
          }

        if ( currentTransformId > m_GenericTransformList.size() - 1 )
          {
          std::cerr << "Out of bounds access for transform vector!" << std::endl;
          exit(-1);
          return;
          }
        m_GenericTransformList[currentTransformId++] = m_CurrentGenericTransform;
        }

      return;
      }

  template <class FixedImageType, class MovingImageType>
    void
    BRAINSFitHelperTemplate<FixedImageType, MovingImageType>::PrintSelf(std::ostream & os, Indent indent) const
      {
      // Superclass::PrintSelf(os,indent);
      os << indent << "FixedVolume:\n"  <<   this->m_FixedVolume << std::endl;
      os << indent << "MovingVolume:\n" <<   this->m_MovingVolume << std::endl;
      if ( this->m_FixedBinaryVolume.IsNotNull() )
        {
        os << indent << "FixedBinaryVolume:\n" << this->m_FixedBinaryVolume << std::endl;
        }
      else
        {
        os << indent << "FixedBinaryVolume: IS NULL" << std::endl;
        }
      if ( this->m_MovingBinaryVolume.IsNotNull() )
        {
        os << indent << "MovingBinaryVolume:\n" << this->m_MovingBinaryVolume << std::endl;
        }
      else
        {
        os << indent << "MovingBinaryVolume: IS NULL" << std::endl;
        }
      os << indent << "NumberOfSamples:      " << this->m_NumberOfSamples << std::endl;

      os << indent << "NumberOfIterations:    [";
      for ( unsigned int q = 0; q < this->m_NumberOfIterations.size(); q++ )
        {
        os << this->m_NumberOfIterations[q] << " ";
        }
      os << "]" << std::endl;
      os << indent << "NumberOfHistogramBins:" << this->m_NumberOfHistogramBins << std::endl;
      os << indent << "MaximumStepLength:    " << this->m_MaximumStepLength << std::endl;
      os << indent << "MinimumStepLength:     [";
      for ( unsigned int q = 0; q < this->m_MinimumStepLength.size(); q++ )
        {
        os << this->m_MinimumStepLength[q] << " ";
        }
      os << "]" << std::endl;
      os << indent << "TransformType:     [";
      for ( unsigned int q = 0; q < this->m_TransformType.size(); q++ )
        {
        os << this->m_TransformType[q] << " ";
        }
      os << "]" << std::endl;

      os << indent << "RelaxationFactor:    " << this->m_RelaxationFactor << std::endl;
      os << indent << "TranslationScale:    " << this->m_TranslationScale << std::endl;
      os << indent << "ReproportionScale:   " << this->m_ReproportionScale << std::endl;
      os << indent << "SkewScale:           " << this->m_SkewScale << std::endl;
      os << indent << "BackgroundFillValue:            " << this->m_BackgroundFillValue << std::endl;
      os << indent << "InitializeTransformMode:        " << this->m_InitializeTransformMode << std::endl;
      os << indent << "MaskInferiorCutOffFromCenter:   " << this->m_MaskInferiorCutOffFromCenter << std::endl;
      os << indent << "ActualNumberOfIterations:       " << this->m_ActualNumberOfIterations << std::endl;
      os << indent << "PermittedNumberOfIterations:       " << this->m_PermittedNumberOfIterations << std::endl;
      // os << indent << "AccumulatedNumberOfIterationsForAllLevels: " <<
      // this->m_AccumulatedNumberOfIterationsForAllLevels << std::endl;

      os << indent << "SplineGridSize:     [";
      for ( unsigned int q = 0; q < this->m_SplineGridSize.size(); q++ )
        {
        os << this->m_SplineGridSize[q] << " ";
        }
      os << "]" << std::endl;

      os << indent << "PermitParameterVariation:     [";
      for ( unsigned int q = 0; q < this->m_PermitParameterVariation.size(); q++ )
        {
        os << this->m_PermitParameterVariation[q] << " ";
        }
      os << "]" << std::endl;

      if ( m_CurrentGenericTransform.IsNotNull() )
        {
        os << indent << "CurrentGenericTransform:\n" << this->m_CurrentGenericTransform << std::endl;
        }
      else
        {
        os << indent << "CurrentGenericTransform: IS NULL" << std::endl;
        }
      }

  template <class FixedImageType, class MovingImageType>
    void
    BRAINSFitHelperTemplate<FixedImageType, MovingImageType>::GenerateData()
      {
      this->StartRegistration();
      }
} // end namespace itk
