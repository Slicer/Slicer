#ifndef __itkDiffusionTensor3DInterpolateImageFunction_txx
#define __itkDiffusionTensor3DInterpolateImageFunction_txx

#include "itkDiffusionTensor3DInterpolateImageFunction.h"

namespace itk
{

template< class TData >
DiffusionTensor3DInterpolateImageFunction< TData >
::DiffusionTensor3DInterpolateImageFunction()
{
  m_InputImage = 0 ;
  latestTime = 0 ;
  P = Semaphore::New() ;
  P->Initialize( 1 ) ;
  m_Origin.Fill( NumericTraits< TensorType >::Zero ) ;
  m_End.Fill( NumericTraits< TensorType >::Zero ) ;
}

template< class TData >
void
DiffusionTensor3DInterpolateImageFunction< TData >
::PreComputeCorners()
{
  //Compute position of the lower and superior corner of the image
  typename DiffusionImageType::SizeType size
         = m_InputImage->GetLargestPossibleRegion().GetSize() ;    
  typename DiffusionImageType::IndexType index ;
  index.Fill( 0 ) ;
  m_InputImage->TransformIndexToPhysicalPoint( index , m_Origin ) ;
  for( int i = 0 ; i < 3 ; i++ )
    { index[ i ] = size[ i ] - 1 ; }
  m_InputImage->TransformIndexToPhysicalPoint( index , m_End ) ;
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
  latestTime = Object::GetMTime() ;
}


}//end namespace itk

#endif
