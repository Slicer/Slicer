#ifndef __itkLargestForegroundFilledMaskImageFilter_txx
#define __itkLargestForegroundFilledMaskImageFilter_txx
#include "itkLargestForegroundFilledMaskImageFilter.h"
#include "itkComputeHistogramQuantileThresholds.h"

#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkImageRegionIterator.h>
// #include <itkSimpleFilterWatcher.h>
#include <itkImageRegionIterator.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <vnl/vnl_sample.h>
#include <vnl/vnl_math.h>
#include <itkConnectedThresholdImageFilter.h>

#include <itkNumericTraits.h>
#include <itkMinimumMaximumImageFilter.h>
#include <itkScalarImageToHistogramGenerator.h>
// Not this:   #include <itkOtsuMultipleThresholdsCalculator.h>
#include <itkOtsuThresholdImageCalculator.h>
#include <itkCastImageFilter.h>

namespace itk
{
template< class TInputImage, class TOutputImage >
LargestForegroundFilledMaskImageFilter< TInputImage, TOutputImage >
::LargestForegroundFilledMaskImageFilter():
  m_OtsuPercentileLowerThreshold(0.01),
  m_OtsuPercentileUpperThreshold(1.0 - 0.01),
  m_ThresholdCorrectionFactor(1.0),
  m_ClosingSize(9.0),
  m_DilateSize(0.0),
  m_InsideValue(NumericTraits< typename IntegerImageType::PixelType >::One),
  m_OutsideValue(NumericTraits< typename IntegerImageType::PixelType >::Zero)
{
  //   this->m_InsideValue =
  //     NumericTraits<typename IntegerImageType::PixelType>::One;
  //   this->m_OutsideValue =
  //     NumericTraits<typename IntegerImageType::PixelType>::Zero;
}

template< class TInputImage, class TOutputImage >
LargestForegroundFilledMaskImageFilter< TInputImage, TOutputImage >
::~LargestForegroundFilledMaskImageFilter()
{}

template< class TInputImage, class TOutputImage >
void
LargestForegroundFilledMaskImageFilter< TInputImage, TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << "OtsuPercentileLowerThreshold "
     << m_OtsuPercentileLowerThreshold << " "
     << "OtsuPercentileUpperThreshold "
     << m_OtsuPercentileUpperThreshold << " "
     << "ClosingSize "
     << m_ClosingSize << " "
     << "InsideValue "
     << m_InsideValue << " "
     << "OutsideValue "
     << m_OutsideValue << std::endl;
}

template< class TInputImage, class TOutputImage >
void
LargestForegroundFilledMaskImageFilter< TInputImage, TOutputImage >
::ImageMinMax(typename TInputImage::PixelType & imageMin,
              typename TInputImage::PixelType & imageMax)
{
  typename MinimumMaximumImageFilter< TInputImage >::Pointer minmaxFilter =
    MinimumMaximumImageFilter< TInputImage >::New();
  minmaxFilter->SetInput( this->GetInput() );
  minmaxFilter->Update();
  imageMax = minmaxFilter->GetMaximum();
  imageMin = minmaxFilter->GetMinimum();
}

