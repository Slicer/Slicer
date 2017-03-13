/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DNearestNeighborInterpolateFunction_txx
#define itkDiffusionTensor3DNearestNeighborInterpolateFunction_txx

#include "itkDiffusionTensor3DNearestNeighborInterpolateFunction.h"

namespace itk
{

template <class TData, class TCoordRep>
typename DiffusionTensor3DNearestNeighborInterpolateFunction<TData, TCoordRep>
::TensorDataType
DiffusionTensor3DNearestNeighborInterpolateFunction<TData, TCoordRep>
::EvaluateAtContinuousIndex( const ContinuousIndexType & index ) const
// ::Evaluate( const PointType &point )
{
  if( this->m_Image.IsNotNull() )
    {
    typename DiffusionImageType::IndexType pixelIndex;
    this->ConvertContinuousIndexToNearestIndex( index, pixelIndex );
    return this->m_Image->GetPixel( pixelIndex );
    }
  else
    {
    itkExceptionMacro( << " No InputImage set" );
    }
}

} // end namespace itk

#endif
