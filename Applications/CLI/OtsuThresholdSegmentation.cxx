/*=========================================================================
  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: OtsuThresholdImageFilter.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/21 13:00:23 $
  Version:   $Revision: 1.10 $

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

#include "itkOtsuThresholdImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkMinimumMaximumImageFilter.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkXMLFilterWatcher.h"

#include "OtsuThresholdSegmentationCLP.h"

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

// Image Types
  typedef  short  InputPixelType;
  typedef  unsigned long  InternalPixelType;
  typedef  short  OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< InternalPixelType, 3 >   InternalImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

// Filter Types
  typedef itk::OtsuThresholdImageFilter<
    InputImageType, InputImageType >  OtsuFilterType;
  typedef itk::ConnectedComponentImageFilter<
    InputImageType, InternalImageType >  CCFilterType;
  typedef itk::RelabelComponentImageFilter<
    InternalImageType, OutputImageType > RelabelType;

// I/O Types
  typedef itk::ImageFileReader< InputImageType >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

// Instances
  ReaderType::Pointer reader = ReaderType::New();
  OtsuFilterType::Pointer OtsuFilter = OtsuFilterType::New();
  CCFilterType::Pointer CCFilter = CCFilterType::New();
  RelabelType::Pointer RelabelFilter = RelabelType::New();
  WriterType::Pointer writer = WriterType::New();

// Watchers
  itk::XMLFilterWatcher OtsuWatcher(OtsuFilter, "Otsu Threshold Image Filter");
  itk::XMLFilterWatcher CCWatcher(CCFilter, "Connected Component Threshold Image Filter");
  itk::XMLFilterWatcher RelabelWatcher(RelabelFilter, "Relabel objects");

  reader->SetFileName (inputVolume.c_str());

  OtsuFilter->SetInput( reader->GetOutput() );
  OtsuFilter->SetOutsideValue( outsideValue );
  OtsuFilter->SetInsideValue(  insideValue  );
  OtsuFilter->SetNumberOfHistogramBins( numberOfBins );

  CCFilter->SetInput (OtsuFilter->GetOutput());
  if (faceConnected)
    {
    CCFilter->FullyConnectedOn();
    }
  else
    {
    CCFilter->FullyConnectedOff();
    }
  RelabelFilter->SetInput (CCFilter->GetOutput());
  RelabelFilter->SetMinimumObjectSize(minimumObjectSize);

  writer->SetInput( RelabelFilter->GetOutput() );
  writer->SetFileName( outputVolume.c_str() );
  writer->Update();


  int threshold = OtsuFilter->GetThreshold();
  std::cout << "Threshold = " << threshold << std::endl;

  RelabelFilter->Print(std::cout);

  return EXIT_SUCCESS;
}

