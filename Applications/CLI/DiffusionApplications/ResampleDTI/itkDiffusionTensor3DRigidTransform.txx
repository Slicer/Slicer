#ifndef __itkDiffusionTensor3DRigidTransform_txx
#define __itkDiffusionTensor3DRigidTransform_txx

#include "itkDiffusionTensor3DRigidTransform.h"

namespace itk
{
    
template< class TData >
void
DiffusionTensor3DRigidTransform< TData >
::SetTransform( typename Rigid3DTransformType::Pointer transform )
{
  MatrixTransformType matrix3x3 ;  
  for( int i = 0 ; i < 3 ; i++ )
    {
    for( int j = 0 ; j < 3 ; j++ )
      {
      matrix3x3[ i ][ j ] = transform->GetParameters().GetElement( i * 3 + j ) ;
      }
    this->m_Translation[ i ] = transform->GetFixedParameters().GetElement( i ) ;
    }
  SetMatrix3x3( matrix3x3 ) ;
}    
    
template< class TData >
void
DiffusionTensor3DRigidTransform< TData >
::SetMatrix4x4( MatrixTransform4x4Type matrix )
{
  MatrixTransformType matrix3x3 ;
  for( int i = 0 ; i < 3 ; i++ )
    {
    for( int j = 0 ; j < 3 ; j++ )
      {
      matrix3x3[ i ][ j ] = matrix[ i ][ j ] ;
      }
    this->m_Translation[ i ] = matrix[ i ][ 3 ] ;
    }
  SetMatrix3x3(matrix3x3);
}

template< class TData >
void
DiffusionTensor3DRigidTransform< TData >
::SetMatrix3x3( MatrixTransformType &matrix )
{
  Matrix< double , 3 , 3 > result ;
  result = matrix * matrix.GetTranspose() ;
  bool ok = 1 ;
  for( int i = 0 ; i < 3 ; i++ )
    {
    for( int j = 0 ; j < 3 ; j++ )
      {
      if( i != j && result[ i ][ j ] > PRECISION ) 
        {
        ok = 0 ;
        break ;
        }
      else if( i == j && ( result[ i ][ j ]< 1.0 - PRECISION || result[ i ][ j ] > 1.0 + PRECISION ) )
        {
        ok = 0 ;
        break ;
        }
      }
    if( !ok ) break ;
    }
  if( ok )
    {
    double det = this->GetDet( matrix ) ;
    if( det > 1 - PRECISION && det < 1 + PRECISION )
      {
      this->m_TransformMatrix = matrix ;
      }
    else
      {
      itkExceptionMacro( << " Matrix is not a rotation matrix" ) ;
      }
    }
  else
    {
    itkExceptionMacro( << " Matrix is not a rotation matrix" ) ;
    }
}

template< class TData >
double
DiffusionTensor3DRigidTransform< TData >
::GetDet( MatrixTransformType &matrix )
{
  double det = 0 ;
  det = matrix[ 0 ][ 0 ] * ( matrix[ 1 ][ 1 ] * matrix[ 2 ][ 2 ] - matrix[ 2 ][ 1 ] * matrix[ 1 ][ 2 ] ) ;
  det -= matrix[ 1 ][ 0 ] * ( matrix[ 0 ][ 1 ] * matrix[ 2 ][ 2 ] - matrix[ 2 ][ 1 ] * matrix[ 0 ][ 2 ] ) ;
  det += matrix[ 2 ][ 0 ] * ( matrix[ 0 ][ 1 ] * matrix[ 1 ][ 2 ] - matrix[ 1 ][ 1 ] * matrix[ 0 ][ 2 ] ) ;
  return det ;
}


template< class TData >
void
DiffusionTensor3DRigidTransform< TData >
::PreCompute()
{
  InternalMatrixTransformType m_TransformMatrixInverse ;
  InternalMatrixTransformType MeasurementFrameTranspose = this->m_MeasurementFrame.GetTranspose() ;
  m_TransformMatrixInverse =  this->m_TransformMatrix.GetTranspose() ;
  InternalMatrixTransformType TransformMatrixTranspose = this->m_TransformMatrix.GetTranspose() ;
  this->m_TransformT = MeasurementFrameTranspose * this->m_TransformMatrix ;
  this->m_Transform = TransformMatrixTranspose * this->m_MeasurementFrame ;
  this->ComputeOffset() ;
  this->latestTime = Object::GetMTime() ;
}


}//end itk namespace
#endif
