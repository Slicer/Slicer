/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkOtsuStatistics.txx,v $
  Language:  C++
  Date:      $Date: 2005/05/4 14:28:51 $
  Version:   $Revision: 1.1
=========================================================================*/
#ifndef _itkOtsuStatistics_txx
#define _itkOtsuStatistics_txx
#include "itkOtsuStatistics.h"

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
OtsuStatistics<TInputImage, TOutputImage>::OtsuStatistics()
{
  m_Radius.Fill(0);
  m_Channels = 1;
  m_Mode     = USE_AVERAGED_BASELINES;
  m_Min      = itk::NumericTraits<double>::max();
  m_Max      = itk::NumericTraits<double>::min();;
  m_Indicator.SetSize(1);
  m_Indicator[0] = 0;
}

/** The requested input region is larger than the corresponding output, so we need to override this method: */
template <class TInputImage, class TOutputImage>
void OtsuStatistics<TInputImage, TOutputImage>
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
void OtsuStatistics<TInputImage, TOutputImage>
::BeforeThreadedGenerateData( void )
{
  m_ThreadMin.SetSize( this->GetNumberOfThreads() );
  m_ThreadMin.Fill( itk::NumericTraits<double>::max() );
  m_ThreadMax.SetSize( this->GetNumberOfThreads() );
  m_ThreadMax.Fill( itk::NumericTraits<double>::min() );
}

template <class TInputImage, class TOutputImage>
void OtsuStatistics<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId )
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
  for( fit = faceList.begin(); fit != faceList.end(); ++fit )  // Iterate through facets
    { // Iterators:
    bit = ConstNeighborhoodIterator<InputImageType>(  m_Radius, input, *fit  );
    it  = ImageRegionIterator<OutputImageType>(        output,     *fit      );
    unsigned int neighborhoodSize = bit.Size();
    // Get the number of baseline images:
    unsigned int nBaselines = m_Indicator.GetSize();
    // Get the normalization fator depending on the mode of operation:
    double factor;

    switch( m_Mode )
      {
      case USE_AVERAGED_GRADIENTS:
        factor = 1.0f / (double)m_Channels;
        break;
      case USE_NEIGHBORHOOD_BASELINES:
        factor = 1.0f / (double)( nBaselines * neighborhoodSize );
        break;
      case USE_NEIGHBORHOOD_GRADIENTS:
        factor = 1.0f / (double)( m_Channels * neighborhoodSize );
        break;
      case USE_AVERAGED_BASELINES: // Default!!
      default:
        factor = 1.0f / (double)nBaselines;
        break;
      }
    // Boundary condition:
    bit.OverrideBoundaryCondition( &nbc );
    // ===========================================================================================================================
    for( bit.GoToBegin(), it.GoToBegin(); !bit.IsAtEnd(); ++bit, ++it )    // Iterate through pixels in the current
                                                                           // facet
      { // Depending on the mode of operation, take the center pixel or the whole vicinity:
      double averagedValue = itk::NumericTraits<float>::ZeroValue();

      switch( m_Mode )
        {
        case USE_AVERAGED_GRADIENTS:
          for( unsigned int k = 0; k < m_Channels; ++k )
            {
            averagedValue += bit.GetCenterPixel()[k];
            }
          break;
        case USE_NEIGHBORHOOD_BASELINES:
          for( unsigned int p = 0; p < neighborhoodSize; ++p )
            {
            InputPixelType ip = bit.GetPixel( p );
            for( unsigned int k = 0; k < nBaselines; ++k )
              {
              averagedValue += ip[m_Indicator[k]];
              }
            }
          break;
        case USE_NEIGHBORHOOD_GRADIENTS:
          for( unsigned int p = 0; p < neighborhoodSize; ++p )
            {
            InputPixelType ip = bit.GetPixel( p );
            for( unsigned int k = 0; k < m_Channels; ++k )
              {
              averagedValue += ip[k];
              }
            }
          break;
        case USE_AVERAGED_BASELINES: // Default!!
        default:
          for( unsigned int k = 0; k < nBaselines; ++k )
            {
            averagedValue += bit.GetCenterPixel()[m_Indicator[k]];
            }
          break;
        }
      averagedValue *= factor;
      if( averagedValue >= 2.0f )
        {
        if( averagedValue < m_ThreadMin[threadId] )
          {
          m_ThreadMin[threadId] = averagedValue;
          }
        if( averagedValue > m_ThreadMax[threadId] )
          {
          m_ThreadMax[threadId] = averagedValue;
          }
        }
      it.Set( static_cast<OutputPixelType>(averagedValue) );
      }
    // ===========================================================================================================================
    }
}

template <class TInputImage, class TOutputImage>
void OtsuStatistics<TInputImage, TOutputImage>
::AfterThreadedGenerateData( void )
{
  m_Min = itk::NumericTraits<double>::max();
  m_Max = itk::NumericTraits<double>::min();
  for( unsigned int k = 0; k < (unsigned int)(this->GetNumberOfThreads()); ++k )
    {
    if( m_ThreadMin[k] < m_Min )
      {
      m_Min = m_ThreadMin[k];
      }
    if( m_ThreadMax[k] > m_Max )
      {
      m_Max = m_ThreadMax[k];
      }
    }
}

/** Standard "PrintSelf" method */
template <class TInputImage, class TOutput>
void OtsuStatistics<TInputImage, TOutput>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius: "                             << m_Radius                             << std::endl;
  os << indent << "Channels: "                           << m_Channels                           << std::endl;
  os << indent << "Mode: "                               << m_Mode                               << std::endl;
  os << indent << "Min: "                                << m_Min                                << std::endl;
  os << indent << "Max: "                                << m_Max                                << std::endl;
  os << indent << "Indicator: "                          << m_Indicator                          << std::endl;
}

} // end namespace itk

#endif
