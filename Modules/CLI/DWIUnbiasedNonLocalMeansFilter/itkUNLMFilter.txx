/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkUNLMFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006/01/11 19:43:31 $
  Version:   $Revision: 1.21 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkUNLMFilter_txx
#define _itkUNLMFilter_txx
#include "itkUNLMFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "math.h"

namespace itk
{
template <class TInputImage, class TOutputImage>
UNLMFilter<TInputImage, TOutputImage>
::UNLMFilter()
{
  m_NDWI          = 0;
  m_NBaselines    = 0;
  m_DWI           = IndicatorType( 0 );
  m_Baselines     = IndicatorType( 0 );
  m_Sigma         = 0.0f; // By default, we implement gaussian behaviour
  m_GradientList  = GradientListType(0);
  m_Neighbours    = 1;    // By default, we use the gradient by gradient behaviour
  m_NeighboursInd = NeighboursIndType(0, 0);
  m_H             = 1.0f;
  m_RSearch.Fill(3);
  m_RComp.Fill(1);
}

template <class TInputImage, class TOutputImage>
void UNLMFilter<TInputImage, TOutputImage>
::BeforeThreadedGenerateData( void )
{
  if( m_Neighbours > m_NDWI )
    {
    m_Neighbours = 5;
    }
  // Find the closest neighbours to each gradient direction
  if( m_NDWI != m_DWI.GetSize() || m_NBaselines != m_Baselines.GetSize() ||
      (m_NDWI < 1 && m_NBaselines < 1) || m_GradientList.size() != m_NDWI || m_Neighbours < 1 || m_Neighbours > m_NDWI )
    {
    itkExceptionMacro( << "Bad iniialisation of the filter!!! Check parameters, please" );
    }
  m_NeighboursInd = NeighboursIndType( m_NDWI, m_Neighbours );

  // Vectors to compute the distance from each gradient direction to each other gradient direction; we need to sort to
  // find the closest
  // gradient directions to each of one.
  std::vector<OrderType> distances;
  OrderType              orderElement;
  for( unsigned int g = 0; g < m_NDWI; ++g )           // For each gradient direction
    {
    distances.clear();
    for( unsigned int k = 0; k < m_NDWI; ++k )       // Compare to each other gradient direction
      {
      orderElement[0] = (double)k;
      orderElement[1] = itk::NumericTraits<double>::ZeroValue();
      for( unsigned int d = 0; d < TInputImage::ImageDimension; ++d ) // Sum of squared differences (euclidean norm)
        {
        orderElement[1] += ( m_GradientList[g][d] * m_GradientList[k][d] );
        }
      if( orderElement[1] < -1.0f || orderElement[1] > 1.0f )
        {
        orderElement[1] = 0.0f;
        }
      else
        {
        orderElement[1] = ::acos( orderElement[1] );
        }
      if( 3.141592654f - orderElement[1] < orderElement[1] )
        {
        orderElement[1] = 3.141592654f - orderElement[1];
        }
      distances.push_back( orderElement );
      }
    std::sort( distances.begin(), distances.end(), UNLM_gradientDistance_smaller );
    for( unsigned int k = 0; k < m_Neighbours; ++k )
      {
      m_NeighboursInd[g][k] = m_DWI[(unsigned int)(distances[k][0])];
      }
    }
  return;
}

template <class TInputImage, class TOutputImage>
void UNLMFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion()
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
  InputImageSizeType radius;
  radius = m_RSearch + m_RComp;
  inputRequestedRegion.PadByRadius( radius );

  // Crop the input requested region at the input's largest possible region
  inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion() );
  inputPtr->SetRequestedRegion( inputRequestedRegion );
  return;
}

