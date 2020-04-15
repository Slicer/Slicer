/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DNonRigidTransform_h
#define itkDiffusionTensor3DNonRigidTransform_h

#include "itkDiffusionTensor3DTransform.h"
#include "itkDiffusionTensor3DFSAffineTransform.h"
#include "itkDiffusionTensor3DPPDAffineTransform.h"
#include <itkTransform.h>

namespace itk
{

template <class TData>
class DiffusionTensor3DNonRigidTransform :
  public DiffusionTensor3DTransform<TData>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DNonRigidTransform<TData>;
  using Superclass = DiffusionTensor3DTransform<DataType>;
  using TensorDataType = typename Superclass::TensorDataType;
  using MatrixTransformType = typename Superclass::MatrixTransformType;
  using PointType = typename Superclass::PointType;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using TransformType = Transform<double, 3, 3>;
  using PPDAffineTransformType = itk::DiffusionTensor3DPPDAffineTransform<DataType>;
  using FSAffineTransformType = itk::DiffusionTensor3DFSAffineTransform<DataType>;
  using AffineTransform = itk::DiffusionTensor3DAffineTransform<DataType>;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DNonRigidTransform, DiffusionTensor3DTransform);

  // SmartPointer
  itkNewMacro( Self );
  // /Set the transform
  itkSetObjectMacro( Transform, TransformType );
  TransformType::Pointer GetTransform() override;

  // /Evaluate the position of the transformed tensor in the output image
  PointType EvaluateTensorPosition( const PointType & point ) override;

  // /Evaluate the transformed tensor
  TensorDataType EvaluateTransformedTensor( TensorDataType & tensor, PointType & outputPosition ) override;

  void SetAffineTransformType(typename AffineTransform::Pointer transform);
protected:
  DiffusionTensor3DNonRigidTransform();
  unsigned long                     m_LatestTime;
  typename TransformType::Pointer   m_Transform;
  typename AffineTransform::Pointer m_Affine;
};

} // end of itk namespace

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DNonRigidTransform.txx"
#endif

#endif
