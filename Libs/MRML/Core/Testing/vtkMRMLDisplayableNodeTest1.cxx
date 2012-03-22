/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLDisplayableNode.h"


#include "vtkMRMLCoreTestingMacros.h"

class vtkMRMLDisplayableNodeTestHelper1 : public vtkMRMLDisplayableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDisplayableNodeTestHelper1 *New(){return new vtkMRMLDisplayableNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLDisplayableNodeTestHelper1, vtkMRMLDisplayableNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLDisplayableNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLNodeTestHelper1";
    }
};
 
int vtkMRMLDisplayableNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLDisplayableNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLDisplayableNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLDisplayableNodeTestHelper1, node1);
  
 

  return EXIT_SUCCESS;
}
