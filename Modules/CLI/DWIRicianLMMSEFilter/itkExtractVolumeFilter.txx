/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkExtractVolumeFilter.txx,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkExtractVolumeFilter_txx
#define _itkExtractVolumeFilter_txx
#include "itkExtractVolumeFilter.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace itk
{

template <class TInputImage, class TOutputImage>
void ExtractVolumeFilter<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType itkNotUsed(threadId) )
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
    double current = m_Factor * static_cast<double>( bit.Get()[m_VolumeNr] );
    it.Set( static_cast<OutputPixelType>(current) );
    }
}

} // end namespace itk

#endif
