/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkNewOtsuThresholdImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006/03/16 19:59:56 $
  Version:   $Revision: 1.1.2.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef itkNewOtsuThresholdImageFilter_txx
#define itkNewOtsuThresholdImageFilter_txx
#include "itkNewOtsuThresholdImageFilter.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkNewOtsuThresholdImageCalculator.h"
#include "itkProgressAccumulator.h"

namespace itk {

template<class TInputImage, class TOutputImage>
NewOtsuThresholdImageFilter<TInputImage, TOutputImage>
::NewOtsuThresholdImageFilter()
{
  m_OutsideValue   = NumericTraits<OutputPixelType>::ZeroValue();
  m_InsideValue    = NumericTraits<OutputPixelType>::max();
  m_Threshold      = NumericTraits<InputPixelType>::ZeroValue();
  m_NumberOfHistogramBins = 128;
  m_Omega = 2;
}

template<class TInputImage, class TOutputImage>
void
NewOtsuThresholdImageFilter<TInputImage, TOutputImage>
::GenerateData()
{
  typename ProgressAccumulator::Pointer progress = ProgressAccumulator::New();
  progress->SetMiniPipelineFilter(this);

  // Compute the Otsu Threshold for the input image
  typename NewOtsuThresholdImageCalculator<TInputImage>::Pointer otsu =
    NewOtsuThresholdImageCalculator<TInputImage>::New();
  otsu->SetImage (this->GetInput());
  otsu->SetNumberOfHistogramBins (m_NumberOfHistogramBins);
  otsu->SetOmega(m_Omega);
  otsu->Compute();
  m_Threshold = otsu->GetThreshold();

  typename BinaryThresholdImageFilter<TInputImage,TOutputImage>::Pointer threshold =
    BinaryThresholdImageFilter<TInputImage,TOutputImage>::New();

  progress->RegisterInternalFilter(threshold,.5f);
  threshold->GraftOutput (this->GetOutput());
  threshold->SetInput (this->GetInput());
  threshold->SetLowerThreshold(NumericTraits<InputPixelType>::NonpositiveMin());
  threshold->SetUpperThreshold(otsu->GetThreshold());
  threshold->SetInsideValue (m_InsideValue);
  threshold->SetOutsideValue (m_OutsideValue);
  threshold->Update();

  this->GraftOutput(threshold->GetOutput());
}

template<class TInputImage, class TOutputImage>
void
NewOtsuThresholdImageFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion()
{
  const_cast<TInputImage *>(this->GetInput())->SetRequestedRegionToLargestPossibleRegion();
}

template<class TInputImage, class TOutputImage>
void
NewOtsuThresholdImageFilter<TInputImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "OutsideValue: "
     << static_cast<typename NumericTraits<OutputPixelType>::PrintType>(m_OutsideValue) << std::endl;
  os << indent << "InsideValue: "
     << static_cast<typename NumericTraits<OutputPixelType>::PrintType>(m_InsideValue) << std::endl;
  os << indent << "NumberOfHistogramBins: "
     << m_NumberOfHistogramBins << std::endl;
  os << indent << "Threshold (computed): "
     << static_cast<typename NumericTraits<InputPixelType>::PrintType>(m_Threshold) << std::endl;

}


}// end namespace itk

#endif
