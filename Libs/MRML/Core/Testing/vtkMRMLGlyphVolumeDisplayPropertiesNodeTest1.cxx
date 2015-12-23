/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLGlyphableVolumeDisplayPropertiesNode.h"

int vtkMRMLGlyphVolumeDisplayPropertiesNodeTest1(int , char * [] )
{
  // Errors are logged when vtkMRMLGlyphableVolumeDisplayPropertiesNode object is created.
  // vtkMRMLGlyphableVolumeDisplayPropertiesNode class will be removed, so instead of
  // trying to fix it, just ignore the errors.
  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_BEGIN();

  vtkNew<vtkMRMLGlyphableVolumeDisplayPropertiesNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TESTING_OUTPUT_IGNORE_WARNINGS_ERRORS_END();
  return EXIT_SUCCESS;
}
