/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLColorNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

class vtkMRMLColorNodeTestHelper1 : public vtkMRMLColorNode
{
public:
  // Provide a concrete New.
  static vtkMRMLColorNodeTestHelper1 *New(){return new vtkMRMLColorNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLColorNodeTestHelper1,vtkMRMLColorNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLColorNodeTestHelper1;
    }

  const char * GetTypeAsString()
    {
    return "vtkMRMLColorNodeTestHelper1";
    }

  int ReadFile()
    {
    std::cout << "vtkMRMLColorNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }
};
 
int vtkMRMLColorNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLColorNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLColorNodeTestHelper1 >::New();

  node1->DebugOn();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  vtkMRMLNode * newNode = node1->CreateNodeInstance();

  if( newNode == NULL )
    {
    std::cerr << "Error in CreateNodeInstance()" << std::endl;
    return EXIT_FAILURE;
    }

  newNode->DebugOn();

  std::cout << "node 1 = " << node1.GetPointer() << std::endl;
  std::cout << "newNode = " << newNode << std::endl;

  vtkMRMLColorNode * newColorNode = dynamic_cast< vtkMRMLColorNode * >( newNode );

  newColorNode->Delete();

  node1->ReadFile();

  std::string nodeTagName = node1->GetNodeTagName();

  if( nodeTagName != "Color" )
    {
    std::cerr << "Error in GetNodeTagName()" << std::endl;
    return EXIT_FAILURE;
    }
  
  node1->Reset();

  vtkMRMLStorageNode * storageNode = node1->CreateDefaultStorageNode();

  if( storageNode != NULL )
    {
    std::cerr << "Error in CreateDefaultStorageNode() " << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
