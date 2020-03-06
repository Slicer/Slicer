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

#ifndef itkImageToImageRegistrationHelper_h
#define itkImageToImageRegistrationHelper_h

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

template <class TImage>
class ImageToImageRegistrationHelper : public Object
{

public:

  typedef ImageToImageRegistrationHelper Self;
  typedef Object                         Superclass;
  typedef SmartPointer<Self>             Pointer;
  typedef SmartPointer<const Self>       ConstPointer;

  itkTypeMacro( ImageToImageRegistrationHelper, Object );

  itkNewMacro( Self );

  //
  // Custom Typedefs
  //
  typedef TImage ImageType;

  typedef typename TImage::PixelType PixelType;

  itkStaticConstMacro( ImageDimension, unsigned int,
                       TImage::ImageDimension );

  //
  // Available Registration Methods
  //
  typedef ImageToImageRegistrationMethod<TImage>
  RegistrationMethodType;

  typedef InitialImageToImageRegistrationMethod<TImage>
  InitialRegistrationMethodType;

  typedef OptimizedImageToImageRegistrationMethod<TImage>
  OptimizedRegistrationMethodType;

  typedef RigidImageToImageRegistrationMethod<TImage>
  RigidRegistrationMethodType;

  typedef AffineImageToImageRegistrationMethod<TImage>
  AffineRegistrationMethodType;

  typedef BSplineImageToImageRegistrationMethod<TImage>
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

  enum InitialMethodEnumType { INIT_WITH_NONE,
                               INIT_WITH_CURRENT_RESULTS,
                               INIT_WITH_IMAGE_CENTERS,
                               INIT_WITH_CENTERS_OF_MASS,
                               INIT_WITH_SECOND_MOMENTS,
                               INIT_WITH_LANDMARKS };

  enum RegistrationStageEnumType { PRE_STAGE,
                                   LOAD_STAGE,
                                   INIT_STAGE,
                                   RIGID_STAGE,
                                   AFFINE_STAGE,
                                   BSPLINE_STAGE };

  typedef typename InitialRegistrationMethodType::TransformType
  InitialTransformType;

  typedef std::vector<std::vector<float>  > LandmarkVectorType;

  typedef typename TImage::PointType PointType;

  typedef typename RigidRegistrationMethodType::TransformType
  RigidTransformType;

  typedef typename AffineRegistrationMethodType::TransformType
  AffineTransformType;

  typedef AffineTransformType MatrixTransformType;

  typedef typename BSplineRegistrationMethodType::TransformType
  BSplineTransformType;

  //
  // Custom Methods
  //

  // **************
  // **************
  //  Specify the fixed and moving images
  // **************
  // **************
  void LoadFixedImage( const std::string filename );

  itkSetConstObjectMacro( FixedImage, TImage );
  itkGetConstObjectMacro( FixedImage, TImage );

  void LoadMovingImage( const std::string filename );

  itkSetConstObjectMacro( MovingImage, TImage );
  itkGetConstObjectMacro( MovingImage, TImage );

  // **************
  //  Generic file-save function
  // **************
  void SaveImage( const std::string filename, const TImage * image );

  itkSetMacro( RandomNumberSeed, unsigned int );
  itkGetMacro( RandomNumberSeed, unsigned int );

  // **************
  // **************
  //  Specify how the fixed image should be sampled when computing the metric and
  //    what ROI of the moving image is valid
  // **************
  // **************
  itkSetMacro( UseFixedImageMaskObject, bool );
  itkGetConstMacro( UseFixedImageMaskObject, bool );
  itkBooleanMacro( UseFixedImageMaskObject );

  void SetFixedImageMaskObject( const MaskObjectType * mask );

  itkGetConstObjectMacro( FixedImageMaskObject, MaskObjectType );

  itkSetMacro( UseMovingImageMaskObject, bool );
  itkGetConstMacro( UseMovingImageMaskObject, bool );
  itkBooleanMacro( UseMovingImageMaskObject );

