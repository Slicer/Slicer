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
  typedef TData                                                     DataType;
  typedef DiffusionTensor3DPPDAffineTransform                       Self;
  typedef DiffusionTensor3DAffineTransform<DataType>                Superclass;
  typedef typename Superclass::TensorDataType                       TensorDataType;
  typedef typename Superclass::MatrixDataType                       MatrixDataType;
  typedef typename Superclass::MatrixTransformType                  MatrixTransformType;
  typedef typename Superclass::InternalTensorDataType               InternalTensorDataType;
  typedef typename Superclass::InternalMatrixDataType               InternalMatrixDataType;
  typedef typename Superclass::InternalMatrixTransformType          InternalMatrixTransformType;
  typedef SmartPointer<Self>                                        Pointer;
  typedef SmartPointer<const Self>                                  ConstPointer;
  typedef typename Superclass::VectorType                           VectorType;
  typedef DiffusionTensor3DExtended<double>::EigenValuesArrayType   EValuesType;
  typedef DiffusionTensor3DExtended<double>::EigenVectorsMatrixType EVectorsType;

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
