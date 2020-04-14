/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: MomentRegistrator.txx,v $
  Language:  C++
  Date:      $Date: 2007/03/29 17:52:55 $
  Version:   $Revision: 1.6 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef itkImageToImageRegistrationHelper_txx
#define itkImageToImageRegistrationHelper_txx

#include "itkImageToImageRegistrationHelper.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkTestingComparisonImageFilter.h"
#include "itkInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkTransformFactory.h"
#include "itkSubtractImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"

namespace itk
{

template <class TImage>
ImageToImageRegistrationHelper<TImage>
::ImageToImageRegistrationHelper()
{
  // Data
  m_FixedImage = nullptr;
  m_MovingImage = nullptr;

  m_SampleFromOverlap = false;
  m_SampleIntensityPortion = 0.0;

  // Masks
  m_UseFixedImageMaskObject = false;
  m_FixedImageMaskObject = nullptr;
  m_UseMovingImageMaskObject = false;
  m_MovingImageMaskObject = nullptr;

  m_UseRegionOfInterest = false;
  m_RegionOfInterestPoint1.Fill(0);
  m_RegionOfInterestPoint2.Fill(0);

  m_RandomNumberSeed = 0;

  // Process
  m_EnableLoadedRegistration = true;
  m_EnableInitialRegistration = true;
  m_EnableRigidRegistration = true;
  m_EnableAffineRegistration = true;
  m_EnableBSplineRegistration = true;

  // Expected transform magnitude
  m_ExpectedOffsetPixelMagnitude = 10;
  m_ExpectedRotationMagnitude = 0.1;
  m_ExpectedScaleMagnitude = 0.05;
  m_ExpectedSkewMagnitude = 0.01;

  // Current state of the registration pipeline
  m_CompletedInitialization = false;
  m_CompletedStage = PRE_STAGE;
  m_CompletedResampling = false;

  m_CurrentMovingImage = nullptr;
  m_CurrentMatrixTransform = nullptr;
  m_CurrentBSplineTransform = nullptr;

  m_LoadedTransformResampledImage = nullptr;
  m_MatrixTransformResampledImage = nullptr;
  m_BSplineTransformResampledImage = nullptr;

  // Results
  m_FinalMetricValue = 0.0;

  // Baseline
  m_BaselineImage = nullptr;
  m_BaselineNumberOfFailedPixelsTolerance = 1000;
  m_BaselineIntensityTolerance = 10;
  m_BaselineRadiusTolerance = 0;
  m_BaselineResampledMovingImage = nullptr;
  m_BaselineDifferenceImage = nullptr;
  m_BaselineNumberOfFailedPixels = 0;
  m_BaselineTestPassed = false;

  // Progress
  m_ReportProgress = false;

  m_MinimizeMemory = false;

  // Loaded
  m_LoadedMatrixTransform = nullptr;
  m_LoadedBSplineTransform = nullptr;

  // Initial
  m_InitialMethodEnum = INIT_WITH_CENTERS_OF_MASS;
  m_InitialTransform = nullptr;

  // Rigid
  m_RigidSamplingRatio = 0.01;
  m_RigidTargetError = 0.0001;
  m_RigidMaxIterations = 100;
  m_RigidTransform = nullptr;
  m_RigidMetricMethodEnum = OptimizedRegistrationMethodType::MATTES_MI_METRIC;
  m_RigidInterpolationMethodEnum = OptimizedRegistrationMethodType::LINEAR_INTERPOLATION;
  m_RigidMetricValue = 0.0;

  // Affine
  m_AffineSamplingRatio = 0.02;
  m_AffineTargetError = 0.0001;
  m_AffineMaxIterations = 50;
  m_AffineTransform = nullptr;
  m_AffineMetricMethodEnum = OptimizedRegistrationMethodType::MATTES_MI_METRIC;
  m_AffineInterpolationMethodEnum = OptimizedRegistrationMethodType::LINEAR_INTERPOLATION;
  m_AffineMetricValue = 0.0;

  // BSpline
  m_BSplineSamplingRatio = 0.10;
  m_BSplineTargetError = 0.0001;
  m_BSplineMaxIterations = 20;
  m_BSplineControlPointPixelSpacing = 40;
  m_BSplineTransform = nullptr;
  m_BSplineMetricMethodEnum = OptimizedRegistrationMethodType::MATTES_MI_METRIC;
  m_BSplineInterpolationMethodEnum = OptimizedRegistrationMethodType::BSPLINE_INTERPOLATION;
  m_BSplineMetricValue = 0.0;

}

template <class TImage>
ImageToImageRegistrationHelper<TImage>
::~ImageToImageRegistrationHelper() = default;

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::LoadFixedImage( const std::string filename )
{
  typedef ImageFileReader<TImage> ImageReaderType;

  typename ImageReaderType::Pointer imageReader = ImageReaderType::New();

  imageReader->SetFileName( filename );

  imageReader->Update();

  SetFixedImage( imageReader->GetOutput() );

  m_CompletedStage = PRE_STAGE;

  m_CompletedInitialization = false;
  m_CompletedResampling = false;
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::LoadMovingImage( const std::string filename )
{
  typedef ImageFileReader<TImage> ImageReaderType;

  typename ImageReaderType::Pointer imageReader = ImageReaderType::New();

  imageReader->SetFileName( filename );

  imageReader->Update();

  SetMovingImage( imageReader->GetOutput() );

  m_CompletedStage = PRE_STAGE;

  m_CompletedInitialization = false;
  m_CompletedResampling = false;
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SaveImage( const std::string filename, const TImage * image )
{
  typedef ImageFileWriter<TImage> FileWriterType;

  typename FileWriterType::Pointer fileWriter = FileWriterType::New();
  fileWriter->SetUseCompression( true );
  fileWriter->SetInput( image );
  fileWriter->SetFileName( filename );
  fileWriter->Update();
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SetFixedImageMaskObject( const MaskObjectType * maskObject )
{
  if( this->m_FixedImageMaskObject.GetPointer() != maskObject )
    {
    this->m_FixedImageMaskObject = maskObject;

    this->Modified();

    if( maskObject != nullptr )
      {
      m_UseFixedImageMaskObject = true;
      }
    else
      {
      m_UseFixedImageMaskObject = false;
      }
    }
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SetMovingImageMaskObject( const MaskObjectType * maskObject )
{
  if( this->m_MovingImageMaskObject.GetPointer() != maskObject )
    {
    this->m_MovingImageMaskObject = maskObject;

    this->Modified();

    if( maskObject != nullptr )
      {
      m_UseMovingImageMaskObject = true;
      }
    else
      {
      m_UseMovingImageMaskObject = false;
      }
    }
}

template <class TImage>
void ImageToImageRegistrationHelper<TImage>::Initialize()
{
  // m_LoadedTransform = 0;  Not Initialized - since it is a user parameter
  m_InitialTransform = nullptr;
  m_RigidTransform = nullptr;
  m_AffineTransform = nullptr;
  m_BSplineTransform = nullptr;

  m_CompletedStage = PRE_STAGE;

  m_CompletedInitialization = true;
  m_CompletedResampling = false;

  m_CurrentMatrixTransform = nullptr;
  m_CurrentBSplineTransform = nullptr;

  m_FinalMetricValue = 0;
  m_RigidMetricValue = 0;
  m_AffineMetricValue = 0;
  m_BSplineMetricValue = 0;

  if( m_InitialMethodEnum == INIT_WITH_CURRENT_RESULTS )
    {
    m_CurrentMovingImage = GetFinalMovingImage();
    }
  else
    {
    m_CurrentMovingImage = m_MovingImage;
    }

  // Eventually these should only be reset if necessary - that is, if the
  //   only difference is enable BSpline registration, it shouldn't be
  //   necessary to re-run the entire registration pipeline
  m_LoadedTransformResampledImage = nullptr;
  m_MatrixTransformResampledImage = nullptr;
  m_BSplineTransformResampledImage = nullptr;
}

/** This class provides an Update() method to fit the appearance of a
 * ProcessObject API, but it is not a ProcessObject.  */
template <class TImage> void ImageToImageRegistrationHelper<TImage>::Update()
{
  if( !(this->m_CompletedInitialization) )
    {
    this->Initialize();
    }

  if( m_EnableLoadedRegistration
      && ( m_LoadedMatrixTransform.IsNotNull()
           || m_LoadedBSplineTransform.IsNotNull() ) )
    {
    if( m_LoadedTransformResampledImage.IsNotNull() )
      {
      m_CurrentMovingImage = m_LoadedTransformResampledImage;
      if( this->GetReportProgress() )
        {
        std::cout << "*** Using existing loaded transform ***" << std::endl;
        }
      }
    else
      {
      if( this->GetReportProgress() )
        {
        std::cout << "*** Resampling using loaded transform ***" << std::endl;
        }
      m_LoadedTransformResampledImage = ResampleImage(
          m_AffineInterpolationMethodEnum,
          m_MovingImage,
          m_LoadedMatrixTransform,
          m_LoadedBSplineTransform );
      m_CurrentMovingImage = m_LoadedTransformResampledImage;
      // this->SaveImage("transform.mha",m_CurrentMovingImage);
      }

    m_MatrixTransformResampledImage = nullptr;
    m_BSplineTransformResampledImage = nullptr;

    m_CompletedStage = LOAD_STAGE;
    m_CompletedResampling = true;

    m_CurrentMatrixTransform = nullptr;
    m_CurrentBSplineTransform = nullptr;
    }

  if( this->GetReportProgress() )
    {
    std::cout << "*** INITIAL REGISTRATION ***" << std::endl;
    }

  typename InitialRegistrationMethodType::Pointer regInit =
    InitialRegistrationMethodType::New();
  regInit->SetReportProgress( m_ReportProgress );
  regInit->SetMovingImage( m_CurrentMovingImage );
  regInit->SetFixedImage( m_FixedImage );
  if( m_UseFixedImageMaskObject )
    {
    if( m_FixedImageMaskObject.IsNotNull() )
      {
      regInit->SetFixedImageMaskObject( m_FixedImageMaskObject );
      }
    }
  if( m_UseMovingImageMaskObject )
    {
    if( m_MovingImageMaskObject.IsNotNull() )
      {
      regInit->SetMovingImageMaskObject( m_MovingImageMaskObject );
      }
    }
  if( m_EnableInitialRegistration )
    {
    switch( m_InitialMethodEnum )
      {
      case INIT_WITH_NONE:
        regInit->SetComputeCenterOfRotationOnly( true );
        break;
      case INIT_WITH_IMAGE_CENTERS:
        regInit->SetNumberOfMoments( 0 );
        break;
      case INIT_WITH_CENTERS_OF_MASS:
        regInit->SetNumberOfMoments( 1 );
        break;
      case INIT_WITH_SECOND_MOMENTS:
        regInit->SetNumberOfMoments( 2 );
        break;
      case INIT_WITH_LANDMARKS:
        regInit->SetUseLandmarks( true );
        regInit->SetFixedLandmarks( m_FixedLandmarks );
        regInit->SetMovingLandmarks( m_MovingLandmarks );
        break;
      case INIT_WITH_CURRENT_RESULTS:
      default:
        break;
      }
    }
  else
    {
    regInit->SetComputeCenterOfRotationOnly( true );
    }

  regInit->Update();

  m_InitialTransform = regInit->GetAffineTransform();
  m_CurrentMatrixTransform = m_InitialTransform;
  m_CurrentBSplineTransform = nullptr;

  m_CompletedStage = INIT_STAGE;
  m_CompletedResampling = false;

  typename TImage::SizeType fixedImageSize;
  fixedImageSize = m_FixedImage->GetLargestPossibleRegion().GetSize();
  unsigned long fixedImageNumPixels = m_FixedImage->GetLargestPossibleRegion()
    .GetNumberOfPixels();

  if( m_EnableRigidRegistration )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "*** RIGID REGISTRATION ***" << std::endl;
      }

    typename RigidRegistrationMethodType::Pointer regRigid;
    regRigid = RigidRegistrationMethodType::New();
    regRigid->SetRandomNumberSeed( m_RandomNumberSeed );
    regRigid->SetReportProgress( m_ReportProgress );
    regRigid->SetMovingImage( m_CurrentMovingImage );
    regRigid->SetFixedImage( m_FixedImage );
    regRigid->SetNumberOfSamples( (unsigned int)( m_RigidSamplingRatio
                                                  * fixedImageNumPixels ) );
    regRigid->SetSampleFromOverlap( m_SampleFromOverlap );
    regRigid->SetMinimizeMemory( m_MinimizeMemory );
    regRigid->SetMaxIterations( m_RigidMaxIterations );
    regRigid->SetTargetError( m_RigidTargetError );
    if( m_UseFixedImageMaskObject )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        regRigid->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      }
    if( m_UseMovingImageMaskObject )
      {
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        regRigid->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    if( m_SampleIntensityPortion > 0 )
      {
      typedef MinimumMaximumImageCalculator<ImageType> MinMaxCalcType;
      typename MinMaxCalcType::Pointer calc = MinMaxCalcType::New();
      calc->SetImage( m_FixedImage );
      calc->Compute();
      PixelType fixedImageMax = calc->GetMaximum();
      PixelType fixedImageMin = calc->GetMinimum();

      regRigid->SetFixedImageSamplesIntensityThreshold( static_cast<PixelType>(
                                                          ( m_SampleIntensityPortion
                                                            * (fixedImageMax - fixedImageMin) )
                                                          + fixedImageMin ) );
      }
    if( m_UseRegionOfInterest )
      {
      regRigid->SetRegionOfInterest( m_RegionOfInterestPoint1, m_RegionOfInterestPoint2 );
      }
    regRigid->SetSampleFromOverlap( m_SampleFromOverlap );
    regRigid->SetMetricMethodEnum( m_RigidMetricMethodEnum );
    regRigid->SetInterpolationMethodEnum( m_RigidInterpolationMethodEnum );
    typename RigidTransformType::ParametersType scales;
    if( ImageDimension == 2 )
      {
      scales.set_size( 3 );
      scales[0] = 1.0 / m_ExpectedRotationMagnitude;
      scales[1] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
      scales[2] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
      }
    else if( ImageDimension == 3 )
      {
      scales.set_size( 6 );
      scales[0] = 1.0 / m_ExpectedRotationMagnitude;
      scales[1] = 1.0 / m_ExpectedRotationMagnitude;
      scales[2] = 1.0 / m_ExpectedRotationMagnitude;
      scales[3] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
      scales[4] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
      scales[5] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
      }
    else
      {
      std::cerr
      << "ERROR: Only 2 and 3 dimensional images are supported due to rigid registration transforms limitations."
      << std::endl;
      }
    /*
    double minS = scales[0];
    for(unsigned int i=1; i<scales.size(); i++)
      {
      if(scales[i] < minS)
        {
        minS = scales[i];
        }
      }
    if(minS < 1)
      {
      for(unsigned int i=0; i<scales.size(); i++)
        {
        scales[i] /= minS;
        }
      }*/
    regRigid->SetTransformParametersScales( scales );

    if( m_CurrentMatrixTransform.IsNotNull() )
      {
      regRigid->GetTypedTransform()->SetCenter( m_CurrentMatrixTransform->GetCenter() );
      regRigid->GetTypedTransform()->SetMatrix( m_CurrentMatrixTransform->GetMatrix() );
      regRigid->GetTypedTransform()->SetOffset( m_CurrentMatrixTransform->GetOffset() );
      regRigid->SetInitialTransformParameters( regRigid->GetTypedTransform()->GetParameters() );
      regRigid->SetInitialTransformFixedParameters( regRigid->GetTypedTransform()->GetFixedParameters() );
      }

    regRigid->Update();

    m_RigidTransform = RigidTransformType::New();
    m_RigidTransform->SetFixedParameters(regRigid->GetTypedTransform()->GetFixedParameters() );
    // must call GetAffineTransform here because the typed transform
    // is a versor and has only 6 parameters (in this code the type
    // RigidTransform is a 12 parameter transform)
    m_RigidTransform->SetParametersByValue(regRigid->GetAffineTransform()->GetParameters() );
    m_CurrentMatrixTransform = regRigid->GetAffineTransform();
    m_CurrentBSplineTransform = nullptr;

    m_FinalMetricValue = regRigid->GetFinalMetricValue();
    m_RigidMetricValue = m_FinalMetricValue;

    m_CompletedStage = RIGID_STAGE;
    m_CompletedResampling = false;
    }

  if( m_EnableAffineRegistration )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "*** AFFINE REGISTRATION ***" << std::endl;
      }

    typename AffineRegistrationMethodType::Pointer regAff = AffineRegistrationMethodType::New();
    regAff->SetRandomNumberSeed( m_RandomNumberSeed );
    regAff->SetReportProgress( m_ReportProgress );
    regAff->SetMovingImage( m_CurrentMovingImage );
    regAff->SetFixedImage( m_FixedImage );
    regAff->SetNumberOfSamples( (unsigned int)(m_AffineSamplingRatio * fixedImageNumPixels) );
    if( m_UseRegionOfInterest )
      {
      regAff->SetRegionOfInterest( m_RegionOfInterestPoint1, m_RegionOfInterestPoint2 );
      }
    regAff->SetSampleFromOverlap( m_SampleFromOverlap );
    regAff->SetMinimizeMemory( m_MinimizeMemory );
    regAff->SetMaxIterations( m_AffineMaxIterations );
    regAff->SetTargetError( m_AffineTargetError );
    if( m_EnableRigidRegistration )
      {
      regAff->SetUseEvolutionaryOptimization( false );
      }
    regAff->SetTargetError( m_AffineTargetError );
    if( m_UseFixedImageMaskObject )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        regAff->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      }
    if( m_UseMovingImageMaskObject )
      {
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        regAff->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    if( m_SampleIntensityPortion > 0 )
      {
      typedef MinimumMaximumImageCalculator<ImageType> MinMaxCalcType;
      typename MinMaxCalcType::Pointer calc = MinMaxCalcType::New();
      calc->SetImage( m_FixedImage );
      calc->Compute();
      PixelType fixedImageMax = calc->GetMaximum();
      PixelType fixedImageMin = calc->GetMinimum();

      regAff->SetFixedImageSamplesIntensityThreshold( static_cast<PixelType>(
                                                        ( m_SampleIntensityPortion
                                                          * (fixedImageMax - fixedImageMin) )
                                                        + fixedImageMin ) );
      }
    regAff->SetMetricMethodEnum( m_AffineMetricMethodEnum );
    regAff->SetInterpolationMethodEnum( m_AffineInterpolationMethodEnum );
    typename AffineTransformType::ParametersType scales;
    scales.set_size( ImageDimension * ImageDimension + ImageDimension );
    unsigned int scaleNum = 0;
    for( unsigned int d1 = 0; d1 < ImageDimension; d1++ )
      {
      for( unsigned int d2 = 0; d2 < ImageDimension; d2++ )
        {
        if( d1 == d2 )
          {
          scales[scaleNum] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedScaleMagnitude);
          }
        else
          {
          scales[scaleNum] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedSkewMagnitude);
          }
        ++scaleNum;
        }
      }
    for( unsigned int d1 = 0; d1 < ImageDimension; d1++ )
      {
      scales[scaleNum] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
      ++scaleNum;
      }
    /*
    double minS = scales[0];
    for(unsigned int i=1; i<scaleNum; i++)
      {
      if(scales[i] < minS)
        {
        minS = scales[i];
        }
      }
    if(minS < 1)
      {
      for(unsigned int i=0; i<scaleNum; i++)
        {
        scales[i] /= minS;
        }
      }*/
    regAff->SetTransformParametersScales( scales );

