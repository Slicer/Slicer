#ifndef  __BRAINSFitHelper_h
#define  __BRAINSFitHelper_h

/**
  * \author Hans J. Johnson
  *
  * The intension of the BRIANSFitHelper is to provide a simple non-templated
  * class that can be used in other programs in a way that is very similar to
  * the command line version of the program from the SlicerExecutionModel
  * version of the BRAINSFitPrimary program.
  *
  * Almost all the command line options are available in this version, but
  * there is no need to read or write files to disk in order to use this class.
  *
  */
#include <fstream>
#include <vector>
#include <string>

#include "BRAINSCommonLibWin32Header.h"

// TODO:  This needs to be moved to the top, and header files moved to this
// header where needed.
#include "BRAINSFitBSpline.h"
#include "BRAINSFitUtils.h"

#include "itkFindCenterOfBrainFilter.h"
#include "itkMedianImageFilter.h"
#include "itkHistogramMatchingImageFilter.h"

#include "itkIO.h"

#include "itkCenteredVersorTransformInitializer.h"
#include "itkCenteredTransformInitializer.h"

#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkVersorTransformOptimizer.h"

#include "itkVector.h"
#include "itkMultiThreader.h"

#include "itkExtractImageFilter.h"

#include "itkResampleImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkAffineTransform.h"
#include <stdio.h>
#include "ConvertToRigidAffine.h"

#include "itkMultiModal3DMutualRegistrationHelper.h"
#include "itkMattesMutualInformationImageToImageMetric.h"

#include "ReadMask.h"
#include "BRAINSMacro.h"

#include "itkImageMaskSpatialObject.h"

#include "GenericTransformImage.h"

typedef itk::SpatialObject< 3 >    SpatialObjectType;
typedef SpatialObjectType::Pointer ImageMaskPointer;

namespace itk
{
/** Method for verifying that the ordering of the transformTypes is consistent
  * with converting routines. */
BRAINSCommonLib_EXPORT extern void ValidateTransformRankOrdering(const std::vector< std::string > & transformType);
}

namespace itk
{
class BRAINSCommonLib_EXPORT BRAINSFitHelper:public Object
{
public:
  /** Standard class typedefs. */
  typedef BRAINSFitHelper            Self;
  typedef ProcessObject              Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef float                         PixelType;
  typedef itk::Image< PixelType, 3 >    FixedVolumeType;
  typedef FixedVolumeType::ConstPointer FixedImageConstPointer;
  typedef FixedVolumeType::Pointer      FixedImagePointer;

  typedef itk::Image< PixelType, 3 >     MovingVolumeType;
  typedef MovingVolumeType::ConstPointer MovingImageConstPointer;
  typedef MovingVolumeType::Pointer      MovingImagePointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(BRAINSFitHelper, ProcessObject);

  /** Set/Get the Fixed image. */
  itkSetObjectMacro(FixedVolume, FixedVolumeType);
  itkGetConstObjectMacro(FixedVolume, FixedVolumeType);

  /** Set/Get the Moving image. */
  itkSetObjectMacro(MovingVolume, MovingVolumeType)
  itkGetConstObjectMacro(MovingVolume, MovingVolumeType);

  /** The preprocessedMoving volume SHOULD NOT BE SET, you can get it out of the
    *  algorithm.*/
  itkGetConstObjectMacro(PreprocessedMovingVolume, MovingVolumeType);

  typedef MattesMutualInformationImageToImageMetric< FixedVolumeType, MovingVolumeType >
  MetricType;
  typedef MetricType::FixedImageMaskType
  FixedBinaryVolumeType;
  typedef FixedBinaryVolumeType::Pointer
  FixedBinaryVolumePointer;
  typedef MetricType::MovingImageMaskType
  MovingBinaryVolumeType;
  typedef MovingBinaryVolumeType::Pointer
  MovingBinaryVolumePointer;

  itkSetObjectMacro (FixedBinaryVolume, FixedBinaryVolumeType);
  itkGetConstObjectMacro(FixedBinaryVolume, FixedBinaryVolumeType);

  itkSetObjectMacro(MovingBinaryVolume, MovingBinaryVolumeType);
  itkGetConstObjectMacro(MovingBinaryVolume, MovingBinaryVolumeType);

  typedef enum {
    LINEAR_INTERP = 0,
    WINDOWSINC_INTERP = 1,
    } InterpolationType;

