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


#ifndef __OptimizedImageToImageRegistrationMethod_txx
#define __OptimizedImageToImageRegistrationMethod_txx

#include "itkOptimizedImageToImageRegistrationMethod.h"

#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"

#include "itkLinearInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"

#include "itkImageRegistrationMethod.h"
#include "itkMultiResolutionImageRegistrationMethod.h"

#include "itkSingleValuedNonLinearOptimizer.h"
#include "itkOnePlusOneEvolutionaryOptimizer.h"
#include "itkNormalVariateGenerator.h"
#include "itkLBFGSBOptimizer.h"
//#include "itkPowellOptimizer.h"
#include "itkFRPROptimizer.h"

namespace itk
{

class ImageRegistrationViewer
: public itk::Command
{
  public :
    typedef ImageRegistrationViewer     Self;
    typedef itk::Command                Superclass;
    typedef itk::SmartPointer<Self>     Pointer;
  
    itkTypeMacro( ImageRegistrationViewer, itk::Command );

    itkNewMacro( ImageRegistrationViewer );
  
    typedef itk::SingleValuedNonLinearOptimizer  OptimizerType;
  
    itkSetMacro(DontShowParameters, bool);
  
    void Execute( itk::Object * caller, const itk::EventObject & event )
      {
      Execute( (const itk::Object *)caller, event );
      }

    void Execute( const itk::Object * object, const itk::EventObject & event )
      {
      if( typeid( event ) != typeid( itk::IterationEvent ) || object == NULL)
        {
        return;
        }
  
      const OptimizerType * opt = dynamic_cast<const OptimizerType *>(object);
  
      if(!m_DontShowParameters)
        {
        std::cout << "   " << opt->GetCurrentPosition() << " : "
          << opt->GetValue( opt->GetCurrentPosition() )
          << std::endl;
        }
      else
        {
        std::cout << "   " << opt->GetValue( opt->GetCurrentPosition() )
          << std::endl;
        }
      }

    void Update()
      {
      this->Execute ( (const itk::Object *)NULL, itk::IterationEvent() );
      }

  protected:

    bool m_DontShowParameters;

    ImageRegistrationViewer() { m_DontShowParameters = false; };
    ~ImageRegistrationViewer() {};

};

  
  
