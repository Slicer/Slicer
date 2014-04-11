/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkComputeRestrictedHistogram.txx,v $
  Language:  C++
  Date:      $Date: 2006/01/11 19:43:31 $
  Version:   $Revision: 1.14 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkComputeRestrictedHistogram_txx
#define _itkComputeRestrictedHistogram_txx
#include "itkComputeRestrictedHistogram.h"

#include "itkImageRegionConstIterator.h"
#include "itkNumericTraits.h"

namespace itk
{

template <class TInputImage, class TOutputImage>
ComputeRestrictedHistogram<TInputImage, TOutputImage>
::ComputeRestrictedHistogram()
{
  m_Ready = false;
  m_Min   = 0.0f;
  m_Max   = 5.0f;
  m_Bins  = 256;
  m_Mode  = 0.0f;
}

template <class TInputImage, class TOutputImage>
void ComputeRestrictedHistogram<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId )
{
  // Allocate input:
  typename InputImageType::ConstPointer input  = this->GetInput();
  // Iterators:
  itk::ImageRegionConstIterator<InputImageType> bit = itk::ImageRegionConstIterator<InputImageType>(
      input, outputRegionForThread );
  // The scale factor to compute histogram position:
  double scale = ( (double)m_Bins ) / ( m_Max - m_Min );
  // Initiallise max and min:
  for( bit.GoToBegin(); !bit.IsAtEnd(); ++bit )
    {
    double current = bit.Get();
    if(   ( current >= m_Min ) && ( current < m_Max)   )
      {
      unsigned int pos = (unsigned int)(   ( current - m_Min ) * scale   );
      m_THist[threadId][pos]++;
      }
    }
}

template <class TInputImage, class TOutputImage>
void ComputeRestrictedHistogram<TInputImage, TOutputImage>
::BeforeThreadedGenerateData()
{
  this->Superclass::BeforeThreadedGenerateData();
  unsigned int k = this->GetNumberOfThreads();
  m_THist.SetSize( k, m_Bins );
  m_THist.Fill( 0 );
}

template <class TInputImage, class TOutputImage>
void ComputeRestrictedHistogram<TInputImage, TOutputImage>
::AfterThreadedGenerateData()
{
  unsigned int  k = this->GetNumberOfThreads();
  unsigned long max    = 0;
  unsigned int  maxPos = 0;

  m_Histogram.SetSize( m_Bins );
  for( unsigned int i = 0; i < m_Bins; ++i )
    {
    m_Histogram[i] = m_THist[0][i];
    for( unsigned int j = 1; j < k; ++j )
      {
      m_Histogram[i] += m_THist[j][i];
      }
    if( m_Histogram[i] > max )
      {
      max    = m_Histogram[i];
      maxPos = i;
      }
    }
  m_Mode  = m_Min + ( (double)maxPos + 0.5f ) * ( m_Max - m_Min ) / ( (double)m_Bins );
  m_Ready = true;
  this->Superclass::AfterThreadedGenerateData();
}

} // end namespace itk

#endif
