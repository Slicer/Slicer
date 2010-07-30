/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkOtsuHistogramMatchingImageFilter.h,v $
 *  Language:  C++
 *  Date:      $Date: 2009-05-02 05:43:54 $
 *  Version:   $Revision: 1.13 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __itkOtsuHistogramMatchingImageFilter_h
#define __itkOtsuHistogramMatchingImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkHistogram.h"
#include "vnl/vnl_matrix.h"

namespace itk
{
/** \class OtsuHistogramMatchingImageFilter
  * \brief Normalize the grayscale values between two image by histogram
  * matching.
  *
  * OtsuHistogramMatchingImageFilter normalizes the grayscale values of a source
  * image based on the grayscale values of a reference image.
  * This filter uses a histogram matching technique where the histograms of the
  * two images are matched only at a specified number of quantile values.
  *
  * This filter was orginally designed to normalize MR images of the same
  * MR protocol and same body part. The algorithm works best if background
  * pixels are excluded from both the source and reference histograms.
  * A simple background exclusion method is to exclude all pixels whose
  * grayscale values are smaller than the mean grayscale value.
  * ThresholdAtMeanIntensityOn() switches on this simple background
  * exclusion method.
  *
  * The source image can be set via either SetInput() or SetSourceImage().
  * The reference image can be set via SetReferenceImage().
  *
  * SetNumberOfHistogramLevels() sets the number of bins used when
  * creating histograms of the source and reference images.
  * SetNumberOfMatchPoints() governs the number of quantile values to be
  * matched.
  *
  * This filter assumes that both the source and reference are of the same
  * type and that the input and output image type have the same number of
  * dimension and have scalar pixel types.
  *
  * \ingroup IntensityImageFilters Multithreaded
  *
  */
/* THistogramMeasurement -- The precision level for which to do
  * HistogramMeasurmenets */
template< class TInputImage, class TOutputImage, class THistogramMeasurement = ITK_TYPENAME TInputImage::PixelType >
class ITK_EXPORT OtsuHistogramMatchingImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef OtsuHistogramMatchingImageFilter                Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(OtsuHistogramMatchingImageFilter, ImageToImageFilter);

  /** ImageDimension enumeration. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Typedef to describe the output image region type. */
  typedef typename TOutputImage::RegionType OutputImageRegionType;

  /** Inherited typedefs. */
  typedef typename Superclass::InputImageType         InputImageType;
  typedef typename Superclass::InputImagePointer      InputImagePointer;
  typedef typename Superclass::InputImageConstPointer InputImageConstPointer;
  typedef typename Superclass::OutputImageType        OutputImageType;
  typedef typename Superclass::OutputImagePointer     OutputImagePointer;

  /** Pixel related typedefs. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename itk::Image< unsigned char, 3 > MaskImageType;

  /** Histogram related typedefs. */
#ifdef ITK_USE_REVIEW_STATISTICS
  typedef Statistics::Histogram< THistogramMeasurement > HistogramType;
#else
  typedef Statistics::Histogram< THistogramMeasurement, 1 > HistogramType;
#endif
  typedef typename HistogramType::Pointer HistogramPointer;

  /** Set/Get the source image. */
  void SetSourceImage(const InputImageType *source)
  {
    this->SetInput(source);
  }

  const InputImageType * GetSourceImage(void)
  {
    return this->GetInput();
  }

  /** Set/Get the reference image. */
  void SetReferenceImage(const InputImageType *reference);

  const InputImageType * GetReferenceImage(void);

  itkSetObjectMacro(SourceMask, MaskImageType);
  itkSetObjectMacro(ReferenceMask, MaskImageType);

  /** Set/Get the number of histogram levels used. */
  itkSetMacro(NumberOfHistogramLevels, unsigned long);
  itkGetConstMacro(NumberOfHistogramLevels, unsigned long);

  /** Set/Get the number of match points used. */
  itkSetMacro(NumberOfMatchPoints, unsigned long);
  itkGetConstMacro(NumberOfMatchPoints, unsigned long);