template <class TInputImage, class TOutputImage>
void UNLMFilter<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread,
                        ThreadIdType itkNotUsed(threadId) )
{
  // Boundary conditions for this filter; Neumann conditions are fine
  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
  // Iterators:
  ConstNeighborhoodIterator<InputImageType> bit;         // Iterator for the input image
  ImageRegionIterator<OutputImageType>      it;          // Iterator for the output image
  // Input and output
  InputImageConstPointer input   =  this->GetInput();
  OutputImagePointer     output  =  this->GetOutput();
  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType           faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;

  faceList = bC( input, outputRegionForThread, m_RComp );
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;
  // Region to search and iterator:
  ConstNeighborhoodIterator<InputImageType> search;
  InputImageRegionType                      searchRegion;
  InputImageSizeType                        searchSize, baseSearchSize;
  for( unsigned int d = 0; d < TInputImage::ImageDimension; ++d )
    {
    baseSearchSize[d] = (2 * m_RSearch[d] + 1);
    }

  unsigned int numNeighbours    = 1;
  for( unsigned int d = 0; d < TInputImage::ImageDimension; ++d ) // The number of voxels which are going to be
                                                                  // accounted in the WA
    {
    numNeighbours    *= baseSearchSize[d];
    }
  // Generate the Gaussian window (std=1)
  // Auxiliar iterator:
  ConstNeighborhoodIterator<InputImageType> test = ConstNeighborhoodIterator<InputImageType>(  m_RComp, input,
                                                                                               outputRegionForThread  );
  unsigned int neighborhoodSize = test.Size();
  float*       gw = new float[neighborhoodSize];      // The window itself
  float        sum = itk::NumericTraits<float>::ZeroValue(); // To normalize the window to sum to 1
  for( unsigned int k = 0; k < neighborhoodSize; ++k )
    {
    if( k != neighborhoodSize / 2 )   // Not the center of the neighbourhhod
      {
      typename ConstNeighborhoodIterator<InputImageType>::OffsetType idx = test.GetOffset(k);
      gw[k] = itk::NumericTraits<float>::ZeroValue();
      for( unsigned int d = 0; d < InputImageType::ImageDimension; ++d )
        {
        gw[k] += static_cast<float>( idx[d] * idx[d] );
        }
      gw[k] = ::exp( -gw[k] / 2 ); // sigma=1
      }
    else   // In the center of the neighbourhood, we correct the weight to avoid over-weighting
      {
      if( neighborhoodSize > 0 )
        {
        gw[k] = gw[k - 1]; // The previous value is the one corresponding to the closest pixel to the center
        }
      else
        {
        gw[k] = 1;
        }
      }
    sum += gw[k];
    }
  // Normalize the Gaussian kernel:
  for( unsigned int k = 0; k < neighborhoodSize; ++k )
    {
    gw[k] /= sum;
    }
  // Auxiliar variables:
  float  norm;
  float  max;
  float  sqh = 1.0f / (m_H * m_H);
  float* distB = new float[numNeighbours];
  float* distD = new float[numNeighbours * m_Neighbours];
  float* valsB = new float[numNeighbours];
  float* valsD = new float[numNeighbours * m_Neighbours];
  for( fit = faceList.begin(); fit != faceList.end(); ++fit )  // Iterate through facets
    { // Iterators:
    InputImageSizeType radius = m_RComp;
    bit = ConstNeighborhoodIterator<InputImageType>(  radius, input, *fit  );
    it  = ImageRegionIterator<OutputImageType>(         output,    *fit          );
    // Boundary condition:
    bit.OverrideBoundaryCondition(&nbc);
    InputImageIndexType originR;
    InputImageSizeType  radiusR;
    radiusR = m_RSearch;
    for( bit.GoToBegin(), it.GoToBegin(); !bit.IsAtEnd(); ++bit, ++it )    // Iterate through pixels in the current
                                                                           // facet
      { // Auxiliar value to store filtered values:
      OutputPixelType op = bit.GetCenterPixel();
      // -------------------------------------------------------------------------------------------------------------
      // CREATE THE REGION TO SEARCH AND THE ITERATORS:
      searchSize = baseSearchSize;
      originR = bit.GetIndex() - radiusR;
      bool needToComputeCenter = false;
      for( unsigned int d = 0; d < TInputImage::ImageDimension; ++d )
        {
        if( originR[d] < 0 )
          {
          originR[d] = 0;
          needToComputeCenter = true;
          }
        if( originR[d] + searchSize[d] >= input->GetLargestPossibleRegion().GetSize()[d] )
          {
          searchSize[d] = input->GetLargestPossibleRegion().GetSize()[d] - originR[d];
          needToComputeCenter = true;
          }
        }
      // ---------------------
      // Compute the index corresponding to the original center:
      unsigned int midPosition = numNeighbours / 2;
      if( needToComputeCenter )
        {
        unsigned int aux = 1;
        for( unsigned int d = 0; d < TInputImage::ImageDimension; ++d )
          {
          aux *= searchSize[d];
          }
        midPosition = 0;
        if( aux > 0 )
          {
          for( int d = (int)TInputImage::ImageDimension - 1; d >= 0; --d )
            {
            aux /= searchSize[d];
            midPosition += ( bit.GetIndex()[d] - originR[d] ) * aux;
            }
          }
        }
      // ---------------------
      searchRegion.SetIndex( originR );
      searchRegion.SetSize( searchSize );
      search = ConstNeighborhoodIterator<InputImageType>(  radius, input, searchRegion  );
      // -------------------------------------------------------------------------------------------------------------
      // FILTER THE BASELINES
      for( unsigned int j = 0; j < m_NBaselines; ++j )  // For each baseline
        {
        norm = 0.0f;                                                           // To normalize the weights to sum to 1
        max  = -100;                                                           // To avoid over-weighting of the central
                                                                               // value
        unsigned int pos;                                                      // Auxiliar variable
        for( pos = 0, search.GoToBegin(); !search.IsAtEnd(); ++search, ++pos ) // For each pixel in the search
                                                                               // neighbourhood
          {
          if( pos != midPosition )
            {
            distB[pos] = itk::NumericTraits<float>::ZeroValue();
            for( unsigned int k = 0; k < neighborhoodSize; ++k )  // For each pixel in the comparison neighbourhood
              {
              float aux   = bit.GetPixel(k)[m_Baselines[j]] - search.GetPixel(k)[m_Baselines[j]];
              distB[pos] += gw[k] * aux * aux;
              }
            // Temporal patch:
            distB[pos]  = exp( -distB[pos] * sqh * 0.0625 );
            norm       += distB[pos];
            if( distB[pos] > max )
              {
              max = distB[pos];
              }
            }
          valsB[pos] = search.GetCenterPixel()[m_Baselines[j]] * search.GetCenterPixel()[m_Baselines[j]];
          }
        if( max > 1e-6 )
          {
          distB[midPosition] = max;
          norm = 1.0f / (max + norm);
          }
        else
          {
          distB[midPosition] = 1.0f;
          norm = 1.0f / (1.0f + norm);
          }
        float value = itk::NumericTraits<float>::ZeroValue();
        for( unsigned int k = 0; k < pos; ++k )
          {
          value += distB[k] * valsB[k] * norm;
          }
        // Remove Rician bias:
        value -= 2.0f * m_Sigma * m_Sigma;
        value = ( value > 1e-10 ? ::sqrt(value) : itk::NumericTraits<float>::ZeroValue() );
        op[m_Baselines[j]] = static_cast<ScalarType>(value);
        }
      // -------------------------------------------------------------------------------------------------------------
      // FILTER THE GRADIENT IMAGES
      for( unsigned int j = 0; j < m_NDWI; ++j )  // For each gradient image
        {
        norm = 0.0f;
        max  = -100;
        unsigned int pos;
        for( pos = 0, search.GoToBegin(); !search.IsAtEnd(); ++search, ++pos )  // For each pixel in the search
                                                                                // neighbourhood
          { // First, we process the gradients in the same direction as the one being processed:
          if( pos != midPosition )
            {
            distD[pos] = itk::NumericTraits<float>::ZeroValue();
            for( unsigned int k = 0; k < neighborhoodSize; ++k )  // For each pixel in the comparison neighbourhood
              {
              float aux = bit.GetPixel(k)[m_DWI[j]] - search.GetPixel(k)[m_DWI[j]];
              distD[pos] += gw[k] * aux * aux;
              }
            distD[pos]  = exp( -distD[pos] * sqh );
            norm       += distD[pos];
            if( distD[pos] > max )
              {
              max = distD[pos];
              }
            }
          valsD[pos] = search.GetCenterPixel()[m_DWI[j]] * search.GetCenterPixel()[m_DWI[j]];
          // Now, we may process the gradient directions similar to the direction under study
          for( unsigned int g = 1; g < m_Neighbours; ++g )
            {
            distD[pos + g * numNeighbours] = itk::NumericTraits<float>::ZeroValue();
            for( unsigned int k = 0; k < neighborhoodSize; ++k )  // For each pixel in the comparison neighbourhood
              {
              float aux = bit.GetPixel(k)[m_DWI[j]] - search.GetPixel(k)[m_NeighboursInd[j][g]];
              distD[pos + g * numNeighbours] += gw[k] * aux * aux;
              }
            distD[pos + g * numNeighbours]  = exp( -distD[pos + g * numNeighbours] * sqh );
            norm                       += distD[pos + g * numNeighbours];
            if( distD[pos + g * numNeighbours] > max )
              {
              max = distD[pos + g * numNeighbours];
              }
            valsD[pos + g
                  * numNeighbours] = search.GetCenterPixel()[m_NeighboursInd[j][g]]
              * search.GetCenterPixel()[m_NeighboursInd[j][g]];
            }
          }
        if( max > 1e-6 )
          {
          distD[midPosition] = max;
          norm = 1.0f / (max + norm);
          }
        else
          {
          distD[midPosition] = 1;
          norm = 1.0f / (1 + norm);
          }
        float value = itk::NumericTraits<float>::ZeroValue();
        for( unsigned int k = 0; k < pos; ++k )
          {
          for( unsigned int l = 0; l < m_Neighbours; ++l )
            {
            value += distD[k + l * numNeighbours] * valsD[k + l * numNeighbours] * norm;
            }
          }
        // Remove Rician bias:
        value -= 2.0f * m_Sigma * m_Sigma;
        value = ( value > 1e-10 ? ::sqrt(value) : itk::NumericTraits<float>::ZeroValue() );
        op[m_DWI[j]] = static_cast<ScalarType>(value);;
        }
      // -------------------------------------------------------------------------------------------------------------
      // Set the output pixel
      it.Set( op );
      }
    }
  // Delete previously allocated memory:
  delete[] gw;
  delete[] distB;
  delete[] distD;
  delete[] valsB;
  delete[] valsD;
}

} // end namespace itk

#endif
