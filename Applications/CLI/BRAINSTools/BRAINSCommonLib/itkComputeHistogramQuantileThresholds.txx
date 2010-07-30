#ifndef __itkComputeHistogramQuantileThresholds_txx
#define __itkComputeHistogramQuantileThresholds_txx
#include "itkComputeHistogramQuantileThresholds.h"

// #include <itkSimpleFilterWatcher.h>
#include <itkImageRegionIterator.h>
#include <vnl/vnl_sample.h>
#include <vnl/vnl_math.h>

#include <itkNumericTraits.h>
#include <itkMinimumMaximumImageFilter.h>
#include "itkScalarImagePortionToHistogramGenerator.h"

namespace itk
{
template< class TInputImage, class TMaskImage >
ComputeHistogramQuantileThresholds< TInputImage, TMaskImage >
::ComputeHistogramQuantileThresholds():
  m_QuantileLowerThreshold(0.0),
  m_QuantileUpperThreshold(1.0)
{}

template< class TInputImage, class TMaskImage >
ComputeHistogramQuantileThresholds< TInputImage, TMaskImage >
::~ComputeHistogramQuantileThresholds()
{}

template< class TInputImage, class TMaskImage >
void
ComputeHistogramQuantileThresholds< TInputImage, TMaskImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << "QuantileLowerThreshold "
     << m_QuantileLowerThreshold << " "
     << "QuantileUpperThreshold "
     << m_QuantileUpperThreshold << std::endl;
}

template< class TInputImage, class TMaskImage >
void
ComputeHistogramQuantileThresholds< TInputImage, TMaskImage >
::ImageMinMax(typename TInputImage::PixelType & ImageMin,
              typename TInputImage::PixelType & ImageMax)
{
  typename MinimumMaximumImageFilter< TInputImage >::Pointer minmaxFilter =
    MinimumMaximumImageFilter< TInputImage >::New();
  minmaxFilter->SetInput( this->GetImage() );
  minmaxFilter->Update();
  ImageMax = minmaxFilter->GetMaximum();
  ImageMin = minmaxFilter->GetMinimum();
}

/**
  *
  * @author hjohnson (8/12/2008)
  *
  * @param m_LowerIntensityThresholdValue  The intensity value where
  *"HistogramQuantileLowerThreshold" voxels
  *              are below this threshold
  * @param m_UpperIntensityThresholdValue  The intensity value where
  *"HistogramQuantileUpperThreshold" voxels
  *              are above this threshold
  * @param m_NumberOfValidHistogramsEntries  The number of non-zero histogram
  *bins
  */
template< class TInputImage, class TMaskImage >
void
ComputeHistogramQuantileThresholds< TInputImage, TMaskImage >
::Calculate()
{
  this->ImageMinMax(this->m_ImageMin, this->m_ImageMax);

  typedef Statistics::ScalarImagePortionToHistogramGenerator< TInputImage, TMaskImage >
  HistogramGeneratorType;
  typename HistogramGeneratorType::Pointer histogramGenerator =
    HistogramGeneratorType::New();
  histogramGenerator->SetInput( this->GetImage() );
  if ( m_BinaryPortionImage.IsNotNull() )
    {
    histogramGenerator->SetBinaryPortionImage(m_BinaryPortionImage);
    }

  int NumberOfBins = static_cast< unsigned int >( m_ImageMax - m_ImageMin + 1 );
#ifdef ITK_USE_REVIEW_STATISTICS
  histogramGenerator->SetNumberOfBins(NumberOfBins);
  histogramGenerator->SetMarginalScale(1.0);
  histogramGenerator->SetHistogramMin(m_ImageMin);
  histogramGenerator->SetHistogramMax(m_ImageMax);
#else
  histogramGenerator->InitializeHistogram(NumberOfBins, m_ImageMin, m_ImageMax);
#endif

  histogramGenerator->Compute();
  typedef typename HistogramGeneratorType::HistogramType HistogramType;
  HistogramType *histogram = const_cast< HistogramType * >( histogramGenerator->GetOutput() );
  //  If the number of non-zero bins is <= 2, then it is a binary image, and
  // Otsu won't do:
  //
  m_NumberOfValidHistogramsEntries = 0;
    {
    typename HistogramType::Iterator histIt = histogram->Begin();
    bool saw_lowest = false;
    while ( histIt != histogram->End() )
      {
      // walking a 1-dimensional histogram from low to high:
      const double measurement(histIt.GetMeasurementVector()[0]);

      if ( histIt.GetFrequency() != 0 )
        {
        ++m_NumberOfValidHistogramsEntries;
        m_UpperIntensityThresholdValue = static_cast< int >( measurement + 0.5 );
        // rounding by chopping
        if ( !saw_lowest )
          {
          m_LowerIntensityThresholdValue = static_cast< int >( measurement + 0.5 );
          // rounding by chopping
          saw_lowest = true;
          }
        }
      ++histIt;
      }
    if ( m_NumberOfValidHistogramsEntries <= 2 ) // then it is a binary image:
      {
      std::cout
      << "Image handled with only two catgegories; effectively, binary thresholding."
      << std::endl;
      }
    else
      {
      m_LowerIntensityThresholdValue =
        static_cast< typename TInputImage::PixelType >
        ( histogram->Quantile(0, this->m_QuantileLowerThreshold) );
      m_UpperIntensityThresholdValue =
        static_cast< typename TInputImage::PixelType >
        ( histogram->Quantile(0, this->m_QuantileUpperThreshold) );
      std::cout
      << m_NumberOfValidHistogramsEntries
      << " ValidHistogramsEntries,  "
      << histogram->GetTotalFrequency()
      << " TotalFrequency"
      << std::endl;
      std::cout
      << m_QuantileLowerThreshold
      << " ---> "
      << static_cast< int >( m_LowerIntensityThresholdValue )
      << std::endl;
      std::cout
      << m_QuantileUpperThreshold
      << " ---> "
      << static_cast< int >( m_UpperIntensityThresholdValue )
      << std::endl;
      }
    }
  return;
}
}
#endif // __itkComputeHistogramQuantileThresholds_txx
