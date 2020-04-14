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

#ifndef itkBSplineImageToImageRegistrationMethod_txx
#define itkBSplineImageToImageRegistrationMethod_txx

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
public:
  typedef BSplineImageRegistrationViewer Self;
  typedef Command                        Superclass;
  typedef SmartPointer<Self>             Pointer;

  itkTypeMacro( BSplineImageRegistrationViewer, Command );

  itkNewMacro( BSplineImageRegistrationViewer );

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

  BSplineImageRegistrationViewer()
  {
    m_Clock = RealTimeClock::New();
    m_LastTime = m_Clock->GetTimeInSeconds();
    m_Iteration = 0;
    m_UpdateInterval = 1;
    m_DontShowParameters = false;
  }
  ~BSplineImageRegistrationViewer() override = default;

};

template <class TImage>
BSplineImageToImageRegistrationMethod<TImage>::BSplineImageToImageRegistrationMethod()
{
  m_NumberOfControlPoints = 10;
  m_NumberOfLevels = 4;
  m_ExpectedDeformationMagnitude = 10;
  m_GradientOptimizeOnly = false;
  this->SetTransformMethodEnum( Superclass::BSPLINE_TRANSFORM );

  // Override superclass defaults:
  this->SetMaxIterations( 40 );
  this->SetNumberOfSamples( 800000 );
  this->SetInterpolationMethodEnum( Superclass::BSPLINE_INTERPOLATION );
}

template <class TImage>
BSplineImageToImageRegistrationMethod<TImage>::~BSplineImageToImageRegistrationMethod() = default;

template <class TImage>
void
BSplineImageToImageRegistrationMethod<TImage>
::ComputeGridRegion( int numberOfControlPoints,
                     typename TransformType::RegionType::SizeType & gridSize,
                     typename TransformType::SpacingType & gridSpacing,
                     typename TransformType::OriginType & gridOrigin,
                     typename TransformType::DirectionType & gridDirection)
{
  if( numberOfControlPoints < 3 )
    {
    itkWarningMacro(<< "ComputeGridRegion: numberOfControlPoints=1; changing to 2.");
    numberOfControlPoints = 3;
    }

  typename TransformType::RegionType::SizeType gridSizeOnImage;
  typename TransformType::RegionType::SizeType gridBorderSize;

  typename TImage::SizeType fixedImageSize = this->GetFixedImage()->GetLargestPossibleRegion().GetSize();

  gridSpacing   = this->GetFixedImage()->GetSpacing();

  double scale = ( (fixedImageSize[0] - 1) * gridSpacing[0]) / (numberOfControlPoints - 1);
  gridSizeOnImage[0] = numberOfControlPoints;
  for( unsigned int i = 1; i < ImageDimension; i++ )
    {
    gridSizeOnImage[i] = (int)( ( (fixedImageSize[i] - 1) * gridSpacing[i]) / scale + 0.01 ) + 1;
    if( gridSizeOnImage[i] < 3 )
      {
      gridSizeOnImage[i] = 3;
      }
    }
  gridBorderSize.Fill( 3 );  // Border for spline order = 3 ( 1 lower, 2 upper )

  gridSize = gridSizeOnImage + gridBorderSize;

  gridOrigin    = this->GetFixedImage()->GetOrigin();
  gridDirection    = this->GetFixedImage()->GetDirection();
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    gridSpacing[i] *= static_cast<double>(fixedImageSize[i] - 1)
      / static_cast<double>(gridSizeOnImage[i] - 1);
    }

  typename TransformType::SpacingType gridOriginOffset;
  gridOriginOffset = gridDirection * gridSpacing;

  gridOrigin = gridOrigin - gridOriginOffset;

  std::cout << "   gridSize = " << gridSize << std::endl;
  std::cout << "   gridSpacing = " << gridSpacing << std::endl;
  std::cout << "   gridOrigin = " << gridOrigin << std::endl;
  std::cout << "   gridDirection = " << gridDirection << std::endl;
}

