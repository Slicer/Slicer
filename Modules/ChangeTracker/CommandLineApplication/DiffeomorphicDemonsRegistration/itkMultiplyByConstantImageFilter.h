#ifndef __itkMultiplyByConstantImageFilter_h
#define __itkMultiplyByConstantImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkNumericTraits.h"

namespace itk
{
  
/** \class MultiplyByConstantImageFilter
 *
 * \brief Multiply input pixels by a constant.
 *
 * This filter is templated over the input image type
 * and the output image type.
 *
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 *
 * \ingroup IntensityImageFilters  Multithreaded
 * \sa UnaryFunctorImageFilter
 * \sa ExtractImageFilter
 */
namespace Functor {  
  
template< class TInput, class TConstant, class TOutput>
class MultiplyByConstant
{
public:
   MultiplyByConstant() : m_Constant(NumericTraits<TConstant>::One) {};
   ~MultiplyByConstant() {};
   bool operator!=( const MultiplyByConstant & other ) const
   {
      return !(*this == other);
   }
   bool operator==( const MultiplyByConstant & other ) const
   {
      return other.m_Constant == m_Constant;
   }
   inline TOutput operator()( const TInput & A )
   {
      // Because the user has to specify the constant we don't
      // check if the cte is not 0;
      return static_cast<TOutput>( A * m_Constant );
   }
   void SetConstant(TConstant ct) {this->m_Constant = ct; };
   const TConstant GetConstant() const { return m_Constant; };
   
   TConstant m_Constant;
};
}

template <class TInputImage, class TConstant, class TOutputImage>
class ITK_EXPORT MultiplyByConstantImageFilter :
      public
UnaryFunctorImageFilter<TInputImage,TOutputImage, 
                        Functor::MultiplyByConstant< 
   typename TInputImage::PixelType, TConstant,
   typename TOutputImage::PixelType> >
{
public:
   /** Standard class typedefs. */
   typedef MultiplyByConstantImageFilter  Self;
   typedef UnaryFunctorImageFilter<TInputImage,TOutputImage, 
                                   Functor::MultiplyByConstant< 
      typename TInputImage::PixelType, TConstant,
      typename TOutputImage::PixelType>   
   >  Superclass;
   typedef SmartPointer<Self>   Pointer;
   typedef SmartPointer<const Self>  ConstPointer;

   /** Method for creation through the object factory. */
   itkNewMacro(Self);
  
   /** Run-time type information (and related methods). */
   itkTypeMacro(MultiplyByConstantImageFilter, UnaryFunctorImageFilter);

   void SetConstant(TConstant ct)
   {
      this->GetFunctor().SetConstant(ct);
      this->Modified();
   }
  

#ifdef ITK_USE_CONCEPT_CHECKING
   /** Begin concept checking */
   itkConceptMacro(InputConvertibleToOutputCheck,
                   (Concept::Convertible<typename TInputImage::PixelType,
                    typename TOutputImage::PixelType>));
   itkConceptMacro(Input1Input2OutputMultiplyOperatorCheck,
                   (Concept::MultiplyOperator<typename TInputImage::PixelType,
                    TConstant,
                    typename TOutputImage::PixelType>));
   /** End concept checking */
#endif

protected:
   MultiplyByConstantImageFilter() {};
   virtual ~MultiplyByConstantImageFilter() {};
   
   MultiplyByConstantImageFilter(const Self&); //purposely not implemented
   void operator=(const Self&); //purposely not implemented

};


} // end namespace itk

#endif
