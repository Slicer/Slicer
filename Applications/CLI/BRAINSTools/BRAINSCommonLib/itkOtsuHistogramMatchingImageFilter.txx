/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkOtsuHistogramMatchingImageFilter.txx,v $
 *  Language:  C++
 *  Date:      $Date: 2009-05-05 17:00:00 $
 *  Version:   $Revision: 1.19 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *    This software is distributed WITHOUT ANY WARRANTY; without even
 *    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *    PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __itkOtsuHistogramMatchingImageFilter_txx
#define __itkOtsuHistogramMatchingImageFilter_txx

#include "itkOtsuHistogramMatchingImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNumericTraits.h"
#include <vector>

#include "itkOtsuThresholdImageCalculator.h"

namespace itk
{
/**
  *
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::OtsuHistogramMatchingImageFilter()
{
  this->SetNumberOfRequiredInputs(2);

  m_NumberOfHistogramLevels = 256;
  m_NumberOfMatchPoints = 1;

  m_QuantileTable.set_size(3, m_NumberOfMatchPoints + 2);
  m_QuantileTable.fill(0);
  m_Gradients.set_size(m_NumberOfMatchPoints + 1);
  m_Gradients.fill(0);

  m_ThresholdAtMeanIntensity = true;
  m_SourceIntensityThreshold = 0;
  m_ReferenceIntensityThreshold = 0;
  m_LowerGradient = 0.0;
  m_UpperGradient = 0.0;

  // Create histograms.
  m_SourceHistogram = OtsuHistogramType::New();
  m_ReferenceHistogram = OtsuHistogramType::New();
  m_OutputHistogram = OtsuHistogramType::New();

  m_SourceMask = NULL;
  m_ReferenceMask = NULL;
}

/*
  *
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
void
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "NumberOfHistogramLevels: ";
  os << m_NumberOfHistogramLevels << std::endl;
  os << indent << "NumberOfMatchPoints: ";
  os << m_NumberOfMatchPoints << std::endl;
  os << indent << "ThresholdAtMeanIntensity: ";
  os << m_ThresholdAtMeanIntensity << std::endl;

  os << indent << "SourceIntensityThreshold: ";
  os << m_SourceIntensityThreshold << std::endl;
  os << indent << "ReferenceIntensityThreshold: ";
  os << m_ReferenceIntensityThreshold << std::endl;
  os << indent << "OutputIntensityThreshold: ";
  os << m_ReferenceIntensityThreshold << std::endl;
  os << indent << "Source histogram: ";
  os << m_SourceHistogram.GetPointer() << std::endl;
  os << indent << "Reference histogram: ";
  os << m_ReferenceHistogram.GetPointer() << std::endl;
  os << indent << "Output histogram: ";
  os << m_OutputHistogram.GetPointer() << std::endl;
  os << indent << "QuantileTable: " << std::endl;
  os << m_QuantileTable << std::endl;
  os << indent << "Gradients: " << std::endl;
  os << m_Gradients << std::endl;
  os << indent << "LowerGradient: ";
  os << m_LowerGradient << std::endl;
  os << indent << "UpperGradient: ";
  os << m_UpperGradient << std::endl;
}

/*
  *
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
void
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::SetReferenceImage(const InputImageType *reference)
{
  this->ProcessObject::SetNthInput( 1,
                                    const_cast< InputImageType * >( reference ) );
}

/*
  *
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
const typename OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::InputImageType *
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::GetReferenceImage()
{
  if ( this->GetNumberOfInputs() < 2 )
    {
    return NULL;
    }

  return dynamic_cast< TInputImage * >(
           this->ProcessObject::GetInput(1) );
}

/*
  * This filter requires all of the input images to be
  * in the buffer.
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
void
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::GenerateInputRequestedRegion()
{
  this->Superclass::GenerateInputRequestedRegion();

  for ( unsigned int idx = 0; idx < this->GetNumberOfInputs(); ++idx )
    {
    if ( this->GetInput(idx) )
      {
      InputImagePointer image =
        const_cast< InputImageType * >( this->GetInput(idx) );
      image->SetRequestedRegionToLargestPossibleRegion();
      }
    }
}

/**
  *
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
void
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::BeforeThreadedGenerateData()
{
  unsigned int j;

  InputImageConstPointer source    = this->GetSourceImage();
  InputImageConstPointer reference = this->GetReferenceImage();

  this->ComputeMinMaxMean(source, m_SourceMinValue,
                          m_SourceMaxValue, m_SourceMeanValue);
  this->ComputeMinMaxMean(reference, m_ReferenceMinValue,
                          m_ReferenceMaxValue, m_ReferenceMeanValue);

  if ( m_ThresholdAtMeanIntensity )
    {
      {
      typedef OtsuThresholdImageCalculator< TInputImage > OtsuImageCalcType;
      typename OtsuImageCalcType::Pointer OtsuImageCalc = OtsuImageCalcType::New();
      OtsuImageCalc->SetImage(source);
      OtsuImageCalc->Compute();
      m_SourceIntensityThreshold    = static_cast< InputPixelType >( OtsuImageCalc->GetThreshold() );
      }

      {
      typedef OtsuThresholdImageCalculator< TInputImage > OtsuImageCalcType;
      typename OtsuImageCalcType::Pointer OtsuImageCalc = OtsuImageCalcType::New();
      OtsuImageCalc->SetImage(reference);
      OtsuImageCalc->Compute();
      m_ReferenceIntensityThreshold = static_cast< InputPixelType >( OtsuImageCalc->GetThreshold() );
      }
    }
  else
    {
    m_SourceIntensityThreshold    = static_cast< InputPixelType >( m_SourceMinValue );
    m_ReferenceIntensityThreshold = static_cast< InputPixelType >( m_ReferenceMinValue );
    }

  this->ConstructHistogram(source, m_SourceMask, m_SourceHistogram,
                           m_SourceIntensityThreshold, m_SourceMaxValue);
  this->ConstructHistogram(reference, m_ReferenceMask, m_ReferenceHistogram,
                           m_ReferenceIntensityThreshold,
                           m_ReferenceMaxValue);

  // Fill in the quantile table.
  m_QuantileTable.set_size(3, m_NumberOfMatchPoints + 2);
  m_QuantileTable[0][0] = m_SourceIntensityThreshold;
  m_QuantileTable[1][0] = m_ReferenceIntensityThreshold;

  m_QuantileTable[0][m_NumberOfMatchPoints + 1] = m_SourceMaxValue;
  m_QuantileTable[1][m_NumberOfMatchPoints + 1] = m_ReferenceMaxValue;

  double delta = 1.0 / ( double(m_NumberOfMatchPoints) + 1.0 );

  for ( j = 1; j < m_NumberOfMatchPoints + 1; j++ )
    {
    m_QuantileTable[0][j] = m_SourceHistogram->Quantile(
      0, double(j) * delta);
    m_QuantileTable[1][j] = m_ReferenceHistogram->Quantile(
      0, double(j) * delta);
    }

  // Fill in the gradient array.
  m_Gradients.set_size(m_NumberOfMatchPoints + 1);
  double denominator;
  for ( j = 0; j < m_NumberOfMatchPoints + 1; j++ )
    {
    denominator = m_QuantileTable[0][j + 1]
                  - m_QuantileTable[0][j];
    if ( denominator != 0 )
      {
      m_Gradients[j] = m_QuantileTable[1][j + 1]
                       - m_QuantileTable[1][j];
      m_Gradients[j] /= denominator;
      }
    else
      {
      m_Gradients[j] = 0.0;
      }
    }

  denominator = m_QuantileTable[0][0] - m_SourceMinValue;
  if ( denominator != 0 )
    {
    m_LowerGradient = m_QuantileTable[1][0] - m_ReferenceMinValue;
    m_LowerGradient /= denominator;
    }
  else
    {
    m_LowerGradient = 0.0;
    }

  denominator = m_QuantileTable[0][m_NumberOfMatchPoints + 1]
                - m_SourceMaxValue;
  if ( denominator != 0 )
    {
    m_UpperGradient = m_QuantileTable[1][m_NumberOfMatchPoints + 1]
                      - m_ReferenceMaxValue;
    m_UpperGradient /= denominator;
    }
  else
    {
    m_UpperGradient = 0.0;
    }
}

/**
  *
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
void
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::AfterThreadedGenerateData()
{
  OutputImagePointer output    = this->GetOutput();

  this->ComputeMinMaxMean(output, m_OutputMinValue,
                          m_OutputMaxValue, m_OutputMeanValue);

  if ( m_ThresholdAtMeanIntensity )
    {
    m_OutputIntensityThreshold    = static_cast< OutputPixelType >( m_OutputMeanValue );
    }
  else
    {
    m_OutputIntensityThreshold    = static_cast< OutputPixelType >( m_OutputMinValue );
    }

  this->ConstructHistogram(output, m_SourceMask, m_OutputHistogram,
                           m_OutputIntensityThreshold, m_OutputMaxValue);

  // Fill in the quantile table.
  m_QuantileTable[2][0] = m_OutputIntensityThreshold;

  m_QuantileTable[2][m_NumberOfMatchPoints + 1] = m_OutputMaxValue;

  double delta = 1.0 / ( double(m_NumberOfMatchPoints) + 1.0 );

  for ( unsigned int j = 1; j < m_NumberOfMatchPoints + 1; j++ )
    {
    m_QuantileTable[2][j] = m_OutputHistogram->Quantile(
      0, double(j) * delta);
    }
}

/**
  *
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
void
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       int threadId)
{
  int          i;
  unsigned int j;

  // Get the input and output pointers;
  InputImageConstPointer input  = this->GetInput();
  OutputImagePointer     output = this->GetOutput();

  // Transform the source image and write to output.
  typedef ImageRegionConstIterator< InputImageType > InputConstIterator;
  typedef ImageRegionIterator< OutputImageType >     OutputIterator;

  InputConstIterator inIter(input, outputRegionForThread);

  OutputIterator outIter(output, outputRegionForThread);

  // support progress methods/callbacks
  unsigned long updateVisits = 0;
  unsigned long totalPixels = 0;
  if ( threadId == 0 )
    {
    totalPixels = outputRegionForThread.GetNumberOfPixels();
    updateVisits = totalPixels / 10;
    if ( updateVisits < 1 ) { updateVisits = 1; }
    }

  double srcValue, mappedValue;

  for ( i = 0; !outIter.IsAtEnd(); ++inIter, ++outIter, i++ )
    {
    if ( threadId == 0 && !( i % updateVisits ) )
      {
      this->UpdateProgress( (float)i / (float)totalPixels );
      }

    srcValue = static_cast< double >( inIter.Get() );

    for ( j = 0; j < m_NumberOfMatchPoints + 2; j++ )
      {
      if ( srcValue < m_QuantileTable[0][j] )
        {
        break;
        }
      }

    if ( j == 0 )
      {
      // Linear interpolate from min to point[0]
      mappedValue = m_ReferenceMinValue
                    + ( srcValue - m_SourceMinValue ) * m_LowerGradient;
      }
    else if ( j == m_NumberOfMatchPoints + 2 )
      {
      // Linear interpolate from point[m_NumberOfMatchPoints+1] to max
      mappedValue = m_ReferenceMaxValue
                    + ( srcValue - m_SourceMaxValue ) * m_UpperGradient;
      }
    else
      {
      // Linear interpolate from point[j] and point[j+1].
      mappedValue = m_QuantileTable[1][j - 1]
                    + ( srcValue - m_QuantileTable[0][j - 1] ) * m_Gradients[j - 1];
      }

    outIter.Set( static_cast< OutputPixelType >( mappedValue ) );
    }
}

/**
  * Compute min, max and mean of an image.
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
void
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::ComputeMinMaxMean(
  const InputImageType *image,
  THistogramMeasurement & minValue,
  THistogramMeasurement & maxValue,
  THistogramMeasurement & meanValue)
{
  typedef ImageRegionConstIterator< InputImageType > ConstIterator;
  ConstIterator iter( image, image->GetBufferedRegion() );

  double   sum = 0.0;
  long int count = 0;

  minValue = static_cast< THistogramMeasurement >( iter.Get() );
  maxValue = minValue;

  while ( !iter.IsAtEnd() )
    {
    const THistogramMeasurement value = static_cast< THistogramMeasurement >( iter.Get() );
    sum += static_cast< double >( value );

    if ( value < minValue ) { minValue = value; }
    if ( value > maxValue ) { maxValue = value; }

    ++iter;
    ++count;
    }

  meanValue = static_cast< THistogramMeasurement >( sum / static_cast< double >( count ) );
}

/**
  * Construct a histogram from an image.
  */
template< class TInputImage, class TOutputImage, class THistogramMeasurement >
void
OtsuHistogramMatchingImageFilter< TInputImage, TOutputImage, THistogramMeasurement >
::ConstructHistogram(
  const InputImageType *image,
  const MaskImageType::Pointer mask,
  // HACK:  This should really be an
 // itkSpatialObject, it is currently
 // hardcoded to itk::Image<unsigned char, 3>
 // Find similar conversion code in BRAINSFitHelper.cxx
  OtsuHistogramType  *histogram,
  const THistogramMeasurement minValue,
  const THistogramMeasurement maxValue)
{
    {
    // allocate memory for the histogram
    typename OtsuHistogramType::SizeType size;
    typename OtsuHistogramType::MeasurementVectorType lowerBound;
    typename OtsuHistogramType::MeasurementVectorType upperBound;

    size.SetSize(1);
    lowerBound.SetSize(1);
    upperBound.SetSize(1);
    histogram->SetMeasurementVectorSize(1);
    size[0] = m_NumberOfHistogramLevels;
    lowerBound.Fill(minValue);
    upperBound.Fill(maxValue);

    // Initialize with equally spaced bins.
    histogram->Initialize(size, lowerBound, upperBound);
    histogram->SetToZero();
    }

  typename OtsuHistogramType::MeasurementVectorType measurement;
  measurement.SetSize(1);

  typedef typename OtsuHistogramType::MeasurementType MeasurementType;
  measurement[0] = NumericTraits< MeasurementType >::Zero;

    {
    // put each image pixel into the histogram
    typedef ImageRegionConstIteratorWithIndex< InputImageType > ConstIterator;
    ConstIterator iter( image, image->GetBufferedRegion() );

    iter.GoToBegin();
    while ( !iter.IsAtEnd() )
      {
      InputPixelType value = iter.Get();
      bool           inMeasurementRegion = false;
      if ( static_cast< double >( value ) >= minValue
           && static_cast< double >( value ) <= maxValue )
        {
        if ( mask.IsNull() ) // Assume entire area is valid
          {
          inMeasurementRegion = true;
          }
        else
          {
          typename TInputImage::PointType physicalPoint;
          image->TransformIndexToPhysicalPoint(iter.GetIndex(), physicalPoint);
          typename MaskImageType::IndexType maskIndex;
          const bool validRange = mask->TransformPhysicalPointToIndex(physicalPoint, maskIndex);
          if ( validRange && ( mask->GetPixel(maskIndex) > 0 ) )
            {
            inMeasurementRegion = true;
            }
          }
        }
      if ( inMeasurementRegion == true )
        {
        // add sample to histogram
        measurement[0] = value;
#if  ITK_VERSION_MAJOR >=4
        histogram->IncreaseFrequencyOfMeasurement(measurement, 1.0F);
#else
        histogram->IncreaseFrequency(measurement, 1.0F);
#endif
        }
      ++iter;
      }
    }
}
} // end namespace itk

#endif
