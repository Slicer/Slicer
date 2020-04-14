/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: MomentRegistrator.txx,v $
  Language:  C++
  Date:      $Date: 2007/03/29 17:52:55 $
  Version:   $Revision: 1.6 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef itkOptimizedImageToImageRegistrationMethod_txx
#define itkOptimizedImageToImageRegistrationMethod_txx

#include "itkOptimizedImageToImageRegistrationMethod.h"

#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"

#include "itkImageRegistrationMethod.h"
#include "itkMultiResolutionImageRegistrationMethod.h"

#include "itkRealTimeClock.h"

#include "itkSingleValuedNonLinearOptimizer.h"
#include "itkOnePlusOneEvolutionaryOptimizer.h"
#include "itkNormalVariateGenerator.h"

#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkFRPROptimizer.h"
#include "itkImageMaskSpatialObject.h"

#include "itkImage.h"
#include <itkConstantBoundaryCondition.h>


#include <sstream>

namespace itk
{

class ImageRegistrationViewer
  : public Command
{
public:
  typedef ImageRegistrationViewer Self;
  typedef Command                 Superclass;
  typedef SmartPointer<Self>      Pointer;

  itkTypeMacro( ImageRegistrationViewer, Command );

  itkNewMacro( ImageRegistrationViewer );

  typedef SingleValuedNonLinearOptimizer OptimizerType;

  itkSetMacro(DontShowParameters, bool);
  itkSetMacro(UpdateInterval, int);

  void Execute( Object * caller, const EventObject & event ) override
  {
    Execute( (const Object *)caller, event );
  }

  void Execute( const Object * object, const EventObject & event ) override
  {
    if( typeid( event ) != typeid( IterationEvent ) || object == nullptr )
      {
      return;
      }

    const OptimizerType * opt = dynamic_cast<const OptimizerType *>(object);

    if( ++m_Iteration % m_UpdateInterval == 0 )
      {
      RealTimeClock::TimeStampType t = m_Clock->GetTimeInSeconds();
      if( !m_DontShowParameters )
        {
        std::cout << "   " << m_Iteration << " : "
                  << opt->GetCurrentPosition() << " = "
                  << opt->GetValue( opt->GetCurrentPosition() )
                  << "   (" << (t - m_LastTime) / m_UpdateInterval << "s)"
                  << std::endl;
        }
      else
        {
        std::cout << "   " << m_Iteration << " : "
                  << opt->GetValue( opt->GetCurrentPosition() )
                  << "   (" << (t - m_LastTime) / m_UpdateInterval << "s)"
                  << std::endl;
        }
      m_LastTime = t;
      }
  }

  void Update()
  {
    this->Execute( (const Object *)nullptr, IterationEvent() );
  }

protected:

  RealTimeClock::Pointer       m_Clock;
  RealTimeClock::TimeStampType m_LastTime;

  int  m_Iteration;
  int  m_UpdateInterval;
  bool m_DontShowParameters;

  ImageRegistrationViewer()
  {
    m_Clock = RealTimeClock::New();
    m_LastTime = m_Clock->GetTimeInSeconds();
    m_Iteration = 0;
    m_UpdateInterval = 1;
    m_DontShowParameters = false;
  }
  ~ImageRegistrationViewer() override = default;

};

template <class TImage>
OptimizedImageToImageRegistrationMethod<TImage>::OptimizedImageToImageRegistrationMethod()
{
  m_InitialTransformParameters = TransformParametersType(1);
  m_InitialTransformParameters.Fill( 0.0f ); \

  m_InitialTransformFixedParameters = TransformParametersType(1);
  m_InitialTransformFixedParameters.Fill( 0.0f ); \

  m_LastTransformParameters = TransformParametersType(1);
  m_LastTransformParameters.Fill( 0.0f );

  m_TransformParametersScales = TransformParametersScalesType(1);
  m_TransformParametersScales.Fill( 1.0f );

  // The following MaxIterations value is a good default for rigid
  //   registration.  Other derived registration methods should use
  //   their own default.
  m_MaxIterations = 100;
  m_SampleFromOverlap = false;
  m_MinimizeMemory = false;

  m_UseEvolutionaryOptimization = true;

  // The following NumberOfSamples value is a good default for rigid
  //   registration.  Other derived registration methods should use
  //   their own default.
  m_NumberOfSamples = 100000;
  m_FixedImageSamplesIntensityThreshold = 0;
  m_UseFixedImageSamplesIntensityThreshold = false;

  m_TargetError = 0.00001;

  m_RandomNumberSeed = 0;

  m_TransformMethodEnum = RIGID_TRANSFORM;

  m_MetricMethodEnum = MATTES_MI_METRIC;
  m_InterpolationMethodEnum = LINEAR_INTERPOLATION;

  m_FinalMetricValue = 0;
}

template <class TImage>
OptimizedImageToImageRegistrationMethod<TImage>::~OptimizedImageToImageRegistrationMethod() = default;

template <class TImage>
void
OptimizedImageToImageRegistrationMethod<TImage>
::SetFixedImageSamplesIntensityThreshold( PixelType val )
{
  m_FixedImageSamplesIntensityThreshold = val;
  m_UseFixedImageSamplesIntensityThreshold = true;
}

template <class TImage>
void OptimizedImageToImageRegistrationMethod<TImage>::GenerateData()
{
  if( this->GetReportProgress() )
    {
    std::cout << "UPDATE START" << std::endl;
    }

  this->Initialize();

  this->GetTransform()->SetParametersByValue( this->GetInitialTransformParameters() );

  typename MetricType::Pointer metric;

  switch( this->GetMetricMethodEnum() )
    {
    case MATTES_MI_METRIC:
        {
        typedef MattesMutualInformationImageToImageMetric<TImage, TImage> TypedMetricType;

        typename TypedMetricType::Pointer typedMetric = TypedMetricType::New();

        typedMetric->SetNumberOfHistogramBins( 100 );
        // Shouldn't need to limit this call to cases of bspline transforms.
        // if( m_MinimizeMemory && m_TransformMethodEnum == BSPLINE_TRANSFORM )
        if( m_MinimizeMemory )
          {
          typedMetric->SetUseExplicitPDFDerivatives( false );
          typedMetric->SetUseCachingOfBSplineWeights( false );
          }
        metric = typedMetric;
        }
      break;
    case NORMALIZED_CORRELATION_METRIC:
      metric = NormalizedCorrelationImageToImageMetric<TImage, TImage>::New();
      break;
    case MEAN_SQUARED_ERROR_METRIC:
      metric = MeanSquaresImageToImageMetric<TImage, TImage>::New();
      break;
    }
  if( m_RandomNumberSeed != 0 )
    {
    metric->ReinitializeSeed( m_RandomNumberSeed );
    }
  else
    {
    metric->ReinitializeSeed();
    }

  typename ImageType::ConstPointer fixedImage = this->GetFixedImage();
  typename ImageType::ConstPointer movingImage = this->GetMovingImage();

  metric->SetFixedImage( fixedImage );
  metric->SetMovingImage( movingImage );

  metric->SetNumberOfSpatialSamples( m_NumberOfSamples );

  if( this->GetUseRegionOfInterest() ||
      this->GetSampleFromOverlap() ||
      this->GetUseFixedImageSamplesIntensityThreshold() ||
      this->GetUseFixedImageMaskObject() )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "Creating fixed image samples" << std::endl;
      }

