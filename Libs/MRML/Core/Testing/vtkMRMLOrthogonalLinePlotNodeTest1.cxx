/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLOrthogonalLinePlotNode.h"

// VTK includes
#include <vtkNew.h>

int vtkMRMLOrthogonalLinePlotNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLOrthogonalLinePlotNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS( node1.GetPointer() );

  return EXIT_SUCCESS;
}