template <class TImage>
void BSplineImageToImageRegistrationMethod<TImage>::GenerateData()
{
  this->SetTransform( BSplineTransformType::New() );

  typename TransformType::RegionType gridRegion;
  typename TransformType::RegionType::SizeType gridSize;
  typename TransformType::SpacingType  gridSpacing;
  typename TransformType::OriginType  gridOrigin;
  typename TransformType::DirectionType  gridDirection;

  this->ComputeGridRegion( this->GetNumberOfControlPoints(),
                           gridSize, gridSpacing, gridOrigin,
                           gridDirection);

  gridRegion.SetSize( gridSize );

  this->GetTypedTransform()->SetGridRegion( gridRegion );
  this->GetTypedTransform()->SetGridSpacing( gridSpacing );
  this->GetTypedTransform()->SetGridOrigin( gridOrigin );
  this->GetTypedTransform()->SetGridDirection( gridDirection );

  const unsigned int numberOfParameters =
    this->GetTypedTransform()->GetNumberOfParameters();
  std::cout << "   numberOfParameters = " << numberOfParameters << std::endl;

  /**/
  /*   Remember the fixed parameters for this transform */
  /**/
  this->SetInitialTransformFixedParameters( this->GetTypedTransform()->GetFixedParameters() );

  /**/
  /*   Make sure Initial Transform Parameters are valid */
  /**/
  if( numberOfParameters != this->GetInitialTransformParameters().GetSize() )
    {
    typename Superclass::TransformParametersType params( numberOfParameters );
    params.Fill( 0.0 );
    this->SetInitialTransformParameters( params );
    }

  /**/
  /*  Set scales = expected amount of movement of a control point */
  /**/
  typename Superclass::TransformParametersType params( numberOfParameters );
  typename TransformType::SpacingType spacing   = this->GetFixedImage()->GetSpacing();
  double scale = 1.0 / (m_ExpectedDeformationMagnitude * spacing[0]);
  params.Fill( scale );
  this->SetTransformParametersScales( params );

  this->Superclass::GenerateData();
}

template <class TImage>
void
BSplineImageToImageRegistrationMethod<TImage>
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

