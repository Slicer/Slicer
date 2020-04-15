/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DInterpolateImageFunction_h
#define itkDiffusionTensor3DInterpolateImageFunction_h

#include <itkObject.h>
#include "itkDiffusionTensor3D.h"
#include <itkImage.h>
#include <itkPoint.h>
#include <itkImageFunction.h>

namespace itk
{
/**
 * \class DiffusionTensor3DInterpolateImageFunction
 *
 * Virtual class to implement diffusion tensor interpolation classes
 *
 */
template <class TData, class TCoordRep = double>
class DiffusionTensor3DInterpolateImageFunction :
  public ImageFunction<Image<DiffusionTensor3D<TData>, 3>,
                       DiffusionTensor3D<TData>,
                       TCoordRep
                       >
{
public:
  using TensorType = TData;
  using Self = DiffusionTensor3DInterpolateImageFunction<TData, TCoordRep>;
  using TensorDataType = DiffusionTensor3D<TensorType>;
  using DiffusionImageType = Image<TensorDataType, 3>;
  using DiffusionImageTypePointer = typename DiffusionImageType::Pointer;
  using PointType = Point<double, 3>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using TensorRealType = typename TensorDataType::RealValueType;

  using Superclass = ImageFunction<Image<DiffusionTensor3D<TData>, 3>, DiffusionTensor3D<TData>, TCoordRep>;

  using ContinuousIndexType = typename Superclass::ContinuousIndexType;
  using IndexType = typename Superclass::IndexType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DInterpolateImageFunction, ImageFunction);

// ///Copied from itkInterpolateImageFunction.h

  /** Interpolate the image at a point position
   *
   * Returns the interpolated image intensity at a
   * specified point position. No bounds checking is done.
   * The point is assume to lie within the image buffer.
   *
   * ImageFunction::IsInsideBuffer() can be used to check bounds before
   * calling the method. */
  TensorDataType Evaluate( const PointType& point ) const override
  {
    ContinuousIndexType index;

    this->GetInputImage()->TransformPhysicalPointToContinuousIndex( point, index );
    return this->EvaluateAtContinuousIndex( index );
  }

  /** Interpolate the image at a continuous index position
   *
   * Returns the interpolated image intensity at a
   * specified index position. No bounds checking is done.
   * The point is assume to lie within the image buffer.
   *
   * Subclasses must override this method.
   *
   * ImageFunction::IsInsideBuffer() can be used to check bounds before
   * calling the method. */
  TensorDataType EvaluateAtContinuousIndex( const ContinuousIndexType & index ) const override = 0;

  /** Interpolate the image at an index position.
   *
   * Simply returns the image value at the
   * specified index position. No bounds checking is done.
   * The point is assume to lie within the image buffer.
   *
   * ImageFunction::IsInsideBuffer() can be used to check bounds before
   * calling the method. */

  TensorDataType EvaluateAtIndex( const IndexType & index ) const override
  {
    return this->GetInputImage()->GetPixel( index );
  }

//  void SetDefaultPixelValue( TensorRealType defaultPixelValue ) ;
//  itkGetMacro( DefaultPixelValue , TensorRealType ) ;
protected:
  DiffusionTensor3DInterpolateImageFunction();
  unsigned long m_LatestTime;
//  TensorRealType m_DefaultPixelValue ;
//  TensorDataType m_DefaultPixel ;
};

} // end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DInterpolateImageFunction.txx"
#endif

#endif
