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
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkDecimatePro.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkReverseSense.h"
#include "vtkPolyDataNormals.h"
#include "vtkStripper.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkImageChangeInformation.h"

#include "vtkPluginFilterWatcher.h"
#include "ModuleDescriptionParser.h"
#include "ModuleDescription.h"
#include "vtkDebugLeaks.h"
#include <vtkVersion.h>

int main(int argc, char * argv[])
{
  PARSE_ARGS;
  vtkDebugLeaks::SetExitError(true);

  ModuleDescription       module;
  ModuleDescriptionParser parser;
  if( parser.Parse(GetXMLModuleDescription(), module) )
    {
    std::cerr << argv[0] << ": One or more XML errors detected." << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Module Description Information" << std::endl;
  std::cout << "\tCategory is: " << module.GetCategory() << std::endl;
  std::cout << "\tTitle is: " << module.GetTitle() << std::endl;
  std::cout << "\tDescription is: " << module.GetDescription() << std::endl;
  std::cout << "\tVersion is: " << module.GetVersion() << std::endl;
  std::cout << "\tDocumentationURL is: " << module.GetDocumentationURL() << std::endl;
  std::cout << "\tLicense is: " << module.GetLicense() << std::endl;
  std::cout << "\tContributor is: " << module.GetContributor() << std::endl;
  std::cout << "\tAcknowledgements: " << module.GetAcknowledgements() << std::endl;

  bool debug = false;

  if( debug )
    {
    std::cout << "The input volume is: " << InputVolume << std::endl;
    std::cout << "The output geometry is: " << OutputGeometry << std::endl;
    std::cout << "The threshold is: " << Threshold << std::endl;
    }

  // vtk and helper variables
  vtkITKArchetypeImageSeriesReader* reader = NULL;
  vtkImageData *                    image;
  vtkWindowedSincPolyDataFilter *   smootherSinc = NULL;
  vtkDecimatePro *                  decimator = NULL;
  vtkMarchingCubes *                mcubes = NULL;
  vtkTransform *                    transformIJKtoRAS = NULL;
  vtkReverseSense *                 reverser = NULL;
  vtkTransformPolyDataFilter *      transformer = NULL;
  vtkPolyDataNormals *              normals = NULL;
  vtkStripper *                     stripper = NULL;
  vtkXMLPolyDataWriter *            writer = NULL;

  // check for the input file
  // - strings that start with slicer: are shared memory references, so they won't exist.
  //   The memory address starts with 0x in linux but not on Windows
  if( InputVolume.find(std::string("slicer:") ) != 0 )
    {
    // check for the input file
    FILE * infile;
    infile = fopen(InputVolume.c_str(), "r");
    if( infile == NULL )
      {
      std::cerr << "ERROR: cannot open input volume file " << InputVolume << endl;
      return EXIT_FAILURE;
      }
    fclose(infile);
    }

  // Read the file
  reader = vtkITKArchetypeImageSeriesScalarReader::New();
  reader->SetArchetype(InputVolume.c_str() );
  reader->SetOutputScalarTypeToNative();
  reader->SetDesiredCoordinateOrientationToNative();
  reader->SetUseNativeOriginOn();
  reader->Update();

  std::cout << "Done reading the file " << InputVolume << endl;

  vtkImageChangeInformation *ici = vtkImageChangeInformation::New();
#if (VTK_MAJOR_VERSION <= 5)
  ici->SetInput(reader->GetOutput() );
#else
  ici->SetInputConnection(reader->GetOutputPort() );
#endif
  ici->SetOutputSpacing( 1, 1, 1 );
  ici->SetOutputOrigin( 0, 0, 0 );
  ici->Update();

#if (VTK_MAJOR_VERSION <= 5)
  image = ici->GetOutput();
  image->Update();
#else
  ici->Update();
  image = ici->GetOutput();
#endif

  // Get the dimensions, marching cubes only works on 3d
  int extents[6];
  image->GetExtent(extents);
  if( debug )
    {
    std::cout << "Image data extents: " << extents[0] << " " << extents[1] << " " << extents[2] << " " << extents[3]
              << " " << extents[4] << " " << extents[5] << endl;
    }
  if( extents[0] == extents[1] ||
      extents[2] == extents[3] ||
      extents[4] == extents[5] )
    {
    std::cerr << "The volume is not 3D.\n";
    std::cerr << "\tImage data extents: " << extents[0] << " " << extents[1] << " " << extents[2] << " "
              << extents[3] << " " << extents[4] << " " << extents[5] << endl;
    return EXIT_FAILURE;
    }
  // Get the RAS to IJK matrix and invert it to get the IJK to RAS which will need
  // to be applied to the model as it will be built in pixel space

  transformIJKtoRAS = vtkTransform::New();
  transformIJKtoRAS->SetMatrix(reader->GetRasToIjkMatrix() );
  if( debug )
    {
    std::cout << "RasToIjk matrix from file = ";
    transformIJKtoRAS->GetMatrix()->Print(std::cout);
    }
  transformIJKtoRAS->Inverse();
  mcubes = vtkMarchingCubes::New();
  vtkPluginFilterWatcher watchMCubes(mcubes,
                                     "Marching Cubes",
                                     CLPProcessInformation,
                                     1.0 / 7.0, 0.0);

#if (VTK_MAJOR_VERSION <= 5)
  mcubes->SetInput(ici->GetOutput() );
#else
  mcubes->SetInputConnection(ici->GetOutputPort() );
#endif
  mcubes->SetValue(0, Threshold);
  mcubes->ComputeScalarsOff();
  mcubes->ComputeGradientsOff();
  mcubes->ComputeNormalsOff();
#if (VTK_MAJOR_VERSION <= 5)
  (mcubes->GetOutput() )->ReleaseDataFlagOn();
#else
  mcubes->ReleaseDataFlagOn();
#endif
  mcubes->Update();

  if( debug )
    {
    std::cout << "Number of polygons = " << (mcubes->GetOutput() )->GetNumberOfPolys() << endl;
    }

  // TODO: look at vtkQuadraticDecimation
  decimator = vtkDecimatePro::New();
  vtkPluginFilterWatcher watchDecimator(decimator,
                                        "Decimator",
                                        CLPProcessInformation,
                                        1.0 / 7.0, 1.0 / 7.0);
#if (VTK_MAJOR_VERSION <= 5)
  decimator->SetInput(mcubes->GetOutput() );
#else
  decimator->SetInputConnection(mcubes->GetOutputPort() );
#endif
  decimator->SetFeatureAngle(60);
  decimator->SplittingOff();
  decimator->PreserveTopologyOn();

  decimator->SetMaximumError(1);
  decimator->SetTargetReduction(Decimate);
#if (VTK_MAJOR_VERSION <= 5)
  (decimator->GetOutput() )->ReleaseDataFlagOff();
#else
  decimator->ReleaseDataFlagOff();
#endif


  std::cout << "Decimating ... \n";
  // TODO add progress to decimator
  decimator->Update();
  if( debug )
    {
    std::cout << "After decimation, number of polygons = " << (decimator->GetOutput() )->GetNumberOfPolys() << endl;
    }

  if( (transformIJKtoRAS->GetMatrix() )->Determinant() < 0 )
    {
    if( debug )
      {
      std::cout << "Determinant " << (transformIJKtoRAS->GetMatrix() )->Determinant()
                << " is less than zero, reversing...\n";
      }
    reverser = vtkReverseSense::New();
    vtkPluginFilterWatcher watchReverser(reverser,
                                         "Reversor",
                                         CLPProcessInformation,
                                         1.0 / 7.0, 2.0 / 7.0);
#if (VTK_MAJOR_VERSION <= 5)
    reverser->SetInput(decimator->GetOutput() );
#else
    reverser->SetInputConnection(decimator->GetOutputPort() );
#endif
    reverser->ReverseNormalsOn();
#if (VTK_MAJOR_VERSION <= 5)
    (reverser->GetOutput() )->ReleaseDataFlagOn();
#else
    reverser->ReleaseDataFlagOn();
#endif
    // TODO: add progress
    }

  smootherSinc = vtkWindowedSincPolyDataFilter::New();
  vtkPluginFilterWatcher watchSmoother(smootherSinc,
                                       "Smoother",
                                       CLPProcessInformation,
                                       1.0 / 7.0, 3.0 / 7.0);
  smootherSinc->SetPassBand(0.1);
  if( Smooth == 1 )
    {
    std::cerr << "Warning: Smoothing iterations of 1 not allowed for Sinc filter, using 2" << endl;
    Smooth = 2;
    }
  if( (transformIJKtoRAS->GetMatrix() )->Determinant() < 0 )
    {
#if (VTK_MAJOR_VERSION <= 5)
    smootherSinc->SetInput(reverser->GetOutput() );
#else
    smootherSinc->SetInputConnection(reverser->GetOutputPort() );
#endif
    }
  else
    {
#if (VTK_MAJOR_VERSION <= 5)
    smootherSinc->SetInput(decimator->GetOutput() );
#else
    smootherSinc->SetInputConnection(decimator->GetOutputPort() );
#endif
    }
  smootherSinc->SetNumberOfIterations(Smooth);
  smootherSinc->FeatureEdgeSmoothingOff();
  smootherSinc->BoundarySmoothingOff();
#if (VTK_MAJOR_VERSION <= 5)
  (smootherSinc->GetOutput() )->ReleaseDataFlagOn();
#else
  smootherSinc->ReleaseDataFlagOn();
#endif

  // TODO: insert progress
  std::cout << "Smoothing...\n";
  smootherSinc->Update();
  transformer = vtkTransformPolyDataFilter::New();
  vtkPluginFilterWatcher watchTranformer(transformer,
                                         "Transformer",
                                         CLPProcessInformation,
                                         1.0 / 7.0, 4.0 / 7.0);
#if (VTK_MAJOR_VERSION <= 5)
  transformer->SetInput(smootherSinc->GetOutput() );
#else
  transformer->SetInputConnection(smootherSinc->GetOutputPort() );
#endif
  if( (transformIJKtoRAS->GetMatrix() )->Determinant() < 0 )
    {
#if (VTK_MAJOR_VERSION <= 5)
    transformer->SetInput(reverser->GetOutput() );
#else
    transformer->SetInputConnection(reverser->GetOutputPort() );
#endif
    }
  else
    {
#if (VTK_MAJOR_VERSION <= 5)
    transformer->SetInput(decimator->GetOutput() );
#else
    transformer->SetInputConnection(decimator->GetOutputPort() );
#endif
    }

  transformer->SetTransform(transformIJKtoRAS);
  if( debug )
    {
    std::cout << "Transforming using inversed matrix:\n";
    transformIJKtoRAS->GetMatrix()->Print(std::cout);
    }

  // TODO: add progress
#if (VTK_MAJOR_VERSION <= 5)
  (transformer->GetOutput() )->ReleaseDataFlagOn();
#else
  transformer->ReleaseDataFlagOn();
#endif

  normals = vtkPolyDataNormals::New();
  vtkPluginFilterWatcher watchNormals(normals,
                                      "Normals",
                                      CLPProcessInformation,
                                      1.0 / 7.0, 5.0 / 7.0);
  if( PointNormals )
    {
    normals->ComputePointNormalsOn();
    }
  else
    {
    normals->ComputePointNormalsOff();
    }
#if (VTK_MAJOR_VERSION <= 5)
  normals->SetInput(transformer->GetOutput() );
#else
  normals->SetInputConnection(transformer->GetOutputPort() );
#endif
  normals->SetFeatureAngle(60);
  normals->SetSplitting(SplitNormals);
  std::cout << "Splitting normals...\n";
  // TODO: add progress
#if (VTK_MAJOR_VERSION <= 5)
  (normals->GetOutput() )->ReleaseDataFlagOn();
#else
  normals->ReleaseDataFlagOn();
#endif

  stripper = vtkStripper::New();
  vtkPluginFilterWatcher watchStripper(stripper,
                                       "Stripper",
                                       CLPProcessInformation,
                                       1.0 / 7.0, 6.0 / 7.0);
#if (VTK_MAJOR_VERSION <= 5)
  stripper->SetInput(normals->GetOutput() );
#else
  stripper->SetInputConnection(normals->GetOutputPort() );
#endif
  std::cout << "Stripping...\n";
  // TODO: add progress
#if (VTK_MAJOR_VERSION <= 5)
  (stripper->GetOutput() )->ReleaseDataFlagOff();
#else
  stripper->ReleaseDataFlagOff();
#endif


  // the poly data output from the stripper can be set as an input to a model's polydata
#if (VTK_MAJOR_VERSION <= 5)
  (stripper->GetOutput() )->Update();
#else
  stripper->Update();
#endif
  // but for now we're just going to write it out

  writer = vtkXMLPolyDataWriter::New();
#if (VTK_MAJOR_VERSION <= 5)
  writer->SetInput(stripper->GetOutput() );
#else
  writer->SetInputConnection(stripper->GetOutputPort() );
#endif
  writer->SetFileName(OutputGeometry.c_str() );
  // TODO: add progress
  writer->Write();

// Cleanup
  if( reader )
    {
    reader->Delete();
    }
  if( ici )
    {
    ici->Delete();
    }
  if( transformIJKtoRAS )
    {
    transformIJKtoRAS->Delete();
    }
  if( mcubes )
    {
    mcubes->Delete();
    }
  if( decimator )
    {
    decimator->Delete();
    }
  if( reverser )
    {
    reverser->Delete();
    }
  if( smootherSinc )
    {
    smootherSinc->Delete();
    }
  if( transformer )
    {
    transformer->Delete();
    }
  if( normals )
    {
    normals->Delete();
    }
  if( stripper )
    {
    stripper->Delete();
    }
  if( writer )
    {
    writer->Delete();
    }
  return EXIT_SUCCESS;
}
