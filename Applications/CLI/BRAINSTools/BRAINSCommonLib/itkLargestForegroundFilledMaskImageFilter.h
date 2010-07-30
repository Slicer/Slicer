#ifndef __itkLargestForegroundFilledMaskImageFilter_h
#define __itkLargestForegroundFilledMaskImageFilter_h

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkNumericTraits.h>

namespace itk
{
/**
  * \class LargestForegroundFilledMaskImageFilter
  * \author Hans J. Johnson
  *
  * This filter does a good job of finding a single largest connected
  * mask that separates the foreground object from the background.
  * It assumes that the corner voxels of the image belong to the backgound.
  * This filter was written for the purpose of finding the tissue
  * region of a brain image with no internal holes.
  *
  * The OtsuPercentile Thresholds are used to define the range of values
  * where the percentage of voxels falls beetween
  * (0+OtsuPercentileLowerThreshold) < "Intensities of Interest" <
  *(1-OtsuPercentileUpperThreshold).
  *
  * The ClosingSize specifies how many mm to dilate followed by
  * erode to fill holes that be present in the image.
  *
  * The DilateSize specifies how many mm to dilate
  * as a final step to include a small amount of surface background in addition
  *to the
  * tissue region present in the image.
  *
  * The image that is returned will be a binary image with foreground and
  *background
  * values specified by the user (defaults to 1 and 0 respectively).
  *
  */
template< class TInputImage, class TOutputImage = TInputImage >
class ITK_EXPORT LargestForegroundFilledMaskImageFilter:
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

  typedef LargestForegroundFilledMaskImageFilter                   Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType >    Superclass;
  typedef SmartPointer< Self >                                     Pointer;
  typedef Image< unsigned short, OutputImageType::ImageDimension > IntegerImageType;
  typedef typename IntegerImageType::PixelType                     IntegerPixelType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(LargestForegroundFilledMaskImageFilter, ImageToImageFilter);

  /** set Otsu Threshold */
  itkSetMacro(OtsuPercentileLowerThreshold, double);
  itkGetConstMacro(OtsuPercentileLowerThreshold, double);
  itkSetMacro(OtsuPercentileUpperThreshold, double);
  itkGetConstMacro(OtsuPercentileUpperThreshold, double);

  /** Short hand for setting both upper and lower
    * (0+OtsuPercentileThreshold) < "Intensities of Interest" <
    *(1-OtsuPercentileThreshold).
    */
  void SetOtsuPercentileThreshold(const double percentile)
  {
    this->SetOtsuPercentileLowerThreshold(percentile);
    this->SetOtsuPercentileUpperThreshold(1.0 - percentile);
  }

  double GetOtsuPercentileThreshold(void) const
  {
    return this->GetOtsuPercentileLowerThreshold();
  }

  /** The closing size in mm, this is rounded up to the next closest number of
    * voxel
    * by taking Spacing into account */
  itkSetMacro(ClosingSize, double);
  itkGetConstMacro(ClosingSize, double);
  /** The dilation size in mm, this is rounded up to the next closest number of
    * voxel
    * by taking Spacing into account */
  itkSetMacro(DilateSize, double);
  itkGetConstMacro(DilateSize, double);
  itkSetMacro(InsideValue, IntegerPixelType);
  itkGetMacro(InsideValue, IntegerPixelType);
  itkSetMacro(OutsideValue, IntegerPixelType);
  itkGetMacro(OutsideValue, IntegerPixelType);
  itkSetMacro(ThresholdCorrectionFactor, double);
  itkGetConstMacro(ThresholdCorrectionFactor, double);
protected:
  LargestForegroundFilledMaskImageFilter();
  ~LargestForegroundFilledMaskImageFilter();
  void PrintSelf(std::ostream & os, Indent indent) const;

  virtual void GenerateData();

private:
  /** Returns true if more than two bins of informaiton are found,
    * returns false if only two bins of informaiton are found (i.e. found a
    *binary image).
    * Low and High are set to the ?????? */
  unsigned int SetLowHigh(InputPixelType & low,
                          InputPixelType & high);

  void ImageMinMax(InputPixelType & min, InputPixelType & max);

  // No longer used  double m_OtsuPercentileThreshold;
  double           m_OtsuPercentileLowerThreshold;
  double           m_OtsuPercentileUpperThreshold;
  double           m_ThresholdCorrectionFactor;
  double           m_ClosingSize;
  double           m_DilateSize;
  IntegerPixelType m_InsideValue;
  IntegerPixelType m_OutsideValue;
};
} // end namespace itk

#if ITK_TEMPLATE_TXX
#  include "itkLargestForegroundFilledMaskImageFilter.txx"
#endif

#endif
