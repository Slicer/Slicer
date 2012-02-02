/*=========================================================================

 Program:   Insight Segmentation & Registration Toolkit
 Module:    $RCSfile: itkNrrdVectorImageReadWriteTest.cxx,v $
 Language:  C++
 Date:      $Date: 2005/08/20 22:47:07 $
 Version:   $Revision: 1.1 $

 Copyright (c) Insight Software Consortium. All rights reserved.
 See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
/*
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
*/
#include <fstream>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkVectorImage.h"
#include "itkLMMSEVectorImageFilter.h"
#include "itkDTIEstimateTensorFilter.h"
#include "itkVector.h"
#include <string.h>
#include "itkDTITensor.h"
#include "itkComputeFAFilter.h"

int main( int ac, char* av[] )
{
  if( ac < 3 )
    {
    std::cerr << "Usage: " << av[0] << " Input Output [radiusLMMSEest=1 radiusLMMSEfilt=2 iterLMMSE=1]\n";
    return EXIT_FAILURE;
    }

  typedef  float                                            PixelType;
  typedef itk::VectorImage<PixelType, 3>                    ImageType;
  typedef itk::LMMSEVectorImageFilter<ImageType, ImageType> FilterType;

  itk::ImageFileReader<ImageType>::Pointer reader = itk::ImageFileReader<ImageType>::New();
  reader->SetFileName(av[1]);

  try
    {
    reader->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "exception in file reader " << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    return EXIT_FAILURE;
    }

  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( reader->GetOutput() );

  // ==========================================================================
  // Set the parameters to the filter:
  FilterType::InputSizeType radius;
  unsigned int              rade = 1;
  if( ac > 3 )
    {
    rade = ::atoi( av[3] );
    }
  radius.Fill( rade );
// -----------------------
  radius[2] = 0;
// -----------------------
  filter->SetRadiusEstimation( radius );
  unsigned int radf = 2;
  if( ac > 4 )
    {
    radf = ::atoi( av[4] );
    }
  radius.Fill( radf );
  radius[2] = 1;
  filter->SetRadiusFiltering( radius );
  unsigned int iter1 = 1;
  if( ac > 5 )
    {
    iter1 = ::atoi( av[5] );
    }

  filter->SetIterations( iter1 );
  filter->SetUseAbsoluteValue( false );
  filter->SetKeepValue( true );
  filter->SetMinimumNumberOfUsedVoxelsEstimation( 5 );
  filter->SetMinimumNumberOfUsedVoxelsFiltering( 5 );
  filter->SetMinimumNoiseSTD( 0 );
  filter->SetMaximumNoiseSTD( 32000 );
  filter->SetFirstBaseline( 0 );
  filter->SetHistogramResolutionFactor( 2.0 );
  filter->SetChannels( reader->GetOutput()->GetVectorLength() );
  // ==========================================================================

  try
    {
    if( iter1 > 0 )
      {
      std::cerr << "Filtrando..." << std::endl;
      filter->Update();
      std::cerr << "Filtrado!" << std::endl;
      }
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "exception inL MMSE filter" << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    return EXIT_FAILURE;
    }

/*
  typedef itk::DTITensor<float>                              TPixelType;
  typedef itk::Image<TPixelType,3>                           TImageType;
  typedef itk::DTIEstimateTensorFilter<ImageType,TImageType> DTIType;
  DTIType::Pointer dti = DTIType::New();
  if( iter1>0 )
    dti->SetInput( filter->GetOutput() );

  else
    dti->SetInput( reader->GetOutput() );

  //==========================================================================
  // Set the parameters to the filter:
  //==========================================================================
  // Parse the input file to obtain the b-value:
  double bvalue;
  bool success = false;
  const unsigned int LENGTH = 80;
  char buffer[LENGTH];
  FILE* fid = fopen( av[1], "r" );
  while(   fgets( (char*)buffer, LENGTH, fid )   !=   (char*)NULL   ){ // Read until EOF
    if( !strncmp(buffer,"DWMRI_b-value:=",15) ){
      bvalue = (float)::atof(  (char*)buffer + 15*sizeof(char)  );
      success = true;
      break;
    }
  }
  fclose( fid );
  if( !success ){
    std::cerr << "Cannot determine the value of b!!!" << std::endl;
    exit(-1);
  }
  //==========================================================================
  std::cout << "b-value: " << bvalue << std::endl;
  dti->SetB( bvalue );
  // The number of iterations; by default, only one iteration (simple WLS) is
  // performed:
  unsigned int iter2 = 1;
  if( ac>6 )
    iter2 = ::atoi( av[6] );
  float th = 200.0f;
  if( ac>7 )
    th = ::atof( av[7] );
  dti->SetIterations( iter2 );
  dti->SetThreshold( th );
  //==========================================================================
  // Parse the input file to obtain the gradient directions:
  DTIType::GradientType grad;
  unsigned int gradients = 0;
  fid = fopen( av[1], "r" );
  char* aux;
  char* aux2;
  char number[10];
  bool control = true;
  bool baseline = false;
  unsigned int counter = 0;
  std::vector<unsigned int> positions;
  while(   control   ){ // Read until EOF
    if(   fgets( (char*)buffer, LENGTH, fid ) == (char*)NULL   )
      control = false;
    if( !strncmp(buffer,"DWMRI_gradient_",15) ){
      // Find the first blank space:
      aux = (char*)buffer + 15*sizeof(char);
      while( *aux!='=' ){
        if( *aux=='\0'){
          std::cerr << "Cannot determine all the values of the gradients!!!" << std::endl;
          exit(-1);
        }
        aux += sizeof(char);
      }
      aux += sizeof(char);
      // Find the second blank space:
      aux2 = aux;                                // Initiallise aux2
      while( *aux2==' ' ){aux2+=sizeof(char);}   // Avoid multiple spaces
      while( *aux2!=' ' ){
        if( *aux2=='\0'){
          std::cerr << "Cannot determine all the values of the gradients!!!" << std::endl;
          exit(-1);
        }
        aux2 += sizeof(char);
      }
      // Copy the number into the number buffer:
      unsigned int l=0;
      while( aux<aux2 ){
        number[l] = *aux;
        aux += sizeof(char);
        l++;
      }
      number[l] = '\0';
      grad[0] = (float)::atof( number );
      // Find the third blank space:
      while( *aux2==' ' ){aux2+=sizeof(char);}   // Avoid multiple spaces
      while( *aux2!=' ' ){
        if( *aux2=='\0'){
          std::cerr << "Cannot determine all the values of the gradients!!!" << std::endl;
        exit(-1);
        }
        aux2 += sizeof(char);
      }
      // Copy the number into the number buffer:
      l=0;
      while( aux<aux2 ){
        number[l] = *aux;
        aux += sizeof(char);
        l++;
      }
      number[l] = '\0';
      grad[1] = (float)::atof( number );
      // Extract the last number:
      grad[2] = (float)::atof( aux );
      // Extract the norm:
      double norm = sqrt( grad[0]*grad[0] + grad[1]*grad[1] + grad[2]*grad[2] );
      if( norm>1e-3 ){
        std::cout << "Adding gradient direction: " << grad << " --> ";
        grad[0] /= norm;
        grad[1] /= norm;
        grad[2] /= norm;
        std::cout << grad << std::endl;
        // One more gradient!!!
                          ++gradients;
                          // Add the new gradient direction:
                          dti->AddGradientDirection( grad );
        positions.push_back( counter );
      }
      else if( !baseline  ){
        std::cout << "Adding baseline: " << grad << std::endl;
        // One more gradient!!!
                                ++gradients;
                                // Add the new gradient direction:
                                dti->AddGradientDirection( grad );
        // We already have a baseline!
        baseline = true;
        positions.push_back( counter );
      }
      else
        std::cout << "Ignoring baseline: " << grad << std::endl;
      ++counter;
    }
  }
  fclose( fid );
  // So the final number of channels is:
  dti->SetChannels( gradients );
  // And the channels that are going to be used are:
  unsigned int* indicator = new unsigned int[gradients];
  for( unsigned int p=0; p<gradients; ++p )
    indicator[p] = positions[p];
  dti->SetDWIChannels( indicator );
  delete[] indicator;
  //==========================================================================




  try
    {
    //dti->Update();
    }
  catch ( itk::ExceptionObject & e )
    {
    std::cerr << "exception in file writer " << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    return EXIT_FAILURE;
    }

  typedef float                                       SPixelType;
  typedef itk::Image<SPixelType,3>                    SImageType;
  typedef itk::ComputeFAFilter<TImageType,SImageType> FAType;
  FAType::Pointer fa = FAType::New();
  fa->SetInput( dti->GetOutput() );
  //fa->Update();
*/

  // Generate test image
  itk::ImageFileWriter<ImageType>::Pointer writer = itk::ImageFileWriter<ImageType>::New();
  writer->SetInput( filter->GetOutput() );
  writer->SetFileName(av[2]);
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "exception in file writer " << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
