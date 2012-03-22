/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLTimeSeriesVolumeNode.h"

#include <vtkPolyData.h>


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLTimeSeriesVolumeNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLTimeSeriesVolumeNode > node1 = vtkSmartPointer< vtkMRMLTimeSeriesVolumeNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLTimeSeriesVolumeNode, node1);
  
  return EXIT_SUCCESS;
}
