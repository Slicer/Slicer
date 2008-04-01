#ifndef __itkDiffusionTensor3DFSAffineTransform_txx
#define __itkDiffusionTensor3DFSAffineTransform_txx

#include "itkDiffusionTensor3DFSAffineTransform.h"

namespace itk
{



template< class TData >
typename DiffusionTensor3DFSAffineTransform< TData > ::MatrixTransformType
DiffusionTensor3DFSAffineTransform< TData >
::ComputeMatrixSquareRoot( MatrixTransformType matrix )
{
  MatrixTransformType sqrMatrix ;
  vnl_matrix< double > M ( 3 , 3 ) ;
  M = matrix.GetVnlMatrix() ;
  vnl_real_eigensystem eig( M ) ;
  vnl_matrix< vcl_complex< double > >  D( 3 , 3 ) ;
  vnl_matrix< vcl_complex< double > >  vnl_sqrMatrix( 3 , 3 ) ;
  D.fill( NumericTraits< TData >::Zero ) ;
  for( int i = 0 ; i < 3 ; i++ )
    {
    D.put( i , i , vcl_pow( eig.D.get( i , i ) , 0.5 ) ) ;
    }
  vnl_sqrMatrix = eig.V * D * vnl_matrix_inverse< vcl_complex< double > > ( eig.V ) ;
  vnl_matrix< double > vnl_sqrMatrix_real( 3 , 3 ) ;
  vnl_sqrMatrix_real = vnl_real( vnl_sqrMatrix ) ;
  for( int i = 0 ; i < 3 ; i++ )
    {
    for( int j = 0 ; j < 3 ; j++ )
      {
      sqrMatrix[ i ][ j ] = vnl_sqrMatrix_real.get( i , j ) ;
      }
    }
  return sqrMatrix ;
}

template< class TData >
void
DiffusionTensor3DFSAffineTransform< TData >
::PreCompute()
{
  InternalMatrixTransformType m_RotationMatrix ;
  InternalMatrixTransformType m_TransformMatrixInverse ;
  m_RotationMatrix=ComputeRotationMatrixFromTransformationMatrix() ;
  m_TransformMatrixInverse= this->m_TransformMatrix.GetInverse() ;
  InternalMatrixTransformType MeasurementFrameTranspose = this->m_MeasurementFrame.GetTranspose() ;
  InternalMatrixTransformType RotationMatrixTranspose = m_RotationMatrix.GetTranspose() ;

  this->m_TransformT = MeasurementFrameTranspose * m_RotationMatrix ;
  this->m_Transform = RotationMatrixTranspose * this->m_MeasurementFrame ;

  this->ComputeOffset() ;
  this->latestTime = Object::GetMTime() ;
}


template< class TData >
typename DiffusionTensor3DFSAffineTransform< TData >::MatrixTransformType
DiffusionTensor3DFSAffineTransform< TData >
::ComputeRotationMatrixFromTransformationMatrix()
{
  MatrixTransformType m_RotationMatrix ;
  InternalMatrixTransformType TransformMatrixTranspose = this->m_TransformMatrix.GetTranspose() ;
  MatrixTransformType matrix( ComputeMatrixSquareRoot( 
          this->m_TransformMatrix * TransformMatrixTranspose ).GetInverse() ) ;
  m_RotationMatrix = matrix * static_cast< MatrixTransformType > ( this->m_TransformMatrix ) ;
  return m_RotationMatrix ;
}


}//end namespace itk
#endif
