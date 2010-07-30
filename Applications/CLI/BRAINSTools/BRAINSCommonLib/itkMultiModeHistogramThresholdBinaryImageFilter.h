#ifndef __itkMultiModeHistogramThresholdBinaryImageFilter_h
#define __itkMultiModeHistogramThresholdBinaryImageFilter_h

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkNumericTraits.h>
#include <itkArray.h>

namespace itk
{
/**
  * \author Hans J. Johnson
  *
  * This filter
  *
  */
template< class TInputImage, class TOutputImage = Image< unsigned short, TInputImage::ImageDimension > >
class ITK_EXPORT MultiModeHistogramThresholdBinaryImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage                           InputImageType;
  typedef typename InputImageType::ConstPointer InputImagePointer;
  typedef typename InputImageType::RegionType   InputImageRegionType;
  typedef typename InputImageType::PixelType    InputPixelType;

  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointer;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputImageType::PixelType  OutputPixelType;

  typedef MultiModeHistogramThresholdBinaryImageFilter          Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef TOutputImage                                          IntegerImageType;
  typedef typename IntegerImageType::PixelType                  IntegerPixelType;

  typedef Array< double > ThresholdArrayType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiModeHistogramThresholdBinaryImageFilter, ImageToImageFilter);

  itkSetMacro(LinearQuantileThreshold, double);
  itkGetConstMacro(LinearQuantileThreshold, double);

  /** set Quantile Threshold Arrays */
  itkSetMacro(QuantileLowerThreshold, ThresholdArrayType);
  itkGetConstMacro(QuantileLowerThreshold, ThresholdArrayType);
  itkSetMacro(QuantileUpperThreshold, ThresholdArrayType);
  itkGetConstMacro(QuantileUpperThreshold, ThresholdArrayType);

  itkGetConstObjectMacro(BinaryPortionImage, IntegerImageType);
  itkSetObjectMacro(BinaryPortionImage, IntegerImageType);

  itkSetMacro(InsideValue, IntegerPixelType);
  itkGetConstMacro(InsideValue, IntegerPixelType);
  itkSetMacro(OutsideValue, IntegerPixelType);
  itkGetConstMacro(OutsideValue, IntegerPixelType);
protected:
  MultiModeHistogramThresholdBinaryImageFilter();
  ~MultiModeHistogramThresholdBinaryImageFilter();
  void PrintSelf(std::ostream & os, Indent indent) const;

  virtual void GenerateData();

private:
  ThresholdArrayType m_QuantileLowerThreshold;
  ThresholdArrayType m_QuantileUpperThreshold;
  double             m_LinearQuantileThreshold;

  typename IntegerImageType::Pointer m_BinaryPortionImage;

  IntegerPixelType m_InsideValue;
  IntegerPixelType m_OutsideValue;
};
} // end namespace itk

#if ITK_TEMPLATE_TXX
#  include "itkMultiModeHistogramThresholdBinaryImageFilter.txx"
#endif

#endif