    itk::ImageRegionConstIteratorWithIndex<ImageType> iter( fixedImage,
                                                            fixedImage->GetLargestPossibleRegion() );
    typename ImageType::IndexType index;
    typename ImageType::IndexType movingIndex;
    typename MetricType::InputPointType fixedPoint;
    typename MetricType::InputPointType movingPoint;

    iter.GoToBegin();
    int count = 0;
    for( iter.GoToBegin(); !iter.IsAtEnd(); ++iter )
      {
      index = iter.GetIndex();
      fixedImage->TransformIndexToPhysicalPoint(index, fixedPoint);
      if( this->GetSampleFromOverlap() )
        {
        movingPoint = this->GetTransform()->TransformPoint( fixedPoint );
        if( !movingImage->TransformPhysicalPointToIndex( movingPoint, movingIndex ) )
          {
          continue;
          }
        }
      if( this->GetUseFixedImageSamplesIntensityThreshold() )
        {
        if( iter.Get() < this->m_FixedImageSamplesIntensityThreshold )
          {
          continue;
          }
        }
      if( this->GetUseFixedImageMaskObject() )
        {
        double val;
        if( this->GetFixedImageMaskObject()->ValueAtInWorldSpace( fixedPoint, val ) )
          {
          if( val == 0 )
            {
            continue;
            }
          }
        }
      if( this->GetUseRegionOfInterest() )
        {
        bool isInside = true;
        for( unsigned int i = 0; i < ImageDimension; i++ )
          {
          if( !( (fixedPoint[i] >= this->GetRegionOfInterestPoint1()[i] &&
                  fixedPoint[i] <= this->GetRegionOfInterestPoint2()[i])
                 || (fixedPoint[i] >= this->GetRegionOfInterestPoint2()[i] &&
                     fixedPoint[i] <= this->GetRegionOfInterestPoint1()[i]) ) )
            {
            isInside = false;
            break;
            }
          }
        if( !isInside )
          {
          continue;
          }
        }

      ++count;
      }
    double samplingRate = (double)(m_NumberOfSamples + 2) / (double)count;
    if( this->GetReportProgress() )
      {
      std::cout << "...Second pass, sampling rate = " << samplingRate << std::endl;
      }