  /** Set/Get the threshold at mean intensity flag.
    * If true, only source (reference) pixels which are greater
    * than the mean source (reference) intensity is used in
    * the histogram matching. If false, all pixels are
    * used. */
  itkSetMacro(ThresholdAtMeanIntensity, bool);
  itkGetConstMacro(ThresholdAtMeanIntensity, bool);
  itkBooleanMacro(ThresholdAtMeanIntensity);

  /** This filter requires all of the input to be in the buffer. */
  virtual void GenerateInputRequestedRegion();

  /** Methods to get the histograms of the source, reference, and
    * output. Objects are only valid after Update() has been called
    * on this filter. */
  itkGetObjectMacro(SourceHistogram, HistogramType);
  itkGetObjectMacro(ReferenceHistogram, HistogramType);
  itkGetObjectMacro(OutputHistogram, HistogramType);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( IntConvertibleToInputCheck,
                   ( Concept::Convertible< int, InputPixelType > ) );
  itkConceptMacro( SameDimensionCheck,
                   ( Concept::SameDimension< ImageDimension, OutputImageDimension > ) );
  itkConceptMacro( DoubleConvertibleToInputCheck,
                   ( Concept::Convertible< double, InputPixelType > ) );
  itkConceptMacro( DoubleConvertibleToOutputCheck,
                   ( Concept::Convertible< double, OutputPixelType > ) );
  itkConceptMacro( InputConvertibleToDoubleCheck,
                   ( Concept::Convertible< InputPixelType, double > ) );
  itkConceptMacro( OutputConvertibleToDoubleCheck,
                   ( Concept::Convertible< OutputPixelType, double > ) );
  itkConceptMacro( SameTypeCheck,
                   ( Concept::SameType< InputPixelType, OutputPixelType > ) );
  /** End concept checking */
#endif
protected:
  OtsuHistogramMatchingImageFilter();
  ~OtsuHistogramMatchingImageFilter() {}
  void PrintSelf(std::ostream & os, Indent indent) const;

  void BeforeThreadedGenerateData();

  void AfterThreadedGenerateData();

  void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                            int threadId);

  /** Compute min, max and mean of an image. */
  void ComputeMinMaxMean(const InputImageType *image,
                         THistogramMeasurement & minValue,
                         THistogramMeasurement & maxValue,
                         THistogramMeasurement & meanValue);

  /** Construct a histogram from an image. */
  void ConstructHistogram(const InputImageType *image,
                          const typename MaskImageType::Pointer mask,
                          HistogramType *histogram, const THistogramMeasurement minValue,
                          const THistogramMeasurement maxValue);

private:
  OtsuHistogramMatchingImageFilter(const Self &); // purposely not implemented
  void operator=(const Self &);                   // purposely not implemented

  unsigned long m_NumberOfHistogramLevels;
  unsigned long m_NumberOfMatchPoints;
  bool          m_ThresholdAtMeanIntensity;

  InputPixelType  m_SourceIntensityThreshold;
  InputPixelType  m_ReferenceIntensityThreshold;
  OutputPixelType m_OutputIntensityThreshold;

  THistogramMeasurement m_SourceMinValue;
  THistogramMeasurement m_SourceMaxValue;
  THistogramMeasurement m_SourceMeanValue;
  THistogramMeasurement m_ReferenceMinValue;
  THistogramMeasurement m_ReferenceMaxValue;
  THistogramMeasurement m_ReferenceMeanValue;
  THistogramMeasurement m_OutputMinValue;
  THistogramMeasurement m_OutputMaxValue;
  THistogramMeasurement m_OutputMeanValue;

  HistogramPointer m_SourceHistogram;
  HistogramPointer m_ReferenceHistogram;
  HistogramPointer m_OutputHistogram;

  typedef vnl_matrix< double > TableType;
  TableType m_QuantileTable;

  typedef vnl_vector< double > GradientArrayType;
  GradientArrayType m_Gradients;
  double            m_LowerGradient;
  double            m_UpperGradient;

  typename MaskImageType::Pointer m_SourceMask;
  typename MaskImageType::Pointer m_ReferenceMask;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkOtsuHistogramMatchingImageFilter.txx"
#endif

#endif
