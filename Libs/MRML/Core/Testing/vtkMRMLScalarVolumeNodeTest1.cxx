/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLScalarVolumeNode.h"
#include <vtkPolyData.h>


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLScalarVolumeNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLScalarVolumeNode > node1 = vtkSmartPointer< vtkMRMLScalarVolumeNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLScalarVolumeNode, node1);

  return EXIT_SUCCESS;
}
