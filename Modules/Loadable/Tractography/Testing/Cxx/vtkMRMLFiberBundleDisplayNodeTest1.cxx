/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLFiberBundleDisplayNode.h"


#include "vtkMRMLCoreTestingMacros.h"

class vtkMRMLFiberBundleDisplayNodeTestHelper1 : public vtkMRMLFiberBundleDisplayNode
{
public:
  // Provide a concrete New.
  static vtkMRMLFiberBundleDisplayNodeTestHelper1 *New(){return new vtkMRMLFiberBundleDisplayNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLFiberBundleDisplayNodeTestHelper1,vtkMRMLFiberBundleDisplayNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLFiberBundleDisplayNodeTestHelper1;
    }

  const char * GetTypeAsString()
    {
    return "vtkMRMLFiberBundleDisplayNodeTestHelper1";
    }

  int ReadFile()
    {
    std::cout << "vtkMRMLFiberBundleDisplayNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }

  virtual const char* GetNodeTagName() 
    {
    return "Testing is good";
    }
};
 
int vtkMRMLFiberBundleDisplayNodeTest1(int , char * [] )
{

  vtkSmartPointer< vtkMRMLFiberBundleDisplayNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLFiberBundleDisplayNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAY_MRML_METHODS(vtkMRMLFiberBundleDisplayNodeTestHelper1, node1);
  
  return EXIT_SUCCESS;
}
