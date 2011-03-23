#ifndef  __BRAINSFitHelperTemplate_h
#define  __BRAINSFitHelperTemplate_h

/**
  * \author Hans J. Johnson
  *
  * The intension of the BRIANSFitHelperTemplate is to provide a
  * class that can be used in other programs in a way that is very similar to
  * the command line version of the program from the SlicerExecutionModel
  * version of the BRAINSFitPrimary program.
  *
  * Almost all the command line options are available in this version, but
  * there is no need to read or write files to disk in order to use this class.
  */
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>

#include "itkIO.h"
#include "itkVector.h"
#include "itkMedianImageFilter.h"
#include "itkHistogramMatchingImageFilter.h"

#include "itkCenteredVersorTransformInitializer.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkVersorTransformOptimizer.h"
#include "itkMultiThreader.h"
#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkImageMaskSpatialObject.h"

#include "itkFindCenterOfBrainFilter.h"

// TODO:  This needs to be moved to the top, and header files moved to this
// header where needed.
#include "BRAINSFitBSpline.h"
#include "BRAINSFitUtils.h"

#include "ConvertToRigidAffine.h"
#include "genericRegistrationHelper.h"
#include "ReadMask.h"
#include "BRAINSMacro.h"
#include "GenericTransformImage.h"
#include "BRAINSCommonLibWin32Header.h"

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

template <class FixedImageType, class MovingImageType>
class BRAINSFitHelperTemplate: public Object
{
public:
  /** Standard class typedefs. */
  typedef BRAINSFitHelperTemplate            Self;
  typedef ProcessObject              Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef typename FixedImageType::ConstPointer FixedImageConstPointer;
  typedef typename FixedImageType::Pointer      FixedImagePointer;

  typedef typename MovingImageType::ConstPointer MovingImageConstPointer;
  typedef typename MovingImageType::Pointer      MovingImagePointer;

  typedef typename itk::ImageToImageMetric<FixedImageType,MovingImageType> MetricType;

  /** Constants for the image dimensions */
  itkStaticConstMacro(FixedImageDimension, unsigned int, FixedImageType::ImageDimension);
  itkStaticConstMacro(MovingImageDimension, unsigned int, MovingImageType::ImageDimension);

  typedef SpatialObject< itkGetStaticConstMacro(FixedImageDimension) >  FixedBinaryVolumeType;
  typedef SpatialObject< itkGetStaticConstMacro(MovingImageDimension) > MovingBinaryVolumeType;
  typedef typename FixedBinaryVolumeType::Pointer  FixedBinaryVolumePointer;
  typedef typename MovingBinaryVolumeType::Pointer MovingBinaryVolumePointer;


  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(BRAINSFitHelperTemplate, ProcessObject);

  /** Set/Get the Fixed image. */
  itkSetObjectMacro(FixedVolume, FixedImageType);
  itkGetConstObjectMacro(FixedVolume, FixedImageType);

  /** Set/Get the Moving image. */
  itkSetObjectMacro(MovingVolume, MovingImageType)
  itkGetConstObjectMacro(MovingVolume, MovingImageType);

  itkSetObjectMacro (FixedBinaryVolume, FixedBinaryVolumeType);
  itkGetConstObjectMacro(FixedBinaryVolume, FixedBinaryVolumeType);
  itkSetObjectMacro(MovingBinaryVolume, MovingBinaryVolumeType);
  itkGetConstObjectMacro(MovingBinaryVolume, MovingBinaryVolumeType);

  itkSetMacro     (OutputFixedVolumeROI,  std::string);
  itkGetConstMacro(OutputFixedVolumeROI,  std::string);
  itkSetMacro     (OutputMovingVolumeROI, std::string);
  itkGetConstMacro(OutputMovingVolumeROI, std::string);

  itkSetObjectMacro(CostMetricObject,MetricType);
  itkGetObjectMacro(CostMetricObject,MetricType);

  //TODO:  This should be converted to use the
  //       interpolation mechanisms from GenericTransform
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
  itkSetMacro(UseExplicitPDFDerivativesMode, std::string);
  itkGetConstMacro(UseExplicitPDFDerivativesMode, std::string);
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

protected:
  BRAINSFitHelperTemplate();
  virtual ~BRAINSFitHelperTemplate() {}

  void PrintSelf(std::ostream & os, Indent indent) const;

  /** Method invoked by the pipeline in order to trigger the computation of
    * the registration. */
  void  GenerateData();

  /** instantiate and call the Registration Helper */
  template <class TransformType,
            class OptimizerType,
            class MetricType>
    void FitCommonCode(int numberOfIterations,
                       double minimumStepLength,
                       typename TransformType::Pointer &initialITKTransform);
private:

  BRAINSFitHelperTemplate(const Self &); // purposely not implemented
  void operator=(const Self &);  // purposely not implemented

  FixedImagePointer  m_FixedVolume;
  MovingImagePointer m_MovingVolume;

  FixedBinaryVolumePointer  m_FixedBinaryVolume;
  MovingBinaryVolumePointer m_MovingBinaryVolume;
  std::string               m_OutputFixedVolumeROI;
  std::string               m_OutputMovingVolumeROI;
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
  double                     m_BackgroundFillValue;
  std::vector< std::string > m_TransformType;
  std::string                m_InitializeTransformMode;
  std::string                m_UseExplicitPDFDerivativesMode;
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
  typename MetricType::Pointer                 m_CostMetricObject;
};  // end BRAINSFitHelperTemplate class
}   // end namespace itk

#include "BRAINSFitHelperTemplate.txx"

#endif  // __BRAINSFITHELPER__
