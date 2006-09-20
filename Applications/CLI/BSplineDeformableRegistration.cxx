/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: DeformableRegistration8.cxx,v $
  Language:  C++
  Date:      $Date: 2005/11/19 16:31:50 $
  Version:   $Revision: 1.13 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif




// Software Guide : BeginLatex
//
// This example illustrates the use of the \doxygen{BSplineDeformableTransform}
// class for performing registration of two $3D$ images and for the case of
// multi-modality images. The image metric of choice in this case is the
// \doxygen{MattesMutualInformationImageToImageMetric}.
//
// \index{itk::BSplineDeformableTransform}
// \index{itk::BSplineDeformableTransform!DeformableRegistration}
// \index{itk::LBFGSBOptimizer}
//
//
// Software Guide : EndLatex 

#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"

#include "itkTimeProbesCollectorBase.h"

//  Software Guide : BeginLatex
//  
//  The following are the most relevant headers to this example.
//
//  \index{itk::BSplineDeformableTransform!header}
//  \index{itk::LBFGSBOptimizer!header}
// 
//  Software Guide : EndLatex 

// Software Guide : BeginCodeSnippet
#include "itkBSplineDeformableTransform.h"
#include "itkLBFGSBOptimizer.h"
#include "itkOnePlusOneEvolutionaryOptimizer.h"
#include "itkNormalVariateGenerator.h" 
#include "itkOrientedImage.h"
#include "itkOrientImageFilter.h"

// Software Guide : EndCodeSnippet

//  Software Guide : BeginLatex
//  
//  The parameter space of the \code{BSplineDeformableTransform} is composed by
//  the set of all the deformations associated with the nodes of the B-spline
//  grid.  This large number of parameters makes possible to represent a wide
//  variety of deformations, but it also has the price of requiring a
//  significant amount of computation time.
//
//  \index{itk::BSplineDeformableTransform!header}
// 
//  Software Guide : EndLatex 

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkSquaredDifferenceImageFilter.h"

#include "BSplineDeformableRegistrationCLP.h"

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
  typedef itk::LBFGSBOptimizer     OptimizerType;
//  typedef itk::OnePlusOneEvolutionaryOptimizer OptimizerType;
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
      std::cout << optimizer->GetValue() << "   ";
      std::cout << optimizer->GetInfinityNormOfProjectedGradient() << std::endl;
    }
};


