#ifndef _VDemonsPreprocessor_h
#define _VDemonsPreprocessor_h

#include "itkObject.h"
#include "itkArray.h"

namespace itk
{
/** \class VDemonsPreprocessor
  *
  * This component pre-processes the moving and fixed image before
  * registration.
  * If the fixed image dimensions are different from the moving image it will
  *    * resample the moving image to match the fixed image dimensions.
  * Histogram matching is done to solve the intensity mismatch problem.
  *
  * The preprocessor also called the skull-stripping filter itkBOBF
  * if an atlas and subject whole brain masks are specified.
  *
  * The preprocessing is activatived by method Execute().
  *
  * Inputs:
  *    - pointer to original fixed image
  *    - pointer original moving image
  *    - number of histogram levels
  *    - number of match points
  *
  * Outputs:
  *    - pointer to processed fixed image
  *    - pointer to processed moving image
  *    - the minimum value of original fixed image
  *    - the minimum value of original moving image
  *
  */
template< typename TInputImage, typename TOutputImage >
class ITK_EXPORT VDemonsPreprocessor:public Object
{
public:

  /** Standard class typedefs. */
  typedef VDemonsPreprocessor        Self;
  typedef Object                     Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(VDemonsPreprocessor, Object);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Input Image Type. */
  typedef TInputImage InputImageType;
  /** Output Image Type. */
  typedef TOutputImage OutputImageType;

  /** Input image pixel type. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename InputImageType::Pointer    InputImagePointer;
  typedef typename OutputImageType::PixelType PixelType;
  typedef typename OutputImageType::IndexType IndexType;
  typedef typename OutputImageType::SizeType  SizeType;
  typedef typename OutputImageType::Pointer   OutputImagePointer;

  /** Image dimension enumeration. */
  itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

  /** Set the input fixed image. */
  void SetInputFixedImage(std::vector< InputImagePointer > & image)
  {
    m_InputFixedImage = image;
  }

  /** Set the input moving image. */
  void SetInputMovingImage(std::vector< InputImagePointer > & image)
  {
    m_InputMovingImage = image;
  }

  /** Deformation field value type. */
  typedef float FieldValueType;

  /** Deformation field pixel type. */
  typedef Vector< FieldValueType,
                  itkGetStaticConstMacro(ImageDimension) > FieldPixelType;

  /** Deformation field type. */
  typedef Image< FieldPixelType,
                 itkGetStaticConstMacro(ImageDimension) > TDeformationField;

  /** Set the initial Deformation Field. */
  itkSetObjectMacro(InitialDeformationField, TDeformationField);
  itkGetObjectMacro(InitialDeformationField, TDeformationField);

  /** Set the number of histogram levels to use. */
  itkSetMacro(NumberOfHistogramLevels, unsigned long);

  /** Set the number of match points to use. */
  itkSetMacro(NumberOfMatchPoints, unsigned long);

  /** Method to execute the preprocessing. */
  virtual void Execute();

  /** Get the output fixed image. */
  std::vector< OutputImagePointer > &  GetOutputFixedImage(void)
  {
    return m_OutputFixedImage;
  }

  /** Get the output moving image. */
  std::vector< OutputImagePointer > &  GetOutputMovingImage()
  {
    return m_OutputMovingImage;
  }

  /** Get the output moving image. */
  std::vector< OutputImagePointer > &  GetUnNormalizedMovingImage()
  {
    return m_UnNormalizedMovingImage;
  }

  /** Get the output moving image. */
  std::vector< OutputImagePointer > &  GetUnNormalizedFixedImage()
  {
    return m_UnNormalizedFixedImage;
  }

  /** Get minimum value of original fixed image. */
  itkGetMacro(FixedImageMinimum, InputPixelType);

  /** Get minimum value of original moving image. */
  itkGetMacro(MovingImageMinimum, InputPixelType);

  /*BOBF macros
   *Set Target Mask filename*/
  itkSetStringMacro(FixedBinaryVolume);
  itkGetStringMacro(FixedBinaryVolume);

  /**Set Template Mask filename*/
  itkSetStringMacro(MovingBinaryVolume);
  itkGetStringMacro(MovingBinaryVolume);

  /** Set/Get the lower threshold. The default is 0. */
  itkSetMacro(Lower, PixelType);
  itkGetMacro(Lower, PixelType);

  /** Set/Get the upper threshold. The default is 70 */
  itkSetMacro(Upper, PixelType);
  itkGetMacro(Upper, PixelType);

  itkSetMacro(DefaultPixelValue,  PixelType);
  itkGetMacro(DefaultPixelValue,  PixelType);

  itkSetMacro(MedianFilterSize,  SizeType);
  itkGetMacro(MedianFilterSize,  SizeType);

  /** Set the radius of the neighborhood used for a mask. */
  itkSetMacro(Radius, SizeType);
  /** Get the radius of the neighborhood used to compute the median */
  itkGetConstReferenceMacro(Radius, SizeType);

  /** Set the Seed of the neighborhood used for a mask. */
  itkSetMacro(Seed, IndexType);
  /** Get the radius of the neighborhood used to compute the median */
  itkGetConstReferenceMacro(Seed, IndexType);

  /**Set Debug mode*/
  itkSetMacro(OutDebug, bool);
  itkGetConstMacro(OutDebug, bool);

  /**Set histogram matching*/
  itkSetMacro(UseHistogramMatching, bool);
  itkGetConstMacro(UseHistogramMatching, bool);
protected:
  VDemonsPreprocessor();
  ~VDemonsPreprocessor()
  {}
private:
  VDemonsPreprocessor(const Self &);      // purposely not implemented
  void operator=(const Self &);           // purposely not implemented

  std::vector< InputImagePointer >  m_InputFixedImage;
  std::vector< InputImagePointer >  m_InputMovingImage;
  std::vector< OutputImagePointer > m_OutputFixedImage;
  std::vector< OutputImagePointer > m_OutputMovingImage;
  std::vector< OutputImagePointer > m_UnNormalizedMovingImage;
  std::vector< OutputImagePointer > m_UnNormalizedFixedImage;
  typename TDeformationField::Pointer m_InitialDeformationField;

  unsigned long m_NumberOfHistogramLevels;
  unsigned long m_NumberOfMatchPoints;

  InputPixelType m_FixedImageMinimum;
  InputPixelType m_MovingImageMinimum;

  std::string m_FixedBinaryVolume;
  std::string m_MovingBinaryVolume;
  IndexType   m_Seed;
  PixelType   m_Lower;
  PixelType   m_Upper;
  PixelType   m_DefaultPixelValue;
  SizeType    m_Radius;
  bool        m_OutDebug;
  SizeType    m_MedianFilterSize;

  bool m_UseHistogramMatching;

  /*MakeBOBF function takes in a brain image and a whole brain mask and strips
   * the
    * skull of the image.*/
  OutputImagePointer MakeBOBFImage(OutputImagePointer input,
                                   std::string MaskName);
};
}   // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "VDemonsPreprocessor.txx"
#endif

#endif // _VDemonsPreprocessor_h
