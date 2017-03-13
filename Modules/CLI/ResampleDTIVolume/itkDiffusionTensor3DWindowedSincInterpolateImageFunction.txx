/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DWindowedSincInterpolateImageFunction_txx
#define itkDiffusionTensor3DWindowedSincInterpolateImageFunction_txx

#include "itkDiffusionTensor3DWindowedSincInterpolateImageFunction.h"

namespace itk
{

template <class TData,
          unsigned int VRadius,
          class TWindowFunction,
          class TBoundaryCondition,
          class TCoordRep
          >
void
DiffusionTensor3DWindowedSincInterpolateImageFunction<TData,
                                                      VRadius,
                                                      TWindowFunction,
                                                      TBoundaryCondition,
                                                      TCoordRep
                                                      >
::AllocateInterpolator()
{
  for( int i = 0; i < 6; i++ )
    {
    windowedSincInterpolator[i] = WindowedSincInterpolateImageFunctionType::New();
    this->m_Interpol[i] = windowedSincInterpolator[i];
    }
}

} // end itk namespace

#endif
