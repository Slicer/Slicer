/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLNonlinearTransformNode.h"

#include <vtkGeneralTransform.h>


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLNonlinearTransformNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLNonlinearTransformNode > node1 = vtkSmartPointer< vtkMRMLNonlinearTransformNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORM_MRML_METHODS(vtkMRMLNonlinearTransformNode, node1);

  return EXIT_SUCCESS;
}