    if( m_CurrentMatrixTransform.IsNotNull() )
      {
      regAff->GetTypedTransform()->SetCenter( m_CurrentMatrixTransform->GetCenter() );
      regAff->GetTypedTransform()->SetMatrix( m_CurrentMatrixTransform->GetMatrix() );
      regAff->GetTypedTransform()->SetOffset( m_CurrentMatrixTransform->GetOffset() );
      regAff->SetInitialTransformParameters( regAff->GetTypedTransform()->GetParameters() );
      regAff->SetInitialTransformFixedParameters( regAff->GetTypedTransform()->GetFixedParameters() );
      }

    regAff->Update();

    m_AffineTransform = regAff->GetAffineTransform();
    m_CurrentMatrixTransform = m_AffineTransform;
    m_CurrentBSplineTransform = nullptr;

    m_FinalMetricValue = regAff->GetFinalMetricValue();
    m_AffineMetricValue = m_FinalMetricValue;

    m_CompletedStage = AFFINE_STAGE;
    m_CompletedResampling = false;
    }

  if( m_EnableBSplineRegistration )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "*** BSPLINE REGISTRATION ***" << std::endl;
      }

    if( m_CurrentMatrixTransform.IsNotNull() && !m_CompletedResampling )
      {
      m_CurrentMovingImage = this->ResampleImage();
      m_CompletedResampling = true;
      }

    typename BSplineRegistrationMethodType::Pointer regBspline = BSplineRegistrationMethodType::New();
    regBspline->SetRandomNumberSeed( m_RandomNumberSeed );
    regBspline->SetReportProgress( m_ReportProgress );
    regBspline->SetFixedImage( m_FixedImage );
    regBspline->SetMovingImage( m_CurrentMovingImage );
    regBspline->SetNumberOfSamples( (unsigned int)(m_BSplineSamplingRatio * fixedImageNumPixels) );
    if( m_UseRegionOfInterest )
      {
      regBspline->SetRegionOfInterest( m_RegionOfInterestPoint1, m_RegionOfInterestPoint2 );
      }
    regBspline->SetSampleFromOverlap( m_SampleFromOverlap );
    regBspline->SetMinimizeMemory( m_MinimizeMemory );
    regBspline->SetMaxIterations( m_BSplineMaxIterations );
    regBspline->SetTargetError( m_BSplineTargetError );
    if( m_UseFixedImageMaskObject )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        regBspline->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      }
    if( m_UseMovingImageMaskObject )
      {
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        regBspline->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    if( m_SampleIntensityPortion > 0 )
      {
      typedef MinimumMaximumImageCalculator<ImageType> MinMaxCalcType;
      typename MinMaxCalcType::Pointer calc = MinMaxCalcType::New();
      calc->SetImage( m_FixedImage );
      calc->Compute();
      PixelType fixedImageMax = calc->GetMaximum();
      PixelType fixedImageMin = calc->GetMinimum();

      regBspline->SetFixedImageSamplesIntensityThreshold( static_cast<PixelType>(
                                                            ( m_SampleIntensityPortion
                                                              * (fixedImageMax - fixedImageMin) )
                                                            + fixedImageMin ) );
      }
    regBspline->SetMetricMethodEnum( m_BSplineMetricMethodEnum );
    regBspline->SetInterpolationMethodEnum( m_BSplineInterpolationMethodEnum );
    regBspline->SetNumberOfControlPoints( (int)(fixedImageSize[0] / m_BSplineControlPointPixelSpacing) );

    regBspline->Update();

    m_BSplineTransform = regBspline->GetBSplineTransform();
    m_CurrentBSplineTransform = m_BSplineTransform;

    m_FinalMetricValue = regBspline->GetFinalMetricValue();
    m_BSplineMetricValue = m_FinalMetricValue;

    m_CompletedStage = BSPLINE_STAGE;
    m_CompletedResampling = false;

    if( this->GetReportProgress() )
      {
      std::cout << "BSpline results stored" << std::endl;
      }
    }
  // this->SaveImage("c:/result.mha",m_CurrentMovingImage);
}

