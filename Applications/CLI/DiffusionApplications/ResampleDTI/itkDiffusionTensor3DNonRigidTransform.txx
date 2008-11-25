/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef __itkDiffusionTensor3DNonRigidTransform_txx
#define __itkDiffusionTensor3DNonRigidTransform_txx

#include "itkDiffusionTensor3DNonRigidTransform.h"

namespace itk
{

template< class TData >
void 
DiffusionTensor3DNonRigidTransform< TData >
::SetAffineTransformType(typename AffineTransform::Pointer transform)
{
  m_Affine=transform;
}

template< class TData >
typename DiffusionTensor3DNonRigidTransform< TData >::PointType
DiffusionTensor3DNonRigidTransform< TData >
::EvaluateTensorPosition( const PointType &point )
{
  if( m_Transform.IsNotNull() )
    {
    return m_Transform->TransformPoint(point) ;
    }
  else
    {
    PointType zeroPoint( 0 ) ;
    return zeroPoint ;
    }
}

template< class TData >
typename DiffusionTensor3DNonRigidTransform< TData >::TensorDataType
DiffusionTensor3DNonRigidTransform< TData >
::EvaluateTransformedTensor( TensorDataType &tensor , PointType &outputPosition )
{
  if( m_Transform.IsNotNull() && m_Affine.IsNotNull() )
    {
    MatrixTransformType matrix ;
    matrix.SetIdentity() ;
    typename TransformType::JacobianType jacobian ;
    jacobian = m_Transform->GetJacobian( outputPosition ) ;
    for( int i = 0 ; i < 3 ; i++ )
      {
      for( int j = 0 ; j < 3 ; j++ )
        {
        matrix[ i ][ j ] = jacobian[ i ][ j ] + matrix[ i ][ j ] ;
        }
      }
    m_Affine->SetMatrix3x3( matrix ) ;
    return m_Affine->EvaluateTransformedTensor( tensor ) ;
    }
  else
    {
    TensorDataType returnedTensor( NumericTraits< DataType >::Zero ) ;
    return returnedTensor ;
    }
}
    
    
}//end of itk namespace

#endif

