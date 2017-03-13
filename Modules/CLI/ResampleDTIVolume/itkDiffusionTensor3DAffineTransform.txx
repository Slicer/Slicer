/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DAffineTransform_txx
#define itkDiffusionTensor3DAffineTransform_txx

#include "itkDiffusionTensor3DAffineTransform.h"

namespace itk
{

template <class TData>
void
DiffusionTensor3DAffineTransform<TData>
::SetTransform( typename AffineTransformType::Pointer transform )
{
/*  for( int i = 0 ; i < 3 ; i++ )
    {
    for( int j = 0 ; j < 3 ; j++ )
      {
      this->m_TransformMatrix[ i ][ j ] = transform->GetParameters().GetElement( i * 3 + j ) ;
      }
    this->m_Translation[ i ] = transform->GetFixedParameters().GetElement( i ) ;
    }*/
  this->m_TransformMatrix = transform->GetMatrix();
  this->m_Translation = transform->GetTranslation();
  this->Modified();
}

template <class TData>
typename AffineTransform<double, 3>::Pointer
DiffusionTensor3DAffineTransform<TData>
::GetAffineTransform()
{
  typename AffineTransformType::Pointer affineTransform = AffineTransformType::New();
  affineTransform->SetMatrix( this->m_TransformMatrix );
  affineTransform->SetTranslation( this->m_Translation );
  return affineTransform;
}

template <class TData>
void
DiffusionTensor3DAffineTransform<TData>
::SetMatrix4x4( MatrixTransform4x4Type matrix )
{
  for( int i = 0; i < 3; i++ )
    {
    for( int j = 0; j < 3; j++ )
      {
      this->m_TransformMatrix[i][j] = matrix[i][j];
      }
    this->m_Translation[i] = matrix[i][3];
    }
  this->Modified();
}

} // end namespace itk
#endif
