/*=========================================================================

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageDuplicator.h"

#include "ImageLabelCombineCLP.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

} // end of anonymous namespace

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  typedef short                               PixelType;
  typedef itk::Image<PixelType, 3>            ImageType;
  typedef itk::ImageFileReader<ImageType>     ReaderType;
  typedef itk::ImageFileWriter<ImageType>     WriterType;
  typedef itk::ImageRegionIterator<ImageType> IteratorType;
  typedef itk::ImageDuplicator<ImageType>     DuplicatorType;

  ReaderType::Pointer     readerA = ReaderType::New();
  ReaderType::Pointer     readerB = ReaderType::New();
  WriterType::Pointer     writer = WriterType::New();
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  ImageType::Pointer      input, output;

  readerA->SetFileName(InputLabelMap_A.c_str() );
  readerB->SetFileName(InputLabelMap_B.c_str() );
  writer->SetFileName(OutputLabelMap.c_str() );

  try
    {
    readerA->Update();
    readerB->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "Failed to read input images. Exception: " << e << std::endl;
    return EXIT_FAILURE;
    }

  if( FirstOverwrites )
    {
    input = readerA->GetOutput();
    duplicator->SetInputImage(readerB->GetOutput() );
    duplicator->Update();
    output = duplicator->GetOutput();
    }
  else
    {
    input = readerB->GetOutput();
    duplicator->SetInputImage(readerA->GetOutput() );
    duplicator->Update();
    output = duplicator->GetOutput();
    }

  // Check dimensions equality
  unsigned int inputDim = input->GetImageDimension();
  unsigned int outputDim = output->GetImageDimension();
  if (inputDim != outputDim)
    {
    std::cerr << "Input images dimensionality is not be the same" << std::endl;
    return EXIT_FAILURE;
    }
  const ImageType::SizeType inputSize = input->GetLargestPossibleRegion().GetSize();
  const ImageType::SizeType outputSize = output->GetLargestPossibleRegion().GetSize();
  bool dimCheck = true;
  for (unsigned int i=0; i<inputDim; i++)
    {
    dimCheck = dimCheck && (inputSize[i] == outputSize[i]);
    }
  if (!dimCheck)
    {
    std::cerr << "Input images dimensions are not be the same" << std::endl;
    return EXIT_FAILURE;
    }

  // This module operates on the image pixels. The images are expected to be
  // coinciding in the voxel space.
  IteratorType itIn(input, input->GetLargestPossibleRegion() );
  IteratorType itOut(output, output->GetLargestPossibleRegion() );
  itIn.GoToBegin();
  itOut.GoToBegin();
  for( ; !itIn.IsAtEnd(); ++itIn, ++itOut )
    {
    if( !itIn.Get() )
      {
      continue;
      }
    itOut.Set(itIn.Get() );
    }

  writer->SetInput(output);
  writer->SetUseCompression(true);
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cerr << "Failed to write output. Exception: " << e << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
