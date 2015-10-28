/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkComputeStatisticsWherePositiveFilter.h,v $
  Language:  C++
  Date:      $Date: 2006/03/29 14:53:40 $
  Version:   $Revision: 1.5 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkComputeStatisticsWherePositiveFilter_h
#define __itkComputeStatisticsWherePositiveFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkArray.h"

namespace itk
{
/** \class ComputeStatisticsWherePositiveFilter
 * \brief Compute mean, std, min, and max of positive pixels
 */
template <class TInputImage, class TOutputImage>
class ComputeStatisticsWherePositiveFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef ComputeStatisticsWherePositiveFilter                Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( ComputeStatisticsWherePositiveFilter, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType               InputPixelType;
  typedef typename OutputImageType::PixelType              OutputPixelType;
  typedef typename NumericTraits<InputPixelType>::RealType InputRealType;
  typedef typename InputImageType::RegionType              InputImageRegionType;
  typedef typename OutputImageType::RegionType             OutputImageRegionType;
  typedef typename InputImageType::SizeType                InputSizeType;

  /** The type to store per thread values */
  typedef itk::Array<double>        StoreType;
  typedef itk::Array<unsigned long> CountType;

  /** Get computed statistics */
  itkGetMacro( Mean, double );
  itkGetMacro( Std, double );
  itkGetMacro( Min, double );
  itkGetMacro( Max, double );
  itkGetMacro( Ready, bool );
protected:
  ComputeStatisticsWherePositiveFilter();
  virtual ~ComputeStatisticsWherePositiveFilter()
  {
  }

  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;

  // Override Modified() method to fix the ready variable:
  void Modified()
  {
    this->Superclass::Modified();
    m_Ready = false;
  }

  using itk::Object::Modified;
  void BeforeThreadedGenerateData() ITK_OVERRIDE;

  void AfterThreadedGenerateData() ITK_OVERRIDE;

private:
  ComputeStatisticsWherePositiveFilter(const Self &); // purposely not implemented
  void operator=(const Self &);                       // purposely not implemented

  // Per-thread values
  StoreType m_TMean;
  StoreType m_TStd;
  StoreType m_TMin;
  StoreType m_TMax;
  CountType m_TCount;

  // Overall values:
  double m_Mean;
  double m_Std;
  double m_Min;
  double m_Max;

  // Are the data already computed?
  bool m_Ready;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkComputeStatisticsWherePositiveFilter.txx"
#endif

#endif