template <class TImage>
typename TImage::ConstPointer
ImageToImageRegistrationHelper<TImage>
::ResampleImage( InterpolationMethodEnumType interpolationMethod,
                 const ImageType * movingImage,
                 const MatrixTransformType * matrixTransform,
                 const BSplineTransformType * bsplineTransform,
                 PixelType defaultPixelValue)
{
  typedef InterpolateImageFunction<TImage, double> InterpolatorType;
  typedef NearestNeighborInterpolateImageFunction<TImage, double>
  NearestNeighborInterpolatorType;
  typedef LinearInterpolateImageFunction<TImage, double>
  LinearInterpolatorType;
  typedef BSplineInterpolateImageFunction<TImage, double>
  BSplineInterpolatorType;
  typedef WindowedSincInterpolateImageFunction<TImage,
                                               4,
                                               Function::HammingWindowFunction<4>,
                                               ConstantBoundaryCondition<TImage>,
                                               double>
  SincInterpolatorType;
  typedef ResampleImageFilter<TImage, TImage, double>
  ResampleImageFilterType;

  typename InterpolatorType::Pointer interpolator = nullptr;

  switch( interpolationMethod )
    {
    case OptimizedRegistrationMethodType::NEAREST_NEIGHBOR_INTERPOLATION:
      interpolator = NearestNeighborInterpolatorType::New();
      break;
    case OptimizedRegistrationMethodType::LINEAR_INTERPOLATION:
      interpolator = LinearInterpolatorType::New();
      break;
    case OptimizedRegistrationMethodType::BSPLINE_INTERPOLATION:
      interpolator = BSplineInterpolatorType::New();
      (static_cast<BSplineInterpolatorType *>(interpolator.GetPointer() ) )->SetSplineOrder( 3 );
      break;
    case OptimizedRegistrationMethodType::SINC_INTERPOLATION:
      interpolator = SincInterpolatorType::New();
      break;
    default:
      std::cerr << "ERROR: Interpolation function not supported in itk::ImageToImageRegistrationHelper::ResampleImage"
                << std::endl;
      interpolator = LinearInterpolatorType::New();
      break;
    }

  if( movingImage == nullptr
      && matrixTransform == nullptr
      && bsplineTransform == nullptr
      && m_CompletedResampling )
    {
    return m_CurrentMovingImage;
    }

  bool doLoaded = false;
  bool doMatrix = false;
  bool doBSpline = false;

  switch( m_CompletedStage )
    {
    default:
    case PRE_STAGE:
      break;
    case LOAD_STAGE:
      doLoaded = true;
      break;
    case INIT_STAGE:
    case RIGID_STAGE:
    case AFFINE_STAGE:
      doMatrix = true;
      break;
    case BSPLINE_STAGE:
      doBSpline = true;
      break;
    }

  bool resampled = false;
  bool passedImage = false;
  typename TImage::ConstPointer mImage = m_CurrentMovingImage;
  if( movingImage != nullptr )
    {
    mImage = movingImage;

    passedImage = true;
    doLoaded = true;
    doMatrix = true;
    doBSpline = true;
    }

  typename AffineTransformType::ConstPointer aTrans = m_CurrentMatrixTransform.GetPointer();
  typename BSplineTransformType::ConstPointer bTrans = m_CurrentBSplineTransform.GetPointer();
  if( matrixTransform != nullptr
      || bsplineTransform != nullptr )
    {
    passedImage = true;
    doLoaded = false;
    doMatrix = false;
    doBSpline = false;

    if( matrixTransform != nullptr )
      {
      aTrans = matrixTransform;
      doMatrix = true;
      }
    if( bsplineTransform != nullptr )
      {
      bTrans = bsplineTransform;
      doBSpline = true;
      }
    }

  interpolator->SetInputImage( mImage );

  if( doLoaded
      && ( m_LoadedMatrixTransform.IsNotNull()
           || m_LoadedBSplineTransform.IsNotNull() ) )
    {
    if( m_LoadedMatrixTransform.IsNotNull() )
      {
      if( this->GetReportProgress() )
        {
        std::cout << "Resampling using loaded matrix." << std::endl;
        }
      // Register using LoadedMatrix
      interpolator->SetInputImage( mImage );
      typename ResampleImageFilterType::Pointer resampler =
        ResampleImageFilterType::New();
      resampler->SetInput( mImage );
      resampler->SetInterpolator( interpolator.GetPointer() );
      // We should not be casting away constness here, but SetOutputParametersFromImage
      // Does not change the image.  This is needed to workaround fixes to ITK
      typename ImageType::Pointer tmp = const_cast<ImageType *>(m_FixedImage.GetPointer() );
      resampler->SetOutputParametersFromImage( tmp );
      resampler->SetTransform( m_LoadedMatrixTransform );
      resampler->SetDefaultPixelValue( defaultPixelValue );
      resampler->Update();
      if( !passedImage )
        {
        m_CurrentMovingImage = resampler->GetOutput();
        m_LoadedTransformResampledImage = m_CurrentMovingImage;
        }

      resampled = true;
      mImage = resampler->GetOutput();
      interpolator->SetInputImage( mImage );
      }

    if( m_LoadedBSplineTransform.IsNotNull() )
      {
      if( this->GetReportProgress() )
        {
        std::cout << "Resampling using loaded bspline." << std::endl;
        }
      // Register using LoadedMatrix
      interpolator->SetInputImage( mImage );
      typename ResampleImageFilterType::Pointer resampler =
        ResampleImageFilterType::New();
      resampler->SetInput( mImage );
      resampler->SetInterpolator( interpolator.GetPointer() );
      // We should not be casting away constness here, but SetOutputParametersFromImage
      // Does not change the image.  This is needed to workaround fixes to ITK
      typename ImageType::Pointer tmp = const_cast<ImageType *>(m_FixedImage.GetPointer() );
      resampler->SetOutputParametersFromImage( tmp );
      resampler->SetTransform( m_LoadedBSplineTransform );
      resampler->SetDefaultPixelValue( defaultPixelValue );
      resampler->Update();
      if( !passedImage )
        {
        m_CurrentMovingImage = resampler->GetOutput();
        m_LoadedTransformResampledImage = m_CurrentMovingImage;
        }

      resampled = true;
      mImage = resampler->GetOutput();
      interpolator->SetInputImage( mImage );
      }
    }

  if( doMatrix && aTrans.IsNotNull() )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "Resampling using matrix." << std::endl;
      }
    // Register using Matrix
    interpolator->SetInputImage( mImage );
    typename ResampleImageFilterType::Pointer resampler =
      ResampleImageFilterType::New();
    resampler->SetInput( mImage );
    resampler->SetInterpolator( interpolator.GetPointer() );
    // We should not be casting away constness here, but SetOutputParametersFromImage
    // Does not change the image.  This is needed to workaround fixes to ITK
    typename ImageType::Pointer tmp = const_cast<ImageType *>(m_FixedImage.GetPointer() );
    resampler->SetOutputParametersFromImage( tmp );
    resampler->SetTransform( aTrans );
    resampler->SetDefaultPixelValue( defaultPixelValue );
    resampler->Update();
    if( !passedImage )
      {
      m_CurrentMovingImage = resampler->GetOutput();
      m_MatrixTransformResampledImage = m_CurrentMovingImage;
      }

    resampled = true;
    mImage = resampler->GetOutput();
    interpolator->SetInputImage( mImage );
    }

  if( doBSpline && bTrans.IsNotNull() )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "Resampling using bspline." << std::endl;
      }
    // Register using BSpline
    interpolator->SetInputImage( mImage );
    typename ResampleImageFilterType::Pointer resampler =
      ResampleImageFilterType::New();
    resampler->SetInput( mImage );
    resampler->SetInterpolator( interpolator.GetPointer() );
    // We should not be casting away constness here, but SetOutputParametersFromImage
    // Does not change the image.  This is needed to workaround fixes to ITK
    typename ImageType::Pointer tmp = const_cast<ImageType *>(m_FixedImage.GetPointer() );
    resampler->SetOutputParametersFromImage( tmp );
    resampler->SetTransform( bTrans );
    resampler->SetDefaultPixelValue( defaultPixelValue );
    resampler->Update();
    if( !passedImage )
      {
      m_CurrentMovingImage = resampler->GetOutput();
      m_BSplineTransformResampledImage = m_CurrentMovingImage;
      }

    resampled = true;
    mImage = resampler->GetOutput();
    interpolator->SetInputImage( mImage );
    }

  if( !resampled )
    {
    // Warning: No registrations computed
    if( this->GetReportProgress() )
      {
      std::cout << "Resampling using identity transform." << std::endl;
      }
    typename RigidTransformType::Pointer tmpTransform = RigidTransformType::New();
    tmpTransform->SetIdentity();

    interpolator->SetInputImage( mImage );
    typename ResampleImageFilterType::Pointer resampler =
      ResampleImageFilterType::New();
    resampler->SetInput( mImage );
    resampler->SetInterpolator( interpolator.GetPointer() );
    // We should not be casting away constness here, but SetOutputParametersFromImage
    // Does not change the image.  This is needed to workaround fixes to ITK
    typename ImageType::Pointer tmp = const_cast<ImageType *>(m_FixedImage.GetPointer() );
    resampler->SetOutputParametersFromImage( tmp );
    resampler->SetTransform( tmpTransform );
    resampler->SetDefaultPixelValue( defaultPixelValue );
    resampler->Update();

    mImage = resampler->GetOutput();
    interpolator->SetInputImage( mImage );
    }
  else if( !passedImage )
    {
    m_CompletedResampling = true;
    }

  return mImage;
}

