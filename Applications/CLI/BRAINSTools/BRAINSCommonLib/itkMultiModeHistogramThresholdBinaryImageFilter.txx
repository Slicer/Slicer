#include "itkMultiModeHistogramThresholdBinaryImageFilter.h"
#include "itkComputeHistogramQuantileThresholds.h"

#include <itkBinaryThresholdImageFilter.h>
#include <itkMultiplyImageFilter.h>

#include <itkNumericTraits.h>
#include <itkMinimumMaximumImageFilter.h>
// Not this:   #include <itkOtsuMultipleThresholdsCalculator.h>
#include <itkOtsuThresholdImageCalculator.h>
#include <itkCastImageFilter.h>

namespace itk
{
template< class TInputImage, class TOutputImage >
MultiModeHistogramThresholdBinaryImageFilter< TInputImage, TOutputImage >
::MultiModeHistogramThresholdBinaryImageFilter():
  m_QuantileLowerThreshold(1), // temporarily estimate how many SetInput images
                               // there are
  m_QuantileUpperThreshold(1),
  m_InsideValue(NumericTraits< typename IntegerImageType::PixelType >::One),
  m_OutsideValue(NumericTraits< typename IntegerImageType::PixelType >::Zero)
{
  m_QuantileLowerThreshold.Fill(0.0);
  m_QuantileUpperThreshold.Fill(1.0);
  m_LinearQuantileThreshold = 0.01;
  //   this->m_InsideValue =
  //     NumericTraits<typename IntegerImageType::PixelType>::One;
  //   this->m_OutsideValue =
  //     NumericTraits<typename IntegerImageType::PixelType>::Zero;
}

template< class TInputImage, class TOutputImage >
MultiModeHistogramThresholdBinaryImageFilter< TInputImage, TOutputImage >
::~MultiModeHistogramThresholdBinaryImageFilter()
{}

template< class TInputImage, class TOutputImage >
void
MultiModeHistogramThresholdBinaryImageFilter< TInputImage, TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << "QuantileLowerThreshold "
     << m_QuantileLowerThreshold << " "
     << "QuantileUpperThreshold "
     << m_QuantileUpperThreshold << " "
     << "InsideValue "
     << m_InsideValue << " "
     << "OutsideValue "
     << m_OutsideValue << std::endl;
}