template <class TImage>
void
BSplineImageToImageRegistrationMethod<TImage>
::GradientOptimize( MetricType * metric,
                    InterpolatorType * interpolator )
{
  if( this->GetReportProgress() )
    {
    std::cout << "BSpline GRADIENT START" << std::endl;
    }

  /* Setup FRPR - set params specific to this optimizer */
  typedef FRPROptimizer GradOptimizerType;
  GradOptimizerType::Pointer gradOpt;
  gradOpt = GradOptimizerType::New();
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

  /* GradientDescent
  typedef GradientDescentOptimizer         GradOptimizerType;
  GradOptimizerType::Pointer gradOpt;
  gradOpt = GradOptimizerType::New();
  gradOpt->SetLearningRate( 0.25 );
  gradOpt->SetMaximize( false );
  gradOpt->SetNumberOfIterations( this->GetMaxIterations() );
  */

  /* LBFGSB
  int numberOfParameters = this->GetTransform()->GetNumberOfParameters();
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

  /**/
  /*  Create observers for reporting progress */
  /**/
  if( this->GetReportProgress() )
    {
    typedef BSplineImageRegistrationViewer ViewerCommandType;
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

  /**/
  /* Setup a standard itk::ImageToImageRegistration method
   *  and plug-in optimizer, interpolator, etc. */
  /**/
  typedef ImageRegistrationMethod<TImage, TImage> ImageRegistrationType;
  typename ImageRegistrationType::Pointer reg = ImageRegistrationType::New();
  typename ImageType::ConstPointer fixedImage = this->GetFixedImage();
  typename ImageType::ConstPointer movingImage = this->GetMovingImage();
  reg->SetFixedImage( fixedImage );
  reg->SetMovingImage( movingImage );
  reg->SetFixedImageRegion( this->GetFixedImage()
                            ->GetLargestPossibleRegion() );
  reg->SetTransform( this->GetTransform() );
  reg->SetInitialTransformParameters(
    this->GetInitialTransformParameters() );
  reg->GetTransform()->SetParametersByValue(
    this->GetInitialTransformParameters() );
  reg->SetMetric( metric );
  reg->SetOptimizer( gradOpt );
  reg->SetInterpolator( interpolator );

  if( this->GetReportProgress() )
    {
    typename TransformType::OutputPointType p;
    p.Fill(100);
    p = reg->GetTransform()->TransformPoint(p);
    std::cout << "Initial Point = " << p << std::endl;
    }

  /**/
  /*  Optimize! */
  /**/
  bool failure = false;
  try
    {
    reg->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cout << "Exception caught during gradient registration of BSpline."
              << excep << std::endl;
    std::cout << "Continuing using best values..." << std::endl;
    std::cout << "  Pos = " << reg->GetLastTransformParameters()
              << std::endl << std::endl;
    if( reg->GetLastTransformParameters().size()
        != reg->GetInitialTransformParameters().size() )
      {
      std::cout << "  Invalid position, using initial parameters."
                << std::endl;
      reg->GetTransform()->SetParametersByValue( reg->GetInitialTransformParameters() );
      failure = true;
      }
    }
  catch( ... )
    {
    std::cerr << "Error in gradient optimization of BSpline." << std::endl;
    std::cout << "Continuing using best values..." << std::endl;
    std::cout << "  Pos = " << reg->GetLastTransformParameters()
              << std::endl << std::endl;
    if( reg->GetLastTransformParameters().size()
        != reg->GetInitialTransformParameters().size() )
      {
      std::cout << "  Invalid position, using initial parameters."
                << std::endl;
      reg->GetTransform()->SetParametersByValue( reg->GetInitialTransformParameters() );
      failure = true;
      }
    }

  if( this->GetReportProgress() )
    {
    typename TransformType::OutputPointType p;
    p.Fill(100);
    p = reg->GetTransform()->TransformPoint(p);
    std::cout << "Resulting Point = " << p << std::endl;
    }

  /**/
  /*  Record results */
  /**/
  if( failure )
    {
    this->SetFinalMetricValue( reg->GetOptimizer()
                               ->GetValue( reg->GetInitialTransformParameters() ) );

    this->SetLastTransformParameters( reg->GetInitialTransformParameters() );
    this->GetTransform()->SetParametersByValue(
      this->GetInitialTransformParameters() );
    }
  else
    {
    this->SetFinalMetricValue( reg->GetOptimizer()
                               ->GetValue( reg->GetLastTransformParameters() ) );

    this->SetLastTransformParameters( reg->GetLastTransformParameters() );
    this->GetTransform()->SetParametersByValue(
      this->GetLastTransformParameters() );
    }

  if( this->GetReportProgress() )
    {
    std::cout << "BSpline GRADIENT END" << std::endl;
    }
}

template <class TImage>
void
BSplineImageToImageRegistrationMethod<TImage>
::MultiResolutionOptimize( MetricType * itkNotUsed(metric),
                           InterpolatorType * itkNotUsed(interpolator) )
{
  if( this->GetReportProgress() )
    {
    std::cout << "BSpline MULTIRESOLUTION START" << std::endl;
    }

  typedef RecursiveMultiResolutionPyramidImageFilter<ImageType,
                                                     ImageType>
  PyramidType;
  typename PyramidType::Pointer fixedPyramid = PyramidType::New();
  typename PyramidType::Pointer movingPyramid = PyramidType::New();

  /**/
  /* Determine the control points, samples, and scales to be used at each level */
  /**/
  double       controlPointFactor = 2;
  unsigned int levelNumberOfControlPoints =
    this->GetNumberOfControlPoints();
  double levelScale = 1;
  if( this->m_NumberOfLevels > 1 )
    {
    for( unsigned int level = 1; level < this->m_NumberOfLevels; level++ )
      {
      levelNumberOfControlPoints = (unsigned int)(levelNumberOfControlPoints / controlPointFactor);
      levelScale *= controlPointFactor;
      }
    }
  if( levelNumberOfControlPoints < 3 )
    {
    levelNumberOfControlPoints = 3;
    }

  /**/
  /* Setup the multi-scale image pyramids */
  /**/
  fixedPyramid->SetNumberOfLevels( this->m_NumberOfLevels );
  movingPyramid->SetNumberOfLevels( this->m_NumberOfLevels );

  typename ImageType::SpacingType fixedSpacing =
    this->GetFixedImage()->GetSpacing();
  typename ImageType::SpacingType movingSpacing =
    this->GetFixedImage()->GetSpacing();

  typename PyramidType::ScheduleType fixedSchedule =
    fixedPyramid->GetSchedule();
  typename PyramidType::ScheduleType movingSchedule =
    movingPyramid->GetSchedule();

  /**/
  /*   First, determine the pyramid at level 0 */
  /**/
  unsigned int level = 0;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    fixedSchedule[0][i] = (unsigned int)(levelScale
                                         * fixedSpacing[0] / fixedSpacing[i]);
    if( fixedSchedule[0][i] < 1 )
      {
      fixedSchedule[0][i] = 1;
      }
    movingSchedule[0][i] = (unsigned int)(levelScale
                                          * fixedSpacing[0] / movingSpacing[i]);
    if( movingSchedule[0][i] < 1 )
      {
      movingSchedule[0][i] = 1;
      }
    }
  /**/
  /*   Second, determine the pyramid at the remaining levels */
  /**/
  for( level = 1; level < this->m_NumberOfLevels; level++ )
    {
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      fixedSchedule[level][i] = (int)(fixedSchedule[level - 1][i]
                                      / controlPointFactor);
      if( fixedSchedule[level][i] < 1 )
        {
        fixedSchedule[level][i] = 1;
        }
      movingSchedule[level][i] = (int)(movingSchedule[level - 1][i]
                                       / controlPointFactor);
      if( movingSchedule[level][i] < 1 )
        {
        movingSchedule[level][i] = 1;
        }
      }
    }

  /**/
  /*   Third, apply pyramid to fixed image */
  /**/
  fixedPyramid->SetSchedule( fixedSchedule );
  fixedPyramid->SetInput( this->GetFixedImage() );
  fixedPyramid->Update();

  /**/
  /*   Fourth, apply pyramid to moving image */
  /**/
  movingPyramid->SetSchedule( movingSchedule );
  movingPyramid->SetInput( this->GetMovingImage() );
  movingPyramid->Update();

  /**/
  /* Assign initial transform parameters at coarse level based on
   *   initial transform parameters - initial transform parameters are
   *   set in the GenerateData() method */
  /**/
  typename Superclass::TransformParametersType levelParameters;
  this->ResampleControlGrid( levelNumberOfControlPoints, levelParameters );
  /* Perform registration at each level */
  for( level = 0; level < this->m_NumberOfLevels; level++ )
    {
    if( this->GetReportProgress() )
      {
      std::cout << "MULTIRESOLUTION LEVEL = " << level << std::endl;
      std::cout << "   Number of control points = "
                << levelNumberOfControlPoints << std::endl;
      std::cout << "   Fixed image = "
                << fixedPyramid->GetOutput(level)
      ->GetLargestPossibleRegion().GetSize()
                << std::endl;
      std::cout << "   Moving image = "
                << movingPyramid->GetOutput(level)
      ->GetLargestPossibleRegion().GetSize()
                << std::endl;
      }

    /**/
    /* Get the fixed and moving images for this pyramid level */
    /**/
    typename ImageType::ConstPointer fixedImage =
      fixedPyramid->GetOutput(level);
    typename ImageType::ConstPointer movingImage =
      movingPyramid->GetOutput(level);

    /*
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
     */

    double levelFactor = levelNumberOfControlPoints / (double)(this->GetNumberOfControlPoints() );

    double levelDeformationMagnitude = this->GetExpectedDeformationMagnitude() / levelFactor;

    unsigned int levelNumberOfSamples = (unsigned int)(this->GetNumberOfSamples() / levelFactor);
    unsigned int fixedImageNumberOfSamples = fixedImage->GetLargestPossibleRegion().GetNumberOfPixels();
    if( levelNumberOfSamples > fixedImageNumberOfSamples )
      {
      levelNumberOfSamples = fixedImageNumberOfSamples;
      }

    if( this->GetReportProgress() )
      {
      std::cout << "   Deformation magnitude = " << levelDeformationMagnitude
                << std::endl;
      std::cout << "   Number of samples = " << levelNumberOfSamples
                << std::endl;
      }

    /**/
    /* Create a BSpline registration module (an instance of this class!) that
     *    will perform gradient optimization (instead of pyramid optimization). */
    /**/
    typedef BSplineImageToImageRegistrationMethod<ImageType> BSplineRegType;
    typename BSplineRegType::Pointer reg = BSplineRegType::New();
    reg->SetReportProgress( this->GetReportProgress() );
    reg->SetFixedImage( fixedImage );
    reg->SetMovingImage( movingImage );
    reg->SetNumberOfControlPoints( levelNumberOfControlPoints );
    reg->SetNumberOfSamples( levelNumberOfSamples );
    reg->SetExpectedDeformationMagnitude( levelDeformationMagnitude );
    reg->SetGradientOptimizeOnly( true );
    reg->SetTargetError( this->GetTargetError() );
    reg->SetSampleFromOverlap( this->GetSampleFromOverlap() );
    reg->SetFixedImageSamplesIntensityThreshold(
      this->GetFixedImageSamplesIntensityThreshold() );
    reg->SetUseFixedImageSamplesIntensityThreshold(
      this->GetUseFixedImageSamplesIntensityThreshold() );
    reg->SetMaxIterations( (unsigned int)(this->GetMaxIterations() / ( (level + 1) / 2.0) ) );
    reg->SetMetricMethodEnum( this->GetMetricMethodEnum() );
    reg->SetInterpolationMethodEnum( this->GetInterpolationMethodEnum() );
    reg->SetInitialTransformParameters( levelParameters );
    // For the last two levels (the ones at the highest resolution, use
    //   user-specified values of MinimizeMemory, otherwise do not
    //   minimizeMemory so as to maximize speed.
    if( level >= this->m_NumberOfLevels - 2 )
      {
      reg->SetMinimizeMemory( this->GetMinimizeMemory() );
      }
    else
      {
      reg->SetMinimizeMemory( false );
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
      reg->GetTransform()->Print(std::cout, 2);
      }
    catch( ... )
      {
      std::cout << "Uncaught exception during helper class registration."
                << std::endl;
      }

    /*
    if( this->GetReportProgress() )
      {
      typedef itk::ResampleImageFilter< ImageType, ImageType > ResamplerType;
      typename ResamplerType::Pointer resampler = ResamplerType::New();
      resampler->SetInput( movingImage );
      // We should not be casting away constness here, but SetOutputParametersFromImage
      // Does not change the image.  This is needed to workaround fixes to ITK
      typename ImageType::Pointer tmp = const_cast<ImageType*>(fixedImage.GetPointer());
      resampler->SetOutputParametersFromImage( tmp );
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
    */

    if( level < this->m_NumberOfLevels - 1 )
      {
      levelNumberOfControlPoints = (unsigned int)(levelNumberOfControlPoints * controlPointFactor);
      if( levelNumberOfControlPoints > this->GetNumberOfControlPoints() ||
          level == this->m_NumberOfLevels - 2 )
        {
        levelNumberOfControlPoints = this->GetNumberOfControlPoints();
        }

      if( levelNumberOfControlPoints != reg->GetNumberOfControlPoints() )
        {
        if( this->GetReportProgress() )
          {
          std::cout << "   Resampling grid..." << std::endl;
          }
        reg->ResampleControlGrid( levelNumberOfControlPoints,
                                  levelParameters );
        }
      else
        {
        levelParameters = reg->GetLastTransformParameters();
        }
      }
    else
      {
      /**/
      /*  Remember the results */
      /**/
      this->SetFinalMetricValue( reg->GetFinalMetricValue() );
      this->SetLastTransformParameters( reg->GetLastTransformParameters() );
      this->GetTransform()->SetParametersByValue( this->GetLastTransformParameters() );
      }

    if( this->GetReportProgress() )
      {
      std::cout << "   Level done." << std::endl;
      }
    }

  if( this->GetReportProgress() )
    {
    std::cout << "BSpline MULTIRESOLUTION END" << std::endl;
    }
}