  void SetMovingImageMaskObject( const MaskObjectType * mask );

  itkGetConstObjectMacro( MovingImageMaskObject, MaskObjectType );

  itkSetMacro(UseRegionOfInterest, bool);
  itkGetMacro(UseRegionOfInterest, bool);
  itkSetMacro(RegionOfInterestPoint1, PointType);
  itkGetMacro(RegionOfInterestPoint1, PointType);
  itkSetMacro(RegionOfInterestPoint2, PointType);
  itkGetMacro(RegionOfInterestPoint2, PointType);
  void SetRegionOfInterest( const PointType & point1, const PointType & point2 );

  void SetRegionOfInterest( const std::vector<float> & points );

  // **************
  //  Initialize the moving image mask as the region of initial overlap
  //  between the fixed and moving images
  // **************
  itkSetMacro( SampleFromOverlap, bool );
  itkGetMacro( SampleFromOverlap, bool );
  itkBooleanMacro( SampleFromOverlap);

  itkSetMacro( SampleIntensityPortion, double );
  itkGetConstMacro( SampleIntensityPortion, double );

  // **************
  // **************
  //  Update
  // **************
  // **************
  void Initialize();

  /** This class provides an Update() method to fit the appearance of a
   * ProcessObject API, but it is not a ProcessObject.  */
  void Update();

  // **************
  // **************
  //  Resample
  // **************
  // **************
  typename TImage::ConstPointer  ResampleImage(
    InterpolationMethodEnumType interp
      = OptimizedRegistrationMethodType
        ::LINEAR_INTERPOLATION, const TImage * movingImage = nullptr,
    const MatrixTransformType * matrixTransform = nullptr, const BSplineTransformType * bsplineTransform = nullptr,
    PixelType defaultPixelValue = 0 );

  // Returns the moving image resampled into the space of the fixed image
  typename TImage::ConstPointer  GetFinalMovingImage(InterpolationMethodEnumType interp
                                                       = OptimizedRegistrationMethodType
                                                         ::LINEAR_INTERPOLATION );

  // **************
  // **************
  // Compute registration "accuracy" by comparing a resampled moving image
  // with a baseline image.
  // **************
  // **************

  // Specify the baseline image.
  void LoadBaselineImage( const std::string filename );

  itkSetConstObjectMacro( BaselineImage, TImage );

  // Bound the required accuracy for the registration test to "pass"
  itkSetMacro( BaselineNumberOfFailedPixelsTolerance,  unsigned int );
  itkSetMacro( BaselineIntensityTolerance, PixelType );
  itkSetMacro( BaselineRadiusTolerance, unsigned int );

  // Must be called after setting the BaselineImage in order to resample
  //   the moving image into the BaselineImage space, compute differences,
  //   and determine if it passed the test within the specified tolerances
  void ComputeBaselineDifference();

  itkGetConstObjectMacro( BaselineDifferenceImage, TImage );
  itkGetConstObjectMacro( BaselineResampledMovingImage, TImage );
  itkGetMacro( BaselineNumberOfFailedPixels, unsigned int );
  itkGetMacro( BaselineTestPassed, bool );

  // **************
  // **************
  // Process Control
  // **************
  // **************

  // **************
  // Control which steps of the registration pipeline are applied
  // **************
  itkSetMacro( EnableLoadedRegistration, bool );
  itkGetConstMacro( EnableLoadedRegistration, bool );
  itkBooleanMacro( EnableLoadedRegistration );

  itkSetMacro( EnableInitialRegistration, bool );
  itkGetConstMacro( EnableInitialRegistration, bool );
  itkBooleanMacro( EnableInitialRegistration );

  itkSetMacro( EnableRigidRegistration, bool );
  itkGetConstMacro( EnableRigidRegistration, bool );
  itkBooleanMacro( EnableRigidRegistration );

