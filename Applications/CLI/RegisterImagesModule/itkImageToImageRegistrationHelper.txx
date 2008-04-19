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


#ifndef __ImageToImageRegistrationHelper_txx
#define __ImageToImageRegistrationHelper_txx

#include "itkImageToImageRegistrationHelper.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkInterpolateImageFunction.h"
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

template< class TImage >
ImageToImageRegistrationHelper< TImage >
::ImageToImageRegistrationHelper()
{
  // Data
  m_FixedImage = 0;
  m_MovingImage = 0;

  m_UseFixedImageMaskObject = false;
  m_FixedImageMaskObject = 0;
  m_UseMovingImageMaskObject = false;
  m_MovingImageMaskObject = 0;

  // Process
  m_EnableLoadedRegistration = true;
  m_EnableInitialRegistration = true;
  m_EnableRigidRegistration = true;
  m_EnableAffineRegistration = true;
  m_EnableBSplineRegistration = true;

  m_ExpectedOffsetPixelMagnitude = 20;
  m_ExpectedRotationMagnitude = 0.2;
  m_ExpectedScaleMagnitude = 0.1;
  m_ExpectedSkewMagnitude = 0.01;

  m_SamplingIntensityThreshold = 0;

  m_CompletedInitialization = false;
  m_CompletedStage = PRE_STAGE;
  m_CompletedResampling = false;

  m_CurrentMovingImage = 0;
  m_CurrentMatrixTransform = 0;
  m_CurrentBSplineTransform = 0;

  m_LoadedTransformResampledImage = 0;
  m_MatrixTransformResampledImage = 0;
  m_BSplineTransformResampledImage = 0;

  m_FinalMetricValue = 0;
  m_ReportProgress = false;
  m_UseOverlapAsROI = false;
  m_MinimizeMemory = false;

  // Loaded
  m_LoadedMatrixTransform = 0;
  m_LoadedBSplineTransform = 0;

  // Initial
  m_InitialMethodEnum = INIT_WITH_CENTERS_OF_MASS;
  m_InitialTransform = 0;

  // Rigid
  m_RigidSamplingRatio = 0.05;
  m_RigidTargetError = 0.0001;
  m_RigidMaxIterations = 100;
  m_RigidTransform = 0;
  m_RigidMetricMethodEnum = OptimizedRegistrationMethodType::MATTES_MI_METRIC;
  m_RigidInterpolationMethodEnum = OptimizedRegistrationMethodType::LINEAR_INTERPOLATION;
  m_RigidOptimizationMethodEnum = OptimizedRegistrationMethodType::EVOLUTIONARY_OPTIMIZATION;
  m_RigidMetricValue = 0;


  // Affine
  m_AffineSamplingRatio = 0.05;
  m_AffineTargetError = 0.0001;
  m_AffineMaxIterations = 200;
  m_AffineTransform = 0;
  m_AffineMetricMethodEnum = OptimizedRegistrationMethodType::MATTES_MI_METRIC;
  m_AffineInterpolationMethodEnum = OptimizedRegistrationMethodType::LINEAR_INTERPOLATION;
  m_AffineOptimizationMethodEnum = OptimizedRegistrationMethodType::GRADIENT_OPTIMIZATION;
  m_AffineMetricValue = 0;

  
  // BSpline
  m_BSplineSamplingRatio = 0.20;
  m_BSplineTargetError = 0.0001;
  m_BSplineMaxIterations = 50;
  m_BSplineControlPointPixelSpacing = 10;
  m_BSplineTransform = 0;
  m_BSplineMetricMethodEnum = OptimizedRegistrationMethodType::MATTES_MI_METRIC;
  m_BSplineInterpolationMethodEnum = OptimizedRegistrationMethodType::BSPLINE_INTERPOLATION;
  m_BSplineOptimizationMethodEnum = OptimizedRegistrationMethodType::MULTIRESOLUTION_OPTIMIZATION;
  m_BSplineMetricValue = 0;

}

