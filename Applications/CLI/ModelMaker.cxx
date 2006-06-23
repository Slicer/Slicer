/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile$
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

#include "ModelMakerCLP.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"

int main(int argc, char * argv[])
{
  PARSE_ARGS;
  std::cout << "The input volume is: " << InputVolume << std::endl;
  std::cout << "The output directory is: " << OutputDirectory << std::endl;

  vtkITKArchetypeImageSeriesReader* reader;
  reader = vtkITKArchetypeImageSeriesScalarReader::New();
  reader->SetArchetype(InputVolume.c_str());
  reader->SetOutputScalarTypeToNative();
  reader->SetDesiredCoordinateOrientationToNative();
  reader->SetUseNativeOriginOff();
  reader->Update();


  reader->Delete();
  return EXIT_SUCCESS;
}
