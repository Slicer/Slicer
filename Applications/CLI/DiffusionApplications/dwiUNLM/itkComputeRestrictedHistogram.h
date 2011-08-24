/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkComputeRestrictedHistogram.h,v $
  Language:  C++
  Date:      $Date: 2006/03/29 14:53:40 $
  Version:   $Revision: 1.5 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkComputeRestrictedHistogram_h
#define __itkComputeRestrictedHistogram_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkArray2D.h"
#include "itkArray.h"

namespace itk
{
/** \class ComputeRestrictedHistogram
 * \brief Compute mean, std, min, and max of positive pixels
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT ComputeRestrictedHistogram : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef ComputeRestrictedHistogram                          Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( ComputeRestrictedHistogram, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType               InputPixelType;
  typedef typename OutputImageType::PixelType              OutputPixelType;
  typedef typename NumericTraits<InputPixelType>::RealType InputRealType;
  typedef typename InputImageType::RegionType              InputImageRegionType;
  typedef typename OutputImageType::RegionType             OutputImageRegionType;
  typedef typename InputImageType::SizeType                InputSizeType;

  /** The type to store per thread values */
  typedef itk::Array2D<unsigned long> ThreadHistogramType;
  typedef itk::Array<unsigned long>   HistogramType;

  /** Parameters */
  itkGetConstReferenceMacro( Min, double );
  itkGetConstReferenceMacro( Max, double );
  itkSetMacro( Min, double );
  itkSetMacro( Max, double );
  itkGetConstReferenceMacro( Bins, unsigned int );
  itkSetMacro( Bins, unsigned int );
  itkGetConstReferenceMacro( Mode, double );
  itkGetConstReferenceMacro( Ready, bool );
  itkGetConstReferenceMacro( Histogram, HistogramType );
protected:
  ComputeRestrictedHistogram();
  virtual ~ComputeRestrictedHistogram()
  {
  }
#if ITK_VERSION_MAJOR < 4
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, int threadId );

#else
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId );

#endif
  // Override Modified() method to fix the ready variable:
  void Modified()
  {
    this->Superclass::Modified();
    m_Ready = false;
  }

  using itk::Object::Modified;
  void BeforeThreadedGenerateData();

  void AfterThreadedGenerateData();

private:
  ComputeRestrictedHistogram(const Self &); // purposely not implemented
  void operator=(const Self &);             // purposely not implemented

  // Per-thread histogram:
  ThreadHistogramType m_THist;
  // Complete histogram:
  HistogramType m_Histogram;

  // Parameters:
  double       m_Min;
  double       m_Max;
  unsigned int m_Bins;

  // Mode of the distribution value:
  double m_Mode;

  // Are the data already computed?
  bool m_Ready;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkComputeRestrictedHistogram.txx"
#endif

#endif
