#ifndef _VBRAINSDemonWarp_h
#define _VBRAINSDemonWarp_h

#include <string>
#include "VDemonsPreprocessor.h"
#include "VDemonsRegistrator.h"
#include "VValidationInputParser.h"
#include "ApplicationBase.h"
#include "itkCheckerBoardImageFilter.h"
#include "itkVectorDiffeomorphicDemonsRegistrationFilter.h"
#include "itkVectorImage.h"
namespace itk
{
/*This file defines Thirion registration class which initializes the input
  * parser, preprocessor and the registrator. */

template< typename TImage,
          typename TRealImage, typename TOutputImage
          >
class VBRAINSDemonWarp:public ApplicationBase<
    VValidationInputParser< TImage >,
    VDemonsPreprocessor< TImage, TRealImage >,
    VDemonsRegistrator< TRealImage, TOutputImage,
                        ITK_TYPENAME TRealImage::PixelType >
    >
{
public:

  /** Standard class typedefs. */
  typedef VBRAINSDemonWarp Self;
  typedef ApplicationBase< VValidationInputParser< TImage >,
                           VDemonsPreprocessor< TImage, TRealImage >,
                           VDemonsRegistrator< TRealImage, TRealImage,
                                               ITK_TYPENAME TRealImage::PixelType >
                           > Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Deformation field pixel type. */
  typedef float                       FieldValueType;
  typedef Vector< FieldValueType, 3 > FieldPixelType;
  typedef Image< FieldPixelType, 3 >  TDeformationField;

  /** Run-time type information (and related methods). */
  itkTypeMacro (VBRAINSDemonWarp, ApplicationBase);

  /** Method for creation through the object factory. */
  itkNewMacro (Self);

  /** Image types. */
  typedef TImage                       ImageType;
  typedef itk::VectorImage< float, 3 > VectorImageType;
  typedef TRealImage                   RealImageType;

  /** Image dimension. */
  itkStaticConstMacro (ImageDimension, unsigned int,
                       TImage::ImageDimension);

  /** Type to hold the number of checker boxes per dimension */
  typedef FixedArray< unsigned int, ::itk::GetImageDimension<
                        TImage >::ImageDimension > PatternArrayType;

  typedef typename ImageType::PixelType PixelType;
  typedef typename ImageType::IndexType IndexType;
  typedef typename ImageType::SizeType  SizeType;

  /** ShrinkFactors type. */
  typedef FixedArray< unsigned int,
                      itk::GetImageDimension< TImage >::ImageDimension >
  ShrinkFactorsType;

  /** IterationArray type. */
  typedef Array< unsigned int > IterationsArrayType;
  typedef Array< float >        WeightFactorType;

  void SetTheFixedImageFilename(std::vector< std::string > & names)
  {
    m_TheFixedImageFilename = names;
  }

  void SetTheMovingImageFilename(std::vector< std::string > & names)
  {
    m_TheMovingImageFilename = names;
  }

  /** Set the initial Displacement Field one of 3 ways. */
  itkSetStringMacro (InitialDeformationFieldFilename);
  itkGetStringMacro (InitialDeformationFieldFilename);

  itkSetStringMacro (InitialCoefficientFilename);
  itkGetStringMacro (InitialCoefficientFilename);

  itkSetStringMacro (InitialTransformFilename);
  itkGetStringMacro (InitialTransformFilename);

  /** Set Displacementname */
  itkSetStringMacro (DisplacementBaseName);
  itkGetStringMacro (DisplacementBaseName);
  /** Set WarpedImageName */
  itkSetStringMacro (WarpedImageName);
  itkGetStringMacro (WarpedImageName);

  /** Set input parameter file */
  // itkSetStringMacro (ParameterFilename);

  /** Set output transformation filename. */
  itkSetStringMacro (OutputFilename);

  /**Set checker board Image filename*/
  itkSetStringMacro (CheckerBoardFilename);
  itkGetStringMacro (CheckerBoardFilename);

  /**Set Deformation field output filename*/
  itkSetStringMacro (DeformationFieldOutputName);
  itkGetStringMacro (DeformationFieldOutputName);

  /** Set Checker pattern */
  itkSetMacro (CheckerBoardPattern, PatternArrayType);
  itkGetConstReferenceMacro (CheckerBoardPattern, PatternArrayType);

  /** Set append output file boolean. */
  itkSetMacro (AppendOutputFile, bool);
  itkGetMacro (AppendOutputFile, bool);
  itkBooleanMacro (AppendOutputFile);

  /*BOBF macros
   *Set Target Mask filename*/
  itkSetStringMacro (FixedBinaryVolume);
  itkGetStringMacro (FixedBinaryVolume);

  /**Set Template Mask filename*/
  itkSetStringMacro (MovingBinaryVolume);
  itkGetStringMacro (MovingBinaryVolume);

  /**Force Centered Image.*/
  itkSetMacro (ForceCoronalZeroOrigin, bool);
  itkGetConstMacro (ForceCoronalZeroOrigin, bool);

  /**Output Normalized Image.*/
  itkSetStringMacro (OutNormalized);
  itkGetStringMacro (OutNormalized);

  /** Set/Get the lower threshold. The default is 0. */
  itkSetMacro (Lower, PixelType);
  itkGetMacro (Lower, PixelType);

  /** Set/Get the upper threshold. The default is 70 */
  itkSetMacro (Upper, PixelType);
  itkGetMacro (Upper, PixelType);

  /** Set/Get value to replace thresholded pixels. Pixels that lie *
    *  within Lower and Upper (inclusive) will be replaced with this
    *  value. The default is 1. */
  itkSetMacro (DefaultPixelValue, PixelType);
  itkGetMacro (DefaultPixelValue, PixelType);

  /** Set the radius of the neighborhood used for a mask. */
  itkSetMacro (Radius, SizeType);
  /** Get the radius of the neighborhood used to compute the median */
  itkGetConstReferenceMacro (Radius, SizeType);

  /** Set the Seed of the neighborhood used for a mask. */
  itkSetMacro (Seed, IndexType);
  /** Get the radius of the neighborhood used to compute the median */
  itkGetConstReferenceMacro (Seed, IndexType);

  itkSetMacro(MedianFilterSize,  SizeType);
  itkGetMacro(MedianFilterSize,  SizeType);

  /** Set the initial deformation field to prime registration */
  //    itkSetObjectMacro(InitialDeformationField,TDeformationField);
  /** Set the Input Landmark Filename*/
  itkSetStringMacro (FixedLandmarkFilename);
  itkGetStringMacro (FixedLandmarkFilename);
  itkSetStringMacro (MovingLandmarkFilename);
  itkGetStringMacro (MovingLandmarkFilename);

  /**Set histogram matching*/
  itkSetMacro(UseHistogramMatching, bool);
  itkGetConstMacro(UseHistogramMatching, bool);

  /** Get the number of histogram bins. */
  itkGetConstMacro(NumberOfHistogramLevels, unsigned long);
  itkSetMacro(NumberOfHistogramLevels, unsigned long);

  /** Get the number of match points. */
  itkGetConstMacro(NumberOfMatchPoints, unsigned long);
  itkSetMacro(NumberOfMatchPoints, unsigned long);

  /** Get the number of levels. */
  itkGetMacro(NumberOfLevels, unsigned short);
  itkSetMacro(NumberOfLevels, unsigned short);

  /** Get the interpolation Mode. */
  itkGetMacro(InterpolationMode, std::string);
  itkSetMacro(InterpolationMode, std::string);

  /** Get the atlas image starting shrink factors. */
  itkGetConstReferenceMacro(TheMovingImageShrinkFactors, ShrinkFactorsType);
  void SetTheMovingImageShrinkFactors(const ShrinkFactorsType & shrinkfactors)
  {
    this->m_TheMovingImageShrinkFactors = shrinkfactors;
  }

  /** Get the subject image starting shrink factors. */
  itkGetConstReferenceMacro(TheFixedImageShrinkFactors, ShrinkFactorsType);
  void SetTheFixedImageShrinkFactors(const ShrinkFactorsType & shrinkfactors)
  {
    this->m_TheFixedImageShrinkFactors = shrinkfactors;
  }

  /** Get the number of iterations at each level. */
  itkGetConstReferenceMacro(NumberOfIterations, IterationsArrayType);
  void SetNumberOfIterations(const IterationsArrayType & iterations)
  {
    m_NumberOfIterations = iterations;
  }

  itkGetConstReferenceMacro(WeightFactors, WeightFactorType);
  void SetWeightFactors(const WeightFactorType & factors)
  {
    m_WeightFactors = factors;
  }

  typedef itk::PDEDeformableRegistrationFilter< RealImageType, RealImageType,
                                                TDeformationField >
  BaseRegistrationFilterType;
  void SetRegistrationFilter(
    typename BaseRegistrationFilterType::Pointer filter)
  {
    this->m_Registrator->SetRegistrationFilter(filter);
  }

  typedef itk::VectorDiffeomorphicDemonsRegistrationFilter< VectorImageType,
                                                            VectorImageType,
                                                            TDeformationField >
  VectorRegistrationFilterType;
  void SetVectorRegistrationFilter(
    typename VectorRegistrationFilterType::Pointer filter)
  {
    this->m_Registrator->SetVectorRegistrationFilter(filter);
  }

protected:

  VBRAINSDemonWarp ();
  virtual ~VBRAINSDemonWarp ()
  {}

  /** Initialize the input parser. */
  virtual void InitializeParser();

  /*** Initialize the preprocessor */
  virtual void InitializePreprocessor();

  /*** Initialize the registrator  */
  virtual void InitializeRegistrator();

private:

  std::vector< std::string > m_TheMovingImageFilename;
  std::vector< std::string > m_TheFixedImageFilename;
  std::string                m_InitialDeformationFieldFilename;
  std::string                m_InitialCoefficientFilename;
  std::string                m_InitialTransformFilename;
  std::string                m_DisplacementBaseName;
  std::string                m_WarpedImageName;

  // std::string m_ParameterFilename;
  bool m_ForceCoronalZeroOrigin;
  bool m_UseHistogramMatching;

  std::string m_OutNormalized;
  //    std::string m_OutDebug;
  std::string      m_OutputFilename;
  std::string      m_CheckerBoardFilename;
  std::string      m_DeformationFieldOutputName;
  bool             m_AppendOutputFile;
  PatternArrayType m_CheckerBoardPattern;
  std::string      m_FixedBinaryVolume;
  std::string      m_MovingBinaryVolume;
  IndexType        m_Seed;
  PixelType        m_Lower;
  PixelType        m_Upper;
  PixelType        m_DefaultPixelValue;
  SizeType         m_Radius;   // for BOBF filter.
  SizeType         m_MedianFilterSize;
  std::string      m_FixedLandmarkFilename;
  std::string      m_MovingLandmarkFilename;
  // typename TDeformationField::Pointer m_InitialDeformationField;
  unsigned long       m_NumberOfHistogramLevels;
  unsigned long       m_NumberOfMatchPoints;
  unsigned short      m_NumberOfLevels;
  ShrinkFactorsType   m_TheMovingImageShrinkFactors;
  ShrinkFactorsType   m_TheFixedImageShrinkFactors;
  IterationsArrayType m_NumberOfIterations;
  WeightFactorType    m_WeightFactors;
  std::string         m_InterpolationMode;
};
}          // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "VBRAINSDemonWarp.txx"
#endif
#endif
