/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// SlicerLib includes
#include "vtkArchive.h"

// VTK includes
#include <vtkObject.h>

// STD includes
#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkArchiveTest1(int argc, char * argv[] )
{
  if (argc < 2)
    {
    std::cerr << "Usage: vtkArchiveTest1 archive.zip" << std::endl;
    return EXIT_FAILURE;
    }
  bool res = extract_tar(argv[1], true, true);
  if (!res)
    {
    std::cerr << "failed to extract archive :" << argc << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