  itkSetMacro( EnableAffineRegistration, bool );
  itkGetConstMacro( EnableAffineRegistration, bool );
  itkBooleanMacro( EnableAffineRegistration );

  itkSetMacro( EnableBSplineRegistration, bool );
  itkGetConstMacro( EnableBSplineRegistration, bool );
  itkBooleanMacro( EnableBSplineRegistration );

  // **************
  // Specify the expected magnitudes within the transform.  Used to
  //   guide the operating space of the optimizers
  // **************
  itkSetMacro( ExpectedOffsetPixelMagnitude, double );
  itkGetConstMacro( ExpectedOffsetPixelMagnitude, double );

  itkSetMacro( ExpectedRotationMagnitude, double );
  itkGetConstMacro( ExpectedRotationMagnitude, double );

  itkSetMacro( ExpectedScaleMagnitude, double );
  itkGetConstMacro( ExpectedScaleMagnitude, double );

  itkSetMacro( ExpectedSkewMagnitude, double );
  itkGetConstMacro( ExpectedSkewMagnitude, double );

  // **************
  //  Return the current product of the registration pipeline
  // **************
  itkGetConstObjectMacro( CurrentMatrixTransform, MatrixTransformType );
  itkGetConstObjectMacro( CurrentBSplineTransform, BSplineTransformType );

  // The image used for registration is updated at certain points in the
  //   registration pipeline for speed and transform composition.
  // Specifically, the image is resmpled using the loaded transforms prior
  //   to running the initial registration method and the image is resampled
  //   after the affine registration / prior to running bspline registration
  // The result of these resamplings is available as the CurrentMovingImage.
  itkGetConstObjectMacro( CurrentMovingImage, TImage );
  itkGetConstObjectMacro( LoadedTransformResampledImage, TImage );
  itkGetConstObjectMacro( MatrixTransformResampledImage, TImage );
  itkGetConstObjectMacro( BSplineTransformResampledImage, TImage );

  // **************
  //  Not implemented at this time :(
  // **************
  void LoadParameters( const std::string filename );

  void SaveParameters( const std::string filename );

  // **************
  //  Final metric value after the pipeline has completed
  // **************
  itkGetMacro( FinalMetricValue, double );

  // **************
  //  Determine if progress messages should be sent to cout
  // **************
  itkSetMacro( ReportProgress, bool );
  itkGetMacro( ReportProgress, bool );
  itkBooleanMacro( ReportProgress );

  itkSetMacro( MinimizeMemory, bool );
  itkGetMacro( MinimizeMemory, bool );
  itkBooleanMacro( MinimizeMemory );

  //
  // Loaded transforms parameters
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
  void SetFixedLandmarks( const LandmarkVectorType & fixedLandmarks );

  void SetMovingLandmarks( const LandmarkVectorType & movingLandmarks );

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

  itkGetConstObjectMacro( BSplineTransform, BSplineTransformType );
  itkGetMacro( BSplineMetricValue, double );
protected:
  ImageToImageRegistrationHelper();
  ~ImageToImageRegistrationHelper() override;

  void PrintSelfHelper( std::ostream & os, Indent indent, const std::string basename, MetricMethodEnumType metric,
                        InterpolationMethodEnumType interpolation ) const;

  void PrintSelf( std::ostream & os, Indent indent ) const override;

private:

  typedef typename InitialRegistrationMethodType::LandmarkPointType
  LandmarkPointType;
  typedef typename InitialRegistrationMethodType::LandmarkPointContainer
  LandmarkPointContainer;

  ImageToImageRegistrationHelper( const Self & );   // Purposely not implemented
  void operator =( const Self & );                  // Purposely not implemented

  //  Data
  typename TImage::ConstPointer         m_FixedImage;
  typename TImage::ConstPointer         m_MovingImage;

  bool   m_SampleFromOverlap;
  double m_SampleIntensityPortion;

