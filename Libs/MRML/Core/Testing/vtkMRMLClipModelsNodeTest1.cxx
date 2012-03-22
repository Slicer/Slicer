/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLClipModelsNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLClipModelsNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLClipModelsNode > node1 = vtkSmartPointer< vtkMRMLClipModelsNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLClipModelsNode, node1);
  
  return EXIT_SUCCESS;
}
