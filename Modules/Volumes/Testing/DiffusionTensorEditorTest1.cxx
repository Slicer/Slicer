/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLScene.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int DiffusionTensorEditorTest1(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer< vtkMRMLScene > scene = vtkSmartPointer< vtkMRMLScene >::New();

  EXERCISE_BASIC_OBJECT_METHODS( scene );

  scene->SetURL( argv[1] );
  scene->Connect();

  vtkCollection * collection = scene->GetCurrentScene();

  std::cout << "Collection GetNumberOfItems() = ";
  std::cout << collection->GetNumberOfItems() << std::endl;

  std::cout << "List of Node Names in this Scene" << std::endl;
  vtkMRMLNode * nodePtr = scene->GetNextNode();
  while( nodePtr != NULL )
    {
    std::cout << nodePtr->GetName() << std::endl;
    nodePtr = scene->GetNextNode();
    }

  return EXIT_SUCCESS;
}
