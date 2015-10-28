/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkLMMSEVectorImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006/03/29 14:53:40 $
  Version:   $Revision: 1.5 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkLMMSEVectorImageFilter_h
#define __itkLMMSEVectorImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkVectorImageCastFilter.h"
#include "itkLMMSEVectorImageFilterStep.h"
#include "itkExtractVolumeFilter.h"
#include "itkComputeStatisticsWherePositiveFilter.h"
#include "itkComputeRestrictedHistogram.h"
#include "itkMaskedMeanImageFilter.h"
namespace itk
{
/** \class LMMSEVectorImageFilter
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
class LMMSEVectorImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef LMMSEVectorImageFilter                              Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  typedef typename OutputImageType::Pointer     OutputImagePointer;
  typedef typename InputImageType::Pointer      InputImagePointer;
  typedef typename InputImageType::ConstPointer InputImageConstPointer;

  /** Typedefs for internal processing */
  typedef itk::VectorImage<float, TInputImage::ImageDimension> InternalImageType;
  typedef typename InternalImageType::Pointer                  InternalImagePointer;
  typedef typename InternalImageType::ConstPointer             InternalImageConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( LMMSEVectorImageFilter, ImageToImageFilter );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename InputImageType::SizeType    InputSizeType;

  typedef LMMSEVectorImageFilterStep<InternalImageType, InternalImageType> LMMSEStepType;
  typedef typename LMMSEStepType::Pointer                                  LMMSEStepPointer;

  /** Typedefs for scalar processing */
  typedef itk::VectorImageCastFilter<InputImageType, InternalImageType>               InputCastType;
  typedef typename InputCastType::Pointer                                             InputCastPointer;
  typedef itk::VectorImageCastFilter<InternalImageType, OutputImageType>              OutputCastType;
  typedef typename OutputCastType::Pointer                                            OutputCastPointer;
  typedef float                                                                       ScalarPixelType;
  typedef itk::Image<ScalarPixelType, TInputImage::ImageDimension>                    ScalarImageType;
  typedef itk::ExtractVolumeFilter<InternalImageType, ScalarImageType>                ExtractType;
  typedef typename ExtractType::Pointer                                               ExtractPointer;
  typedef itk::MaskedMeanImageFilter<ScalarImageType, ScalarImageType>                LocalMeanType;
  typedef typename LocalMeanType::Pointer                                             LocalMeanPointer;
  typedef itk::ComputeStatisticsWherePositiveFilter<ScalarImageType, ScalarImageType> StatsType;
  typedef typename StatsType::Pointer                                                 StatsPointer;
  typedef itk::ComputeRestrictedHistogram<ScalarImageType, ScalarImageType>           HistogramType;
  typedef typename HistogramType::Pointer                                             HistogramPointer;

  /** Set and get the radius of the neighbourhoods for estimation and filtering used to compute the mean. */
  itkSetMacro( RadiusFiltering, InputSizeType );
  itkSetMacro( RadiusEstimation, InputSizeType );
  itkGetConstReferenceMacro(RadiusFiltering, InputSizeType);
  itkGetConstReferenceMacro(RadiusEstimation, InputSizeType);

  /** Set and get the number of filter iterations. */
  itkSetMacro( Iterations, unsigned int );
  itkGetMacro( Iterations, unsigned int );

  /** Set the id for the first baseline image used to compute the noise from. */
  itkSetMacro( FirstBaseline, unsigned int );
  itkGetMacro( FirstBaseline, unsigned int );

  /** Set the id for the first baseline image used to compute the noise from. */
  itkSetMacro( MaximumNumberOfBins, unsigned long );
  itkGetMacro( MaximumNumberOfBins, unsigned long );

  /** Fix behaviour in case negative values are obtained */
  itkSetMacro( UseAbsoluteValue, bool );
  itkBooleanMacro( UseAbsoluteValue );
  itkGetMacro( KeepValue, bool );
  itkSetMacro( KeepValue, bool );
  itkBooleanMacro( KeepValue );

  /** Minimum and maximum allowed noise standard deviations.*/
  itkSetMacro( MinimumNoiseSTD, double );
  itkGetMacro( MinimumNoiseSTD, double );
  itkSetMacro( MaximumNoiseSTD, double );
  itkGetMacro( MaximumNoiseSTD, double );

  /** Set and get the minimum number of voxels used for estimation and filtering. */
  itkSetMacro(MinimumNumberOfUsedVoxelsEstimation, int );
  itkSetMacro(MinimumNumberOfUsedVoxelsFiltering, int );
  itkGetMacro(MinimumNumberOfUsedVoxelsEstimation, int );
  itkGetMacro(MinimumNumberOfUsedVoxelsFiltering, int );

  /** Set and get the resolution factor for histogram computing */
  itkSetMacro(HistogramResolutionFactor, double );
  itkGetMacro(HistogramResolutionFactor, double );

  itkSetMacro( Channels, unsigned int );
  itkGetConstReferenceMacro( Channels, unsigned int );

  /** This filter requires the whole input to produce its output */
  virtual void GenerateInputRequestedRegion()
  throw (InvalidRequestedRegionError) ITK_OVERRIDE;

protected:
  LMMSEVectorImageFilter();
  virtual ~LMMSEVectorImageFilter()
  {
  }
  void GenerateData() ITK_OVERRIDE;

  void PrintSelf( std::ostream& os, Indent indent) const ITK_OVERRIDE;

private:
  LMMSEVectorImageFilter(const Self &); // purposely not implemented
  void operator=(const Self &);         // purposely not implemented

  InputSizeType m_RadiusEstimation;
  InputSizeType m_RadiusFiltering;
  unsigned int  m_Iterations;
  bool          m_UseAbsoluteValue;
  bool          m_KeepValue;

  int m_MinimumNumberOfUsedVoxelsEstimation;
  int m_MinimumNumberOfUsedVoxelsFiltering;

  double m_MinimumNoiseSTD;
  double m_MaximumNoiseSTD;

  unsigned int m_FirstBaseline;
  double       m_HistogramResolutionFactor;

  unsigned int  m_Channels;
  unsigned long m_MaximumNumberOfBins;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLMMSEVectorImageFilter.txx"
#endif

#endif
