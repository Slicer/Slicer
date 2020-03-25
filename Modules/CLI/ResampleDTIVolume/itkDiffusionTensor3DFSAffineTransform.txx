/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DFSAffineTransform_txx
#define itkDiffusionTensor3DFSAffineTransform_txx

#include "itkDiffusionTensor3DFSAffineTransform.h"

namespace itk
{

template <class TData>
typename DiffusionTensor3DFSAffineTransform<TData>::MatrixTransformType
DiffusionTensor3DFSAffineTransform<TData>
::ComputeMatrixSquareRoot( MatrixTransformType matrix )
{
  MatrixTransformType sqrMatrix;

  vnl_matrix_fixed<double, 3, 3> M( 3, 3 );
  M = matrix.GetVnlMatrix();
  vnl_real_eigensystem             eig( M.as_matrix() );
  vnl_matrix_fixed<std::complex<double>, 3, 3 > D( 3, 3 );
  vnl_matrix_fixed<std::complex<double>, 3, 3 > vnl_sqrMatrix( 3, 3 );
  D.fill( NumericTraits<TData>::ZeroValue() );
  for( int i = 0; i < 3; i++ )
    {
    D.put( i, i, std::pow( eig.D.get( i, i ), 0.5 ) );
    }
  vnl_sqrMatrix = eig.V * D * vnl_matrix_inverse<std::complex<double> >( eig.V ).as_matrix();
  vnl_matrix_fixed<double, 3, 3> vnl_sqrMatrix_real( 3, 3 );
  vnl_sqrMatrix_real = vnl_real( vnl_sqrMatrix );
  for( int i = 0; i < 3; i++ )
    {
    for( int j = 0; j < 3; j++ )
      {
      sqrMatrix[i][j] = vnl_sqrMatrix_real.get( i, j );
      }
    }
  return sqrMatrix;
}

template <class TData>
void
DiffusionTensor3DFSAffineTransform<TData>
::PreCompute()
{
  InternalMatrixTransformType m_RotationMatrix;

  m_RotationMatrix = ComputeRotationMatrixFromTransformationMatrix();
  InternalMatrixTransformType MeasurementFrameTranspose = this->m_MeasurementFrame.GetTranspose();
  InternalMatrixTransformType RotationMatrixTranspose = m_RotationMatrix.GetTranspose();
// Instead of computing the inverse transform (which go from the input image to the output image)
// and compute the rotation matrix from the inverse, we compute the rotation matrix from the original
// transform and takes its inverse (=transpose since it is a rotation). Both rotation matrices are equals,
// therefore we avoid computing the inverse of our original transform since it is not necessary.
  this->m_TransformT = MeasurementFrameTranspose * m_RotationMatrix;
  this->m_Transform = RotationMatrixTranspose * this->m_MeasurementFrame;

  this->ComputeOffset();
  this->m_LatestTime = Object::GetMTime();
}

template <class TData>
typename DiffusionTensor3DFSAffineTransform<TData>::MatrixTransformType
DiffusionTensor3DFSAffineTransform<TData>
::ComputeRotationMatrixFromTransformationMatrix()
{
  MatrixTransformType         m_RotationMatrix;
  InternalMatrixTransformType TransformMatrixTranspose = this->m_TransformMatrix.GetTranspose();
  MatrixTransformType         matrix;

  try
    {
    matrix = ComputeMatrixSquareRoot(
        this->m_TransformMatrix * TransformMatrixTranspose ).GetInverse();
    }
  catch( ... )
    {
    itkExceptionMacro(<< "Matrix is not invertible while computing rotation matrix" );
    }
  m_RotationMatrix = matrix * static_cast<MatrixTransformType>( this->m_TransformMatrix );
  return m_RotationMatrix;
}

} // end namespace itk
#endif
