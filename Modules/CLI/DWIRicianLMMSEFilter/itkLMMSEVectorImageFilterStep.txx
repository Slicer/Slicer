/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkLMMSEVectorImageFilterStep.txx,v $
  Language:  C++
  Date:      $Date: 2005/05/4 14:28:51 $
  Version:   $Revision: 1.1
=========================================================================*/
#ifndef _itkLMMSEVectorImageFilterStep_txx
#define _itkLMMSEVectorImageFilterStep_txx
#include "itkLMMSEVectorImageFilterStep.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "vnl/vnl_math.h"

namespace itk
{

/** Constructor */
template <class TInputImage, class TOutputImage>
LMMSEVectorImageFilterStep<TInputImage, TOutputImage>::LMMSEVectorImageFilterStep()
{
  m_Radius.Fill(1);
  m_Channels = 1;
  m_MinimumNumberOfUsedVoxelsFiltering = 1;
  m_UseAbsoluteValue = false;
  m_KeepValue = false;
  m_NoiseVariance = 1.0f;
}

/** The requested input region is larger than the corresponding output, so we need to override this method: */
template <class TInputImage, class TOutputImage>
void LMMSEVectorImageFilterStep<TInputImage, TOutputImage>
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
  InputImageRegionType inputRequestedRegion = inputPtr->GetRequestedRegion();

  // Pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_Radius );

  // Crop the input requested region at the input's largest possible region
  inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion() );
  inputPtr->SetRequestedRegion( inputRequestedRegion );
  return;
}

