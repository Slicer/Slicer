/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile$
Date:      $Date$
Version:   $Revision$

=========================================================================auto=*/

#include "GrayscaleModelMakerCLP.h"

#include "vtkDecimatePro.h"
#include "vtkFieldData.h"
#include "vtkFlyingEdges3D.h"
#include "vtkImageChangeInformation.h"
#include "vtkImageData.h"
#include "vtkPolyDataNormals.h"
#include "vtkReverseSense.h"
#include "vtkStringArray.h"
#include "vtkStripper.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkXMLPolyDataWriter.h"

#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkPluginFilterWatcher.h"
#include "ModuleDescriptionParser.h"
#include "ModuleDescription.h"

int main(int argc, char* argv[])
{
  PARSE_ARGS;
  if (Debug)
    {
    std::cout << "The input volume is: " << InputVolume << std::endl;
    std::cout << "The output geometry is: " << OutputGeometry << std::endl;
    std::cout << "The threshold is: " << Threshold << std::endl;
    }

  // check for the input file
  // - strings that start with slicer: are shared memory references, so they won't exist.
  //   The memory address starts with 0x in linux but not on Windows
  if (InputVolume.find(std::string("slicer:")) != 0)
    {
    // check for the input file
    FILE* infile = fopen(InputVolume.c_str(), "r");
    if (infile == nullptr)
      {
      std::cerr << "ERROR: cannot open input volume file " << InputVolume << std::endl;
      return EXIT_FAILURE;
      }
    fclose(infile);
    }

  vtkSmartPointer<vtkImageData> image_IJK;
  vtkNew<vtkTransform> transformIJKtoLPS;
  {
    // Read the file
    vtkNew<vtkITKArchetypeImageSeriesScalarReader> reader;
    reader->SetArchetype(InputVolume.c_str());
    reader->SetOutputScalarTypeToNative();
    reader->SetDesiredCoordinateOrientationToNative();
    reader->SetUseNativeOriginOn();
    reader->Update();
    if (Debug)
      {
      std::cout << "Done reading the file " << InputVolume << endl;
      }
    vtkNew<vtkMatrix4x4> ijkToRasMatrix;
    vtkMatrix4x4::Invert(reader->GetRasToIjkMatrix(), ijkToRasMatrix);
    transformIJKtoLPS->Scale(-1.0, -1.0, 1.0); // RAS to LPS
    transformIJKtoLPS->Concatenate(ijkToRasMatrix);

    vtkNew<vtkImageChangeInformation> ici;
    ici->SetInputConnection(reader->GetOutputPort());
    ici->SetOutputSpacing(1, 1, 1);
    ici->SetOutputOrigin(0, 0, 0);
    ici->Update();
    image_IJK = ici->GetOutput();
  }

  // Get the dimensions, marching cubes only works on 3d
  int extents[6];
  image_IJK->GetExtent(extents);
  if (Debug)
    {
    std::cout << "Image data extents: " << extents[0] << " " << extents[1] << " " << extents[2] << " " << extents[3]
      << " " << extents[4] << " " << extents[5] << endl;
    }
  if (extents[0] >= extents[1] ||
    extents[2] >= extents[3] ||
    extents[4] >= extents[5])
    {
    std::cerr << "The volume is not 3D. Image file: " << InputVolume << ", extents: " << extents[0] << " " << extents[1] << " " << extents[2] << " "
      << extents[3] << " " << extents[4] << " " << extents[5] << endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkFlyingEdges3D> mcubes;
  vtkPluginFilterWatcher watchMCubes(mcubes, "Marching Cubes", CLPProcessInformation, 1.0 / 7.0, 0.0);
  mcubes->SetInputData(image_IJK);
  mcubes->SetValue(0, Threshold);
  mcubes->ComputeScalarsOff();
  mcubes->ComputeGradientsOff();
  mcubes->ComputeNormalsOff();
  if (Debug)
    {
    std::cout << "Number of polygons = " << (mcubes->GetOutput())->GetNumberOfPolys() << endl;
    }

  // Convert the mesh from voxel space to physical space before decimating or smoothing,
  // as they rely on actual size and aspect ratio of the mesh.
  vtkSmartPointer<vtkPolyData> mesh_LPS;
  {
    if (Debug)
      {
      std::cout << "Transforming to mesh to LPS coordinate system" << std::endl;
      std::cout << "IJK to LPS matrix from file = ";
      transformIJKtoLPS->GetMatrix()->Print(std::cout);
      }
    vtkNew<vtkTransformPolyDataFilter> transformer;
    vtkPluginFilterWatcher watchTranformer(transformer, "Transformer", CLPProcessInformation, 1.0 / 7.0, 4.0 / 7.0);
    transformer->SetInputConnection(mcubes->GetOutputPort());
    transformer->SetTransform(transformIJKtoLPS);
    transformer->Update();
    mesh_LPS = transformer->GetOutput();
  }

  if ((transformIJKtoLPS->GetMatrix())->Determinant() < 0)
    {
    if (Debug)
      {
      std::cout << "Determinant " << (transformIJKtoLPS->GetMatrix())->Determinant()
        << " is less than zero, reversing..." << std::endl;
      }
    vtkNew<vtkReverseSense> reverser;
    vtkPluginFilterWatcher watchReverser(reverser, "Reversor", CLPProcessInformation, 1.0 / 7.0, 2.0 / 7.0);
    reverser->SetInputData(mesh_LPS);
    reverser->ReverseNormalsOn();
    reverser->Update();
    mesh_LPS = reverser->GetOutput();
    }

  if (Decimate > 0)
    {
    if (Debug)
      {
      std::cout << "Decimating ... " << std::endl;
      }
    // TODO: look at vtkQuadraticDecimation, it produces nicer mesh
    vtkNew<vtkDecimatePro> decimator;
    vtkPluginFilterWatcher watchDecimator(decimator, "Decimator", CLPProcessInformation, 1.0 / 7.0, 1.0 / 7.0);
    decimator->SetInputData(mesh_LPS);
    decimator->SetFeatureAngle(60);
    decimator->SplittingOff();
    decimator->PreserveTopologyOn();
    decimator->SetMaximumError(1);
    decimator->SetTargetReduction(Decimate);
    // TODO add progress to decimator
    decimator->Update();
    if (Debug)
      {
      std::cout << "After decimation, number of polygons = " << (decimator->GetOutput())->GetNumberOfPolys() << endl;
      }
    mesh_LPS = decimator->GetOutput();
    }

  if (Smooth > 0)
    {
    if (Debug)
      {
      std::cout << "Smoothing..." << std::endl;
      }
    vtkNew<vtkWindowedSincPolyDataFilter> smootherSinc;
    vtkPluginFilterWatcher watchSmoother(smootherSinc, "Smoother", CLPProcessInformation, 1.0 / 7.0, 3.0 / 7.0);
    smootherSinc->SetPassBand(0.1);
    if (Smooth == 1)
      {
      std::cerr << "Warning: Smoothing iterations of 1 not allowed for Sinc filter, using 2" << endl;
      Smooth = 2;
      }
    smootherSinc->SetInputData(mesh_LPS);
    smootherSinc->SetNumberOfIterations(Smooth);
    smootherSinc->FeatureEdgeSmoothingOff();
    smootherSinc->BoundarySmoothingOff();
    smootherSinc->Update();
    mesh_LPS = smootherSinc->GetOutput();
    }

  if (Debug)
    {
    std::cout << "Computing normals..." << std::endl;
    }
  vtkNew<vtkPolyDataNormals> normals;
  vtkPluginFilterWatcher watchNormals(normals, "Normals", CLPProcessInformation, 1.0 / 7.0, 5.0 / 7.0);
  normals->SetComputePointNormals(PointNormals);
  normals->SetInputData(mesh_LPS);
  normals->SetFeatureAngle(60);
  normals->SetSplitting(SplitNormals);

  if (Debug)
    {
    std::cout << "Triangle stripping..." << std::endl;
    }
  vtkNew<vtkStripper> stripper;
  vtkPluginFilterWatcher watchStripper(stripper, "Stripper", CLPProcessInformation, 1.0 / 7.0, 6.0 / 7.0);
  stripper->SetInputConnection(normals->GetOutputPort());
  stripper->Update();
  vtkPolyData* meshToWrite = stripper->GetOutput();

  // Save coordinate system information in the output model
  vtkNew<vtkFieldData> fieldData;
  meshToWrite->SetFieldData(fieldData);
  vtkNew<vtkStringArray> coordinateSystemFieldArray;
  coordinateSystemFieldArray->SetName("SPACE");
  coordinateSystemFieldArray->InsertNextValue("LPS");
  fieldData->AddArray(coordinateSystemFieldArray);

  if (Debug)
    {
    std::cout << "Write result to file..." << std::endl;
    }
  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetInputData(meshToWrite);
  writer->SetFileName(OutputGeometry.c_str());
  // TODO: add progress
  writer->Write();

  if (Debug)
    {
    std::cout << "Done." << std::endl;
    }
  return EXIT_SUCCESS;
}
