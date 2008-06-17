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


#ifndef __BSplineImageToImageRegistrationMethod_txx
#define __BSplineImageToImageRegistrationMethod_txx

#include "itkBSplineImageToImageRegistrationMethod.h"

#include "itkBSplineResampleImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkResampleImageFilter.h"
#include "itkBSplineDecompositionImageFilter.h"
#include "itkImageFileWriter.h"

#include "itkLBFGSBOptimizer.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkGradientDescentOptimizer.h"

#include "itkRealTimeClock.h"
#include "itkCommand.h"

namespace itk
{

class BSplineImageRegistrationViewer
: public Command
{
  public :
    typedef BSplineImageRegistrationViewer     Self;
    typedef Command                Superclass;
    typedef SmartPointer<Self>     Pointer;
  
    itkTypeMacro( BSplineImageRegistrationViewer, Command );

    itkNewMacro( BSplineImageRegistrationViewer );
  
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
  
      if(++m_Iteration % m_UpdateInterval == 0)
        {
        RealTimeClock::TimeStampType t = m_Clock->GetTimeStamp();
        if(!m_DontShowParameters)
          {
          std::cout << "   " << m_Iteration << " : " 
                    << opt->GetCurrentPosition() << " = "
                    << opt->GetValue( opt->GetCurrentPosition() )
                    << "   (" << (t - m_LastTime)/m_UpdateInterval << "s)"
                    << std::endl;
          }
        else
          {
          std::cout << "   " << m_Iteration << " : " 
                    << opt->GetValue( opt->GetCurrentPosition() )
                    << "   (" << (t - m_LastTime)/m_UpdateInterval << "s)"
                    << std::endl;
          }
        m_LastTime = t;
        }
      }

    void Update()
      {
      this->Execute ( (const Object *)NULL, IterationEvent() );
      }

  protected:

    RealTimeClock::Pointer       m_Clock;
    RealTimeClock::TimeStampType m_LastTime;

    int  m_Iteration;
    int  m_UpdateInterval;
    bool m_DontShowParameters;

    BSplineImageRegistrationViewer() 
      { 
      m_Clock = RealTimeClock::New();
      m_LastTime = m_Clock->GetTimeStamp();
      m_Iteration = 0;
      m_UpdateInterval = 1;
      m_DontShowParameters = false; 
      };
    ~BSplineImageRegistrationViewer() {};

};


template< class TImage >
BSplineImageToImageRegistrationMethod< TImage >
::BSplineImageToImageRegistrationMethod( void )
{
  m_NumberOfControlPoints = 10;
  m_ExpectedDeformationMagnitude = 10;
  m_GradientOptimizeOnly = false;
  this->SetTransformMethodEnum( Superclass::BSPLINE_TRANSFORM );
  //this->SetUseOverlapAsROI( true );
  
  // Override superclass defaults:
  this->SetMaxIterations( 20 );
  this->SetNumberOfSamples( 200000 );
  //this->SetInterpolationMethodEnum( Superclass::BSPLINE_INTERPOLATION );
}

template< class TImage >
BSplineImageToImageRegistrationMethod< TImage >
::~BSplineImageToImageRegistrationMethod( void )
{
}