int main( int argc, char *argv[] )
{
  const    unsigned int  ImageDimension = 3;
  typedef  signed short  PixelType;
  typedef  signed short  OutputPixelType;

  typedef itk::OrientedImage< PixelType, ImageDimension >       InputImageType;
  typedef itk::OrientedImage< OutputPixelType, ImageDimension > OutputImageType;

  PARSE_ARGS;

  //  Software Guide : BeginLatex
  //
  //  We instantiate now the type of the \code{BSplineDeformableTransform} using
  //  as template parameters the type for coordinates representation, the
  //  dimension of the space, and the order of the B-spline. 
  // 
  //  \index{BSplineDeformableTransform!New}
  //  \index{BSplineDeformableTransform!Instantiation}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  const unsigned int SpaceDimension = ImageDimension;
  const unsigned int SplineOrder = 3;
  typedef double CoordinateRepType;

  typedef itk::BSplineDeformableTransform<
                            CoordinateRepType,
                            SpaceDimension,
                            SplineOrder >     TransformType;
  // Software Guide : EndCodeSnippet


  typedef itk::LBFGSBOptimizer       OptimizerType;
#if 0
  typedef itk::OnePlusOneEvolutionaryOptimizer OptimizerType;
#endif

  typedef itk::MattesMutualInformationImageToImageMetric< 
                                    InputImageType, 
                                    InputImageType >    MetricType;

  typedef itk:: LinearInterpolateImageFunction< 
                                    InputImageType,
                                    double          >    InterpolatorType;

  typedef itk::ImageRegistrationMethod< 
                                    InputImageType, 
                                    OutputImageType >    RegistrationType;

  MetricType::Pointer         metric        = MetricType::New();
  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
  RegistrationType::Pointer   registration  = RegistrationType::New();
  

  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetInterpolator(  interpolator  );


  //  Software Guide : BeginLatex
  //
  //  The transform object is constructed below and passed to the registration
  //  method.
  //  \index{itk::RegistrationMethod!SetTransform()}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  TransformType::Pointer  transform = TransformType::New();
  registration->SetTransform( transform );
  // Software Guide : EndCodeSnippet

  typedef itk::ImageFileReader< InputImageType > FixedImageReaderType;
  typedef itk::ImageFileReader< InputImageType > MovingImageReaderType;
  typedef itk::OrientImageFilter<InputImageType,InputImageType> OrientFilterType;

  FixedImageReaderType::Pointer  fixedImageReader  = FixedImageReaderType::New();
  MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

  fixedImageReader->SetFileName(  fixedImageFileName.c_str() );
  movingImageReader->SetFileName( movingImageFileName.c_str() );

  OrientFilterType::Pointer fixedOrient = OrientFilterType::New();
  OrientFilterType::Pointer movingOrient = OrientFilterType::New();

  fixedOrient->UseImageDirectionOn();
  fixedOrient->SetDesiredCoordinateOrientationToAxial();
  fixedOrient->SetInput (fixedImageReader->GetOutput());

  movingOrient->UseImageDirectionOn();
  movingOrient->SetDesiredCoordinateOrientationToAxial();
  movingOrient->SetInput (movingImageReader->GetOutput());

  InputImageType::ConstPointer fixedImage = fixedImageReader->GetOutput();

  registration->SetFixedImage(  fixedOrient->GetOutput()   );
  registration->SetMovingImage(   movingOrient->GetOutput()   );


  // Add a time probe
  itk::TimeProbesCollectorBase collector;

  collector.Start( "Read fixed volume" );
  fixedOrient->Update();
  fixedOrient->GetOutput()->Print(std::cout);
  collector.Stop( "Read fixed volume" );

  collector.Start( "Read moving volume" );
  movingOrient->Update();
  movingOrient->GetOutput()->Print(std::cout);
  collector.Stop( "Read moving volume" );


  //  Software Guide : BeginLatex
  //
  //  Here we define the parameters of the BSplineDeformableTransform grid.  We
  //  arbitrarily decide to use a grid with $5 \times 5$ nodes within the image. 
  //  The reader should note that the B-spline computation requires a
  //  finite support region ( 1 grid node at the lower borders and 2
  //  grid nodes at upper borders). Therefore in this example, we set
  //  the grid size to be $8 \times 8$ and place the grid origin such that
  //  grid node (1,1) coincides with the first pixel in the fixed image.
  // 
  //  \index{BSplineDeformableTransform}
  //
  //  Software Guide : EndLatex 


  // Software Guide : BeginCodeSnippet
  typedef TransformType::RegionType RegionType;
  RegionType bsplineRegion;
  RegionType::SizeType   gridSizeOnImage;
  RegionType::SizeType   gridBorderSize;
  RegionType::SizeType   totalGridSize;

  gridSizeOnImage.Fill( gridSize );
  gridBorderSize.Fill( 3 );    // Border for spline order = 3 ( 1 lower, 2 upper )
  totalGridSize = gridSizeOnImage + gridBorderSize;

  bsplineRegion.SetSize( totalGridSize );

  typedef TransformType::SpacingType SpacingType;
  SpacingType spacing = fixedOrient->GetOutput()->GetSpacing();

  typedef TransformType::OriginType OriginType;
  OriginType origin = fixedOrient->GetOutput()->GetOrigin();;

  InputImageType::RegionType fixedRegion = fixedOrient->GetOutput()->GetLargestPossibleRegion();
  InputImageType::SizeType fixedImageSize = fixedRegion.GetSize();

  for(unsigned int r=0; r<ImageDimension; r++)
    {
    spacing[r] *= floor( static_cast<double>(fixedImageSize[r] - 1)  / 
                  static_cast<double>(gridSizeOnImage[r] - 1) );
    origin[r]  -=  spacing[r]; 
    }

  transform->SetGridSpacing( spacing );
  transform->SetGridOrigin( origin );
  transform->SetGridRegion( bsplineRegion );
  

  typedef TransformType::ParametersType     ParametersType;

  const unsigned int numberOfParameters =
               transform->GetNumberOfParameters();
  
  ParametersType parameters( numberOfParameters );

  parameters.Fill( 0.0 );

  transform->SetParameters( parameters );
  //  Software Guide : EndCodeSnippet



  //  Software Guide : BeginLatex
  //  
  //  We now pass the parameters of the current transform as the initial
  //  parameters to be used when the registration process starts. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  registration->SetInitialTransformParameters( transform->GetParameters() );
  // Software Guide : EndCodeSnippet

 // std::cout << "Intial Parameters = " << std::endl;
  //  std::cout << transform->GetParameters() << std::endl;

  //  Software Guide : BeginLatex
  //  
  //  Next we set the parameters of the LBFGSB Optimizer. 
  //
  //  Software Guide : EndLatex 


  // Software Guide : BeginCodeSnippet
  OptimizerType::BoundSelectionType boundSelect( transform->GetNumberOfParameters() );
  OptimizerType::BoundValueType upperBound( transform->GetNumberOfParameters() );
  OptimizerType::BoundValueType lowerBound( transform->GetNumberOfParameters() );

  boundSelect.Fill( 2 );
  upperBound.Fill( 10.0 );
  lowerBound.Fill( -10.0 );
  boundSelect.Fill( 0 );
  upperBound.Fill( 0.0 );
  lowerBound.Fill( 0.0 );
  optimizer->SetBoundSelection( boundSelect );
  optimizer->SetUpperBound( upperBound );
  optimizer->SetLowerBound( lowerBound );

  optimizer->SetCostFunctionConvergenceFactor( 1e+1 );
  optimizer->SetProjectedGradientTolerance( 1e-7 );
  optimizer->SetMaximumNumberOfIterations( Iterations );
  optimizer->SetMaximumNumberOfEvaluations( 500 );
  optimizer->SetMaximumNumberOfCorrections( 12 );

  // Software Guide : EndCodeSnippet
#if 0
  typedef itk::Statistics::NormalVariateGenerator  GeneratorType;

  GeneratorType::Pointer generator = GeneratorType::New();
  generator->Initialize(12345);

  typedef OptimizerType::ScalesType       OptimizerScalesType;
  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );
  optimizerScales.Fill(1.0);
  optimizer->SetNormalVariateGenerator( generator );
  optimizer->SetInitialRadius( 1.0 );
  optimizer->SetMaximumIteration( 100 );
  optimizer->MaximizeOn();