  // For Hierarchical registration
template< typename TRegistration >
class HierarchicalRegistrationLevelObserver
: public itk::Command
{
  public:
    typedef  HierarchicalRegistrationLevelObserver  Self;
    typedef  itk::Command                           Superclass;
    typedef  itk::SmartPointer<Self>                Pointer;

    itkTypeMacro( HierarchicalRegistrationLevelObserver, itk::Command );

    itkNewMacro( Self );

  protected:

    HierarchicalRegistrationLevelObserver() {};

  public:

    typedef   TRegistration                              RegistrationType;
    typedef   RegistrationType *                         RegistrationPointer;

    typedef   SingleValuedNonLinearOptimizer             OptimizerType;
    typedef   OptimizerType *                            OptimizerPointer;
    typedef   OptimizerType::ScalesType                  ScalesType;

    void Execute(itk::Object * object, const itk::EventObject & event)
      {
      if( !(itk::IterationEvent().CheckEvent( &event )) )
        {
        return;
        }
      RegistrationPointer registration =
                            dynamic_cast<RegistrationPointer>( object );
      OptimizerPointer optimizer = 
                            dynamic_cast< OptimizerPointer >(
                                 registration->GetOptimizer() );

      int level = registration->GetCurrentLevel();
      if( level == 0 )
        {
        return;
        }
      else
        {
        OptimizerType::ParametersType tmpScales = optimizer->GetScales();
        for(unsigned int i=0; i<tmpScales.size(); i++)
          {
          tmpScales[i] = tmpScales[i] / 2;
          }
        optimizer->SetScales( tmpScales );
        }
      }

    void Execute(const itk::Object * , const itk::EventObject & )
      {
      return;
      }

  private:

};


template< class TImage >
OptimizedImageToImageRegistrationMethod< TImage >
::OptimizedImageToImageRegistrationMethod( void )
{
  m_InitialTransformParameters = TransformParametersType(1);
  m_InitialTransformParameters.Fill( 0.0f );\

  m_LastTransformParameters = TransformParametersType(1);
  m_LastTransformParameters.Fill( 0.0f );

  m_TransformParametersScales = TransformParametersScalesType(1);
  m_TransformParametersScales.Fill( 1.0f );

  m_MaxIterations = 500;
  
  m_NumberOfSamples = 20000;
  m_FixedImageSamplesIntensityThreshold = 0;

  m_TargetError = 0.001;

  m_TransformMethodEnum = RIGID_TRANSFORM;

  m_MetricMethodEnum = MATTES_MI_METRIC;
  m_InterpolationMethodEnum = LINEAR_INTERPOLATION;
  m_OptimizationMethodEnum = EVOLUTIONARY_OPTIMIZATION;

  m_FinalMetricValue = 0;

}

template< class TImage >
OptimizedImageToImageRegistrationMethod< TImage >
::~OptimizedImageToImageRegistrationMethod( void )
{
}

template< class TImage >
void
OptimizedImageToImageRegistrationMethod< TImage >
::Update( void )
{
  this->Initialize();

  if( this->GetReportProgress() )
    {
    std::cout << "UPDATE START" << std::endl;
    }

  this->GetTransform()->SetParametersByValue( this->GetInitialTransformParameters() );

  typedef ImageToImageMetric< TImage, TImage >        MetricType;
  typename MetricType::Pointer metric;
  switch( this->GetMetricMethodEnum() )
    {
    case MATTES_MI_METRIC:
      {
      typedef MattesMutualInformationImageToImageMetric< TImage, TImage >  TypedMetricType;
      metric = TypedMetricType::New();

      TypedMetricType * m = static_cast< TypedMetricType * >( metric.GetPointer() );
      m->SetNumberOfHistogramBins( 200 );
      break;
      }
    case NORMALIZED_CORRELATION_METRIC:
      {
      metric = NormalizedCorrelationImageToImageMetric< TImage, TImage >::New();
      break;
      }
    case MEAN_SQUARED_ERROR_METRIC:
      {
      metric = MeanSquaresImageToImageMetric< TImage, TImage >::New();
      break;
      }
    }
  metric->SetFixedImage( this->GetMovingImage() );
  metric->SetMovingImage( this->GetMovingImage() );
  
  typedef InterpolateImageFunction< TImage, double >        InterpolationType;
  typename InterpolationType::Pointer interpolator;
  switch( this->GetInterpolationMethodEnum() )
    {
    case LINEAR_INTERPOLATION:
      {
      interpolator = LinearInterpolateImageFunction< TImage, double >::New();
      break;
      }
    case BSPLINE_INTERPOLATION:
      {
      interpolator = BSplineInterpolateImageFunction< TImage, double >::New();
      break;
      }
    case SINC_INTERPOLATION:
      {
      interpolator = WindowedSincInterpolateImageFunction< TImage, 
                                                           4,
                                                           Function::HammingWindowFunction< 4 >,
                                                           ConstantBoundaryCondition< TImage >,
                                                           double >::New();
      break;
      }
    }
  interpolator->SetInputImage( this->GetMovingImage() );

  switch( this->GetOptimizationMethodEnum() )
    {
    case MULTIRESOLUTION_OPTIMIZATION:
      {
      if( this->GetReportProgress() )
        {
        std::cout << "MULTIRESOLUTION START" << std::endl;
        }

      typedef MultiResolutionImageRegistrationMethod< TImage, TImage > RegType;
      typedef MultiResolutionPyramidImageFilter< TImage, TImage >      PyramidType;
      typedef SingleValuedNonLinearOptimizer                           OptimizerType;

      int numberOfParameters = this->GetTransform()->GetNumberOfParameters();

      OptimizerType::Pointer gradOpt;
      if( m_TransformMethodEnum == BSPLINE_TRANSFORM )
        {
        typedef LBFGSBOptimizer                  GradOptimizerType;
        gradOpt = GradOptimizerType::New();
        GradOptimizerType::Pointer tmpOpt = static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

        GradOptimizerType::BoundSelectionType boundSelect( numberOfParameters );
        GradOptimizerType::BoundValueType upperBound( numberOfParameters );
        GradOptimizerType::BoundValueType lowerBound( numberOfParameters );
        boundSelect.Fill( 0 );
        upperBound.Fill( 0.0 );
        lowerBound.Fill( 0.0 );
        tmpOpt->SetBoundSelection( boundSelect );
        tmpOpt->SetUpperBound( upperBound );
        tmpOpt->SetLowerBound( lowerBound );
        tmpOpt->SetCostFunctionConvergenceFactor( this->GetTargetError() );
        tmpOpt->SetProjectedGradientTolerance( 1e-10 );
        tmpOpt->SetMaximumNumberOfIterations( this->GetMaxIterations() );
        tmpOpt->SetMaximumNumberOfEvaluations( this->GetMaxIterations() );
        tmpOpt->SetMaximumNumberOfCorrections( (int)log( (float)numberOfParameters ) );
        }
      else
        {
        //typedef PowellOptimizer                  GradOptimizerType;
        typedef FRPROptimizer                  GradOptimizerType;

        gradOpt = GradOptimizerType::New();
        GradOptimizerType::Pointer tmpOpt = static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

        tmpOpt->SetMaximize( false );
        tmpOpt->SetStepLength( 2 );
        tmpOpt->SetStepTolerance( this->GetTargetError() );
        tmpOpt->SetMaximumIteration( this->GetTransform()->GetNumberOfParameters() * 4 );
        tmpOpt->SetMaximumLineIteration( (int)( this->GetMaxIterations() / (this->GetTransform()->GetNumberOfParameters() * 0.5) ) );
        tmpOpt->SetScales( this->GetTransformParametersScales() );
        tmpOpt->SetUseUnitLengthGradient(true);
        }
 
      if( this->GetReportProgress() )
        {
        typedef ImageRegistrationViewer ViewerCommandType;
        typename ViewerCommandType::Pointer command = ViewerCommandType::New();
        if( this->GetTransform()->GetNumberOfParameters() > 16)
          {
          command->SetDontShowParameters( true );
          }
        gradOpt->AddObserver( itk::IterationEvent(), command );
        }

      if( this->GetObserver() )
        {
        gradOpt->AddObserver( itk::IterationEvent(), this->GetObserver() );
        }

      typename RegType::Pointer reg = RegType::New();

      reg->SetFixedImage( this->GetFixedImage() );
      reg->SetMovingImage( this->GetMovingImage() );
      reg->SetFixedImageRegion( this->GetFixedImage()->GetLargestPossibleRegion() );

      typename PyramidType::Pointer fixedImagePyramid = PyramidType::New();
      typename PyramidType::Pointer movingImagePyramid = PyramidType::New();

      reg->SetFixedImagePyramid( fixedImagePyramid );
      reg->SetMovingImagePyramid( movingImagePyramid );
      reg->SetNumberOfLevels( 3 );


      // Add the observer that adjusts registration params between levels of the hierarchy
      typedef HierarchicalRegistrationLevelObserver< RegType > HierarchicalCommandType;
      typename HierarchicalCommandType::Pointer hCommand = HierarchicalCommandType::New();
      reg->AddObserver( itk::IterationEvent(), hCommand );

      if( this->GetUseMasks() )
        {
        if( this->GetFixedImageMaskObject() )
          {
          metric->SetFixedImageMask( this->GetFixedImageMaskObject() );
          }
        if( this->GetMovingImageMaskObject() )
          {
          metric->SetMovingImageMask( this->GetMovingImageMaskObject() );
          }
        }

      reg->SetTransform( this->GetTransform() );
      reg->SetInitialTransformParameters( this->GetInitialTransformParameters() );
      reg->SetMetric( metric );
      reg->SetInterpolator( interpolator );
      reg->SetOptimizer( gradOpt );

      reg->StartRegistration();

      this->SetLastTransformParameters( reg->GetLastTransformParameters() );
      this->GetTransform()->SetParametersByValue( this->GetLastTransformParameters() );

      m_FinalMetricValue = reg->GetOptimizer()->GetValue( this->GetLastTransformParameters() );

      if( this->GetReportProgress() )
        {
        std::cout << "MULTIRESOLUTION END" << std::endl;
        }
      break;
      }
    case EVOLUTIONARY_OPTIMIZATION:
      {
      if( this->GetReportProgress() )
        {
        std::cout << "EVOLUTIONARY START" << std::endl;
        }

      typedef ImageRegistrationMethod< TImage, TImage >  RegType;
      typedef OnePlusOneEvolutionaryOptimizer            EvoOptimizerType;
      typedef SingleValuedNonLinearOptimizer             OptimizerType;

      EvoOptimizerType::Pointer evoOpt = EvoOptimizerType::New();
      evoOpt->SetNormalVariateGenerator( Statistics::NormalVariateGenerator::New() );
      evoOpt->SetEpsilon( 1e-10 );
      evoOpt->Initialize( 1.01 );
      evoOpt->SetScales( this->GetTransformParametersScales() );
      evoOpt->SetMaximumIteration( this->GetMaxIterations() / 2 );

      int numberOfParameters = this->GetTransform()->GetNumberOfParameters();

      OptimizerType::Pointer gradOpt;
      if( m_TransformMethodEnum == BSPLINE_TRANSFORM )
        {
        typedef LBFGSBOptimizer                  GradOptimizerType;
        gradOpt = GradOptimizerType::New();
        GradOptimizerType::Pointer tmpOpt = static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

        GradOptimizerType::BoundSelectionType boundSelect( numberOfParameters );
        GradOptimizerType::BoundValueType upperBound( numberOfParameters );
        GradOptimizerType::BoundValueType lowerBound( numberOfParameters );
        boundSelect.Fill( 0 );
        upperBound.Fill( 0.0 );
        lowerBound.Fill( 0.0 );
        tmpOpt->SetBoundSelection( boundSelect );
        tmpOpt->SetUpperBound( upperBound );
        tmpOpt->SetLowerBound( lowerBound );
        tmpOpt->SetCostFunctionConvergenceFactor( this->GetTargetError() );
        tmpOpt->SetProjectedGradientTolerance( 1e-10 );
        tmpOpt->SetMaximumNumberOfIterations( this->GetMaxIterations() / 2 );
        tmpOpt->SetMaximumNumberOfEvaluations( this->GetMaxIterations() / 2 );
        tmpOpt->SetMaximumNumberOfCorrections( (int)log( (float)numberOfParameters ) );
        }
      else
        {
        //typedef PowellOptimizer                  GradOptimizerType;
        typedef FRPROptimizer                  GradOptimizerType;

        gradOpt = GradOptimizerType::New();
        GradOptimizerType::Pointer tmpOpt = static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

        tmpOpt->SetMaximize( false );
        tmpOpt->SetStepLength( 0.25 );
        tmpOpt->SetStepTolerance( this->GetTargetError() );
        tmpOpt->SetMaximumIteration( this->GetTransform()->GetNumberOfParameters() * 4 );
        tmpOpt->SetMaximumLineIteration( (int)( this->GetMaxIterations() / (this->GetTransform()->GetNumberOfParameters() * 0.5) ) );
        tmpOpt->SetScales( this->GetTransformParametersScales() );
        tmpOpt->SetUseUnitLengthGradient(true);
        }

      if( this->GetReportProgress() )
        {
        typedef ImageRegistrationViewer ViewerCommandType;
        typename ViewerCommandType::Pointer command = ViewerCommandType::New();
        if( this->GetTransform()->GetNumberOfParameters() > 16)
          {
          command->SetDontShowParameters( true );
          }
        evoOpt->AddObserver( itk::IterationEvent(), command );
        gradOpt->AddObserver( itk::IterationEvent(), command );
        }

      if( this->GetObserver() )
        {
        evoOpt->AddObserver( itk::IterationEvent(), this->GetObserver() );
        gradOpt->AddObserver( itk::IterationEvent(), this->GetObserver() );
        }

      typename RegType::Pointer reg = RegType::New();
      reg->SetFixedImage( this->GetFixedImage() );
      reg->SetMovingImage( this->GetMovingImage() );
      reg->SetFixedImageRegion( this->GetFixedImage()->GetLargestPossibleRegion() );

      if( this->GetUseMasks() )
        {
        if( this->GetFixedImageMaskObject() )
          {
          metric->SetFixedImageMask( this->GetFixedImageMaskObject() );
          }
        if( this->GetMovingImageMaskObject() )
          {
          metric->SetMovingImageMask( this->GetMovingImageMaskObject() );
          }
        }

      reg->SetTransform( this->GetTransform() );
      reg->SetInitialTransformParameters( this->GetInitialTransformParameters() );
      reg->SetMetric( metric );
      reg->SetInterpolator( interpolator );
      reg->SetOptimizer( evoOpt );

      reg->StartRegistration();

      this->SetLastTransformParameters( reg->GetLastTransformParameters() );
      this->GetTransform()->SetParametersByValue( this->GetLastTransformParameters() );

      m_FinalMetricValue = reg->GetOptimizer()->GetValue( this->GetLastTransformParameters() );

      gradOpt->SetCostFunction( metric );
      gradOpt->SetInitialPosition( reg->GetLastTransformParameters() );

      try
        {
        gradOpt->StartOptimization();
        }
      catch(...)
        {
        std::cout << "Exception caught...continuing using best previous values..." << std::endl;
        std::cout << "  Pos = " << gradOpt->GetCurrentPosition() << std::endl << std::endl;
        std::cout << "  Value = " << gradOpt->GetValue( gradOpt->GetCurrentPosition() ) << std::endl;;
        }
        

      this->SetLastTransformParameters( gradOpt->GetCurrentPosition() );
      this->GetTransform()->SetParametersByValue( this->GetLastTransformParameters() );

      m_FinalMetricValue = reg->GetOptimizer()->GetValue( this->GetLastTransformParameters() );
      if( this->GetReportProgress() )
        {
        std::cout << "EVOLUTIONARY END" << std::endl;
        }
      break;
      }
    case GRADIENT_OPTIMIZATION:
      {
      if( this->GetReportProgress() )
        {
        std::cout << "GRADIENT START" << std::endl;
        }

      typedef SingleValuedNonLinearOptimizer            OptimizerType;

      typedef ImageRegistrationMethod< TImage, TImage > RegType;

      int numberOfParameters = this->GetTransform()->GetNumberOfParameters();

      OptimizerType::Pointer gradOpt;
      if( m_TransformMethodEnum == BSPLINE_TRANSFORM )
        {
        typedef LBFGSBOptimizer                  GradOptimizerType;
        gradOpt = GradOptimizerType::New();
        GradOptimizerType::Pointer tmpOpt = static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

        GradOptimizerType::BoundSelectionType boundSelect( numberOfParameters );
        GradOptimizerType::BoundValueType upperBound( numberOfParameters );
        GradOptimizerType::BoundValueType lowerBound( numberOfParameters );
        boundSelect.Fill( 0 );
        upperBound.Fill( 0.0 );
        lowerBound.Fill( 0.0 );
        tmpOpt->SetBoundSelection( boundSelect );
        tmpOpt->SetUpperBound( upperBound );
        tmpOpt->SetLowerBound( lowerBound );
        tmpOpt->SetCostFunctionConvergenceFactor( this->GetTargetError() );
        tmpOpt->SetProjectedGradientTolerance( 1e-10 );
        tmpOpt->SetMaximumNumberOfIterations( this->GetMaxIterations() );
        tmpOpt->SetMaximumNumberOfEvaluations( this->GetMaxIterations() );
        tmpOpt->SetMaximumNumberOfCorrections( (int)log( (float)numberOfParameters ) );
        }
      else
        {
        //typedef PowellOptimizer                  GradOptimizerType;
        typedef FRPROptimizer                  GradOptimizerType;

        gradOpt = GradOptimizerType::New();
        GradOptimizerType::Pointer tmpOpt = static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

        tmpOpt->SetMaximize( false );
        tmpOpt->SetStepLength( 0.25 );
        tmpOpt->SetStepTolerance( this->GetTargetError() );
        tmpOpt->SetMaximumIteration( this->GetTransform()->GetNumberOfParameters() * 4 );
        tmpOpt->SetMaximumLineIteration( (int)( this->GetMaxIterations() / (this->GetTransform()->GetNumberOfParameters() * 0.5) ) );
        tmpOpt->SetScales( this->GetTransformParametersScales() );
        tmpOpt->SetUseUnitLengthGradient(true);
        }

      if( this->GetReportProgress() )
        {
        typedef ImageRegistrationViewer ViewerCommandType;
        typename ViewerCommandType::Pointer command = ViewerCommandType::New();
        if( this->GetTransform()->GetNumberOfParameters() > 16)
          {
          command->SetDontShowParameters( true );
          }
        gradOpt->AddObserver( itk::IterationEvent(), command );
        }

      if( this->GetObserver() )
        {
        gradOpt->AddObserver( itk::IterationEvent(), this->GetObserver() );
        }

      typename RegType::Pointer reg = RegType::New();
      reg->SetFixedImage( this->GetFixedImage() );
      reg->SetMovingImage( this->GetMovingImage() );
      reg->SetFixedImageRegion( this->GetFixedImage()->GetLargestPossibleRegion() );

      if( this->GetUseMasks() )
        {
        if( this->GetFixedImageMaskObject() )
          {
          metric->SetFixedImageMask( this->GetFixedImageMaskObject() );
          }
        if( this->GetMovingImageMaskObject() )
          {
          metric->SetMovingImageMask( this->GetMovingImageMaskObject() );
          }
        }

      reg->SetTransform( this->GetTransform() );
      reg->SetInitialTransformParameters( this->GetInitialTransformParameters() );
      reg->SetMetric( metric );
      reg->SetInterpolator( interpolator );
      reg->SetOptimizer( gradOpt );

      reg->StartRegistration();

      this->SetLastTransformParameters( reg->GetLastTransformParameters() );
      this->GetTransform()->SetParametersByValue( this->GetLastTransformParameters() );

      m_FinalMetricValue = reg->GetOptimizer()->GetValue( this->GetLastTransformParameters() );

      if( this->GetReportProgress() )
        {
        std::cout << "GRADIENT END" << std::endl;
        }
      break;
      }
    }

  if( this->GetReportProgress() )
    {
    std::cout << "UPDATE END" << std::endl;
    }
}


template< class TImage >
void
OptimizedImageToImageRegistrationMethod< TImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Initial Transform Parameters = " << m_InitialTransformParameters << std::endl;

