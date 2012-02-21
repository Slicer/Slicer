/*=========================================================================

  Program:   Extract Skeleton
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"

#include "ExtractSkeletonCLP.h"

#include "SkelGraph.h"
#include "tilg_iso_3D.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

} // end of anonymous namespace

/** Main command */
int main(int argc, char * *argv)
{
  PARSE_ARGS;
  std::cout << "Processed args." << std::endl;

  try
    {

    typedef unsigned char                        InputPixelType;
    typedef itk::Image<InputPixelType, 3>        InputImageType;
    typedef itk::ImageFileReader<InputImageType> ReaderType;

    typedef unsigned char                         OutputPixelType;
    typedef itk::Image<OutputPixelType, 3>        OutputImageType;
    typedef itk::ImageFileWriter<OutputImageType> WriterType;

    ReaderType::Pointer Reader = ReaderType::New();
    Reader->SetFileName(InputImageFileName.c_str() );
    Reader->Update();
    InputImageType::Pointer inputImage = Reader->GetOutput();

    std::cout << "Read image." << std::endl;

    OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetRegions(Reader->GetOutput()->GetLargestPossibleRegion() );
    outputImage->SetSpacing(Reader->GetOutput()->GetSpacing() );
    outputImage->SetOrigin(Reader->GetOutput()->GetOrigin() );
    outputImage->SetDirection(Reader->GetOutput()->GetDirection() );
    outputImage->Allocate();

    itk::Size<3> itkSize = inputImage->GetLargestPossibleRegion().GetSize();
    int          dim[3];
    dim[0] = itkSize[0];
    dim[1] = itkSize[1];
    dim[2] = itkSize[2];

    InputPixelType * inputImageBuffer = inputImage->GetBufferPointer();

    OutputPixelType * outputImageBuffer = outputImage->GetBufferPointer();
    memset(outputImageBuffer, 0, dim[0] * dim[1] * dim[2] * sizeof(OutputPixelType) );

    std::cout << "Initialized output image." << std::endl;

    int extract2DSheet = 0;
    if( SkeletonType == "2D" )
      {
      extract2DSheet = 1;
      }
    tilg_iso_3D(dim[0], dim[1], dim[2],
                inputImageBuffer, outputImageBuffer, extract2DSheet);
    std::cout << "Extracted skeleton." << std::endl;

    std::list<point> axisPoints;

    SkelGraph * graph = new SkelGraph();
    graph->Extract_skel_graph(outputImageBuffer, dim);
    graph->Extract_max_axis_in_graph();
    graph->Sample_along_axis(NumberOfPoints, &axisPoints);

    std::ofstream writeOutputFile;
    writeOutputFile.open(OutputPointsFileName.c_str() );

    if( !DontPruneBranches )
      {
      memset(outputImageBuffer, 0,
             dim[0] * dim[1] * dim[2] * sizeof(OutputPixelType) );
      }

    int                        i = 0;
    std::list<point>::iterator iter = axisPoints.begin();
    while( iter != axisPoints.end() )
      {
      OutputImageType::IndexType pt;
      pt[0] = iter->x;
      pt[1] = iter->y;
      pt[2] = iter->z;

      if( !DontPruneBranches )
        {
        outputImage->SetPixel(pt, 255);
        }

      writeOutputFile << i << " " << iter->x
                      << " " << iter->y
                      << " " << iter->z << std::endl;
      iter++;
      i++;
      }

    std::cout << "Wrote points file." << std::endl;

    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(OutputImageFileName.c_str() );
    writer->SetInput(outputImage);
    writer->Update();
    std::cout << "Wrote output image." << std::endl;

    }
  catch( itk::ExceptionObject & excep )
    {

    std::cerr << argv[0] << " : Exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;

    }

  return EXIT_SUCCESS;
}
