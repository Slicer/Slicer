/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"

int vtkMRMLBSplineTransformNodeTest1(int, char*[])
{
  vtkNew<vtkMRMLBSplineTransformNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  return EXIT_SUCCESS;
}
