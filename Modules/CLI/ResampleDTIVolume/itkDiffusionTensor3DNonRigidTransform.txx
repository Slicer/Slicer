/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DNonRigidTransform_txx
#define itkDiffusionTensor3DNonRigidTransform_txx

#include "itkDiffusionTensor3DNonRigidTransform.h"

namespace itk
{

template <class TData>
DiffusionTensor3DNonRigidTransform<TData>
::DiffusionTensor3DNonRigidTransform() = default;

template <class TData>
void
DiffusionTensor3DNonRigidTransform<TData>
::SetAffineTransformType(typename AffineTransform::Pointer transform)
{
  m_Affine = transform;
}

template <class TData>
typename Transform<double, 3, 3>::Pointer
DiffusionTensor3DNonRigidTransform<TData>
::GetTransform()
{
  return m_Transform;
}

template <class TData>
typename DiffusionTensor3DNonRigidTransform<TData>::PointType
DiffusionTensor3DNonRigidTransform<TData>
::EvaluateTensorPosition( const PointType & point )
{
  if( m_Transform.IsNotNull() )
    {
    return m_Transform->TransformPoint(point);
    }
  else
    {
    itkExceptionMacro( << "Transform not set" );
    }
}

template <class TData>
typename DiffusionTensor3DNonRigidTransform<TData>::TensorDataType
DiffusionTensor3DNonRigidTransform<TData>
::EvaluateTransformedTensor( TensorDataType & tensor, PointType & outputPosition )
{
  if( m_Transform.IsNotNull() && m_Affine.IsNotNull() )
    {
    MatrixTransformType matrix;
    matrix.SetIdentity();
    typename TransformType::JacobianType jacobian;
    m_Transform->ComputeJacobianWithRespectToParameters( outputPosition, jacobian );
    for( int i = 0; i < 3; i++ )
      {
      for( int j = 0; j < 3; j++ )
        {
        matrix[i][j] = jacobian[i][j] + matrix[i][j];
        }
      }
    // ITKv4 does not require locking, because ComputeJacobianWithRespectToParameters is thread-safe
    LightObject::Pointer newTransform = m_Affine->CreateAnother();
    typename AffineTransform::Pointer newAffine = dynamic_cast<AffineTransform *>( newTransform.GetPointer() );
    /*m_Affine->SetMatrix3x3( matrix ) ;
    return m_Affine->EvaluateTransformedTensor( tensor ) ;*/
    newAffine->SetMeasurementFrame( this->m_MeasurementFrame );
    newAffine->SetMatrix3x3( matrix );
    return newAffine->EvaluateTransformedTensor( tensor );
    }
  else
    {
    itkExceptionMacro( << "Transform or affine transform type not set" );
    }
}

} // end of itk namespace

#endif
