/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkComputeStatisticsWherePositiveFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006/01/11 19:43:31 $
  Version:   $Revision: 1.14 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkComputeStatisticsWherePositiveFilter_txx
#define _itkComputeStatisticsWherePositiveFilter_txx
#include "itkComputeStatisticsWherePositiveFilter.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNumericTraits.h"

namespace itk
{

template <class TInputImage, class TOutputImage>
ComputeStatisticsWherePositiveFilter<TInputImage, TOutputImage>
::ComputeStatisticsWherePositiveFilter()
{
  m_Ready = false;
}

template <class TInputImage, class TOutputImage>
void ComputeStatisticsWherePositiveFilter<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadId )
{
  // Allocate images:
  typename OutputImageType::Pointer     output = this->GetOutput();
  typename InputImageType::ConstPointer input  = this->GetInput();
  // Iterators:
  itk::ImageRegionConstIterator<InputImageType> bit = itk::ImageRegionConstIterator<InputImageType>(
      input, outputRegionForThread );
  itk::ImageRegionIterator<OutputImageType> it  = itk::ImageRegionIterator<OutputImageType>( output,
                                                                                             outputRegionForThread );
  // Initiallise max and min:
  for( bit.GoToBegin(), it.GoToBegin(); !bit.IsAtEnd(); ++bit, ++it )
    {
    double current = bit.Get();
    if( current >= 0 )
      {
      m_TMean[threadId] += current;
      m_TStd[threadId]  += current * current;
      if( current > m_TMax[threadId] )
        {
        m_TMax[threadId] = current;
        }
      if( current < m_TMin[threadId] )
        {
        m_TMin[threadId] = current;
        }
      m_TCount[threadId]++;
      }
    it.Set( static_cast<OutputPixelType>(current) );
    }
}

template <class TInputImage, class TOutputImage>
void ComputeStatisticsWherePositiveFilter<TInputImage, TOutputImage>
::BeforeThreadedGenerateData()
{
  this->Superclass::BeforeThreadedGenerateData();
  unsigned int k = this->GetNumberOfThreads();
  m_TMean.SetSize( k );
  m_TStd.SetSize( k );
  m_TMin.SetSize( k );
  m_TMax.SetSize( k );
  m_TCount.SetSize( k );
  m_TMean.Fill( itk::NumericTraits<double>::ZeroValue() );
  m_TStd.Fill( itk::NumericTraits<double>::ZeroValue() );
  m_TMin.Fill( itk::NumericTraits<double>::max() );
  m_TMax.Fill( itk::NumericTraits<double>::min() );
  m_TCount.Fill( 0 );
}

template <class TInputImage, class TOutputImage>
void ComputeStatisticsWherePositiveFilter<TInputImage, TOutputImage>
::AfterThreadedGenerateData()
{
  unsigned int k = this->GetNumberOfThreads();

  m_Mean = itk::NumericTraits<double>::ZeroValue();
  m_Std  = itk::NumericTraits<double>::ZeroValue();
  m_Min  = itk::NumericTraits<double>::max();
  m_Max  = itk::NumericTraits<double>::min();
  unsigned long count = 0;
  for( unsigned int l = 0; l < k; ++l )
    {
    count  += m_TCount[l];
    m_Mean += m_TMean[l];
    m_Std  += m_TStd[l];
    if( m_TMin[l] < m_Min )
      {
      m_Min = m_TMin[l];
      }
    if( m_TMax[l] > m_Max )
      {
      m_Max = m_TMax[l];
      }
    }
  if( count > 0 )
    {
    m_Mean /= (double)count;
    m_Std  /= (double)count;
    m_Std  = m_Std - m_Min * m_Min;
    if( m_Std >= 0 )
      {
      m_Std = sqrt( m_Std );
      }
    else
      {
      m_Std = itk::NumericTraits<double>::ZeroValue();
      }
    m_Ready = true;
    }
  this->Superclass::AfterThreadedGenerateData();
}

} // end namespace itk

#endif
