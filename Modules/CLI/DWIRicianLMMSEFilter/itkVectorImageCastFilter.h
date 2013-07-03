/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkVectorImageCastFilter.h,v $
  Language:  C++
  Date:      $Date: 2003/09/10 14:28:51 $
  Version:   $Revision: 1.16 $

=========================================================================*/
#ifndef __itkVectorImageCastFilter_h
#define __itkVectorImageCastFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "vnl/vnl_math.h"

namespace itk
{
namespace Function
{
template <class TInput, class TOutput>
class Cast
{
public:
  Cast()
  {
  }
  ~Cast()
  {
  }
  inline TOutput operator()( const TInput & a )
  {
    TOutput b;

    b.SetSize( a.GetSize() );
    for( unsigned int k = 0; k < a.GetSize(); ++k )
      {
      b[k] = static_cast<typename TOutput::RealValueType>( a[k] );
      }
    return b;
  }

};
}

template <class TInputImage, class TOutputImage>
class VectorImageCastFilter :
  public UnaryFunctorImageFilter<TInputImage, TOutputImage,
                                 Function::Cast<typename TInputImage::PixelType, typename TOutputImage::PixelType>   >
{
public:
  /** Standard class typedefs. */
  typedef VectorImageCastFilter Self;
  typedef UnaryFunctorImageFilter<TInputImage,  TOutputImage,
                                  Function::Cast<typename TInputImage::PixelType, typename TOutputImage::PixelType>   >
  Superclass;
  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
protected:
  VectorImageCastFilter()
  {
  }
  virtual ~VectorImageCastFilter()
  {
  }
private:
  VectorImageCastFilter(const Self &); // purposely not implemented
  void operator=(const Self &);        // purposely not implemented

};

} // end namespace itk

#endif
