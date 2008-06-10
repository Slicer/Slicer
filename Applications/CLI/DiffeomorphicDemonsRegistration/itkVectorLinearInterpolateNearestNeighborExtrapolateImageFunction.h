#ifndef __itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction_h
#define __itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction_h

#include "itkVectorInterpolateImageFunction.h"

namespace itk
{

/** 
 * \class VectorLinearInterpolateNearestNeighborExtrapolateImageFunction
 * \brief Linearly interpolate or NN extrapolate a vector image at
 * specified positions.
 *
 * VectorLinearInterpolateNearestNeighborExtrapolateImageFunction
 * linearly interpolates (or NN extrapolates) a vector
 * image intensity non-integer pixel position. This class is templated
 * over the input image type and the coordinate representation type.
 *
 * This class is designed to work as a VectorInterpolateImageFunction,
 * hence the  IsInsideBuffer(PointType p) is overridden to always
 * answer true
 *
 * This function works for N-dimensional images.
 *
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 *
 * \warning This function work only for Vector images. For
 * scalar images use LinearInterpolateImageFunction.
 *
 *
 * \ingroup ImageFunctions ImageInterpolators
 * 
 */
template <class TInputImage, class TCoordRep = float>
class ITK_EXPORT VectorLinearInterpolateNearestNeighborExtrapolateImageFunction : 
      public VectorInterpolateImageFunction<TInputImage,TCoordRep> 
{
public:
   /** Standard class typedefs. */
   typedef VectorLinearInterpolateNearestNeighborExtrapolateImageFunction Self;
   typedef VectorInterpolateImageFunction<TInputImage,TCoordRep> Superclass;
   typedef SmartPointer<Self> Pointer;
   typedef SmartPointer<const Self>  ConstPointer;
  
   /** Method for creation through the object factory. */
   itkNewMacro(Self);  

   /** Run-time type information (and related methods). */
   itkTypeMacro(VectorLinearInterpolateNearestNeighborExtrapolateImageFunction, 
                VectorInterpolateImageFunction);

   /** InputImageType typedef support. */
   typedef typename Superclass::InputImageType InputImageType;
   typedef typename Superclass::PixelType      PixelType;
   typedef typename Superclass::ValueType      ValueType;
   typedef typename Superclass::RealType       RealType;

   typedef typename Superclass::PointType      PointType;
    
   /** Grab the vector dimension from the superclass. */
   itkStaticConstMacro(Dimension, unsigned int,
                       Superclass::Dimension);

   /** Dimension underlying input image. */
   itkStaticConstMacro(ImageDimension, unsigned int,Superclass::ImageDimension);

   /** Index typedef support. */
   typedef typename Superclass::IndexType IndexType;

   /** ContinuousIndex typedef support. */
   typedef typename Superclass::ContinuousIndexType ContinuousIndexType;

   /** Output type is Vector<double,Dimension> */
   typedef typename Superclass::OutputType OutputType;

   /** Should check if a point is inside the image buffer however we
   * require that it answers true to use the extrapolation
   * possibility. */
   virtual bool IsInsideBuffer( const PointType & point ) const
   { 
      return true;
   };

   virtual bool IsInsideBuffer( const ContinuousIndexType & index ) const
   { 
      return true;
   };

   /** Evaluate the function at a ContinuousIndex position
   *
   * Returns the linearly interpolated image intensity at a 
   * specified point position. If the point does not lie within the
   * image buffer a nearest neighbor interpolation is done. */
   virtual OutputType EvaluateAtContinuousIndex( 
      const ContinuousIndexType & index ) const;

protected:
   VectorLinearInterpolateNearestNeighborExtrapolateImageFunction();
   ~VectorLinearInterpolateNearestNeighborExtrapolateImageFunction(){};
   void PrintSelf(std::ostream& os, Indent indent) const;

private:
   VectorLinearInterpolateNearestNeighborExtrapolateImageFunction(const Self&); //purposely not implemented
   void operator=(const Self&); //purposely not implemented

   /** Number of neighbors used in the interpolation */
   static const unsigned long  m_Neighbors;  

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction.txx"
#endif

#endif
