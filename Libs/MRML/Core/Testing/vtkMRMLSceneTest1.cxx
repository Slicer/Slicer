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
  vtkSmartPointer< vtkMRMLScene > scene1 = vtkSmartPointer< vtkMRMLScene >::New();

  EXERCISE_BASIC_OBJECT_METHODS( scene1 );

  std::cout << "GetNumberOfRegisteredNodeClasses() = ";
  std::cout << scene1->GetNumberOfRegisteredNodeClasses() << std::endl;

  std::cout << "GetNodeClasses() = " << std::endl;
  std::cout << scene1->GetNodeClasses() << std::endl;

  TEST_SET_GET_STRING( scene1, URL );
  TEST_SET_GET_STRING( scene1, RootDirectory );

  scene1->ResetNodes();

  return EXIT_SUCCESS;
}
