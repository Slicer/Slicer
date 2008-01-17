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

namespace itk
{

template< class TImage >
ImageToImageRegistrationHelper< TImage >
::ImageToImageRegistrationHelper()
{
  // Data
  m_FixedImage = 0;
  m_MovingImage = 0;

  m_UseMasks = true;
  m_FixedImageMaskObject = 0;
  m_MovingImageMaskObject = 0;

  // Process
  m_EnableLoadedRegistration = true;
  m_EnableInitialRegistration = true;
  m_EnableRigidRegistration = true;
  m_EnableAffineRegistration = true;
  m_EnableBSplineRegistration = true;

  m_ExpectedOffsetPixelMagnitude = 10;
  m_ExpectedRotationMagnitude = 0.02;
  m_ExpectedScaleMagnitude = 0.1;
  m_ExpectedSkewMagnitude = 0.01;

  m_CompletedInitialization = false;
  m_CompletedRegistration = false;
  m_CompletedResampling = false;

  m_CurrentMovingImage = 0;
  m_CurrentMatrixTransform = 0;
  m_CurrentBSplineTransform = 0;

  m_LoadedTransformResampledImage = 0;
  m_MatrixTransformResampledImage = 0;
  m_BSplineTransformResampledImage = 0;

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


  // Affine
  m_AffineSamplingRatio = 0.05;
  m_AffineTargetError = 0.0001;
  m_AffineMaxIterations = 200;
  m_AffineTransform = 0;
  m_AffineMetricMethodEnum = OptimizedRegistrationMethodType::MATTES_MI_METRIC;
  m_AffineInterpolationMethodEnum = OptimizedRegistrationMethodType::LINEAR_INTERPOLATION;
  m_AffineOptimizationMethodEnum = OptimizedRegistrationMethodType::GRADIENT_OPTIMIZATION;

  
  // BSpline
  m_BSplineSamplingRatio = 0.20;
  m_BSplineTargetError = 0.0001;
  m_BSplineMaxIterations = 1000;
  m_BSplineControlPointPixelSpacing = 30;
  m_BSplineTransform = 0;
  m_BSplineMetricMethodEnum = OptimizedRegistrationMethodType::MATTES_MI_METRIC;
  m_BSplineInterpolationMethodEnum = OptimizedRegistrationMethodType::LINEAR_INTERPOLATION;
  m_BSplineOptimizationMethodEnum = OptimizedRegistrationMethodType::GRADIENT_OPTIMIZATION;

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

  m_CompletedInitialization = false;
  m_CompletedRegistration = false;
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

  m_CompletedInitialization = false;
  m_CompletedRegistration = false;
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
::Initialize( void )
{
  //m_LoadedTransform = 0;  Not Initialized - since it is a user parameter
  m_InitialTransform = 0;
  m_RigidTransform = 0;
  m_AffineTransform = 0;
  m_BSplineTransform = 0;

  m_CompletedInitialization = true;
  m_CompletedRegistration = false;
  m_CompletedResampling = false;

  m_CurrentMatrixTransform = 0;
  m_CurrentBSplineTransform = 0;

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
  bool resamplingNeeded = false;
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
      m_MatrixTransformResampledImage = 0;
      m_BSplineTransformResampledImage = 0;
      m_CompletedRegistration = true;
      m_CompletedResampling = true;
      }
    }
  if( m_EnableInitialRegistration
      && m_InitialMethodEnum != INIT_WITH_NONE
      && m_InitialMethodEnum != INIT_WITH_CURRENT_RESULTS )
    {
    std::cout << "*** INITIAL REGISTRATION ***" << std::endl;

    typename InitialRegistrationMethodType::Pointer reg = InitialRegistrationMethodType::New();
    reg->SetMovingImage( m_CurrentMovingImage );
    reg->SetFixedImage( m_FixedImage );
    if( m_UseMasks )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        reg->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
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
      }

    reg->Update();

    m_InitialTransform = reg->GetTypedTransform();
    m_CurrentMatrixTransform = reg->GetAffineTransform();
    m_CurrentBSplineTransform = 0;
    m_CompletedRegistration = true;
    m_CompletedResampling = false;

