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

#include "itkBSplineImageToImageRegistrationMethod.h"

#include "itkImageToImageRegistrationMethodTestingHelper.h"

// ITK includes
#include <itkImage.h>
#include <metaCommand.h>

// Description:
// Get the PixelType and ComponentType from fileName
void GetImageType(std::string fileName,
                  itk::ImageIOBase::IOPixelType & pixelType,
                  itk::ImageIOBase::IOComponentType & componentType,
                  int & dimensions)
{
  typedef itk::Image<short, 3> ImageType;
  itk::ImageFileReader<ImageType>::Pointer imageReader =
    itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName(fileName.c_str() );
  imageReader->UpdateOutputInformation();

  pixelType = imageReader->GetImageIO()->GetPixelType();
  componentType = imageReader->GetImageIO()->GetComponentType();
  dimensions = imageReader->GetImageIO()->GetNumberOfDimensions();
}

template <unsigned int DimensionsT>
int DoIt( MetaCommand & command )
{
  typedef short PixelType;

  typedef itk::Image<PixelType, DimensionsT>
  ImageType;

  typedef itk::BSplineImageToImageRegistrationMethod<ImageType>
  RegistrationMethodType;

  typedef itk::ImageToImageRegistrationMethodTestingHelper<RegistrationMethodType>
  TestingHelperType;

  //  Setup the registration
  TestingHelperType helper;

  helper.SetFixedImageFileName( command.GetValueAsString("FixedImage").c_str() );
  helper.SetMovingImageFileName( command.GetValueAsString("MovingImage").c_str() );
  if( command.GetOptionWasSet("ResampledMovingImage") )
    {
    helper.SetResampledImageFileName( command.GetValueAsString("ResampledMovingImage").c_str() );
    }
  if( command.GetOptionWasSet("BaselineResampledMovingImage") )
    {
    helper.SetBaselineImageFileName( command.GetValueAsString("BaselineResampledMovingImage").c_str() );
    }
  if( command.GetOptionWasSet("DifferenceImage") )
    {
    helper.SetDifferenceImageFileName( command.GetValueAsString("DifferenceImage").c_str() );
    }

  typename RegistrationMethodType::Pointer  registrationMethod = helper.GetRegistrationMethod();

  // BSpline specific parameters
  registrationMethod->SetExpectedDeformationMagnitude( command.GetValueAsFloat("ExpectedOffset") );
  if( command.GetOptionWasSet("MinimizeMemory") )
    {
    registrationMethod->SetMinimizeMemory( true );
    }
  registrationMethod->SetNumberOfControlPoints( command.GetValueAsInt("NumberOfControlPoints") );

  registrationMethod->SetNumberOfLevels( command.GetValueAsInt("NumberOfLevels") );

  // General optimizer parameters
  registrationMethod->SetNumberOfSamples( command.GetValueAsInt("NumberOfSamples") );

  registrationMethod->SetMaxIterations( command.GetValueAsInt("MaxIterations") );

  registrationMethod->SetRandomNumberSeed( command.GetValueAsInt("RandomNumberSeed") );

  if( command.GetOptionWasSet("MeanSquares") )
    {
    registrationMethod->SetMetricMethodEnum( RegistrationMethodType::MEAN_SQUARED_ERROR_METRIC );
    }
  if( command.GetOptionWasSet("NormalizedCorrelation") )
    {
    registrationMethod->SetMetricMethodEnum( RegistrationMethodType::NORMALIZED_CORRELATION_METRIC );
    }

  if( command.GetOptionWasSet("Mode") )
    {
    if( command.GetValueAsString("Mode") == "DRAFT" )
      {
      // registrationMethod->SetNumberOfControlPoints( 0.5 * registrationMethod->GetNumberOfControlPoints() );
      registrationMethod->SetNumberOfLevels( 2 );
      registrationMethod->SetMaxIterations( (unsigned int)(0.5 * registrationMethod->GetMaxIterations() ) );
      registrationMethod->SetNumberOfSamples( (unsigned int)(0.5 * registrationMethod->GetNumberOfSamples() ) );
      }
    else if( command.GetValueAsString("Mode") == "NORMAL" )
      {
      // registrationMethod->SetMaxIterations( registrationMethod->GetMaxIterations() );
      // registrationMethod->SetNumberOfSamples( registrationMethod->GetNumberOfSamples() );
      }
    else if( command.GetValueAsString("Mode") == "PRECISE" )
      {
      // registrationMethod->SetNumberOfControlPoints( 2 * registrationMethod->GetNumberOfControlPoints() );
      registrationMethod->SetMaxIterations( (unsigned int)(1.25 * registrationMethod->GetMaxIterations() ) );
      registrationMethod->SetNumberOfSamples( (unsigned int)(1.25 * registrationMethod->GetNumberOfSamples() ) );
      }
    else
      {
      std::cerr << "Mode type " << command.GetValueAsString("Mode")
                << " not recognized.  Using NORMAL." << std::endl;
      }
    }

  // Run
  helper.PrepareRegistration();
  helper.SetNumberOfFailedPixelsTolerance( command.GetValueAsInt("FailurePixelTolerance") );
  helper.SetIntensityTolerance( command.GetValueAsFloat("FailureIntensityTolerance") );
  helper.SetRadiusTolerance( command.GetValueAsInt("FailureOffsetTolerance") );
  helper.RunRegistration();
  // helper.PrintTest();
  // helper.ReportResults();
  helper.ResampleOutputImage();
  helper.PerformRegressionTest();

  return helper.GetTestResult();
}