#endif


  // Create the Command observer and register it with the optimizer.
  //
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );


  //  Software Guide : BeginLatex
  //  
  //  Next we set the parameters of the Mattes Mutual Information Metric. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  metric->SetNumberOfHistogramBins( HistogramBins );
  metric->SetNumberOfSpatialSamples( SpatialSamples );
  // Software Guide : EndCodeSnippet
 

  //  Software Guide : BeginLatex
  //  
  //  Given that the Mattes Mutual Information metric uses a random iterator in
  //  order to collect the samples from the images, it is usually convenient to
  //  initialize the seed of the random number generator.
  //
  //  \index{itk::Mattes\-Mutual\-Information\-Image\-To\-Image\-Metric!ReinitializeSeed()}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  metric->ReinitializeSeed( 76926294 );
  // Software Guide : EndCodeSnippet


  std::cout << std::endl << "Starting Registration" << std::endl;

  try 
    { 
    collector.Start( "Registration" );
    registration->StartRegistration(); 
    collector.Stop( "Registration" );
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    return -1;
    } 
  
  OptimizerType::ParametersType finalParameters = 
                    registration->GetLastTransformParameters();


  // Software Guide : BeginCodeSnippet
  transform->SetParameters( finalParameters );
  // Software Guide : EndCodeSnippet


  typedef itk::ResampleImageFilter< 
                            InputImageType, 
                            OutputImageType >    ResampleFilterType;

  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform( transform );
  resample->SetInput( movingOrient->GetOutput() );
  resample->SetOutputParametersFromImage ( fixedOrient->GetOutput() );
  resample->SetDefaultPixelValue( 100 );
  collector.Start( "Resample" );
  resample->Update();
  collector.Stop( "Resample" );
  
  typedef itk::CastImageFilter< 
                        InputImageType,
                        OutputImageType > CastFilterType;
                    
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;


  WriterType::Pointer      writer =  WriterType::New();
  CastFilterType::Pointer  caster =  CastFilterType::New();


  writer->SetFileName( resampledImageFileName.c_str() );


  caster->SetInput( resample->GetOutput() );
  writer->SetInput( caster->GetOutput()   );


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
    return -1;
    } 
 


