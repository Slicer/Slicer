#include "BRAINSFitHelper.h"
#include "BRAINSFitHelperTemplate.h"

#include "genericRegistrationHelper.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkKullbackLeiblerCompareHistogramImageToImageMetric.h"
#include "itkHistogramImageToImageMetric.h"
#include "itkKappaStatisticImageToImageMetric.h"
#include "itkMeanReciprocalSquareDifferenceImageToImageMetric.h"
#include "itkMutualInformationHistogramImageToImageMetric.h"
#include "itkGradientDifferenceImageToImageMetric.h"
#include "itkCompareHistogramImageToImageMetric.h"
#include "itkCorrelationCoefficientHistogramImageToImageMetric.h"
#include "itkMatchCardinalityImageToImageMetric.h"
#include "itkMeanSquaresHistogramImageToImageMetric.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkNormalizedMutualInformationHistogramImageToImageMetric.h"

namespace itk
{

BRAINSFitHelper::BRAINSFitHelper()
{
  m_TransformType.resize(1);
  m_TransformType[0] = "Rigid";
  m_FixedVolume = NULL;
  m_MovingVolume = NULL;
  m_PreprocessedMovingVolume = NULL;
  m_FixedBinaryVolume = NULL;
  m_MovingBinaryVolume = NULL;
  m_PermitParameterVariation.resize(0);
  m_NumberOfSamples = 500000;
  m_NumberOfHistogramBins = 50;
  m_HistogramMatch = false;
  m_NumberOfMatchPoints = 10;
  m_NumberOfIterations.resize(1);
  m_NumberOfIterations[0] = 1500;
  m_MaximumStepLength = 0.2;
  m_MinimumStepLength.resize(1);
  m_MinimumStepLength[0] = 0.005;
  m_RelaxationFactor = 0.5;
  m_TranslationScale = 1000.0;
  m_ReproportionScale = 1.0;
  m_SkewScale = 1.0;
  m_UseExplicitPDFDerivativesMode = "AUTO";
  m_UseCachingOfBSplineWeightsMode = "ON";
  m_BackgroundFillValue = 0.0;
  m_InitializeTransformMode = "Off";
  m_MaskInferiorCutOffFromCenter = 1000;
  m_CurrentGenericTransform = NULL;
  m_SplineGridSize.resize(3);
  m_SplineGridSize[0] = 14;
  m_SplineGridSize[1] = 10;
  m_SplineGridSize[2] = 12;
  m_MaxBSplineDisplacement = 0.0,
  m_CostFunctionConvergenceFactor = 1e+9,
  m_ProjectedGradientTolerance = 1e-5,
  m_ActualNumberOfIterations = 0;
  m_PermittedNumberOfIterations = 0;
  // m_AccumulatedNumberOfIterationsForAllLevels=0;
  m_DebugLevel = 0;
  m_DisplayDeformedImage = false;
  m_PromptUserAfterDisplay = false;
  m_CostMetric="MMI"; //Default to Mattes Mutual Information Metric
}

#define BRAINSFitCommonTransferToFromTemplatedVersionMacro(LOCAL_COST_METRIC_OBJECT,FIXEDIMAGETYPE,MOVINGIMAGETYPE)     \
{                                                                                        \
    BRAINSFitHelperTemplate<FIXEDIMAGETYPE,MOVINGIMAGETYPE>::Pointer                                         \
                                    myHelper=BRAINSFitHelperTemplate<FIXEDIMAGETYPE,MOVINGIMAGETYPE>::New(); \
    myHelper->SetTransformType(this->m_TransformType);                                   \
    myHelper->SetFixedVolume(this->m_FixedVolume);                                       \
    myHelper->SetMovingVolume(this->m_PreprocessedMovingVolume);                         \
    myHelper->SetHistogramMatch(this->m_HistogramMatch);                                 \
    myHelper->SetNumberOfMatchPoints(this->m_NumberOfMatchPoints);                       \
    myHelper->SetFixedBinaryVolume(this->m_FixedBinaryVolume);                           \
    myHelper->SetMovingBinaryVolume(this->m_MovingBinaryVolume);                         \
    myHelper->SetOutputFixedVolumeROI(this->m_OutputFixedVolumeROI);                     \
    myHelper->SetOutputMovingVolumeROI(this->m_OutputMovingVolumeROI);                   \
    myHelper->SetPermitParameterVariation(this->m_PermitParameterVariation);             \
    myHelper->SetNumberOfSamples(this->m_NumberOfSamples);                               \
    myHelper->SetNumberOfHistogramBins(this->m_NumberOfHistogramBins);                   \
    myHelper->SetNumberOfIterations(this->m_NumberOfIterations);                         \
    myHelper->SetMaximumStepLength(this->m_MaximumStepLength);                           \
    myHelper->SetMinimumStepLength(this->m_MinimumStepLength);                           \
    myHelper->SetRelaxationFactor(this->m_RelaxationFactor);                             \
    myHelper->SetTranslationScale(this->m_TranslationScale);                             \
    myHelper->SetReproportionScale(this->m_ReproportionScale);                           \
    myHelper->SetSkewScale(this->m_SkewScale);                                           \
    myHelper->SetBackgroundFillValue(this->m_BackgroundFillValue);                       \
    myHelper->SetInitializeTransformMode(this->m_InitializeTransformMode);               \
    myHelper->SetUseExplicitPDFDerivativesMode(this->m_UseExplicitPDFDerivativesMode);   \
    myHelper->SetMaskInferiorCutOffFromCenter(this->m_MaskInferiorCutOffFromCenter);     \
    myHelper->SetCurrentGenericTransform(this->m_CurrentGenericTransform);               \
    myHelper->SetSplineGridSize(this->m_SplineGridSize);                                 \
    myHelper->SetCostFunctionConvergenceFactor(this->m_CostFunctionConvergenceFactor);   \
    myHelper->SetProjectedGradientTolerance(this->m_ProjectedGradientTolerance);         \
    myHelper->SetMaxBSplineDisplacement(this->m_MaxBSplineDisplacement);                 \
    myHelper->SetDisplayDeformedImage(this->m_DisplayDeformedImage);                     \
    myHelper->SetPromptUserAfterDisplay(this->m_PromptUserAfterDisplay);                 \
    myHelper->SetDebugLevel(this->m_DebugLevel);                                         \
    myHelper->SetCostMetricObject(LOCAL_COST_METRIC_OBJECT);                             \
    if ( this->m_DebugLevel > 7 ) { this->PrintCommandLine(true, "BF"); }                \
    myHelper->StartRegistration();                                                       \
    this->m_CurrentGenericTransform = myHelper->GetCurrentGenericTransform();            \
    this->m_ActualNumberOfIterations = myHelper->GetActualNumberOfIterations();          \
    this->m_PermittedNumberOfIterations = myHelper->GetPermittedNumberOfIterations();    \
}


#define BRAINSFitCommonMetricSetupMacro()                                                     \
    MetricType::Pointer localCostMetric=MetricType::New();                                    \
    {                                                                                         \
    localCostMetric->ReinitializeSeed(76926294);                                              \
    localCostMetric->SetInterpolator(localLinearInterpolator);                                \
    localCostMetric->SetFixedImage(this->m_FixedVolume);                                      \
    localCostMetric->SetFixedImageRegion( this->m_FixedVolume->GetLargestPossibleRegion() );  \
    localCostMetric->SetMovingImage(this->m_PreprocessedMovingVolume);                        \
    if ( this->m_FixedBinaryVolume.IsNotNull() )                                              \
      {                                                                                       \
      localCostMetric->SetFixedImageMask(this->m_FixedBinaryVolume);                          \
      }                                                                                       \
    if ( this->m_MovingBinaryVolume.IsNotNull() )                                             \
      {                                                                                       \
      localCostMetric->SetMovingImageMask(this->m_MovingBinaryVolume);                        \
      }                                                                                       \
    if ( this->m_NumberOfSamples > 0 )                                                        \
      {                                                                                       \
        localCostMetric->SetNumberOfSpatialSamples(this->m_NumberOfSamples);                  \
      }                                                                                       \
    else                                                                                      \
      {                                                                                       \
        localCostMetric->SetUseAllPixels(true);                                               \
      }                                                                                       \
    }                                                                                         \

void
BRAINSFitHelper::StartRegistration(void)
{
  // Do Histogram equalization on moving image if requested.
  if ( m_HistogramMatch )
    {
    typedef itk::OtsuHistogramMatchingImageFilter< FixedVolumeType,MovingVolumeType > HistogramMatchingFilterType;
    HistogramMatchingFilterType::Pointer histogramfilter = HistogramMatchingFilterType::New();

    histogramfilter->SetInput(this->m_MovingVolume);
    histogramfilter->SetReferenceImage(this->m_FixedVolume);

    histogramfilter->SetNumberOfHistogramLevels(this->m_NumberOfHistogramBins);
    histogramfilter->SetNumberOfMatchPoints(this->m_NumberOfMatchPoints);
    histogramfilter->ThresholdAtMeanIntensityOff();
    // histogramfilter->ThresholdAtMeanIntensityOn();
    histogramfilter->Update();
    this->m_PreprocessedMovingVolume = histogramfilter->GetOutput();
    }
  else
    {
    this->m_PreprocessedMovingVolume = this->m_MovingVolume;
    }

  typedef itk::LinearInterpolateImageFunction< MovingVolumeType, double > InterpolatorType;
  InterpolatorType::Pointer localLinearInterpolator = InterpolatorType::New();

  if(this->m_CostMetric == "MMI")
    {
    //Setup the CostMetricObject
    typedef itk::MattesMutualInformationImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
      { // MatttesMutualInformation Metric options.
      localCostMetric->SetNumberOfHistogramBins(this->m_NumberOfHistogramBins);
      const bool UseCachingOfBSplineWeights = ( m_UseCachingOfBSplineWeightsMode == "ON" ) ? true : false;
      localCostMetric->SetUseCachingOfBSplineWeights(UseCachingOfBSplineWeights);
      }
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "MSE")
    {
    typedef itk::MeanSquaresImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "NC")
    {
    typedef itk::NormalizedCorrelationImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  // This requires additional machinery (training transform, etc) and hence isn't as easy to incorporate
  // into the BRAINSFit framework.
  /*else if(this->m_CostMetric == "KL")
    {
    typedef itk::KullbackLeiblerCompareHistogramImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }*/
  else if(this->m_CostMetric == "KS")
    {
    // This metric only works with binary images that it knows the value of.
    // It defaults to 255, so we threshold the inputs to 255.
    typedef itk::BinaryThresholdImageFilter<FixedVolumeType,FixedVolumeType> BinaryThresholdFixedVolumeType;
    BinaryThresholdFixedVolumeType::Pointer binaryThresholdFixedVolume = BinaryThresholdFixedVolumeType::New();
    binaryThresholdFixedVolume->SetInput(this->m_FixedVolume);
    binaryThresholdFixedVolume->SetOutsideValue(0);
    binaryThresholdFixedVolume->SetInsideValue(255);
    binaryThresholdFixedVolume->SetLowerThreshold(1);
    binaryThresholdFixedVolume->Update();
    this->m_FixedVolume = binaryThresholdFixedVolume->GetOutput();

    typedef itk::BinaryThresholdImageFilter<MovingVolumeType,MovingVolumeType> BinaryThresholdMovingVolumeType;
    BinaryThresholdMovingVolumeType::Pointer binaryThresholdMovingVolume = BinaryThresholdMovingVolumeType::New();
    binaryThresholdMovingVolume->SetInput(this->m_MovingVolume);
    binaryThresholdMovingVolume->SetOutsideValue(0);
    binaryThresholdMovingVolume->SetInsideValue(255);
    binaryThresholdMovingVolume->SetLowerThreshold(1);
    binaryThresholdMovingVolume->Update();
    this->m_MovingVolume = binaryThresholdMovingVolume->GetOutput();

    typedef itk::KappaStatisticImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "MRSD")
    {
    typedef itk::MeanReciprocalSquareDifferenceImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "MIH")
    {
    typedef itk::MutualInformationHistogramImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "GD")
    {
    typedef itk::GradientDifferenceImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "CCH")
    {
    typedef itk::CorrelationCoefficientHistogramImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "MC")
    {
    typedef itk::MatchCardinalityImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "MSEH")
    {
    typedef itk::MeanSquaresHistogramImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else if(this->m_CostMetric == "NMIH")
    {
    typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedVolumeType,MovingVolumeType> MetricType;
    BRAINSFitCommonMetricSetupMacro();
    BRAINSFitCommonTransferToFromTemplatedVersionMacro(localCostMetric,MetricType::FixedImageType,MetricType::MovingImageType);
    }
  else
    {
    std::cout << "Metric \"" << this->m_CostMetric << "\" not valid." << std::endl;
    }
}

void
BRAINSFitHelper::PrintSelf(std::ostream & os, Indent indent) const
{
  // Superclass::PrintSelf(os,indent);
  os << indent << "FixedVolume:\n"  <<   this->m_FixedVolume << std::endl;
  os << indent << "MovingVolume:\n" <<   this->m_MovingVolume << std::endl;
  os << indent << "PreprocessedMovingVolume:\n" <<   this->m_PreprocessedMovingVolume << std::endl;
  if ( this->m_FixedBinaryVolume.IsNotNull() )
    {
    os << indent << "FixedBinaryVolume:\n" << this->m_FixedBinaryVolume << std::endl;
    }
  else
    {
    os << indent << "FixedBinaryVolume: IS NULL" << std::endl;
    }
  if ( this->m_MovingBinaryVolume.IsNotNull() )
    {
    os << indent << "MovingBinaryVolume:\n" << this->m_MovingBinaryVolume << std::endl;
    }
  else
    {
    os << indent << "MovingBinaryVolume: IS NULL" << std::endl;
    }
  os << indent << "NumberOfSamples:      " << this->m_NumberOfSamples << std::endl;

  os << indent << "NumberOfIterations:    [";
  for ( unsigned int q = 0; q < this->m_NumberOfIterations.size(); q++ )
    {
    os << this->m_NumberOfIterations[q] << " ";
    }
  os << "]" << std::endl;
  os << indent << "NumberOfHistogramBins:" << this->m_NumberOfHistogramBins << std::endl;
  os << indent << "MaximumStepLength:    " << this->m_MaximumStepLength << std::endl;
  os << indent << "MinimumStepLength:     [";
  for ( unsigned int q = 0; q < this->m_MinimumStepLength.size(); q++ )
    {
    os << this->m_MinimumStepLength[q] << " ";
    }
  os << "]" << std::endl;
  os << indent << "TransformType:     [";
  for ( unsigned int q = 0; q < this->m_TransformType.size(); q++ )
    {
    os << this->m_TransformType[q] << " ";
    }
  os << "]" << std::endl;

  os << indent << "RelaxationFactor:    " << this->m_RelaxationFactor << std::endl;
  os << indent << "TranslationScale:    " << this->m_TranslationScale << std::endl;
  os << indent << "ReproportionScale:   " << this->m_ReproportionScale << std::endl;
  os << indent << "SkewScale:           " << this->m_SkewScale << std::endl;
  os << indent << "UseExplicitPDFDerivativesMode:  " << this->m_UseExplicitPDFDerivativesMode << std::endl;
  os << indent << "UseCachingOfBSplineWeightsMode: " << this->m_UseCachingOfBSplineWeightsMode << std::endl;
  os << indent << "BackgroundFillValue:            " << this->m_BackgroundFillValue << std::endl;
  os << indent << "InitializeTransformMode:        " << this->m_InitializeTransformMode << std::endl;
  os << indent << "MaskInferiorCutOffFromCenter:   " << this->m_MaskInferiorCutOffFromCenter << std::endl;
  os << indent << "ActualNumberOfIterations:       " << this->m_ActualNumberOfIterations << std::endl;
  os << indent << "PermittedNumberOfIterations:       " << this->m_PermittedNumberOfIterations << std::endl;
  // os << indent << "AccumulatedNumberOfIterationsForAllLevels: " <<
  // this->m_AccumulatedNumberOfIterationsForAllLevels << std::endl;

  os << indent << "SplineGridSize:     [";
  for ( unsigned int q = 0; q < this->m_SplineGridSize.size(); q++ )
    {
    os << this->m_SplineGridSize[q] << " ";
    }
  os << "]" << std::endl;

  os << indent << "PermitParameterVariation:     [";
  for ( unsigned int q = 0; q < this->m_PermitParameterVariation.size(); q++ )
    {
    os << this->m_PermitParameterVariation[q] << " ";
    }
  os << "]" << std::endl;

  if ( m_CurrentGenericTransform.IsNotNull() )
    {
    os << indent << "CurrentGenericTransform:\n" << this->m_CurrentGenericTransform << std::endl;
    }
  else
    {
    os << indent << "CurrentGenericTransform: IS NULL" << std::endl;
    }
  os << indent << "CostMetric:       " << this->m_CostMetric << std::endl;
}

void
BRAINSFitHelper::PrintCommandLine(const bool dumpTempVolumes, const std::string suffix) const
{
  std::cout << "The equivalent command line to the current run would be:" << std::endl;

  const std::string fixedVolumeString("DEBUGFixedVolume_" + suffix + ".nii.gz");
  const std::string movingVolumeString("DEBUGMovingVolume_" + suffix + ".nii.gz");
  const std::string fixedBinaryVolumeString("DEBUGFixedBinaryVolume_" + suffix + ".nii.gz");
  const std::string movingBinaryVolumeString("DEBUGMovingBinaryVolume_" + suffix + ".nii.gz");

  std::ostringstream oss;
  oss << "BRAINSFit \\" << std::endl;
  if ( dumpTempVolumes == true )
    {
      {
      typedef itk::ImageFileWriter< FixedVolumeType > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->UseCompressionOn();
      writer->SetFileName(fixedVolumeString);
      writer->SetInput(this->m_FixedVolume);
      try
        {
        writer->Update();
        }
      catch ( itk::ExceptionObject & err )
        {
        oss << "Exception Object caught: " << std::endl;
        oss << err << std::endl;
        throw;
        }
      }
      {
      typedef itk::ImageFileWriter< MovingVolumeType > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->UseCompressionOn();
      writer->SetFileName(movingVolumeString);
      writer->SetInput(this->m_MovingVolume);
      try
        {
        writer->Update();
        }
      catch ( itk::ExceptionObject & err )
        {
        oss << "Exception Object caught: " << std::endl;
        oss << err << std::endl;
        throw;
        }
      }
    }
  oss << "--costMetric " << this->m_CostMetric << " \\" << std::endl;
  oss << "--fixedVolume "  <<  fixedVolumeString   << "  \\" << std::endl;
  oss << "--movingVolume " <<  movingVolumeString  << "  \\" << std::endl;
  if ( this->m_HistogramMatch )
    {
    oss << "--histogramMatch " <<  "  \\" << std::endl;
    }

    {
    if ( this->m_FixedBinaryVolume.IsNotNull() )
      {
      oss << "--fixedBinaryVolume " << fixedBinaryVolumeString  << "  \\" << std::endl;
        {
        typedef itk::Image< unsigned char, 3 >                               MaskImageType;
        typedef itk::ImageMaskSpatialObject< MaskImageType::ImageDimension > ImageMaskSpatialObjectType;
          {
          ImageMaskSpatialObjectType::Pointer fixedImageMask(
            dynamic_cast< ImageMaskSpatialObjectType * >( m_FixedBinaryVolume.GetPointer() ) );
          MaskImageType::Pointer tempOutputFixedVolumeROI = const_cast< MaskImageType * >( fixedImageMask->GetImage() );
          itkUtil::WriteImage< MaskImageType >(tempOutputFixedVolumeROI, fixedBinaryVolumeString);
          }
        }
      }
    if ( this->m_MovingBinaryVolume.IsNotNull() )
      {
      oss << "--movingBinaryVolume " << movingBinaryVolumeString  << "  \\" << std::endl;
        {
        typedef itk::Image< unsigned char, 3 >                               MaskImageType;
        typedef itk::ImageMaskSpatialObject< MaskImageType::ImageDimension > ImageMaskSpatialObjectType;
          {
          ImageMaskSpatialObjectType::Pointer movingImageMask(
            dynamic_cast< ImageMaskSpatialObjectType * >( m_MovingBinaryVolume.GetPointer() ) );
          MaskImageType::Pointer tempOutputMovingVolumeROI = const_cast< MaskImageType * >( movingImageMask->GetImage() );
          itkUtil::WriteImage< MaskImageType >(tempOutputMovingVolumeROI, movingBinaryVolumeString);
          }
        }
      }
    if ( this->m_FixedBinaryVolume.IsNotNull()  || this->m_MovingBinaryVolume.IsNotNull() )
      {
      oss << "--maskProcessingMode ROI "   << "  \\" << std::endl;
      }
    }
  oss << "--numberOfSamples " << this->m_NumberOfSamples  << "  \\" << std::endl;

  oss << "--numberOfIterations ";
  for ( unsigned int q = 0; q < this->m_NumberOfIterations.size(); q++ )
    {
    oss << this->m_NumberOfIterations[q];
    if ( q < this->m_NumberOfIterations.size() - 1 )
      {
      oss << ",";
      }
    }
  oss << " \\" << std::endl;
  oss << "--numberOfHistogramBins " << this->m_NumberOfHistogramBins  << "  \\" << std::endl;
  oss << "--maximumStepSize " << this->m_MaximumStepLength  << "  \\" << std::endl;
  oss << "--minimumStepSize ";
  for ( unsigned int q = 0; q < this->m_MinimumStepLength.size(); q++ )
    {
    oss << this->m_MinimumStepLength[q];
    if ( q < this->m_MinimumStepLength.size() - 1 )
      {
      oss << ",";
      }
    }
  oss << " \\" << std::endl;
  oss << "--transformType ";
  for ( unsigned int q = 0; q < this->m_TransformType.size(); q++ )
    {
    oss << this->m_TransformType[q];
    if ( q < this->m_TransformType.size() - 1 )
      {
      oss << ",";
      }
    }
  oss << " \\" << std::endl;

  oss << "--relaxationFactor " << this->m_RelaxationFactor  << "  \\" << std::endl;
  oss << "--translationScale " << this->m_TranslationScale  << "  \\" << std::endl;
  oss << "--reproportionScale " << this->m_ReproportionScale  << "  \\" << std::endl;
  oss << "--skewScale " << this->m_SkewScale  << "  \\" << std::endl;
  oss << "--useExplicitPDFDerivativesMode " << this->m_UseExplicitPDFDerivativesMode  << "  \\" << std::endl;
  oss << "--useCachingOfBSplineWeightsMode " << this->m_UseCachingOfBSplineWeightsMode  << "  \\" << std::endl;
  oss << "--maxBSplineDisplacement " << this->m_MaxBSplineDisplacement << " \\" << std::endl;
  oss << "--projectedGradientTolerance " << this->m_ProjectedGradientTolerance << " \\" << std::endl;
  oss << "--costFunctionConvergenceFactor " << this->m_CostFunctionConvergenceFactor << " \\" << std::endl;
  oss << "--backgroundFillValue " << this->m_BackgroundFillValue  << "  \\" << std::endl;
  if ( this->m_InitializeTransformMode == "useGeometryAlign" )
    {
    oss << "--useGeometryAlign \\" << std::endl;
    }
  else if ( this->m_InitializeTransformMode == "useMomentsAlign" )
    {
    oss << "--useMomentsAlign \\" << std::endl;
    }
  else if ( this->m_InitializeTransformMode == "useCenterOfHeadAlign" )
    {
    oss << "--useCenterOfHeadAlign \\" << std::endl;
    }
  oss  << "--initializeTransformMode " << this->m_InitializeTransformMode  << "  \\" << std::endl;
  oss << "--maskInferiorCutOffFromCenter " << this->m_MaskInferiorCutOffFromCenter  << "  \\" << std::endl;
  oss << "--splineGridSize ";
  for ( unsigned int q = 0; q < this->m_SplineGridSize.size(); q++ )
    {
    oss << this->m_SplineGridSize[q];
    if ( q < this->m_SplineGridSize.size() - 1 )
      {
      oss << ",";
      }
    }
  oss << " \\" << std::endl;

  if ( this->m_PermitParameterVariation.size() > 0 )
    {
    oss << "--permitParameterVariation ";
    for ( unsigned int q = 0; q < this->m_PermitParameterVariation.size(); q++ )
      {
      oss << this->m_PermitParameterVariation[q];
      if ( q < this->m_PermitParameterVariation.size() - 1 )
        {
        oss << ",";
        }
      }
    oss << " \\" << std::endl;
    }
  if ( m_CurrentGenericTransform.IsNotNull() )
    {
    const std::string initialTransformString("DEBUGInitialTransform_" + suffix + ".mat");
    WriteBothTransformsToDisk(this->m_CurrentGenericTransform.GetPointer(), initialTransformString, "");
    oss << "--initialTransform " << initialTransformString  << "  \\" << std::endl;
    }
    {
    const std::string outputVolume("DEBUGOutputVolume_" + suffix + ".nii.gz");
    oss << "--outputVolume " << outputVolume  << "  \\" << std::endl;
    std::cout << oss.str() << std::endl;
    }
    {
    const std::string outputTransform("DEBUGOutputTransform" + suffix + ".mat");
    oss << "--outputTransform " << outputTransform  << "  \\" << std::endl;
    std::cout << oss.str() << std::endl;
    }
  const std::string TesterScript("DEBUGScript" + suffix + ".sh");
  std::ofstream     myScript;
  myScript.open( TesterScript.c_str() );
  myScript << oss.str() << std::endl;
  myScript.close();
}

void
BRAINSFitHelper::GenerateData()
{
  this->StartRegistration();
}
} // end namespace itk
