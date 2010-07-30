#ifndef _VDemonsRegistrator_h
#define _VDemonsRegistrator_h

#include "itkObject.h"
#include "itkVector.h"
#include "itkImage.h"
#include "itkVectorMultiResolutionPDEDeformableRegistration.h"
#include "itkRecursiveMultiResolutionPyramidImageFilter.h"
#include "itkVectorDiffeomorphicDemonsRegistrationFilter.h"

#include "itkArray.h"
#include "itkVectorImage.h"

namespace itk
{
/** \class VDemonsRegistrator
  *
  * This component computes the transform to register a
  * moving image onto a fixed image.
  *
  * In particular, it uses the deformable demons registration
  * algorithm.
  *
  * The registration is done using a multiresolution strategy.
  * At each resolution level, the downsampled images are obtained
  * using a RecursiveMultiResolutionPyramidImageFilter.
  *
  * \warning This class requires both images to be 3D.
  * It can write out the deformation field and the checker board image
  * of the fixed and output image.
  *
  * The registration process is activated by method Execute().
  *
  * Inputs:
  *   - pointer to fixed image
  *   - pointer to moving image
  *   - number of resolution levels
  *   - number of optimization iterations at each level
  *   - the initial rigid (quaternion) transform parameters
  *   - the coarest level shrink factors for the fixed image
  *   - the coarest level shrink factors for the moving image
  *
  * Outputs:
  *   - output deformation field
  *   - output image
  *   - Checkerboard image
  *   - x,y,z components of displacement fields.
  */
template<
  class TRealImage,
  class TOutputImage,
  class TFieldValue = ITK_TYPENAME TRealImage::PixelType
  >
class VDemonsRegistrator:public Object
{
public:

  /** Standard class typedefs. */
  typedef VDemonsRegistrator         Self;
  typedef Object                     Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(VDemonsRegistrator, Object);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Fixed Image Type. */
  typedef TRealImage                        RealImageType;
  typedef typename RealImageType::PixelType RealPixelType;

  /** Moving Image Type. */
  typedef typename TOutputImage::PixelType              PixelType;
  typedef typename RealImageType::Pointer               ImagePointer;
  typedef typename itk::VectorImage< RealPixelType, 3 > VectorImageType;

  typedef Array< float > WeightFactorType;

  /** Image dimension enumeration. */
  itkStaticConstMacro(ImageDimension, unsigned int, TRealImage::ImageDimension);

  /** Type to hold the number of checker boxes per dimension */
  typedef FixedArray< unsigned int,
                      ::itk::GetImageDimension< TRealImage >::ImageDimension >
  PatternArrayType;

  /** Set Checker pattern */
  itkSetMacro(CheckerBoardPattern, PatternArrayType);
  itkGetConstReferenceMacro(CheckerBoardPattern, PatternArrayType);

  /** Deformation field value type. */
  typedef TFieldValue FieldValueType;

  /** Deformation field pixel type. */
  typedef Vector< FieldValueType,
                  itkGetStaticConstMacro(ImageDimension) > FieldPixelType;

  /** Deformation field type. */
  typedef Image< FieldPixelType,
                 itkGetStaticConstMacro(ImageDimension) > TDeformationField;

  /** Fixed Image Pyramid Type. */
  typedef RecursiveMultiResolutionPyramidImageFilter<
    RealImageType,
    RealImageType >    FixedImagePyramidType;

  /** Moving Image Pyramid Type. */
  typedef RecursiveMultiResolutionPyramidImageFilter<
    RealImageType,
    RealImageType >   MovingImagePyramidType;

  /** Registration Method. */
  typedef MultiResolutionPDEDeformableRegistration<
    RealImageType,
    RealImageType,
    TDeformationField, float >    RegistrationType;

  typedef VectorMultiResolutionPDEDeformableRegistration<
    VectorImageType,
    VectorImageType,
    TDeformationField, float >    VectorRegistrationType;

  /** UnsignedIntArray type. */
  typedef Array< unsigned int > UnsignedIntArray;

  /** ShrinkFactorsArray type. */
  typedef FixedArray< unsigned int,
                      itkGetStaticConstMacro(ImageDimension) > ShrinkFactorsArray;

  /** Set the intial deformation field **/
  itkSetObjectMacro (InitialDeformationField, TDeformationField);

  /** Set the fixed image. */
  void SetFixedImage(std::vector< ImagePointer > & image)
  {
    m_FixedImage = image;
  }

  /** Set the moving image. */
  void SetMovingImage(std::vector< ImagePointer > & image)
  {
    m_MovingImage = image;
  }

  /** Set the Unnormalized moving image. */
  void SetUnNormalizedFixedImage(std::vector< ImagePointer > & image)
  {
    m_UnNormalizedFixedImage = image;
  }

  /** Set the Unnormalized moving image. */
  void SetUnNormalizedMovingImage(std::vector< ImagePointer > & image)
  {
    m_UnNormalizedMovingImage = image;
  }

  /** Set the number of resolution levels. */
  itkSetClampMacro( NumberOfLevels, unsigned short, 1,
                    NumericTraits< unsigned short >::max() );