template< class TImage >
ImageToImageRegistrationHelper< TImage >
::~ImageToImageRegistrationHelper()
{
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::LoadFixedImage( const std::string filename ) 
{
  typedef ImageFileReader< TImage > ImageReaderType;
  
  typename ImageReaderType::Pointer imageReader = ImageReaderType::New();

  imageReader->SetFileName( filename );

  imageReader->Update();

  SetFixedImage( imageReader->GetOutput() );

  m_CompletedStage = PRE_STAGE;

  m_CompletedInitialization = false;
  m_CompletedResampling = false;
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::LoadMovingImage( const std::string filename ) 
{
  typedef ImageFileReader< TImage > ImageReaderType;
  
  typename ImageReaderType::Pointer imageReader = ImageReaderType::New();

  imageReader->SetFileName( filename );

  imageReader->Update();

  SetMovingImage( imageReader->GetOutput() );

  m_CompletedStage = PRE_STAGE;

  m_CompletedInitialization = false;
  m_CompletedResampling = false;
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::SaveImage( const std::string filename, const TImage * image ) 
{
  typedef ImageFileWriter< TImage >    FileWriterType;
  
  typename FileWriterType::Pointer fileWriter = FileWriterType::New();
  fileWriter->SetInput( image );
  fileWriter->SetFileName( filename );
  fileWriter->Update();
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::SetFixedImageMaskObject( typename MaskObjectType::ConstPointer & maskObject )
{
  if( this->m_FixedImageMaskObject.GetPointer() != maskObject.GetPointer() )
    {
    this->m_FixedImageMaskObject = maskObject;

    this->Modified();

    if( maskObject.IsNotNull() )
      {
      m_UseFixedImageMaskObject = true;
      }
    else
      {
      m_UseFixedImageMaskObject = false;
      }
    }
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::SetMovingImageMaskObject( typename MaskObjectType::ConstPointer & maskObject )
{
  if( this->m_MovingImageMaskObject.GetPointer() != maskObject.GetPointer() )
    {
    this->m_MovingImageMaskObject = maskObject;

    this->Modified();

    if( maskObject.IsNotNull() )
      {
      m_UseMovingImageMaskObject = true;
      }
    else
      {
      m_UseMovingImageMaskObject = false;
      }
    }
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::Initialize( void )
{
  //m_LoadedTransform = 0;  Not Initialized - since it is a user parameter
  m_InitialTransform = 0;
  m_RigidTransform = 0;
  m_AffineTransform = 0;
  m_BSplineTransform = 0;

  m_CompletedStage = PRE_STAGE;

  m_CompletedInitialization = true;
  m_CompletedResampling = false;

  m_CurrentMatrixTransform = 0;
  m_CurrentBSplineTransform = 0;

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
  m_LoadedTransformResampledImage = 0;
  m_MatrixTransformResampledImage = 0;
  m_BSplineTransformResampledImage = 0;
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::Update( void )
{
  if( !m_CompletedInitialization )
    {
    Initialize();
    }

  if( m_EnableLoadedRegistration 
      && ( m_LoadedMatrixTransform.IsNotNull() 
           || m_LoadedBSplineTransform.IsNotNull() ) )
    {
    if( m_LoadedTransformResampledImage.IsNotNull() )
      {
      m_CurrentMovingImage = m_LoadedTransformResampledImage;
      }
    else
      {
      m_LoadedTransformResampledImage = ResampleImage( 
                                   m_AffineInterpolationMethodEnum,
                                   m_MovingImage,
                                   m_LoadedMatrixTransform,
                                   m_LoadedBSplineTransform );
      m_CurrentMovingImage = m_LoadedTransformResampledImage;

      }

    m_MatrixTransformResampledImage = 0;
    m_BSplineTransformResampledImage = 0;

    m_CompletedStage = LOAD_STAGE;
    m_CompletedResampling = true;

    m_CurrentMatrixTransform = 0;
    m_CurrentBSplineTransform = 0;
    }

  if( m_EnableInitialRegistration
      && m_InitialMethodEnum != INIT_WITH_NONE
      && m_InitialMethodEnum != INIT_WITH_CURRENT_RESULTS )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "*** INITIAL REGISTRATION ***" << std::endl;
      }

    typename InitialRegistrationMethodType::Pointer reg =
                                          InitialRegistrationMethodType::New();
    reg->SetReportProgress( m_ReportProgress );
    reg->SetMovingImage( m_CurrentMovingImage );
    reg->SetFixedImage( m_FixedImage );
    if( m_UseFixedImageMaskObject )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        reg->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      }
    if( m_UseMovingImageMaskObject )
      {
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        reg->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    switch( m_InitialMethodEnum )
      {
      case INIT_WITH_IMAGE_CENTERS:
        reg->SetNumberOfMoments( 0 );
        break;
      case INIT_WITH_CENTERS_OF_MASS:
        reg->SetNumberOfMoments( 1 );
        break;
      case INIT_WITH_SECOND_MOMENTS:
        reg->SetNumberOfMoments( 2 );
        break;
      default:
        break;
      }

    reg->Update();

    m_InitialTransform = reg->GetAffineTransform();
    m_CurrentMatrixTransform = m_InitialTransform;
    m_CurrentBSplineTransform = 0;

    m_CompletedStage = INIT_STAGE;
    m_CompletedResampling = false;
    }

  typename ImageType::SizeType fixedImageSize;
  fixedImageSize = m_FixedImage->GetLargestPossibleRegion().GetSize();
  unsigned long fixedImageNumPixels = m_FixedImage->GetLargestPossibleRegion()
                                                    .GetNumberOfPixels();

  if( m_EnableRigidRegistration )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "*** RIGID REGISTRATION ***" << std::endl;
      }

    typename RigidRegistrationMethodType::Pointer reg;
    reg = RigidRegistrationMethodType::New();
    reg->SetReportProgress( m_ReportProgress );
    reg->SetMovingImage( m_CurrentMovingImage );
    reg->SetFixedImage( m_FixedImage );
    reg->SetNumberOfSamples( (unsigned int)( m_RigidSamplingRatio 
                                             * fixedImageNumPixels ) );
    if( m_SamplingIntensityThreshold > 0 )
      {
      typedef MinimumMaximumImageCalculator< ImageType >  MinMaxCalcType;
      typename MinMaxCalcType::Pointer calc = MinMaxCalcType::New();
      calc->SetImage( m_FixedImage );
      calc->Compute();
      PixelType fixedImageMax = calc->GetMaximum();
      PixelType fixedImageMin = calc->GetMinimum();

      reg->SetFixedImageSamplesIntensityThreshold( static_cast<PixelType>( 
                                          ( m_SamplingIntensityThreshold 
                                            * (fixedImageMax - fixedImageMin) )
                                          + fixedImageMin ) );
      }

    reg->SetUseOverlapAsROI( m_UseOverlapAsROI );
    reg->SetMinimizeMemory( m_MinimizeMemory );
    reg->SetMaxIterations( m_RigidMaxIterations );
    reg->SetTargetError( m_RigidTargetError );
    if( m_UseFixedImageMaskObject )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        reg->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      }
    if( m_UseMovingImageMaskObject )
      {
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        reg->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    reg->SetMetricMethodEnum( m_RigidMetricMethodEnum );
    reg->SetInterpolationMethodEnum( m_RigidInterpolationMethodEnum );
    reg->SetOptimizationMethodEnum( m_RigidOptimizationMethodEnum );
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
      std::cerr << "ERROR: Only 2 and 3 dimensional images are supported due to rigid registration transforms limitations." << std::endl;
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
    reg->SetTransformParametersScales( scales );
    
    if( m_CurrentMatrixTransform.IsNotNull() )
      {
      reg->GetTypedTransform()->SetCenter( m_CurrentMatrixTransform->GetCenter() );
      reg->GetTypedTransform()->SetMatrix( m_CurrentMatrixTransform->GetMatrix() );
      reg->GetTypedTransform()->SetOffset( m_CurrentMatrixTransform->GetOffset() );
      reg->SetInitialTransformParameters( reg->GetTypedTransform()->GetParameters() );
      }

    reg->Update();

    m_RigidTransform = RigidTransformType::New();
    m_RigidTransform->SetFixedParameters(reg->GetTypedTransform()->GetFixedParameters() );
    m_RigidTransform->SetParametersByValue(reg->GetTypedTransform()->GetParameters() );
    m_CurrentMatrixTransform = reg->GetAffineTransform();
    m_CurrentBSplineTransform = 0;

    m_FinalMetricValue = reg->GetFinalMetricValue();
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

    typename AffineRegistrationMethodType::Pointer reg = AffineRegistrationMethodType::New();
    reg->SetReportProgress( m_ReportProgress );
    reg->SetMovingImage( m_CurrentMovingImage );
    reg->SetFixedImage( m_FixedImage );
    reg->SetNumberOfSamples( (unsigned int)(m_AffineSamplingRatio * fixedImageNumPixels) );
    reg->SetUseOverlapAsROI( m_UseOverlapAsROI );
    reg->SetMinimizeMemory( m_MinimizeMemory );
    reg->SetMaxIterations( m_AffineMaxIterations );
    reg->SetTargetError( m_AffineTargetError );
    if( m_UseFixedImageMaskObject )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        reg->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      }
    if( m_UseMovingImageMaskObject )
      {
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        reg->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    reg->SetMetricMethodEnum( m_AffineMetricMethodEnum );
    reg->SetInterpolationMethodEnum( m_AffineInterpolationMethodEnum );
    reg->SetOptimizationMethodEnum( m_AffineOptimizationMethodEnum );
    typename AffineTransformType::ParametersType scales;
    scales.set_size( ImageDimension*ImageDimension + ImageDimension );
    unsigned int scaleNum = 0;
    for(int d1=0; d1<ImageDimension; d1++)
      {
      for(int d2=0; d2<ImageDimension; d2++)
        {
        if(d1 == d2)
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
    for(int d1=0; d1<ImageDimension; d1++)
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
    reg->SetTransformParametersScales( scales );

    if( m_CurrentMatrixTransform.IsNotNull() )
      {
      reg->GetTypedTransform()->SetCenter( m_CurrentMatrixTransform->GetCenter() );
      reg->GetTypedTransform()->SetMatrix( m_CurrentMatrixTransform->GetMatrix() );
      reg->GetTypedTransform()->SetOffset( m_CurrentMatrixTransform->GetOffset() );
      reg->SetInitialTransformParameters( reg->GetTypedTransform()->GetParameters() );
      }

    reg->Update();

    m_AffineTransform = reg->GetAffineTransform();
    m_CurrentMatrixTransform = m_AffineTransform;
    m_CurrentBSplineTransform = 0;

    m_FinalMetricValue = reg->GetFinalMetricValue();
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
      //this->SaveImage("affineResampled.mha", m_CurrentMovingImage);
      }

    typename BSplineRegistrationMethodType::Pointer reg = BSplineRegistrationMethodType::New();
    reg->SetReportProgress( m_ReportProgress );
    reg->SetMovingImage( m_CurrentMovingImage );
    reg->SetFixedImage( m_FixedImage );
    reg->SetNumberOfSamples( (unsigned int)(m_BSplineSamplingRatio * fixedImageNumPixels) );
    reg->SetUseOverlapAsROI( m_UseOverlapAsROI );
    reg->SetMinimizeMemory( m_MinimizeMemory );
    reg->SetMaxIterations( m_BSplineMaxIterations );
    reg->SetTargetError( m_BSplineTargetError );
    if( m_UseFixedImageMaskObject )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        reg->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      }
    if( m_UseMovingImageMaskObject )
      {
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        reg->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    reg->SetMetricMethodEnum( m_BSplineMetricMethodEnum );
    reg->SetInterpolationMethodEnum( m_BSplineInterpolationMethodEnum );
    reg->SetOptimizationMethodEnum( m_BSplineOptimizationMethodEnum );
    reg->SetNumberOfControlPoints( (int)(fixedImageSize[0] / m_BSplineControlPointPixelSpacing) );

    reg->Update();

    m_BSplineTransform = reg->GetBSplineTransform();
    m_CurrentBSplineTransform = m_BSplineTransform;

    m_FinalMetricValue = reg->GetFinalMetricValue();
    m_BSplineMetricValue = m_FinalMetricValue;

    m_CompletedStage = BSPLINE_STAGE;
    m_CompletedResampling = false;
    }
}


template< class TImage >
typename TImage::ConstPointer
ImageToImageRegistrationHelper< TImage >
::ResampleImage( InterpolationMethodEnumType interpolationMethod,
                 const ImageType * movingImage,
                 const MatrixTransformType * matrixTransform,
                 const BSplineTransformType * bsplineTransform )
{
  typedef InterpolateImageFunction< TImage, double >  InterpolatorType;
  typedef LinearInterpolateImageFunction< TImage, double >  
                                                     LinearInterpolatorType;
  typedef BSplineInterpolateImageFunction< TImage, double >  
                                                     BSplineInterpolatorType;
  typedef WindowedSincInterpolateImageFunction< TImage,
                                                4,
                                                Function::HammingWindowFunction< 4 >,
                                                ConstantBoundaryCondition< TImage >,
                                                double >  
                                                     SincInterpolatorType;
  typedef ResampleImageFilter< TImage, TImage, double >  
                                                     ResampleImageFilterType;

  typename InterpolatorType::Pointer interpolator = 0;

  switch(interpolationMethod)
    {
    case OptimizedRegistrationMethodType::LINEAR_INTERPOLATION:
      interpolator = LinearInterpolatorType::New();
      break;
    case OptimizedRegistrationMethodType::BSPLINE_INTERPOLATION:
      interpolator = BSplineInterpolatorType::New();
      (static_cast< BSplineInterpolatorType * >(interpolator.GetPointer()))->SetSplineOrder( 3 );
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
  
  typename ResampleImageFilterType::Pointer resampler = 
                                             ResampleImageFilterType::New();

  if( movingImage == NULL 
      && matrixTransform == NULL 
      && bsplineTransform == NULL
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
  if( movingImage != NULL)
    {
    mImage = movingImage;

    passedImage = true;
    doLoaded = true;
    doMatrix = true;
    doBSpline = true;
    }

  typename AffineTransformType::ConstPointer aTrans = m_CurrentMatrixTransform.GetPointer();
  typename BSplineTransformType::ConstPointer bTrans = m_CurrentBSplineTransform.GetPointer();
  if( matrixTransform != NULL
      || bsplineTransform != NULL )
    {
    passedImage = true;
    doLoaded = false;
    doMatrix = false;
    doBSpline = false;

    if( matrixTransform != NULL )
      {
      aTrans = matrixTransform;
      doMatrix = true;
      }
    if( bsplineTransform != NULL )
      {
      bTrans = bsplineTransform;
      doBSpline = true;
      }
    }

  interpolator->SetInputImage( mImage );
  resampler->SetInput( mImage );
  resampler->SetInterpolator( interpolator.GetPointer() );
  resampler->SetOutputParametersFromConstImage( m_FixedImage );

  if( doLoaded 
      && ( m_LoadedMatrixTransform.IsNotNull()
           || m_LoadedBSplineTransform.IsNotNull() ) )
    {
    if( m_LoadedMatrixTransform.IsNotNull() )
      {
      // Register using LoadedMatrix
      resampler->SetTransform( m_LoadedMatrixTransform );
      resampler->Update();
      if( !passedImage )
        {
        m_CurrentMovingImage = resampler->GetOutput();
        m_LoadedTransformResampledImage = m_CurrentMovingImage;
        }

      resampled = true;
      mImage = resampler->GetOutput();
      interpolator->SetInputImage( mImage );
      resampler->SetInput( mImage );
      resampler->SetInterpolator( interpolator.GetPointer() );
      }

    if( m_LoadedBSplineTransform.IsNotNull() )
      {
      // Register using LoadedMatrix
      resampler->SetTransform( m_LoadedBSplineTransform );
      resampler->Update();
      if( !passedImage )
        {
        m_CurrentMovingImage = resampler->GetOutput();
        m_LoadedTransformResampledImage = m_CurrentMovingImage;
        }

      resampled = true;
      mImage = resampler->GetOutput();
      interpolator->SetInputImage( mImage );
      resampler->SetInput( mImage );
      resampler->SetInterpolator( interpolator.GetPointer() );
      }
    }

  if( doMatrix && aTrans.IsNotNull() )
    {
    // Register using Matrix
    resampler->SetTransform( aTrans );
    resampler->Update();
    if( !passedImage )
      {
      m_CurrentMovingImage = resampler->GetOutput();
      m_MatrixTransformResampledImage = m_CurrentMovingImage;
      }

    resampled = true;
    mImage = resampler->GetOutput();
    interpolator->SetInputImage( mImage );
    resampler->SetInput( mImage );
    resampler->SetInterpolator( interpolator.GetPointer() );
    }

  if( doBSpline && bTrans.IsNotNull() )
    {
    // Register using BSpline
    resampler->SetTransform( bTrans );
    resampler->Update();
    if( !passedImage )
      {
      m_CurrentMovingImage = resampler->GetOutput();
      m_BSplineTransformResampledImage = m_CurrentMovingImage;
      }

    resampled = true;
    mImage = resampler->GetOutput();
    interpolator->SetInputImage( mImage );
    resampler->SetInput( mImage );
    resampler->SetInterpolator( interpolator.GetPointer() );
    }

  if( !resampled )
    {
    // Warning: No registrations computed
    typename RigidTransformType::Pointer tmpTransform = RigidTransformType::New();
    tmpTransform->SetIdentity();
    resampler->SetTransform( tmpTransform );
    resampler->Update();

    mImage = resampler->GetOutput();
    interpolator->SetInputImage( mImage );
    resampler->SetInput( mImage );
    resampler->SetInterpolator( interpolator.GetPointer() );
    }
  else if( !passedImage )
    {
    m_CompletedResampling = true;
    }

  return mImage;
}

template< class TImage >
typename TImage::ConstPointer
ImageToImageRegistrationHelper< TImage >
::GetFinalMovingImage( InterpolationMethodEnumType interpolationMethod )
{
  return ResampleImage( interpolationMethod );
}

template< class TImage >
typename TImage::ConstPointer
ImageToImageRegistrationHelper< TImage >
::GetFixedToFinalMovingDifferenceImage( InterpolationMethodEnumType interpolationMethod )
{
  typename ImageType::ConstPointer moving = ResampleImage( interpolationMethod );

  typedef itk::SubtractImageFilter< TImage, TImage > SubtractFilterType;
  typename SubtractFilterType::Pointer subtract = SubtractFilterType::New();
  subtract->SetInput1( this->GetFixedImage() );
  subtract->SetInput2( moving );
  subtract->Update();

  return subtract->GetOutput();
}



template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::LoadParameters( const std::string filename )
{
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::SaveParameters( const std::string filename )
{
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::LoadTransform( std::string filename )
{
  typedef TransformFileReader                    TransformReaderType;
  typedef TransformReaderType::TransformListType TransformListType;
  
  TransformReaderType::Pointer transformReader = TransformReaderType::New();
  transformReader->SetFileName( filename );

  TransformFactory< BSplineTransformType >::RegisterTransform();

  transformReader->Update();

  TransformListType * transforms = transformReader->GetTransformList();
  TransformListType::const_iterator transformIt = transforms->begin();
  while(transformIt != transforms->end())
    {
    if(!strcmp((*transformIt)->GetNameOfClass(), "AffineTransform"))
      {
      typename MatrixTransformType::Pointer affine_read = 
            static_cast< MatrixTransformType * >( (*transformIt).GetPointer() );
      typename MatrixTransformType::ConstPointer affine = affine_read.GetPointer();
      SetLoadedMatrixTransform( *affine.GetPointer() );
      }

    if (!strcmp((*transformIt)->GetNameOfClass(), "BSplineDeformableTransform"))
      {
      typename BSplineTransformType::Pointer bspline_read =
            static_cast< BSplineTransformType * >( (*transformIt).GetPointer() );
      typename BSplineTransformType::ConstPointer bspline = bspline_read.GetPointer();
      SetLoadedBSplineTransform( *bspline.GetPointer() );
      }

    ++transformIt;
    }
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::SaveTransform( std::string filename )
{
  typedef TransformFileWriter  TransformWriterType;

  TransformWriterType::Pointer transformWriter = TransformWriterType::New();
  transformWriter->SetFileName( filename );

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

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::SetLoadedMatrixTransform( const MatrixTransformType & tfm )
{
  m_LoadedMatrixTransform = MatrixTransformType::New();
  m_LoadedMatrixTransform->SetIdentity();
  m_LoadedMatrixTransform->SetCenter( tfm.GetCenter() );
  m_LoadedMatrixTransform->SetMatrix( tfm.GetMatrix() );
  m_LoadedMatrixTransform->SetOffset( tfm.GetOffset() );

  m_EnableLoadedRegistration = true;
  m_LoadedTransformResampledImage = 0;
  m_CurrentMovingImage = m_MovingImage;
}
    
template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::SetLoadedBSplineTransform( const BSplineTransformType & tfm )
{
  m_LoadedBSplineTransform = BSplineTransformType::New();
  m_LoadedBSplineTransform->SetGridRegion( tfm.GetGridRegion() );
  m_LoadedBSplineTransform->SetGridSpacing( tfm.GetGridSpacing() );
  m_LoadedBSplineTransform->SetGridOrigin( tfm.GetGridOrigin() );
  m_LoadedBSplineTransform->SetParametersByValue( tfm.GetParameters() );

  m_EnableLoadedRegistration = true;
  m_LoadedTransformResampledImage = 0;
  m_CurrentMovingImage = m_MovingImage;
}
    
template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
::PrintSelfHelper( std::ostream & os, Indent indent,
                    const std::string basename,
                    MetricMethodEnumType metric,
                    InterpolationMethodEnumType interpolation,
                    OptimizationMethodEnumType optimization ) const
{
  switch( metric )
    {
    case OptimizedRegistrationMethodType::MATTES_MI_METRIC:
      {
      os << indent << basename << " Metric Method = MATTES_MI_METRIC" << std::endl;
      break;
      }
    case OptimizedRegistrationMethodType::NORMALIZED_CORRELATION_METRIC:
      {
      os << indent << basename << " Metric Method = CROSS_CORRELATION_METRIC" << std::endl;
      break;
      }
    case OptimizedRegistrationMethodType::MEAN_SQUARED_ERROR_METRIC:
      {
      os << indent << basename << " Metric Method = MEAN_SQUARED_ERROR_METRIC" << std::endl;
      break;
      }
    default:
      {
      os << indent << basename << " Metric Method = UNKNOWN" << std::endl;
      break;
      }
    }
  os << indent << std::endl;
  switch( interpolation )
    {
    case OptimizedRegistrationMethodType::LINEAR_INTERPOLATION:
      {
      os << indent << basename << " Interpolation Method = LINEAR_INTERPOLATION" << std::endl;
      break;
      }
    case OptimizedRegistrationMethodType::BSPLINE_INTERPOLATION:
      {
      os << indent << basename << " Interpolation Method = BSPLINE_INTERPOLATION" << std::endl;
      break;
      }
    case OptimizedRegistrationMethodType::SINC_INTERPOLATION:
      {
      os << indent << basename << " Interpolation Method = SINC_INTERPOLATION" << std::endl;
      break;
      }
    default:
      {
      os << indent << basename << " Interpolation Method = UNKNOWN" << std::endl;
      break;
      }
    }
  os << indent << std::endl;
  switch( optimization )
    {
    case OptimizedRegistrationMethodType::MULTIRESOLUTION_OPTIMIZATION:
      {
      os << indent << basename << " Optimization Method = Multiresolution" << std::endl;
      break;
      }
    case OptimizedRegistrationMethodType::EVOLUTIONARY_OPTIMIZATION:
      {
      os << indent << basename << " Optimization Method = EVOLUTIONARY_OPTIMIZATION" << std::endl;
      break;
      }
    case OptimizedRegistrationMethodType::GRADIENT_OPTIMIZATION:
      {
      os << indent << basename << " Optimization Method = GRADIENT_OPTIMIZATION" << std::endl;
      break;
      }
    default:
      {
      os << indent << basename << " Registration Optimization Method = UNKNOWN" << std::endl;
      break;
      }
    }
}

template< class TImage >
void
ImageToImageRegistrationHelper< TImage >
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
      {
      os << indent << "Initial Registration Enum = INIT_WITH_NONE" << std::endl;
      break;
      }
    case INIT_WITH_CURRENT_RESULTS:
      {
      os << indent << "Initial Registration Enum = INIT_WITH_CURRENT_RESULTS" << std::endl;
      break;
      }
    case INIT_WITH_IMAGE_CENTERS:
      {
      os << indent << "Initial Registration Enum = INIT_WITH_IMAGE_CENTERS" << std::endl;
      break;
      }
    case INIT_WITH_CENTERS_OF_MASS:
      {
      os << indent << "Initial Registration Enum = INIT_WITH_CENTERS_OF_MASS" << std::endl;
      break;
      }
    case INIT_WITH_SECOND_MOMENTS:
      {
      os << indent << "Initial Registration Enum = INIT_WITH_SECOND_MOMENTS" << std::endl;
      break;
      }
    default:
      {
      os << indent << "Initial Registration Enum = UNKNOWN" << std::endl;
      break;
      }
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
                            m_RigidInterpolationMethodEnum,
                            m_RigidOptimizationMethodEnum );
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
                            m_AffineInterpolationMethodEnum,
                            m_AffineOptimizationMethodEnum );
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
                            m_BSplineInterpolationMethodEnum,
                            m_BSplineOptimizationMethodEnum );
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

};

#endif 
