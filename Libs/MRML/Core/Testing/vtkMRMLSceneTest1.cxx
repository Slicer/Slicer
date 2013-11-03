/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLScene.h"


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLSceneTest1(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene1;

  EXERCISE_BASIC_OBJECT_METHODS(scene1.GetPointer());

  std::cout << "GetNumberOfRegisteredNodeClasses() = ";
  std::cout << scene1->GetNumberOfRegisteredNodeClasses() << std::endl;

  TEST_SET_GET_STRING(scene1.GetPointer(), URL);
  TEST_SET_GET_STRING(scene1.GetPointer(), RootDirectory);

  scene1->ResetNodes();

  return EXIT_SUCCESS;
}
