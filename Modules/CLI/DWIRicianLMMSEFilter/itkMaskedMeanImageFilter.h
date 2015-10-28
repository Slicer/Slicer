/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMaskedMeanImageFilter.h,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkMaskedMeanImageFilter_h
#define __itkMaskedMeanImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkNumericTraits.h"

namespace itk
{
/** \class MaskedMeanImageFilter
 * \brief Applies an averaging filter to an image
 *
 * Computes an image where a given pixel is the mean value of the
 * the pixels in a neighborhood about the corresponding input pixel.
 * Only uses pixels with values greater than zero!
 *
 * A mean filter is one of the family of linear filters.
 *
 * \sa Image
 * \sa Neighborhood
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 *
 * \ingroup IntensityImageFilters
 */
template <class TInputImage, class TOutputImage>
class MaskedMeanImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage                           InputImageType;
  typedef TOutputImage                          OutputImageType;
  typedef typename InputImageType::Pointer      InputImagePointer;
  typedef typename InputImageType::ConstPointer InputImageConstPointer;
  typedef typename OutputImageType::Pointer     OutputImagePointer;

  /** Standard class typedefs. */
  typedef MaskedMeanImageFilter                               Self;
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro( MaskedMeanImageFilter, ImageToImageFilter );

  /** Set and get the minimum number of samples for reliable estimates. */
  itkSetMacro( MinimumNumberOfUsedVoxels, int );
  itkGetMacro( MinimumNumberOfUsedVoxels, int );

  /** Image typedef support. */
  typedef typename InputImageType::PixelType               InputPixelType;
  typedef typename OutputImageType::PixelType              OutputPixelType;
  typedef typename NumericTraits<InputPixelType>::RealType InputRealType;
  typedef typename InputImageType::RegionType              InputImageRegionType;
  typedef typename OutputImageType::RegionType             OutputImageRegionType;
  typedef typename InputImageType::SizeType                InputSizeType;

  /** Set and get the radius of the neighborhood used to compute the mean. */
  itkSetMacro(Radius, InputSizeType);
  itkGetConstReferenceMacro(Radius, InputSizeType);

  /** MaskedMeanImageFilter needs a larger input requested region than
   * the output requested region.  As such, MaskedMeanImageFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion()
  throw (InvalidRequestedRegionError) ITK_OVERRIDE;

protected:
  MaskedMeanImageFilter();
  virtual ~MaskedMeanImageFilter()
  {
  }
  void PrintSelf( std::ostream& os, Indent indent) const ITK_OVERRIDE;

  /** MaskedMeanImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData()
   * routine which is called for each processing thread. The output
   * image data is allocated automatically by the superclass prior to
   * calling ThreadedGenerateData().  ThreadedGenerateData can only
   * write to the portion of the output image specified by the
   * parameter "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData() */
  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;

private:
  MaskedMeanImageFilter(const Self &); // purposely not implemented
  void operator=(const Self &);        // purposely not implemented

  InputSizeType m_Radius;
  int           m_MinimumNumberOfUsedVoxels;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMaskedMeanImageFilter.txx"
#endif

#endif
