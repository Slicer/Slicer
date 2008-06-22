/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "BSplineDeformableRegistrationCLP.h"


#include "itkCommand.h"

#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkBSplineDeformableTransform.h"
#include "itkLBFGSBOptimizer.h"

#include "itkOrientedImage.h"
#include "itkOrientImageFilter.h"
#include "itkResampleImageFilter.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"

#include "itkPluginUtilities.h"

#include "itkTimeProbesCollectorBase.h"


class CommandIterationUpdate : public itk::Command 
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
  typedef itk::SingleValuedCostFunction CostFunctionType;
  void SetProcessInformation (ModuleProcessInformation * info)
    {
    m_ProcessInformation = info; 
    }
  void SetCostFunction(CostFunctionType* fn)
    {
      m_CostFunction = fn;
    }
protected:
  CommandIterationUpdate() {};
  ModuleProcessInformation *m_ProcessInformation;
  CostFunctionType::Pointer m_CostFunction;
public:
  typedef itk::LBFGSBOptimizer  OptimizerType;
  typedef OptimizerType   *OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
      Execute( (const itk::Object *)caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event)
    {
      OptimizerPointer optimizer = 
        dynamic_cast< OptimizerPointer >( const_cast<itk::Object *>(object) );
      if( !(itk::IterationEvent().CheckEvent( &event )) )
        {
        return;
        }
      
      if (m_ProcessInformation)
        {
        m_ProcessInformation->Progress = 
          static_cast<double>(optimizer->GetCurrentIteration()) /
           static_cast<double>(optimizer->GetMaximumNumberOfIterations());
        }
      else
        {
        std::cout << "Optimizer Iteration: "
                  << optimizer->GetCurrentIteration() << ", "
                  << " Metric: "
                  << m_CostFunction->GetValue(optimizer->GetCurrentPosition())
                  << std::endl;
//         std::cout << "<filter-comment>"
//                   << "Optimizer Iteration: "
//                   << optimizer->GetCurrentIteration() << ", "
//                   << " Metric: "
//                   << optimizer->GetValue()
//                   << "</filter-comment>"
//                   << std::endl;
//         std::cout << "<filter-progress>"
//                   << (static_cast<double>(optimizer->GetCurrentIteration()) /
//                       static_cast<double>(optimizer->GetMaximumNumberOfIterations()))
        
//                   << "</filter-progress>"
//                   << std::endl;
        std::cout << std::flush;
        }
    }
};