    if( samplingRate > 1 )
      {
      samplingRate = 1;
      itkWarningMacro(<< "Adjusting the number of samples due to restrictive threshold/overlap criteria.");
      this->SetNumberOfSamples( count );
      metric->SetNumberOfSpatialSamples( m_NumberOfSamples );
      }
    double step = 0;
    typename MetricType::FixedImageIndexContainer indexList;
    indexList.clear();
    for( iter.GoToBegin(); !iter.IsAtEnd(); ++iter )
      {
      index = iter.GetIndex();
      fixedImage->TransformIndexToPhysicalPoint(index, fixedPoint);
      if( this->GetSampleFromOverlap() )
        {
        movingPoint = this->GetTransform()->TransformPoint( fixedPoint );
        if( !movingImage->TransformPhysicalPointToIndex( movingPoint, movingIndex ) )
          {
          continue;
          }
        }
      if( this->GetUseFixedImageSamplesIntensityThreshold() )
        {
        if( iter.Get() < this->m_FixedImageSamplesIntensityThreshold )
          {
          continue;
          }
        }
      if( this->GetUseFixedImageMaskObject() )
        {
        double val;
        if( this->GetFixedImageMaskObject()->ValueAtInWorldSpace( fixedPoint, val ) )
          {
          if( val == 0 )
            {
            continue;
            }
          }
        }
      step = step + samplingRate;
      if( step > 1 )
        {
        indexList.push_back( index );
        while( step > 1 )
          {
          step -= 1;
          }

        if( indexList.size() == m_NumberOfSamples )
          {
          break;
          }
        }
      }
    if( indexList.size() != m_NumberOfSamples )
      {
      itkWarningMacro(<< "Full set of samples not collected. Collected "
                      << indexList.size() << " of " << m_NumberOfSamples );
      this->SetNumberOfSamples( indexList.size() );
      metric->SetNumberOfSpatialSamples( m_NumberOfSamples );
      }
    std::cout << "Passing index list to metric..." << std::endl;
    std::cout << "  List size = " << indexList.size() << std::endl;
    metric->SetFixedImageIndexes( indexList );
    }

  if( this->GetUseMovingImageMaskObject() )
    {
    if( this->GetMovingImageMaskObject() )
      {
      metric->SetMovingImageMask( const_cast<itk::SpatialObject<ImageDimension> *>(this->GetMovingImageMaskObject() ) );
      }
    }

  typename InterpolatorType::Pointer interpolator;

  switch( this->GetInterpolationMethodEnum() )
    {
    case NEAREST_NEIGHBOR_INTERPOLATION:
      interpolator = NearestNeighborInterpolateImageFunction<TImage, double>::New();
      break;
    case LINEAR_INTERPOLATION:
      interpolator = LinearInterpolateImageFunction<TImage, double>::New();
      break;
    case BSPLINE_INTERPOLATION:
      interpolator = BSplineInterpolateImageFunction<TImage, double>::New();
      break;
    case SINC_INTERPOLATION:
      interpolator = WindowedSincInterpolateImageFunction<TImage,
                                                          4,
                                                          Function::HammingWindowFunction<4>,
                                                          ConstantBoundaryCondition<TImage>,
                                                          double>::New();
      break;
    }
  interpolator->SetInputImage( this->GetMovingImage() );

  try
    {
    this->Optimize(metric, interpolator);
    }
  catch( ... )
    {
    std::cerr << "Optimization threw an exception." << std::endl;
    }

  if( this->GetReportProgress() )
    {
    std::cout << "UPDATE END" << std::endl;
    }
}

