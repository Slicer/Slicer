/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkOtsuStatistics.h,v $
  Language:  C++
  Date:      $Date: 2008/02/7 14:28:51 $
  Version:   $Revision: 0.0 $
=========================================================================*/
#ifndef __itkOtsuThreshold_h
#define __itkOtsuThreshold_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkArray.h"
#include "itkArray2D.h"

namespace itk
{
/** \class OtsuThreshold */

template <class TInputImage, class TOutputImage>
class ITK_EXPORT OtsuThreshold : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage                            InputImageType;
  typedef typename InputImageType::Pointer       InputImagePointer;
  typedef typename InputImageType::ConstPointer  InputImageConstPointer;
  typedef TOutputImage                           OutputImageType;
  typedef typename OutputImageType::Pointer      OutputImagePointer;
  typedef typename OutputImageType::ConstPointer OutputImageConstPointer;

  /** Standard class typedefs. */
  typedef OtsuThreshold                                       Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( OtsuThreshold, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  /** Set and get the number of DWI channels. */
  itkGetMacro( Threshold, double );
  itkGetMacro( W, double );
  itkSetMacro( W, double );
  itkGetMacro( Min, double );
  itkSetMacro( Min, double );
  itkGetMacro( Max, double );
  itkSetMacro( Max, double );
  itkGetMacro( Bins, unsigned int );
  itkSetMacro( Bins, unsigned int );
protected:
  OtsuThreshold();
  virtual ~OtsuThreshold()
  {
  }
  void PrintSelf(std::ostream& os, Indent indent) const;

  // Threaded filter!
  void BeforeThreadedGenerateData( void );

#if ITK_VERSION_MAJOR < 4
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, int threadId );

#else
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId );

#endif
  void AfterThreadedGenerateData( void );

private:
  OtsuThreshold(const Self &);   // purposely not implemented
  void operator=(const Self &);  // purposely not implemented

  double       m_W;
  double       m_Min;
  double       m_Max;
  unsigned int m_Bins;
  // Per thread histograms:
  itk::Array2D<double> m_ThreadHist;
  // itk::Array<double> m_ThreadMax;
  itk::Array<double> m_ThreadCount;
  // Final Threshold
  double m_Threshold;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOtsuThreshold.txx"
#endif

#endif
