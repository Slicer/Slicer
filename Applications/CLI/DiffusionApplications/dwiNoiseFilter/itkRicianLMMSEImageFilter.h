/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkRicianLMMSEImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006/03/29 14:53:40 $
  Version:   $Revision: 1.5 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkRicianLMMSEImageFilter_h
#define __itkRicianLMMSEImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkNumericTraits.h"
#include <itkThresholdImageFilter.h>
#include <itkVectorImage.h>

namespace itk
{
/** \class RicianLMMSEImageFilter
 * \brief Applies a Rician Linear Minimum Mean Square Error Filter to an Image
 *
 * \sa Image
 * \sa Neighborhood
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 * 
 * \ingroup IntensityImageFilters
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT RicianLMMSEImageFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef RicianLMMSEImageFilter Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  typedef typename OutputImageType::Pointer OutputPointer;
  typedef typename InputImageType::Pointer InputPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(RicianLMMSEImageFilter, ImageToImageFilter);
  
  /** Image typedef support. */
  typedef typename InputImageType::PixelType InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename itk::Image<typename InputPixelType::ValueType,InputImageDimension> ScalarImageType;
  typedef typename itk::Image<typename OutputPixelType::ValueType,OutputImageDimension> ScalarDoubleImageType;

  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  /** Set the radius of the neighborhood used to compute the mean. */
  itkSetMacro(RadiusFiltering, InputSizeType);
  itkSetMacro(RadiusEstimation, InputSizeType);
  
  /** Get the radius of the neighborhood used to compute the mean */
  itkGetConstReferenceMacro(RadiusFiltering, InputSizeType);
  itkGetConstReferenceMacro(RadiusEstimation, InputSizeType);

  /** Set the noise standard deviation required for the filter. */
  itkSetMacro(NoiseSTD, double );

  /** Get the noise standard deviation used for the filter. */
  itkGetConstMacro(NoiseSTD, double );
  
  /** Set the number of filter iterations. */
  itkSetMacro(Iterations, int );

  /** Get the number of iterations. */
  itkGetMacro(Iterations, int );

  /** Set the id for the first baseline image used to compute the noise from. */
  itkSetMacro(FirstBaseline, int );

  /** Get the id for the first baseline image used to compute the noise from. */
  itkGetMacro(FirstBaseline, int );

  itkGetMacro( UseAbsoluteValue, bool );
  itkSetMacro( UseAbsoluteValue, bool );
  itkBooleanMacro( UseAbsoluteValue );

  itkGetMacro( WriteHistogram, bool );
  itkSetMacro( WriteHistogram, bool );
  itkBooleanMacro( WriteHistogram );

  /** Minimum and maximum allowed noise standard deviations.*/
  itkSetMacro(MinimumNoiseSTD, double);
  itkGetMacro(MinimumNoiseSTD, double);
  itkSetMacro(MaximumNoiseSTD, double);
  itkGetMacro(MaximumNoiseSTD, double);

  /** Set the number of filter iterations. */
  itkSetMacro(MinimumNumberOfUsedVoxelsEstimation, int );
  itkSetMacro(MinimumNumberOfUsedVoxelsFiltering, int );

  /** Get the number of iterations. */
  itkGetMacro(MinimumNumberOfUsedVoxelsEstimation, int );
  itkGetMacro(MinimumNumberOfUsedVoxelsFiltering, int );

  itkSetMacro(HistogramResolutionFactor, double );
  itkGetMacro(HistogramResolutionFactor, double );

  /** RicianLMMSEImageFilter needs a larger input requested region than
   * the output requested region.  As such, RicianLMMSEImageFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() throw(InvalidRequestedRegionError);

protected:
  RicianLMMSEImageFilter();
  virtual ~RicianLMMSEImageFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** RicianLMMSEImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData()
   * routine which is called for each processing thread. The output
   * image data is allocated automatically by the superclass prior to
   * calling ThreadedGenerateData().  ThreadedGenerateData can only
   * write to the portion of the output image specified by the
   * parameter "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData() */
  //void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
  //                          int threadId );

  void GenerateData(); // would be nice to have this multi-threaded. Not sure how to do this at the moment.
  void GenerateOutputInformation();
  double EstimateNoiseSTD( OutputPointer, int );
  void SetupComputationMask();

private:
  RicianLMMSEImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  InputSizeType m_RadiusEstimation;
  InputSizeType m_RadiusFiltering;
  double m_NoiseSTD;
  int m_Iterations;
  bool m_UseAbsoluteValue;
  bool m_WriteHistogram;

  int m_MinimumNumberOfUsedVoxelsEstimation;
  int m_MinimumNumberOfUsedVoxelsFiltering;

  double m_MinimumNoiseSTD;
  double m_MaximumNoiseSTD;

  int m_FirstBaseline;

  double m_HistogramResolutionFactor;

  int iNumBins;
  double dLowerBound;
  double dUpperBound;

  typedef itk::ThresholdImageFilter<ScalarImageType> ThresholdImageFilterType;

  typename ThresholdImageFilterType::Pointer zeroMaskImageFilter;


};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRicianLMMSEImageFilter.txx"
#endif

#endif
