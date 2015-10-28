/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkExtractVolumeFilter.h,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkExtractVolumeFilter_h
#define __itkExtractVolumeFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"

namespace itk
{
/** \class ExtractVolumeFilter
 *
 * \sa Image
 */
template <class TInputImage, class TOutputImage>
class ExtractVolumeFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef ExtractVolumeFilter Self;

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage                       InputImageType;
  typedef typename InputImageType::Pointer  InputImagePointer;
  typedef TOutputImage                      OutputImageType;
  typedef typename OutputImageType::Pointer OutputImagePointer;

  /** Standard class typedefs. */
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( ExtractVolumeFilter, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType   InputPixelType;
  typedef typename OutputImageType::PixelType  OutputPixelType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  /** Set and get the parameters */
  itkSetMacro( VolumeNr, unsigned int );
  itkGetMacro( VolumeNr, unsigned int );
  itkSetMacro( Factor,   double       );
  itkGetMacro( Factor,   double       );
protected:
  ExtractVolumeFilter()
  {
    m_VolumeNr = 0;
    m_Factor   = 1.0f;
  }

  virtual ~ExtractVolumeFilter()
  {
  }
  // Threaded filter!
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;

private:
  ExtractVolumeFilter(const Self &);  // purposely not implemented
  void operator=(const Self &);       // purposely not implemented

  unsigned int m_VolumeNr;
  double       m_Factor;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkExtractVolumeFilter.txx"
#endif

#endif
