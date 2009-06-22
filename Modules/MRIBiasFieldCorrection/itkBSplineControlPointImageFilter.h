/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkBSplineControlPointImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2008/10/18 00:16:51 $
  Version:   $Revision: 1.1.1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkBSplineControlPointImageFilter_h
#define __itkBSplineControlPointImageFilter_h

#include "itkImageToImageFilter.h"

#include "itkBSplineKernelFunction.h"
#include "itkCoxDeBoorBSplineKernelFunction.h"
#include "itkFixedArray.h"
#include "itkPointSet.h"
#include "itkVariableSizeMatrix.h"
#include "itkVector.h"
#include "itkVectorContainer.h"

#include "vnl/vnl_matrix.h"

namespace itk
{
/**
 * \class BSplineControlPointImageFilter.h
 * \brief Auxilary class for the output of the class
 * itkBSplineScatteredDataPointSetToImageFilter.
 * \par  The output of the class itkBSplineScatteredDataPointSetToImageFilter
 * is a control point grid.  This class is used to hold various routines meant
 * to operate on that control point grid.
 */

template <class TInputImage, class TOutputImage>
class BSplineControlPointImageFilter
: public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  typedef BSplineControlPointImageFilter                      Self;
  typedef ImageToImageFilter<TInputImage, TOutputImage>       Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Extract dimension from input image. */
  itkStaticConstMacro( ImageDimension, unsigned int,
                       TInputImage::ImageDimension );

  typedef TInputImage                                ControlPointLatticeType;
  typedef TOutputImage                               ImageType;

  /** Image typedef support. */
  typedef typename ImageType::PixelType              PixelType;
  typedef typename ImageType::RegionType             RegionType;
  typedef typename ImageType::IndexType              IndexType;
  typedef typename ImageType::PointType              PointType;
  typedef typename ImageType::PointType              ContinuousIndexType;

  typedef typename TOutputImage::SpacingType         SpacingType;
  typedef typename TOutputImage::PointType           OriginType;
  typedef typename TOutputImage::SizeType            SizeType;
  typedef typename TOutputImage::DirectionType       DirectionType;

  /** Other typedef */
  typedef float                                      RealType;
  typedef Image<RealType,
    itkGetStaticConstMacro( ImageDimension )>        RealImageType;
  typedef FixedArray<unsigned,
    itkGetStaticConstMacro( ImageDimension )>        ArrayType;
  typedef VariableSizeMatrix<RealType>               GradientType;
  typedef RealImageType                              HessianType;

  /** PointSet typedef support. */
  typedef PointSet<PixelType,
    itkGetStaticConstMacro( ImageDimension )>        PointSetType;
  typedef typename PointSetType::PixelType           PointDataType;
  typedef typename PointSetType::PointDataContainer  PointDataContainerType;

  /** Interpolation kernel type (default spline order = 3) */
  typedef CoxDeBoorBSplineKernelFunction<3>          KernelType;
  typedef BSplineKernelFunction<0>                   KernelOrder0Type;
  typedef BSplineKernelFunction<1>                   KernelOrder1Type;
  typedef BSplineKernelFunction<2>                   KernelOrder2Type;
  typedef BSplineKernelFunction<3>                   KernelOrder3Type;

  /** Helper functions */
  void SetSplineOrder( unsigned int );
  void SetSplineOrder( ArrayType );
  itkGetConstReferenceMacro( SplineOrder, ArrayType );

  itkSetMacro( CloseDimension, ArrayType );
  itkGetConstReferenceMacro( CloseDimension, ArrayType );

  /** Gets and sets for the output image. */
  itkSetMacro( Spacing, SpacingType );
  itkGetConstMacro( Spacing, SpacingType );
  itkSetMacro( Origin, OriginType );
  itkGetConstMacro( Origin, OriginType );
  itkSetMacro( Size, SizeType );
  itkGetConstMacro( Size, SizeType );
  itkSetMacro( Direction, DirectionType );
  itkGetConstMacro( Direction, DirectionType );

