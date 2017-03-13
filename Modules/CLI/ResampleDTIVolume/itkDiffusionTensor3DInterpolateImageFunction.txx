/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DInterpolateImageFunction_txx
#define itkDiffusionTensor3DInterpolateImageFunction_txx

#include "itkDiffusionTensor3DInterpolateImageFunction.h"

namespace itk
{

template <class TData, class TCoordRep>
DiffusionTensor3DInterpolateImageFunction<TData, TCoordRep>
::DiffusionTensor3DInterpolateImageFunction()
{
//  m_InputImage = 0 ;
  m_LatestTime = 0;
//  SetDefaultPixelValue( ITK_DIFFUSION_TENSOR_3D_ZERO ) ;
}

/*
template< class TData >
void
DiffusionTensor3DInterpolateImageFunction< TData >
::SetDefaultPixelValue( TensorRealType defaultPixelValue )
{
  m_DefaultPixelValue = defaultPixelValue ;
  m_DefaultPixel.SetIdentity() ;
  for( unsigned int i = 0 ; i < 3 ; i++ )
    {
    m_DefaultPixel( i , i ) *= static_cast< TData >( this->m_DefaultPixelValue ) ;
    }
}
*/

} // end namespace itk

#endif
