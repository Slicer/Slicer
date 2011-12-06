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
#include "itkUNLMFilter.h"
#include "itkVector.h"
#include <string.h>

int main( int ac, char* av[] )
{
  if( ac < 3 )
    {
    std::cerr << "Usage: " << av[0]
              << " Input Output [sigma=0.0 radiusSearch=3 radiusZ=1 radiusComp=1 numNeighbours=1 h=1.2*sigma ]\n";
    return EXIT_FAILURE;
    }

  typedef unsigned short                        PixelType;
  typedef itk::VectorImage<PixelType, 3>        ImageType;
  typedef itk::UNLMFilter<ImageType, ImageType> FilterType;

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
  float sigma = 0.0f;
  if( ac > 3 )
    {
    sigma = ::atof( av[3] );
    }
  filter->SetSigma( sigma );

  unsigned int rads = 3;
  if( ac > 4 )
    {
    rads = ::atoi( av[4] );
    }
  filter->SetRSearch( rads );

  unsigned int radz = 1;
  if( ac > 5 )
    {
    rads = ::atoi( av[5] );
    }
  filter->SetRZ( radz );

  unsigned int radc = 1;
  if( ac > 6 )
    {
    radc = ::atoi( av[6] );
    }
  filter->SetRComp( radc );

  unsigned int neighbours = 1;
  if( ac > 7 )
    {
    neighbours = ::atoi( av[7] );
    }
  filter->SetNeighbours( neighbours );

  float hpar = 1.2 * sigma;
  if( ac > 8 )
    {
    hpar = ::atof( av[8] );
    }
  filter->SetH( hpar );

  // ==========================================================================

  // ==========================================================================
  // Set the parameters to the filter:
  // ==========================================================================
  // ==========================================================================
  // Parse the input file to obtain the gradient directions:
  FilterType::GradientType  grad;
  unsigned int              counter = 0;
  const unsigned int        LENGTH = 80;
  char                      buffer[LENGTH];
  FILE*                     fid = fopen( av[1], "r" );
  char*                     aux;
  char*                     aux2;
  char                      number[100];
  bool                      control = true;
  unsigned int              nDWI = 0;
  unsigned int              nBaselines = 0;
  std::vector<unsigned int> pDWI;
  std::vector<unsigned int> pBaselines;
  while(   control   )  // Read until EOF
    {
    if(   fgets( (char *)buffer, LENGTH, fid ) == (char *)NULL   )
      {
      control = false;
      break;
      }
    if( !strncmp(buffer, "DWMRI_gradient_", 15) )
      {
      // Find the first blank space:
      aux = (char *)buffer + 15 * sizeof(char);
      while( *aux != '=' )
        {
        if( *aux == '\0' )
          {
          std::cerr << "Cannot determine all the values of the gradients!!!" << std::endl;
          exit(-1);
          }
        aux += sizeof(char);
        }

      aux += sizeof(char);
      // Find the second blank space:
      aux2 = aux;                                // Initiallise aux2
      while( *aux2 == ' ' )
        {
        aux2 += sizeof(char);
        }                                        // Avoid multiple spaces

      while( *aux2 != ' ' )
        {
        if( *aux2 == '\0' )
          {
          std::cerr << "Cannot determine all the values of the gradients!!!" << std::endl;
          exit(-1);
          }
        aux2 += sizeof(char);
        }

      // Copy the number into the number buffer:
      unsigned int l = 0;
      while( aux < aux2 )
        {
        number[l] = *aux;
        aux += sizeof(char);
        l++;
        }

      number[l] = '\0';
      grad[0] = (float)::atof( number );
      // Find the third blank space:
      while( *aux2 == ' ' )
        {
        aux2 += sizeof(char);
        }                                        // Avoid multiple spaces

      while( *aux2 != ' ' )
        {
        if( *aux2 == '\0' )
          {
          std::cerr << "Cannot determine all the values of the gradients!!!" << std::endl;
          exit(-1);
          }
        aux2 += sizeof(char);
        }

      // Copy the number into the number buffer:
      l = 0;
      while( aux < aux2 )
        {
        number[l] = *aux;
        aux += sizeof(char);
        l++;
        }

      number[l] = '\0';
      grad[1] = (float)::atof( number );
      // Extract the last number:
      grad[2] = (float)::atof( aux );
      // Extract the norm:
      double norm = sqrt( grad[0] * grad[0] + grad[1] * grad[1] + grad[2] * grad[2] );
      if( norm > 1e-3 )
        {
        std::cout << "Adding gradient direction: " << grad << " --> ";
        grad[0] /= norm;
        grad[1] /= norm;
        grad[2] /= norm;
        std::cout << grad << std::endl;
        // One more gradient!!!
        ++nDWI;
        // Add the new gradient direction:
        filter->AddGradientDirection( grad );
        pDWI.push_back( counter );
        }
      else
        {
        std::cout << "Adding baseline: " << grad << std::endl;
        // One more baseline!!!
        ++nBaselines;
        pBaselines.push_back( counter );
        }
      ++counter;
      }
    }

  fclose( fid );

  // So the final number of channels is:
  filter->SetNDWI( nDWI );
  filter->SetNBaselines( nBaselines );

  // And the channels that are going to be used are:
  unsigned int* indicator = new unsigned int[nDWI];
  for( unsigned int p = 0; p < nDWI; ++p )
    {
    indicator[p] = pDWI[p];
    }
  filter->SetDWI( indicator );
  delete[] indicator;

  indicator = new unsigned int[nBaselines];
  for( unsigned int p = 0; p < nBaselines; ++p )
    {
    indicator[p] = pBaselines[p];
    }
  filter->SetBaselines( indicator );
  delete[] indicator;
  // ==========================================================================

  try
    {
    filter->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "exception in UNLM filter" << std::endl;
    std::cerr << e.GetDescription() << std::endl;
    std::cerr << e.GetLocation() << std::endl;
    return EXIT_FAILURE;
    }

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
