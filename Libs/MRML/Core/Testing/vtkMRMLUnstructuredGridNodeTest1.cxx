/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLUnstructuredGridNode.h"

// VTK includes
#include <vtkPolyData.h>

// STD includes

int vtkMRMLUnstructuredGridNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLUnstructuredGridNode > node1 = vtkSmartPointer< vtkMRMLUnstructuredGridNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLUnstructuredGridNode, node1);

  return EXIT_SUCCESS;
}
