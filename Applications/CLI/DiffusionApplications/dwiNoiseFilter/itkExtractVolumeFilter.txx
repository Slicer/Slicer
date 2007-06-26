/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkExtractVolumeFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006/01/11 19:43:31 $
  Version:   $Revision: 1.21 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkExtractVolumeFilter_txx
#define _itkExtractVolumeFilter_txx
#include "itkExtractVolumeFilter.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace itk
{


template <class TInputImage, class TOutputImage>
void
ExtractVolumeFilter<TInputImage,TOutputImage>
::GenerateInputRequestedRegion()
{
  
  Superclass::GenerateInputRequestedRegion();

  // get pointer to the input

  typename Superclass::InputImagePointer  inputPtr  = 
    const_cast< TInputImage * >( this->GetInput());

  if ( !inputPtr )
    {
      return;
    }


  const TOutputImage * outputPtr = this->GetOutput();
  if ( !outputPtr )
    {
      return;
    }

  OutputImageRegionType outputRegion;
  
  OutputImageSizeType output_sz =
    outputPtr->GetLargestPossibleRegion().GetSize();

  OutputImageIndexType output_idx =
    outputPtr->GetLargestPossibleRegion().GetIndex();  

  // compute the requested region

  InputImageRegionType inputRegion;
  
  InputImageSizeType input_sz = output_sz;
  InputImageIndexType input_idx =output_idx;
  
  inputRegion.SetSize( input_sz );
  inputRegion.SetIndex( input_idx );
  inputPtr->SetRequestedRegion( inputRegion );

}

template <class TInputImage, class TOutputImage>
void
ExtractVolumeFilter<TInputImage, TOutputImage>
::GenerateOutputInformation()
{

  Superclass::GenerateOutputInformation();

  const TInputImage * inputPtr = this->GetInput();
  if( !inputPtr )
    {
    return;
    }

  // Compute the new region size.
  OutputImageRegionType outputRegion;
  OutputImageIndexType   idx;

  InputImageSizeType input_sz =
    inputPtr->GetLargestPossibleRegion().GetSize();

  InputImageIndexType input_idx =
    inputPtr->GetLargestPossibleRegion().GetIndex();

  // output will have dimensions that are multiplied by the I and the J tilings

  OutputImageSizeType output_sz = input_sz;

  idx = input_idx;

  outputRegion.SetSize( output_sz );
  outputRegion.SetIndex( idx );

  this->GetOutput()->SetRegions( outputRegion );
  this->GetOutput()->SetLargestPossibleRegion( outputRegion );
  this->GetOutput()->Allocate();

}

template< class TInputImage, class TOutputImage>
void
ExtractVolumeFilter< TInputImage, TOutputImage >
::GenerateData()
{
  typedef itk::ImageRegionConstIterator< TInputImage > ConstIteratorType;
  typedef itk::ImageRegionIterator< TOutputImage > IteratorType;

  // Get the input and output
  typename OutputImageType::Pointer       output = this->GetOutput();
  typename  InputImageType::ConstPointer  input  = this->GetInput();

  ConstIteratorType nit( input, input->GetLargestPossibleRegion() );
  IteratorType it( output, output->GetLargestPossibleRegion() );

  long int iCount = 0;

  for ( nit.GoToBegin(), it.GoToBegin(); !nit.IsAtEnd() && !it.IsAtEnd(); ++nit, ++it ) {

    OutputPixelType dVal = static_cast<OutputPixelType>(dFactor*((nit.Get())[iVolumeNr]));

    it.Set( dVal );

  }

}

/**
 * Standard "PrintSelf" method
 */
template< class TInputImage, class TOutputImage>
void
ExtractVolumeFilter< TInputImage, TOutputImage >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os, indent );

}

} // end namespace itk


#endif