int main(int argc, char *argv[])
{
  MetaCommand command;

  command.SetOption("Mode", "M", false,
                    "Registration mode: DRAFT, NORMAL, PRECISE");
  command.SetOptionLongTag("Mode",
                           "Mode");
  command.AddOptionField("Mode", "Mode", MetaCommand::STRING, true);

  // Scales
  command.SetOption("ExpectedOffset", "o", false,
                    "Expected offset needed to align moving with fixed");
  command.SetOptionLongTag("ExpectedOffset", "ExpectedOffset");
  command.AddOptionField("ExpectedOffset", "ExpectedOffset",
                         MetaCommand::FLOAT, true, "10");

  // General Optimizer Params
  command.SetOption("NumberOfSamples", "s", false,
                    "Number of samples from the fixed images for computing the metric");
  command.SetOptionLongTag("NumberOfSamples", "NumberOfSamples");
  command.AddOptionField("NumberOfSamples", "NumberOfSamples",
                         MetaCommand::INT, true, "400000");

  command.SetOption("MaxIterations", "s", false,
                    "Maximum number of optimizer iterations");
  command.SetOptionLongTag("MaxIterations", "MaxIterations");
  command.AddOptionField("MaxIterations", "MaxIterations",
                         MetaCommand::INT, true, "20");

  command.SetOption("RandomNumberSeed", "u", false,
                    "Seed used to generate random numbers (0 = random seed)");
  command.SetOptionLongTag("RandomNumberSeed", "RandomNumberSeed");
  command.AddOptionField("RandomNumberSeed", "RandomNumberSeed",
                         MetaCommand::INT, true, "0");

  // BSpline Specific Params
  command.SetOption("MinimizeMemory", "M", false,
                    "Reduce the amount of memory required");
  command.SetOptionLongTag("MinimizeMemory", "MinimizeMemory");

  command.SetOption("NumberOfControlPoints", "c", false,
                    "Number of control points to align moving with fixed");
  command.SetOptionLongTag("NumberOfControlPoints", "NumberOfControlPoints");
  command.AddOptionField("NumberOfControlPoints", "NumberOfControlPoints",
                         MetaCommand::INT, true, "5");

  command.SetOption("NumberOfLevels", "l", false,
                    "Number of multi-resolution levels to use during optimization");
  command.SetOptionLongTag("NumberOfLevels", "NumberOfLevels");
  command.AddOptionField("NumberOfLevels", "NumberOfLevels",
                         MetaCommand::INT, true, "4");

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
                    "Save the difference between the resampled image and baseline");
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
  int                               dimensions = 0;
  itk::ImageIOBase::IOPixelType     pixelType;
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
    std::cerr << "Fixed image dimensions = " << dimensions << std::endl;

    return EXIT_FAILURE;
    }
}