template< class TInputImage, class TOutputImage >
void
MultiModeHistogramThresholdBinaryImageFilter< TInputImage, TOutputImage >
::GenerateData()
{
  this->AllocateOutputs();

  typename IntegerImageType::Pointer accumulate = IntegerImageType::New();

  const unsigned int NumInputs = this->GetNumberOfInputs();
  for ( unsigned int j = 0; j < NumInputs; j++ )
    {
    //Compute the quantile regions for linearizing the percentages.
    typedef ComputeHistogramQuantileThresholds< TInputImage, TOutputImage > ImageCalcType;
    typename ImageCalcType::Pointer ImageCalc = ImageCalcType::New();
    ImageCalc->SetImage( this->GetInput(j) );

    ImageCalc->SetQuantileLowerThreshold(m_LinearQuantileThreshold);
    ImageCalc->SetQuantileUpperThreshold(1.0 - m_LinearQuantileThreshold);

    std::cout << "Quantile Thresholds: [ "
              << m_QuantileLowerThreshold.GetElement(j) << ", "
              << m_QuantileUpperThreshold.GetElement(j) << " ]"
              << std::endl;

    ImageCalc->SetBinaryPortionImage(this->m_BinaryPortionImage);
    ImageCalc->Calculate();

    const typename InputImageType::PixelType thresholdLowerLinearRegion = ImageCalc->GetLowerIntensityThresholdValue();
    const typename InputImageType::PixelType thresholdUpperLinearRegion  = ImageCalc->GetUpperIntensityThresholdValue();
    const typename InputImageType::PixelType imageMinValue  = ImageCalc->GetImageMin();
    const typename InputImageType::PixelType imageMaxValue  = ImageCalc->GetImageMax();
    const unsigned int numNonZeroHistogramBins = ImageCalc->GetNumberOfValidHistogramsEntries();

    typename InputImageType::PixelType thresholdLowerLinearRegion_foreground;
    if ( numNonZeroHistogramBins <= 2 )
      {
      thresholdLowerLinearRegion_foreground = thresholdUpperLinearRegion;
      }
    else
      {
      thresholdLowerLinearRegion_foreground = thresholdLowerLinearRegion;
      }

    std::cout << "LowHigh Thresholds: [ " << thresholdLowerLinearRegion << ", "
              << thresholdLowerLinearRegion_foreground << ", " << thresholdUpperLinearRegion << " ]"
              << std::endl;

    typedef BinaryThresholdImageFilter< InputImageType,
                                        IntegerImageType >
    ThresholdFilterType;
    typename ThresholdFilterType::Pointer threshold =
      ThresholdFilterType::New();
    threshold->SetInput ( this->GetInput(j) );
    threshold->SetInsideValue(this->m_InsideValue);
    threshold->SetOutsideValue(this->m_OutsideValue);
    typename InputImageType::PixelType intensity_thresholdLowerLinearRegion;
    typename InputImageType::PixelType intensity_thresholdUpperLinearRegion;
    if ( m_QuantileLowerThreshold.GetElement(j) < m_LinearQuantileThreshold )
      {
      const double range = ( m_LinearQuantileThreshold - 0.0 );
      const double percentValue = ( m_QuantileLowerThreshold.GetElement(j) - 0.0 ) / range;
      intensity_thresholdLowerLinearRegion =
        static_cast< typename InputImageType::PixelType >(
          imageMinValue + ( thresholdLowerLinearRegion_foreground - imageMinValue ) * percentValue );
      }
    else
      {
      const double range = ( 1.0 - m_LinearQuantileThreshold ) - m_LinearQuantileThreshold;
      const double percentValue = ( m_QuantileLowerThreshold.GetElement(j) - m_LinearQuantileThreshold ) / range;
      intensity_thresholdLowerLinearRegion =
        static_cast< typename InputImageType::PixelType >(
          thresholdLowerLinearRegion_foreground
          + ( thresholdUpperLinearRegion - thresholdLowerLinearRegion_foreground ) * percentValue );
      }
    if ( m_QuantileUpperThreshold.GetElement(j) > ( 1.0 - m_LinearQuantileThreshold ) )
      {
      const double range = 1.0 - m_LinearQuantileThreshold;
      const double percentValue = ( m_QuantileUpperThreshold.GetElement(j) - m_LinearQuantileThreshold ) / range;
      intensity_thresholdUpperLinearRegion = static_cast< typename InputImageType::PixelType >(
        thresholdUpperLinearRegion
        + ( imageMaxValue - thresholdUpperLinearRegion ) * percentValue );
      }
    else
      {
      const double range = ( 1.0 - m_LinearQuantileThreshold ) - m_LinearQuantileThreshold;
      const double percentValue = ( m_QuantileUpperThreshold.GetElement(j) - m_LinearQuantileThreshold ) / range;
      intensity_thresholdUpperLinearRegion = static_cast< typename InputImageType::PixelType >(
        thresholdLowerLinearRegion_foreground
        + ( thresholdUpperLinearRegion - thresholdLowerLinearRegion_foreground ) * percentValue );
      }
    std::cout << "DEBUG:MINMAX:DEBUG: ["
              << imageMinValue << "," << imageMaxValue << "]" << std::endl;
    std::cout << "DEBUG:LINLOWHIGH:DEBUG: ["
              << thresholdLowerLinearRegion << "," << thresholdUpperLinearRegion << "]" << std::endl;
    std::cout << "DEBUG:RANGE:DEBUG:  ["
              << intensity_thresholdLowerLinearRegion << "," << intensity_thresholdUpperLinearRegion << "]"
              << std::endl;
    threshold->SetLowerThreshold(intensity_thresholdLowerLinearRegion);
    threshold->SetUpperThreshold(intensity_thresholdUpperLinearRegion);
    // threshold->SetUpperThreshold( NumericTraits<typename
    // InputImageType::PixelType>::max() );
    threshold->Update();
    typename IntegerImageType::Pointer thresholdImage=threshold->GetOutput();

    if ( j == 0 )
      {
      accumulate = thresholdImage;
      }
    else
      {
      typedef MultiplyImageFilter< IntegerImageType, IntegerImageType > IntersectMasksFilterType;
      if(accumulate->GetLargestPossibleRegion().GetSize() != thresholdImage->GetLargestPossibleRegion().GetSize())
        {
        itkExceptionMacro(<< "Image data size mismatch " << accumulate->GetLargestPossibleRegion().GetSize() << " != " << thresholdImage->GetLargestPossibleRegion().GetSize() << "." << std::endl );
        }
      if(accumulate->GetSpacing() != thresholdImage->GetSpacing())
        {
        itkExceptionMacro(<< "Image data spacing mismatch " << accumulate->GetSpacing() << " != " << thresholdImage->GetSpacing() << "." << std::endl );
        }
      if(accumulate->GetDirection() != thresholdImage->GetDirection())
        {
        itkExceptionMacro(<< "Image data spacing mismatch " << accumulate->GetDirection() << " != " << thresholdImage->GetDirection() << "." << std::endl );
        }
      if(accumulate->GetOrigin() != thresholdImage->GetOrigin())
        {
        itkExceptionMacro(<< "Image data spacing mismatch " << accumulate->GetOrigin() << " != " << thresholdImage->GetOrigin() << "." << std::endl );
        }
      typename IntersectMasksFilterType::Pointer intersect = IntersectMasksFilterType::New();
      intersect->SetInput1(accumulate);
      intersect->SetInput2( thresholdImage );
      intersect->Update();
      accumulate = intersect->GetOutput();
      }
    }

  typedef CastImageFilter< IntegerImageType, OutputImageType > outputCasterType;
  typename outputCasterType::Pointer outputCaster = outputCasterType::New();
  outputCaster->SetInput(accumulate);

  outputCaster->GraftOutput( this->GetOutput() );
  outputCaster->Update();
  this->GraftOutput( outputCaster->GetOutput() );
  //  typename OutputImageType::Pointer outputMaskImage =
  // outputCaster->GetOutput();
  //  return outputMaskImage;
}
}