    std::cout << "*** INITIAL Transform = " << std::endl << "       "
              << m_CurrentMatrixTransform->GetParameters() << std::endl;
    }

  typename ImageType::SizeType fixedImageSize = m_FixedImage->GetLargestPossibleRegion().GetSize();
  unsigned long fixedImageNumPixels = fixedImageSize[0] * fixedImageSize[1] * fixedImageSize[2];

  if( m_EnableRigidRegistration )
    {
    std::cout << "*** RIGID REGISTRATION ***" << std::endl;

    typename RigidRegistrationMethodType::Pointer reg = RigidRegistrationMethodType::New();
    reg->SetMovingImage( m_CurrentMovingImage );
    reg->SetFixedImage( m_FixedImage );
    reg->SetNumberOfSamples( (unsigned int)(m_RigidSamplingRatio * fixedImageNumPixels) );
    reg->SetMaxIterations( m_RigidMaxIterations );
    reg->SetTargetError( m_RigidTargetError );
    if( m_UseMasks )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        reg->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        reg->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    reg->SetMetricMethodEnum( m_RigidMetricMethodEnum );
    reg->SetInterpolationMethodEnum( m_RigidInterpolationMethodEnum );
    reg->SetOptimizationMethodEnum( m_RigidOptimizationMethodEnum );
    typename RigidTransformType::ParametersType scales;
    scales.set_size( 6 );
    scales[0] = 1.0 / m_ExpectedRotationMagnitude;
    scales[1] = 1.0 / m_ExpectedRotationMagnitude;
    scales[2] = 1.0 / m_ExpectedRotationMagnitude;
    scales[3] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
    scales[4] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
    scales[5] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
    double minS = scales[0];
    for(int i=1; i<6; i++)
      {
      if(scales[i] < minS)
        {
        minS = scales[i];
        }
      }
    if(minS < 1)
      {
      for(int i=0; i<6; i++)
        {
        scales[i] /= minS;
        }
      }
    std::cout << "  Scales = " << scales << std::endl;
    reg->SetTransformParametersScales( scales );
    if( m_CurrentMatrixTransform.IsNotNull() )
      {
      reg->GetTypedTransform()->SetCenter( m_CurrentMatrixTransform->GetCenter() );
      reg->GetTypedTransform()->SetMatrix( m_CurrentMatrixTransform->GetMatrix() );
      reg->GetTypedTransform()->SetOffset( m_CurrentMatrixTransform->GetOffset() );
      reg->SetInitialTransformParameters( reg->GetTypedTransform()->GetParameters() );
      }

    reg->Update();

    m_RigidTransform = reg->GetTypedTransform();
    m_CurrentMatrixTransform = reg->GetAffineTransform();
    m_CurrentBSplineTransform = 0;
    m_CompletedRegistration = true;
    m_CompletedResampling = false;
    
    std::cout << "*** RIGID Transform = " << std::endl << "       "
              << m_CurrentMatrixTransform->GetParameters() << std::endl;
    }

  if( m_EnableAffineRegistration )
    {
    std::cout << "*** AFFINE REGISTRATION ***" << std::endl;

    typename AffineRegistrationMethodType::Pointer reg = AffineRegistrationMethodType::New();
    reg->SetMovingImage( m_CurrentMovingImage );
    reg->SetFixedImage( m_FixedImage );
    reg->SetNumberOfSamples( (unsigned int)(m_AffineSamplingRatio * fixedImageNumPixels) );
    reg->SetMaxIterations( m_AffineMaxIterations );
    reg->SetTargetError( m_AffineTargetError );
    if( m_UseMasks )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        reg->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        reg->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    reg->SetMetricMethodEnum( m_AffineMetricMethodEnum );
    reg->SetInterpolationMethodEnum( m_AffineInterpolationMethodEnum );
    reg->SetOptimizationMethodEnum( m_AffineOptimizationMethodEnum );
    typename AffineTransformType::ParametersType scales;
    scales.set_size( 12 );
    scales[0] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedScaleMagnitude);
    scales[1] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedSkewMagnitude);
    scales[2] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedSkewMagnitude);

    scales[3] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedSkewMagnitude);
    scales[4] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedScaleMagnitude);
    scales[5] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedSkewMagnitude);

    scales[6] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedSkewMagnitude);
    scales[7] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedSkewMagnitude);
    scales[8] = 1.0 / (m_ExpectedRotationMagnitude + m_ExpectedScaleMagnitude);

    scales[9] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
    scales[10] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
    scales[11] = 1.0 / (m_ExpectedOffsetPixelMagnitude * m_FixedImage->GetSpacing()[0]);
    double minS = scales[0];
    for(int i=1; i<12; i++)
      {
      if(scales[i] < minS)
        {
        minS = scales[i];
        }
      }
    if(minS < 1)
      {
      for(int i=0; i<12; i++)
        {
        scales[i] /= minS;
        }
      }
    reg->SetTransformParametersScales( scales );
    std::cout << "  Scales = " << scales << std::endl;

    if( m_CurrentMatrixTransform.IsNotNull() )
      {
      reg->GetTypedTransform()->SetCenter( m_CurrentMatrixTransform->GetCenter() );
      reg->GetTypedTransform()->SetMatrix( m_CurrentMatrixTransform->GetMatrix() );
      reg->GetTypedTransform()->SetOffset( m_CurrentMatrixTransform->GetOffset() );
      reg->SetInitialTransformParameters( reg->GetTypedTransform()->GetParameters() );
      }

    std::cout << "AFFINE UPDATE START" << std::endl;
    reg->Update();
    std::cout << "AFFINE UPDATE END" << std::endl;

    m_AffineTransform = reg->GetTypedTransform();
    m_CurrentMatrixTransform = reg->GetAffineTransform();
    m_CurrentBSplineTransform = 0;
    m_CompletedRegistration = true;
    m_CompletedResampling = false;

    std::cout << "*** AFFINE Transform = " << std::endl << "       "
              << m_CurrentMatrixTransform->GetParameters() << std::endl;
    }

  if( m_EnableBSplineRegistration )
    {
    std::cout << "*** BSPLINE REGISTRATION ***" << std::endl;

    typename BSplineRegistrationMethodType::Pointer reg = BSplineRegistrationMethodType::New();
    reg->SetMovingImage( m_CurrentMovingImage );
    reg->SetFixedImage( m_FixedImage );
    reg->SetNumberOfSamples( (unsigned int)(m_BSplineSamplingRatio * fixedImageNumPixels) );
    reg->SetMaxIterations( m_BSplineMaxIterations );
    reg->SetTargetError( m_BSplineTargetError );
    if( m_UseMasks )
      {
      if( m_FixedImageMaskObject.IsNotNull() )
        {
        reg->SetFixedImageMaskObject( m_FixedImageMaskObject );
        }
      if( m_MovingImageMaskObject.IsNotNull() )
        {
        reg->SetMovingImageMaskObject( m_MovingImageMaskObject );
        }
      }
    reg->SetMetricMethodEnum( m_BSplineMetricMethodEnum );
    reg->SetInterpolationMethodEnum( m_BSplineInterpolationMethodEnum );
    reg->SetOptimizationMethodEnum( m_BSplineOptimizationMethodEnum );
    reg->SetNumberOfControlPoints( (int)(fixedImageSize[0] / m_BSplineControlPointPixelSpacing) );

    if( m_CurrentMatrixTransform.IsNotNull() )
      {
      std::cout << "BSpline: Resampling image prior to registration." << std::endl;
      m_CurrentMovingImage = this->ResampleImage();
      }

    reg->Update();

    m_BSplineTransform = reg->GetTypedTransform();
    m_CurrentBSplineTransform = m_BSplineTransform;
    m_CompletedRegistration = true;
    m_CompletedResampling = false;

    std::cout << "*** BSpline Transform = " << std::endl << "       "
              << m_CurrentBSplineTransform->GetParameters() << std::endl;
    }
}


