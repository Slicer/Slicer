#ifndef __itkDiffusionTensor3DNearestNeighborInterpolateFunction_txx
#define __itkDiffusionTensor3DNearestNeighborInterpolateFunction_txx

#include "itkDiffusionTensor3DNearestNeighborInterpolateFunction.h"

namespace itk
{



template< class TData >
typename DiffusionTensor3DNearestNeighborInterpolateFunction< TData >
::TensorDataType
DiffusionTensor3DNearestNeighborInterpolateFunction< TData >
::Evaluate( const PointType &point )
{
  TensorDataType pixelValue( NumericTraits< DataType >::Zero ) ;
  if( this->m_InputImage.IsNotNull() )
    {
    if( this->latestTime< Object::GetMTime() )
      { 
      this->P->Down() ;
      if( this->latestTime< Object::GetMTime() )
        {
        this->PreComputeCorners() ;
        }
      this->P->Up() ;
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
      pixelValue = this->m_InputImage->GetPixel( pixelIndex ) ;
      }
    }
  else
    {
    itkExceptionMacro( << " No InputImage set" ) ;
    }
  return pixelValue ;
}

}//end namespace itk

#endif
