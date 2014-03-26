/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLVectorVolumeNode.h"
#include <vtkPolyData.h>


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLVectorVolumeNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLVectorVolumeNode > node1 = vtkSmartPointer< vtkMRMLVectorVolumeNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLVectorVolumeNode, node1);

  return EXIT_SUCCESS;
}
