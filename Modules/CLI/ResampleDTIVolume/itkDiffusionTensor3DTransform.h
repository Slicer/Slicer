/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DTransform_h
#define itkDiffusionTensor3DTransform_h

#include <itkObject.h>
#include "itkDiffusionTensor3DExtended.h"
#include <itkPoint.h>
#include "itkMatrixExtended.h"
#include <itkNumericTraits.h>
#include <itkTransform.h>

namespace itk
{
/** \class DiffusionTensor3DTransform
 *
 * This is an abstract class to represent the diffusion tensor transform
 */
template <class TData>
class DiffusionTensor3DTransform : public Object
{
public:
  typedef TData                               DataType;
  typedef double                              TransformType;
  typedef DiffusionTensor3DTransform          Self;
  typedef Point<TransformType, 3>             PointType;
  typedef DiffusionTensor3D<DataType>         TensorDataType;
  typedef DiffusionTensor3DExtended<DataType> InternalTensorDataType;
  typedef Matrix<TransformType, 3, 3>         MatrixTransformType;
  typedef Matrix<DataType, 3, 3>              MatrixDataType;
  typedef MatrixExtended<TransformType, 3, 3> InternalMatrixTransformType;
  typedef MatrixExtended<DataType, 3, 3>      InternalMatrixDataType;
  typedef SmartPointer<Self>                  Pointer;
  typedef SmartPointer<const Self>            ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DTransform, Object);

  // /Evaluate the position of the transformed tensor
  virtual PointType EvaluateTensorPosition( const PointType & point ) = 0;

  // /Evaluate the transformed tensor
  virtual TensorDataType EvaluateTransformedTensor(  TensorDataType & tensor, PointType & outputPosition) = 0;

  // /Set the measurement frame of the tensor
  itkSetMacro( MeasurementFrame, MatrixTransformType );
  virtual typename Transform<TransformType, 3, 3>::Pointer GetTransform() = 0;

protected:
  DiffusionTensor3DTransform();
  InternalMatrixTransformType m_MeasurementFrame;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DTransform.txx"
#endif

#endif
