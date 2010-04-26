#ifndef __itkVelocityFieldLieBracketFilter_txx
#define __itkVelocityFieldLieBracketFilter_txx
#include "itkVelocityFieldLieBracketFilter.h"

#include <itkImageRegionIterator.h>
#include <itkProgressReporter.h>

namespace itk
{

/**
 * Default constructor.
 */
template <class TInputImage, class TOutputImage>
VelocityFieldLieBracketFilter<TInputImage,TOutputImage>
::VelocityFieldLieBracketFilter()
{
  // Setup the number of required inputs
  this->SetNumberOfRequiredInputs( 2 );  

  m_RightGradientCalculator = InputFieldGradientCalculatorType::New();
  m_LeftGradientCalculator  = InputFieldGradientCalculatorType::New();

}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
VelocityFieldLieBracketFilter<TInputImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Right gradient calculator" << m_RightGradientCalculator << std::endl;
  os << indent << "Left gradient calculator" << m_LeftGradientCalculator << std::endl;
}


template <class TInputImage, class TOutputImage>
void
VelocityFieldLieBracketFilter<TInputImage,TOutputImage>
::GenerateInputRequestedRegion() throw (InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // get pointers to the input and output
  InputFieldPointer inputPtr0 = const_cast< InputFieldType * >( this->GetInput(0) );
  InputFieldPointer inputPtr1 = const_cast< InputFieldType * >( this->GetInput(1) );
  OutputFieldPointer outputPtr = this->GetOutput();
  
  if ( !inputPtr0 || !inputPtr1 || !outputPtr )
    {
    return;
    }

  // The kernel size is one by default
  //\todo find a way to get the radius from the gradient calculator
  const unsigned long radius = 1;
  
  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion0
     = inputPtr0->GetRequestedRegion();
  typename TInputImage::RegionType inputRequestedRegion1
     = inputPtr1->GetRequestedRegion();

  // pad the input requested region by the operator radius
  inputRequestedRegion0.PadByRadius( radius );
  inputRequestedRegion1.PadByRadius( radius );

  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion0.Crop(inputPtr0->GetLargestPossibleRegion()) )
    {
    inputPtr0->SetRequestedRegion( inputRequestedRegion0 );
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr0->SetRequestedRegion( inputRequestedRegion0 );
    
    // build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr0);
    throw e;
    }

  
  if ( inputRequestedRegion1.Crop(inputPtr1->GetLargestPossibleRegion()) )
    {
    inputPtr1->SetRequestedRegion( inputRequestedRegion1 );
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr1->SetRequestedRegion( inputRequestedRegion1 );
    
    // build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr1);
    throw e;
    }
}


template <class TInputImage, class TOutputImage>
void 
VelocityFieldLieBracketFilter<TInputImage,TOutputImage>
::BeforeThreadedGenerateData()
{
  // Initialize gradient calculators
  m_LeftGradientCalculator->SetInputImage( this->GetInput(0) );
  m_RightGradientCalculator->SetInputImage( this->GetInput(1) );
}


/** 
 * GenerateData()
 */
template <class TInputImage, class TOutputImage>
void 
VelocityFieldLieBracketFilter<TInputImage,TOutputImage>
::ThreadedGenerateData( const OutputFieldRegionType &outputRegionForThread,
                        int threadId)
{
  // Get the input and output pointers
  InputFieldConstPointer leftField = this->GetInput(0);
  InputFieldConstPointer rightField = this->GetInput(1);
  OutputFieldPointer outputPtr = this->GetOutput();
  
  // Progress tracking
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // Input and output iterators/
  typedef ImageRegionConstIterator<InputFieldType>  InputFieldIteratorType;
  typedef ImageRegionIterator<OutputFieldType>      OutputFieldIteratorType; 
  InputFieldIteratorType  leftIter(   leftField,  outputRegionForThread );
  InputFieldIteratorType  rightIter(  rightField, outputRegionForThread );
  OutputFieldIteratorType outputIter( outputPtr,  outputRegionForThread );
  
  InputFieldGradientType leftgrad, rightgrad;

  while ( ! leftIter.IsAtEnd() )
    {
    leftgrad = m_LeftGradientCalculator->EvaluateAtIndex(leftIter.GetIndex());
    rightgrad = m_RightGradientCalculator->EvaluateAtIndex(rightIter.GetIndex());
    
    const InputFieldPixelType & leftval = leftIter.Value();
    const InputFieldPixelType & rightval = rightIter.Value();
    OutputFieldPixelType & outVal = outputIter.Value();
    
    for (unsigned int d=0; d<InputFieldDimension; d++)
      {
      outVal[d] = ( leftgrad(d,0)*rightval[0]
               - rightgrad(d,0)*leftval[0] );
      for (unsigned int dd=1; dd<InputFieldDimension; dd++)
        {
        outVal[d] += ( leftgrad(d,dd)*rightval[dd]
                  - rightgrad(d,dd)*leftval[dd] );
        }
      }
    
    ++leftIter;
    ++rightIter;
    ++outputIter;
    progress.CompletedPixel(); // potential exception thrown here
    }
}
  

} // end namespace itk


#endif
