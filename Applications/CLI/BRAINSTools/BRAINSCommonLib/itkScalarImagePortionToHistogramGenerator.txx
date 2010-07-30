/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkScalarImagePortionToHistogramGenerator.txx,v $
 *  Language:  C++
 *  Date:      $Date: 2009-08-17 18:29:01 $
 *  Version:   $Revision: 1.4 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *    This software is distributed WITHOUT ANY WARRANTY; without even
 *    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *    PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __itkScalarImagePortionToHistogramGenerator_txx
#define __itkScalarImagePortionToHistogramGenerator_txx

#include "itkScalarImagePortionToHistogramGenerator.h"

namespace itk
{
namespace Statistics
{
template< class TImageType, class TMaskType >
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::ScalarImagePortionToHistogramGenerator()
{
  m_ImageToListSampleGenerator = ListSampleGeneratorType::New();
  m_HistogramGenerator = GeneratorType::New();
#ifdef ITK_USE_REVIEW_STATISTICS
  m_HistogramGenerator->SetInput( m_ImageToListSampleGenerator->GetOutput() );
#else
  m_HistogramGenerator->SetListSample( m_ImageToListSampleGenerator->GetListSample() );
#endif
}

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::SetInput(const ImageType *image)
{
  m_ImageToListSampleGenerator->SetInput(image);
}

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::SetBinaryPortionImage(const TMaskType *binaryImage)
{
  m_ImageToListSampleGenerator->SetMaskImage(binaryImage);
  m_ImageToListSampleGenerator->SetMaskValue(NumericTraits< typename TMaskType::PixelType >::One);
}

template< class TImageType, class TMaskType >
const typename ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >::HistogramType *
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::GetOutput() const
{
#ifdef ITK_USE_REVIEW_STATISTICS
  return m_HistogramGenerator->GetOutput();
#else
  return m_Histogram;
#endif
}

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::Compute()
{
  m_ImageToListSampleGenerator->Update();
#ifdef ITK_USE_REVIEW_STATISTICS
  std::cout << "ListSample TotalFrequency is  " << m_ImageToListSampleGenerator->GetOutput()->GetTotalFrequency()
            << std::endl;
#endif
  m_HistogramGenerator->Update();
}

#ifdef ITK_USE_REVIEW_STATISTICS

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::SetNumberOfBins(unsigned int numberOfBins)
{
  typename HistogramType::SizeType size;

  size.SetSize(1);
  size.Fill(numberOfBins);
  m_HistogramGenerator->SetHistogramSize(size);
}

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::SetHistogramMin(RealPixelType minimumValue)
{
  typedef typename GeneratorType::HistogramMeasurementVectorType MeasurementVectorType;
  MeasurementVectorType minVector(1);

  minVector[0] = minimumValue;
  m_HistogramGenerator->SetHistogramBinMinimum(minVector);
}

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::SetHistogramMax(RealPixelType maximumValue)
{
  typedef typename GeneratorType::HistogramMeasurementVectorType MeasurementVectorType;
  MeasurementVectorType maxVector(1);

  maxVector[0] = maximumValue;
  m_HistogramGenerator->SetHistogramBinMaximum(maxVector);
}

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::SetMarginalScale(double marginalScale)
{
  m_HistogramGenerator->SetMarginalScale(marginalScale);
}

#else

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::InitializeHistogram(unsigned int numberOfBins, RealPixelType minimumValue, RealPixelType maximumValue)
{
  typedef typename HistogramType::MeasurementVectorType MeasurementVectorType;
  MeasurementVectorType minVector(1);

  minVector[0] = minimumValue;
  MeasurementVectorType maxVector(1);

  maxVector[0] = maximumValue;

  typedef typename HistogramType::SizeType SizeType;
  SizeType size;
  size.SetElement(0, numberOfBins);

  m_Histogram = HistogramType::New();
  m_Histogram->Initialize(size, minVector, maxVector);
  m_HistogramGenerator->SetHistogram(m_Histogram);
}

#endif

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << "ImageToListSampleGenerator = " << m_ImageToListSampleGenerator << std::endl;
  os << "HistogramGenerator = " << m_HistogramGenerator << std::endl;
}
} // end of namespace Statistics
} // end of namespace itk

#endif
