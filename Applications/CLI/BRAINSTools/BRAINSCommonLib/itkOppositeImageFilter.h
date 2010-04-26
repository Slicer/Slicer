/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkOppositeImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009-02-19 21:18:10 $
  Version:   $Revision: 0.0 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkOppositeImageFilter_h
#define __itkOppositeImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkNumericTraits.h"

namespace itk
{
  
/** \class OppositeImageFilter
 *
 * \brief Take the opposite of the input pixels.
 *
 * This filter is templated over the input image type
 * and the output image type.
 *
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 *
 * \ingroup IntensityImageFilters  Multithreaded
 * \sa UnaryFunctorImageFilter
 */
namespace Functor {  
  
template< class TInput, class TOutput>
class Opposite
{
public:
  Opposite() {};
  ~Opposite() {};
  bool operator!=( const Opposite & other ) const
    {
    return false;
    }
  bool operator==( const Opposite & other ) const
    {
    return true;
    }
  inline TOutput operator()( const TInput & A ) const
    {
    // We don't check if the TOutput can be signed.
    // It's up to the user to decide whether this makes sense.
    return static_cast<TOutput>( - A );
    }
};
}

template <class TInputImage, class TOutputImage>
class ITK_EXPORT OppositeImageFilter :
      public
UnaryFunctorImageFilter<TInputImage,TOutputImage, 
                        Functor::Opposite< 
   typename TInputImage::PixelType,
   typename TOutputImage::PixelType> >
{
public:
  /** Standard class typedefs. */
  typedef OppositeImageFilter                           Self;
  typedef UnaryFunctorImageFilter<
    TInputImage,TOutputImage, 
    Functor::Opposite< 
      typename TInputImage::PixelType,
      typename TOutputImage::PixelType>   >             Superclass;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(OppositeImageFilter, UnaryFunctorImageFilter);
  

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(InputConvertibleToOutputCheck,
                  (Concept::Convertible<typename TInputImage::PixelType,
                   typename TOutputImage::PixelType>));
  /** End concept checking */
#endif

protected:
  OppositeImageFilter() {};
  virtual ~OppositeImageFilter() {};
   
  void PrintSelf(std::ostream &os, Indent indent) const
    {
    Superclass::PrintSelf(os, indent);
    }

private:
  OppositeImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace itk

#endif