template< class TImage >
typename TImage::Pointer
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

  resampler->SetInterpolator( interpolator.GetPointer() );
  resampler->SetOutputParametersFromImage( m_FixedImage );

  if( movingImage == NULL 
      && matrixTransform == NULL 
      && bsplineTransform == NULL )
    {
    if( m_CompletedResampling )
      {
      return m_CurrentMovingImage;
      }
    if( m_CompletedRegistration )
      {
      if( m_EnableBSplineRegistration 
          && m_CurrentBSplineTransform.IsNotNull() )
        {
        // Register using BSpline
        interpolator->SetInputImage( m_CurrentMovingImage );
        resampler->SetInput( m_CurrentMovingImage );
        resampler->SetTransform( m_CurrentBSplineTransform );
        resampler->Update();
        m_CurrentMovingImage = resampler->GetOutput();
        m_CompletedResampling = true;
        return m_CurrentMovingImage;
        }
      else if( ( m_EnableInitialRegistration
                 || m_EnableRigidRegistration
                 || m_EnableAffineRegistration )
               && m_CurrentMatrixTransform.IsNotNull() )
        {
        // Register using Matrix
        interpolator->SetInputImage( m_CurrentMovingImage );
        resampler->SetInput( m_CurrentMovingImage );
        resampler->SetTransform( m_CurrentMatrixTransform );
        resampler->Update();
        m_CurrentMovingImage = resampler->GetOutput();
        m_MatrixTransformResampledImage = m_CurrentMovingImage;
        m_CompletedResampling = true;
        return m_CurrentMovingImage;
        }
      else if( m_EnableLoadedRegistration )
        {
        interpolator->SetInputImage( m_MovingImage );
        resampler->SetInput( m_MovingImage );
        if( m_LoadedMatrixTransform.IsNotNull() )
          {
          // Register using LoadedMatrix
          resampler->SetTransform( m_LoadedMatrixTransform );
          resampler->Update();
          m_CurrentMovingImage = resampler->GetOutput();
          m_LoadedTransformResampledImage = m_CurrentMovingImage;
          m_CompletedResampling = true;
          resampler->SetInput( m_CurrentMovingImage );
          }
        if( m_LoadedBSplineTransform.IsNotNull() )
          {
          // Register using LoadedMatrix
          resampler->SetTransform( m_LoadedBSplineTransform );
          resampler->Update();
          m_CurrentMovingImage = resampler->GetOutput();
          m_LoadedTransformResampledImage = m_CurrentMovingImage;
          m_CompletedResampling = true;
          }
        if( !m_CompletedResampling )
          {
          typename RigidTransformType::Pointer tmpTransform = RigidTransformType::New();
          tmpTransform->SetIdentity();
          resampler->SetTransform( tmpTransform );
          resampler->Update();
          m_CurrentMovingImage = resampler->GetOutput();
          m_LoadedTransformResampledImage = m_CurrentMovingImage;
          m_CompletedResampling = true;
          }
        return m_CurrentMovingImage;
        }
      else
        {
        // Warning: No registrations enabled
        typename RigidTransformType::Pointer tmpTransform = RigidTransformType::New();
        tmpTransform->SetIdentity();
        interpolator->SetInputImage( m_MovingImage );
        resampler->SetInput( m_MovingImage );
        resampler->SetTransform( tmpTransform );
        resampler->Update();
        m_CurrentMovingImage = resampler->GetOutput();
        m_LoadedTransformResampledImage = m_CurrentMovingImage;
        m_CompletedResampling = true;
        return m_CurrentMovingImage;
        }
      }
    else
      {
      // Warning: Registration has not been computed - using loaded transforms only
      if( m_EnableLoadedRegistration )
        {
        interpolator->SetInputImage( m_MovingImage );
        resampler->SetInput( m_MovingImage );
        if( m_LoadedMatrixTransform.IsNotNull() )
          {
          // Register using LoadedMatrix
          resampler->SetTransform( m_LoadedMatrixTransform );
          resampler->Update();
          m_CurrentMovingImage = resampler->GetOutput();
          m_LoadedTransformResampledImage = m_CurrentMovingImage;
          m_CompletedResampling = true;
          resampler->SetInput( m_CurrentMovingImage );
          }
        if( m_LoadedBSplineTransform.IsNotNull() )
          {
          // Register using LoadedMatrix
          resampler->SetTransform( m_LoadedBSplineTransform );
          resampler->Update();
          m_CurrentMovingImage = resampler->GetOutput();
          m_LoadedTransformResampledImage = m_CurrentMovingImage;
          m_CompletedResampling = true;
          }
        if( !m_CompletedResampling )
          {
          typename RigidTransformType::Pointer tmpTransform = RigidTransformType::New();
          tmpTransform->SetIdentity();
          resampler->SetTransform( tmpTransform );
          resampler->Update();
          m_CurrentMovingImage = resampler->GetOutput();
          m_LoadedTransformResampledImage = m_CurrentMovingImage;
          m_CompletedResampling = true;
          }
        return m_CurrentMovingImage;
        }
      else
        {
        // Warning: No registrations enabled
        typename RigidTransformType::Pointer tmpTransform = RigidTransformType::New();
        tmpTransform->SetIdentity();
        interpolator->SetInputImage( m_MovingImage );
        resampler->SetInput( m_MovingImage );
        resampler->SetTransform( tmpTransform );
        resampler->Update();
        m_CurrentMovingImage = resampler->GetOutput();
        m_LoadedTransformResampledImage = m_CurrentMovingImage;
        m_CompletedResampling = true;
        return m_CurrentMovingImage;
        }
      }
    }
  if( matrixTransform == NULL 
      && bsplineTransform == NULL )
    {
    // movingImage is not null
    resampler->SetInput( movingImage );
    interpolator->SetInputImage( movingImage );

    typename ImageType::Pointer outputImage = 0;

    if( m_EnableLoadedRegistration )
      {
      if( m_LoadedMatrixTransform.IsNotNull() )
        {
        resampler->SetTransform( m_LoadedMatrixTransform );
        resampler->Update();
        outputImage = resampler->GetOutput();
        resampler->SetInput( outputImage );
        }
      if( m_LoadedBSplineTransform.IsNotNull() )
        {
        resampler->SetTransform( m_LoadedBSplineTransform );
        resampler->Update();
        outputImage = resampler->GetOutput();
        resampler->SetInput( outputImage );
        }
      }
    if( outputImage.IsNull() )
      {
      // Warning: No registrations enabled
      typename RigidTransformType::Pointer tmpTransform = RigidTransformType::New();
      tmpTransform->SetIdentity();
      resampler->SetTransform( tmpTransform );
      resampler->Update();
      outputImage = resampler->GetOutput();
      }

    return outputImage;
    }
  else
    {
    if(movingImage != NULL)
      {
      resampler->SetInput( movingImage );
      interpolator->SetInputImage( movingImage );
      }
    else
      {
      resampler->SetInput( m_MovingImage );
      interpolator->SetInputImage( m_MovingImage );
      }

    typename ImageType::Pointer outputImage = 0;
    if( matrixTransform != NULL )
      {
      resampler->SetTransform( matrixTransform );
      resampler->Update();
      outputImage = resampler->GetOutput();
      resampler->SetInput( outputImage );
      }
    if( bsplineTransform != NULL )
      {
      resampler->SetTransform( bsplineTransform );
      resampler->Update();
      outputImage = resampler->GetOutput();
      }

    return outputImage;
    }
}