  os << indent << "Last Transform Parameters = " << m_LastTransformParameters << std::endl;

  os << indent << "Transform Parameter Scales = " << m_TransformParametersScales << std::endl;

  os << indent << "Max Iterations = " << m_MaxIterations << std::endl;

  os << indent << "Number of Samples = " << m_NumberOfSamples << std::endl;

  os << indent << "Samples threshold = " << m_FixedImageSamplesIntensityThreshold << std::endl;

  os << indent << "Target Error = " << m_TargetError << std::endl;

  switch( m_MetricMethodEnum )
    {
    case MATTES_MI_METRIC :
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
      os << indent << "ERROR: Interpolation method NOT HANDLED BY itk::OptimizedImageToImageRegistrationMethod::PrintSelf"
         << std::endl;
      break;
    }

  switch( m_OptimizationMethodEnum )
    {
    case MULTIRESOLUTION_OPTIMIZATION:
      os << indent << "Optimization method = Multiresolution" << std::endl;
      break;
    case EVOLUTIONARY_OPTIMIZATION:
      os << indent << "Optimization method = Evolution" << std::endl;
      break;
    case GRADIENT_OPTIMIZATION:
      os << indent << "Optimization method = Evolution" << std::endl;
      break;
    default:
      os << indent << "ERROR: Optimization method NOT HANDLED BY itk::OptimizedImageToImageRegistrationMethod::PrintSelf"
         << std::endl;
      break;
    }
}

};

#endif 