  /** Set the number of iterations per level. */
  itkSetMacro(NumberOfIterations, UnsignedIntArray);

  /** Set the fixed and moving image shrink factors. */
  itkSetMacro(FixedImageShrinkFactors, ShrinkFactorsArray);
  itkSetMacro(MovingImageShrinkFactors, ShrinkFactorsArray);

  /** Set Displacementname */
  itkSetStringMacro(DisplacementBaseName);
  itkGetStringMacro(DisplacementBaseName);

  /** Set WarpedImageName */
  itkSetStringMacro(WarpedImageName);
  itkGetStringMacro(WarpedImageName);

  /** Set CheckerBoard ImageName */
  itkSetStringMacro(CheckerBoardFilename);
  itkGetStringMacro(CheckerBoardFilename);

  /** Set Deformation field output file Name */
  itkSetStringMacro(DeformationFieldOutputName);
  itkGetStringMacro(DeformationFieldOutputName);

  /**Set histogram matching*/
  itkSetMacro(UseHistogramMatching, bool);
  itkGetConstMacro(UseHistogramMatching, bool);

  /** Method to execute the registration. */
  virtual void Execute();

  /** Get the deformation field. */
  itkGetObjectMacro(DeformationField, TDeformationField);

  /** Initialize registration at the start of new level. */
  void StartNewLevel();

  /**Output Normalized Image.*/
  itkSetStringMacro(OutNormalized);
  itkGetStringMacro(OutNormalized);

  /**Set Debug mode*/
  itkSetMacro(OutDebug, bool);
  itkGetConstMacro(OutDebug,  bool);

  itkSetStringMacro (FixedLandmarkFilename);
  itkGetStringMacro (FixedLandmarkFilename);
  itkSetStringMacro (MovingLandmarkFilename);
  itkGetStringMacro (MovingLandmarkFilename);

  itkSetMacro(DefaultPixelValue, typename RealImageType::PixelType);
  itkGetMacro(DefaultPixelValue, typename RealImageType::PixelType);

  /** Get the interpolation Mode. */
  itkGetMacro(InterpolationMode, std::string);
  itkSetMacro(InterpolationMode, std::string);

  typedef itk::PDEDeformableRegistrationFilter< RealImageType, RealImageType,
                                                TDeformationField >
  BaseRegistrationFilterType;
  void SetRegistrationFilter(
    typename BaseRegistrationFilterType::Pointer filter)
  {
    this->m_Registration->SetRegistrationFilter(filter);
    //      this->m_VectorRegistration->SetRegistrationFilter(filter);
  }

  typedef itk::VectorDiffeomorphicDemonsRegistrationFilter< VectorImageType,
                                                            VectorImageType,
                                                            TDeformationField >
  VectorRegistrationFilterType;
  void SetVectorRegistrationFilter(
    typename VectorRegistrationFilterType::Pointer filter)
  {
    this->m_VectorRegistration->SetRegistrationFilter(filter);
  }

  void SetWeightFactors(const WeightFactorType & factors)
  {
    m_WeightFactors = factors;
  }

protected:
  VDemonsRegistrator();
  ~VDemonsRegistrator();
private:
  VDemonsRegistrator(const Self &);                // purposely not implemented
  void operator=(const Self &);                    // purposely not implemented

  void WriteDisplacementComponents();

  typename TDeformationField::Pointer m_InitialDeformationField;
  std::vector< ImagePointer > m_FixedImage;
  std::vector< ImagePointer > m_MovingImage;
  std::vector< ImagePointer > m_UnNormalizedMovingImage;
  std::vector< ImagePointer > m_UnNormalizedFixedImage;
  typename FixedImagePyramidType::Pointer m_FixedImagePyramid;
  typename MovingImagePyramidType::Pointer m_MovingImagePyramid;
  typename RegistrationType::Pointer m_Registration;
  typename VectorRegistrationType::Pointer m_VectorRegistration;
  typename RealImageType::PixelType m_DefaultPixelValue;

  unsigned short   m_NumberOfLevels;
  UnsignedIntArray m_NumberOfIterations;

  ShrinkFactorsArray m_MovingImageShrinkFactors;
  ShrinkFactorsArray m_FixedImageShrinkFactors;

  typename TDeformationField::Pointer m_DeformationField;
  std::string      m_FixedLandmarkFilename;
  std::string      m_MovingLandmarkFilename;
  unsigned long    m_Tag;
  unsigned long    m_VectorTag;
  std::string      m_DisplacementBaseName;
  std::string      m_WarpedImageName;
  std::string      m_CheckerBoardFilename;
  std::string      m_DeformationFieldOutputName;
  PatternArrayType m_CheckerBoardPattern;
  std::string      m_OutNormalized;
  bool             m_OutDebug;
  bool             m_UseHistogramMatching;
  typename VectorImageType::Pointer m_VectorFixedImage;
  typename VectorImageType::Pointer m_VectorMovingImage;

  WeightFactorType m_WeightFactors;
  std::string      m_InterpolationMode;
};
}   // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "VDemonsRegistrator.txx"
#endif

#endif