template <class TImage>
typename BSplineImageToImageRegistrationMethod<TImage>::TransformType*
BSplineImageToImageRegistrationMethod<TImage>::GetTypedTransform()
{
  return static_cast<TransformType  *>( Superclass::GetTransform() );
}

template <class TImage>
const typename BSplineImageToImageRegistrationMethod<TImage>::TransformType*
BSplineImageToImageRegistrationMethod<TImage>::GetTypedTransform() const
{
  return static_cast<const TransformType  *>( Superclass::GetTransform() );
}

template <class TImage>
typename BSplineImageToImageRegistrationMethod<TImage>::BSplineTransformPointer
BSplineImageToImageRegistrationMethod<TImage>::GetBSplineTransform() const
{
  typename BSplineTransformType::Pointer trans = BSplineTransformType::New();

  trans->SetFixedParameters( this->GetTypedTransform()->GetFixedParameters() );
  trans->SetParametersByValue( this->GetTypedTransform()->GetParameters() );

  return trans;
}

template <class TImage>
void
BSplineImageToImageRegistrationMethod<TImage>
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
  for( unsigned int i = 1; i < ImageDimension; i++ )
    {
    numberOfParameters *= gridSize[i];
    }
  numberOfParameters *= ImageDimension;

  parameters.SetSize( numberOfParameters );

  int parameterCounter = 0;

  typedef typename BSplineTransformType::ImageType                      ParametersImageType;
  typedef ResampleImageFilter<ParametersImageType, ParametersImageType> ResamplerType;
  typedef BSplineResampleImageFunction<ParametersImageType, double>     FunctionType;
  typedef IdentityTransform<double, ImageDimension>                     IdentityTransformType;

  for( unsigned int k = 0; k < ImageDimension; k++ )
    {
    typename ResamplerType::Pointer upsampler = ResamplerType::New();

    typename FunctionType::Pointer function = FunctionType::New();
    function->SetSplineOrder(3);

    typename IdentityTransformType::Pointer identity =
      IdentityTransformType::New();

    /*
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
    */

    upsampler->SetInput( this->GetTypedTransform()
                         ->GetCoefficientImages()[k] );
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

    typedef BSplineDecompositionImageFilter<ParametersImageType,
                                            ParametersImageType>
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

    /*
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
      */

    // copy the coefficients into the parameter array
    typedef ImageRegionIterator<ParametersImageType> Iterator;
    Iterator it( newCoefficients,
                 newCoefficients->GetLargestPossibleRegion() );
    while( !it.IsAtEnd() )
      {
      parameters[parameterCounter++] = it.Get();
      ++it;
      }
    }
}

template <class TImage>
void
BSplineImageToImageRegistrationMethod<TImage>
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}

}

#endif