  itkSetMacro(NumberOfSamples,        unsigned int);
  itkGetConstMacro(NumberOfSamples,        unsigned int);
  itkSetMacro(NumberOfHistogramBins,         unsigned int);
  itkGetConstMacro(NumberOfHistogramBins,         unsigned int);
  itkSetMacro(NumberOfMatchPoints,           unsigned int);
  itkGetConstMacro(NumberOfMatchPoints,           unsigned int);
  VECTORitkSetMacro(NumberOfIterations,   std::vector< int > /**/);
  VECTORitkSetMacro(MinimumStepLength, std::vector< double > );
  itkSetMacro(MaximumStepLength,             double);
  itkGetConstMacro(MaximumStepLength,             double);
  itkSetMacro(RelaxationFactor,              double);
  itkGetConstMacro(RelaxationFactor,              double);
  itkSetMacro(TranslationScale,              double);
  itkGetConstMacro(TranslationScale,              double);
  itkSetMacro(ReproportionScale,             double);
  itkGetConstMacro(ReproportionScale,             double);
  itkSetMacro(SkewScale,                     double);
  itkGetConstMacro(SkewScale,                     double);
  itkSetMacro(UseExplicitPDFDerivativesMode, std::string);
  itkGetConstMacro(UseExplicitPDFDerivativesMode, std::string);
  itkSetMacro(UseCachingOfBSplineWeightsMode, std::string);
  itkGetConstMacro(UseCachingOfBSplineWeightsMode, std::string);
  itkSetMacro(CostFunctionConvergenceFactor, double);
  itkGetConstMacro(CostFunctionConvergenceFactor, double);
  itkSetMacro(ProjectedGradientTolerance,    double);
  itkGetConstMacro(ProjectedGradientTolerance,    double);
  itkSetMacro(MaxBSplineDisplacement,        double);
  itkGetConstMacro(MaxBSplineDisplacement,        double);
  itkSetMacro(BackgroundFillValue,           double);
  itkGetConstMacro(BackgroundFillValue,           double);
  VECTORitkSetMacro(TransformType, std::vector< std::string > );
  itkSetMacro(InitializeTransformMode, std::string);
  itkGetConstMacro(InitializeTransformMode, std::string);
  itkSetMacro(MaskInferiorCutOffFromCenter, double);
  itkGetConstMacro(MaskInferiorCutOffFromCenter, double);
  itkSetMacro(CurrentGenericTransform,  GenericTransformType::Pointer);
  itkGetConstMacro(CurrentGenericTransform,  GenericTransformType::Pointer);
  VECTORitkSetMacro(SplineGridSize, std::vector< int >       );

  itkGetConstMacro(ActualNumberOfIterations,      unsigned int);
  itkGetConstMacro(PermittedNumberOfIterations,   unsigned int);

  itkGetConstMacro(FinalMetricValue,         double);
  /** Set/Get the Debugging level for filter verboseness */
  itkSetMacro(DebugLevel, unsigned int);
  itkGetConstMacro(DebugLevel, unsigned int);
  itkSetMacro(DisplayDeformedImage, bool);
  itkGetConstMacro(DisplayDeformedImage, bool);
  itkSetMacro(PromptUserAfterDisplay, bool);
  itkGetConstMacro(PromptUserAfterDisplay, bool);
  itkSetMacro(ObserveIterations,        bool);
  itkGetConstMacro(ObserveIterations,        bool);

  const std::vector< GenericTransformType::Pointer > * GetGenericTransformListPtr()
  {
    return &m_GenericTransformList;
  }

  /** Method to set the Permission to vary by level  */
  void SetPermitParameterVariation(std::vector< int > perms)
  {
    m_PermitParameterVariation.resize( perms.size() );
    for ( unsigned int i = 0; i < perms.size(); i++ )
      {
      m_PermitParameterVariation[i] = perms[i];
      }
  }

  itkSetMacro(HistogramMatch, bool);
  itkGetConstMacro(HistogramMatch, bool);

  /** Method that initiates the registration. */
  void StartRegistration(void);

  void PrintCommandLine(const bool dumpTempVolumes, const std::string suffix) const;

protected:
  BRAINSFitHelper();
  virtual ~BRAINSFitHelper() {}

  void PrintSelf(std::ostream & os, Indent indent) const;

  /** Method invoked by the pipeline in order to trigger the computation of
    * the registration. */
  void  GenerateData();

private:
  BRAINSFitHelper(const Self &); // purposely not implemented
  void operator=(const Self &);  // purposely not implemented

  FixedImagePointer  m_FixedVolume;
  MovingImagePointer m_MovingVolume;
  MovingImagePointer m_PreprocessedMovingVolume;

  FixedBinaryVolumePointer  m_FixedBinaryVolume;
  MovingBinaryVolumePointer m_MovingBinaryVolume;
  std::vector< int >        m_PermitParameterVariation;

  unsigned int m_NumberOfSamples;
  unsigned int m_NumberOfHistogramBins;
  bool         m_HistogramMatch;
  unsigned int m_NumberOfMatchPoints;
  // TODO:  Would be better to have unsigned int
  std::vector< int >         m_NumberOfIterations;
  double                     m_MaximumStepLength;
  std::vector< double >      m_MinimumStepLength;
  double                     m_RelaxationFactor;
  double                     m_TranslationScale;
  double                     m_ReproportionScale;
  double                     m_SkewScale;
  std::string                m_UseExplicitPDFDerivativesMode;
  std::string                m_UseCachingOfBSplineWeightsMode;
  double                     m_BackgroundFillValue;
  std::vector< std::string > m_TransformType;
  std::string                m_InitializeTransformMode;
  double                     m_MaskInferiorCutOffFromCenter;
  std::vector< int >         m_SplineGridSize;
  double                     m_CostFunctionConvergenceFactor;
  double                     m_ProjectedGradientTolerance;
  double                     m_MaxBSplineDisplacement;
  unsigned int               m_ActualNumberOfIterations;
  unsigned int               m_PermittedNumberOfIterations;
  // unsigned int             m_AccumulatedNumberOfIterationsForAllLevels;
  unsigned int                                 m_DebugLevel;
  GenericTransformType::Pointer                m_CurrentGenericTransform;
  std::vector< GenericTransformType::Pointer > m_GenericTransformList;
  bool                                         m_DisplayDeformedImage;
  bool                                         m_PromptUserAfterDisplay;
  double                                       m_FinalMetricValue;
  bool                                         m_ObserveIterations;
};  // end BRAINSFitHelper class
}   // end namespace itk

#endif  // __BRAINSFITHELPER__
