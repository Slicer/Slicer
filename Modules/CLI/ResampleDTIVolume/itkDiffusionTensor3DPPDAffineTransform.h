/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DPPDAffineTransform_h
#define itkDiffusionTensor3DPPDAffineTransform_h

#include "itkDiffusionTensor3DAffineTransform.h"
#include <vnl/vnl_vector.h>
#include <vnl/vnl_cross.h>

namespace itk
{
/**
 * \class DiffusionTensor3DFSAffineTransform
 *
 *
 * This class implements an affine transformation for diffusion tensors. It implements the
 * Preservation of Principal Direction method presented in the following paper:
 * D.C. Alexander, Member IEEE, C. Pierpaoli, P.J. Basser and J.C Gee:
 * Spatial Transformations of Diffusion Tensor Magnetic Resonance Images,
 * IEEE Transactions on Medical Imaging, Vol 20, No. 11, November 2001
 *
 */
template <class TData>
class DiffusionTensor3DPPDAffineTransform :
  public DiffusionTensor3DAffineTransform<TData>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DPPDAffineTransform<TData>;
  using Superclass = DiffusionTensor3DAffineTransform<DataType>;
  using TensorDataType = typename Superclass::TensorDataType;
  using MatrixDataType = typename Superclass::MatrixDataType;
  using MatrixTransformType = typename Superclass::MatrixTransformType;
  using InternalTensorDataType = typename Superclass::InternalTensorDataType;
  using InternalMatrixDataType = typename Superclass::InternalMatrixDataType;
  using InternalMatrixTransformType = typename Superclass::InternalMatrixTransformType;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using VectorType = typename Superclass::VectorType;
  using EValuesType = DiffusionTensor3DExtended<double>::EigenValuesArrayType;
  using EVectorsType = DiffusionTensor3DExtended<double>::EigenVectorsMatrixType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DPPDAffineTransform, DiffusionTensor3DAffineTransform);

  itkNewMacro( Self );
  using Superclass::EvaluateTransformedTensor;
  TensorDataType EvaluateTransformedTensor( TensorDataType & tensor ) override;

  void SetMatrix( MatrixTransformType & matrix );

protected:
  void PreCompute() override;

  InternalMatrixTransformType ComputeMatrixFromAxisAndAngle( VectorType axis, double cosangle );

  InternalMatrixTransformType m_TransformMatrixInverse;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DPPDAffineTransform.txx"
#endif

#endif