template <class TImage>
typename TImage::ConstPointer
ImageToImageRegistrationHelper<TImage>
::GetFinalMovingImage( InterpolationMethodEnumType interpolationMethod )
{
  return ResampleImage( interpolationMethod );
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::LoadBaselineImage( const std::string filename )
{
  typedef ImageFileReader<TImage> ImageReaderType;

  typename ImageReaderType::Pointer imageReader = ImageReaderType::New();

  imageReader->SetFileName( filename );

  imageReader->Update();

  SetBaselineImage( imageReader->GetOutput() );
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::ComputeBaselineDifference()
{
  if( m_BaselineImage.IsNull() )
    {
    std::cerr << "Error: ComputeBaselineDifference called prior to setting a baseline image."
              << std::endl;
    m_BaselineResampledMovingImage = nullptr;
    m_BaselineDifferenceImage = nullptr;
    m_BaselineNumberOfFailedPixels = 0;
    m_BaselineTestPassed = false;
    return;
    }

  typedef itk::Testing::ComparisonImageFilter<TImage, TImage> DifferenceFilterType;

  typename TImage::ConstPointer imTemp = this->GetFixedImage();
  this->SetFixedImage( this->m_BaselineImage );
  this->m_BaselineResampledMovingImage = this->ResampleImage();
  this->SetFixedImage( imTemp );

  typename DifferenceFilterType::Pointer differ = DifferenceFilterType::New();
  differ->SetValidInput( this->m_BaselineImage );
  differ->SetTestInput( this->m_BaselineResampledMovingImage );
  differ->SetDifferenceThreshold( this->m_BaselineIntensityTolerance );
  differ->SetToleranceRadius( this->m_BaselineRadiusTolerance );
  differ->SetIgnoreBoundaryPixels( true );
  differ->UpdateLargestPossibleRegion();

  this->m_BaselineDifferenceImage = differ->GetOutput();

  this->m_BaselineNumberOfFailedPixels = differ->GetNumberOfPixelsWithDifferences();
  if( this->m_BaselineNumberOfFailedPixels > this->m_BaselineNumberOfFailedPixelsTolerance )
    {
    m_BaselineTestPassed = false;
    }
  else
    {
    m_BaselineTestPassed = true;
    }
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::LoadParameters( const std::string itkNotUsed(filename) )
{
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SaveParameters( const std::string itkNotUsed(filename) )
{
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::LoadTransform( std::string filename )
{
  typedef TransformFileReader                    TransformReaderType;
  typedef TransformReaderType::TransformListType TransformListType;

  TransformReaderType::Pointer transformReader = TransformReaderType::New();
  transformReader->SetFileName( filename );

  TransformFactory<BSplineTransformType>::RegisterTransform();

  transformReader->Update();

  TransformListType *               transforms = transformReader->GetTransformList();
  TransformListType::const_iterator transformIt = transforms->begin();
  while( transformIt != transforms->end() )
    {
    if( !strcmp( (*transformIt)->GetNameOfClass(), "AffineTransform") )
      {
      typename MatrixTransformType::Pointer affine_read =
        static_cast<MatrixTransformType *>( (*transformIt).GetPointer() );
      typename MatrixTransformType::ConstPointer affine = affine_read.GetPointer();
      SetLoadedMatrixTransform( *affine.GetPointer() );
      }

    if( !strcmp( (*transformIt)->GetNameOfClass(), "BSplineDeformableTransform") )
      {
      typename BSplineTransformType::Pointer bspline_read =
        static_cast<BSplineTransformType *>( (*transformIt).GetPointer() );
      typename BSplineTransformType::ConstPointer bspline = bspline_read.GetPointer();
      SetLoadedBSplineTransform( *bspline.GetPointer() );
      }

    ++transformIt;
    }
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SaveTransform( std::string filename )
{
  typedef TransformFileWriter TransformWriterType;

  TransformWriterType::Pointer transformWriter = TransformWriterType::New();
  transformWriter->SetFileName( filename );

  TransformFactory<BSplineTransformType>::RegisterTransform();

  if( m_CurrentMatrixTransform.IsNotNull() )
    {
    transformWriter->SetInput( m_CurrentMatrixTransform );
    if( m_CurrentBSplineTransform.IsNotNull() )
      {
      transformWriter->AddTransform( m_CurrentBSplineTransform );
      }
    transformWriter->Update();
    }
  else if( m_CurrentBSplineTransform.IsNotNull() )
    {
    transformWriter->SetInput( m_CurrentBSplineTransform );
    transformWriter->Update();
    }
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SetLoadedMatrixTransform( const MatrixTransformType & tfm )
{
  m_LoadedMatrixTransform = MatrixTransformType::New();
  m_LoadedMatrixTransform->SetIdentity();
  m_LoadedMatrixTransform->SetCenter( tfm.GetCenter() );
  m_LoadedMatrixTransform->SetMatrix( tfm.GetMatrix() );
  m_LoadedMatrixTransform->SetOffset( tfm.GetOffset() );

  m_EnableLoadedRegistration = true;
  m_LoadedTransformResampledImage = nullptr;
  m_CurrentMovingImage = m_MovingImage;
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SetLoadedBSplineTransform( const BSplineTransformType & tfm )
{
  m_LoadedBSplineTransform = BSplineTransformType::New();
  m_LoadedBSplineTransform->SetFixedParameters( tfm.GetFixedParameters() );
  m_LoadedBSplineTransform->SetParametersByValue( tfm.GetParameters() );

  m_EnableLoadedRegistration = true;
  m_LoadedTransformResampledImage = nullptr;
  m_CurrentMovingImage = m_MovingImage;
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SetRegionOfInterest( const PointType & point1,
                       const PointType & point2 )
{
  m_RegionOfInterestPoint1 = point1;
  m_RegionOfInterestPoint2 = point2;
  m_UseRegionOfInterest = true;
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SetRegionOfInterest( const std::vector<float> & points )
{
  if( points.size() != 2 * ImageDimension )
    {
    throw "Error: size of points passed to SetRegionOfInterest is not twice the image dimension";
    }
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    m_RegionOfInterestPoint1[i] = points[i];
    m_RegionOfInterestPoint2[i] = points[ImageDimension + i];
    }
  m_UseRegionOfInterest = true;
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SetFixedLandmarks( const std::vector<std::vector<float> > & fixedLandmarks )
{
  m_FixedLandmarks.clear();
  for( std::vector<std::vector<float> >::const_iterator i = fixedLandmarks.begin();
       i != fixedLandmarks.end(); ++i )
    {
    LandmarkPointType landmark;
    std::copy(i->begin(), i->end(), landmark.Begin() );
    m_FixedLandmarks.push_back(landmark);
    }
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::SetMovingLandmarks( const std::vector<std::vector<float> > & movingLandmarks )
{
  m_MovingLandmarks.clear();
  for( std::vector<std::vector<float> >::const_iterator i = movingLandmarks.begin();
       i != movingLandmarks.end(); ++i )
    {
    LandmarkPointType landmark;
    std::copy(i->begin(), i->end(), landmark.Begin() );
    m_MovingLandmarks.push_back(landmark);
    }
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::PrintSelfHelper( std::ostream & os, Indent indent,
                   const std::string basename,
                   MetricMethodEnumType metric,
                   InterpolationMethodEnumType interpolation ) const
{
  switch( metric )
    {
    case OptimizedRegistrationMethodType::MATTES_MI_METRIC:
      os << indent << basename << " Metric Method = MATTES_MI_METRIC" << std::endl;
      break;
    case OptimizedRegistrationMethodType::NORMALIZED_CORRELATION_METRIC:
      os << indent << basename << " Metric Method = CROSS_CORRELATION_METRIC" << std::endl;
      break;
    case OptimizedRegistrationMethodType::MEAN_SQUARED_ERROR_METRIC:
      os << indent << basename << " Metric Method = MEAN_SQUARED_ERROR_METRIC" << std::endl;
      break;
    default:
      os << indent << basename << " Metric Method = UNKNOWN" << std::endl;
      break;
    }
  os << indent << std::endl;

  switch( interpolation )
    {
    case OptimizedRegistrationMethodType::NEAREST_NEIGHBOR_INTERPOLATION:
      os << indent << basename << " Interpolation Method = NEAREST_NEIGHBOR_INTERPOLATION" << std::endl;
      break;
    case OptimizedRegistrationMethodType::LINEAR_INTERPOLATION:
      os << indent << basename << " Interpolation Method = LINEAR_INTERPOLATION" << std::endl;
      break;
    case OptimizedRegistrationMethodType::BSPLINE_INTERPOLATION:
      os << indent << basename << " Interpolation Method = BSPLINE_INTERPOLATION" << std::endl;
      break;
    case OptimizedRegistrationMethodType::SINC_INTERPOLATION:
      os << indent << basename << " Interpolation Method = SINC_INTERPOLATION" << std::endl;
      break;
    default:
      os << indent << basename << " Interpolation Method = UNKNOWN" << std::endl;
      break;
    }
}

template <class TImage>
void
ImageToImageRegistrationHelper<TImage>
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  if( m_FixedImage.IsNotNull() )
    {
    os << indent << "Fixed Image = " << m_FixedImage << std::endl;
    }
  if( m_MovingImage.IsNotNull() )
    {
    os << indent << "Moving Image = " << m_MovingImage << std::endl;
    }
  os << indent << std::endl;
  os << indent << "Use region of interest = " << m_UseRegionOfInterest
     << std::endl;
  os << indent << "Region of interest point1 = " << m_RegionOfInterestPoint1
     << std::endl;
  os << indent << "Region of interest point2 = " << m_RegionOfInterestPoint2
     << std::endl;
  os << indent << std::endl;
  os << indent << "Use Fixed Image Mask Object = " << m_UseFixedImageMaskObject << std::endl;
  os << indent << std::endl;
  if( m_FixedImageMaskObject.IsNotNull() )
    {
    os << indent << "Fixed Image Mask Object = " << m_FixedImageMaskObject << std::endl;
    }
  os << indent << "Use Moving Image Mask Object = " << m_UseMovingImageMaskObject << std::endl;
  os << indent << std::endl;
  if( m_MovingImageMaskObject.IsNotNull() )
    {
    os << indent << "Moving Image Mask Object = " << m_MovingImageMaskObject << std::endl;
    }
  os << indent << std::endl;
  os << indent << "Random Number Seed = " << m_RandomNumberSeed << std::endl;
  os << indent << std::endl;
  os << indent << "Enable Loaded Registration = " << m_EnableLoadedRegistration << std::endl;
  os << indent << "Enable Initial Registration = " << m_EnableInitialRegistration << std::endl;
  os << indent << "Enable Rigid Registration = " << m_EnableRigidRegistration << std::endl;
  os << indent << "Enable Affine Registration = " << m_EnableAffineRegistration << std::endl;
  os << indent << "Enable BSpline Registration = " << m_EnableBSplineRegistration << std::endl;
  os << indent << std::endl;
  os << indent << "Expected Offset (in Pixels) Magnitude = " << m_ExpectedOffsetPixelMagnitude << std::endl;
  os << indent << "Expected Rotation Magnitude = " << m_ExpectedRotationMagnitude << std::endl;
  os << indent << "Expected Scale Magnitude = " << m_ExpectedScaleMagnitude << std::endl;
  os << indent << "Expected Skew Magnitude = " << m_ExpectedSkewMagnitude << std::endl;
  os << indent << std::endl;
  os << indent << "Completed Initialization = " << m_CompletedInitialization << std::endl;
  os << indent << "Completed Resampling = " << m_CompletedResampling << std::endl;
  os << indent << std::endl;
  os << indent << "Rigid Metric Value = " << m_RigidMetricValue << std::endl;
  os << indent << "Affine Metric Value = " << m_AffineMetricValue << std::endl;
  os << indent << "BSpline Metric Value = " << m_BSplineMetricValue << std::endl;
  os << indent << "Final Metric Value = " << m_FinalMetricValue << std::endl;
  os << indent << std::endl;
  os << indent << "Report Progress = " << m_ReportProgress << std::endl;
  os << indent << std::endl;
  if( m_CurrentMovingImage.IsNotNull() )
    {
    os << indent << "Current Moving Image = " << m_CurrentMovingImage << std::endl;
    }
  else
    {
    os << indent << "Current Moving Image = NULL" << std::endl;
    }
  if( m_CurrentMatrixTransform.IsNotNull() )
    {
    os << indent << "Current Matrix Transform = " << m_CurrentMatrixTransform << std::endl;
    }
  else
    {
    os << indent << "Current Matrix Transform = NULL" << std::endl;
    }
  if( m_CurrentBSplineTransform.IsNotNull() )
    {
    os << indent << "Current BSpline Transform = " << m_CurrentBSplineTransform << std::endl;
    }
  else
    {
    os << indent << "Current BSpline Transform = NULL" << std::endl;
    }
  os << indent << std::endl;
  if( m_LoadedTransformResampledImage.IsNotNull() )
    {
    os << indent << "Loaded Transform Resampled Image = " << m_LoadedTransformResampledImage << std::endl;
    }
  else
    {
    os << indent << "Loaded Transform Resampled Image = NULL" << std::endl;
    }
  if( m_MatrixTransformResampledImage.IsNotNull() )
    {
    os << indent << "Matrix Transform Resampled Image = " << m_MatrixTransformResampledImage << std::endl;
    }
  else
    {
    os << indent << "Matrix Transform Resampled Image = NULL" << std::endl;
    }
  if( m_BSplineTransformResampledImage.IsNotNull() )
    {
    os << indent << "BSpline Transform Resampled Image = " << m_BSplineTransformResampledImage << std::endl;
    }
  else
    {
    os << indent << "BSpline Transform Resampled Image = NULL" << std::endl;
    }
  os << indent << std::endl;
  if( m_LoadedMatrixTransform.IsNotNull() )
    {
    os << indent << "Loaded Matrix Transform = " << m_LoadedMatrixTransform << std::endl;
    }
  else
    {
    os << indent << "Loaded Matrix Transform = NULL" << std::endl;
    }
  if( m_LoadedBSplineTransform.IsNotNull() )
    {
    os << indent << "Loaded BSpline Transform = " << m_LoadedBSplineTransform << std::endl;
    }
  else
    {
    os << indent << "Loaded BSpline Transform = NULL" << std::endl;
    }
  os << indent << std::endl;

  switch( m_InitialMethodEnum )
    {
    case INIT_WITH_NONE:
      os << indent << "Initial Registration Enum = INIT_WITH_NONE" << std::endl;
      break;
    case INIT_WITH_CURRENT_RESULTS:
      os << indent << "Initial Registration Enum = INIT_WITH_CURRENT_RESULTS" << std::endl;
      break;
    case INIT_WITH_IMAGE_CENTERS:
      os << indent << "Initial Registration Enum = INIT_WITH_IMAGE_CENTERS" << std::endl;
      break;
    case INIT_WITH_CENTERS_OF_MASS:
      os << indent << "Initial Registration Enum = INIT_WITH_CENTERS_OF_MASS" << std::endl;
      break;
    case INIT_WITH_SECOND_MOMENTS:
      os << indent << "Initial Registration Enum = INIT_WITH_SECOND_MOMENTS" << std::endl;
      break;
    default:
      os << indent << "Initial Registration Enum = UNKNOWN" << std::endl;
      break;
    }
  if( m_InitialTransform.IsNotNull() )
    {
    os << indent << "Initial Transform = " << m_InitialTransform << std::endl;
    }
  else
    {
    os << indent << "Initial Transform = NULL" << std::endl;
    }
  os << indent << std::endl;
  os << indent << "Rigid Sampling Ratio = " << m_RigidSamplingRatio << std::endl;
  os << indent << "Rigid Target Error = " << m_RigidTargetError << std::endl;
  os << indent << "Rigid Max Iterations = " << m_RigidMaxIterations << std::endl;
  PrintSelfHelper( os, indent, "Rigid", m_RigidMetricMethodEnum,
                   m_RigidInterpolationMethodEnum );
  os << indent << std::endl;
  if( m_RigidTransform.IsNotNull() )
    {
    os << indent << "Rigid Transform = " << m_RigidTransform << std::endl;
    }
  else
    {
    os << indent << "Rigid Transform = NULL" << std::endl;
    }
  os << indent << std::endl;
  os << indent << "Affine Sampling Ratio = " << m_AffineSamplingRatio << std::endl;
  os << indent << "Affine Target Error = " << m_AffineTargetError << std::endl;
  os << indent << "Affine Max Iterations = " << m_AffineMaxIterations << std::endl;
  PrintSelfHelper( os, indent, "Affine", m_AffineMetricMethodEnum,
                   m_AffineInterpolationMethodEnum );
  os << indent << std::endl;
  if( m_AffineTransform.IsNotNull() )
    {
    os << indent << "Affine Transform = " << m_AffineTransform << std::endl;
    }
  else
    {
    os << indent << "Affine Transform = NULL" << std::endl;
    }
  os << indent << std::endl;
  os << indent << "BSpline Sampling Ratio = " << m_BSplineSamplingRatio << std::endl;
  os << indent << "BSpline Target Error = " << m_BSplineTargetError << std::endl;
  os << indent << "BSpline Max Iterations = " << m_BSplineMaxIterations << std::endl;
  os << indent << "BSpline Control Point Pixel Spacing = " << m_BSplineControlPointPixelSpacing << std::endl;
  PrintSelfHelper( os, indent, "BSpline", m_BSplineMetricMethodEnum,
                   m_BSplineInterpolationMethodEnum );
  os << indent << std::endl;
  if( m_BSplineTransform.IsNotNull() )
    {
    os << indent << "BSpline Transform = " << m_BSplineTransform << std::endl;
    }
  else
    {
    os << indent << "BSpline Transform = NULL" << std::endl;
    }
  os << indent << std::endl;

}

}

#endif
