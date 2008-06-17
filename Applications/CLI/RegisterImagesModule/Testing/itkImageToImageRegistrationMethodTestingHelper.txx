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
#ifndef __ImageToImageRegistrationMethodTestingHelper_txx
#define __ImageToImageRegistrationMethodTestingHelper_txx

#include "itkImageToImageRegistrationMethodTestingHelper.h"

namespace itk
{

template< class TRegistrationMethod >
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::ImageToImageRegistrationMethodTestingHelper()
  {
  this->m_RegistrationMethod  = RegistrationMethodType::New();
  this->m_FixedImageReader    = ImageReaderType::New();
  this->m_MovingImageReader   = ImageReaderType::New();
  this->m_MovingImageWriter   = ImageWriterType::New();
  this->m_BaselineImageReader = ImageReaderType::New();
  this->m_DifferenceImageWriter   = ImageWriterType::New();
  this->m_WriteDifferenceImage = false;
  this->m_ResampleFilter      = ResampleFilterType::New();
  this->m_DifferenceFilter    = DifferenceFilterType::New();
  this->m_NumberOfFailedPixelsTolerance = 100;
  this->m_RadiusTolerance     = 0;
  this->m_IntensityTolerance  = 10;
  this->m_ErrorState          = false;
  }

template< class TRegistrationMethod >
typename ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod >::RegistrationMethodType *
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::GetRegistrationMethod()
  {
  return this->m_RegistrationMethod;
  }

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::SetFixedImageFileName( const char * filename )
  {
  this->m_FixedImageReader->SetFileName( filename );
  }

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::SetMovingImageFileName( const char * filename )
  {
  this->m_MovingImageReader->SetFileName( filename );
  }

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::SetResampledImageFileName( const char * filename )
  {
  this->m_MovingImageWriter->SetFileName( filename );
  }

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::SetBaselineImageFileName( const char * filename )
  {
  this->m_BaselineImageReader->SetFileName( filename );
  }

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::SetDifferenceImageFileName( const char * filename )
  {
  if( filename != NULL )
    {
    this->m_DifferenceImageWriter->SetFileName( filename );
    this->m_WriteDifferenceImage = true;
    }
  else
    {
    this->m_WriteDifferenceImage = false;
    }
  }


template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::PrepareRegistration()
  {
  this->m_RegistrationMethod->SetFixedImage( this->m_FixedImageReader->GetOutput() );
  this->m_RegistrationMethod->SetMovingImage( this->m_MovingImageReader->GetOutput() );
  
  this->m_RegistrationMethod->ReportProgressOn();

  this->m_ResampleFilter->SetInput( this->m_MovingImageReader->GetOutput() );
  this->m_MovingImageWriter->SetInput( this->m_ResampleFilter->GetOutput() );
  }

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::RunRegistration()
  {

  this->m_ErrorState = false;

  try
    {
    this->m_RegistrationMethod->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << excp << std::endl;
    this->m_ErrorState = true;
    }
  catch( ... ) 
    {
    std::cerr << "Unknown exception" << std::endl;
    this->m_ErrorState = true;
    }
  }

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::ReportResults()
  {
  if( this->m_ErrorState )
    {
    return;
    }

  //
  // Registration result
  //
  typedef typename RegistrationMethodType::TransformType  TransformType;
  
  typename TransformType::Pointer transform = this->m_RegistrationMethod->GetTypedTransform();

  std::cout << "Transform Result = " << std::endl;
  std::cout << transform << std::endl;
}


template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::ReportImagesMetaData()
{
  if( this->m_ErrorState )
    {
    return;
    }

  std::cout << "Fixed Image " << std::endl;
  this->m_FixedImageReader->GetOutput()->Print( std::cout );

  std::cout << "Moving Image " << std::endl;
  this->m_MovingImageReader->GetOutput()->Print( std::cout );
  }

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::ResampleOutputImage()
  {

  if( this->m_ErrorState )
    {
    return;
    }

  this->m_ResampleFilter->SetTransform( this->m_RegistrationMethod->GetTypedTransform() );
  this->m_ResampleFilter->SetDefaultPixelValue( 0 );

  // SetReferenceImage() has to be called here, when we are sure that the fixed
  // image has been read. Something that should occur in the RunRegistration()
  // method. 
  this->m_ResampleFilter->SetReferenceImage( this->m_FixedImageReader->GetOutput() );
  this->m_ResampleFilter->UseReferenceImageOn();
  
  try
    {
    this->m_MovingImageWriter->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << excp << std::endl;
    this->m_ErrorState = true;
    }
  catch( ... )
    {
    std::cerr << "Unknown exception" << std::endl;
    this->m_ErrorState = true;
    }
  }


template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::SetNumberOfFailedPixelsTolerance( unsigned long numberOfFailedPixels )
{
  this->m_NumberOfFailedPixelsTolerance = numberOfFailedPixels;
}

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::SetRadiusTolerance( unsigned long radius )
{
  this->m_RadiusTolerance = radius;
}

template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::SetIntensityTolerance( double intensity )
{
  this->m_IntensityTolerance = intensity;
}


template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::PerformRegressionTest()
  {
  if( this->m_ErrorState )
    {
    return;
    }

  this->m_BaselineImageReader->Update();

  this->m_DifferenceFilter->SetValidInput( this->m_BaselineImageReader->GetOutput() );
  this->m_DifferenceFilter->SetTestInput(  this->m_ResampleFilter->GetOutput() );

  this->m_DifferenceFilter->SetDifferenceThreshold( this->m_IntensityTolerance );
  this->m_DifferenceFilter->SetToleranceRadius( this->m_RadiusTolerance );
  this->m_DifferenceFilter->SetIgnoreBoundaryPixels( true );

  try
    {
    this->m_DifferenceFilter->UpdateLargestPossibleRegion();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << excp << std::endl;
    this->m_ErrorState = true;
    return;
    }
  catch( ... )
    {
    std::cerr << "Unknwon exception" << std::endl;
    this->m_ErrorState = true;
    return;
    }

  if(this->m_WriteDifferenceImage)
    {
    this->m_DifferenceImageWriter->SetInput( this->m_DifferenceFilter->GetOutput() );
    this->m_DifferenceImageWriter->Write();
    }

  unsigned long numberOfFailedPixels = this->m_DifferenceFilter->GetNumberOfPixelsWithDifferences();

  if( numberOfFailedPixels > this->m_NumberOfFailedPixelsTolerance )
    {
    std::cerr << "Too many failed pixels in regression Tests" << std::endl;
    std::cerr << numberOfFailedPixels << " Failed pixels " << std::endl;
    std::cerr << this->m_NumberOfFailedPixelsTolerance << " can be tolerated " << std::endl;
    this->m_ErrorState = true;
    }
  else 
    {
    std::cout << "Regression Test passed !" << std::endl;
    std::cout << numberOfFailedPixels << " Failed pixels " << std::endl;
    std::cout << this->m_NumberOfFailedPixelsTolerance << " can be tolerated " << std::endl;
    }

  }


template< class TRegistrationMethod >
void 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::PrintTest()
  {
  //
  // Exercise the Print() method.
  //
  this->m_RegistrationMethod->Print( std::cout );
  }


template< class TRegistrationMethod >
int 
ImageToImageRegistrationMethodTestingHelper< TRegistrationMethod > 
::GetTestResult() const
  {
  if( this->m_ErrorState )
    {
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;  
  }


} // end namespace itk

#endif