template<class T> int DoIt( int argc, char * argv[], T )
{
  PARSE_ARGS;

  // typedefs
  const    unsigned int  ImageDimension = 3;
  typedef  T  PixelType;
  typedef  T  OutputPixelType;
  typedef itk::OrientedImage< PixelType, ImageDimension >       InputImageType;
  typedef itk::OrientedImage< OutputPixelType, ImageDimension > OutputImageType;

  typedef itk::ImageFileReader< InputImageType > FixedImageReaderType;
  typedef itk::ImageFileReader< InputImageType > MovingImageReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  typedef itk::OrientImageFilter<InputImageType,InputImageType> OrientFilterType;
  typedef itk::ResampleImageFilter< 
                            InputImageType, 
                            OutputImageType >    ResampleFilterType;

  const unsigned int SpaceDimension = ImageDimension;
  const unsigned int SplineOrder = 3;
  typedef double CoordinateRepType;
  typedef itk::ContinuousIndex<CoordinateRepType, ImageDimension> ContinuousIndexType;

  typedef itk::BSplineDeformableTransform<
                            CoordinateRepType,
                            SpaceDimension,
                              SplineOrder >     TransformType;
  typedef itk::AffineTransform<CoordinateRepType> AffineTransformType;
  typedef itk::LBFGSBOptimizer       OptimizerType;
  typedef itk::MattesMutualInformationImageToImageMetric< 
                                    InputImageType, 
                                    InputImageType >    MetricType;
  typedef itk:: LinearInterpolateImageFunction< 
                                    InputImageType,
                                    double          >    InterpolatorType;
  typedef itk::ImageRegistrationMethod< 
                                    InputImageType, 
                                    OutputImageType >    RegistrationType;

  typename MetricType::Pointer         metric        = MetricType::New();
  typename OptimizerType::Pointer      optimizer     = OptimizerType::New();
  typename InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
  typename TransformType::Pointer      transform     = TransformType::New();
  typename RegistrationType::Pointer   registration  = RegistrationType::New();

  typedef TransformType::RegionType RegionType;
  typedef TransformType::SpacingType SpacingType;
  typedef TransformType::OriginType OriginType;
  typedef TransformType::ParametersType     ParametersType;  


  // Read fixed and moving images
  //
  //
  typename FixedImageReaderType::Pointer  fixedImageReader  = FixedImageReaderType::New();
  typename MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

  fixedImageReader->SetFileName(  FixedImageFileName.c_str() );
  movingImageReader->SetFileName( MovingImageFileName.c_str() );

  // If an initial transform was specified, read it
  //
  //
  typedef itk::TransformFileReader TransformReaderType;
  TransformReaderType::Pointer initialTransform;

  if (InitialTransform != "")
    {
    initialTransform= TransformReaderType::New();
    initialTransform->SetFileName( InitialTransform );
    try
      {
      initialTransform->Update();
      }
    catch (itk::ExceptionObject &err)
      {
      std::cerr << err << std::endl;
      return  EXIT_FAILURE;
      }
    }

  
  // Reorient to axials to avoid issues with registration metrics not
  // transforming image gradients with the image orientation in
  // calculating the derivative of metric wrt transformation
  // parameters.
  //
  // Forcing image to be axials avoids this problem. Note, that
  // reorientation only affects the internal mapping from index to
  // physical coordinates.  The reoriented data spans the same
  // physical space as the original data.  Thus, the registration
  // transform calculated on the reoriented data is also the
  // transform forthe original un-reoriented data. 
  //
  typename OrientFilterType::Pointer fixedOrient = OrientFilterType::New();
  typename OrientFilterType::Pointer movingOrient = OrientFilterType::New();

  fixedOrient->UseImageDirectionOn();
  fixedOrient->SetDesiredCoordinateOrientationToAxial();
  fixedOrient->SetInput (fixedImageReader->GetOutput());

  movingOrient->UseImageDirectionOn();
  movingOrient->SetDesiredCoordinateOrientationToAxial();
  movingOrient->SetInput (movingImageReader->GetOutput());

  // Add a time probe
  itk::TimeProbesCollectorBase collector;

  collector.Start( "Read fixed volume" );
//   itk::PluginFilterWatcher watchOrientFixed(fixedOrient,
//                                             "Orient Fixed Image",
//                                             CLPProcessInformation,
//                                             1.0/3.0, 0.0);
  fixedOrient->Update();
  collector.Stop( "Read fixed volume" );

  collector.Start( "Read moving volume" );
//   itk::PluginFilterWatcher watchOrientMoving(movingOrient,
//                                             "Orient Moving Image",
//                                              CLPProcessInformation,
//                                             1.0/3.0, 1.0/3.0);
  movingOrient->Update();
  collector.Stop( "Read moving volume" );


  // Setup BSpline deformation
  //
  //  Note that the B-spline computation requires a finite support
  //  region ( 1 grid node at the lower borders and 2 grid nodes at
  //  upper borders).
  RegionType bsplineRegion;
  typename RegionType::SizeType   gridSizeOnImage;
  typename RegionType::SizeType   gridBorderSize;
  typename RegionType::SizeType   totalGridSize;

  gridSizeOnImage.Fill( gridSize );
  gridBorderSize.Fill( 3 );    // Border for spline order = 3 ( 1 lower, 2 upper )
  totalGridSize = gridSizeOnImage + gridBorderSize;

  bsplineRegion.SetSize( totalGridSize );

  SpacingType spacing = fixedOrient->GetOutput()->GetSpacing();
  OriginType origin = fixedOrient->GetOutput()->GetOrigin();;

  typename InputImageType::RegionType fixedRegion =
    fixedOrient->GetOutput()->GetLargestPossibleRegion();
  typename InputImageType::SizeType fixedImageSize =
    fixedRegion.GetSize();

  for(unsigned int r=0; r<ImageDimension; r++)
    {
    spacing[r] *= floor( static_cast<double>(fixedImageSize[r] - 1)  / 
                  static_cast<double>(gridSizeOnImage[r] - 1) );
    origin[r]  -=  spacing[r]; 
    }

  transform->SetGridSpacing ( spacing );
  transform->SetGridOrigin  ( origin );
  transform->SetGridRegion  ( bsplineRegion );

  const unsigned int numberOfParameters =
               transform->GetNumberOfParameters();
  
  ParametersType parameters( numberOfParameters );
  parameters.Fill( 0.0 );

  transform->SetParameters  ( parameters );

  // Initialize the transform with a bulk transform using either a
  // transform that aligns the centers of the volumes or a specified
  // bulk transform 
  //
  //
  typename TransformType::InputPointType centerFixed;
  typename InputImageType::RegionType::SizeType sizeFixed = fixedOrient->GetOutput()->GetLargestPossibleRegion().GetSize();
  // Find the center
  ContinuousIndexType indexFixed;
  for ( unsigned j = 0; j < 3; j++ )
    {
    indexFixed[j] = (sizeFixed[j]-1) / 2.0;
    }
  fixedOrient->GetOutput()->TransformContinuousIndexToPhysicalPoint ( indexFixed, centerFixed );

  typename TransformType::InputPointType centerMoving;
  typename InputImageType::RegionType::SizeType sizeMoving = movingOrient->GetOutput()->GetLargestPossibleRegion().GetSize();
  // Find the center
  ContinuousIndexType indexMoving;
  for ( unsigned j = 0; j < 3; j++ )
    {
    indexMoving[j] = (sizeMoving[j]-1) / 2.0;
    }
  movingOrient->GetOutput()->TransformContinuousIndexToPhysicalPoint ( indexMoving, centerMoving );

  typename AffineTransformType::Pointer centeringTransform;
  centeringTransform = AffineTransformType::New();

  centeringTransform->SetIdentity();
  centeringTransform->SetCenter( centerFixed );
  centeringTransform->Translate(centerMoving-centerFixed);
  std::cout << "Centering transform: "; centeringTransform->Print( std::cout );

  transform->SetBulkTransform( centeringTransform );

  // If an initial transformation was provided, then use it instead.
  //
  if (InitialTransform != ""
      && initialTransform->GetTransformList()->size() != 0)
    {
    TransformReaderType::TransformType::Pointer initial
      = *(initialTransform->GetTransformList()->begin());

    TransformType::BulkTransformType::Pointer
      bulk = dynamic_cast<TransformType::BulkTransformType*>(initial.GetPointer());

    if (bulk)
      {
      transform->SetBulkTransform( bulk );
      }
    else
      {
      std::cout << "Initial transform is an unsupported type." << std::endl;
      }
    
    std::cout << "Initial transform: "; initial->Print ( std::cout );
    }

  
  // Setup optimizer
  //
  //
  typename OptimizerType::BoundSelectionType boundSelect( transform->GetNumberOfParameters() );
  typename OptimizerType::BoundValueType upperBound( transform->GetNumberOfParameters() );
  typename OptimizerType::BoundValueType lowerBound( transform->GetNumberOfParameters() );
  if (ConstrainDeformation)
    {
    boundSelect.Fill( 2 );
    upperBound.Fill(  MaximumDeformation );
    lowerBound.Fill( -MaximumDeformation );
    }
  else
    {
    boundSelect.Fill( 0 );
    upperBound.Fill( 0.0 );
    lowerBound.Fill( 0.0 );
    }

  optimizer->SetBoundSelection( boundSelect );
  optimizer->SetUpperBound    ( upperBound );
  optimizer->SetLowerBound    ( lowerBound );

  optimizer->SetCostFunctionConvergenceFactor ( 1e+1 );
  optimizer->SetProjectedGradientTolerance    ( 1e-7 );
  optimizer->SetMaximumNumberOfIterations     ( Iterations );
  optimizer->SetMaximumNumberOfEvaluations    ( 500 );
  optimizer->SetMaximumNumberOfCorrections    ( 12 );

  
  // Create the Command observer and register it with the optimizer.
  //
  typename CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  observer->SetProcessInformation (CLPProcessInformation);
  observer->SetCostFunction( metric );

  optimizer->AddObserver( itk::IterationEvent(), observer );

  // Setup metric
  //
  //
  metric->ReinitializeSeed( 76926294 );
  metric->SetNumberOfHistogramBins( HistogramBins );
  metric->SetNumberOfSpatialSamples( SpatialSamples );

  std::cout << std::endl << "Starting Registration" << std::endl;

  // Registration
  //
  //
  registration->SetFixedImage  ( fixedOrient->GetOutput()  );
  registration->SetMovingImage ( movingOrient->GetOutput() );
  registration->SetMetric      ( metric       );
  registration->SetOptimizer   ( optimizer    );
  registration->SetInterpolator( interpolator );
  registration->SetTransform   ( transform    );
  registration->SetInitialTransformParameters( transform->GetParameters() );

  try 
    { 
//     itk::PluginFilterWatcher watchRegistration(registration,
//                                                "Registering",
//                                                CLPProcessInformation,
//                                                1.0/3.0, 2.0/3.0);
    collector.Start( "Registration" );
    registration->Update();
    collector.Stop( "Registration" );
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    return EXIT_FAILURE;
    } 
  
  typename OptimizerType::ParametersType finalParameters = 
    registration->GetLastTransformParameters();
  std::cout << "Final parameters: " << finalParameters[50] << std::endl;
  transform->SetParameters      ( finalParameters );

  if (OutputTransform != "")
    {
    typedef itk::TransformFileWriter TransformWriterType;
    TransformWriterType::Pointer outputTransformWriter;

    outputTransformWriter= TransformWriterType::New();
    outputTransformWriter->SetFileName( OutputTransform );
    outputTransformWriter->SetInput( transform );
    outputTransformWriter->AddTransform( transform->GetBulkTransform() );
    try
      {
      outputTransformWriter->Update();
      }
    catch (itk::ExceptionObject &err)
      {
      std::cerr << err << std::endl;
      exit( EXIT_FAILURE );
      }
    }

  // Resample to the original coordinate frame (not the reoriented
  // axial coordinate frame) of the fixed image
  //
  if (ResampledImageFileName != "")
    {
    typename ResampleFilterType::Pointer resample = ResampleFilterType::New();
    
    itk::PluginFilterWatcher watcher(
      resample,
      "Resample",
      CLPProcessInformation,
      1.0/3.0, 2.0/3.0);
    
    resample->SetTransform        ( transform );
    resample->SetInput            ( movingImageReader->GetOutput() );
    resample->SetDefaultPixelValue( DefaultPixelValue );
    resample->SetOutputParametersFromImage ( fixedImageReader->GetOutput() );
    
    collector.Start( "Resample" );
    resample->Update();
    collector.Stop( "Resample" );

    typename WriterType::Pointer      writer =  WriterType::New();
    writer->SetFileName( ResampledImageFileName.c_str() );
    writer->SetInput( resample->GetOutput()   );

    try
      {
      collector.Start( "Write resampled volume" );
      writer->Update();
      collector.Stop( "Write resampled volume" );
      }
    catch( itk::ExceptionObject & err ) 
      { 
      std::cerr << "ExceptionObject caught !" << std::endl; 
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
  
  // Print out the arguments (need to add --echo to the argument list 
  // 
  std::vector<char *> vargs;
  for (int vi=0; vi < argc; ++vi) vargs.push_back(argv[vi]);
  vargs.push_back("--echo");
  
  argc = vargs.size();
  argv = &(vargs[0]);

  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType (FixedImageFileName, pixelType, componentType);

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
