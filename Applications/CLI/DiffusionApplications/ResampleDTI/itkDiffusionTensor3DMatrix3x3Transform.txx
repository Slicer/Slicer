#ifndef __itkDiffusionTensor3DMatrix3x3Transform_txx
#define __itkDiffusionTensor3DMatrix3x3Transform_txx

#include "itkDiffusionTensor3DMatrix3x3Transform.h"

namespace itk
{


template< class TData >
DiffusionTensor3DMatrix3x3Transform< TData >
::DiffusionTensor3DMatrix3x3Transform()
{
  m_TransformMatrix.SetIdentity() ;
  m_Transform.SetIdentity() ;
  m_TransformT.SetIdentity() ;
  P = Semaphore::New() ;
  P->Initialize( 1 ) ;
  latestTime = 0 ;
  m_Translation.Fill( NumericTraits< DataType >::Zero ) ;
  m_Offset.Fill( NumericTraits< DataType >::Zero ) ;
  m_Center.Fill( NumericTraits< DataType >::Zero ) ;
}

template< class TData >
void
DiffusionTensor3DMatrix3x3Transform< TData >
::SetTranslation( VectorType translation )
{
  m_Translation = translation ;
}

template< class TData >
void
DiffusionTensor3DMatrix3x3Transform< TData >
::SetCenter( PointType center )
{
  m_Center = center ;
}

template< class TData >
void
DiffusionTensor3DMatrix3x3Transform< TData >
::ComputeOffset()
{
  for( int i = 0 ; i < 3 ; i++ )
    {
    m_Offset[ i ] = m_Translation[ i ] + m_Center[ i ] ;
    for( int j = 0 ; j < 3 ; j++ )
      {
      m_Offset[ i ] -= m_TransformMatrix[ i ][ j ] * m_Center[ j ] ;
      }
    }
}



template< class TData >
void
DiffusionTensor3DMatrix3x3Transform< TData >
::SetMatrix3x3( MatrixTransformType &matrix )
{
  m_TransformMatrix = matrix ;
}




template< class TData >
typename DiffusionTensor3DMatrix3x3Transform< TData >::PointType
DiffusionTensor3DMatrix3x3Transform< TData >
::EvaluateTensorPosition( const PointType &point )
{
  if( latestTime < Object::GetMTime() )
    {
    P->Down() ;
    if( latestTime < Object::GetMTime() )
      {
      PreCompute() ;
      }
    P->Up() ;
    }
  return m_TransformMatrix * point + m_Offset ;
}


template< class TData >
typename DiffusionTensor3DMatrix3x3Transform< TData >::TensorDataType
DiffusionTensor3DMatrix3x3Transform< TData >
::EvaluateTransformedTensor( TensorDataType &tensor , PointType &outputPosition )
{
  return EvaluateTransformedTensor( tensor ) ;
}



template< class TData >
typename DiffusionTensor3DMatrix3x3Transform< TData >::TensorDataType
DiffusionTensor3DMatrix3x3Transform< TData >
::EvaluateTransformedTensor( TensorDataType &tensor )
{
  InternalTensorDataType internalTensor = tensor ;
  if( latestTime < Object::GetMTime() )
    {
    P->Down() ;
    if( latestTime < Object::GetMTime() )
      {
      PreCompute() ;
      }
    P->Up() ;
    }
  InternalMatrixDataType tensorMatrix = internalTensor.GetTensor2Matrix() ;
  InternalMatrixTransformType mat = this->m_Transform
         * ( InternalMatrixTransformType ) tensorMatrix
         * this->m_TransformT ;
  tensorMatrix = ( InternalMatrixDataType ) mat ;
  internalTensor.SetTensorFromMatrix( static_cast< MatrixDataType >( tensorMatrix ) ) ;
  return static_cast< TensorDataType >( internalTensor ) ;
}


}//end namespace itk

#endif
