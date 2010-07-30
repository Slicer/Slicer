/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkBRAINSROIAutoImageFilter.h,v $
 *  Language:  C++
 *  Date:      $Date: 2008-10-16 19:33:40 $
 *  Version:   $Revision: 1.7 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __itkBRAINSROIAutoImageFilter_h
#define __itkBRAINSROIAutoImageFilter_h

// First make sure that the configuration is available.
// This line can be removed once the optimized versions
// gets integrated into the main directories.
#include "itkConfigure.h"
#include "itkImageToImageFilter.h"
#include "itkImage.h"

#include "itkImageMaskSpatialObject.h"
#include "itkLargestForegroundFilledMaskImageFilter.h"
#include "itkCastImageFilter.h"

typedef itk::SpatialObject< 3 >    SpatialObjectType;
typedef SpatialObjectType::Pointer ImageMaskPointer;

namespace itk
{
/** \class BRAINSROIAutoImageFilter
  * \brief This is a class to help with identifying common tissue
  * Regions in an image.
  *
  * \sa Image
  * \sa Neighborhood
  *
  * \ingroup IntensityImageFilters
  */
template< class TInputImage, class TOutputImage >
class ITK_EXPORT BRAINSROIAutoImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef BRAINSROIAutoImageFilter                              Self;
  typedef ImageToImageFilter< InputImageType, OutputImageType > Superclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(BRAINSROIAutoImageFilter, ImageToImageFilter);

  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  typedef itk::Image< unsigned char, 3 >                            UCHARIMAGE;
  typedef itk::ImageMaskSpatialObject< UCHARIMAGE::ImageDimension > ImageMaskSpatialObjectType;

  /** */
  itkSetMacro(OtsuPercentileThreshold, double);
  itkGetConstMacro(OtsuPercentileThreshold, double);
  /** */
  itkSetMacro(ThresholdCorrectionFactor, double);
  itkGetConstMacro(ThresholdCorrectionFactor, double);
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

  // NOTE:  This will generate a new spatial object each time it is called, and
  // not return the previous spatial object
  ImageMaskPointer GetSpatialObjectROI(void)
  {
    if ( m_ResultMaskPointer.IsNull() ) // This is a cheap way to only create
                                        // the mask once, note that this is made
                                        // null when GenerateData is called.
      {
      typedef itk::CastImageFilter< OutputImageType, UCHARIMAGE > CastImageFilter;
      typename CastImageFilter::Pointer castFilter = CastImageFilter::New();
      castFilter->SetInput( this->GetOutput() );
      castFilter->Update();

      // convert mask image to mask
      typename ImageMaskSpatialObjectType::Pointer mask = ImageMaskSpatialObjectType::New();
      mask->SetImage( castFilter->GetOutput() );
      mask->ComputeObjectToWorldTransform();
      m_ResultMaskPointer = dynamic_cast< ImageMaskSpatialObjectType * >( mask.GetPointer() );
      }
    return m_ResultMaskPointer;
  }

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( SameDimensionCheck,
                   ( Concept::SameDimension< InputImageDimension, OutputImageDimension > ) );
  /** End concept checking */
#endif
protected:
  BRAINSROIAutoImageFilter();
  virtual ~BRAINSROIAutoImageFilter() {}
  void PrintSelf(std::ostream & os, Indent indent) const;

  void GenerateData();

private:
  BRAINSROIAutoImageFilter(const Self &); // purposely not implemented
  void operator=(const Self &);           // purposely not implemented

  double           m_OtsuPercentileThreshold;
  double           m_ThresholdCorrectionFactor;
  double           m_ClosingSize;
  double           m_DilateSize;
  ImageMaskPointer m_ResultMaskPointer;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkBRAINSROIAutoImageFilter.txx"
#endif

#endif
