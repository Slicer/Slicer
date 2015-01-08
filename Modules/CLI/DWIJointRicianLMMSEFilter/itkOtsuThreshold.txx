/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkOtsuStatistics.txx,v $
  Language:  C++
  Date:      $Date: 2005/05/4 14:28:51 $
  Version:   $Revision: 1.1
=========================================================================*/
#ifndef _itkOtsuThreshold_txx
#define _itkOtsuThreshold_txx
#include "itkOtsuThreshold.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "vnl/vnl_math.h"

namespace itk
{

/** Constructor */
template <class TInputImage, class TOutputImage>
OtsuThreshold<TInputImage, TOutputImage>::OtsuThreshold()
{
  m_Min       = 0.0f;
  m_Max       = 20000.0f;
  m_Bins      = 256;
  m_Threshold = 20000.0f;
  m_W         = 2.0f;
}

template <class TInputImage, class TOutputImage>
void OtsuThreshold<TInputImage, TOutputImage>
::BeforeThreadedGenerateData( void )
{
  m_ThreadHist.SetSize( this->GetNumberOfThreads(), m_Bins );
  m_ThreadHist.Fill( 0.0f );
  m_ThreadCount.SetSize( this->GetNumberOfThreads() );
  m_ThreadCount.Fill(0.0f);
}

template <class TInputImage, class TOutputImage>
void OtsuThreshold<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId )
{
  // Boundary conditions for this filter; Neumann conditions are fine
  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
  // Input and output
  InputImageConstPointer input   =  this->GetInput();
  OutputImagePointer     output  =  this->GetOutput();
  // Iterators:
  ImageRegionConstIterator<InputImageType> bit( input, outputRegionForThread );   // Iterator for the input image
  ImageRegionIterator<OutputImageType>     it( output, outputRegionForThread );   // Iterator for the output image
  // The size of each bin:
  double bfact = (double)m_Bins / ( m_Max - m_Min );
  for( bit.GoToBegin(), it.GoToBegin(); !bit.IsAtEnd(); ++bit, ++it )    // Iterate through pixels in the current facet
    { // Compute histogram bin:
    double val = static_cast<double>( bit.Get() );
    double aux = (val - m_Min) * bfact;
    if( aux >= 0.0f && aux < m_Bins  )
      {
      m_ThreadHist[threadId][(unsigned int)aux] += 1.0f;
      m_ThreadCount[threadId] += 1.0f;
      }
    it.Set( static_cast<OutputPixelType>(val) );
    }
}

template <class TInputImage, class TOutputImage>
void OtsuThreshold<TInputImage, TOutputImage>
::AfterThreadedGenerateData( void )
{
  double totalSamples = itk::NumericTraits<double>::ZeroValue();

  for( unsigned int k = 0; k < (unsigned int)(this->GetNumberOfThreads()); ++k )
    {
    totalSamples += m_ThreadCount[k];
    }
  for( unsigned int b = 0; b < m_Bins; ++b )
    {
    for( unsigned int k = 1; k < (unsigned int)(this->GetNumberOfThreads()); ++k )
      {
      m_ThreadHist[0][b] += m_ThreadHist[k][b];
      }
    }
  double totalMean = itk::NumericTraits<double>::ZeroValue();
  for( unsigned int b = 0; b < m_Bins; ++b )
    {
    m_ThreadHist[0][b] /= totalSamples;
    totalMean          += (b + 1) * m_ThreadHist[0][b];
    }
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // Compute Otsu's threshold by maximizing the between-class variance
  double freqLeft = m_ThreadHist[0][0];
  double meanLeft = 1.0;
  double meanRight = ( totalMean - freqLeft ) / ( 1.0 - freqLeft );

  double maxVarBetween = freqLeft * ( 1.0 - freqLeft ) * pow(  fabs(meanLeft - meanRight),   m_W   );
  int    maxBinNumber = 0;

  double freqLeftOld = freqLeft;
  double meanLeftOld = meanLeft;
  for( unsigned int j = 1; j < m_Bins; j++ )
    {
    freqLeft += m_ThreadHist[0][j];
    meanLeft = ( meanLeftOld * freqLeftOld + (j + 1) * m_ThreadHist[0][j] ) / freqLeft;
    if( freqLeft == 1.0 )
      {
      meanRight = 0.0;
      }
    else
      {
      meanRight = ( totalMean - meanLeft * freqLeft ) / ( 1.0 - freqLeft );
      }

    double varBetween = freqLeft * ( 1.0 - freqLeft ) * pow(   fabs(meanLeft - meanRight),   m_W   );
    if( varBetween > maxVarBetween )
      {
      maxVarBetween = varBetween;
      maxBinNumber = j;
      }
    // cache old values
    freqLeftOld = freqLeft;
    meanLeftOld = meanLeft;
    }
  double bfact = ( m_Max - m_Min ) / (double)m_Bins;
  m_Threshold = double( m_Min + ( (double)maxBinNumber + 1.5f ) * bfact );
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
}

/** Standard "PrintSelf" method */
template <class TInputImage, class TOutput>
void OtsuThreshold<TInputImage, TOutput>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "W: "                                  << m_W                                  << std::endl;
  os << indent << "Min: "                                << m_Min                                << std::endl;
  os << indent << "Max: "                                << m_Max                                << std::endl;
  os << indent << "Bins: "                               << m_Bins                               << std::endl;
  os << indent << "Threshold: "                          << m_Threshold                          << std::endl;
}

} // end namespace itk

#endif
