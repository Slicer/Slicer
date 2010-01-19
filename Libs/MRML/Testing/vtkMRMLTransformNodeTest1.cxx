/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLTransformNode.h"

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

class vtkMRMLTransformNodeTestHelper1 : public vtkMRMLTransformNode
{
public:
  // Provide a concrete New.
  static vtkMRMLTransformNodeTestHelper1 *New(){return new vtkMRMLTransformNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLTransformNodeTestHelper1,vtkMRMLTransformNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLTransformNodeTestHelper1;
    }

  const char * GetTypeAsString()
    {
    return "vtkMRMLTransformNodeTestHelper1";
    }

  int ReadFile()
    {
    std::cout << "vtkMRMLTransformNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }

  virtual const char* GetNodeTagName() 
    {
    return "Testing is good";
    }

  virtual int IsLinear() { return 1; }

  virtual int GetMatrixTransformToWorld(vtkMatrix4x4* vtkNotUsed(transformToWorld)) 
    {
    return 0;
    }

  // Description:
  // Get concatenated transforms  between nodes
  virtual int GetMatrixTransformToNode(vtkMRMLTransformNode* vtkNotUsed(node), 
                                       vtkMatrix4x4* vtkNotUsed(transformToNode))
    {
    return 0;
    } 

};
 
int vtkMRMLTransformNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLTransformNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLTransformNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  node1->UpdateReferences();

  vtkSmartPointer< vtkMRMLTransformNodeTestHelper1 > node2 = vtkSmartPointer< vtkMRMLTransformNodeTestHelper1 >::New();

  node2->Copy( node1 );

  node2->Reset();

  node2->StartModify();

  std::string nodeTagName = node1->GetNodeTagName();

  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  std::string attributeName;
  std::string attributeValue;

  node1->SetAttribute( attributeName.c_str(), attributeValue.c_str() );

  std::string attributeValue2 = node1->GetAttribute( attributeName.c_str() );

  if( attributeValue != attributeValue2 )
    {
    std::cerr << "Error in Set/GetAttribute() " << std::endl;
    return EXIT_FAILURE;
    }
  
  TEST_SET_GET_BOOLEAN( node1, HideFromEditors );
  TEST_SET_GET_BOOLEAN( node1, Selectable );

  TEST_SET_GET_STRING( node1, Description );
  TEST_SET_GET_STRING( node1, SceneRootDir );
  TEST_SET_GET_STRING( node1, Name );
  TEST_SET_GET_STRING( node1, SingletonTag );

  TEST_SET_GET_BOOLEAN( node1, ModifiedSinceRead );
  TEST_SET_GET_BOOLEAN( node1, SaveWithScene );
  TEST_SET_GET_BOOLEAN( node1, AddToScene );
  TEST_SET_GET_BOOLEAN( node1, Selected );

  node1->Modified();

  vtkMRMLScene * scene = node1->GetScene();

  if( scene != NULL )
    {
    std::cerr << "Error in GetScene() " << std::endl;
    return EXIT_FAILURE;
    }

  std::string stringToEncode = "Thou Shall Test !";
  std::string stringURLEncoded = node1->URLEncodeString( stringToEncode.c_str() );

  std::string stringDecoded = node1->URLDecodeString( stringURLEncoded.c_str() );

  if( stringDecoded != stringToEncode )
    {
    std::cerr << "Error in URLEncodeString/URLDecodeString() " << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
