/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DLinearInterpolateFunction_txx
#define itkDiffusionTensor3DLinearInterpolateFunction_txx

#include "itkDiffusionTensor3DLinearInterpolateFunction.h"

namespace itk
{

template <class TData, class TCoordRep>
void
DiffusionTensor3DLinearInterpolateFunction<TData, TCoordRep>
::AllocateInterpolator()
{
  for( int i = 0; i < 6; i++ )
    {
    linearInterpolator[i] = LinearInterpolateImageFunctionType::New();
    this->m_Interpol[i] = linearInterpolator[i];
    }
}

} // end itk namespace

#endif
