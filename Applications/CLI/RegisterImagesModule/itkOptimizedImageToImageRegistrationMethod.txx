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
#include "itkFRPROptimizer.h"
#include "itkImageMaskSpatialObject.h"

#include "itkBSplineImageToImageRegistrationMethod.h"

namespace itk
{

class ImageRegistrationViewer
: public Command
{
  public :
    typedef ImageRegistrationViewer     Self;
    typedef Command                Superclass;
    typedef SmartPointer<Self>     Pointer;
  
    itkTypeMacro( ImageRegistrationViewer, Command );

    itkNewMacro( ImageRegistrationViewer );
  
    typedef SingleValuedNonLinearOptimizer  OptimizerType;
  
    itkSetMacro(DontShowParameters, bool);
    itkSetMacro(UpdateInterval, int);
  
    void Execute( Object * caller, const EventObject & event )
      {
      Execute( (const Object *)caller, event );
      }

    void Execute( const Object * object, const EventObject & event )
      {
      if( typeid( event ) != typeid( IterationEvent ) || object == NULL)
        {
        return;
        }
  
      const OptimizerType * opt = dynamic_cast<const OptimizerType *>(object);
  
      if(++m_Iteration > m_UpdateInterval)
        {
        m_Iteration = 0;
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
      }

    void Update()
      {
      this->Execute ( (const Object *)NULL, IterationEvent() );
      }

  protected:

    int  m_Iteration;
    int  m_UpdateInterval;
    bool m_DontShowParameters;

    ImageRegistrationViewer() 
      { 
      m_Iteration = 0;
      m_UpdateInterval = 1;
      m_DontShowParameters = false; 
      };
    ~ImageRegistrationViewer() {};

};

  
template< class TImage >
OptimizedImageToImageRegistrationMethod< TImage >
::OptimizedImageToImageRegistrationMethod( void )
{
  m_InitialTransformParameters = TransformParametersType(1);
  m_InitialTransformParameters.Fill( 0.0f );\

  m_InitialTransformFixedParameters = TransformParametersType(1);
  m_InitialTransformFixedParameters.Fill( 0.0f );\

  m_LastTransformParameters = TransformParametersType(1);
  m_LastTransformParameters.Fill( 0.0f );

  m_TransformParametersScales = TransformParametersScalesType(1);
  m_TransformParametersScales.Fill( 1.0f );

  m_MaxIterations = 500;
  m_UseOverlapAsROI = false;
  m_MinimizeMemory = false;
  
  m_NumberOfSamples = 20000;
  m_FixedImageSamplesIntensityThreshold = 0;

  m_TargetError = 0.00001;

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

  if( m_TransformMethodEnum != BSPLINE_TRANSFORM )
    {
    this->GetTransform()
        ->SetFixedParameters( this->GetInitialTransformFixedParameters() );
    }
  this->GetTransform()
      ->SetParametersByValue( this->GetInitialTransformParameters() );

  typedef ImageToImageMetric< TImage, TImage >        MetricType;
  typename MetricType::Pointer metric;
  switch( this->GetMetricMethodEnum() )
    {
    case MATTES_MI_METRIC:
      {
      typedef MattesMutualInformationImageToImageMetric< TImage, TImage >  
                                                                TypedMetricType;
      metric = TypedMetricType::New();

      TypedMetricType * m = 
                        static_cast< TypedMetricType * >( metric.GetPointer() );
      m->SetNumberOfHistogramBins( 100 );
      if( m_MinimizeMemory && m_TransformMethodEnum == BSPLINE_TRANSFORM )
        {
        m->SetUseExplicitPDFDerivatives( false );
        m->SetUseCachingOfBSplineWeights( false );
        }
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
  typename ImageType::ConstPointer fixedImage = this->GetFixedImage();
  typename ImageType::ConstPointer movingImage = this->GetMovingImage();
  metric->SetFixedImage( fixedImage );
  metric->SetMovingImage( movingImage );
  if( this->GetUseFixedImageMaskObject() && !this->m_UseOverlapAsROI )
    {
    if( this->GetFixedImageMaskObject() )
      {
      metric->SetFixedImageMask( const_cast<itk::SpatialObject<ImageDimension> *>(this->GetFixedImageMaskObject()) );
      }
    }
  else if( this->m_UseOverlapAsROI )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "Creating mask from overlap" << std::endl;
      }

    typedef itk::OrientedImage<unsigned char, ImageDimension> MaskImageType;
    typename MaskImageType::Pointer maskImage = MaskImageType::New();
    typename ImageType::ConstPointer fixedImage = this->GetFixedImage();
    typename ImageType::ConstPointer movingImage = this->GetMovingImage();
    typename ImageType::SizeType fixedSize = fixedImage
                                             ->GetLargestPossibleRegion()
                                               .GetSize();
    typename ImageType::IndexType index;
    typename ImageType::PointType fixedPoint;
    typename ImageType::PointType movingPoint;
    maskImage->SetRegions( fixedSize );
    maskImage->CopyInformation( fixedImage );
    maskImage->Allocate();
    itk::ImageRegionIteratorWithIndex< MaskImageType > iter( maskImage,
                                    maskImage->GetLargestPossibleRegion() );
    iter.GoToBegin();
    while( !iter.IsAtEnd() )
      {
      index = iter.GetIndex();
      fixedImage->TransformIndexToPhysicalPoint(index, fixedPoint);
      movingPoint = this->GetTransform()->TransformPoint( fixedPoint );
      if( movingImage->TransformPhysicalPointToIndex( movingPoint, index ) )
        {
        if( this->GetUseFixedImageMaskObject() )
          {
          if( this->GetFixedImageMaskObject() )
            {
            if( this->GetFixedImageMaskObject()->IsInside( movingPoint ) )
              {
              iter.Set( 1 );
              }
            }
          }
        }
      else
        {
        iter.Set( 0 );
        }
      ++iter;
      }

    typedef itk::ImageMaskSpatialObject< ImageDimension > MaskObjectType;
    typename MaskObjectType::Pointer maskObject = MaskObjectType::New();
    maskObject->SetImage( maskImage );
    metric->SetFixedImageMask( maskObject );
    }

  if( this->GetUseMovingImageMaskObject() )
    {
    if( this->GetMovingImageMaskObject() )
      {
      metric->SetMovingImageMask( const_cast<itk::SpatialObject<ImageDimension> *>(this->GetMovingImageMaskObject()) );
      }
    }

#ifdef ITK_USE_REVIEW 
  #ifdef ITK_USE_OPTIMIZED_REGISTRATION_METHODS
    metric->SetNumberOfSpatialSamples( m_NumberOfSamples );
  #else
    itkWarningMacro(<< "ITK not compiled with ITK_USE_OPTIMIZED_REGISTRATION_METHODS. Performance will suffer.");
  #endif
#else
  itkWarningMacro(<< "ITK not compiled with ITK_USE_REVIEW. Performance will suffer.");
#endif

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

      typename TransformType::ParametersType nextParameters = 
                                     this->GetInitialTransformParameters();

      if( m_TransformMethodEnum == BSPLINE_TRANSFORM )
        {
        typedef BSplineImageToImageRegistrationMethod< ImageType > RegType;
        typename RegType::Pointer thisBSpline = dynamic_cast<RegType *>(this);

        int numberOfLevels = 2;
        int nextNumberOfSamples = this->GetNumberOfSamples();
        int nextNumberOfControlPoints = thisBSpline->GetNumberOfControlPoints();

        for(int level=1; level<numberOfLevels; level++)
          {
          nextNumberOfSamples /= ImageDimension;
          nextNumberOfControlPoints /= 2;
          }
        if(nextNumberOfControlPoints < 2)
          {
          nextNumberOfControlPoints = 2;
          }
        for(int level=numberOfLevels-1; level>=0; level--)
          {
          if( this->GetReportProgress() )
            {
            std::cout << "MULTIRESOLUTION LEVEL = " << level << std::endl;
            std::cout << "   Number of samples = " << nextNumberOfSamples
                      << std::endl;
            std::cout << "   Number of control points = " 
                      << nextNumberOfControlPoints << std::endl;
            }


          typename RegType::Pointer reg = RegType::New();
          typename ImageType::ConstPointer fixedImage = this->GetFixedImage();
          typename ImageType::ConstPointer movingImage = this->GetMovingImage();
          reg->SetReportProgress( this->GetReportProgress() );
          reg->SetFixedImage( fixedImage );
          reg->SetMovingImage( movingImage );
          reg->SetNumberOfControlPoints( nextNumberOfControlPoints );
          reg->SetNumberOfSamples( nextNumberOfSamples );
          reg->SetOptimizationMethodEnum(GRADIENT_OPTIMIZATION);
          reg->SetTargetError( this->GetTargetError() );
          reg->SetMinimizeMemory( this->GetMinimizeMemory() );
          reg->SetUseOverlapAsROI( this->GetUseOverlapAsROI() );
          reg->SetMaxIterations( this->GetMaxIterations() );
          reg->SetFixedImageSamplesIntensityThreshold(
                           this->GetFixedImageSamplesIntensityThreshold() );
          reg->SetMetricMethodEnum( this->GetMetricMethodEnum() );
          reg->SetInterpolationMethodEnum( this->GetInterpolationMethodEnum() );

          if( level < numberOfLevels-1 )
            {
            reg->SetInitialTransformParameters( nextParameters );
            //reg->SetInitialTransformFixedParameters( this->GetInitialTransformFixedParameters() );
            }
          std::cout << "   Registering..." << std::endl;
          try
            {
            reg->Update();
            }
          catch( itk::ExceptionObject & excep )
            {
            std::cout << "Exception caught during level registration." 
                      << excep << std::endl;
            std::cout << "Initial params = " << reg->GetInitialTransformParameters() << std::endl;
            std::cout << "Current BSpline Transform = " << std::endl; 
            reg->GetTransform()->Print(std::cout , 2);
            }
          catch( ... )
            {
            std::cout << "Uncaught exception during helper class registration." 
                      << std::endl;
            }

          nextNumberOfSamples *= ImageDimension;
          nextNumberOfControlPoints *= 2;
          if( level == 0)
            {
            nextNumberOfSamples = this->GetNumberOfSamples();
            nextNumberOfControlPoints = thisBSpline->GetNumberOfControlPoints();
            }
          std::cout << "   Resampling grid..." << std::endl;
          reg->ResampleControlGrid( nextNumberOfControlPoints, nextParameters );
          std::cout << "   Level done." << std::endl;
          }
        }
      else
        {
        int numberOfLevels = 3;
        int nextNumberOfSamples = this->GetNumberOfSamples();
        for(int level=1; level<numberOfLevels; level++)
          {
          nextNumberOfSamples /= ImageDimension;
          }
        for(int level=numberOfLevels-1; level>=0; level--)
          {
          if( this->GetReportProgress() )
            {
            std::cout << "MULTIRESOLUTION LEVEL = " << level << std::endl;
            }

          typedef OptimizedImageToImageRegistrationMethod< ImageType > RegType;
          typename RegType::Pointer reg = this->New();
          typename ImageType::ConstPointer fixedImage = this->GetFixedImage();
          typename ImageType::ConstPointer movingImage = this->GetMovingImage();
          reg->SetReportProgress( this->GetReportProgress() );
          reg->SetFixedImage( fixedImage );
          reg->SetMovingImage( movingImage );
          reg->SetNumberOfSamples( nextNumberOfSamples );
          reg->SetOptimizationMethodEnum(GRADIENT_OPTIMIZATION);
          reg->SetTargetError( this->GetTargetError() );
          reg->SetMinimizeMemory( this->GetMinimizeMemory() );
          reg->SetUseOverlapAsROI( this->GetUseOverlapAsROI() );
          reg->SetMaxIterations( this->GetMaxIterations() );
          reg->SetFixedImageSamplesIntensityThreshold(
                           this->GetFixedImageSamplesIntensityThreshold() );
          reg->SetMetricMethodEnum( this->GetMetricMethodEnum() );
          reg->SetInterpolationMethodEnum( this->GetInterpolationMethodEnum() );

          if( level < numberOfLevels-1 )
            {
            reg->SetInitialTransformParameters( nextParameters );
            reg->SetInitialTransformFixedParameters( this->GetInitialTransformFixedParameters() );
            }
          try
            {
            reg->Update();
            }
          catch( itk::ExceptionObject & excep )
            {
            std::cout << "Exception caught during level registration." 
                       << excep << std::endl;
            std::cout << "Current Matrix Transform = " << std::endl; 
            reg->GetTransform()->Print(std::cout , 2);
            }
          catch( ... )
            {
            std::cout << "Uncaught exception during level registration." 
                      << std::endl;
            }

          nextNumberOfSamples *= ImageDimension;
          if( level == 0)
            {
            nextNumberOfSamples = this->GetNumberOfSamples();
            }
          nextParameters = reg->GetLastTransformParameters();
          }
        }
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
      evoOpt->SetNormalVariateGenerator( Statistics::NormalVariateGenerator
        ::New() );
      evoOpt->SetEpsilon( this->GetTargetError() );
      evoOpt->Initialize( 0.1 );
      EvoOptimizerType::ParametersType scales = 
        this->GetTransformParametersScales();
      for(unsigned int i=0; i<scales.size(); i++)
        {
        scales[i] = scales[i] * scales[i]; // OnePlusOne opt uses squared scales
        }
      evoOpt->SetScales( scales ); //this->GetTransformParametersScales() );
      evoOpt->SetMaximumIteration( this->GetMaxIterations() );

      OptimizerType::Pointer gradOpt;
      //typedef PowellOptimizer                  GradOptimizerType;
      typedef FRPROptimizer                  GradOptimizerType;

      gradOpt = GradOptimizerType::New();
      GradOptimizerType::Pointer tmpOpt = 
        static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

      tmpOpt->SetMaximize( false );
      tmpOpt->SetStepLength( 0.25 );
      tmpOpt->SetStepTolerance( this->GetTargetError() );
      tmpOpt->SetMaximumIteration( this->GetTransform()
                                       ->GetNumberOfParameters() * 4 );
      tmpOpt->SetMaximumLineIteration( (int)( this->GetMaxIterations() 
                                       / (this->GetTransform()
                                              ->GetNumberOfParameters()
                                          * 0.5) ) );
      tmpOpt->SetScales( this->GetTransformParametersScales() );
      tmpOpt->SetUseUnitLengthGradient(true);

      if( this->GetReportProgress() )
        {
        typedef ImageRegistrationViewer ViewerCommandType;
        typename ViewerCommandType::Pointer command = ViewerCommandType::New();
        if( this->GetTransform()->GetNumberOfParameters() > 16)
          {
          command->SetDontShowParameters( true );
          }
        evoOpt->AddObserver( IterationEvent(), command );
        gradOpt->AddObserver( IterationEvent(), command );
        }

      if( this->GetObserver() )
        {
        evoOpt->AddObserver( IterationEvent(), this->GetObserver() );
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
      if( m_TransformMethodEnum != BSPLINE_TRANSFORM )
        {
        reg->GetTransform()->SetFixedParameters(
                               this->GetInitialTransformFixedParameters() );
        }
      reg->SetInitialTransformParameters( 
                               this->GetInitialTransformParameters() );
      reg->SetMetric( metric );
      reg->SetInterpolator( interpolator );
      reg->SetOptimizer( evoOpt );

      reg->StartRegistration();

      m_FinalMetricValue = reg->GetOptimizer()->GetValue( 
                                           reg->GetLastTransformParameters() );

      this->SetLastTransformParameters( reg->GetLastTransformParameters() );
      this->GetTransform()->SetParametersByValue( 
                                          this->GetLastTransformParameters() );

      gradOpt->SetCostFunction( metric );
      gradOpt->SetInitialPosition( reg->GetLastTransformParameters() );

      if( this->GetReportProgress() )
        {
        std::cout << "EVOLUTIONARY GRADIENT START" << std::endl;
        }

      try
        {
        gradOpt->StartOptimization();
        }
      catch( itk::ExceptionObject & excep )
        {
        std::cout << "Exception caught during gradient registration." 
                   << excep << std::endl;
        std::cout << "  Pos = " << gradOpt->GetCurrentPosition() 
                  << std::endl << std::endl;
        std::cout << "  Value = " 
                  << gradOpt->GetValue( gradOpt->GetCurrentPosition() ) 
                  << std::endl;;
        }
      catch(...)
        {
        std::cout << "Exception caught...continuing using best values..." 
                  << std::endl;
        std::cout << "  Pos = " << gradOpt->GetCurrentPosition() 
                  << std::endl << std::endl;
        std::cout << "  Value = " 
                  << gradOpt->GetValue( gradOpt->GetCurrentPosition() ) 
                  << std::endl;;
        }
        
      m_FinalMetricValue = gradOpt->GetValue( gradOpt->GetCurrentPosition() );

      this->SetLastTransformParameters( gradOpt->GetCurrentPosition() );
      this->GetTransform()
          ->SetParametersByValue( this->GetLastTransformParameters() );

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

      OptimizerType::Pointer gradOpt;
      if( m_TransformMethodEnum == BSPLINE_TRANSFORM )
        {
        int numberOfParameters = this->GetTransform()->GetNumberOfParameters();

        typedef LBFGSBOptimizer                  GradOptimizerType;
        gradOpt = GradOptimizerType::New();
        GradOptimizerType::Pointer tmpOpt =
                      static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

        GradOptimizerType::BoundSelectionType boundSelect( numberOfParameters );
        GradOptimizerType::BoundValueType upperBound( numberOfParameters );
        GradOptimizerType::BoundValueType lowerBound( numberOfParameters );
        boundSelect.Fill( 0 );
        upperBound.Fill( 0.0 );
        lowerBound.Fill( 0.0 );
        tmpOpt->SetBoundSelection( boundSelect );
        tmpOpt->SetUpperBound( upperBound );
        tmpOpt->SetLowerBound( lowerBound );
        tmpOpt->SetCostFunctionConvergenceFactor( 1.0 / this->GetTargetError() );
        tmpOpt->SetProjectedGradientTolerance( 1e-10 );
        tmpOpt->SetMaximumNumberOfIterations( this->GetMaxIterations() );
        tmpOpt->SetMaximumNumberOfEvaluations( this->GetMaxIterations() );
        tmpOpt->SetMaximumNumberOfCorrections( 
                                       (int)log( (float)numberOfParameters ) );
        }
      else
        {
        //typedef PowellOptimizer                  GradOptimizerType;
        typedef FRPROptimizer                  GradOptimizerType;

        gradOpt = GradOptimizerType::New();
        GradOptimizerType::Pointer tmpOpt = 
                       static_cast<GradOptimizerType *>( gradOpt.GetPointer() );

        tmpOpt->SetMaximize( false );
        tmpOpt->SetStepLength( 0.25 );
        tmpOpt->SetStepTolerance( this->GetTargetError() );
        tmpOpt->SetMaximumIteration( this->GetTransform()
                                         ->GetNumberOfParameters() * 4 );
        tmpOpt->SetMaximumLineIteration( (int)( this->GetMaxIterations() / 
                      (this->GetTransform()->GetNumberOfParameters() * 0.5) ) );
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
      if( m_TransformMethodEnum != BSPLINE_TRANSFORM )
        {
        reg->GetTransform()->SetFixedParameters( 
                               this->GetInitialTransformFixedParameters() );
        }
      reg->SetInitialTransformParameters( 
                               this->GetInitialTransformParameters() );
      reg->SetMetric( metric );
      reg->SetInterpolator( interpolator );
      reg->SetOptimizer( gradOpt );

      reg->StartRegistration();

      m_FinalMetricValue = reg->GetOptimizer()->GetValue( reg->GetLastTransformParameters() );

      this->SetLastTransformParameters( reg->GetLastTransformParameters() );
      this->GetTransform()->SetParametersByValue( this->GetLastTransformParameters() );

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

  os << indent << "Initial Transform Fixed Parameters = " << m_InitialTransformFixedParameters << std::endl;

  os << indent << "Last Transform Parameters = " << m_LastTransformParameters << std::endl;

  os << indent << "Transform Parameter Scales = " << m_TransformParametersScales << std::endl;

  os << indent << "Max Iterations = " << m_MaxIterations << std::endl;

  os << indent << "Use Overlap As ROI = " << m_UseOverlapAsROI << std::endl;

  os << indent << "Minimize Memory = " << m_MinimizeMemory << std::endl;

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
      os << indent << "ERROR: Interpolation method NOT HANDLED BY OptimizedImageToImageRegistrationMethod::PrintSelf"
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
      os << indent << "Optimization method = Gradient" << std::endl;
      break;
    default:
      os << indent << "ERROR: Optimization method NOT HANDLED BY OptimizedImageToImageRegistrationMethod::PrintSelf"
         << std::endl;
      break;
    }
}

};

#endif 