template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::ComputeGridRegion( int numberOfControlPoints,
                 typename TransformType::RegionType::SizeType & gridSize,
                 typename TransformType::SpacingType & gridSpacing,
                 typename TransformType::OriginType & gridOrigin,
                 typename TransformType::DirectionType & gridDirection)
{
  if(numberOfControlPoints < 2)
    {
    itkWarningMacro(<< "ComputeGridRegion: numberOfControlPoints=1; changing to 2.");
    numberOfControlPoints = 2;
    }

  typename TransformType::RegionType::SizeType gridSizeOnImage;
  typename TransformType::RegionType::SizeType gridBorderSize;

  typename TImage::SizeType fixedImageSize = this->GetFixedImage()->GetLargestPossibleRegion().GetSize();

  gridSpacing   = this->GetFixedImage()->GetSpacing();

  double scale = (fixedImageSize[0] * gridSpacing[0]) / numberOfControlPoints;
  gridSizeOnImage[0] = numberOfControlPoints;
  for(unsigned int i=1; i<ImageDimension; i++)
    {
    gridSizeOnImage[i] = (int)((fixedImageSize[i] * gridSpacing[i]) / scale);
    if( gridSizeOnImage[i] < 2 )
      {
      gridSizeOnImage[i] = 2;
      }
    }
  gridBorderSize.Fill( 3 );  // Border for spline order = 3 ( 1 lower, 2 upper )

  gridSize = gridSizeOnImage + gridBorderSize;

  gridOrigin    = this->GetFixedImage()->GetOrigin();
  gridDirection    = this->GetFixedImage()->GetDirection();

  for(unsigned int r=0; r<ImageDimension; r++)
    {
    gridSpacing[r] *=  static_cast<double>(fixedImageSize[r] - 1)  /
                         static_cast<double>(gridSizeOnImage[r] - 1);
    }

  typename TransformType::SpacingType gridOriginOffset;
  gridOriginOffset = gridDirection * gridSpacing;

  gridOrigin = gridOrigin - gridOriginOffset;

  //std::cout << "gridSize = " << gridSize << std::endl;
  //std::cout << "gridSpacing = " << gridSpacing << std::endl;
  //std::cout << "gridOrigin = " << gridOrigin << std::endl;
  //std::cout << "gridDirection = " << gridDirection << std::endl;
}

 
template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::GenerateData( void )
{
  this->SetTransform( BSplineTransformType::New() );

  typename TransformType::RegionType gridRegion;
  typename TransformType::RegionType::SizeType gridSize;
  typename TransformType::SpacingType  gridSpacing;
  typename TransformType::OriginType  gridOrigin;
  typename TransformType::DirectionType  gridDirection;

  this->ComputeGridRegion( m_NumberOfControlPoints,
                           gridSize, gridSpacing, gridOrigin,
                           gridDirection);

  gridRegion.SetSize( gridSize );

  this->GetTypedTransform()->SetGridRegion( gridRegion );
  this->GetTypedTransform()->SetGridSpacing( gridSpacing );
  this->GetTypedTransform()->SetGridOrigin( gridOrigin );
  this->GetTypedTransform()->SetGridDirection( gridDirection );

  const unsigned int numberOfParameters =
               this->GetTypedTransform()->GetNumberOfParameters();

  this->SetInitialTransformFixedParameters( this->GetTypedTransform()->GetFixedParameters() );

  if( numberOfParameters != this->GetInitialTransformParameters().GetSize() )
    {
    typename Superclass::TransformParametersType params( numberOfParameters );
    params.Fill( 0.0 );
    this->GetTypedTransform()->SetParametersByValue( params );
  
    this->SetInitialTransformParameters( this->GetTypedTransform()->GetParameters() );
    }

  typename Superclass::TransformParametersType params( numberOfParameters );
  typename TImage::SizeType fixedImageSize = this->GetFixedImage()->GetLargestPossibleRegion().GetSize();
  typename TransformType::SpacingType spacing   = this->GetFixedImage()->GetSpacing();
  double scale = 1.0 / (m_ExpectedDeformationMagnitude * spacing[0]);
  std::cout << "BSpline Parameter Scale = " << scale << std::endl;
  params.Fill( scale );
  this->SetTransformParametersScales( params );

  this->Superclass::GenerateData();
}

template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::Optimize( MetricType * metric, InterpolatorType * interpolator )
{
  if( this->GetGradientOptimizeOnly() )
    {
    this->GradientOptimize( metric, interpolator );
    }
  else
    {
    this->MultiResolutionOptimize( metric, interpolator );
    }
}

