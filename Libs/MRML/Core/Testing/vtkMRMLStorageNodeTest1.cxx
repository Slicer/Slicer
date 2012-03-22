/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkURIHandler.h"

#include <vtkAbstractTransform.h>


#include "vtkMRMLCoreTestingMacros.h"

class vtkMRMLStorageNodeTestHelper1 : public vtkMRMLStorageNode
{
public:
  // Provide a concrete New.
  static vtkMRMLStorageNodeTestHelper1 *New(){return new vtkMRMLStorageNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLStorageNodeTestHelper1,vtkMRMLStorageNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLStorageNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLStorageNodeTestHelper1";
    }

  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) { return; }

  virtual int ReadData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
  virtual int WriteData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
};
 
int vtkMRMLStorageNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLStorageNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLStorageNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  
  vtkMRMLNode * newNode = node1->CreateNodeInstance();

  if( newNode == NULL )
    {
    std::cerr << "Error in CreateNodeInstance()" << std::endl;
    return EXIT_FAILURE;
    }

  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLStorageNodeTestHelper1, node1);
  
  newNode->Delete();

  return EXIT_SUCCESS;
}
