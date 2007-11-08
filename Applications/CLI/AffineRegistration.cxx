/*=========================================================================

  Program:   Registration stand-alone
  Module:    $RCSfile: $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

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

#include "itkPluginUtilities.h"

#include "itkTimeProbesCollectorBase.h"

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
  void SetProcessInformation (ModuleProcessInformation * info)
    {
    m_ProcessInformation = info; 
    }
protected:
  CommandIterationUpdate() {};
  ModuleProcessInformation *m_ProcessInformation;
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
      if (m_ProcessInformation)
        {
        m_ProcessInformation->Progress = 
          static_cast<double>(optimizer->GetCurrentIteration()) /
           static_cast<double>(optimizer->GetNumberOfIterations());
        }
      else
        {
        std::cout << "<filter-comment>"
                  << " \"" 
                  << "Optimizer Iteration: "
                  << optimizer->GetCurrentIteration()
                  << " Metric: "
                  << optimizer->GetValue()
                  << "\" "
                  << "</filter-comment>"
                  << std::endl;
        std::cout << "<filter-progress>"
                  << (static_cast<double>(optimizer->GetCurrentIteration()) /
                      static_cast<double>(optimizer->GetNumberOfIterations()))
        
                  << "</filter-progress>"
                  << std::endl;
        std::cout << std::flush;
        }
    }
};

template<class T> int DoIt( int argc, char * argv[], T )
{  
  //
  // Command line processing
  //
  PARSE_ARGS;

  const    unsigned int  ImageDimension = 3;
  typedef  T  PixelType;
  typedef itk::OrientedImage<PixelType, ImageDimension> ImageType;

  typedef itk::ImageFileReader<ImageType> FileReaderType;
  typedef itk::OrientImageFilter<ImageType,ImageType> OrientFilterType;
  typedef itk::MattesMutualInformationImageToImageMetric<ImageType, ImageType>
    MetricType; 
  typedef itk::RegularStepGradientDescentOptimizer
    OptimizerType;    
  typedef itk::LinearInterpolateImageFunction<ImageType, double>
    InterpolatorType;
  typedef itk::ImageRegistrationMethod<ImageType,ImageType>
    RegistrationType;
  typedef itk::AffineTransform<double> TransformType;
  typedef OptimizerType::ScalesType OptimizerScalesType;
  typedef itk::ResampleImageFilter<ImageType,ImageType> ResampleType;
  typedef itk::LinearInterpolateImageFunction<ImageType, double> ResampleInterpolatorType;
  typedef itk::ImageFileWriter<ImageType> WriterType;

  bool DoInitializeTransform = false;
  int RandomSeed = 1234567;

  // Add a time probe
  itk::TimeProbesCollectorBase collector;

  /////////////////////////////////////////////////////////////////////////////
  // Read the fixed and moving volumes
  //
  typename FileReaderType::Pointer fixedReader = FileReaderType::New();
    fixedReader->SetFileName ( fixedImageFileName.c_str() );

  try
    {
    collector.Start( "Read fixed volume" );
    fixedReader->Update();
    collector.Stop( "Read fixed volume" );
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "Error Reading Fixed image: " << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
    }

  typename FileReaderType::Pointer movingReader = FileReaderType::New();
    movingReader->SetFileName ( movingImageFileName.c_str() );

  try
    {
    collector.Start( "Read moving volume" );
    movingReader->Update();
    collector.Stop( "Read moving volume" );
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "Error Reading Moving image: " << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
    }

  /////////////////////////////////////////////////////////////////////////////
  // Reorient the volumes to a consistent acquisition direction
  //
  typename OrientFilterType::Pointer orientFixed = OrientFilterType::New();
  itk::PluginFilterWatcher watchOrientFixed(orientFixed,
                                            "Orient Fixed Image",
                                            CLPProcessInformation,
                                            1.0/3.0, 0.0);
    orientFixed->UseImageDirectionOn();
    orientFixed->SetDesiredCoordinateOrientationToAxial();
    orientFixed->SetInput (fixedReader->GetOutput());
    collector.Start( "Orient fixed volume" );
    orientFixed->Update();
    collector.Stop( "Orient fixed volume" );

  typename OrientFilterType::Pointer orientMoving = OrientFilterType::New();
  itk::PluginFilterWatcher watchOrientMoving(orientMoving,
                                            "Orient Moving Image",
                                             CLPProcessInformation,
                                            1.0/3.0, 1.0/3.0);
    orientMoving->UseImageDirectionOn();
    orientMoving->SetDesiredCoordinateOrientationToAxial();
    orientMoving->SetInput (movingReader->GetOutput());
    collector.Start( "Orient moving volume" );
    orientMoving->Update();
    collector.Stop( "Orient moving volume" );

  /////////////////////////////////////////////////////////////////////////////
  // Register the volumes
  //

  typename OptimizerType::Pointer      optimizer     = OptimizerType::New();
    optimizer->SetNumberOfIterations ( Iterations );
    optimizer->SetMinimumStepLength ( .0005 );
    optimizer->SetMaximumStepLength ( 10.0 );
    optimizer->SetMinimize(true);   

  typename TransformType::Pointer transform = TransformType::New();
  OptimizerScalesType scales( transform->GetNumberOfParameters() );
    scales.Fill ( 1.0 );
  for( unsigned j = 9; j < 12; j++ )
    {
    scales[j] = 1.0 / vnl_math_sqr(TranslationScale);
    }
    optimizer->SetScales( scales );

  // Create the Command observer and register it with the optimizer.
  //
  typename CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    observer->SetProcessInformation (CLPProcessInformation);

    optimizer->AddObserver( itk::IterationEvent(), observer );

  /////////////////////////////////////////////////////////////////////////////
  // Initialize the transform
  //
  typename TransformType::InputPointType centerFixed;
  typename ImageType::RegionType::SizeType sizeFixed = orientFixed->GetOutput()->GetLargestPossibleRegion().GetSize();
  // Find the center
  typename ImageType::IndexType indexFixed;
  for ( unsigned j = 0; j < 3; j++ )
    {
    indexFixed[j] = (long) ( (sizeFixed[j]-1) / 2.0 );
    }
  orientFixed->GetOutput()->TransformIndexToPhysicalPoint ( indexFixed, centerFixed );

  typename TransformType::InputPointType centerMoving;
  typename ImageType::RegionType::SizeType sizeMoving = orientMoving->GetOutput()->GetLargestPossibleRegion().GetSize();
  // Find the center
  typename ImageType::IndexType indexMoving;
  for ( unsigned j = 0; j < 3; j++ )
    {
    indexMoving[j] = (long) ( (sizeMoving[j]-1) / 2.0 );
    }
  orientMoving->GetOutput()->TransformIndexToPhysicalPoint ( indexMoving, centerMoving );

  transform->Translate(centerMoving-centerFixed);
  std::cout << "---------------" << centerMoving-centerFixed << std::endl;
  
  typename MetricType::Pointer  metric        = MetricType::New();
    metric->SetNumberOfHistogramBins ( HistogramBins );
    metric->SetNumberOfSpatialSamples( SpatialSamples );

  typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

  typename RegistrationType::Pointer registration = RegistrationType::New();
    registration->SetTransform ( transform );
    registration->SetInitialTransformParameters ( transform->GetParameters() );
    registration->SetMetric ( metric );
    registration->SetOptimizer ( optimizer );
    registration->SetInterpolator ( interpolator );
    registration->SetFixedImage ( orientFixed->GetOutput() );
    registration->SetMovingImage ( orientMoving->GetOutput() );

  try
    {
    collector.Start( "Register" );
    registration->StartRegistration();     
    collector.Stop( "Register" );
    } 
  catch( itk::ExceptionObject & err )
    {
    std::cout << err << std::endl;
    std::cerr << err << std::endl;
    exit ( EXIT_FAILURE );
    } 
  catch ( ... )
    {
    exit ( EXIT_FAILURE );
    }

  if (resampledImageFileName != "")
    {
    //////////////////////////////////////////////////////////////////////////
    // Resample using the registration results
    //
    typename ResampleType::Pointer resample = ResampleType::New();
    typename ResampleInterpolatorType::Pointer Interpolator = ResampleInterpolatorType::New();
    itk::PluginFilterWatcher watchResample(resample,
                                           "Resample",
                                           CLPProcessInformation,
                                           1.0/3.0, 2.0/3.0);
    
    transform->SetParameters ( registration->GetLastTransformParameters() );

    resample->SetInput ( orientMoving->GetOutput() ); 
    resample->SetTransform ( transform );
    resample->SetInterpolator ( Interpolator );
    resample->SetOutputOrigin ( orientFixed->GetOutput()->GetOrigin() );
    resample->SetOutputSpacing ( orientFixed->GetOutput()->GetSpacing() );
    resample->SetOutputDirection ( orientFixed->GetOutput()->GetDirection() );
    resample->SetSize ( orientFixed->GetOutput()->GetLargestPossibleRegion().GetSize() );
    collector.Start( "Resample" );
    resample->Update();
    collector.Stop( "Resample" );
    
    //////////////////////////////////////////////////////////////////////////
    // Write the registerded volume
    //
    typename WriterType::Pointer resampledWriter = WriterType::New();
    resampledWriter->SetFileName ( resampledImageFileName.c_str() );
    resampledWriter->SetInput ( resample->GetOutput() );
    try
      {
      collector.Start( "Write volume" );
      resampledWriter->Write();
      collector.Stop( "Write volume" );
      }
    catch( itk::ExceptionObject & err )
      { 
      std::cerr << err << std::endl;
      std::cerr << err << std::endl;
      return EXIT_FAILURE;
      }
    }
  
  // Report the time taken by the registration
  collector.Report();

  return EXIT_SUCCESS;
}

int main( int argc, char * argv[] )
{
  
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType (fixedImageFileName, pixelType, componentType);

    // This filter handles all types
    
    switch (componentType)
      {
      case itk::ImageIOBase::CHAR:
      case itk::ImageIOBase::UCHAR:
      case itk::ImageIOBase::USHORT:
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0));
        break;
      case itk::ImageIOBase::ULONG:
      case itk::ImageIOBase::LONG:
      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0));
        break;
      case itk::ImageIOBase::DOUBLE:
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0));
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
  
