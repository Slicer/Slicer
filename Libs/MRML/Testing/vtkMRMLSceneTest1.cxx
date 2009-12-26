/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLScene.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLSceneTest1(int argc, char * argv [] )
{
  vtkSmartPointer< vtkMRMLScene > scene1 = vtkSmartPointer< vtkMRMLScene >::New();

  EXERCISE_BASIC_OBJECT_METHODS( scene1 );

  return EXIT_SUCCESS;
}
