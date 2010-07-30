#ifndef __itkFindeCenterOfBrainFilter_h
#define __itkFindeCenterOfBrainFilter_h
#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include "itkLargestForegroundFilledMaskImageFilter.h"

namespace itk
{
/**
  * \class FindCenterOfBrainFilter
  */
template< class TInputImage, class TMaskImage = itk::Image< unsigned char, 3 > >
class ITK_EXPORT FindCenterOfBrainFilter:
  public ImageToImageFilter< TInputImage, TInputImage >
{
public:
  typedef FindCenterOfBrainFilter                        Self;
  typedef ImageToImageFilter< TInputImage, TInputImage > Superclass;
  typedef SmartPointer< Self >                           Pointer;
  typedef SmartPointer< const Self >                     ConstPointer;
  itkNewMacro(Self);
  itkTypeMacro(FindCenterOfBrain, Superclass);

  typedef TInputImage                     ImageType;
  typedef TMaskImage                      MaskImageType;
  typedef typename MaskImageType::Pointer MaskImagePointer;
  typedef typename ImageType::Pointer     InputImagePointer;
  typedef typename ImageType::PixelType   PixelType;
  typedef typename ImageType::PointType   PointType;
  typedef typename ImageType::SizeType    SizeType;
  typedef typename ImageType::SpacingType SpacingType;
  typedef typename ImageType::IndexType   IndexType;
  typedef typename itk::ImageRegionIteratorWithIndex< ImageType >
  ImageIteratorType;
  typedef typename itk::ImageRegionConstIteratorWithIndex< ImageType >
  ImageConstIteratorType;
  typedef LargestForegroundFilledMaskImageFilter< ImageType, MaskImageType >
  LFFMaskFilterType;
  typedef typename itk::Image< float, 3 >     DistanceImageType;
  typedef typename DistanceImageType::Pointer DistanceImagePointer;
  /** Image related typedefs. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TInputImage::ImageDimension);

  itkSetMacro(Maximize, bool);
  itkGetConstMacro(Maximize, bool);
  itkSetMacro(Axis, unsigned int);
  itkGetConstMacro(Axis, unsigned int);
  itkSetMacro(OtsuPercentileThreshold, double);
  itkGetConstMacro(OtsuPercentileThreshold, double);
  itkSetMacro(ClosingSize, unsigned int);
  itkGetConstMacro(ClosingSize, unsigned int);
  itkSetMacro(HeadSizeLimit, double);
  itkGetConstMacro(HeadSizeLimit, double);
  itkSetMacro(HeadSizeEstimate, double);
  itkGetConstMacro(HeadSizeEstimate, double);
  itkSetMacro(BackgroundValue, PixelType);
  itkGetConstMacro(BackgroundValue, PixelType);

  itkGetConstMacro(CenterOfBrain, PointType);
  itkGetObjectMacro(TrimmedImage, TInputImage);

  itkSetObjectMacro(ImageMask, TMaskImage);
  itkGetObjectMacro(ImageMask, TMaskImage);

  // THIS IS OUTPUT ONLY  itkSetObjectMacro(ClippedImageMask, TMaskImage);
  itkGetObjectMacro(ClippedImageMask, TMaskImage);

  // DEBUGGING STUFF
  itkSetMacro(GenerateDebugImages, bool);
  itkGetMacro(GenerateDebugImages, bool);
  DistanceImagePointer GetDebugDistanceImage() { return m_DebugDistanceImage; }
  InputImagePointer GetDebugGridImage() { return m_DebugGridImage; }
  MaskImagePointer GetDebugAfterGridComputationsForegroundImage()
  {
    return m_DebugAfterGridComputationsForegroundImage;
  }

  MaskImagePointer GetDebugClippedImageMask()
  {
    return m_DebugClippedImageMask;
  }

  InputImagePointer GetDebugTrimmedImage()
  {
    return m_DebugTrimmedImage;
  }

protected:
  FindCenterOfBrainFilter();
  ~FindCenterOfBrainFilter();
  void PrintSelf(std::ostream & os, Indent indent) const;

  void AllocateOutputs();

  virtual void GenerateData();

private:
  bool         m_Maximize;
  unsigned int m_Axis;
  double       m_OtsuPercentileThreshold;
  unsigned int m_ClosingSize;
  double       m_HeadSizeLimit;
  double       m_HeadSizeEstimate;
  PixelType    m_BackgroundValue;
  PointType    m_CenterOfBrain;
  //
  // DEBUGGING
  bool m_GenerateDebugImages;
  /** The foreground mask, computed automatically if not specified
   * on the command line. **/
  typename TMaskImage::Pointer m_ImageMask;
  /** The foreground mask, computed automatically if
   * not specified on the command line. **/
  typename TMaskImage::Pointer m_ClippedImageMask;
  typename TInputImage::Pointer m_TrimmedImage;
  DistanceImagePointer m_DebugDistanceImage;
  InputImagePointer    m_DebugGridImage;
  MaskImagePointer     m_DebugAfterGridComputationsForegroundImage;
  MaskImagePointer     m_DebugClippedImageMask;
  InputImagePointer    m_DebugTrimmedImage;
};
}

#if ITK_TEMPLATE_TXX
#  include "itkFindCenterOfBrainFilter.txx"
#endif

#endif // itkFindeCenterOfBrainFilter_txx
