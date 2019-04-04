/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DPPDAffineTransform_txx
#define itkDiffusionTensor3DPPDAffineTransform_txx

#include "itkDiffusionTensor3DPPDAffineTransform.h"

namespace itk
{

template <class TData>
void
DiffusionTensor3DPPDAffineTransform<TData>
::PreCompute()
{
  try
    {
    m_TransformMatrixInverse = this->m_TransformMatrix.GetInverse();
    }
  catch( ... )
    {
    itkExceptionMacro(<< "Transform matrix is not invertible" );
    }
  this->ComputeOffset();
  this->m_LatestTime = Object::GetMTime();
}

template <class TData>
typename DiffusionTensor3DPPDAffineTransform<TData>::TensorDataType
DiffusionTensor3DPPDAffineTransform<TData>
::EvaluateTransformedTensor( TensorDataType & tensor )
{
  InternalTensorDataType internalTensor = tensor;

  if( this->m_LatestTime < Object::GetMTime() )
    {
    this->m_Lock.lock();
    if( this->m_LatestTime < Object::GetMTime() )
      {
      PreCompute();
      }
    this->m_Lock.unlock();
    }
  EValuesType                       eigenValues;
  EVectorsType                      eigenVectors;
  DiffusionTensor3DExtended<double> tensorDouble ( tensor );
  InternalMatrixTransformType tensorMatrixDouble = tensorDouble.GetTensor2Matrix();
  // InternalMatrixDataType tensorMatrix = tensorDouble.GetTensor2Matrix() ;
  InternalMatrixTransformType MFT = this->m_MeasurementFrame.GetTranspose();
  tensorDouble.SetTensorFromMatrix( this->m_MeasurementFrame * tensorMatrixDouble * MFT );
  tensorDouble.ComputeEigenAnalysis( eigenValues, eigenVectors );
  if( eigenValues[0] == 0 && eigenValues[1] == 0 && eigenValues[2] == 0 )
    {
    return tensor;
    }
  VectorType e1;
  VectorType e2;
  VectorType n1;
  VectorType n2;
  for( int i = 0; i < 3; i++ )
    {
    e1[i] = eigenVectors[2][i];       // eigen values sorted in ascending order, Vectors in line
    e2[i] = eigenVectors[1][i];
    }
  // InternalMatrixTransformType transformMF = m_TransformMatrixInverse * ( InternalMatrixTransformType )
  // this->m_MeasurementFrame ;

// InternalMatrixTransformType transformMF=this->m_TransformMatrix * ( InternalMatrixTransformType )
// this->m_MeasurementFrame;
  n1 = m_TransformMatrixInverse  * e1;
  // n1 = transformMF  * e1 ;
  double normtemp = n1.GetVnlVector().two_norm();
  if( normtemp )
    {
    n1 /= normtemp;
    }
  n2 = m_TransformMatrixInverse * e2;
  // n2 = transformMF * e2 ;
  normtemp = n2.GetVnlVector().two_norm();
  if( normtemp )
    {
    n2 /= normtemp;
    }
  double     costheta = dot_product( e1.GetVnlVector(), n1.GetVnlVector() );
  VectorType axis;
  axis.SetVnlVector( vnl_cross_3d( e1.GetVnlVector(), n1.GetVnlVector() ) );
  double              norm = axis.GetVnlVector().two_norm();
  MatrixTransformType r1;
  if( norm < 0.00001 )
    {
    r1.SetIdentity();
    }
  else
    {
    r1 = ComputeMatrixFromAxisAndAngle( axis / norm, costheta );
    }
  VectorType pn2;
  pn2 = n2 - dot_product( n2.GetVnlVector(), n1.GetVnlVector() ) * n1;
  pn2 /= pn2.GetVnlVector().two_norm();
  VectorType r1e2 = r1 * e2;
  r1e2 /= r1e2.GetVnlVector().two_norm();
  costheta = dot_product( r1e2.GetVnlVector(), pn2.GetVnlVector() );
  MatrixTransformType r2;
  axis.SetVnlVector( vnl_cross_3d( r1e2.GetVnlVector(), pn2.GetVnlVector() ) );
  norm = axis.GetVnlVector().two_norm();
  if( norm < .00001 )
    {
    r2.SetIdentity();
    }
  else
    {
    r2 = ComputeMatrixFromAxisAndAngle( axis / norm, costheta );
    }
  InternalMatrixTransformType R = r2 * r1;
//  InternalMatrixDataType tensorMatrix = internalTensor.GetTensor2Matrix() ;
  InternalMatrixTransformType RTranspose = R.GetTranspose();
//  InternalMatrixTransformType MFT = this->m_MeasurementFrame.GetTranspose() ;
// InternalMatrixTransformType mat = R * ( InternalMatrixTransformType )tensorMatrix * RTranspose ;
  InternalMatrixTransformType mat = R * tensorDouble.GetTensor2Matrix() * RTranspose;
//  InternalMatrixTransformType mat = RTranspose * ( InternalMatrixTransformType ) this->m_MeasurementFrame
//                                * ( InternalMatrixTransformType )tensorMatrix * MFT * R ;
  InternalMatrixDataType tensorMatrix;
  tensorMatrix = ( InternalMatrixDataType ) mat;
  internalTensor.SetTensorFromMatrix( static_cast<MatrixDataType>( tensorMatrix ) );
  return static_cast<TensorDataType>( internalTensor );
}

template <class TData>
typename DiffusionTensor3DPPDAffineTransform<TData>::InternalMatrixTransformType
DiffusionTensor3DPPDAffineTransform<TData>
::ComputeMatrixFromAxisAndAngle( VectorType axis, double cosangle )
{
  double c = cosangle;

  if( cosangle < -1 )
    {
    cosangle = -1;
    }
  else if( cosangle > 1 )
    {
    cosangle = 1;
    }
  double                      s = sqrt( 1 - cosangle * cosangle );
  double                      C = 1 - c;
  double                      xs = axis[0] * s;
  double                      ys = axis[1] * s;
  double                      zs = axis[2] * s;
  double                      xC = axis[0] * C;
  double                      yC = axis[1] * C;
  double                      zC = axis[2] * C;
  double                      xyC = axis[0] * yC;
  double                      yzC = axis[1] * zC;
  double                      zxC = axis[2] * xC;
  InternalMatrixTransformType mat;
  mat[0][0] = axis[0] * xC + c;
  mat[0][1] = xyC - zs;
  mat[0][2] = zxC + ys;
  mat[1][0] = xyC + zs;
  mat[1][1] = axis[1] * yC + c;
  mat[1][2] = yzC - xs;
  mat[2][0] = zxC - ys;
  mat[2][1] = yzC + xs;
  mat[2][2] = axis[2] * zC + c;
  return mat;
}

} // end namespace itk
#endif
