#ifndef __itkGridForwardWarpImageFilter_h
#define __itkGridForwardWarpImageFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{

/** \class GridForwardWarpImageFilter
 * \brief Warps a grid using an input deformation field.
 *
 * GridForwardWarpImageFilter warps a grid with respect to
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
 */
template <
  class TDeformationField,
  class TOutputImage
  >
class ITK_EXPORT GridForwardWarpImageFilter :
    public ImageToImageFilter<TDeformationField, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef GridForwardWarpImageFilter      Self;
  typedef ImageToImageFilter<TDeformationField,TOutputImage> Superclass;
  typedef SmartPointer<Self>         Pointer;
  typedef SmartPointer<const Self>   ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods) */
  itkTypeMacro( GridForwardWarpImageFilter, ImageToImageFilter );

  /** Typedef to describe the output image region type. */
  typedef typename TOutputImage::RegionType OutputImageRegionType;

  /** Inherit some types from the superclass. */
  typedef typename Superclass::OutputImageType       OutputImageType;
  typedef typename Superclass::OutputImagePointer    OutputImagePointer;
  typedef typename OutputImageType::IndexType        IndexType;
  typedef typename OutputImageType::SizeType         SizeType;
  typedef typename OutputImageType::PixelType        PixelType;
  typedef typename OutputImageType::SpacingType      SpacingType;

  /** Determine the image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TOutputImage::ImageDimension );
  itkStaticConstMacro(DeformationFieldDimension, unsigned int,
                      TDeformationField::ImageDimension );

  /** Deformation field typedef support. */
  typedef TDeformationField    DeformationFieldType;
  typedef typename DeformationFieldType::ConstPointer  DeformationFieldConstPointer;
  typedef typename DeformationFieldType::PixelType DisplacementType;

  /** Set the background value */
  itkSetMacro( BackgroundValue, PixelType );

  /** Get the background value */
  itkGetMacro( BackgroundValue, PixelType );

  /** Set the foreground value */
  itkSetMacro( ForegroundValue, PixelType );

  /** Get the foreground value */
  itkGetMacro( ForegroundValue, PixelType );


#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(SameDimensionCheck,
    (Concept::SameDimension<ImageDimension, DeformationFieldDimension>));
  itkConceptMacro(DeformationFieldHasNumericTraitsCheck,
    (Concept::HasNumericTraits<typename TDeformationField::PixelType::ValueType>));
  /** End concept checking */
#endif

protected:
  GridForwardWarpImageFilter();
  ~GridForwardWarpImageFilter() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

   /** 
   * GenerateData()
   */
  void GenerateData();

private:
  GridForwardWarpImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  PixelType                  m_BackgroundValue;
  PixelType                  m_ForegroundValue;
  unsigned int               m_GridPixSpacing;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGridForwardWarpImageFilter.txx"
#endif

#endif
