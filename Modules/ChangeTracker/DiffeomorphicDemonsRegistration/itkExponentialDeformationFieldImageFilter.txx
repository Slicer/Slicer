
#ifndef _itkExponentialDeformationFieldImageFilter_txx
#define _itkExponentialDeformationFieldImageFilter_txx

#include "itkExponentialDeformationFieldImageFilter.h"
#include "itkProgressReporter.h"
#include "itkImageRegionConstIterator.h"

namespace itk
{

/**
 * Initialize new instance
 */
template <class TInputImage, class TOutputImage>
ExponentialDeformationFieldImageFilter<TInputImage, TOutputImage>
::ExponentialDeformationFieldImageFilter()
{
   m_AutomaticNumberOfIterations = true;
   m_MaximumNumberOfIterations = 20;
   m_Divider = DivideByConstantType::New();
   m_Caster = CasterType::New();
   m_Warper = VectorWarperType::New();

   FieldInterpolatorPointer VectorInterpolator =
      FieldInterpolatorType::New();
   m_Warper->SetInterpolator(VectorInterpolator);

   m_Adder = AdderType::New();
   m_Adder->InPlaceOn();
}


/**
 * Print out a description of self
 *
 * \todo Add details about this class
 */
template <class TInputImage, class TOutputImage>
void 
ExponentialDeformationFieldImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "AutomaticNumberOfIterations: " << m_AutomaticNumberOfIterations << std::endl;
  os << indent << "MaximumNumberOfIterations:     " << m_MaximumNumberOfIterations << std::endl;

  return;
}



/**
 * GenerateData
 */
template <class TInputImage, class TOutputImage>
void 
ExponentialDeformationFieldImageFilter<TInputImage,TOutputImage>
::GenerateData()
{
   itkDebugMacro(<<"Actually executing");

   InputImageConstPointer inputPtr = this->GetInput();

   unsigned int numiter = 0;

   if (m_AutomaticNumberOfIterations)
      {
      // Compute a good number of iterations based on the rationale
      // that the initial first order approximation,
      // exp(Phi/2^N) = Phi/2^N,
      // needs to be diffeomorphic. For this we simply impose to have
      // max(norm(Phi)/2^N) < 0.5*pixelspacing

      InputPixelRealValueType maxnorm2 = 0.0;

      double minpixelspacing = inputPtr->GetSpacing()[0];
      for (unsigned int i = 1; i<itkGetStaticConstMacro(ImageDimension); ++i)
         {
         if ( inputPtr->GetSpacing()[i] < minpixelspacing )
            minpixelspacing = inputPtr->GetSpacing()[i];
         }
      

      typedef ImageRegionConstIterator<InputImageType> InputConstIterator;
      InputConstIterator InputIt = InputConstIterator(
         inputPtr, inputPtr->GetRequestedRegion());

      for (InputIt.GoToBegin(); !InputIt.IsAtEnd(); ++InputIt)
         {
         InputPixelRealValueType norm2 = InputIt.Get().GetSquaredNorm();
         if (norm2>maxnorm2) maxnorm2=norm2;
         }

      //std::cout<<std::endl<<"maxnorm (units)="<<std::sqrt(maxnorm2)<<std::endl;

      // Divide the norm by the minimum pixel spacing
      maxnorm2 /= minpixelspacing;

      // max(norm(Phi)/2^N) < 0.5*pixelspacing
      //InputPixelRealValueType numiterfloat = 1.0 +
      //   0.5 * vcl_log(maxnorm2)/vnl_math::ln2;

      // max(norm(Phi))/2^N < 0.25*pixelspacing
      InputPixelRealValueType numiterfloat = 2.0 +
         0.5 * vcl_log(maxnorm2)/vnl_math::ln2;

      //std::cout<<std::endl<<"maxnorm (pixels)="<<std::sqrt(maxnorm2)<<std::endl;
      //std::cout<<std::endl<<"numiterfloat="<<numiterfloat<<std::endl;
      
      if (numiterfloat>=0.0)
         {
         // take the ceil and threshold
         numiter = std::min(
            static_cast<unsigned int>(numiterfloat + 1.0),
            m_MaximumNumberOfIterations );
         }
      else
         numiter = 0;
      }
   else
      numiter = m_MaximumNumberOfIterations;

   //std::cout<<std::endl<<"numiter = "<<numiter<<std::endl;
   
   ProgressReporter progress(this, 0, numiter+1, numiter+1);

   if (numiter==0)
      {
      m_Caster->SetInput(inputPtr);
      m_Caster->GraftOutput(this->GetOutput());
      m_Caster->Update();
      // Region passing stuff
      this->GraftOutput( m_Caster->GetOutput() );
      
      progress.CompletedPixel();
      return;
      }


   // Get the first order approximation (division by 2^numiter)
   m_Divider->SetConstant( static_cast<InputPixelRealValueType>(1<<numiter) );
   m_Divider->SetInput(inputPtr);
   m_Divider->GraftOutput( this->GetOutput() );
   m_Divider->Update();
   // Region passing stuff
   this->GraftOutput( m_Divider->GetOutput() );

   progress.CompletedPixel();


   // Do the iterative composition of the vector field
   m_Warper->SetOutputSpacing(inputPtr->GetSpacing());
   m_Warper->SetOutputOrigin(inputPtr->GetOrigin());
   
   for (unsigned int i=0; i<numiter; i++)
      {
      m_Warper->SetInput(this->GetOutput());
      m_Warper->SetDeformationField(this->GetOutput());

      // Remember we chose to use an inplace adder
      m_Adder->SetInput1(this->GetOutput());
      m_Adder->SetInput2(m_Warper->GetOutput());
      m_Adder->GraftOutput(this->GetOutput()); //necessary?
      //std::cout<<"inplace: "<<m_Adder->GetInPlace()<<std::endl;
      m_Adder->Update();

      // Region passing stuff
      this->GraftOutput( m_Adder->GetOutput() );

      progress.CompletedPixel();
      }
}


} // end namespace itk

#endif
