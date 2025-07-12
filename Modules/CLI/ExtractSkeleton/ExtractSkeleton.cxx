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

// MRML includes
#include <vtkMRMLNode.h>

// VTK includes
#include <vtkNew.h>

// Markups includes
#include <vtkMRMLMarkupsCurveNode.h>
#include <vtkMRMLMarkupsJsonStorageNode.h>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

} // end of anonymous namespace

/** Main command */
int main(int argc, char** argv)
{
  PARSE_ARGS;
  std::cout << "Processed args." << std::endl;

  try
  {

    if (!OutputCurveFileName.empty())
    {
      if (FullTree)
      {
        std::cerr << "Output curve can only be computed if fullTree option is disabled" << std::endl;
        return EXIT_FAILURE;
      }
      if (SkeletonType != "1D")
      {
        std::cerr << "Output curve can only be computed if skeleton type is 1D" << std::endl;
        return EXIT_FAILURE;
      }
    }

    typedef unsigned char InputPixelType;
    typedef itk::Image<InputPixelType, 3> InputImageType;
    typedef itk::ImageFileReader<InputImageType> ReaderType;

    typedef unsigned char OutputPixelType;
    typedef itk::Image<OutputPixelType, 3> OutputImageType;
    typedef itk::ImageFileWriter<OutputImageType> WriterType;

    typedef OutputImageType::PointType OutputPointType;
    typedef OutputImageType::IndexType OutputIndexType;

    ReaderType::Pointer Reader = ReaderType::New();
    Reader->SetFileName(InputImageFileName.c_str());
    Reader->Update();
    InputImageType::Pointer inputImage = Reader->GetOutput();

    std::cout << "Read image." << std::endl;

    OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetRegions(Reader->GetOutput()->GetLargestPossibleRegion());
    outputImage->SetSpacing(Reader->GetOutput()->GetSpacing());
    outputImage->SetOrigin(Reader->GetOutput()->GetOrigin());
    outputImage->SetDirection(Reader->GetOutput()->GetDirection());
    outputImage->Allocate();

    itk::Size<3> itkSize = inputImage->GetLargestPossibleRegion().GetSize();
    int dim[3];
    dim[0] = itkSize[0];
    dim[1] = itkSize[1];
    dim[2] = itkSize[2];

    InputPixelType* inputImageBuffer = inputImage->GetBufferPointer();

    OutputPixelType* outputImageBuffer = outputImage->GetBufferPointer();
    memset(outputImageBuffer, 0, dim[0] * dim[1] * dim[2] * sizeof(OutputPixelType));

    std::cout << "Initialized output image." << std::endl;

    int extract2DSheet = 0;
    if (SkeletonType == "2D")
    {
      extract2DSheet = 1;
    }
    tilg_iso_3D(dim[0], dim[1], dim[2], inputImageBuffer, outputImageBuffer, extract2DSheet);
    std::cout << "Extracted skeleton." << std::endl;

    SkelGraph graph;
    auto spacingVector = Reader->GetOutput()->GetSpacing();
    double spacing[3] = { spacingVector[0], spacingVector[1], spacingVector[2] };
    graph.ExtractSkeletalGraph(outputImageBuffer, dim, spacing);
    graph.FindMaximalPath();
    std::deque<Coord3i> axisPoints;
    graph.SampleAlongMaximalPath(NumberOfPoints, axisPoints);

    std::ofstream writeOutputFile;
    bool writeSeedsFile = !OutputPointsFileName.empty();
    if (writeSeedsFile)
    {
      writeOutputFile.open(OutputPointsFileName.c_str());
    }

    vtkNew<vtkMRMLMarkupsCurveNode> curveNode;
    curveNode->SetName("C");

    if (FullTree)
    {
      memset(outputImageBuffer, 0, dim[0] * dim[1] * dim[2] * sizeof(OutputPixelType));
    }

    OutputPointType position_LPS;
    OutputIndexType position_IJK;

    int i = 0;
    std::deque<Coord3i>::iterator iter = axisPoints.begin();
    while (iter != axisPoints.end())
    {
      position_IJK[0] = (*iter)[0];
      position_IJK[1] = (*iter)[1];
      position_IJK[2] = (*iter)[2];

      if (FullTree)
      {
        outputImage->SetPixel(position_IJK, 255);
      }

      if (writeSeedsFile)
      {
        writeOutputFile << i << " " << (*iter)[0] << " " << (*iter)[1] << " " << (*iter)[2] << std::endl;
      }

      outputImage->TransformIndexToPhysicalPoint(position_IJK, position_LPS);
      // first two coordinates are inverted because MRML is always in RAS coordinate system
      curveNode->AddControlPointWorld(vtkVector3d(-position_LPS[0], -position_LPS[1], position_LPS[2]));

      iter++;
      i++;
    }

    if (writeSeedsFile)
    {
      std::cout << "Wrote points file." << std::endl;
    }

    if (!OutputCurveFileName.empty())
    {
      vtkNew<vtkMRMLMarkupsJsonStorageNode> outputCurveStorageNode;
      outputCurveStorageNode->SetFileName(OutputCurveFileName.c_str());
      outputCurveStorageNode->UseLPSOn();
      outputCurveStorageNode->WriteData(curveNode.GetPointer());
      std::cout << "Wrote output curve." << std::endl;
    }

    if (!OutputImageFileName.empty())
    {
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName(OutputImageFileName.c_str());
      writer->SetInput(outputImage);
      writer->Update();
      std::cout << "Wrote output image." << std::endl;
    }
  }
  catch (itk::ExceptionObject& excep)
  {

    std::cerr << argv[0] << " : Exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
