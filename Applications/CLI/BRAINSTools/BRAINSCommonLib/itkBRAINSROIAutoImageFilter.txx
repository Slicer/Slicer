/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkBRAINSROIAutoImageFilter.txx,v $
 *  Language:  C++
 *  Date:      $Date: 2008-10-16 16:45:09 $
 *  Version:   $Revision: 1.13 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *    This software is distributed WITHOUT ANY WARRANTY; without even
 *    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *    PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __itkBRAINSROIAutoImageFilter_txx
#define __itkBRAINSROIAutoImageFilter_txx
#include "itkBRAINSROIAutoImageFilter.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNumericTraits.h"
#include "itkProgressAccumulator.h"

namespace itk
{
template< class TInputImage, class TOutputImage >
BRAINSROIAutoImageFilter< TInputImage, TOutputImage >
::BRAINSROIAutoImageFilter()
{
  // this filter requires two input images
  this->SetNumberOfRequiredInputs(1);
  m_OtsuPercentileThreshold = 0.01;
  m_ClosingSize = 9.0;
  m_ThresholdCorrectionFactor = 1.0;
  m_DilateSize = 0.0;
  m_ResultMaskPointer = NULL;
}

template< class TInputImage, class TOutputImage >
void
BRAINSROIAutoImageFilter< TInputImage, TOutputImage >
::GenerateData()
{
  m_ResultMaskPointer = NULL; // Need to make this null during every re-run of
                              // the data.
  // Create a process accumulator for tracking the progress of this minipipeline
  ProgressAccumulator::Pointer progress = ProgressAccumulator::New();
  progress->SetMiniPipelineFilter(this);

  typedef itk::LargestForegroundFilledMaskImageFilter< TInputImage, TOutputImage > LFFMaskFilterType;
  typename LFFMaskFilterType::Pointer LFF = LFFMaskFilterType::New();
  // Register the filter with the with progress accumulator using
  // equal weight proportion
  progress->RegisterInternalFilter(LFF, 1.0f);
  LFF->SetInput( this->GetInput() );
  LFF->SetOtsuPercentileThreshold(m_OtsuPercentileThreshold);
  LFF->SetClosingSize(m_ClosingSize);
  LFF->SetDilateSize(m_DilateSize);
  LFF->SetThresholdCorrectionFactor(m_ThresholdCorrectionFactor);
  LFF->Update();
  this->GraftOutput( LFF->GetOutput() );
}

template< class TInputImage, class TOutputImage >
void
BRAINSROIAutoImageFilter< TInputImage, TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "OtsuPercentileThreshold: "
     << m_OtsuPercentileThreshold << std::endl;
  os << indent << "ThresholdCorrectionFactor: "
     << m_ThresholdCorrectionFactor << std::endl;
  os << indent << "ClosingSize: "
     << m_ClosingSize << std::endl;
  os << indent << "DilateSize: "
     << m_DilateSize << std::endl;
}
} // end namespace itk
#endif
