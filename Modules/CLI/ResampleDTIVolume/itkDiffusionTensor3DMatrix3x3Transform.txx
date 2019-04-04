/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DMatrix3x3Transform_txx
#define itkDiffusionTensor3DMatrix3x3Transform_txx

#include "itkDiffusionTensor3DMatrix3x3Transform.h"

namespace itk
{

template <class TData>
DiffusionTensor3DMatrix3x3Transform<TData>
::DiffusionTensor3DMatrix3x3Transform()
{
  m_TransformMatrix.SetIdentity();
  m_Transform.SetIdentity();
  m_TransformT.SetIdentity();
  m_LatestTime = 0;
  m_Translation.Fill( NumericTraits<DataType>::ZeroValue() );
  m_Offset.Fill( NumericTraits<DataType>::ZeroValue() );
  m_Center.Fill( NumericTraits<DataType>::ZeroValue() );
}

template <class TData>
void
DiffusionTensor3DMatrix3x3Transform<TData>
::SetTranslation( VectorType translation )
{
  m_Translation = translation;
  this->Modified();
}

template <class TData>
void
DiffusionTensor3DMatrix3x3Transform<TData>
::SetCenter( PointType center )
{
  m_Center = center;
  this->Modified();
}

template <class TData>
void
DiffusionTensor3DMatrix3x3Transform<TData>
::ComputeOffset()
{
  for( int i = 0; i < 3; i++ )
    {
    m_Offset[i] = m_Translation[i] + m_Center[i];
    for( int j = 0; j < 3; j++ )
      {
      m_Offset[i] -= m_TransformMatrix[i][j] * m_Center[j];
      }
    }
}

template <class TData>
void
DiffusionTensor3DMatrix3x3Transform<TData>
::SetMatrix3x3( MatrixTransformType & matrix )
{
  m_TransformMatrix = matrix;
  this->Modified();
}

template <class TData>
typename DiffusionTensor3DMatrix3x3Transform<TData>::PointType
DiffusionTensor3DMatrix3x3Transform<TData>
::EvaluateTensorPosition( const PointType & point )
{
  if( m_LatestTime < Object::GetMTime() )
    {
    m_Lock.lock();
    if( m_LatestTime < Object::GetMTime() )
      {
      PreCompute();
      }
    m_Lock.unlock();
    }
  return m_TransformMatrix * point + m_Offset;
}

template <class TData>
typename DiffusionTensor3DMatrix3x3Transform<TData>::TensorDataType
DiffusionTensor3DMatrix3x3Transform<TData>
::EvaluateTransformedTensor( TensorDataType & tensor, PointType & itkNotUsed(outputPosition) )
{
  return EvaluateTransformedTensor( tensor );
}

template <class TData>
typename DiffusionTensor3DMatrix3x3Transform<TData>::TensorDataType
DiffusionTensor3DMatrix3x3Transform<TData>
::EvaluateTransformedTensor( TensorDataType & tensor )
{
  InternalTensorDataType internalTensor = tensor;

  if( m_LatestTime < Object::GetMTime() )
    {
    m_Lock.lock();
    if( m_LatestTime < Object::GetMTime() )
      {
      PreCompute();
      }
    m_Lock.unlock();
    }
  InternalMatrixDataType      tensorMatrix = internalTensor.GetTensor2Matrix();
  InternalMatrixTransformType mat = this->m_Transform
    * ( InternalMatrixTransformType ) tensorMatrix
    * this->m_TransformT;
  tensorMatrix = ( InternalMatrixDataType ) mat;
  internalTensor.SetTensorFromMatrix( static_cast<MatrixDataType>( tensorMatrix ) );
  return static_cast<TensorDataType>( internalTensor );
}

template <class TData>
typename DiffusionTensor3DMatrix3x3Transform<TData>::
InternalMatrixTransformType
DiffusionTensor3DMatrix3x3Transform<TData>::GetMatrix3x3()
{
  if( m_LatestTime < Object::GetMTime() )
    {
    m_Lock.lock();
    if( m_LatestTime < Object::GetMTime() )
      {
      PreCompute();
      }
    m_Lock.unlock();
    }
  return m_TransformMatrix;
}

template <class TData>
typename DiffusionTensor3DMatrix3x3Transform<TData>::
VectorType
DiffusionTensor3DMatrix3x3Transform<TData>::GetTranslation()
{
  if( m_LatestTime < Object::GetMTime() )
    {
    m_Lock.lock();
    if( m_LatestTime < Object::GetMTime() )
      {
      PreCompute();
      }
    m_Lock.unlock();
    }
  return m_Translation;
}

template <class TData>
typename Transform<double, 3, 3>::Pointer
DiffusionTensor3DMatrix3x3Transform<TData>
::GetTransform()
{
  typename itk::MatrixOffsetTransformBase<TransformType, 3, 3>::Pointer matrixTransform;
  matrixTransform = itk::MatrixOffsetTransformBase<TransformType, 3, 3>::New();
  matrixTransform->SetMatrix( m_TransformMatrix );
  matrixTransform->SetTranslation( m_Translation );
  typename itk::Transform<TransformType, 3, 3>::Pointer transform;
  transform = matrixTransform;
  return transform;
}

} // end namespace itk

#endif
