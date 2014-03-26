/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkGeneralTransform.h>

//---------------------------------------------------------------------------
int vtkMRMLLinearTransformNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLLinearTransformNode > node1 = vtkSmartPointer< vtkMRMLLinearTransformNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORM_MRML_METHODS(vtkMRMLLinearTransformNode, node1);

  return EXIT_SUCCESS;
}
