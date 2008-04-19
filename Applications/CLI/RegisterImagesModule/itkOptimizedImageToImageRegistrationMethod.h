/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: RigidRegistrator.h,v $
  Language:  C++
  Date:      $Date: 2006/11/06 14:39:34 $
  Version:   $Revision: 1.15 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __OptimizedImageToImageRegistrationMethod_h
#define __OptimizedImageToImageRegistrationMethod_h

#include "itkImage.h"

#include "itkImageToImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
class OptimizedImageToImageRegistrationMethod 
: public ImageToImageRegistrationMethod< TImage >
  {

  public:

    typedef OptimizedImageToImageRegistrationMethod   Self;
    typedef ImageToImageRegistrationMethod< TImage >  Superclass;
    typedef SmartPointer< Self >                      Pointer;
    typedef SmartPointer< const Self >                ConstPointer;

    itkTypeMacro( OptimizedImageToImageRegistrationMethod,
                  ImageToImageRegistrationMethod );

    itkNewMacro( Self );

    //
    // Typedefs from Superclass
    //
    typedef TImage                                     ImageType;

    typedef typename ImageType::PixelType              PixelType;

    typedef typename Superclass::TransformType         TransformType;

    typedef typename TransformType::ParametersType     TransformParametersType;

    typedef typename TransformType::ParametersType     TransformParametersScalesType;

    itkStaticConstMacro( ImageDimension, unsigned int,
                         TImage::ImageDimension );

    //
    // Custom Typedefs
    //
    enum TransformMethodEnumType { RIGID_TRANSFORM,
                                   AFFINE_TRANSFORM,
                                   BSPLINE_TRANSFORM };

    enum MetricMethodEnumType { MATTES_MI_METRIC,
                                NORMALIZED_CORRELATION_METRIC,
                                MEAN_SQUARED_ERROR_METRIC };

    enum InterpolationMethodEnumType { LINEAR_INTERPOLATION,
                                       BSPLINE_INTERPOLATION,
                                       SINC_INTERPOLATION };

    enum OptimizationMethodEnumType { MULTIRESOLUTION_OPTIMIZATION,
                                      EVOLUTIONARY_OPTIMIZATION,
                                      GRADIENT_OPTIMIZATION };

    // 
    // Methods from Superclass
    //
    void Update( void );

    //
    // Custom Methods
    //
    itkSetMacro( InitialTransformParameters, TransformParametersType );
    itkGetConstMacro( InitialTransformParameters, TransformParametersType );

    itkSetMacro( InitialTransformFixedParameters, TransformParametersType );
    itkGetConstMacro( InitialTransformFixedParameters, TransformParametersType );

    itkSetMacro( LastTransformParameters, TransformParametersType );
    itkGetConstMacro( LastTransformParameters, TransformParametersType );

    itkSetMacro( TransformParametersScales, TransformParametersScalesType );
    itkGetConstMacro( TransformParametersScales, TransformParametersScalesType );

    itkSetMacro( UseOverlapAsROI, bool );
    itkGetConstMacro( UseOverlapAsROI, bool );

    itkSetMacro( MinimizeMemory, bool );
    itkGetConstMacro( MinimizeMemory, bool );

    itkSetMacro( MaxIterations, unsigned int );
    itkGetConstMacro( MaxIterations, unsigned int );

    itkSetMacro( NumberOfSamples, unsigned int );
    itkGetConstMacro( NumberOfSamples, unsigned int );

    itkSetMacro( FixedImageSamplesIntensityThreshold, PixelType );
    itkGetConstMacro( FixedImageSamplesIntensityThreshold, PixelType );

    itkSetMacro( TargetError, double );
    itkGetConstMacro( TargetError, double );

    itkGetConstMacro( TransformMethodEnum, TransformMethodEnumType );

    itkSetMacro( MetricMethodEnum, MetricMethodEnumType );
    itkGetConstMacro( MetricMethodEnum, MetricMethodEnumType );

    itkSetMacro( InterpolationMethodEnum, InterpolationMethodEnumType );
    itkGetConstMacro( InterpolationMethodEnum, InterpolationMethodEnumType );

    itkSetMacro( OptimizationMethodEnum, OptimizationMethodEnumType );
    itkGetConstMacro( OptimizationMethodEnum, OptimizationMethodEnumType );

    itkGetMacro( FinalMetricValue, double );

  protected:

    OptimizedImageToImageRegistrationMethod( void );
    virtual ~OptimizedImageToImageRegistrationMethod( void );

    itkSetMacro( TransformMethodEnum, TransformMethodEnumType );

    void PrintSelf( std::ostream & os, Indent indent ) const;

  private:

    OptimizedImageToImageRegistrationMethod( const Self & );  // Purposely not implemented
    void operator = ( const Self & );                         // Purposely not implemented


    TransformParametersType             m_InitialTransformParameters;
    TransformParametersType             m_InitialTransformFixedParameters;

    TransformParametersType             m_LastTransformParameters;

    TransformParametersScalesType       m_TransformParametersScales;

    bool                                m_UseOverlapAsROI;

    bool                                m_MinimizeMemory;

    unsigned int                        m_MaxIterations;

    unsigned int                        m_NumberOfSamples;

    PixelType                           m_FixedImageSamplesIntensityThreshold;

    double                              m_TargetError;

    TransformMethodEnumType             m_TransformMethodEnum;

    MetricMethodEnumType                m_MetricMethodEnum;

    InterpolationMethodEnumType         m_InterpolationMethodEnum;

    OptimizationMethodEnumType          m_OptimizationMethodEnum;

    double                              m_FinalMetricValue;
  };

} 

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOptimizedImageToImageRegistrationMethod.txx"
#endif


#endif 

