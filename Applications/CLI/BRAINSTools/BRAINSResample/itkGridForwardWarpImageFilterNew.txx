/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkGridForwardWarpImageFilterNew.txx,v $
 *  Language:  C++
 *  Date:      $Date: 2009-10-27 18:12:48 $
 *  Version:   $Revision: 1.7 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *    This software is distributed WITHOUT ANY WARRANTY; without even
 *    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *    PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

#ifndef __itkGridForwardWarpImageFilterNew_txx
#define __itkGridForwardWarpImageFilterNew_txx

#include "itkGridForwardWarpImageFilterNew.h"

#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"
#include "itkNumericTraits.h"
#include "itkProgressReporter.h"
#include "itkLineIterator.h"

namespace itk
{
/**
  * Default constructor.
  */
template< class TDeformationField, class TOutputImage >
GridForwardWarpImageFilterNew< TDeformationField, TOutputImage >
::GridForwardWarpImageFilterNew():
  m_BackgroundValue (NumericTraits< PixelType >::Zero),
  m_ForegroundValue (NumericTraits< PixelType >::One)
{
  // Setup default values
  for ( unsigned int q = 0; q < ImageDimension; q++ )
    {
    m_GridPixelSpacing[q] = 10; // Old default was 5
    }
}

/**
  * Standard PrintSelf method.
  */
template< class TDeformationField, class TOutputImage >
void
GridForwardWarpImageFilterNew< TDeformationField, TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "BackgroundValue: "
     << static_cast< typename NumericTraits< PixelType >::PrintType >( m_BackgroundValue )
     << std::endl;
  os << indent << "ForegroundValue: "
     << static_cast< typename NumericTraits< PixelType >::PrintType >( m_ForegroundValue )
     << std::endl;
}

/**
  * Compute the output for the region specified by outputRegionForThread.
  */
template< class TDeformationField, class TOutputImage >
void
GridForwardWarpImageFilterNew< TDeformationField, TOutputImage >
::GenerateData()
{
  OutputImagePointer           outputPtr = this->GetOutput();
  DeformationFieldConstPointer fieldPtr = this->GetInput();

  SpacingType spacing = fieldPtr->GetSpacing();

  outputPtr->SetRegions( fieldPtr->GetRequestedRegion() );
  outputPtr->CopyInformation(fieldPtr);
  outputPtr->Allocate();
  outputPtr->FillBuffer(m_BackgroundValue);

  IndexType FirstIndex = fieldPtr->GetRequestedRegion().GetIndex();
  IndexType OnePastValidIndex = fieldPtr->GetRequestedRegion().GetIndex()
                                + fieldPtr->GetRequestedRegion().GetSize();

  // iterator for the output image
  typedef ImageRegionIteratorWithIndex< OutputImageType > OutputImageIteratorWithIndex;
  OutputImageIteratorWithIndex iter( outputPtr, outputPtr->GetRequestedRegion() );

  // iterator for the deformation field
  typedef ImageRegionConstIterator< DeformationFieldType > DeformationFieldIterator;
  DeformationFieldIterator fieldIt( fieldPtr, outputPtr->GetRequestedRegion() );

  // Bresenham line iterator
  typedef LineIterator< OutputImageType > LineIteratorType;

  typedef typename IndexType::IndexValueType IndexValueType;

  IndexType index;
  IndexType refIndex;
  IndexType targetIndex;
  // ContinuousIndex<float, ImageDimension> contindex;
  unsigned int nonZeroGridDirections = 0;
  for ( int q = 0; q < ImageDimension; q++ )
    {
    if ( m_GridPixelSpacing[q] != 0 )
      {
      nonZeroGridDirections++;
      }
    }

  for ( iter.GoToBegin(), fieldIt.GoToBegin(); !iter.IsAtEnd(); ++iter, ++fieldIt )
    {
    index = iter.GetIndex();

    unsigned int numGridIntersect = 0;
    for ( unsigned int dim = 0; dim < ImageDimension; dim++ )
      {
      numGridIntersect +=
        ( ( m_GridPixelSpacing[dim] != 0 ) && ( ( index[dim] % vcl_abs(m_GridPixelSpacing[dim]) ) == 0 ) );
      }
    if ( numGridIntersect == nonZeroGridDirections ) // else do nothing!
      {
      // we are on a grid refPoint => transform it
      typename TOutputImage::PointType refPoint;
      outputPtr->TransformIndexToPhysicalPoint(index, refPoint);
      // compute the mapped refPoint
        {
        // get the required displacement
        DisplacementType displacement = fieldIt.Get();
        for ( unsigned int j = 0; j < ImageDimension; j++ )
          {
          if ( m_GridPixelSpacing[j] != 0 ) // Do not compute offsets for
          // collapsed dimensions
            {
            refPoint[j] += displacement[j];
            }
          // else refPoint[j]=refPoint[j];
          }
        }
      const bool inside = outputPtr->TransformPhysicalPointToIndex(refPoint, refIndex);
      if ( inside )
        {
        // We know the current grid refPoint is inside
        // we will check if the grid points that are above are also inside
        // In such a case we draw a Bresenham line
        for ( unsigned int dim = 0; dim < ImageDimension; dim++ )
          {
          if ( m_GridPixelSpacing[dim] <= 0 ) // Don't do invisible direction
            {
            // targetIndex[dim]=targetIndex[dim];//Leave as same value
            continue;
            }
          targetIndex = index;
          targetIndex[dim] += m_GridPixelSpacing[dim]; // For non-collapsed
                                                       // dimension.
          // compute the mapped targetPoint
          typename TOutputImage::PointType targetPoint;
          outputPtr->TransformIndexToPhysicalPoint(targetIndex, targetPoint);
            {
            // get the required targetDisplacement
            DisplacementType targetDisplacement = fieldPtr->GetPixel(targetIndex);
            for ( unsigned int j = 0; j < ImageDimension; j++ )
              {
              if ( m_GridPixelSpacing[j] != 0 ) // Do not compute offsets for
              // collapsed dimensions
                {
                targetPoint[j] += targetDisplacement[j];
                }
              // else targetPoint[j]=targetPoint[j];
              }
            }
          const bool targetIn = outputPtr->TransformPhysicalPointToIndex(targetPoint, targetIndex);
          if ( targetIn )
            {
            for ( LineIteratorType lineIter(outputPtr, refIndex, targetIndex);
                  !lineIter.IsAtEnd(); ++lineIter )
              {
#if 0
              typename TOutputImage::IndexType testIndex = lineIter.GetIndex();
              bool lineIsInside = true;
              for ( unsigned int j = 0; j < ImageDimension; j++ )
                {
                if ( ( testIndex[j] < FirstIndex[j] ) || ( testIndex[j] >= OnePastValidIndex[j] ) )
                  {
                  lineIsInside = false;
                  break;
                  }
                }
              if ( lineIsInside )
#endif
                {
                lineIter.Set(m_ForegroundValue);
                }
              }
            }
          } // end for loop for radiating lines in each direction
        }
      }
    }
  // ProgressReporter progress(this, 0, numiter+1, numiter+1);
}
} // end namespace itk

#endif
