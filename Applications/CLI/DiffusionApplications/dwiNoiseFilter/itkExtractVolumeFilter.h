/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkExtractVolumeFilter.h,v $
  Language:  C++
  Date:      $Date: 2006/03/27 17:01:10 $
  Version:   $Revision: 1.15 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkExtractVolumeFilter_h
#define __itkExtractVolumeFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"

namespace itk
{
/** \class ExtractVolumeFilter
 *
 * \sa Image
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT ExtractVolumeFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Extract dimension from input image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TInputImage::ImageDimension);

  /** Standard class typedefs. */
  typedef ExtractVolumeFilter Self;

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage InputImageType;
  typedef typename InputImageType::Pointer InputImagePointer;
  typedef TOutputImage OutputImageType;
  typedef typename OutputImageType::Pointer OutputImagePointer;

  /** Standard class typedefs. */
  typedef ImageToImageFilter< InputImageType, OutputImageType> Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  void SetVolumeNr( int iNr ) { iVolumeNr = iNr; std::cout << "Setting volume nr to " << iNr << std::endl; };
  void SetMultiplyFactor( double dFactor ) { this->dFactor = dFactor; };

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ExtractVolumeFilter, ImageToImageFilter);
  
  /** Image typedef support. */
  typedef typename InputImageType::PixelType InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  
  /** Typedef to describe the output and input image index and size types. */
  typedef typename TOutputImage::IndexType OutputImageIndexType;
  typedef typename TInputImage::IndexType  InputImageIndexType;
  typedef typename TOutputImage::SizeType  OutputImageSizeType;
  typedef typename TInputImage::SizeType   InputImageSizeType;
  typedef InputImageSizeType SizeType;


  /** ExtractVolumeFilter needs a larger input requested region than
   * the output requested region.  As such, ExtractVolumeFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */

protected:
  ExtractVolumeFilter()
    {
      iVolumeNr = 0;
      dFactor = 1;
    }
  virtual ~ExtractVolumeFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  void GenerateOutputInformation();
  void GenerateInputRequestedRegion();
  void GenerateData();

private:
  ExtractVolumeFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  int iVolumeNr;
  double dFactor;
};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkExtractVolumeFilter.txx"
#endif

#endif
