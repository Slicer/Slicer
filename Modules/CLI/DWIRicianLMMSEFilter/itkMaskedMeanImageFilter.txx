/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMaskedMeanImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006/01/11 19:43:31 $
  Version:   $Revision: 1.14 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkMaskedMeanImageFilter_txx
#define _itkMaskedMeanImageFilter_txx
#include "itkMaskedMeanImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

namespace itk
{

template <class TInputImage, class TOutputImage>
MaskedMeanImageFilter<TInputImage, TOutputImage>
::MaskedMeanImageFilter()
{
  m_Radius.Fill(1);
  m_MinimumNumberOfUsedVoxels = 1;
}

template <class TInputImage, class TOutputImage>
void  MaskedMeanImageFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion()
throw (InvalidRequestedRegionError)
{
  // Call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  // Get pointers to the input and output
  InputImagePointer  inputPtr  = const_cast<TInputImage *>( this->GetInput() );
  OutputImagePointer outputPtr = this->GetOutput();
  if( !inputPtr || !outputPtr )
    {
    return;
    }
  // Get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();
  // Pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_Radius );

  // Crop the input requested region at the input's largest possible region
  inputRequestedRegion.Crop( inputPtr->GetLargestPossibleRegion() );
  inputPtr->SetRequestedRegion( inputRequestedRegion );
  return;
}

template <class TInputImage, class TOutputImage>
void MaskedMeanImageFilter<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId )
{

  unsigned int i;

  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;

  ConstNeighborhoodIterator<InputImageType> bit;
  ImageRegionIterator<OutputImageType>      it;

  // Allocate output
  typename OutputImageType::Pointer output = this->GetOutput();
  typename  InputImageType::ConstPointer input  = this->GetInput();

  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC(input, outputRegionForThread, m_Radius);

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;

  // support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels() );

  InputRealType sum;
  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  for( fit = faceList.begin(); fit != faceList.end(); ++fit )
    {
    bit = ConstNeighborhoodIterator<InputImageType>(m_Radius,
                                                    input, *fit);
    unsigned int neighborhoodSize = bit.Size();
    it = ImageRegionIterator<OutputImageType>(output, *fit);
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();
    for( bit.GoToBegin(), it.GoToBegin(); !bit.IsAtEnd(); ++bit, ++it )
      {
      int iNumberOfUsedVoxels = 0;
      sum = NumericTraits<InputRealType>::ZeroValue();
      for( i = 0; i < neighborhoodSize; ++i )
        {
        if( bit.GetPixel(i) > 0 )
          {
          iNumberOfUsedVoxels++;
          sum += static_cast<InputRealType>( bit.GetPixel(i) );
          }
        }
      // Get the mean value
      if( iNumberOfUsedVoxels >= m_MinimumNumberOfUsedVoxels && ( bit.GetCenterPixel() > 0 ) )
        {
        it.Set( static_cast<OutputPixelType>(sum / double(iNumberOfUsedVoxels) ) );
        }
      else
        {
        it.Set(   static_cast<OutputPixelType>( -128 )   );
        }
      progress.CompletedPixel();
      }
    }
}

/** Standard "PrintSelf" method */
template <class TInputImage, class TOutput>
void MaskedMeanImageFilter<TInputImage, TOutput>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius: " << m_Radius << std::endl;
}

} // end namespace itk

#endif
