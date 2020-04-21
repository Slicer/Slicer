/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDifferenceImageFilter.h,v $
  Language:  C++

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef itkDifferenceDiffusionTensor3DImageFilter_h
#define itkDifferenceDiffusionTensor3DImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkDiffusionTensor3D.h"
#include "itkNumericTraits.h"
#include "itkArray.h"
#include "itkMatrix.h"
#include "itkMetaDataObject.h"

namespace itk
{

/** \class DifferenceDiffusionTensor3DImageFilter
 * \brief Implements comparison between two DiffusionTensor3D images.
 *
 * This filter is used by the testing system to compute the difference between
 * a valid DiffusionTensor3D image and a DiffusionTensor3D image produced by
 * the test. The comparison value is computed by visiting all the pixels in the
 * baseline images and comparing their values with the pixel values in the
 * neighborhood of the homologous pixel in the other image. At one voxel, it
 * adds the absolute value of the difference between the different components
 * of the diffusion tensor. It compares the sum to a threshold set by the
 * developer
 *
 * \ingroup IntensityImageFilters   Multithreaded
 */
template <class TInputImage, class TOutputImage>
class DifferenceDiffusionTensor3DImageFilter :
  public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef DifferenceDiffusionTensor3DImageFilter        Self;
  typedef ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(DifferenceDiffusionTensor3DImageFilter, ImageToImageFilter);

  /** Some convenient typedefs. */
  typedef TInputImage                                       InputImageType;
  typedef TOutputImage                                      OutputImageType;
  typedef typename OutputImageType::PixelType               OutputPixelType;
  typedef typename InputImageType::PixelType                InputPixelType;
  typedef typename OutputImageType::RegionType              OutputImageRegionType;
  typedef typename NumericTraits<OutputPixelType>::RealType RealType;
  typedef typename NumericTraits<RealType>::AccumulateType  AccumulateType;

  typedef std::vector<std::vector<double> >     DoubleVectorType;
  typedef itk::MetaDataObject<DoubleVectorType> MetaDataDoubleVectorType;
  typedef itk::Matrix<double, 3, 3>             MatrixType;

  /** Set the valid image input.  This will be input 0.  */
  virtual void SetValidInput(const InputImageType* validImage);

  /** Set the test image input.  This will be input 1.  */
  virtual void SetTestInput(const InputImageType* testImage);

  /** Set/Get the maximum distance away to look for a matching pixel.
      Default is 0. */
  itkSetMacro(ToleranceRadius, int);
  itkGetConstMacro(ToleranceRadius, int);

  /** Set/Get the minimum threshold for pixels to be different.
      Default is 0. */
  itkSetMacro(DifferenceThreshold, OutputPixelType);
  itkGetConstMacro(DifferenceThreshold, OutputPixelType);

  /** Set/Get ignore boundary pixels.  Useful when resampling may have
   *    introduced difference pixel values along the image edge
   *    Default = false */
  itkSetMacro(IgnoreBoundaryPixels, bool);
  itkGetConstMacro(IgnoreBoundaryPixels, bool);

  /** Get parameters of the difference image after execution.  */
  itkGetConstMacro(MeanDifference, RealType);
  itkGetConstMacro(TotalDifference, AccumulateType);
  itkGetConstMacro(NumberOfPixelsWithDifferences, unsigned long);
protected:
  MatrixType GetMetaDataDictionary( const InputImageType* image );

  DifferenceDiffusionTensor3DImageFilter();
  ~DifferenceDiffusionTensor3DImageFilter() override = default;

  void PrintSelf(std::ostream& os, Indent indent) const override;

  /** DifferenceImageFilter can be implemented as a multithreaded
   * filter.  Therefore, this implementation provides a
   * ThreadedGenerateData() routine which is called for each
   * processing thread. The output image data is allocated
   * automatically by the superclass prior to calling
   * ThreadedGenerateData().  ThreadedGenerateData can only write to
   * the portion of the output image specified by the parameter
   * "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData()  */
  void ThreadedGenerateData(const OutputImageRegionType& threadRegion, ThreadIdType threadId) override;

  void BeforeThreadedGenerateData() override;

  void AfterThreadedGenerateData() override;

  InputPixelType ApplyMeasurementFrameToTensor( InputPixelType tensor, const MatrixType & measurementFrame );

  OutputPixelType m_DifferenceThreshold;
  RealType        m_MeanDifference;
  AccumulateType  m_TotalDifference;
  unsigned long   m_NumberOfPixelsWithDifferences;
  int             m_ToleranceRadius;

  Array<AccumulateType> m_ThreadDifferenceSum;
  Array<unsigned long>  m_ThreadNumberOfPixels;
  MatrixType            m_MeasurementFrameValid;
  MatrixType            m_MeasurementFrameTest;
private:
  DifferenceDiffusionTensor3DImageFilter(const Self &) = delete;
  void operator=(const Self &) = delete;

  bool m_IgnoreBoundaryPixels;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDifferenceDiffusionTensor3DImageFilter.txx"
#endif

#endif