template <class TImage>
void
OptimizedImageToImageRegistrationMethod<TImage>
::Optimize( MetricType * metric, InterpolatorType * interpolator )
{
  typedef ImageRegistrationMethod<TImage, TImage> RegType;

  if( m_UseEvolutionaryOptimization )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "EVOLUTIONARY START" << std::endl;
      }

    typedef OnePlusOneEvolutionaryOptimizer EvoOptimizerType;
    EvoOptimizerType::Pointer evoOpt = EvoOptimizerType::New();

    evoOpt->SetNormalVariateGenerator( Statistics::NormalVariateGenerator
                                       ::New() );
    evoOpt->SetEpsilon( this->GetTargetError() );
    evoOpt->Initialize( 0.1 );
    evoOpt->SetCatchGetValueException( true );
    evoOpt->SetMetricWorstPossibleValue( 0 );
    EvoOptimizerType::ParametersType scales =
      this->GetTransformParametersScales();
    for( unsigned int i = 0; i < scales.size(); i++ )
      {
      scales[i] = scales[i] * scales[i]; // OnePlusOne opt uses squared scales
      }
    evoOpt->SetScales( scales ); // this->GetTransformParametersScales() );
    evoOpt->SetMaximumIteration( this->GetMaxIterations() );

    if( this->GetObserver() )
      {
      evoOpt->AddObserver( IterationEvent(), this->GetObserver() );
      }

    if( this->GetReportProgress() )
      {
      typedef ImageRegistrationViewer ViewerCommandType;
      typename ViewerCommandType::Pointer command = ViewerCommandType::New();
      if( this->GetTransform()->GetNumberOfParameters() > 16 )
        {
        command->SetDontShowParameters( true );
        }
      evoOpt->AddObserver( IterationEvent(), command );
      }

    typename RegType::Pointer reg = RegType::New();
    typename ImageType::ConstPointer fixedImage = this->GetFixedImage();
    typename ImageType::ConstPointer movingImage = this->GetMovingImage();
    reg->SetFixedImage( fixedImage );
    reg->SetMovingImage( movingImage );
    reg->SetFixedImageRegion( this->GetFixedImage()
                              ->GetLargestPossibleRegion() );
    reg->SetTransform( this->GetTransform() );
    reg->SetInitialTransformParameters(
      this->GetInitialTransformParameters() );
    reg->SetMetric( metric );
    reg->SetInterpolator( interpolator );
    reg->SetOptimizer( evoOpt );

    try
      {
      reg->Update();
      }
    catch( itk::ExceptionObject & excep )
      {
      std::cout << "Exception caught in evolutionary registration."
                << excep << std::endl;
      std::cout << "Continuing using best values..." << std::endl;
      std::cout << "  Pos = " << evoOpt->GetCurrentPosition()
                << std::endl << std::endl;
      }
    catch( ... )
      {
      std::cout << "Exception caught in evolutionary registration."
                << std::endl;
      std::cout << "Continuing using best values..." << std::endl;
      std::cout << "  Pos = " << evoOpt->GetCurrentPosition()
                << std::endl << std::endl;
      }

    m_FinalMetricValue = reg->GetOptimizer()->GetValue(
        reg->GetLastTransformParameters() );

    this->SetLastTransformParameters( reg->GetLastTransformParameters() );
    this->GetTransform()->SetParametersByValue(
      this->GetLastTransformParameters() );

    if( this->GetReportProgress() )
      {
      std::cout << "EVOLUTIONARY END" << std::endl;
      }
    }
  else
    {
    this->GetTransform()->SetParametersByValue(
      this->GetInitialTransformParameters() );
    }

  if( this->GetReportProgress() )
    {
    std::cout << "GRADIENT START" << std::endl;
    }

  typedef FRPROptimizer GradOptimizerType;
  GradOptimizerType::Pointer gradOpt = GradOptimizerType::New();

  gradOpt->SetMaximize( false );
  gradOpt->SetCatchGetValueException( true );
  gradOpt->SetMetricWorstPossibleValue( 0 );
  gradOpt->SetStepLength( 0.25 );
  gradOpt->SetStepTolerance( this->GetTargetError() );
  gradOpt->SetMaximumIteration( this->GetMaxIterations() );
  gradOpt->SetMaximumLineIteration( 10 );
  gradOpt->SetScales( this->GetTransformParametersScales() );
  gradOpt->SetUseUnitLengthGradient(true);
  gradOpt->SetToFletchReeves();

  if( this->GetReportProgress() )
    {
    typedef ImageRegistrationViewer ViewerCommandType;
    typename ViewerCommandType::Pointer command = ViewerCommandType::New();
    if( this->GetTransform()->GetNumberOfParameters() > 16 )
      {
      command->SetDontShowParameters( true );
      }
    gradOpt->AddObserver( IterationEvent(), command );
    }
  if( this->GetObserver() )
    {
    gradOpt->AddObserver( IterationEvent(), this->GetObserver() );
    }

  typename RegType::Pointer reg = RegType::New();
  typename ImageType::ConstPointer fixedImage = this->GetFixedImage();
  typename ImageType::ConstPointer movingImage = this->GetMovingImage();
  reg->SetFixedImage( fixedImage );
  reg->SetMovingImage( movingImage );
  reg->SetFixedImageRegion( this->GetFixedImage()
                            ->GetLargestPossibleRegion() );
  reg->SetTransform( this->GetTransform() );
  reg->SetInitialTransformParameters( this->GetTransform()->GetParameters() );
  reg->SetMetric( metric );
  reg->SetInterpolator( interpolator );
  reg->SetOptimizer( gradOpt );

  bool failure = false;
  try
    {
    reg->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cout << "Exception caught during gradient registration."
              << excep << std::endl;
    std::cout << "Continuing using best values..." << std::endl;
    std::cout << "  Pos = " << reg->GetLastTransformParameters()
              << std::endl << std::endl;
    if( reg->GetLastTransformParameters().size()
        != reg->GetInitialTransformParameters().size() )
      {
      std::cout << "  Invalid position, using initial parameters." << std::endl;
      failure = true;
      }
    }
  catch( ... )
    {
    std::cout << "Exception caught in gradient registration."
              << std::endl;
    std::cout << "Continuing using best values..." << std::endl;
    std::cout << "  Pos = " << reg->GetLastTransformParameters()
              << std::endl << std::endl;
    if( reg->GetLastTransformParameters().size()
        != reg->GetInitialTransformParameters().size() )
      {
      std::cout << "  Invalid position, using initial parameters." << std::endl;
      failure = true;
      }
    }

  if( failure )
    {
    m_FinalMetricValue = reg->GetOptimizer()->GetValue(
        reg->GetInitialTransformParameters() );
    this->SetLastTransformParameters( reg->GetInitialTransformParameters() );
    this->GetTransform()->SetParametersByValue(
      this->GetInitialTransformParameters() );
    }
  else
    {
    m_FinalMetricValue = reg->GetOptimizer()->GetValue(
        reg->GetLastTransformParameters() );
    this->SetLastTransformParameters( reg->GetLastTransformParameters() );
    this->GetTransform()->SetParametersByValue(
      this->GetLastTransformParameters() );
    }

  if( this->GetReportProgress() )
    {
    std::cout << "GRADIENT END" << std::endl;
    }
}