  /**
   * Evaluate the resulting B-spline object at a specified
   * point or index within the image domain.
   */
  void EvaluateAtPoint( PointType, PixelType & );
  void EvaluateAtIndex( IndexType, PixelType & );
  void EvaluateAtContinuousIndex( ContinuousIndexType, PixelType & );

  /**
   * Evaluate the resulting B-spline object at a specified
   * parameteric point.  Note that the parameterization over
   * each dimension of the B-spline object is [0, 1).
   */
  void Evaluate( PointType, PixelType & );

  /**
   * Evaluate the gradient of the resulting B-spline object at a specified
   * point or index within the image domain.
   */
  void EvaluateGradientAtPoint( PointType, GradientType & );
  void EvaluateGradientAtIndex( IndexType, GradientType & );
  void EvaluateGradientAtContinuousIndex( ContinuousIndexType, GradientType & );

  /**
   * Evaluate the gradient of the resulting B-spline object
   * at a specified parameteric point.  Note that the
   * parameterization over each dimension of the B-spline
   * object is [0, 1).
   */
  void EvaluateGradient( PointType, GradientType & );

  /**
   * Evaluate the Jacobian of the resulting B-spline object at a specified
   * point or index within the image domain.
   */
  void EvaluateJacobianAtPoint( PointType pt, GradientType &jac )
    {
    this->EvaluateGradientAtPoint( pt, jac );
    GradientType I( jac.Rows(), jac.Cols() );
    I.SetIdentity();
    jac += I;
    }
  void EvaluateJacobianAtIndex( IndexType idx, GradientType &jac )
    {
    this->EvaluateGradientAtIndex( idx, jac );
    GradientType I( jac.Rows(), jac.Cols() );
    I.SetIdentity();
    jac += I;
    }
  void EvaluateJacobianAtContinuousIndex( ContinuousIndexType cidx,
    GradientType &jac )
    {
    this->EvaluateGradientAtContinuousIndex( cidx, jac );
    GradientType I( jac.Rows(), jac.Cols() );
    I.SetIdentity();
    jac += I;
    }

  /**
   * Evaluate the Jacobian with respect to the image of the resulting B-spline
   * object at a specified point or index within the image domain.
   */
  void EvaluateSpatialJacobianAtPoint( PointType pt, GradientType &jac )
    {
    this->EvaluateGradientAtPoint( pt, jac );
    for ( unsigned int i = 0; i < jac.Cols(); i++ )
      {
      RealType factor = static_cast<RealType>(
        this->GetOutput()->GetLargestPossibleRegion().GetSize()[i] )
        * this->GetOutput()->GetSpacing()[i];
      for ( unsigned int j = 0; j < jac.Rows(); j++ )
        {
        jac(i, j) *= factor;
        if ( i == j )
          {
          jac(i, j) += 1.0;
          }
        }
      }
    }
  void EvaluateSpatialJacobianAtIndex( IndexType idx, GradientType &jac )
    {
    PointType pt;
    this->GetOutput()->TransformIndexToPhysicalPoint( idx, pt );
    this->EvaluateSpatialJacobianAtPoint( pt, jac );
    }
  void EvaluateSpatialJacobianAtContinuousIndex(
    ContinuousIndexType cidx, GradientType &jac )
    {
    PointType pt;
    this->GetOutput()->TransformContinuousIndexToPhysicalPoint( cidx, pt );
    this->EvaluateSpatialJacobianAtPoint( pt, jac );
    }

  /**
   * Evaluate the Jacobian determinant image with respect to the image of
   * the resulting B-spline object at a specified point or index within
   * the image domain.
   */
  typename ImageType::Pointer EvaluateSpatialJacobianDeterminantImage();

