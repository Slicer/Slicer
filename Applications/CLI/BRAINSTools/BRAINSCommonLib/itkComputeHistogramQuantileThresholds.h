#ifndef __itkComputeHistogramQuantileThresholds_h
#define __itkComputeHistogramQuantileThresholds_h

#include <itkImage.h>
#include <itkNumericTraits.h>

namespace itk
{
/**
  * \class ComputeHistogramQuantileThresholds
  * \author Hans J. Johnson
  *
  * This filter just computes Histogram Quantile Thresholds.  It does not apply
  *the thresholds.
  *
  */
template< class TInputImage, class TMaskImage >
class ITK_EXPORT ComputeHistogramQuantileThresholds:
  public Object
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage                           InputImageType;
  typedef typename InputImageType::ConstPointer InputImagePointer;
  typedef typename InputImageType::RegionType   InputImageRegionType;
  typedef typename InputImageType::PixelType    InputPixelType;

  typedef ComputeHistogramQuantileThresholds Self;
  typedef Object                             Superclass;
  typedef SmartPointer< Self >               Pointer;
  typedef typename TMaskImage::PixelType     MaskPixelType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ComputeHistogramQuantileThresholds, Object);

  /** set Quantile Threshold */
  itkSetMacro(QuantileLowerThreshold, double);
  itkGetConstMacro(QuantileLowerThreshold, double);
  itkSetMacro(QuantileUpperThreshold, double);
  itkGetConstMacro(QuantileUpperThreshold, double);

  itkGetConstMacro(LowerIntensityThresholdValue, typename InputImageType::PixelType);
  itkGetConstMacro(UpperIntensityThresholdValue, typename InputImageType::PixelType);
  itkGetConstMacro(NumberOfValidHistogramsEntries, unsigned int);

  itkGetConstObjectMacro(Image, InputImageType);
  itkSetConstObjectMacro(Image, InputImageType);

  itkSetMacro(ImageMin, typename TInputImage::PixelType);
  itkGetConstMacro(ImageMin, typename TInputImage::PixelType);
  itkSetMacro(ImageMax, typename TInputImage::PixelType);
  itkGetConstMacro(ImageMax, typename TInputImage::PixelType);

  itkGetConstObjectMacro(BinaryPortionImage, TMaskImage);
  itkSetObjectMacro(BinaryPortionImage, TMaskImage);

  void Calculate();

protected:
  ComputeHistogramQuantileThresholds();
  ~ComputeHistogramQuantileThresholds();
  void PrintSelf(std::ostream & os, Indent indent) const;

private:
  InputImagePointer m_Image;
  typename TMaskImage::Pointer m_BinaryPortionImage;
  void ImageMinMax(InputPixelType & min, InputPixelType & max);

  double m_QuantileLowerThreshold;
  double m_QuantileUpperThreshold;
  typename TInputImage::PixelType m_ImageMin;
  typename TInputImage::PixelType m_ImageMax;

  typename InputImageType::PixelType m_LowerIntensityThresholdValue;
  typename InputImageType::PixelType m_UpperIntensityThresholdValue;
  unsigned int m_NumberOfValidHistogramsEntries;
};
} // end namespace itk

#if ITK_TEMPLATE_TXX
#  include "itkComputeHistogramQuantileThresholds.txx"
#endif

#endif