template <class TImage>
void
OptimizedImageToImageRegistrationMethod<TImage>
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Initial Transform Parameters = " << m_InitialTransformParameters << std::endl;

  os << indent << "Initial Transform Fixed Parameters = " << m_InitialTransformFixedParameters << std::endl;

  os << indent << "Last Transform Parameters = " << m_LastTransformParameters << std::endl;

  os << indent << "Transform Parameter Scales = " << m_TransformParametersScales << std::endl;

  os << indent << "Max Iterations = " << m_MaxIterations << std::endl;

  os << indent << "Use Evolutionary Optimization = " << m_UseEvolutionaryOptimization << std::endl;

  os << indent << "Sample From Overlap = " << m_SampleFromOverlap << std::endl;

  os << indent << "Minimize Memory = " << m_MinimizeMemory << std::endl;

  os << indent << "Number of Samples = " << m_NumberOfSamples << std::endl;

  os << indent << "Samples threshold = " << m_FixedImageSamplesIntensityThreshold << std::endl;

  os << indent << "Target Error = " << m_TargetError << std::endl;

  switch( m_MetricMethodEnum )
    {
    case MATTES_MI_METRIC:
      os << indent << "Metric method = Mattes Mutual Information" << std::endl;
      break;
    case NORMALIZED_CORRELATION_METRIC:
      os << indent << "Metric method = Normalized Correlation" << std::endl;
      break;
    case MEAN_SQUARED_ERROR_METRIC:
      os << indent << "Metric method = Mean Squared Error" << std::endl;
      break;
    }

  switch( m_InterpolationMethodEnum )
    {
    case NEAREST_NEIGHBOR_INTERPOLATION:
      os << indent << "Interpolation method = NearestNeighbor "
         << std::endl;
      break;
    case LINEAR_INTERPOLATION:
      os << indent << "Interpolation method = Linear "
         << std::endl;
      break;
    case BSPLINE_INTERPOLATION:
      os << indent << "Interpolation method = BSpline"
         << std::endl;
      break;
    case SINC_INTERPOLATION:
      os << indent << "Interpolation method = Sinc"
         << std::endl;
      break;
    default:
      os << indent << "ERROR: Interpolation method NOT HANDLED BY OptimizedImageToImageRegistrationMethod::PrintSelf"
         << std::endl;
      break;
    }
}

}

#endif
