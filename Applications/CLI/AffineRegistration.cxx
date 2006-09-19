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
#include "itkCommand.h"
#include "itkOrientedImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// ITK Stuff
// Registration
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImageFileWriter.h"
#include "itkImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkAffineTransform.h"
#include "itkResampleImageFilter.h"
#include "itkStdStreamLogOutput.h"

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

typedef itk::OrientedImage<signed short, 3> Volume;

int main ( int argc, char* argv[] ) 
{  
  itk::StdStreamLogOutput::Pointer coutput = itk::StdStreamLogOutput::New();
  coutput->SetStream(std::cout);

PARSE_ARGS;
  bool DoInitializeTransform = false;
  int RandomSeed = 1234567;

  std::cout << "Parsed arguments" << std::endl
            << "HistogramBins: " << HistogramBins << std::endl
            << "RandomSeed: " << RandomSeed << std::endl
            << "SpatialSamples: " << SpatialSamples << std::endl
            << "TranslationScale: " << TranslationScale << std::endl
            << "DoInitializeTransform: " << DoInitializeTransform << std::endl
            << "fixedImageFileName: " << fixedImageFileName << std::endl
            << "movingImageFileName: " << movingImageFileName << std::endl
            << "resampledImageFileName: " << resampledImageFileName << std::endl
            << std::endl;

  Volume::Pointer fixed, moving;
  typedef itk::ImageFileReader<Volume> FileReaderType;
  FileReaderType::Pointer FixedReader = FileReaderType::New();
  FileReaderType::Pointer MovingReader = FileReaderType::New();
  FixedReader->SetFileName ( fixedImageFileName.c_str() );
  MovingReader->SetFileName ( movingImageFileName.c_str() );

  try
    {
    FixedReader->Update();
    std::cout << "Fixed read!" << std::endl;
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "Error Reading Fixed image: " << std::endl;
    return EXIT_FAILURE;
    }

  // read in the moving image and do nothing
  try
    {
    MovingReader->Update();
    std::cout << "Moving read!" << std::endl;
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "Error Reading Moving image: " << std::endl;
    return EXIT_FAILURE;
    }

  fixed = FixedReader->GetOutput();
  moving = MovingReader->GetOutput();


  fixed->Print ( std::cout );
  moving->Print ( std::cout );


  typedef itk::MattesMutualInformationImageToImageMetric<Volume, Volume>
    MetricType; 
  typedef itk::RegularStepGradientDescentOptimizer
    OptimizerType;    
  typedef itk::LinearInterpolateImageFunction<Volume, double>
    InterpolatorType;
  typedef itk::ImageRegistrationMethod<Volume,Volume>
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
  registration->SetFixedImage ( fixed );
  registration->SetMovingImage ( moving );

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

  transform->SetIdentity();
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

  typedef itk::ResampleImageFilter<Volume,Volume> ResampleType;
  ResampleType::Pointer Resample = ResampleType::New();

  typedef itk::LinearInterpolateImageFunction<Volume, double> ResampleInterpolatorType;
  ResampleInterpolatorType::Pointer Interpolator = ResampleInterpolatorType::New();

  transform->SetParameters ( registration->GetLastTransformParameters() );

  fixed->Print ( std::cout );
  moving->Print ( std::cout );
  transform->Print ( std::cout );
  
  Resample->SetInput ( moving ); 
  Resample->SetTransform ( transform );
  Resample->SetInterpolator ( Interpolator );
  Resample->SetOutputParametersFromImage ( fixed );
  
  Resample->Update();
  typedef itk::ImageFileWriter<Volume> WriterType;
  WriterType::Pointer ResampledWriter = WriterType::New();
  ResampledWriter->SetFileName ( resampledImageFileName.c_str() );
  ResampledWriter->SetInput ( Resample->GetOutput() );
  try
    {
    ResampledWriter->Write();
    }
  catch( itk::ExceptionObject & err )
    { 
    std::cerr << err << std::endl;
    exit ( EXIT_FAILURE );
    }
  
  exit ( EXIT_SUCCESS );
}
  
