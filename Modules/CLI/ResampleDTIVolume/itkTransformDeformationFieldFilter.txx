#ifndef itkTransformDeformationFieldFilter_txx
#define itkTransformDeformationFieldFilter_txx

#include "itkTransformDeformationFieldFilter.h"

namespace itk
{

template <class TInput, class TOutput, int NDimensions>
TransformDeformationFieldFilter<TInput, TOutput, NDimensions>
::TransformDeformationFieldFilter()
{
  this->SetNumberOfRequiredInputs( 1 );
}

template <class TInput, class TOutput, int NDimensions>
ModifiedTimeType
TransformDeformationFieldFilter<TInput, TOutput, NDimensions>
::GetMTime() const
{
  unsigned long latestTime = Object::GetMTime();

  if( m_Transform.IsNotNull() )
    {
    if( latestTime < m_Transform->GetMTime() )
      {
      latestTime = m_Transform->GetMTime();
      }
    }
  return latestTime;
}

template <class TInput, class TOutput, int NDimensions>
void
TransformDeformationFieldFilter<TInput, TOutput, NDimensions>
::BeforeThreadedGenerateData()
{
  if( m_Transform.IsNull() )
    {
    itkExceptionMacro( << "Transform not set" );
    }
  if( !this->GetInput( 0 ) )
    {
    itkExceptionMacro( << "Input deformation field not set" );
    }
}

template <class TInput, class TOutput, int NDimensions>
void
TransformDeformationFieldFilter<TInput, TOutput, NDimensions>
::DynamicThreadedGenerateData( const OutputImageRegionType &outputRegionForThread)
  {
  OutputDeformationFieldPointerType outputImagePtr = this->GetOutput( 0 );
  InputIteratorType                 it( this->GetInput( 0 ), outputRegionForThread );
  OutputIteratorType                out( outputImagePtr, outputRegionForThread );

  itk::Index<NDimensions>         index;
  itk::Point<double, NDimensions> point;
  itk::Point<double, NDimensions> tempPoint;
  itk::Point<double, NDimensions> outputPoint;
  for( it.GoToBegin(), out.GoToBegin(); !it.IsAtEnd(); ++it, ++out )
    {
    index = it.GetIndex();
    InputDeformationPixelType vector = it.Get();
    this->GetInput( 0 )->TransformIndexToPhysicalPoint( index, point );
    for( int i = 0; i < NDimensions; i++ )
      {
      tempPoint[i] = point[i] + static_cast<double>( vector[i] );
      }
    outputPoint = m_Transform->TransformPoint( tempPoint );
    OutputDeformationPixelType outputVector;
    for( int i = 0; i < NDimensions; i++ )
      {
      outputVector[i] = static_cast<OutputDataType>( outputPoint[i] - point[i] );
      }
    out.Set( outputVector );
    }
  }

/**
 * Inform pipeline of required output region
 */
template <class TInput, class TOutput, int NDimensions>
void
TransformDeformationFieldFilter<TInput, TOutput, NDimensions>
::GenerateOutputInformation()
{
  // call the superclass' implementation of this method
  Superclass::GenerateOutputInformation();
  // get pointers to the input and output
  OutputDeformationFieldPointerType outputPtr = this->GetOutput( 0 );
  if( !outputPtr )
    {
    return;
    }
  outputPtr->SetSpacing( this->GetInput( 0 )->GetSpacing() );
  outputPtr->SetOrigin( this->GetInput( 0 )->GetOrigin() );
  outputPtr->SetDirection( this->GetInput( 0 )->GetDirection() );
  // Set the size of the output region
  outputPtr->SetRegions( this->GetInput( 0 )->GetLargestPossibleRegion() );
  return;
}

/**
 * Inform pipeline of necessary input image region
 *
 * Determining the actual input region is non-trivial, especially
 * when we cannot assume anything about the transform being used.
 * So we do the easy thing and request the entire input image.
 */
template <class TInput, class TOutput, int NDimensions>
void
TransformDeformationFieldFilter<TInput, TOutput, NDimensions>
::GenerateInputRequestedRegion()
{
  // call the superclass's implementation of this method
  Superclass::GenerateInputRequestedRegion();

  if( !this->GetInput() )
    {
    return;
    }
  // get pointers to the input and output
  InputDeformationFieldPointerType inputPtr  =
    const_cast<InputDeformationFieldType *>( this->GetInput() );

  // Request the entire input image
  typename InputDeformationFieldType::RegionType inputRegion;
  inputRegion = inputPtr->GetLargestPossibleRegion();
  inputPtr->SetRequestedRegion( inputRegion );
  return;
}

} // end namespace itk
#endif