  bool                                  m_UseFixedImageMaskObject;
  typename MaskObjectType::ConstPointer m_FixedImageMaskObject;
  bool                                  m_UseMovingImageMaskObject;
  typename MaskObjectType::ConstPointer m_MovingImageMaskObject;

  bool      m_UseRegionOfInterest;
  PointType m_RegionOfInterestPoint1;
  PointType m_RegionOfInterestPoint2;

  unsigned int m_RandomNumberSeed;

  //  Process
  bool m_EnableLoadedRegistration;
  bool m_EnableInitialRegistration;
  bool m_EnableRigidRegistration;
  bool m_EnableAffineRegistration;
  bool m_EnableBSplineRegistration;

  double m_ExpectedOffsetPixelMagnitude;
  double m_ExpectedRotationMagnitude;
  double m_ExpectedScaleMagnitude;
  double m_ExpectedSkewMagnitude;

  bool                      m_CompletedInitialization;
  RegistrationStageEnumType m_CompletedStage;
  bool                      m_CompletedResampling;

  typename TImage::ConstPointer           m_CurrentMovingImage;
  typename MatrixTransformType::Pointer   m_CurrentMatrixTransform;
  typename BSplineTransformType::Pointer  m_CurrentBSplineTransform;

  typename TImage::ConstPointer         m_LoadedTransformResampledImage;
  typename TImage::ConstPointer         m_MatrixTransformResampledImage;
  typename TImage::ConstPointer         m_BSplineTransformResampledImage;

  double m_FinalMetricValue;

  typename TImage::ConstPointer  m_BaselineImage;
  unsigned int                   m_BaselineNumberOfFailedPixelsTolerance;
  PixelType                      m_BaselineIntensityTolerance;
  unsigned int                   m_BaselineRadiusTolerance;
  typename TImage::ConstPointer  m_BaselineResampledMovingImage;
  typename TImage::ConstPointer  m_BaselineDifferenceImage;
  unsigned int                   m_BaselineNumberOfFailedPixels;
  bool                           m_BaselineTestPassed;

  bool m_ReportProgress;
  bool m_MinimizeMemory;

  //  Loaded Tansform
  typename MatrixTransformType::Pointer   m_LoadedMatrixTransform;
  typename BSplineTransformType::Pointer  m_LoadedBSplineTransform;

  //  Initial Parameters
  InitialMethodEnumType                   m_InitialMethodEnum;
  typename InitialTransformType::Pointer  m_InitialTransform;
  LandmarkPointContainer                  m_FixedLandmarks;
  LandmarkPointContainer                  m_MovingLandmarks;

  //  Rigid Parameters
  double       m_RigidSamplingRatio;
  double       m_RigidTargetError;
  unsigned int m_RigidMaxIterations;

  typename RigidTransformType::Pointer m_RigidTransform;
  MetricMethodEnumType                 m_RigidMetricMethodEnum;
  InterpolationMethodEnumType          m_RigidInterpolationMethodEnum;

  double m_RigidMetricValue;

  //  Affine Parameters
  double       m_AffineSamplingRatio;
  double       m_AffineTargetError;
  unsigned int m_AffineMaxIterations;

  typename AffineTransformType::Pointer m_AffineTransform;
  MetricMethodEnumType                  m_AffineMetricMethodEnum;
  InterpolationMethodEnumType           m_AffineInterpolationMethodEnum;

  double m_AffineMetricValue;

  //  BSpline Parameters
  double       m_BSplineSamplingRatio;
  double       m_BSplineTargetError;
  unsigned int m_BSplineMaxIterations;
  double       m_BSplineControlPointPixelSpacing;

  typename BSplineTransformType::Pointer m_BSplineTransform;
  MetricMethodEnumType                   m_BSplineMetricMethodEnum;
  InterpolationMethodEnumType            m_BSplineInterpolationMethodEnum;

  double m_BSplineMetricValue;

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageToImageRegistrationHelper.txx"
#endif

#endif