template <class TInputImage, class TOutputImage>
void LMMSEVectorImageFilterStep<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType itkNotUsed(threadId) )
{
  // Boundary conditions for this filter; Neumann conditions are fine
  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
  // Iterators:
  ConstNeighborhoodIterator<InputImageType> bit;  // Iterator for the input image
  ImageRegionIterator<OutputImageType>      it;   // Iterator for the output image
  // Input and output
  InputImageConstPointer input   =  this->GetInput();
  OutputImagePointer     output  =  this->GetOutput();
  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType           faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC( input, outputRegionForThread, m_Radius );
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;
  // Auxilair variables to compute statistics for each DWI channel:
  double*       sum                   = new double[m_Channels];
  double*       dSecondAveragedMoment = new double[m_Channels];
  double*       dFourthAveragedMoment = new double[m_Channels];
  double*       dSquaredMagnitude     = new double[m_Channels];
  unsigned int* iNumberOfUsedVoxels   = new unsigned int[m_Channels];
  for( fit = faceList.begin(); fit != faceList.end(); ++fit )  // Iterate through facets
    { // Iterators:
    bit = ConstNeighborhoodIterator<InputImageType>(  m_Radius, input, *fit  );
    it  = ImageRegionIterator<OutputImageType>(        output,     *fit      );
    unsigned int neighborhoodSize = bit.Size();
    // Boundary condition:
    bit.OverrideBoundaryCondition(&nbc);
    for( bit.GoToBegin(), it.GoToBegin(); !bit.IsAtEnd(); ++bit, ++it )    // Iterate through pixels in the current
                                                                           // facet
      { // For the central voxel:
      OutputPixelType dMagnitude = bit.GetCenterPixel();
      for( unsigned int iJ = 0; iJ < m_Channels; iJ++ )   // For each channel
        {
        dSquaredMagnitude[iJ] = dMagnitude[iJ] * dMagnitude[iJ];
        dSecondAveragedMoment[iJ] = 0;
        dFourthAveragedMoment[iJ] = 0;
        iNumberOfUsedVoxels[iJ] = 0;
        sum[iJ] = 0;
        }
      for( unsigned int i = 0; i < neighborhoodSize; ++i )   // For each voxel in the neighbourhood
        {
        OutputPixelType currentPixelValue = bit.GetPixel( i );
        for( unsigned int iJ = 0; iJ < m_Channels; ++iJ )  // For each channel
          {
          if( currentPixelValue[iJ] > 0 )  // exactly zero indicates an artifical value filled in by the scanner, maybe
                                           // make a flag for this test
            {
            iNumberOfUsedVoxels[iJ]++;
            double dMagnitudeSquared  = currentPixelValue[iJ] * currentPixelValue[iJ];
            dSecondAveragedMoment[iJ] += dMagnitudeSquared;
            dFourthAveragedMoment[iJ] += dMagnitudeSquared * dMagnitudeSquared;
            }
          }
        }
      // Filter the pixel:
      OutputPixelType dFiltered = dMagnitude;           // if use VariableLengthVector as pixel type, operator = fixes
                                                        // the size
      for( unsigned int iJ = 0; iJ < m_Channels; ++iJ ) // For each DWI channel
        { // The current voxel is not processed if the corrresponding voxel in the input image was negative,
         // or if the number of voxels used to compute statistics is not large enough to ensure a reasonable
         // behaviour of sample statistics:
        if( iNumberOfUsedVoxels[iJ] >= m_MinimumNumberOfUsedVoxelsFiltering && dMagnitude[iJ] > 0 )
          {
          double norm = 1.0f / iNumberOfUsedVoxels[iJ]; // Auxiliar value; the cost of a division is far larger than
                                                        // that for a product
          dSecondAveragedMoment[iJ] *= norm;
          dFourthAveragedMoment[iJ] *= norm;
          double dDenominator =
            ( dFourthAveragedMoment[iJ] - dSecondAveragedMoment[iJ] * dSecondAveragedMoment[iJ] );
          const double dAbsFactor = 1;
          if( vnl_math_abs(dDenominator) > dAbsFactor * std::numeric_limits<double>::epsilon() )   // Check numeric
                                                                                                   // precission
            {
            double dGain = 1 - (4 * m_NoiseVariance * (dSecondAveragedMoment[iJ] - m_NoiseVariance) ) / dDenominator;
            if( dGain < 0 )
              {
              dGain = 0.0;
              }
            double dFilteredSquared = dSecondAveragedMoment[iJ] - 2 * m_NoiseVariance + dGain
              * (dSquaredMagnitude[iJ] - dSecondAveragedMoment[iJ]);
            if( dFilteredSquared >= 0 )
              {
              dFiltered[iJ] = sqrt( dFilteredSquared );
              }
            else
              {
              if( m_UseAbsoluteValue )
                {
                dFiltered[iJ] = sqrt( -dFilteredSquared );
                }
              else if( m_KeepValue )
                {
                dFiltered[iJ] = dMagnitude[iJ];
                }
              else
                {
                dFiltered[iJ] = 0;
                }
              }
            }
          else   // Denominator module is too small!
            {
            dFiltered[iJ] = dMagnitude[iJ];
            }
          }
        else   // This situation is likely to occur at background voxels
          {
          dFiltered[iJ] = dMagnitude[iJ];
          }
        }
      // Put the output in place:
      it.Set( dFiltered );
      }
    }
  // Delete previously alloctaed memory:
  delete [] sum;
  delete [] dSecondAveragedMoment;
  delete [] dFourthAveragedMoment;
  delete [] dSquaredMagnitude;
  delete [] iNumberOfUsedVoxels;
}

/** Standard "PrintSelf" method */
template <class TInputImage, class TOutput>
void LMMSEVectorImageFilterStep<TInputImage, TOutput>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius: "                             << m_Radius                             << std::endl;
  os << indent << "Channels: "                           << m_Channels                           << std::endl;
  os << indent << "UseAbsoluteValue: "                   << m_UseAbsoluteValue                   << std::endl;
  os << indent << "KeepValue: "                          << m_KeepValue                          << std::endl;
  os << indent << "NoiseVariance: "                      << m_NoiseVariance                      << std::endl;
  os << indent << "MinimumNumberOfUsedVoxelsFiltering: " << m_MinimumNumberOfUsedVoxelsFiltering << std::endl;
}

} // end namespace itk

#endif
