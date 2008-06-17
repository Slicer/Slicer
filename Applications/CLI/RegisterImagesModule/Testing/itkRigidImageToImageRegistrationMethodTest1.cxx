/*=========================================================================

  Program:   Surface Extraction Program
  Module:    $RCSfile: ExtractSurface.cxx,v $

  Copyright (c) Kitware Inc. 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "itkInitialImageToImageRegistrationMethod.h"
#include "itkRigidImageToImageRegistrationMethod.h"
#include "itkOrientedImage.h"
#include "itkImageToImageRegistrationMethodTestingHelper.h"

#include "metaCommand.h"

// Description:
// Get the PixelType and ComponentType from fileName
void GetImageType (std::string fileName,
                   itk::ImageIOBase::IOPixelType & pixelType,
                   itk::ImageIOBase::IOComponentType & componentType,
                   int & dimensions)
{
  typedef itk::OrientedImage<short, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader =
        itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName(fileName.c_str());
  imageReader->UpdateOutputInformation();

  pixelType = imageReader->GetImageIO()->GetPixelType();
  componentType = imageReader->GetImageIO()->GetComponentType();
  dimensions = imageReader->GetImageIO()->GetNumberOfDimensions();
}

template <unsigned int DimensionsT>
int DoIt( MetaCommand & command )
{
  typedef short     PixelType;
  
  typedef itk::OrientedImage< PixelType, DimensionsT >    
                                                  ImageType;

  typedef itk::InitialImageToImageRegistrationMethod< ImageType >   
                                                  InitializationMethodType;

  typedef itk::RigidImageToImageRegistrationMethod< ImageType >
                                                  RegistrationMethodType;

  typedef itk::ImageToImageRegistrationMethodTestingHelper< InitializationMethodType >
                                                  InitializationHelperType;

  typedef itk::ImageToImageRegistrationMethodTestingHelper< RegistrationMethodType >   
                                                  TestingHelperType;

  // Use this to set center of rotation to image centers
  InitializationHelperType initializer;

  initializer.SetFixedImageFileName( command.GetValueAsString("FixedImage").c_str() );
  initializer.SetMovingImageFileName( command.GetValueAsString("MovingImage").c_str() );

  typename InitializationMethodType::Pointer  initializationMethod = initializer.GetRegistrationMethod();
  //initializationMethod->SetComputeCenterOfRotationOnly( true );
  initializationMethod->SetNumberOfMoments( 1 );

  initializer.PrepareRegistration();
  try
    {
    initializer.RunRegistration();
    }
  catch(...)
    {
    std::cerr << "Intialization failed" << std::endl;
    return EXIT_FAILURE;
    }

  //  Setup the registration
  TestingHelperType  helper;

  helper.SetFixedImageFileName( command.GetValueAsString("FixedImage").c_str() );
  helper.SetMovingImageFileName( command.GetValueAsString("MovingImage").c_str() );
  if( command.GetOptionWasSet("ResampledMovingImage") )
    {
    std::cout << "Setting output filename = " << command.GetValueAsString("ResampledMovingImage")
              << std::endl;
    helper.SetResampledImageFileName( command.GetValueAsString("ResampledMovingImage").c_str() );
    }
  if( command.GetOptionWasSet("BaselineResampledMovingImage") )
    {
    std::cout << "Setting baseline filename = " << command.GetValueAsString("BaselineResampledMovingImage")
              << std::endl;
    helper.SetBaselineImageFileName( command.GetValueAsString("BaselineResampledMovingImage").c_str() );
    }

  if( command.GetOptionWasSet("DifferenceImage") )
    {
    std::cout << "Setting difference filename = " << command.GetValueAsString("DifferenceImage")
              << std::endl;
    helper.SetDifferenceImageFileName( command.GetValueAsString("DifferenceImage").c_str() );
    }

  typename RegistrationMethodType::Pointer  registrationMethod = helper.GetRegistrationMethod();

  typedef typename RegistrationMethodType::TransformParametersScalesType    
                                              TransformParametersScalesType;

  TransformParametersScalesType optimizerScales( registrationMethod->GetTypedTransform()->GetNumberOfParameters() );
  const double offsetScale = 1.0 / command.GetValueAsFloat("ExpectedOffset");
  const double rotationScale = 1.0 / command.GetValueAsFloat("ExpectedRotation");
  std::cout << "Setting Offset Scale = " << offsetScale << std::endl;
  std::cout << "Setting Rotation Scale = " << rotationScale << std::endl;

  if(DimensionsT == 2)
    {
    optimizerScales[0] = rotationScale;

    optimizerScales[1] = offsetScale;
    optimizerScales[2] = offsetScale;
    }
  else
    {
    optimizerScales[0] = rotationScale;
    optimizerScales[1] = rotationScale;
    optimizerScales[2] = rotationScale;

    optimizerScales[3] = offsetScale;
    optimizerScales[4] = offsetScale;
    optimizerScales[5] = offsetScale;
    }

  registrationMethod->SetTransformParametersScales( optimizerScales );

  if( command.GetOptionWasSet("MeanSquares") )
    {
    std::cout << "Setting Mean Squares Error metric " << std::endl;
    registrationMethod->SetMetricMethodEnum( RegistrationMethodType::MEAN_SQUARED_ERROR_METRIC );
    }
  if( command.GetOptionWasSet("NormalizedCorrelation") )
    {
    std::cout << "Setting Normalized Correlation metric " << std::endl;
    registrationMethod->SetMetricMethodEnum( RegistrationMethodType::NORMALIZED_CORRELATION_METRIC );
    }

  // Set the center of rotation from the initializer
  registrationMethod->SetInitialTransformParametersFromAffineTransform( 
    initializer.GetRegistrationMethod()->GetTypedTransform() );

  // Run
  try
    {
    helper.PrepareRegistration();
    helper.SetNumberOfFailedPixelsTolerance( command.GetValueAsInt("FailurePixelTolerance") );
    helper.SetIntensityTolerance( command.GetValueAsFloat("FailureIntensityTolerance") );
    helper.SetRadiusTolerance( command.GetValueAsInt("FailureOffsetTolerance") );
    helper.RunRegistration();
    helper.PrintTest();
    helper.ReportResults();
    helper.ResampleOutputImage();
    helper.PerformRegressionTest();
    }
  catch(...)
    {
    std::cerr << "Registration class threw an exception" << std::endl;
    return EXIT_FAILURE;
    }

  return helper.GetTestResult();
}

int main(int argc, char *argv[])
{
  MetaCommand command;

  // Scales
  command.SetOption("ExpectedOffset", "o", false,
                    "Expected offset needed to align moving with fixed");
  command.SetOptionLongTag("ExpectedOffset", "ExpectedOffset");
  command.AddOptionField("ExpectedOffset", "ExpectedOffset",
                         MetaCommand::FLOAT, true, "10");

  command.SetOption("ExpectedRotation", "r", false,
                    "Expected rotation needed to align moving with fixed");
  command.SetOptionLongTag("ExpectedRotation", "ExpectedRotation");
  command.AddOptionField("ExpectedRotation", "ExpectedRotation",
                         MetaCommand::FLOAT, true, "0.1");

  // Optimizer
  command.SetOption("MultiResolution", "m", false,
                    "Use multi-resolution framework during optimization");
  command.SetOptionLongTag("MultiResolution", "MultiResolution");

  // Metric
  command.SetOption("MeanSquares", "q", false,
                    "Use mean squared error metric, not mutual information");
  command.SetOptionLongTag("MeanSquares", "MeanSquares");

  command.SetOption("NormalizedCorrelation", "n", false,
                    "Use normalized correlation metric, not mutual information");
  command.SetOptionLongTag("NormalizedCorrelation", "NormalizedCorrelation");

  // Output Image
  command.SetOption("ResampledMovingImage", "R", false,
                    "Save registered moving image <Filename>");
  command.SetOptionLongTag("ResampledMovingImage", "ResampledMovingImage");
  command.AddOptionField("ResampledMovingImage", "ResampledMovingImage",
                         MetaCommand::STRING, true);

  // Compute Error
  command.SetOption("BaselineResampledMovingImage", "B", false,
                    "Load baseline registered moving image <Filename>");
  command.SetOptionLongTag("BaselineResampledMovingImage",
                           "BaselineResampledMovingImage");
  command.AddOptionField("BaselineResampledMovingImage", "BaselineResampledMovingImage",
                         MetaCommand::STRING, true);

  command.SetOption("DifferenceImage", "D", false,
                    "Save the difference between resample moving and baseline");
  command.SetOptionLongTag("DifferenceImage",
                           "DifferenceImage");
  command.AddOptionField("DifferenceImage", "DifferenceImage",
                         MetaCommand::STRING, true);

  command.SetOption("FailurePixelTolerance", "P", false,
                    "Number of pixel mis-matches between baseline and resampled allowed");
  command.SetOptionLongTag("FailurePixelTolerance",
                           "FailurePixelTolerance");
  command.AddOptionField("FailurePixelTolerance", "FailurePixelTolerance",
                         MetaCommand::INT, true, "1000");

  command.SetOption("FailureIntensityTolerance", "I", false,
                    "Intensity mis-matches between baseline and resampled allowed");
  command.SetOptionLongTag("FailureIntensityTolerance",
                           "FailureIntensityTolerance");
  command.AddOptionField("FailureIntensityTolerance", "FailureIntensityTolerance",
                         MetaCommand::INT, true, "10");

  command.SetOption("FailureOffsetTolerance", "O", false,
                    "Number of pixel offset between baseline and resampled allowed");
  command.SetOptionLongTag("FailureOffsetTolerance",
                           "FailureOffsetTolerance");
  command.AddOptionField("FailureOffsetTolerance", "FailureOffsetTolerance",
                         MetaCommand::FLOAT, true, "0");

  // Input images: Fixed and Moving
  command.AddField("FixedImage", "Fixed image's file name",
                   MetaCommand::STRING, true);
  command.AddField("MovingImage", "Moving image's file name",
                   MetaCommand::STRING, true);

  // Parse
  if( !command.Parse(argc, argv) )
    {
    return EXIT_FAILURE;
    }

  // Determine properties of the input images
  int dimensions = 0;
  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    GetImageType( command.GetValueAsString("FixedImage").c_str(),
                  pixelType, componentType, dimensions );
    }
  catch( ... )
    {
    std::cerr << "ERROR: exception caught while determining properties of the fixed image."
              << std::endl;
    return EXIT_FAILURE;
    }

  // Call 2D or 3D version as appropriate
  if( dimensions == 2 )
    {
    return DoIt<2>( command );
    }
  else if( dimensions == 3 )
    {
    return DoIt<3>( command );
    }
  else
    {
    std::cerr << "ERROR: Only 2 and 3 dimensional images supported."
              << std::endl;
    std::cerr << "Fixed image = " << command.GetValueAsString("FixedImage")
              << std::endl;
    std::cerr << "Fixed image dimensions = " << dimensions << std::endl;

    return EXIT_FAILURE;
    }
}
