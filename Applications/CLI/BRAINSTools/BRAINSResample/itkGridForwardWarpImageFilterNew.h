/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkGridForwardWarpImageFilterNew.h,v $
 *  Language:  C++
 *  Date:      $Date: 2009-04-23 03:43:41 $
 *  Version:   $Revision: 1.2 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

#ifndef __itkGridForwardWarpImageFilterNew_h
#define __itkGridForwardWarpImageFilterNew_h

#include "itkImageToImageFilter.h"
#include "itkFixedArray.h"

namespace itk
{
/** \class GridForwardWarpImageFilterNew
  * \brief Warps a grid using an input deformation field.
  *
  * GridForwardWarpImageFilterNew warps a grid with respect to
  * a given deformation field.
  *
  * A deformation field is represented as a image whose pixel type is some
  * vector type with at least N elements, where N is the dimension of
  * the input image. The vector type must support element access via operator
  * [].
  *
  * The output image is produced by forward mapping.
  *
  * Each vector in the deformation field represent the distance between
  * a geometric point in the input space and a point in the output space such
  * that:
  *
  * \f[ p_{in} = p_{out} + d \f]
  *
  * Typically the mapped position does not correspond to an integer pixel
  * position in the output image. We round it.
  *
  * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
  *
  * This implementation was taken from the Insight Journal paper:
  * http://hdl.handle.net/1926/510
  *
  */
template<
  class TDeformationField,
  class TOutputImage
  >
class ITK_EXPORT GridForwardWarpImageFilterNew:
  public ImageToImageFilter< TDeformationField, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef GridForwardWarpImageFilterNew                         Self;
  typedef ImageToImageFilter< TDeformationField, TOutputImage > Superclass;
  typedef SmartPointer< Self >                                  Pointer;
  typedef SmartPointer< const Self >                            ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods) */
  itkTypeMacro(GridForwardWarpImageFilterNew, ImageToImageFilter);

  /** Typedef to describe the output image region type. */
  typedef typename TOutputImage::RegionType OutputImageRegionType;

  /** Inherit some types from the superclass. */
  typedef typename Superclass::OutputImageType    OutputImageType;
  typedef typename Superclass::OutputImagePointer OutputImagePointer;
  typedef typename OutputImageType::IndexType     IndexType;
  typedef typename OutputImageType::SizeType      SizeType;
  typedef typename OutputImageType::PixelType     PixelType;
  typedef typename OutputImageType::SpacingType   SpacingType;

  /** Determine the image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TOutputImage::ImageDimension);
  itkStaticConstMacro(DeformationFieldDimension, unsigned int,
                      TDeformationField::ImageDimension);

  /** Deformation field typedef support. */
  typedef TDeformationField                           DeformationFieldType;
  typedef typename DeformationFieldType::ConstPointer DeformationFieldConstPointer;
  typedef typename DeformationFieldType::PixelType    DisplacementType;

  /** Set the background value */
  itkSetMacro(BackgroundValue, PixelType);
  /** Get the background value */
  itkGetConstMacro(BackgroundValue, PixelType);

  /** Set the foreground value */
  itkSetMacro(ForegroundValue, PixelType);
  /** Get the foreground value */
  itkGetConstMacro(ForegroundValue, PixelType);

  /** Set the spacing for the grids value, a spacing of 0 indicates that
    * displacements in that direction should be set to zero (thus keeping the
    *lines
    * in plane).  Negative grid spacings indicate that grid lines in that
    *direction should
    * not be rendered, but that in the multi-dimensional framework, the absolute
    *value of
    * that spacing should be used when deterimining which lines to render.
    * For example, if you want only Z-dir warped lines in a 2D X-dir view, then
    * set grid spacing to 0,-8,8.
    */
  typedef FixedArray< int, ImageDimension > GridSpacingType;
  itkSetMacro(GridPixelSpacing, GridSpacingType);
  /** Get the foreground value */
  itkGetConstMacro(GridPixelSpacing, GridSpacingType);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( SameDimensionCheck,
                   ( Concept::SameDimension< ImageDimension, DeformationFieldDimension > ) );
  itkConceptMacro( DeformationFieldHasNumericTraitsCheck,
                   ( Concept::HasNumericTraits< typename TDeformationField::PixelType::ValueType > ) );
  /** End concept checking */
#endif
protected:
  GridForwardWarpImageFilterNew();
  ~GridForwardWarpImageFilterNew() {}

  void PrintSelf(std::ostream & os, Indent indent) const;

  /**
    * GenerateData()
    */
  void GenerateData();

private:
  GridForwardWarpImageFilterNew(const Self &); // purposely not implemented
  void operator=(const Self &);                // purposely not implemented

  PixelType       m_BackgroundValue;
  PixelType       m_ForegroundValue;
  GridSpacingType m_GridPixelSpacing;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkGridForwardWarpImageFilterNew.txx"
#endif

#endif
