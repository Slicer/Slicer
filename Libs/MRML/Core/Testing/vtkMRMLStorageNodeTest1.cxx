/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLStorageNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkMRMLStorageNodeTestHelper1 : public vtkMRMLStorageNode
{
public:
  // Provide a concrete New.
  static vtkMRMLStorageNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLStorageNodeTestHelper1,vtkMRMLStorageNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return vtkMRMLStorageNodeTestHelper1::New();
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLStorageNodeTestHelper1";
    }

  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) { return; }

  bool CanReadInReferenceNode(vtkMRMLNode * refNode)
    {
    return refNode->IsA(this->SupportedClass);
    }
  int ReadDataInternal(vtkMRMLNode * vtkNotUsed(refNode))
    {
    return this->ReadDataReturnValue;
    }

  const char* SupportedClass;
  int ReadDataReturnValue;
protected:
  vtkMRMLStorageNodeTestHelper1()
    :SupportedClass(0)
    ,ReadDataReturnValue(0)
  {}
};
vtkStandardNewMacro(vtkMRMLStorageNodeTestHelper1);

//---------------------------------------------------------------------------
int TestBasics();
bool TestReadData();
bool TestWriteData();

//---------------------------------------------------------------------------
int vtkMRMLStorageNodeTest1(int , char * [] )
{
  bool res = true;
  res = (TestBasics() == EXIT_SUCCESS) && res;
  res = TestReadData() && res;
  res = TestWriteData() && res;
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestBasics()
{
  vtkNew< vtkMRMLStorageNodeTestHelper1 > node1;

  EXERCISE_BASIC_OBJECT_METHODS( node1.GetPointer() );

  vtkMRMLNode * newNode = node1->CreateNodeInstance();

  if( newNode == NULL )
    {
    std::cerr << "Error in CreateNodeInstance()" << std::endl;
    return EXIT_FAILURE;
    }

  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLStorageNodeTestHelper1, node1.GetPointer());

  newNode->Delete();
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool TestReadData(int referenceNodeType,
                  const char* supportedClass,
                  int readDataReturn,
                  int expectedRes)
{
  vtkNew<vtkMRMLStorageNodeTestHelper1> storageNode;
  storageNode->SupportedClass = supportedClass;
  storageNode->ReadDataReturnValue = readDataReturn;
  storageNode->SetFileName("file.ext");
  vtkNew<vtkMRMLLinearTransformNode> transformNode;
  vtkNew<vtkMRMLModelNode> modelNode;
  vtkMRMLNode* referenceNode = (referenceNodeType == 0 ? vtkMRMLNode::SafeDownCast(0):
                               (referenceNodeType == 1 ? vtkMRMLNode::SafeDownCast(transformNode.GetPointer()) :
                                  vtkMRMLNode::SafeDownCast(modelNode.GetPointer())));
  int res = storageNode->ReadData(referenceNode);
  if (res != expectedRes)
    {
    std::cout << "Failed to read data:"
              << "Res: " << res << ", "
              << "StoredTime: " << storageNode->GetStoredTime()
              << std::endl;
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool TestReadData()
{
  bool res = true;
  res = TestReadData(0, "invalid", 0, 0) && res;
  res = TestReadData(0, "invalid", 1, 0) && res;
  res = TestReadData(0, "vtkMRMLModelNode", 0, 0) && res;
  res = TestReadData(0, "vtkMRMLModelNode", 1, 0) && res;
  res = TestReadData(1, "invalid", 0, 0) && res;
  res = TestReadData(1, "invalid", 1, 0) && res;
  res = TestReadData(1, "vtkMRMLModelNode", 0, 0) && res;
  res = TestReadData(1, "vtkMRMLModelNode", 1, 0) && res;
  res = TestReadData(2, "invalid", 0, 0) && res;
  res = TestReadData(2, "invalid", 1, 0) && res;
  res = TestReadData(2, "vtkMRMLModelNode", 0, 0) && res;
  res = TestReadData(2, "vtkMRMLModelNode", 1, 1) && res;
  return res;
}

//---------------------------------------------------------------------------
bool TestWriteData()
{
  bool res = true;
  return res;
}
