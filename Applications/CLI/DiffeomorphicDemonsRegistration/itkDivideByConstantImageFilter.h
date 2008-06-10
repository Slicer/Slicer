#ifndef __itkDivideByConstantImageFilter_h
#define __itkDivideByConstantImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkNumericTraits.h"

namespace itk
{
  
/** \class DivideByConstantImageFilter
 *
 * \brief Divide input pixels by a constant.
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
class DivideByConstant
{
public:
   DivideByConstant() : m_Constant(NumericTraits<TConstant>::One) {};
   ~DivideByConstant() {};
   bool operator!=( const DivideByConstant & other ) const
   {
      return !(*this == other);
   }
   bool operator==( const DivideByConstant & other ) const
   {
      return other.m_Constant == m_Constant;
   }
   inline TOutput operator()( const TInput & A )
   {
      // Because the user has to specify the constant we don't
      // check if the cte is not too small (i.e. almost equal to zero);
      return static_cast<TOutput>( A / m_Constant );
   }
   void SetConstant(TConstant ct)
   {
      if ( ct==NumericTraits<TConstant>::Zero )
      {
         itkGenericExceptionMacro(
            <<"The constant value used as denominator should not be set to zero");
      }

      this->m_Constant = ct;
   };
   const TConstant GetConstant() const { return m_Constant; };
   
   TConstant m_Constant;
};
}

template <class TInputImage, class TConstant, class TOutputImage>
class ITK_EXPORT DivideByConstantImageFilter :
      public
UnaryFunctorImageFilter<TInputImage,TOutputImage, 
                        Functor::DivideByConstant< 
   typename TInputImage::PixelType, TConstant,
   typename TOutputImage::PixelType> >
{
public:
   /** Standard class typedefs. */
   typedef DivideByConstantImageFilter  Self;
   typedef UnaryFunctorImageFilter<TInputImage,TOutputImage, 
                                   Functor::DivideByConstant< 
      typename TInputImage::PixelType, TConstant,
      typename TOutputImage::PixelType>   
   >  Superclass;
   typedef SmartPointer<Self>   Pointer;
   typedef SmartPointer<const Self>  ConstPointer;

   /** Method for creation through the object factory. */
   itkNewMacro(Self);
  
   /** Run-time type information (and related methods). */
   itkTypeMacro(DivideByConstantImageFilter, UnaryFunctorImageFilter);

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
   //itkConceptMacro(Input1Input2OutputDivisionOperatorsCheck,
   //                (Concept::DivisionOperators<typename TInputImage::PixelType,
   //                 TConstant,
   //                 typename TOutputImage::PixelType>));
   /** End concept checking */
#endif

protected:
   DivideByConstantImageFilter() {};
   virtual ~DivideByConstantImageFilter() {};
   
   DivideByConstantImageFilter(const Self&); //purposely not implemented
   void operator=(const Self&); //purposely not implemented

};


} // end namespace itk

#endif
