#ifndef __itkBOBFFilter_h
#define __itkBOBFFilter_h

#include "itkImageToImageFilter.h"
#include "itkFixedArray.h"

namespace itk
{
/** \class BOBFilter
  */
template< class TInputImage, class TOutputImage >
class ITK_EXPORT BOBFFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef BOBFFilter                                      Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(BOBFFilter, ImageToImageFilter);

  /** ImageDimension enumeration. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TInputImage::ImageDimension);

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
  typedef typename InputImageType::IndexType  IndexType;
  typedef typename InputImageType::SizeType   InputSizeType;

  /** Set/Get the Input image. */
  void SetInputImage(const InputImageType *source)
  {
    this->SetInput(source);
  }

  const InputImageType * GetInputImage(void)
  {
    return this->GetInput();
  }

  /** Set the input mask */
  void SetInputMask(const InputImageType *image);

  /** Get the input mask */
  const InputImageType * GetInputMask(void);

  /** Set seed point. */

  /** Set/Get the lower threshold. The default is 0. */
  itkSetMacro(Lower, InputPixelType);
  itkGetMacro(Lower, InputPixelType);

  /** Set/Get the upper threshold. The default is the largest possible
    *  value for the InputPixelType. */
  itkSetMacro(Upper, InputPixelType);
  itkGetMacro(Upper, InputPixelType);

  /** Set/Get value to replace thresholded pixels. Pixels that lie *
    *  within Lower and Upper (inclusive) will be replaced with this
    *  value. The default is 1. */
  itkSetMacro(ReplaceValue, OutputPixelType);
  itkGetMacro(ReplaceValue, OutputPixelType);

  /** Set the radius of the neighborhood used for a mask. */
  itkSetMacro(Radius, InputSizeType);

  /** Get the radius of the neighborhood used to compute the median */
  itkGetConstReferenceMacro(Radius, InputSizeType);

  /** Set the Seed of the neighborhood used for a mask. */
  itkSetMacro(Seed, IndexType);

  /** Get the radius of the neighborhood used to compute the median */
  itkGetConstReferenceMacro(Seed, IndexType);

  /** Method to execute the Filter */
  void GenerateData();

protected:
  BOBFFilter();
  ~BOBFFilter() {}
private:
  BOBFFilter(const Self &);      // purposely not implemented
  void operator=(const Self &);  // purposely not implemented

  void PrintSelf(std::ostream & os, Indent indent) const;

  // std::vector<IndexType> m_Seeds;
  IndexType       m_Seed;
  InputPixelType  m_Lower;
  InputPixelType  m_Upper;
  OutputPixelType m_ReplaceValue;
  InputSizeType   m_Radius;
};
}   // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkBOBFFilter.txx"
#endif

#endif
