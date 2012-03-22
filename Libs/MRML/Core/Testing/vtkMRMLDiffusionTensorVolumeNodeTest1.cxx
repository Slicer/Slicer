/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDiffusionTensorVolumeNode.h"

#include <vtkPolyData.h>


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLDiffusionTensorVolumeNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLDiffusionTensorVolumeNode > node1 = vtkSmartPointer< vtkMRMLDiffusionTensorVolumeNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLDiffusionTensorVolumeNode, node1);
  
  return EXIT_SUCCESS;
}
