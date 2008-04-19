/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: ImageRegistrator.h,v $
  Language:  C++
  Date:      $Date: 2006/11/06 14:39:34 $
  Version:   $Revision: 1.15 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __ImageToImageRegistrationHelper_h
#define __ImageToImageRegistrationHelper_h

#include "itkImage.h"
#include "itkCommand.h"

#include "itkImageToImageRegistrationMethod.h"
#include "itkInitialImageToImageRegistrationMethod.h"
#include "itkOptimizedImageToImageRegistrationMethod.h"
#include "itkRigidImageToImageRegistrationMethod.h"
#include "itkAffineImageToImageRegistrationMethod.h"
#include "itkBSplineImageToImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
class ImageToImageRegistrationHelper : public Object
  {

  public:

    typedef ImageToImageRegistrationHelper         Self;
    typedef Object                                 Superclass;
    typedef SmartPointer< Self >                   Pointer;
    typedef SmartPointer< const Self >             ConstPointer;

    itkTypeMacro( ImageToImageRegistrationHelper, Object );

    itkNewMacro( Self );
  
    //
    // Custom Typedefs
    //
    typedef TImage                                 ImageType;

    typedef typename ImageType::PixelType          PixelType;

    itkStaticConstMacro( ImageDimension, unsigned int, 
                         TImage::ImageDimension );

    //
    // Available Registration Methods
    //
    typedef ImageToImageRegistrationMethod< TImage >
                                                   RegistrationMethodType;

    typedef InitialImageToImageRegistrationMethod< TImage >
                                                   InitialRegistrationMethodType;

    typedef OptimizedImageToImageRegistrationMethod< TImage >
                                                   OptimizedRegistrationMethodType;

    typedef RigidImageToImageRegistrationMethod< TImage >
                                                   RigidRegistrationMethodType;

    typedef AffineImageToImageRegistrationMethod< TImage >
                                                   AffineRegistrationMethodType;

    typedef BSplineImageToImageRegistrationMethod< TImage >
                                                   BSplineRegistrationMethodType;

    //
    // Typedefs for the parameters of the registration methods
    //
    typedef typename RegistrationMethodType::MaskObjectType
                                                   MaskObjectType;

    typedef typename OptimizedRegistrationMethodType::MetricMethodEnumType
                                                   MetricMethodEnumType;

    typedef typename OptimizedRegistrationMethodType::InterpolationMethodEnumType
                                                   InterpolationMethodEnumType;

    typedef typename OptimizedRegistrationMethodType::OptimizationMethodEnumType
                                                   OptimizationMethodEnumType;

    enum InitialMethodEnumType { INIT_WITH_NONE,
                                 INIT_WITH_CURRENT_RESULTS,
                                 INIT_WITH_IMAGE_CENTERS,
                                 INIT_WITH_CENTERS_OF_MASS,
                                 INIT_WITH_SECOND_MOMENTS };

    enum RegistrationStageEnumType { PRE_STAGE,
                                     LOAD_STAGE,
                                     INIT_STAGE,
                                     RIGID_STAGE,
                                     AFFINE_STAGE,
                                     BSPLINE_STAGE };

    typedef typename InitialRegistrationMethodType::TransformType
                                                   InitialTransformType;

    typedef typename RigidRegistrationMethodType::TransformType
                                                   RigidTransformType;

    typedef typename AffineRegistrationMethodType::TransformType
                                                   AffineTransformType;

    typedef AffineTransformType                    MatrixTransformType;

    typedef typename BSplineRegistrationMethodType::TransformType
                                                   BSplineTransformType;

    // 
    // Custom Methods
    //
    void LoadFixedImage( const std::string filename );

    itkSetConstObjectMacro( FixedImage, ImageType );
    itkGetConstObjectMacro( FixedImage, ImageType );

    void LoadMovingImage( const std::string filename );

    itkSetConstObjectMacro( MovingImage, ImageType );
    itkGetConstObjectMacro( MovingImage, ImageType );

    void SaveImage( const std::string filename, const ImageType * image );

    //
    itkSetMacro( UseFixedImageMaskObject, bool );
    itkGetConstMacro( UseFixedImageMaskObject, bool );

    void SetFixedImageMaskObject( typename MaskObjectType::ConstPointer & mask );
    itkGetConstObjectMacro( FixedImageMaskObject, MaskObjectType );

    itkSetMacro( UseMovingImageMaskObject, bool );
    itkGetConstMacro( UseMovingImageMaskObject, bool );

    void SetMovingImageMaskObject( typename MaskObjectType::ConstPointer & mask );
    itkGetConstObjectMacro( MovingImageMaskObject, MaskObjectType );

    void Initialize( void );

    void Update( void );

    typename ImageType::ConstPointer  ResampleImage( 
                                    InterpolationMethodEnumType interp=OptimizedRegistrationMethodType::LINEAR_INTERPOLATION,
                                    const ImageType * movingImage=NULL,
                                    const MatrixTransformType * matrixTransform=NULL,
                                    const BSplineTransformType * bsplineTransform=NULL );

    typename ImageType::ConstPointer  GetFinalMovingImage( InterpolationMethodEnumType interp=OptimizedRegistrationMethodType::LINEAR_INTERPOLATION );

    typename ImageType::ConstPointer  GetFixedToFinalMovingDifferenceImage( InterpolationMethodEnumType interp=OptimizedRegistrationMethodType::LINEAR_INTERPOLATION );

    //
    // Process Control
    //
    itkSetMacro( EnableLoadedRegistration, bool );
    itkGetConstMacro( EnableLoadedRegistration, bool );

    itkSetMacro( EnableInitialRegistration, bool );
    itkGetConstMacro( EnableInitialRegistration, bool );

    itkSetMacro( EnableRigidRegistration, bool );
    itkGetConstMacro( EnableRigidRegistration, bool );

    itkSetMacro( EnableAffineRegistration, bool );
    itkGetConstMacro( EnableAffineRegistration, bool );

    itkSetMacro( EnableBSplineRegistration, bool );
    itkGetConstMacro( EnableBSplineRegistration, bool );

    //
    itkSetMacro( ExpectedOffsetPixelMagnitude, double );
    itkGetConstMacro( ExpectedOffsetPixelMagnitude, double );

    itkSetMacro( ExpectedRotationMagnitude, double );
    itkGetConstMacro( ExpectedRotationMagnitude, double );

    itkSetMacro( ExpectedScaleMagnitude, double );
    itkGetConstMacro( ExpectedScaleMagnitude, double );

    itkSetMacro( ExpectedSkewMagnitude, double );
    itkGetConstMacro( ExpectedSkewMagnitude, double );

    itkSetMacro( SamplingIntensityThreshold, double );
    itkGetConstMacro( SamplingIntensityThreshold, double );



    //
    itkGetConstObjectMacro( CurrentMatrixTransform, MatrixTransformType );
    itkGetConstObjectMacro( CurrentBSplineTransform, BSplineTransformType );

    // The image used for registration is updated at certain points in the
    //   registration pipeline for speed and transform composition.
    // Specifically, the image is resmpled using the loaded transforms prior 
    //   to running the initial registration method and the image is resampled
    //   after the affine registration / prior to running bspline registration
    // The result of these resamplings is available as the CurrentMovingImage.
    itkGetConstObjectMacro( CurrentMovingImage, ImageType );
    itkGetConstObjectMacro( LoadedTransformResampledImage, ImageType );
    itkGetConstObjectMacro( MatrixTransformResampledImage, ImageType );
    itkGetConstObjectMacro( BSplineTransformResampledImage, ImageType );

    void LoadParameters( const std::string filename );
    void SaveParameters( const std::string filename );

    itkGetMacro( FinalMetricValue, double );

    itkSetMacro( ReportProgress, bool );
    itkGetMacro( ReportProgress, bool );

    itkSetMacro( UseOverlapAsROI, bool );
    itkGetMacro( UseOverlapAsROI, bool );

    itkSetMacro( MinimizeMemory, bool );
    itkGetMacro( MinimizeMemory, bool );

    //
    // Loaded transforms
    //
    void LoadTransform( const std::string filename );
    void SaveTransform( const std::string filename );

    void SetLoadedMatrixTransform( const MatrixTransformType & tfm );
    itkGetConstObjectMacro( LoadedMatrixTransform, MatrixTransformType );

    void SetLoadedBSplineTransform( const BSplineTransformType & tfm );
    itkGetConstObjectMacro( LoadedBSplineTransform, BSplineTransformType );

    //
    // Initial Parameters
    //
    itkSetMacro( InitialMethodEnum, InitialMethodEnumType );
    itkGetConstMacro( InitialMethodEnum, InitialMethodEnumType );

    //
    // Rigid Parameters
    //
    itkSetMacro( RigidSamplingRatio, double );
    itkGetConstMacro( RigidSamplingRatio, double );
    
    itkSetMacro( RigidTargetError, double );
    itkGetConstMacro( RigidTargetError, double );
    
    itkSetMacro( RigidMaxIterations, unsigned int );
    itkGetConstMacro( RigidMaxIterations, unsigned int );

    itkSetMacro( RigidMetricMethodEnum, MetricMethodEnumType );
    itkGetConstMacro( RigidMetricMethodEnum, MetricMethodEnumType );

    itkSetMacro( RigidInterpolationMethodEnum, InterpolationMethodEnumType );
    itkGetConstMacro( RigidInterpolationMethodEnum, InterpolationMethodEnumType );

    itkSetMacro( RigidOptimizationMethodEnum, OptimizationMethodEnumType );
    itkGetConstMacro( RigidOptimizationMethodEnum, OptimizationMethodEnumType );

    itkGetConstObjectMacro( RigidTransform, RigidTransformType );
    itkGetMacro( RigidMetricValue, double );

    //
    // Affine Parameters
    //
    itkSetMacro( AffineSamplingRatio, double );
    itkGetConstMacro( AffineSamplingRatio, double );
    
    itkSetMacro( AffineTargetError, double );
    itkGetConstMacro( AffineTargetError, double );
    
    itkSetMacro( AffineMaxIterations, unsigned int );
    itkGetConstMacro( AffineMaxIterations, unsigned int );

    itkSetMacro( AffineMetricMethodEnum, MetricMethodEnumType );
    itkGetConstMacro( AffineMetricMethodEnum, MetricMethodEnumType );

    itkSetMacro( AffineInterpolationMethodEnum, InterpolationMethodEnumType );
    itkGetConstMacro( AffineInterpolationMethodEnum, InterpolationMethodEnumType );

    itkSetMacro( AffineOptimizationMethodEnum, OptimizationMethodEnumType );
    itkGetConstMacro( AffineOptimizationMethodEnum, OptimizationMethodEnumType );

    itkGetConstObjectMacro( AffineTransform, AffineTransformType );
    itkGetMacro( AffineMetricValue, double );

    //
    // BSpline Parameters
    //
    itkSetMacro( BSplineSamplingRatio, double );
    itkGetConstMacro( BSplineSamplingRatio, double );
    
    itkSetMacro( BSplineTargetError, double );
    itkGetConstMacro( BSplineTargetError, double );
    
    itkSetMacro( BSplineMaxIterations, unsigned int );
    itkGetConstMacro( BSplineMaxIterations, unsigned int );

    itkSetMacro( BSplineControlPointPixelSpacing, double );
    itkGetConstMacro( BSplineControlPointPixelSpacing, double );

    itkSetMacro( BSplineMetricMethodEnum, MetricMethodEnumType );
    itkGetConstMacro( BSplineMetricMethodEnum, MetricMethodEnumType );

    itkSetMacro( BSplineInterpolationMethodEnum, InterpolationMethodEnumType );
    itkGetConstMacro( BSplineInterpolationMethodEnum, InterpolationMethodEnumType );

    itkSetMacro( BSplineOptimizationMethodEnum, OptimizationMethodEnumType );
    itkGetConstMacro( BSplineOptimizationMethodEnum, OptimizationMethodEnumType );

    itkGetConstObjectMacro( BSplineTransform, BSplineTransformType );
    itkGetMacro( BSplineMetricValue, double );

  protected:

    ImageToImageRegistrationHelper( void );
    virtual ~ImageToImageRegistrationHelper( void );

    void PrintSelfHelper( std::ostream & os, Indent indent,
                          const std::string basename,
                          MetricMethodEnumType metric,
                          InterpolationMethodEnumType interpolation,
                          OptimizationMethodEnumType optimization ) const;
    void PrintSelf( std::ostream & os, Indent indent ) const;

  private:

    ImageToImageRegistrationHelper( const Self & ); // Purposely not implemented
    void operator = ( const Self & );               // Purposely not implemented

    //  Data
    typename ImageType::ConstPointer      m_FixedImage;
    typename ImageType::ConstPointer      m_MovingImage;

    bool                                  m_UseFixedImageMaskObject;
    typename MaskObjectType::ConstPointer m_FixedImageMaskObject;
    bool                                  m_UseMovingImageMaskObject;
    typename MaskObjectType::ConstPointer m_MovingImageMaskObject;

    //  Process
    bool                                  m_EnableLoadedRegistration;
    bool                                  m_EnableInitialRegistration;
    bool                                  m_EnableRigidRegistration;
    bool                                  m_EnableAffineRegistration;
    bool                                  m_EnableBSplineRegistration;

    double                                m_ExpectedOffsetPixelMagnitude;
    double                                m_ExpectedRotationMagnitude;
    double                                m_ExpectedScaleMagnitude;
    double                                m_ExpectedSkewMagnitude;

    double                                m_SamplingIntensityThreshold;

    bool                                  m_CompletedInitialization;
    RegistrationStageEnumType             m_CompletedStage;
    bool                                  m_CompletedResampling;

    typename ImageType::ConstPointer        m_CurrentMovingImage;
    typename MatrixTransformType::Pointer   m_CurrentMatrixTransform;
    typename BSplineTransformType::Pointer  m_CurrentBSplineTransform;

    typename ImageType::ConstPointer        m_LoadedTransformResampledImage;
    typename ImageType::ConstPointer        m_MatrixTransformResampledImage;
    typename ImageType::ConstPointer        m_BSplineTransformResampledImage;

    double                                  m_FinalMetricValue;

    bool                                    m_ReportProgress;

    bool                                    m_UseOverlapAsROI;

    bool                                    m_MinimizeMemory;

    //  Loaded Tansform
    typename MatrixTransformType::Pointer   m_LoadedMatrixTransform;
    typename BSplineTransformType::Pointer  m_LoadedBSplineTransform;

    //  Initial Parameters
    InitialMethodEnumType                   m_InitialMethodEnum;
    typename InitialTransformType::Pointer  m_InitialTransform;

    //  Rigid Parameters
    double                                  m_RigidSamplingRatio;
    double                                  m_RigidTargetError;
    unsigned int                            m_RigidMaxIterations;
    typename RigidTransformType::Pointer    m_RigidTransform;
    MetricMethodEnumType                    m_RigidMetricMethodEnum;
    InterpolationMethodEnumType             m_RigidInterpolationMethodEnum;
    OptimizationMethodEnumType              m_RigidOptimizationMethodEnum;

    double                                  m_RigidMetricValue;


    //  Affine Parameters
    double                                  m_AffineSamplingRatio;
    double                                  m_AffineTargetError;
    unsigned int                            m_AffineMaxIterations;
    typename AffineTransformType::Pointer   m_AffineTransform;
    MetricMethodEnumType                    m_AffineMetricMethodEnum;
    InterpolationMethodEnumType             m_AffineInterpolationMethodEnum;
    OptimizationMethodEnumType              m_AffineOptimizationMethodEnum;

    double                                  m_AffineMetricValue;

    //  BSpline Parameters
    double                                  m_BSplineSamplingRatio;
    double                                  m_BSplineTargetError;
    unsigned int                            m_BSplineMaxIterations;
    double                                  m_BSplineControlPointPixelSpacing;
    typename BSplineTransformType::Pointer  m_BSplineTransform;
    MetricMethodEnumType                    m_BSplineMetricMethodEnum;
    InterpolationMethodEnumType             m_BSplineInterpolationMethodEnum;
    OptimizationMethodEnumType              m_BSplineOptimizationMethodEnum;

    double                                  m_BSplineMetricValue;

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageToImageRegistrationHelper.txx"
#endif


#endif

