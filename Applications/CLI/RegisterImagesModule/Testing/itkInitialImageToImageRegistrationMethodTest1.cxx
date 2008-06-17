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
                                                  RegistrationMethodType;

  typedef itk::ImageToImageRegistrationMethodTestingHelper< RegistrationMethodType >   
                                                  TestingHelperType;

  //  Setup the registration
  TestingHelperType  helper;

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

  if( command.GetOptionWasSet("NumberOfMoments") )
    {
    registrationMethod->SetNumberOfMoments( command.GetValueAsInt("NumberOfMoments") );
    }
  else if( command.GetOptionWasSet("SetCenterOfRotationOnly") )
    {
    registrationMethod->SetComputeCenterOfRotationOnly( true );
    }

  helper.PrepareRegistration();
  helper.SetNumberOfFailedPixelsTolerance( command.GetValueAsInt("FailedPixelTolerance") );
  helper.SetIntensityTolerance( command.GetValueAsFloat("FailedIntensityTolerance") );
  helper.SetRadiusTolerance( command.GetValueAsInt("FailedOffsetTolerance") );
  helper.RunRegistration();
  helper.PrintTest();
  helper.ReportResults();
  helper.ResampleOutputImage();
  helper.PerformRegressionTest();

  return helper.GetTestResult();
}

int main(int argc, char *argv[])
{
  MetaCommand command;


  command.SetOption("NumberOfMoments", "m", false,
                    "Number of images moments to use to align moving with fixed");
  command.SetOptionLongTag("NumberOfMoments", "NumberOfMoments");
  command.AddOptionField("NumberOfMoments", "NumberOfMoments",
                         MetaCommand::INT, true, "1");

  command.SetOption("SetCenterOfRotationOnly", "c", false,
                    "Only set the center of rotation");
  command.SetOptionLongTag("SetCenterOfRotationOnly", "SetCenterOfRotationOnly");

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
                    "Save the difference between the resampled image and the baseline");
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
    std::cerr << "Fixed image dimensions = " << dimensions << std::endl;

    return EXIT_FAILURE;
    }
}
