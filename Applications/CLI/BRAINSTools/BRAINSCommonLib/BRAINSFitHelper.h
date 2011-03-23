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
#include <stdio.h>
#include "itkImageRandomNonRepeatingConstIteratorWithIndex.h"

#include "BRAINSCommonLibWin32Header.h"

// TODO:  This needs to be moved to the top, and header files moved to this
// header where needed.
#include "BRAINSFitHelperTemplate.h"
#include "BRAINSFitBSpline.h"
#include "BRAINSFitUtils.h"

#include "itkIO.h"
#include "GenericTransformImage.h"
#include "itkFindCenterOfBrainFilter.h"
#include "ReadMask.h"
#include "BRAINSMacro.h"

typedef itk::SpatialObject< 3 >    SpatialObjectType;
typedef SpatialObjectType::Pointer ImageMaskPointer;

namespace itk
{
/** Method for verifying that the ordering of the transformTypes is consistent
  * with converting routines. */
BRAINSCommonLib_EXPORT extern void
ValidateTransformRankOrdering(const std::vector< std::string > & transformType);
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

  /** Constants for the image dimensions */
  itkStaticConstMacro(FixedImageDimension, unsigned int, FixedVolumeType::ImageDimension);
  itkStaticConstMacro(MovingImageDimension, unsigned int, MovingVolumeType::ImageDimension);

  typedef SpatialObject< itkGetStaticConstMacro(FixedImageDimension) >  FixedBinaryVolumeType;
  typedef SpatialObject< itkGetStaticConstMacro(MovingImageDimension) > MovingBinaryVolumeType;
  typedef FixedBinaryVolumeType::Pointer  FixedBinaryVolumePointer;
  typedef MovingBinaryVolumeType::Pointer MovingBinaryVolumePointer;

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
  itkGetObjectMacro(PreprocessedMovingVolume, MovingVolumeType);


  itkSetObjectMacro (FixedBinaryVolume, FixedBinaryVolumeType);
  itkGetConstObjectMacro(FixedBinaryVolume, FixedBinaryVolumeType);
  itkSetObjectMacro(MovingBinaryVolume, MovingBinaryVolumeType);
  itkGetConstObjectMacro(MovingBinaryVolume, MovingBinaryVolumeType);

  itkSetMacro     (OutputFixedVolumeROI,  std::string);
  itkGetConstMacro(OutputFixedVolumeROI,  std::string);
  itkSetMacro     (OutputMovingVolumeROI, std::string);
  itkGetConstMacro(OutputMovingVolumeROI, std::string);

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

  itkSetMacro(CostMetric, std::string);
  itkGetConstMacro(CostMetric, std::string);

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

  template<class TLocalCostMetric>
    void SetupRegistration();
  template<class TLocalCostMetric>
    void RunRegistration();
  template<class TLocalCostMetric>
    typename TLocalCostMetric::Pointer GetCostMetric();

  FixedImagePointer  m_FixedVolume;
  MovingImagePointer m_MovingVolume;
  MovingImagePointer m_PreprocessedMovingVolume;

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
  std::string                                  m_CostMetric;
  itk::Object::Pointer                         m_Helper;
};  // end BRAINSFitHelper class

