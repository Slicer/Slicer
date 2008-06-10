#ifndef __itkVectorCentralDifferenceImageFunction_txx
#define __itkVectorCentralDifferenceImageFunction_txx

#include "itkVectorCentralDifferenceImageFunction.h"

namespace itk
{


/**
 * Constructor
 */
template <class TInputImage, class TCoordRep>
VectorCentralDifferenceImageFunction<TInputImage,TCoordRep>
::VectorCentralDifferenceImageFunction()
{
}


/**
 *
 */
template <class TInputImage, class TCoordRep>
void
VectorCentralDifferenceImageFunction<TInputImage,TCoordRep>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os,indent);
}


/**
 *
 */
template <class TInputImage, class TCoordRep>
typename VectorCentralDifferenceImageFunction<TInputImage,TCoordRep>::OutputType
VectorCentralDifferenceImageFunction<TInputImage,TCoordRep>
::EvaluateAtIndex( const IndexType& index ) const
{
  
  OutputType derivative;
  derivative.Fill( 0.0 );
  
  IndexType neighIndex = index;
  InputPixelType pix;

  const typename InputImageType::SizeType& size =
    this->GetInputImage()->GetBufferedRegion().GetSize();
  const typename InputImageType::IndexType& start =
    this->GetInputImage()->GetBufferedRegion().GetIndex();

  for ( unsigned int dim = 0; dim < TInputImage::ImageDimension; dim++ )
    {
    // bounds checking
    if( index[dim] < static_cast<long>(start[dim]) + 1 ||
        index[dim] > (start[dim] + static_cast<long>(size[dim]) - 2 ) )
      {
      for (unsigned int vdim=0; vdim<Dimension; ++vdim)
        {
        derivative(vdim,dim) = 0.0;
        }
      continue;
      }
    
    // compute derivative
    neighIndex[dim] += 1;
    pix = this->GetInputImage()->GetPixel( neighIndex );
    for (unsigned int vdim=0; vdim<Dimension; ++vdim)
      {
      derivative(vdim,dim) = pix[vdim];
      }

    neighIndex[dim] -= 2;
    pix = this->GetInputImage()->GetPixel( neighIndex );
    for (unsigned int vdim=0; vdim<Dimension; ++vdim)
      {
      derivative(vdim,dim) -= pix[vdim];
      }

    for (unsigned int vdim=0; vdim<Dimension; ++vdim)
      {
      derivative(vdim,dim) *= 0.5 / this->GetInputImage()->GetSpacing()[dim];
      }
    neighIndex[dim] += 1;
    }

  return ( derivative );

}


} // end namespace itk

#endif