  /**
   * Evaluate the Hessian of the resulting B-spline object at a specified
   * point or index within the image domain.  Since the Hessian for a vector
   * function is a 3-tensor, one must specify the component.
   */
  void EvaluateHessianAtPoint( PointType, HessianType &, unsigned int );
  void EvaluateHessianAtIndex( IndexType, HessianType &, unsigned int );
  void EvaluateHessianAtContinuousIndex(
    ContinuousIndexType, GradientType &, unsigned int );

  void EvaluateHessian( PointType, GradientType &, unsigned int );

  /**
   * Given a B-spline object value and an initial parametric guess, use
   * conjugate gradient descent to find the parameters corresponding to the
   * B-spline object value.
   */
  void EvaluateParametersAtPoint( PointDataType, PointType & );

  /**
   * Generate a refined control point lattice from the input control point
   * lattice such that the resolution is doubled for each level.
   */
  typename ControlPointLatticeType::Pointer RefineControlLattice( ArrayType );


protected:
  BSplineControlPointImageFilter();
  virtual ~BSplineControlPointImageFilter();
  void PrintSelf( std::ostream& os, Indent indent ) const;

  void GenerateData();

private:
  BSplineControlPointImageFilter( const Self& ); //purposely not implemented
  void operator=( const Self& ); //purposely not implemented

  void GenerateOutputImageFast();
  void CollapsePhiLattice( ControlPointLatticeType *,
    ControlPointLatticeType *, RealType, unsigned int );
  void SetNumberOfLevels( ArrayType );

  /** Parameters for the output image. */

  SizeType                                       m_Size;
  SpacingType                                    m_Spacing;
  OriginType                                     m_Origin;
  DirectionType                                  m_Direction;

  ArrayType                                      m_NumberOfLevels;
  bool                                           m_DoMultilevel;
  unsigned int                                   m_MaximumNumberOfLevels;
  vnl_matrix<RealType>                           m_RefinedLatticeCoefficients[ImageDimension];
  ArrayType                                      m_CloseDimension;
  ArrayType                                      m_SplineOrder;
  ArrayType                                      m_NumberOfControlPoints;

  typename KernelType::Pointer                   m_Kernel[ImageDimension];
  typename KernelOrder0Type::Pointer             m_KernelOrder0;
  typename KernelOrder1Type::Pointer             m_KernelOrder1;
  typename KernelOrder2Type::Pointer             m_KernelOrder2;
  typename KernelOrder3Type::Pointer             m_KernelOrder3;

  RealType                                       m_BSplineEpsilon;

  inline typename RealImageType::IndexType
  NumberToIndex( unsigned int number, typename RealImageType::SizeType size )
    {
    typename RealImageType::IndexType k;
    k[0] = 1;

    for ( unsigned int i = 1; i < ImageDimension; i++ )
      {
      k[i] = size[ImageDimension-i-1]*k[i-1];
      }
    typename RealImageType::IndexType index;
    for ( unsigned int i = 0; i < ImageDimension; i++ )
      {
      index[ImageDimension-i-1]
        = static_cast<unsigned int>( number/k[ImageDimension-i-1] );
      number %= k[ImageDimension-i-1];
      }
    return index;
    }

  /**
   * Functionality used by EvaluateParametersAtPoint() meant
   */

  RealType EvaluateMetricForCGD( PointType, PointDataType );
  PointDataType EvaluateGradientForCGD( PointType, PointDataType );
  RealType EvaluateEnergyForLineSearch( RealType, PointType,
    PointDataType, PointDataType );
  void LineMinimizationForCGD( RealType *, RealType *, PointType,
    PointDataType, PointDataType );
  void FindBracketingTriplet( RealType *, RealType *, RealType *,
    PointType, PointDataType, PointDataType );
  void BrentSearch( RealType, RealType, RealType, RealType *, RealType *,
    PointType, PointDataType, PointDataType );

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBSplineControlPointImageFilter.txx"
#endif

#endif
