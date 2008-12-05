/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxAccumulator.h,v $
Language:  C++
Date:      $Date: 2008/08/03 20:20:24 $
Version:   $Revision: 1.1 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

The following bit of code was copied from the the Insight Segmentation & 
Registration Toolkit. It is currently in the review directory as part
of the projection filters. This code can be eliminated once this makes
it way into ITK proper.
=========================================================================*/

/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMimxAccumulator.h,v $
  Language:  C++
  Date:      $Date: 2008/08/03 20:20:24 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkMimxAccumulator_h
#define __itkMimxAccumulator_h

#include "vtkMimxFilterWin32Header.h"

namespace mimxFunction {

  /*********** Mean Accumulator Function ***********/
  template <class TInputPixel, class TAccumulate>
  class MeanAccumulator
  {
  public:
    typedef typename itk::NumericTraits<TInputPixel>::RealType RealType;

    MeanAccumulator( unsigned long size )
      {
      m_Size = size;
      }
    ~MeanAccumulator(){}

    inline void Initialize()
      {
      m_Sum = itk::NumericTraits< TAccumulate >::Zero;
      }

    inline void operator()( const TInputPixel &input )
      {
      m_Sum = m_Sum + input;
      }

    inline RealType GetValue()
      {
      return ((RealType) m_Sum) / m_Size;
      }

    TAccumulate   m_Sum;
    unsigned long m_Size;
  };

  /*********** Maximum Accumulator Function ***********/
  template <class TInputPixel>
  class MaximumAccumulator
  {
  public:
    MaximumAccumulator( unsigned long ) {}
    ~MaximumAccumulator(){}

    inline void Initialize()
      {
      m_Maximum = itk::NumericTraits< TInputPixel >::NonpositiveMin();
      }

    inline void operator()( const TInputPixel &input )
      {
      m_Maximum = vnl_math_max( m_Maximum, input );
      }

    inline TInputPixel GetValue()
      {
      return m_Maximum;
      }

    TInputPixel m_Maximum;
  };

  /*********** Median Accumulator Function ***********/
  template <class TInputPixel>
  class MedianAccumulator
  {
  public:
    MedianAccumulator( unsigned long size )
      {
      m_Values.reserve( size );
      }
    ~MedianAccumulator(){}

    inline void Initialize()
      {
      m_Values.clear();
      }

    inline void operator()( const TInputPixel &input )
      {
      m_Values.push_back( input );
      }

    inline TInputPixel GetValue()
      {
      typedef typename std::vector<TInputPixel>::iterator ContainerIterator;
      ContainerIterator medianIterator = m_Values.begin() +  m_Values.size() / 2;
      std::nth_element(m_Values.begin(), medianIterator, m_Values.end() );
      return *medianIterator;
      }

    std::vector<TInputPixel> m_Values;
  };
} // end namespace mimxFunction

#endif

