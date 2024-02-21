/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DResample_txx
#define itkDiffusionTensor3DResample_txx

#include "itkDiffusionTensor3DResample.h"

namespace itk
{

template <class TInput, class TOutput>
DiffusionTensor3DResample<TInput, TOutput>
::DiffusionTensor3DResample()
{
  this->SetNumberOfRequiredInputs( 1 );
  m_OutputSpacing.Fill( 1.0 );
  m_OutputOrigin.Fill( 0.0 );
  m_OutputDirection.SetIdentity();
  m_OutputSize.Fill( 0 );
  m_DefaultPixelValue = static_cast<OutputDataType>( ITK_DIFFUSION_TENSOR_3D_ZERO );
}

template <class TInput, class TOutput>
ModifiedTimeType
DiffusionTensor3DResample<TInput, TOutput>
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
  if( m_Interpolator.IsNotNull() )
  {
    if( latestTime < m_Interpolator->GetMTime() )
    {
      latestTime = m_Interpolator->GetMTime();
    }
  }
  return latestTime;
}

template <class TInput, class TOutput>
void
DiffusionTensor3DResample<TInput, TOutput>
::BeforeThreadedGenerateData()
{
  if( m_Interpolator.IsNull() )
  {
    itkExceptionMacro( << "Interpolator not set" );
  }
  if( m_Transform.IsNull() )
  {
    itkExceptionMacro( << "Transform not set" );
  }
  // m_Interpolator->SetNumberOfThreads( this->GetNumberOfThreads() ) ;
  m_Interpolator->SetInputImage( const_cast<InputImageType *>
                                 ( this->GetInput() )  );
  // m_Interpolator->SetDefaultPixelValue( m_DefaultPixelValue ) ;
  m_DefaultTensor.SetIdentity();
  m_DefaultTensor *= this->m_DefaultPixelValue;
/*  for( unsigned int i = 0 ; i < 3 ; i++ )
    {
    m_DefaultTensor( i , i ) *= this->m_DefaultPixelValue ;
    }*/
}

template <class TInput, class TOutput>
void
DiffusionTensor3DResample<TInput, TOutput>
::DynamicThreadedGenerateData( const OutputImageRegionType &outputRegionForThread)
{
  OutputImageType*       outputImagePtr = this->GetOutput( 0 );
  IteratorType           it( outputImagePtr, outputRegionForThread );
  InputTensorDataType    inputTensor;
  OutputTensorDataType   outputTensor;

  typename OutputImageType::IndexType index;
  Point<double, 3> point;
  for( it.GoToBegin(); !it.IsAtEnd(); ++it )
  {
    index = it.GetIndex();
    outputImagePtr->TransformIndexToPhysicalPoint( index, point );
    const Point<double, 3> pointTransform
      = m_Transform->EvaluateTensorPosition( point );
    if( m_Interpolator->IsInsideBuffer( pointTransform ) )
    {
      inputTensor = m_Interpolator->Evaluate( pointTransform );
      outputTensor = m_Transform->EvaluateTransformedTensor( inputTensor,
                                                             point );
      it.Set( OutputTensorDataType( outputTensor ) );
    }
    else
    {
      it.Set( m_DefaultTensor );
    }
  }
}

template <class TInput, class TOutput>
void
DiffusionTensor3DResample<TInput, TOutput>
::SetOutputParametersFromImage( InputImagePointerType Image )
{
  typename OutputImageType::RegionType region;
  region = Image->GetLargestPossibleRegion();
  m_OutputSize = region.GetSize();
  m_OutputSpacing = Image->GetSpacing();
  m_OutputDirection = Image->GetDirection();
  m_OutputOrigin = Image->GetOrigin();
}

/**
 * Inform pipeline of required output region
 */
template <class TInput, class TOutput>
void
DiffusionTensor3DResample<TInput, TOutput>
::GenerateOutputInformation()
{
  // call the superclass' implementation of this method
  Superclass::GenerateOutputInformation();
  // get pointers to the input and output
  OutputImagePointerType outputPtr = this->GetOutput( 0 );
  if( !outputPtr )
  {
    return;
  }
  outputPtr->SetSpacing( m_OutputSpacing );
  outputPtr->SetOrigin( m_OutputOrigin );
  outputPtr->SetDirection( m_OutputDirection );
  // Set the size of the output region
  typename OutputImageType::RegionType outputLargestPossibleRegion;
  outputLargestPossibleRegion.SetSize( m_OutputSize );
  typename OutputImageType::IndexType index;
  for( int i = 0; i < 3; i++ )
  {
    index[i] = 0;
  }
  outputLargestPossibleRegion.SetIndex( index );
  outputPtr->SetRegions( outputLargestPossibleRegion );
  return;
}

/**
 * Set up state of filter after multi-threading.
 */
template <class TInput, class TOutput>
void
DiffusionTensor3DResample<TInput, TOutput>
::AfterThreadedGenerateData()
{
  m_Interpolator->SetInputImage( nullptr );
}

/**
 * Inform pipeline of necessary input image region
 *
 * Determining the actual input region is non-trivial, especially
 * when we cannot assume anything about the transform being used.
 * So we do the easy thing and request the entire input image.
 */
template <class TInput, class TOutput>
void
DiffusionTensor3DResample<TInput, TOutput>
::GenerateInputRequestedRegion()
{
  // call the superclass's implementation of this method
  Superclass::GenerateInputRequestedRegion();

  if( !this->GetInput() )
  {
    return;
  }
  // get pointers to the input and output
  InputImagePointerType inputPtr  =
    const_cast<InputImageType *>( this->GetInput() );

  // Request the entire input image
  typename InputImageType::RegionType inputRegion;
  inputRegion = inputPtr->GetLargestPossibleRegion();
  inputPtr->SetRequestedRegion( inputRegion );
  return;
}

template <class TInput, class TOutput>
Matrix<double, 3, 3>
DiffusionTensor3DResample<TInput, TOutput>
::GetOutputMeasurementFrame()
{
  Matrix<double, 3, 3> mat;
  mat.SetIdentity();
  return mat;
}

} // end namespace itk
#endif
