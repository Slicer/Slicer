/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef __itkDiffusionTensor3DNearestNeighborInterpolateFunction_txx
#define __itkDiffusionTensor3DNearestNeighborInterpolateFunction_txx

#include "itkDiffusionTensor3DNearestNeighborInterpolateFunction.h"

namespace itk
{

template< class TData >
DiffusionTensor3DNearestNeighborInterpolateFunction< TData >
::DiffusionTensor3DNearestNeighborInterpolateFunction()
{
  m_Origin.Fill( NumericTraits< double >::Zero ) ;
  m_End.Fill( NumericTraits< double >::Zero ) ;
  lock = MutexLock::New() ;
}

template< class TData >
void
DiffusionTensor3DNearestNeighborInterpolateFunction< TData >
::PreComputeCorners()
{
  //Compute position of the lower and superior corner of the image
  typename DiffusionImageType::SizeType size
         = this->m_InputImage->GetLargestPossibleRegion().GetSize() ;    
  typename DiffusionImageType::IndexType index ;
  index.Fill( 0 ) ;
  this->m_InputImage->TransformIndexToPhysicalPoint( index , m_Origin ) ;
  for( int i = 0 ; i < 3 ; i++ )
    {
    index[ i ] = size[ i ] - 1 ;
    }
  this->m_InputImage->TransformIndexToPhysicalPoint( index , m_End ) ;
  double temp ;
  for( int i = 0 ; i < 3 ; i++ )
    {
    if( m_End[ i ] < m_Origin[ i ] )
      {
      temp = m_Origin[ i ] ;
      m_Origin[ i ] = m_End[ i ] ;
      m_End[ i ] = temp ;
      }
    }
  this->latestTime = Object::GetMTime() ;
}

template< class TData >
typename DiffusionTensor3DNearestNeighborInterpolateFunction< TData >
::TensorDataType
DiffusionTensor3DNearestNeighborInterpolateFunction< TData >
::Evaluate( const PointType &point )
{
  if( this->m_InputImage.IsNotNull() )
    {
    if( this->latestTime< Object::GetMTime() )
      { 
      lock->Lock() ;
      if( this->latestTime< Object::GetMTime() )
        {
        PreComputeCorners() ;
        }
      lock->Unlock() ;
      }
    bool ok = 1 ;
    typename DiffusionImageType::IndexType pixelIndex ;
    typename DiffusionImageType::RegionType region ;
    region = this->m_InputImage->GetLargestPossibleRegion() ;
    typename DiffusionImageType::SizeType size = region.GetSize() ;
    typedef typename DiffusionImageType::IndexType::IndexValueType ValueType ;
    ContinuousIndex< double , 3 > continuousIndex ;
    for( int i = 0 ; i < 3 ; i++ )
      {
      if( point[ i ] > this->m_End[ i ] || point[ i ] < this->m_Origin[ i ] )
        { ok = 0 ; }//order of the operations as in itkNearestNeighborhoodInterpolateImageFunction
      this->m_InputImage->TransformPhysicalPointToContinuousIndex( point , continuousIndex ) ;
      pixelIndex[ i ] = static_cast< ValueType >( vnl_math_rnd( continuousIndex[ i ] ) ) ;
      }
    if( ok )
      {
      return this->m_InputImage->GetPixel( pixelIndex ) ;
      }
    else
      {
      return this->m_DefaultPixel ;
      }
    }
  else
    {
    itkExceptionMacro( << " No InputImage set" ) ;
    }
}

}//end namespace itk

#endif
