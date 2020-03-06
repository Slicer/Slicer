/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: BSplineRegistrator.h,v $
  Language:  C++
  Date:      $Date: 2006/11/06 14:39:34 $
  Version:   $Revision: 1.15 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef itkBSplineImageToImageRegistrationMethod_h
#define itkBSplineImageToImageRegistrationMethod_h

#include "itkImage.h"
#include "itkBSplineDeformableTransform.h"

#include "itkOptimizedImageToImageRegistrationMethod.h"

namespace itk
{

template <class TImage>
class BSplineImageToImageRegistrationMethod
  : public OptimizedImageToImageRegistrationMethod<TImage>
{

public:

  typedef BSplineImageToImageRegistrationMethod           Self;
  typedef OptimizedImageToImageRegistrationMethod<TImage> Superclass;
  typedef SmartPointer<Self>                              Pointer;
  typedef SmartPointer<const Self>                        ConstPointer;

  itkTypeMacro( BSplineImageToImageRegistrationMethod,
                OptimizedImageToImageRegistrationMethod );

  itkNewMacro( Self );

  //
  // Typedefs from Superclass
  //
  typedef TImage ImageType;
  itkStaticConstMacro( ImageDimension, unsigned int,
                       TImage::ImageDimension );

  // Overrides the superclass' TransformType typedef
  typedef BSplineDeformableTransform<double,
                                     itkGetStaticConstMacro( ImageDimension ),
                                     itkGetStaticConstMacro( ImageDimension )>
  BSplineTransformType;

  typedef typename BSplineTransformType::Pointer BSplineTransformPointer;

  typedef BSplineTransformType TransformType;

  typedef typename BSplineTransformType::ParametersType ParametersType;

  //
  // Methods from Superclass
  //

  void GenerateData() override;

  //
  // Custom Methods
  //

  /**
   * The function performs the casting.  This function should only appear
   *   once in the class hierarchy.  It is provided so that member
   *   functions that exist only in specific transforms (e.g., SetIdentity)
   *   can be called without the caller having to do the casting.
   **/
  virtual TransformType* GetTypedTransform();

  virtual const TransformType* GetTypedTransform() const;

  itkSetMacro( ExpectedDeformationMagnitude, double );
  itkGetConstMacro( ExpectedDeformationMagnitude, double );

  itkSetClampMacro( NumberOfControlPoints, unsigned int, 3, 2000 );
  itkGetConstMacro( NumberOfControlPoints, unsigned int );

  itkSetClampMacro( NumberOfLevels, unsigned int, 1, 5 );
  itkGetConstMacro( NumberOfLevels, unsigned int );

  BSplineTransformPointer GetBSplineTransform() const;

  void ComputeGridRegion( int numberOfControlPoints,
                          typename TransformType::RegionType::SizeType & regionSize,
                          typename TransformType::SpacingType & regionSpacing,
                          typename TransformType::OriginType & regionOrigin,
                          typename TransformType::DirectionType & regionDirection);

  void ResampleControlGrid( int newNumberOfControlPoints, ParametersType & newParameters );

  itkSetMacro( GradientOptimizeOnly, bool );
  itkGetMacro( GradientOptimizeOnly, bool );
protected:
  BSplineImageToImageRegistrationMethod();
  ~BSplineImageToImageRegistrationMethod() override;

  typedef InterpolateImageFunction<TImage, double> InterpolatorType;
  typedef ImageToImageMetric<TImage, TImage>       MetricType;

  void Optimize( MetricType * metric, InterpolatorType * interpolator ) override;

  virtual void GradientOptimize( MetricType * metric, InterpolatorType * interpolator );

  virtual void MultiResolutionOptimize( MetricType * metric, InterpolatorType * interpolator );

  void PrintSelf( std::ostream & os, Indent indent ) const override;

private:

  BSplineImageToImageRegistrationMethod( const Self & );  // Purposely not implemented
  void operator =( const Self & );                        // Purposely not implemented

  double m_ExpectedDeformationMagnitude;

  unsigned int m_NumberOfControlPoints;

  unsigned int m_NumberOfLevels;

  bool m_GradientOptimizeOnly;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBSplineImageToImageRegistrationMethod.txx"
#endif

#endif // __ImageToImageRegistrationMethod_h