template< class TInputImage, class TOutputImage >
void
LargestForegroundFilledMaskImageFilter< TInputImage, TOutputImage >
::GenerateData()
{
    {
    // These checks guarantee that setLowHigh works with at least two bins.
    // Threshold 0.0 is special:  a pure Otsu on 100 bins.
    if ( this->m_OtsuPercentileLowerThreshold < 0.0 || this->m_OtsuPercentileLowerThreshold > 1.5
         || this->m_OtsuPercentileUpperThreshold < 0 )
      {
      itkExceptionMacro(<< "Throwing out worthless PercentileThreshold:  "
                        << this->m_OtsuPercentileLowerThreshold << " " <<  this->m_OtsuPercentileUpperThreshold << " ");
      }
    if ( this->m_OtsuPercentileUpperThreshold > 1.5 )
      {
      itkExceptionMacro(
        << "To save the day, PRETENDING an apparently mistaken histogram-trimming"
           " threshold >= 1.5 really indicates number of histogram bins"
           " (3.5 rounds up and indicates quartiles, etc.):  "
        << this->m_OtsuPercentileLowerThreshold << " " <<  this->m_OtsuPercentileUpperThreshold << " ");
      }
    if ( this->m_OtsuPercentileLowerThreshold > 0.5  || this->m_OtsuPercentileUpperThreshold < 0.5 )
      {
      itkExceptionMacro(<< "Trimming back worthless PercentileThreshold"
                           " over the two-tailed maximum of 0.5:  "
                        << this->m_OtsuPercentileLowerThreshold << " " <<  this->m_OtsuPercentileUpperThreshold << " ");
      }
    }
  this->AllocateOutputs();

  typedef ComputeHistogramQuantileThresholds< TInputImage, TOutputImage > ImageCalcType;
  typename ImageCalcType::Pointer ImageCalc = ImageCalcType::New();
  ImageCalc->SetImage( this->GetInput() );

  ImageCalc->SetQuantileLowerThreshold(m_OtsuPercentileLowerThreshold);
  ImageCalc->SetQuantileUpperThreshold(m_OtsuPercentileUpperThreshold);

  ImageCalc->Calculate();

  typename TInputImage::PixelType threshold_low_foreground;
  typename TInputImage::PixelType threshold_low = ImageCalc->GetLowerIntensityThresholdValue();
  typename TInputImage::PixelType threshold_hi  = ImageCalc->GetUpperIntensityThresholdValue();
  const unsigned int numNonZeroHistogramBins = ImageCalc->GetNumberOfValidHistogramsEntries();

  if ( numNonZeroHistogramBins <= 2 )
    {
    threshold_low_foreground = threshold_hi;
    }
  else
    {
    // ## The Otsu thresholding stuff below should not be part of the new class,
    // it shout really be a separate function.
    typedef OtsuThresholdImageCalculator< TInputImage > OtsuImageCalcType;
    typename OtsuImageCalcType::Pointer OtsuImageCalc = OtsuImageCalcType::New();
    OtsuImageCalc->SetImage( this->GetInput() );
    OtsuImageCalc->Compute();
    typename TInputImage::PixelType otsuThreshold = OtsuImageCalc->GetThreshold();
    // std::cout << "whole-image-based otsuThreshold was: " << otsuThreshold <<
    // std::endl;

    typename TInputImage::PixelType otsuThresholdResult =
      static_cast< typename TInputImage::PixelType >
      ( m_ThresholdCorrectionFactor * static_cast< double >( otsuThreshold ) );
    threshold_low_foreground = otsuThresholdResult;
    }

  typedef BinaryThresholdImageFilter< TInputImage,
                                      IntegerImageType >
  InputThresholdFilterType;
  typename InputThresholdFilterType::Pointer threshold =
    InputThresholdFilterType::New();
  threshold->SetInput ( this->GetInput() );
  threshold->SetInsideValue(this->m_InsideValue);
  threshold->SetOutsideValue(this->m_OutsideValue);
  threshold->SetLowerThreshold(threshold_low_foreground);
  // threshold->SetUpperThreshold(threshold_hi);
  threshold->SetUpperThreshold(
    NumericTraits< typename TInputImage::PixelType >::max() );
  threshold->Update();
  // C'mon, guys.  I need to know what's going on.  Leave this output visible
  // for me.  Please?
#if 1
  // HACK:  The printout below is not what is really being used to threshold.
  //  it is really threshold_low_foreground, NumericTraits<typename
  // OutputImageType::PixelType>::max()
  std::cout << "LowHigh Thresholds: [" << static_cast< int >( threshold_low ) << ","
            << threshold_low_foreground << "," << static_cast< int >( threshold_hi ) << "]"
            << std::endl;
#endif

  typedef ConnectedComponentImageFilter< IntegerImageType,
                                         IntegerImageType > FilterType;
  typename FilterType::Pointer labelConnectedComponentsFilter = FilterType::New();
  //  SimpleFilterWatcher watcher(labelConnectedComponentsFilter);
  //  watcher.QuietOn();
  labelConnectedComponentsFilter->SetInput ( threshold->GetOutput() );
  // labelConnectedComponentsFilter->Update();

  typedef RelabelComponentImageFilter< IntegerImageType,
                                       IntegerImageType > RelabelType;
  typename RelabelType::Pointer relabel = RelabelType::New();
  relabel->SetInput( labelConnectedComponentsFilter->GetOutput() );

  try
    {
    relabel->Update();
    }
  catch ( ExceptionObject & excep )
    {
    std::cerr << "Relabel: exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    }

  typedef BinaryThresholdImageFilter< IntegerImageType,
                                      IntegerImageType > ThresholdFilterType;
  // unsigned short numObjects = relabel->GetNumberOfObjects();
  // std::cout << "Removed " << numObjects - 1 << " smaller objects." <<
  // std::endl;
  typename ThresholdFilterType::Pointer LargestFilter =
    ThresholdFilterType::New();
  LargestFilter->SetInput( relabel->GetOutput() );
  LargestFilter->SetInsideValue(this->m_InsideValue);
  LargestFilter->SetOutsideValue(this->m_OutsideValue);
  LargestFilter->SetLowerThreshold(1);
  LargestFilter->SetUpperThreshold(1);
  LargestFilter->Update();

  typedef BinaryBallStructuringElement< typename IntegerImageType::
                                        PixelType,
                                        IntegerImageType::ImageDimension >
  myKernelType;

  typedef BinaryErodeImageFilter< IntegerImageType, IntegerImageType,
                                  myKernelType > ErodeFilterType;
  typename ErodeFilterType::Pointer ErodeFilter = ErodeFilterType::New();
    {
    myKernelType dilateBall;
    myKernelType erodeBall;
    typename myKernelType::SizeType dilateBallSize;
    typename myKernelType::SizeType erodeBallSize;
    for ( unsigned int d = 0; d < 3; d++ )
      {
      const unsigned int ClosingVoxels = vnl_math_ceil( m_ClosingSize / ( relabel->GetOutput()->GetSpacing()[d] ) );
      dilateBallSize[d] = ClosingVoxels;
      erodeBallSize[d]  = ClosingVoxels;
      }
    dilateBall.SetRadius(dilateBallSize);
    dilateBall.CreateStructuringElement();
    erodeBall.SetRadius(erodeBallSize);
    erodeBall.CreateStructuringElement();

    typedef BinaryDilateImageFilter< IntegerImageType, IntegerImageType,
                                     myKernelType > DilateFilterType;
    typename DilateFilterType::Pointer DilateFilter = DilateFilterType::New();

    // DilateFilter->SetForegroundValue(1);
    DilateFilter->SetDilateValue(1);
    DilateFilter->SetBackgroundValue(0);
    DilateFilter->SetInput( LargestFilter->GetOutput() );
    DilateFilter->SetKernel(dilateBall);
    DilateFilter->Update();

    // ErodeFilter->SetForegroundValue(1);
    ErodeFilter->SetErodeValue(1);
    ErodeFilter->SetBackgroundValue(0);
    ErodeFilter->SetInput( DilateFilter->GetOutput() );
    ErodeFilter->SetKernel(erodeBall);
    ErodeFilter->Update();
    }

  const typename IntegerImageType::SizeType ImageSize =
    ErodeFilter->GetOutput()->GetLargestPossibleRegion().GetSize();
  // HACK:  The most robust way to do this would be to find the largest
  // background labeled image, and then choose one of those locations as the
  // seed.
  // For now just choose all the corners as seed points
  typedef ConnectedThresholdImageFilter< IntegerImageType,
                                         IntegerImageType >
  seededConnectedThresholdFilterType;
  typename seededConnectedThresholdFilterType::Pointer
  seededConnectedThresholdFilter = seededConnectedThresholdFilterType::New();

    {
    const typename IntegerImageType::IndexType SeedLocation = { { 0, 0, 0 } };
    seededConnectedThresholdFilter->SetSeed(SeedLocation);
    }
    {
    const typename IntegerImageType::IndexType SeedLocation = { { ImageSize[0] - 1, 0, 0 } };
    seededConnectedThresholdFilter->SetSeed(SeedLocation);
    }
    {
    const typename IntegerImageType::IndexType SeedLocation = { { 0, ImageSize[1] - 1, 0 } };
    seededConnectedThresholdFilter->SetSeed(SeedLocation);
    }
    {
    const typename IntegerImageType::IndexType SeedLocation = { { ImageSize[0] - 1, ImageSize[1] - 1, 0 } };
    seededConnectedThresholdFilter->SetSeed(SeedLocation);
    }
    {
    const typename IntegerImageType::IndexType SeedLocation = { { 0, 0, ImageSize[2] - 1 } };
    seededConnectedThresholdFilter->SetSeed(SeedLocation);
    }
    {
    const typename IntegerImageType::IndexType SeedLocation = { { ImageSize[0] - 1, 0, ImageSize[2] - 1 } };
    seededConnectedThresholdFilter->SetSeed(SeedLocation);
    }
    {
    const typename IntegerImageType::IndexType SeedLocation = { { 0, ImageSize[1] - 1, ImageSize[2] - 1 } };
    seededConnectedThresholdFilter->SetSeed(SeedLocation);
    }
    {
    const typename IntegerImageType::IndexType SeedLocation = { { ImageSize[0] - 1, ImageSize[1] - 1, ImageSize[2] - 1 } };
    seededConnectedThresholdFilter->SetSeed(SeedLocation);
    }

  seededConnectedThresholdFilter->SetReplaceValue(100);
  seededConnectedThresholdFilter->SetUpper(0);
  seededConnectedThresholdFilter->SetLower(0);
  seededConnectedThresholdFilter->SetInput( ErodeFilter->GetOutput() );
  seededConnectedThresholdFilter->Update();

  typename IntegerImageType::Pointer dilateMask = NULL;
    {
    typename ThresholdFilterType::Pointer FinalThreshold =
      ThresholdFilterType::New();
    FinalThreshold->SetInput ( seededConnectedThresholdFilter->GetOutput() );
    FinalThreshold->SetInsideValue(this->m_OutsideValue);
    FinalThreshold->SetOutsideValue(this->m_InsideValue);
    FinalThreshold->SetLowerThreshold(100);
    FinalThreshold->SetUpperThreshold(100);
    FinalThreshold->Update();

    if ( m_DilateSize > 0.0 )
      {
      // Dilate to get some background to better drive BSplineRegistration
      typedef itk::BinaryDilateImageFilter< IntegerImageType, IntegerImageType,
                                            myKernelType > DilateType;

      myKernelType dilateBall;
      typename myKernelType::SizeType dilateBallSize;
      for ( unsigned int d = 0; d < 3; d++ )
        {
        const unsigned int DilateVoxels = vnl_math_ceil( m_DilateSize / ( FinalThreshold->GetOutput()->GetSpacing()[d] ) );
        dilateBallSize[d] = DilateVoxels;
        }
      dilateBall.SetRadius(dilateBallSize);
      dilateBall.CreateStructuringElement();

      typename DilateType::Pointer dil = DilateType::New();
      dil->SetDilateValue(this->m_InsideValue);
      dil->SetKernel(dilateBall);
      dil->SetInput( FinalThreshold->GetOutput() );
      dil->Update();
      dilateMask = dil->GetOutput();
      }
    else
      {
      dilateMask = FinalThreshold->GetOutput();
      }
    }

  typedef CastImageFilter< IntegerImageType, OutputImageType > outputCasterType;
  typename outputCasterType::Pointer outputCaster = outputCasterType::New();
  outputCaster->SetInput(dilateMask);

  outputCaster->GraftOutput( this->GetOutput() );
  outputCaster->Update();
  this->GraftOutput( outputCaster->GetOutput() );
  //  typename OutputImageType::Pointer outputMaskImage =
  // outputCaster->GetOutput();
  //  return outputMaskImage;
}
}
#endif // itkLargestForegroundFilledMaskImageFilter_txx
