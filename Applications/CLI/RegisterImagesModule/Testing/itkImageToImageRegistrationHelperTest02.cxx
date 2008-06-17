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

#include "itkImageToImageRegistrationHelper.h"
#include "itkOrientedImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

int main( int argc, char *argv[] )
{
  if( argc < 4 )
    {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " fixedImageFile  movingImageFile ";
    std::cerr << " outputImagefile "<< std::endl;
    return EXIT_FAILURE;
    }
 
  typedef short          PixelType;
  const unsigned int     Dimension=3;
  
  typedef itk::OrientedImage< PixelType, Dimension >    ImageType;

  typedef itk::ImageToImageRegistrationHelper< ImageType >   RegistrationMethodType;

  typedef itk::ImageFileReader< ImageType  > ImageReaderType;

  ImageReaderType::Pointer  fixedImageReader  = ImageReaderType::New();
  ImageReaderType::Pointer  movingImageReader = ImageReaderType::New();

  fixedImageReader->SetFileName(  argv[1] );
  movingImageReader->SetFileName( argv[2] );

  RegistrationMethodType::Pointer  registrationMethod = RegistrationMethodType::New();


  registrationMethod->SetFixedImage( fixedImageReader->GetOutput() );
  registrationMethod->SetMovingImage( movingImageReader->GetOutput() );
  
  registrationMethod->ReportProgressOn();
  
  try
    {
    registrationMethod->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }


  //
  // Exercise the Print() method.
  //
  registrationMethod->Print( std::cout );

  return EXIT_SUCCESS;
}
