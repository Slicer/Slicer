/*=========================================================================

  Program:   Registration stand-alone
  Module:    $RCSfile: $
  Language:  C++
  Date:      $Date: 2006-07-21 10:13:01 -0400 (Fri, 21 Jul 2006) $
  Version:   $Revision: 916 $

=========================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

#include "AffineRegistrationCLP.h"

// ITK Stuff
// Registration
#include "itkOrientedImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkOrientImageFilter.h"

#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImageFileWriter.h"
#include "itkImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkAffineTransform.h"
#include "itkResampleImageFilter.h"

//  The following section of code implements a Command observer
//  used to monitor the evolution of the registration process.
//
#include "itkCommand.h"
class CommandIterationUpdate : public itk::Command 
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
protected:
  CommandIterationUpdate() {};
public:
  typedef itk::RegularStepGradientDescentOptimizer  OptimizerType;
  typedef   const OptimizerType   *    OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
      Execute( (const itk::Object *)caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event)
    {
      OptimizerPointer optimizer = 
        dynamic_cast< OptimizerPointer >( object );
      if( !(itk::IterationEvent().CheckEvent( &event )) )
        {
        return;
        }
      std::cout << optimizer->GetCurrentIteration() << "   ";
      std::cout << optimizer->GetCurrentStepLength() << "   ";
      std::cout << optimizer->GetValue() << std::endl;
    }
};

const    unsigned int  ImageDimension = 3;
typedef  signed short  PixelType;
typedef itk::OrientedImage<PixelType, ImageDimension> ImageType;

int main ( int argc, char* argv[] ) 
{  
  PARSE_ARGS;

  bool DoInitializeTransform = false;
  int RandomSeed = 1234567;

  ImageType::Pointer fixed, moving;
  typedef itk::ImageFileReader<ImageType> FileReaderType;

  FileReaderType::Pointer fixedReader = FileReaderType::New();
  FileReaderType::Pointer movingReader = FileReaderType::New();
  fixedReader->SetFileName ( fixedImageFileName.c_str() );
  movingReader->SetFileName ( movingImageFileName.c_str() );

  try
    {
    fixedReader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "Error Reading Fixed image: " << std::endl;
    return EXIT_FAILURE;
    }

  // read in the moving image and do nothing
  try
    {
    movingReader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "Error Reading Moving image: " << std::endl;
    return EXIT_FAILURE;
    }

  fixed = fixedReader->GetOutput();
  moving = movingReader->GetOutput();


  typedef itk::OrientImageFilter<ImageType,ImageType> OrientFilterType;
  OrientFilterType::Pointer orientFixed = OrientFilterType::New();
  OrientFilterType::Pointer orientMoving = OrientFilterType::New();

  orientFixed->UseImageDirectionOn();
  orientFixed->SetDesiredCoordinateOrientationToAxial();
  orientFixed->SetInput (fixed);
  orientFixed->Update();

  orientMoving->UseImageDirectionOn();
  orientMoving->SetDesiredCoordinateOrientationToAxial();
  orientMoving->SetInput (moving);
  orientMoving->Update();

  typedef itk::MattesMutualInformationImageToImageMetric<ImageType, ImageType>
    MetricType; 
  typedef itk::RegularStepGradientDescentOptimizer
    OptimizerType;    
  typedef itk::LinearInterpolateImageFunction<ImageType, double>
    InterpolatorType;
  typedef itk::ImageRegistrationMethod<ImageType,ImageType>
    RegistrationType;
  typedef itk::AffineTransform<double>
    TransformType;

  MetricType::Pointer         metric        = MetricType::New();
  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
  RegistrationType::Pointer   registration  = RegistrationType::New();
  TransformType::Pointer      transform     = TransformType::New();

  registration->SetMetric ( metric );
  registration->SetOptimizer ( optimizer );
  registration->SetInterpolator ( interpolator );
  registration->SetTransform ( transform );
  registration->SetFixedImage ( orientFixed->GetOutput() );
  registration->SetMovingImage ( orientMoving->GetOutput() );

  optimizer->SetNumberOfIterations ( Iterations );
  optimizer->SetMinimumStepLength ( .0005 );
  optimizer->SetMaximumStepLength ( 10.0 );
  optimizer->SetMinimize(true);   

  typedef OptimizerType::ScalesType OptimizerScalesType;
  OptimizerScalesType scales( transform->GetNumberOfParameters() );
  scales.Fill ( 1.0 );
  for( unsigned j = 9; j < 12; j++ )
    {
    scales[j] = 1.0 / vnl_math_sqr(TranslationScale);
    }
  optimizer->SetScales( scales );

  // Create the Command observer and register it with the optimizer.
  //
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  // Initialize the transform
  TransformType::InputPointType centerFixed;
  ImageType::RegionType::SizeType sizeFixed = orientFixed->GetOutput()->GetLargestPossibleRegion().GetSize();
  // Find the center
  ImageType::IndexType indexFixed;
  for ( unsigned j = 0; j < 3; j++ )
    {
    indexFixed[j] = (long) ( (sizeFixed[j]-1) / 2.0 );
    }
  orientFixed->GetOutput()->TransformIndexToPhysicalPoint ( indexFixed, centerFixed );

  TransformType::InputPointType centerMoving;
  ImageType::RegionType::SizeType sizeMoving = orientMoving->GetOutput()->GetLargestPossibleRegion().GetSize();
  // Find the center
  ImageType::IndexType indexMoving;
  for ( unsigned j = 0; j < 3; j++ )
    {
    indexMoving[j] = (long) ( (sizeMoving[j]-1) / 2.0 );
    }
  orientMoving->GetOutput()->TransformIndexToPhysicalPoint ( indexMoving, centerMoving );

  transform->Translate(centerMoving-centerFixed);
  std::cout << "---------------" << centerMoving-centerFixed << std::endl;
  registration->SetInitialTransformParameters ( transform->GetParameters() );
  
  metric->SetNumberOfHistogramBins ( HistogramBins );
  metric->SetNumberOfSpatialSamples( SpatialSamples );

  try
    {
    registration->StartRegistration();     
    } 
  catch( itk::ExceptionObject & err )
    {
    std::cout << err << std::endl;
    exit ( EXIT_FAILURE );
    } 
  catch ( ... )
    {
    exit ( EXIT_FAILURE );
    }

  typedef itk::ResampleImageFilter<ImageType,ImageType> ResampleType;
  ResampleType::Pointer resample = ResampleType::New();

  typedef itk::LinearInterpolateImageFunction<ImageType, double> ResampleInterpolatorType;
  ResampleInterpolatorType::Pointer Interpolator = ResampleInterpolatorType::New();

  transform->SetParameters ( registration->GetLastTransformParameters() );

  resample->SetInput ( orientMoving->GetOutput() ); 
  resample->SetTransform ( transform );
  resample->SetInterpolator ( Interpolator );
  resample->SetOutputParametersFromImage ( orientFixed->GetOutput() );
  
  resample->Update();
  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer resampledWriter = WriterType::New();
  resampledWriter->SetFileName ( resampledImageFileName.c_str() );
  resampledWriter->SetInput ( resample->GetOutput() );
  try
    {
    resampledWriter->Write();
    }
  catch( itk::ExceptionObject & err )
    { 
    std::cerr << err << std::endl;
    exit ( EXIT_FAILURE );
    }
  
  exit ( EXIT_SUCCESS );
}
  