#if 0
  typedef itk::SquaredDifferenceImageFilter< 
                                  FixedImageType, 
                                  FixedImageType, 
                                  OutputImageType > DifferenceFilterType;

  DifferenceFilterType::Pointer difference = DifferenceFilterType::New();

  WriterType::Pointer writer2 = WriterType::New();
  writer2->SetInput( difference->GetOutput() );  
  
  // Compute the difference image between the 
  // fixed and resampled moving image.
  if( argc >= 5 )
    {
    difference->SetInput1( fixedImageReader->GetOutput() );
    difference->SetInput2( resample->GetOutput() );
    writer2->SetFileName( argv[4] );
    try
      {
      writer2->Update();
      }
    catch( itk::ExceptionObject & err ) 
      { 
      std::cerr << "ExceptionObject caught !" << std::endl; 
      std::cerr << err << std::endl; 
      return -1;
      } 
    }


  // Compute the difference image between the 
  // fixed and moving image before registration.
  if( argc >= 6 )
    {
    writer2->SetFileName( argv[5] );
    difference->SetInput1( fixedImageReader->GetOutput() );
    difference->SetInput2( movingImageReader->GetOutput() );
    try
      {
      writer2->Update();
      }
    catch( itk::ExceptionObject & err ) 
      { 
      std::cerr << "ExceptionObject caught !" << std::endl; 
      std::cerr << err << std::endl; 
      return -1;
      } 
    }


  // Generate the explicit deformation field resulting from 
  // the registration.
  if( 1 )
    {

    typedef itk::Vector< float, ImageDimension >  VectorType;
    typedef itk::Image< VectorType, ImageDimension >  DeformationFieldType;

    DeformationFieldType::Pointer field = DeformationFieldType::New();
    field->SetRegions( fixedRegion );
    field->SetOrigin( fixedImage->GetOrigin() );
    field->SetSpacing( fixedImage->GetSpacing() );
    field->Allocate();

    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi( field, fixedRegion );

    fi.GoToBegin();

    TransformType::InputPointType  fixedPoint;
    TransformType::OutputPointType movingPoint;
    DeformationFieldType::IndexType index;

    VectorType displacement;

    while( ! fi.IsAtEnd() )
      {
      index = fi.GetIndex();
      field->TransformIndexToPhysicalPoint( index, fixedPoint );
      movingPoint = transform->TransformPoint( fixedPoint );
      displacement = movingPoint - fixedPoint;
      fi.Set( displacement );
      ++fi;
      }



    typedef itk::ImageFileWriter< DeformationFieldType >  FieldWriterType;
    FieldWriterType::Pointer fieldWriter = FieldWriterType::New();

    fieldWriter->SetInput( field );

    fieldWriter->SetFileName( "d:/lorensen/Data/DeformableRegistration/Dartmouth/vector.mhd" );
    try
      {
      fieldWriter->Update();
      }
    catch( itk::ExceptionObject & excp )
      {
      std::cerr << "Exception thrown " << std::endl;
      std::cerr << excp << std::endl;
      return EXIT_FAILURE;
      }
    }
#endif

  // Report the time taken by the registration
  collector.Report();

  return 0;
}

