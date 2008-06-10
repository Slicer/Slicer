
#ifndef __itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction_txx
#define __itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction_txx

#include "itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction.h"

#include "vnl/vnl_math.h"

namespace itk
{

/**
 * Define the number of neighbors
 */
template<class TInputImage, class TCoordRep>
const unsigned long
VectorLinearInterpolateNearestNeighborExtrapolateImageFunction< TInputImage, TCoordRep >
::m_Neighbors = 1 << TInputImage::ImageDimension;


/**
 * Constructor
 */
template<class TInputImage, class TCoordRep>
VectorLinearInterpolateNearestNeighborExtrapolateImageFunction< TInputImage, TCoordRep >
::VectorLinearInterpolateNearestNeighborExtrapolateImageFunction()
{

}


/**
 * PrintSelf
 */
template<class TInputImage, class TCoordRep>
void
VectorLinearInterpolateNearestNeighborExtrapolateImageFunction< TInputImage, TCoordRep >
::PrintSelf(std::ostream& os, Indent indent) const
{
   this->Superclass::PrintSelf(os,indent);
}


/**
 * Evaluate at image index position
 */
template<class TInputImage, class TCoordRep>
typename VectorLinearInterpolateNearestNeighborExtrapolateImageFunction< TInputImage, TCoordRep >
::OutputType
VectorLinearInterpolateNearestNeighborExtrapolateImageFunction< TInputImage, TCoordRep >
::EvaluateAtContinuousIndex(
   const ContinuousIndexType& index) const
{
   unsigned int dim;  // index over dimension

   /**
   * Compute base index = closest index below point
   * Compute distance from point to base index
   */
   signed long baseIndex[ImageDimension];
   IndexType neighIndex;
   double distance[ImageDimension];
   
   for( dim = 0; dim < ImageDimension; dim++ )
      {
      baseIndex[dim] = (long) vcl_floor(index[dim] );
      
      if (baseIndex[dim] >=  this->m_StartIndex[dim])
         {
         if (baseIndex[dim] <  this->m_EndIndex[dim])
            distance[dim] = index[dim] - double( baseIndex[dim] );
         else
            {
            baseIndex[dim] = this->m_EndIndex[dim];
            distance[dim] = 0.0;
            }
         }
      else
         {
         baseIndex[dim] = this->m_StartIndex[dim];
         distance[dim] = 0.0;
         }
      }
  
   /**
   * Interpolated value is the weight some of each of the surrounding
   * neighbors. The weight for each neighbour is the fraction overlap
   * of the neighbor pixel with respect to a pixel centered on point.
   */
   OutputType output;
   output.Fill( 0.0 );

   RealType totalOverlap = 0.0;

   for( unsigned int counter = 0; counter < m_Neighbors; counter++ )
      {

      double overlap = 1.0;          // fraction overlap
      unsigned int upper = counter;  // each bit indicates upper/lower neighbour

      // get neighbor index and overlap fraction
      for( dim = 0; dim < ImageDimension; dim++ )
         {

         if ( upper & 1 )
            {
            neighIndex[dim] = baseIndex[dim] + 1;
            overlap *= distance[dim];
            }
         else
            {
            neighIndex[dim] = baseIndex[dim];
            overlap *= 1.0 - distance[dim];
            }

         upper >>= 1;

         }
    
      // get neighbor value only if overlap is not zero
      if( overlap )
         {
         const PixelType input = this->GetInputImage()->GetPixel( neighIndex );
         for(unsigned int k = 0; k < Dimension; k++ )
            {
            output[k] += overlap * static_cast<RealType>( input[k] );
            }
         totalOverlap += overlap;
         }

      if( totalOverlap == 1.0 )
         {
         // finished
         break;
         }

      }

   return ( output );
}

} // end namespace itk

#endif
