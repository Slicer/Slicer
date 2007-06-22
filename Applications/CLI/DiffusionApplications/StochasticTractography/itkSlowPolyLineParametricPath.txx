#ifndef _itkSlowPolyLineParametricPath_txx
#define _itkSlowPolyLineParametricPath_txx

#include "itkSlowPolyLineParametricPath.h"
#include <math.h>



namespace itk
{

//template<unsigned int VDimension>
//typename SlowPolyLineParametricPath<VDimension>::VectorType
//SlowPolyLineParametricPath<VDimension>
//::EvaluateDerivative(const InputType & input) const
//{
//}



/**
 * Constructor
 */
template <unsigned int VDimension>
SlowPolyLineParametricPath<VDimension>
::SlowPolyLineParametricPath()
{
  this->SetDefaultInputStepSize( 0.3 );
}


template<unsigned int VDimension>
typename SlowPolyLineParametricPath<VDimension>::OffsetType
SlowPolyLineParametricPath<VDimension>
::IncrementInput(InputType & input) const
{
  int         iterationCount;
  bool        tooSmall;
  bool        tooBig;
  InputType   inputStepSize;
  InputType   finalInputValue;
  OffsetType  offset;
  IndexType   currentImageIndex;
  IndexType   nextImageIndex;
  IndexType   finalImageIndex;
  
  iterationCount    = 0;
  inputStepSize     = this->GetDefaultInputStepSize();

  // Are we already at (or past) the end of the input?
  finalInputValue   = this->EndOfInput();
  currentImageIndex = this->EvaluateToIndex( input );
  finalImageIndex   = this->EvaluateToIndex( finalInputValue );
  offset            = finalImageIndex - currentImageIndex;
  if(  ( offset == this->GetZeroOffset() && input != this->StartOfInput() )  ||
       ( input >=finalInputValue )  )
    {
    return this->GetZeroOffset();
    }
  
  do
    {
    if( iterationCount++ > 10000 ) {itkExceptionMacro(<<"Too many iterations");}
    
    nextImageIndex    = this->EvaluateToIndex( input + inputStepSize );
    offset            = nextImageIndex - currentImageIndex;
    
    tooBig = false;
    tooSmall = ( offset == this->GetZeroOffset() );
    if( tooSmall )
      {
      // increase the input step size, but don't go past the end of the input
      inputStepSize *= 2;
      if(  (input + inputStepSize) >= finalInputValue  ){
        //inputStepSize = finalInputValue - input;
        inputStepSize += this->GetDefaultInputStepSize();
      }
    }
    else
      {
      // Search for an offset dimension that is too big
      for( unsigned int i=0; i<VDimension && !tooBig; i++ )
        {
        tooBig = ( offset[i] >= 2 || offset[i] <= -2 );
        }
      
      if( tooBig ){
        //inputStepSize /= 1.5;
        inputStepSize -= (this->GetDefaultInputStepSize()/0.5);
      }
    }
  }
  while( tooSmall || tooBig );
  
  input += inputStepSize;
  return offset;
}


/**
 * Standard "PrintSelf" method
 */
template <unsigned int VDimension>
void
SlowPolyLineParametricPath<VDimension>
::PrintSelf( std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}



} // end namespaceitk

#endif
