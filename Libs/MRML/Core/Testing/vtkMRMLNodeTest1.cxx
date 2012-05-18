/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"

// VTK includes
#include <vtkNew.h>

//---------------------------------------------------------------------------
class vtkMRMLNodeTestHelper1 : public vtkMRMLNode
{
public:
  // Provide a concrete New.
  static vtkMRMLNodeTestHelper1 *New(){return new vtkMRMLNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLNodeTestHelper1,vtkMRMLNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLNodeTestHelper1";
    }
};

//---------------------------------------------------------------------------
bool TestAttribute();

//---------------------------------------------------------------------------
int vtkMRMLNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLNodeTestHelper1, node1);

  bool res = true;
  res = TestAttribute();

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
bool TestSetAttribute(const char* attribute, const char* value,
                      const char* expectedValue,
                      int expectedSize = 1, int expectedModified = 0)
{
  vtkNew<vtkMRMLNodeTestHelper1> node;
  node->SetAttribute("Attribute0", "Value0");

  vtkNew<vtkMRMLNodeCallback> spy;
  node->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  node->SetAttribute(attribute, value);
  if ((expectedValue == 0 && node->GetAttribute(attribute) != 0) ||
      (expectedValue != 0 && strcmp(expectedValue, node->GetAttribute(attribute)) != 0))
    {
    std::cout << __LINE__ << ": TestSetAttribute failed: "
              << "attribute: " << (attribute ? attribute : "null") << " "
              << "value: " << (value ? value : "null")
              << std::endl;
    return false;
    }
  if (node->GetAttributeNames().size() != expectedSize)
    {
    std::cout << __LINE__ << ": TestSetAttribute failed: "
              << "attribute: " << (attribute ? attribute : "null") << " "
              << "value: " << (value ? value : "null")
              << std::endl;
    return false;
    }
  if (spy->GetTotalNumberOfEvents() != expectedModified ||
    spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) != expectedModified)
    {
    std::cout << __LINE__ << ": SetViewArrangement failed. "
              << spy->GetTotalNumberOfEvents() << " events, "
              << spy->GetNumberOfEvents(vtkCommand::ModifiedEvent) << " modified events"
              << std::endl;
    return false;
    }
  spy->ResetNumberOfEvents();
  return true;
}

//---------------------------------------------------------------------------
bool TestAttribute()
{
  vtkNew<vtkMRMLNodeTestHelper1> node;
  // Test defaults and make sure it doesn't crash
  if (node->GetAttribute(0) != 0 ||
      node->GetAttributeNames().size() != 0 ||
      node->GetAttribute("") != 0 ||
      node->GetAttribute("Attribute1") != 0)
    {
    std::cout << "vtkMRMLNode bad default attributes" << std::endl;
    return false;
    }

  // Test sets
  bool res = true;
  res = TestSetAttribute(0,0,0) && res;
  res = TestSetAttribute(0,"",0) && res;
  res = TestSetAttribute(0,"Value1",0) && res;
  res = TestSetAttribute("",0,0) && res;
  res = TestSetAttribute("","",0) && res;
  res = TestSetAttribute("","Value1",0) && res;
  res = TestSetAttribute("Attribute1",0,0) && res;
  res = TestSetAttribute("Attribute1","","",2,1) && res;
  res = TestSetAttribute("Attribute1","Value1","Value1",2,1) && res;
  res = TestSetAttribute("Attribute0",0,0,0,1) && res;
  res = TestSetAttribute("Attribute0","","",1,1) && res;
  res = TestSetAttribute("Attribute0","Value1","Value1",1,1) && res;
  res = TestSetAttribute("Attribute0","Value0","Value0",1,0) && res;
  return res;
}
