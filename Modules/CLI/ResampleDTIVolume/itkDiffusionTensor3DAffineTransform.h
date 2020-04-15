/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DAffineTransform_h
#define itkDiffusionTensor3DAffineTransform_h

#include "itkDiffusionTensor3DMatrix3x3Transform.h"
#include <itkAffineTransform.h>

namespace itk
{
/**
 * \class DiffusionTensor3DAffineTransform
 *
 *
 * This is an abstract class to implement affine transformation for diffusion tensors.
 */
template <class TData>
class DiffusionTensor3DAffineTransform :
  public DiffusionTensor3DMatrix3x3Transform<TData>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DAffineTransform<TData>;
  using Superclass = DiffusionTensor3DMatrix3x3Transform<DataType>;
  using PointType = typename Superclass::PointType;
  using TensorDataType = typename Superclass::TensorDataType;
  using MatrixDataType = typename Superclass::MatrixDataType;
  using MatrixTransformType = typename Superclass::MatrixTransformType;
  using InternalTensorDataType = typename Superclass::InternalTensorDataType;
  using InternalMatrixDataType = typename Superclass::InternalMatrixDataType;
  using InternalMatrixTransformType = typename Superclass::InternalMatrixTransformType;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using MatrixTransform4x4Type = MatrixExtended<double, 4, 4>;
  using AffineTransformType = AffineTransform<double, 3>;
  using VectorType = typename Superclass::VectorType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DAffineTransform, DiffusionTensor3DMatrix3x3Transform);

  /** Set the transformation matrix from an itk::AffineTransform< double , 3 > object
  */
  void SetTransform( typename AffineTransformType::Pointer transform );
  typename AffineTransformType::Pointer GetAffineTransform();

  /** Set the transformation matrix directly from a 4x4 matrix
  * The 4th row is ignored and considered to be 0,0,0,1
  */
  void SetMatrix4x4( MatrixTransform4x4Type matrix );

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DAffineTransform.txx"
#endif

#endif
