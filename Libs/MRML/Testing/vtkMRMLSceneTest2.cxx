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

void printNode(vtkObject *vtkNotUsed(vtkcaller), unsigned long vtkNotUsed(eid), void *vtkNotUsed(clientdata), void *calldata)
{
  vtkObject* object = reinterpret_cast<vtkObject*>(calldata);
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(object);
  if (node == 0)
    {
    std::cout << "node invalid" << std::endl;
    return;
    }
  std::cout << "  " << node->GetName() << std::endl;
}

int vtkMRMLSceneTest2(int argc, char * argv [] )
{
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLScene* scene = vtkMRMLScene::New();

  EXERCISE_BASIC_OBJECT_METHODS( scene );

  scene->SetURL( argv[1] );
  scene->Connect();

  vtkCollection * collection = scene->GetCurrentScene();

  std::cout << "Collection GetNumberOfItems() = ";
  std::cout << collection->GetNumberOfItems() << std::endl;

  std::cout << "List of Node Names in this Scene" << std::endl;
  scene->InitTraversal();
  vtkMRMLNode * nodePtr = scene->GetNextNode();
  while( nodePtr != 0 )
    {
    std::cout << " " << nodePtr->GetName() << std::endl;
    nodePtr = scene->GetNextNode();
    }
  
  vtkSmartPointer<vtkCallbackCommand> nodeRemovedCallback = 
    vtkSmartPointer<vtkCallbackCommand>::New();
  nodeRemovedCallback->SetCallback(printNode);
  scene->AddObserver(vtkMRMLScene::NodeRemovedEvent, nodeRemovedCallback);
  std::cout << "Delete Scene" << scene->GetNumberOfNodes() << std::endl;
  scene->Delete();
  

  return EXIT_SUCCESS;
}