template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::GradientOptimize( MetricType * metric,
                    InterpolatorType * interpolator )
{
  if( this->GetReportProgress() )
    {
    std::cout << "BSpline GRADIENT START" << std::endl;
    }

  typedef ImageRegistrationMethod< TImage, TImage > RegType;


  int numberOfParameters = this->GetTransform()->GetNumberOfParameters();

  std::cout << "NoP = " << numberOfParameters << std::endl;

  /* FRPR */
  typedef FRPROptimizer         GradOptimizerType;
  GradOptimizerType::Pointer gradOpt;
  gradOpt = GradOptimizerType::New();
  gradOpt->SetMaximize( false );
  gradOpt->SetCatchGetValueException( true );
  gradOpt->SetMetricWorstPossibleValue( 0 );
  gradOpt->SetStepLength( 0.25 );
  gradOpt->SetStepTolerance( this->GetTargetError() );
  gradOpt->SetMaximumIteration( this->GetMaxIterations() );
  gradOpt->SetMaximumLineIteration( this->GetMaxIterations() );
  gradOpt->SetScales( this->GetTransformParametersScales() );
  gradOpt->SetUseUnitLengthGradient(true);
  gradOpt->SetToFletchReeves();

  /* GradientDescent 
  typedef GradientDescentOptimizer         GradOptimizerType;
  GradOptimizerType::Pointer gradOpt;
  gradOpt = GradOptimizerType::New();
  gradOpt->SetLearningRate( 0.25 );
  gradOpt->SetMaximize( false );
  gradOpt->SetNumberOfIterations( this->GetMaxIterations() );
  */

  /* LBFGSB
  typedef LBFGSBOptimizer                  GradOptimizerType;
  GradOptimizerType::Pointer gradOpt;
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
  tmpOpt->SetCostFunctionConvergenceFactor( 1000.0 );
  tmpOpt->SetProjectedGradientTolerance( 1e-50 );
  tmpOpt->SetMaximumNumberOfIterations( this->GetMaxIterations() );
  tmpOpt->SetMaximumNumberOfEvaluations( this->GetMaxIterations()*this->GetMaxIterations() );
  tmpOpt->SetMaximumNumberOfCorrections( this->GetMaxIterations()*this->GetMaxIterations() );
  */

  if( this->GetReportProgress() )
    {
    typedef BSplineImageRegistrationViewer ViewerCommandType;
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
  //reg->GetTransform()->SetFixedParameters( 
                         //this->GetInitialTransformFixedParameters() );
  reg->SetInitialTransformParameters( 
                         this->GetInitialTransformParameters() );
  reg->GetTransform()->SetParametersByValue( 
                         this->GetInitialTransformParameters() );
  reg->SetMetric( metric );
  reg->SetOptimizer( gradOpt );
  reg->SetInterpolator( interpolator );

  // reg->GetTransform()->Print( std::cout );
  // std::cout << reg->GetTransform()->GetParameters() << std::endl;
  // std::cout << reg->GetTransform()->GetFixedParameters() << std::endl;

  typename TransformType::OutputPointType p;
  p.Fill(100);
  p = reg->GetTransform()->TransformPoint(p);
  std::cout << "Initial Point = " << p << std::endl;

  std::cout << "  start registration" << std::endl;
  reg->StartRegistration();
  std::cout << "  stop registration" << std::endl;
  p.Fill(100);
  p = reg->GetTransform()->TransformPoint(p);
  std::cout << "Final Point = " << p << std::endl;

  this->SetFinalMetricValue( reg->GetOptimizer()
                                ->GetValue( reg->GetLastTransformParameters() ) );

  this->SetLastTransformParameters( reg->GetLastTransformParameters() );
  this->GetTransform()->SetParametersByValue( 
                                     this->GetLastTransformParameters() );

  if( this->GetReportProgress() )
    {
    std::cout << "BSpline GRADIENT END" << std::endl;
    }
}

template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::MultiResolutionOptimize( MetricType * metric,
                           InterpolatorType * interpolator )
{
  if( this->GetReportProgress() )
    {
    std::cout << "BSpline MULTIRESOLUTION START" << std::endl;
    }

  typedef RecursiveMultiResolutionPyramidImageFilter< ImageType,
                                                      ImageType > 
                                                        PyramidType;
  typename PyramidType::Pointer fixedPyramid = PyramidType::New();
  typename PyramidType::Pointer movingPyramid = PyramidType::New();

  typename TransformType::ParametersType nextParameters = 
                                 this->GetInitialTransformParameters();

  typedef BSplineImageToImageRegistrationMethod< ImageType > RegType;
  typename RegType::Pointer thisBSpline = dynamic_cast<RegType *>(this);

  unsigned int numberOfLevels = 4;
  double controlPointFactor = 2;
  double deformationMagFactor = 2;
  double sampleFactor = (double)pow((double)(controlPointFactor), (double)ImageDimension);
  unsigned int nextNumberOfSamples = this->GetNumberOfSamples();
  unsigned int nextNumberOfControlPoints = 
                                 thisBSpline->GetNumberOfControlPoints();
  double nextDeformationMag = 10;
  double nextScale = 1;
  if(numberOfLevels>1)
    {
    for(unsigned int level=1; level<numberOfLevels; level++)
      {
      nextNumberOfSamples = (int)( nextNumberOfSamples 
                                   / sampleFactor );
      nextNumberOfControlPoints = (int)( nextNumberOfControlPoints 
                                         / controlPointFactor );
      nextDeformationMag /= deformationMagFactor;
      nextScale *= controlPointFactor;
      }
    }
  if(nextNumberOfControlPoints < 3)
    {
    nextNumberOfControlPoints = 3;
    }

  fixedPyramid->SetNumberOfLevels( numberOfLevels );
  movingPyramid->SetNumberOfLevels( numberOfLevels );

  typename ImageType::SpacingType fixedSpacing =
                                      this->GetFixedImage()->GetSpacing();
  typename ImageType::SpacingType movingSpacing =
                                      this->GetFixedImage()->GetSpacing();

  typename PyramidType::ScheduleType fixedSchedule =
                                             fixedPyramid->GetSchedule();
  typename PyramidType::ScheduleType movingSchedule =
                                             movingPyramid->GetSchedule();
  for(unsigned int level=0; level<numberOfLevels; level++)
    {
    if(level == 0)
      {
      for(unsigned int i=0; i<ImageDimension; i++)
        {
        fixedSchedule[0][i] = (unsigned int)(nextScale 
                                    * fixedSpacing[0]/fixedSpacing[i]);
        if(fixedSchedule[0][i]<1)
          {
          fixedSchedule[0][i] = 1;
          }
        movingSchedule[0][i] = (unsigned int)(nextScale 
                                    * fixedSpacing[0]/movingSpacing[i]);
        if(movingSchedule[0][i]<1)
          {
          movingSchedule[0][i] = 1;
          }
        }
      }
    else
      {
      for(unsigned int i=0; i<ImageDimension; i++)
        {
        fixedSchedule[level][i] = (int)(fixedSchedule[level-1][i] 
                                        / controlPointFactor);
        if(fixedSchedule[level][i]<1)
          {
          fixedSchedule[level][i] = 1;
          }
        movingSchedule[level][i] = (int)(movingSchedule[level-1][i] 
                                         / controlPointFactor);
        if(movingSchedule[level][i]<1)
          {
          movingSchedule[level][i] = 1;
          }
        }
      }
    }

  fixedPyramid->SetSchedule( fixedSchedule );
  fixedPyramid->SetInput( this->GetFixedImage() );
  if( this->GetReportProgress() )
    {
    std::cout << "Generating fixed image pyramid" << std::endl;
    }
  fixedPyramid->Update();

  movingPyramid->SetSchedule( movingSchedule );
  movingPyramid->SetInput( this->GetMovingImage() );
  if( this->GetReportProgress() )
    {
    std::cout << "Generating moving image pyramid" << std::endl;
    }
  movingPyramid->Update();

  if( this->GetReportProgress() )
    {
    std::cout << "Number of levels = " << numberOfLevels << std::endl;
    std::cout << "Fixed schedule = " << std::endl << fixedSchedule;
    std::cout << "Fixed spacing 0 = " 
              << fixedPyramid->GetOutput(0)->GetSpacing() << std::endl;
    std::cout << "Moving schedule = " << std::endl << movingSchedule;
    std::cout << "Moving spacing 0 = " 
              << movingPyramid->GetOutput(0)->GetSpacing() << std::endl;
    }

  for(unsigned int level=0; level<numberOfLevels; level++)
    {
    if( this->GetReportProgress() )
      {
      std::cout << "MULTIRESOLUTION LEVEL = " << level << std::endl;
      std::cout << "   Number of samples = " << nextNumberOfSamples
                      << std::endl;
      std::cout << "   Number of control points = " 
                << nextNumberOfControlPoints << std::endl;
      std::cout << "   Deformation magnitude = " 
                << nextDeformationMag << std::endl;
      std::cout << "   Fixed image = " 
                << fixedPyramid->GetOutput(level)
                               ->GetLargestPossibleRegion().GetSize() 
                << std::endl;
      std::cout << "   Moving image = " 
                << movingPyramid->GetOutput(level)
                                ->GetLargestPossibleRegion().GetSize() 
                << std::endl;
      }

    typename RegType::Pointer reg = RegType::New();
    typename ImageType::ConstPointer fixedImage =
                                          fixedPyramid->GetOutput(level);
    typename ImageType::ConstPointer movingImage =
                                          movingPyramid->GetOutput(level);

    typedef itk::ImageFileWriter< ImageType > FileWriterType;
    typename FileWriterType::Pointer writer = FileWriterType::New();
    std::stringstream ss;
    std::string name;

    if( this->GetReportProgress() )
      {
      writer->SetInput( fixedImage );
      ss << "level" << level << "Fixed.mha";
      ss >> name;
      writer->SetFileName( name );
      writer->Update();
 
      writer->SetInput( movingImage );
      ss.clear();
      ss << "level" << level << "Moving.mha";
      ss >> name;
      writer->SetFileName( name );
      writer->Update();
      }

    reg->SetReportProgress( this->GetReportProgress() );
    reg->SetFixedImage( fixedImage );
    reg->SetMovingImage( movingImage );
    reg->SetNumberOfControlPoints( nextNumberOfControlPoints );
    reg->SetNumberOfSamples( nextNumberOfSamples );
    reg->SetExpectedDeformationMagnitude( nextDeformationMag );
    reg->SetGradientOptimizeOnly( true );
    reg->SetTargetError( this->GetTargetError() );
    if( level > numberOfLevels/2 )
      {
      reg->SetMinimizeMemory( this->GetMinimizeMemory() );
      }
    reg->SetUseOverlapAsROI( this->GetUseOverlapAsROI() );
    reg->SetMaxIterations( this->GetMaxIterations() );
    reg->SetFixedImageSamplesIntensityThreshold(
                     this->GetFixedImageSamplesIntensityThreshold() );
    reg->SetMetricMethodEnum( this->GetMetricMethodEnum() );
    reg->SetInterpolationMethodEnum( this->GetInterpolationMethodEnum() );

    if( level > 0 )
      {
      reg->SetInitialTransformParameters( nextParameters );
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
      std::cout << "Uncaught exception during helper class registration." 
                << std::endl;
      }

    if( this->GetReportProgress() )
      {
      typedef itk::ResampleImageFilter< ImageType, ImageType > ResamplerType;
      typename ResamplerType::Pointer resampler = ResamplerType::New();
      resampler->SetInput( movingImage );
      resampler->SetOutputParametersFromConstImage( fixedImage );
      reg->GetTransform()->SetParametersByValue( reg->GetLastTransformParameters() );
      resampler->SetTransform( reg->GetTransform() );
      try
        {
        resampler->Update();
        }
      catch( itk::ExceptionObject & excep )
        {
        std::cout << "Exception during resmpling for logs:" << std::endl;
        std::cout << excep << std::endl;
        break;
        }
      catch( ... )
        {
        std::cout << "Exception during resmpling for logs:" << std::endl;
        break;
        }
      writer->SetInput( resampler->GetOutput() );
      ss.clear();
      ss << "level" << level << "ResampledMoving.mha";
      ss >> name;
      writer->SetFileName( name );
      writer->Update();
      }

    if( level == numberOfLevels-1 )
      {
      this->SetFinalMetricValue( reg->GetFinalMetricValue() );
      this->SetLastTransformParameters( 
                                     reg->GetLastTransformParameters() );
      this->GetTransform()->SetParametersByValue( 
                                     this->GetLastTransformParameters() );
      }
    else
      {
      nextNumberOfSamples = (int)( nextNumberOfSamples
                                     * sampleFactor );
      nextNumberOfControlPoints = (int)( nextNumberOfControlPoints 
                                           * controlPointFactor );
      nextDeformationMag *= deformationMagFactor;
      if(nextNumberOfControlPoints 
         > thisBSpline->GetNumberOfControlPoints())
        {
        nextNumberOfControlPoints = 
                             thisBSpline->GetNumberOfControlPoints();
        }
      if( level == numberOfLevels-2)
        {
        nextNumberOfSamples = this->GetNumberOfSamples();
        nextNumberOfControlPoints =
                             thisBSpline->GetNumberOfControlPoints();
        }
      if( this->GetReportProgress() )
        {
        std::cout << "   Resampling grid..." << std::endl;
        }
      if(nextNumberOfControlPoints != reg->GetNumberOfControlPoints() )
        {
        reg->ResampleControlGrid( nextNumberOfControlPoints,
                                  nextParameters );
        }
      else
        {
        nextParameters = reg->GetLastTransformParameters();
        }
      if( this->GetReportProgress() )
        {
        std::cout << "   Level done." << std::endl;
        }
      }
    }

  if( this->GetReportProgress() )
    {
    std::cout << "BSpline MULTIRESOLUTION END" << std::endl;
    }
}


template< class TImage >
typename BSplineImageToImageRegistrationMethod< TImage >::TransformType *
BSplineImageToImageRegistrationMethod< TImage >
::GetTypedTransform( void )
{
  return static_cast< TransformType  * >( Superclass::GetTransform() );
}

template< class TImage >
const typename BSplineImageToImageRegistrationMethod< TImage >::TransformType *
BSplineImageToImageRegistrationMethod< TImage >
::GetTypedTransform( void ) const
{
  return static_cast< TransformType  * >( Superclass::GetTransform() );
}


template< class TImage >
typename BSplineImageToImageRegistrationMethod< TImage >::TransformType *
BSplineImageToImageRegistrationMethod< TImage >
::GetBSplineTransform( void )
{
  typename BSplineTransformType::Pointer trans = BSplineTransformType::New();
  
  trans->SetFixedParameters( this->GetTypedTransform()->GetFixedParameters() );
  trans->SetParametersByValue( this->GetTypedTransform()->GetParameters() );

  return trans;
}

template< class TImage >
const typename BSplineImageToImageRegistrationMethod< TImage >::TransformType *
BSplineImageToImageRegistrationMethod< TImage >
::GetBSplineTransform( void ) const
{
  typename BSplineTransformType::Pointer trans = BSplineTransformType::New();
  
  trans->SetFixedParameters( this->GetTypedTransform()->GetFixedParameters() );
  trans->SetParametersByValue( this->GetTypedTransform()->GetParameters() );

  return trans;
}


template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::ResampleControlGrid(int numberOfControlPoints,
                      ParametersType & parameters )
{
  typename TransformType::RegionType::SizeType gridSize;
  typename TransformType::SpacingType  gridSpacing;
  typename TransformType::OriginType  gridOrigin;
  typename TransformType::DirectionType  gridDirection;

  this->ComputeGridRegion( numberOfControlPoints,
                           gridSize, gridSpacing, gridOrigin,
                           gridDirection);

  int numberOfParameters = gridSize[0];
  for(unsigned int i=1; i<ImageDimension; i++)
    {
    numberOfParameters *= gridSize[i];
    }
  numberOfParameters *= ImageDimension;

  parameters.SetSize( numberOfParameters );
    
  int parameterCounter = 0;

  typedef typename BSplineTransformType::ImageType  ParametersImageType;
  typedef ResampleImageFilter< ParametersImageType, ParametersImageType> 
                                                      ResamplerType;
  typedef BSplineResampleImageFunction< ParametersImageType, double > 
                                                      FunctionType;
  typedef IdentityTransform< double, ImageDimension > IdentityTransformType;
  typedef itk::ImageFileWriter< ParametersImageType > WriterType;

  for( unsigned int k = 0; k < ImageDimension; k++ )
    {
    typename ResamplerType::Pointer upsampler = ResamplerType::New();
 
    typename FunctionType::Pointer function = FunctionType::New();
    function->SetSplineOrder(3);
 
    typename IdentityTransformType::Pointer identity = 
                                               IdentityTransformType::New();

    if( this->GetReportProgress() )
      {
      typename WriterType::Pointer writer = WriterType::New();
      std::stringstream ss;
      std::string name;
      ss << "inCoeImage" << k << ".mha";
      ss >> name;
      writer->SetInput( this->GetTypedTransform()->GetCoefficientImage()[k] );
      writer->SetFileName( name );
      try
        {
        writer->Update();
        }
      catch( ... )
        {
        std::cout << "Error writing coefficient image.  Ignoring." << std::endl;
        }
      }
 
    upsampler->SetInput( this->GetTypedTransform()
                             ->GetCoefficientImage()[k] );
    upsampler->SetInterpolator( function );
    upsampler->SetTransform( identity );
    upsampler->SetSize( gridSize );
    upsampler->SetOutputSpacing( gridSpacing );
    upsampler->SetOutputOrigin( gridOrigin );
    upsampler->SetOutputDirection( gridDirection );
    try
      {
      upsampler->Update();
      }
    catch( itk::ExceptionObject & excep )
      {
      std::cout << "Exception in upsampler: " << excep << std::endl;
      }
    catch( ... )
      {
      std::cout << "Uncaught exception in upsampler" << std::endl;
      }
 
    typedef BSplineDecompositionImageFilter< ParametersImageType,
                                             ParametersImageType >
                                                 DecompositionType;
    typename DecompositionType::Pointer decomposition =
                                          DecompositionType::New();
 
    decomposition->SetSplineOrder( 3 );
    decomposition->SetInput( upsampler->GetOutput() );
    try
      {
      decomposition->Update();
      }
    catch( itk::ExceptionObject & excep )
      {
      std::cout << "Exception in decomposition: " << excep << std::endl;
      }
    catch( ... )
      {
      std::cout << "Uncaught exception in decomposition" << std::endl;
      }
 
    typename ParametersImageType::Pointer newCoefficients =
                                        decomposition->GetOutput();
 
    if( this->GetReportProgress() )
      {
      typename WriterType::Pointer writer = WriterType::New();
      std::stringstream ss;
      std::string name;
      ss << "outCoeImage" << k << ".mha";
      ss >> name;
      writer->SetInput( newCoefficients );
      writer->SetFileName( name );
      try
        {
        writer->Update();
        }
      catch( ... )
        {
        std::cout << "Error while writing coefficient image.  Ignoring." << std::endl;
        }
      }

    // copy the coefficients into the parameter array
    typedef ImageRegionIterator<ParametersImageType> Iterator;
    Iterator it( newCoefficients, 
                 newCoefficients->GetLargestPossibleRegion() );
    while ( !it.IsAtEnd() )
      {
      parameters[ parameterCounter++ ] = it.Get();
      ++it;
      }
    }
}


template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}

};

#endif 