template< class TImage >
typename TImage::Pointer
ImageToImageRegistrationHelper< TImage >
::GetFinalMovingImage( InterpolationMethodEnumType interpolationMethod )
{
  return ResampleImage( interpolationMethod );
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
  m_LoadedBSplineTransform->SetParameters( tfm.GetParameters() );

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
  os << indent << "Use Masks = " << m_UseMasks << std::endl;
  os << indent << std::endl;
  if( m_FixedImageMaskObject.IsNotNull() )
    {
    os << indent << "Fixed Image Mask Object = " << m_FixedImageMaskObject << std::endl;
    }
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
  os << indent << "Completed Registration = " << m_CompletedRegistration << std::endl;
  os << indent << "Completed Resampling = " << m_CompletedResampling << std::endl;
  os << indent << std::endl;
  if( m_CurrentMovingImage.IsNotNull() )
    {
    os << indent << "Current Moving Image = " << m_CurrentMovingImage << std::endl;
    }
  if( m_CurrentMatrixTransform.IsNotNull() )
    {
    os << indent << "Current Matrix Transform = " << m_CurrentMatrixTransform << std::endl;
    }
  if( m_CurrentBSplineTransform.IsNotNull() )
    {
    os << indent << "Current BSpline Transform = " << m_CurrentBSplineTransform << std::endl;
    }
  os << indent << std::endl;
  if( m_LoadedTransformResampledImage.IsNotNull() )
    {
    os << indent << "Loaded Transform Resampled Image = " << m_LoadedTransformResampledImage << std::endl;
    }
  if( m_MatrixTransformResampledImage.IsNotNull() )
    {
    os << indent << "Matrix Transform Resampled Image = " << m_MatrixTransformResampledImage << std::endl;
    }
  if( m_BSplineTransformResampledImage.IsNotNull() )
    {
    os << indent << "BSpline Transform Resampled Image = " << m_BSplineTransformResampledImage << std::endl;
    }
  os << indent << std::endl;
  if( m_LoadedMatrixTransform.IsNotNull() )
    {
    os << indent << "Loaded Matrix Transform = " << m_LoadedMatrixTransform << std::endl;
    }
  if( m_LoadedBSplineTransform.IsNotNull() )
    {
    os << indent << "Loaded BSpline Transform = " << m_LoadedBSplineTransform << std::endl;
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
  os << indent << std::endl;

}

};

#endif 
