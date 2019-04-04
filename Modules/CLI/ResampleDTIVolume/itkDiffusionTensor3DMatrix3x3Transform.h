/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DMatrix3x3Transform_h
#define itkDiffusionTensor3DMatrix3x3Transform_h

#include "itkDiffusionTensor3DTransform.h"
#include <itkVector.h>
#include <itkMatrixOffsetTransformBase.h>
#include <mutex>

namespace itk
{

/**
 * \class DiffusionTensor3DMatrix3x3Transform
 *
 * Virtual class to implement diffusion images transformation based on a 3x3 matrix
 */

template <class TData>
class DiffusionTensor3DMatrix3x3Transform :
  public DiffusionTensor3DTransform<TData>
{
public:
  typedef TData                                            DataType;
  typedef DiffusionTensor3DMatrix3x3Transform              Self;
  typedef DiffusionTensor3DTransform<DataType>             Superclass;
  typedef typename Superclass::TransformType               TransformType;
  typedef typename Superclass::TensorDataType              TensorDataType;
  typedef typename Superclass::InternalTensorDataType      InternalTensorDataType;
  typedef typename Superclass::PointType                   PointType;
  typedef typename Superclass::MatrixTransformType         MatrixTransformType;
  typedef typename Superclass::MatrixDataType              MatrixDataType;
  typedef typename Superclass::InternalMatrixTransformType InternalMatrixTransformType;
  typedef typename Superclass::InternalMatrixDataType      InternalMatrixDataType;
  typedef Vector<TransformType, 3>                         VectorType;
  typedef SmartPointer<Self>                               Pointer;
  typedef SmartPointer<const Self>                         ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DMatrix3x3Transform, DiffusionTensor3DTransform);

  // /Set the translation vector
  void SetTranslation( VectorType translation );

  // /Get the translation vector
  VectorType GetTranslation();

  // /Set the center of the transformation
  void SetCenter( PointType center );

  // /Evaluate the position of the transformed tensor in the output image
  PointType EvaluateTensorPosition( const PointType & point ) override;

  // /Set the 3x3 transform matrix
  virtual void SetMatrix3x3( MatrixTransformType & matrix );

  // /Get the 3x3 transform matrix
  virtual InternalMatrixTransformType GetMatrix3x3();

  // /Evaluate the transformed tensor
  virtual TensorDataType EvaluateTransformedTensor( TensorDataType & tensor );

  TensorDataType EvaluateTransformedTensor( TensorDataType & tensor, PointType & outputPosition ) override; // dummy
                                                                                                           // output
                                                                                                           // position;
                                                                                                           // to be
                                                                                                           // compatible
                                                                                                           // with
                                                                                                           // non-rigid
                                                                                                           // transforms

  typename Transform<double, 3, 3>::Pointer GetTransform() override;

protected:
  void ComputeOffset();

  virtual void PreCompute() = 0;

  DiffusionTensor3DMatrix3x3Transform();
  InternalMatrixTransformType m_TransformMatrix;
  InternalMatrixTransformType m_Transform;
  InternalMatrixTransformType m_TransformT;
  unsigned long               m_LatestTime;
  VectorType                  m_Translation;
  VectorType                  m_Offset;
  PointType                   m_Center;
  std::mutex                  m_Lock;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DMatrix3x3Transform.txx"
#endif

#endif
