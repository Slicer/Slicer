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
  m_HistogramGenerator->SetInput( m_ImageToListSampleGenerator->GetOutput() );
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
  return m_HistogramGenerator->GetOutput();
}

template< class TImageType, class TMaskType >
void
ScalarImagePortionToHistogramGenerator< TImageType, TMaskType >
::Compute()
{
  m_ImageToListSampleGenerator->Update();
  std::cout << "ListSample TotalFrequency is  " << m_ImageToListSampleGenerator->GetOutput()->GetTotalFrequency()
            << std::endl;
  m_HistogramGenerator->Update();
}

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