template<class TLocalCostMetric>
void
BRAINSFitHelper::
SetupRegistration()
{

  typedef typename TLocalCostMetric::FixedImageType FixedImageType;
  typedef typename TLocalCostMetric::MovingImageType MovingImageType;
  typedef typename itk::BRAINSFitHelperTemplate<FixedImageType,MovingImageType> HelperType;

  typedef typename itk::LinearInterpolateImageFunction< MovingVolumeType, double > InterpolatorType;
  typename InterpolatorType::Pointer localLinearInterpolator = InterpolatorType::New();
  //
  // set up cost metric
  typename TLocalCostMetric::Pointer localCostMetric = TLocalCostMetric::New();
  localCostMetric->ReinitializeSeed(76926294);
  localCostMetric->SetInterpolator(localLinearInterpolator);
  localCostMetric->SetFixedImage(this->m_FixedVolume);
  localCostMetric->SetFixedImageRegion( this->m_FixedVolume->GetLargestPossibleRegion() );
  localCostMetric->SetMovingImage(this->m_PreprocessedMovingVolume);


  if ( this->m_MovingBinaryVolume.IsNotNull() )
    {
    localCostMetric->SetMovingImageMask(this->m_MovingBinaryVolume);
    }
  if ( this->m_FixedBinaryVolume.IsNotNull() )
    {
    localCostMetric->SetUseAllPixels(false);
    //Convert to using list of samples that are desired.
    // Do not set this if using explicit samples localCostMetric->SetFixedImageMask(this->m_FixedBinaryVolume);
    typename TLocalCostMetric::FixedImageIndexContainer myListOfIndexLocations;
    myListOfIndexLocations.reserve(this->m_NumberOfSamples);
    itk::ImageRandomNonRepeatingConstIteratorWithIndex<FixedImageType> NRit( this->m_FixedVolume,
      this->m_FixedVolume->GetBufferedRegion() );
    NRit.SetNumberOfSamples(this->m_FixedVolume->GetBufferedRegion().GetNumberOfPixels());
    NRit.GoToBegin();
    if( this->m_NumberOfSamples == 0 )
      {
      this->m_NumberOfSamples = this->m_FixedVolume->GetBufferedRegion().GetNumberOfPixels();
      }
    size_t currentCount=0;
    while( ( ! NRit.IsAtEnd() ) && ( currentCount < this->m_NumberOfSamples ) )
      {
      typename FixedImageType::PointType testPoint;
      this->m_FixedVolume->TransformIndexToPhysicalPoint(NRit.GetIndex(),testPoint);
      if(this->m_FixedBinaryVolume->IsInside(testPoint) )
        {
        myListOfIndexLocations.push_back(NRit.GetIndex());
        currentCount++;
        }
      ++NRit;
      }
    assert(currentCount != 0 );
    this->m_NumberOfSamples=myListOfIndexLocations.size();
    localCostMetric->SetNumberOfFixedImageSamples(myListOfIndexLocations.size());
    localCostMetric->SetNumberOfSpatialSamples(this->m_NumberOfSamples);
    localCostMetric->SetFixedImageIndexes(myListOfIndexLocations);
    }
  else
    {
    if ( this->m_NumberOfSamples > 0 )
      {
      localCostMetric->SetNumberOfSpatialSamples(this->m_NumberOfSamples);
      }
    else
      {
      localCostMetric->SetUseAllPixels(true);
      }
    }

  typename HelperType::Pointer
    myHelper = BRAINSFitHelperTemplate<FixedImageType,MovingImageType>::New();
  myHelper->SetTransformType(this->m_TransformType);
  myHelper->SetFixedVolume(this->m_FixedVolume);
  myHelper->SetMovingVolume(this->m_PreprocessedMovingVolume);
  myHelper->SetHistogramMatch(this->m_HistogramMatch);
  myHelper->SetNumberOfMatchPoints(this->m_NumberOfMatchPoints);
  myHelper->SetFixedBinaryVolume(this->m_FixedBinaryVolume);
  myHelper->SetMovingBinaryVolume(this->m_MovingBinaryVolume);
  myHelper->SetOutputFixedVolumeROI(this->m_OutputFixedVolumeROI);
  myHelper->SetOutputMovingVolumeROI(this->m_OutputMovingVolumeROI);
  myHelper->SetPermitParameterVariation(this->m_PermitParameterVariation);
  myHelper->SetNumberOfSamples(this->m_NumberOfSamples);
  myHelper->SetNumberOfHistogramBins(this->m_NumberOfHistogramBins);
  myHelper->SetNumberOfIterations(this->m_NumberOfIterations);
  myHelper->SetMaximumStepLength(this->m_MaximumStepLength);
  myHelper->SetMinimumStepLength(this->m_MinimumStepLength);
  myHelper->SetRelaxationFactor(this->m_RelaxationFactor);
  myHelper->SetTranslationScale(this->m_TranslationScale);
  myHelper->SetReproportionScale(this->m_ReproportionScale);
  myHelper->SetSkewScale(this->m_SkewScale);
  myHelper->SetBackgroundFillValue(this->m_BackgroundFillValue);
  myHelper->SetInitializeTransformMode(this->m_InitializeTransformMode);
  myHelper->SetUseExplicitPDFDerivativesMode(this->m_UseExplicitPDFDerivativesMode);
  myHelper->SetMaskInferiorCutOffFromCenter(this->m_MaskInferiorCutOffFromCenter);
  myHelper->SetCurrentGenericTransform(this->m_CurrentGenericTransform);
  myHelper->SetSplineGridSize(this->m_SplineGridSize);
  myHelper->SetCostFunctionConvergenceFactor(this->m_CostFunctionConvergenceFactor);
  myHelper->SetProjectedGradientTolerance(this->m_ProjectedGradientTolerance);
  myHelper->SetMaxBSplineDisplacement(this->m_MaxBSplineDisplacement);
  myHelper->SetDisplayDeformedImage(this->m_DisplayDeformedImage);
  myHelper->SetPromptUserAfterDisplay(this->m_PromptUserAfterDisplay);
  myHelper->SetDebugLevel(this->m_DebugLevel);
  myHelper->SetCostMetricObject(localCostMetric);
  if ( this->m_DebugLevel > 7 )
    {
    this->PrintCommandLine(true, "BF");
    }
  this->m_Helper = static_cast<itk::Object *>(myHelper.GetPointer());
}

template<class TLocalCostMetric>
void
BRAINSFitHelper::
RunRegistration()
{
  typedef typename TLocalCostMetric::FixedImageType FixedImageType;
  typedef typename TLocalCostMetric::MovingImageType MovingImageType;
  typedef typename itk::BRAINSFitHelperTemplate<FixedImageType,MovingImageType>
    HelperType;

  typename HelperType::Pointer myHelper =
    dynamic_cast<HelperType *>(this->m_Helper.GetPointer());

  myHelper->StartRegistration();
  this->m_CurrentGenericTransform = myHelper->GetCurrentGenericTransform();
  this->m_ActualNumberOfIterations = myHelper->GetActualNumberOfIterations();
  this->m_PermittedNumberOfIterations = myHelper->GetPermittedNumberOfIterations();
  this->m_GenericTransformList.resize(myHelper->GetGenericTransformListPtr()->size());
  std::copy(myHelper->GetGenericTransformListPtr()->begin(),
    myHelper->GetGenericTransformListPtr()->end(), this->m_GenericTransformList.begin());

}

template <class TLocalCostMetric>
typename TLocalCostMetric::Pointer
BRAINSFitHelper::
GetCostMetric()
{
  typedef typename TLocalCostMetric::FixedImageType FixedImageType;
  typedef typename TLocalCostMetric::MovingImageType MovingImageType;
  typedef typename itk::BRAINSFitHelperTemplate<FixedImageType,MovingImageType>
    HelperType;
  typedef typename HelperType::MetricType GenericMetricType;

  typename HelperType::Pointer myHelper =
    dynamic_cast<HelperType *>(this->m_Helper.GetPointer());

  typename GenericMetricType::Pointer metric = myHelper->GetCostMetricObject();
  typename TLocalCostMetric::Pointer rval =
    dynamic_cast<TLocalCostMetric *>(metric.GetPointer());
  return rval;
}

}   // end namespace itk

#endif  // __BRAINSFITHELPER__
