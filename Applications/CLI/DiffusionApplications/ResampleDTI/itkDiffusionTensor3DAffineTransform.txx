/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef __itkDiffusionTensor3DAffineTransform_txx
#define __itkDiffusionTensor3DAffineTransform_txx

#include "itkDiffusionTensor3DAffineTransform.h"

namespace itk
{


template< class TData >
void
DiffusionTensor3DAffineTransform< TData >
::SetTransform( typename AffineTransformType::Pointer transform )
{
  for( int i = 0 ; i < 3 ; i++ )
    {
    for( int j = 0 ; j < 3 ; j++ )
      {
      this->m_TransformMatrix[ i ][ j ] = transform->GetParameters().GetElement( i * 3 + j ) ;
      }
    this->m_Translation[ i ] = transform->GetFixedParameters().GetElement( i ) ;
    }
}    



template< class TData >
void
DiffusionTensor3DAffineTransform< TData >
::SetMatrix4x4( MatrixTransform4x4Type matrix )
{
  for( int i = 0 ; i < 3 ; i++ )
    {
    for( int j = 0 ; j < 3 ; j++ )
      {
      this->m_TransformMatrix[ i ][ j ] = matrix[ i ][ j ] ;
      }
    this->m_Translation[ i ] = matrix[ i ][ 3 ] ;
    }
}



}//end namespace itk
#endif
