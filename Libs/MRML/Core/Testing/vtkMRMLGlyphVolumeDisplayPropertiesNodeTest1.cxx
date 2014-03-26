/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLGlyphableVolumeDisplayPropertiesNode.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLGlyphVolumeDisplayPropertiesNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLGlyphableVolumeDisplayPropertiesNode > node1 = vtkSmartPointer< vtkMRMLGlyphableVolumeDisplayPropertiesNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_STORABLE_MRML_METHODS(vtkMRMLGlyphableVolumeDisplayPropertiesNode, node1);

  return EXIT_SUCCESS;
}
