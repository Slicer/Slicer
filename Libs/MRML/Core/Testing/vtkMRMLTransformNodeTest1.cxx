/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLTransformNode.h"

#include <vtkGeneralTransform.h>


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

  EXERCISE_BASIC_TRANSFORM_MRML_METHODS(vtkMRMLTransformNodeTestHelper1, node1);

  return EXIT_SUCCESS;
}
