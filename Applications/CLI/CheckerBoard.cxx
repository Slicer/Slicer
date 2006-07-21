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

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkXMLFilterWatcher.h"

#include "itkCheckerBoardImageFilter.h"

#include "CheckerBoardCLP.h"

int main( int argc, char * argv[] )
{
  typedef   short  InputPixelType;
  typedef   short  OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

  typedef itk::ImageFileReader< InputImageType  >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  PARSE_ARGS;

  ReaderType::Pointer reader1 = ReaderType::New();
  ReaderType::Pointer reader2 = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader1->SetFileName( inputVolume1.c_str() );
  reader2->SetFileName( inputVolume2.c_str() );
  writer->SetFileName( outputVolume.c_str() );

  typedef itk::CheckerBoardImageFilter<
               InputImageType>  FilterType;

  FilterType::Pointer filter = FilterType::New();
  itk::XMLFilterWatcher watcher(filter, "CheckerBoard Image Filter");

  FilterType::PatternArrayType pattern;
  
  pattern[0] = checkerPattern[0];
  pattern[1] = checkerPattern[1];
  pattern[2] = checkerPattern[2];

  filter->SetCheckerPattern(pattern);
  filter->SetInput1( reader1->GetOutput() );
  filter->SetInput2( reader2->GetOutput() );

  writer->SetInput( filter->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

