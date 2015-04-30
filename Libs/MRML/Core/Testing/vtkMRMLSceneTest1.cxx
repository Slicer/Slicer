/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

#include "vtkMRMLCoreTestingMacros.h"

//------------------------------------------------------------------------------
class vtkMRMLCustomNode
  : public vtkMRMLNode
{
public:
  static vtkMRMLCustomNode *New();
  vtkTypeMacro(vtkMRMLCustomNode, vtkMRMLNode);

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "Custom"; }

protected:
  vtkMRMLCustomNode(){}
  ~vtkMRMLCustomNode(){}
  vtkMRMLCustomNode(const vtkMRMLCustomNode&);
  void operator=(const vtkMRMLCustomNode&);
};

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCustomNode);

//------------------------------------------------------------------------------
class vtkMRMLAnotherCustomNode
  : public vtkMRMLNode
{
public:
  static vtkMRMLAnotherCustomNode *New();
  vtkTypeMacro(vtkMRMLAnotherCustomNode, vtkMRMLNode);

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "AnotherCustom"; }

protected:
  vtkMRMLAnotherCustomNode(){}
  ~vtkMRMLAnotherCustomNode(){}
  vtkMRMLAnotherCustomNode(const vtkMRMLAnotherCustomNode&);
  void operator=(const vtkMRMLAnotherCustomNode&);
};

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnotherCustomNode);

//------------------------------------------------------------------------------
int vtkMRMLSceneTest1(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene1;

  EXERCISE_BASIC_OBJECT_METHODS(scene1.GetPointer());

  std::cout << "GetNumberOfRegisteredNodeClasses() = ";
  std::cout << scene1->GetNumberOfRegisteredNodeClasses() << std::endl;

  TEST_SET_GET_STRING(scene1.GetPointer(), URL);
  TEST_SET_GET_STRING(scene1.GetPointer(), RootDirectory);

  scene1->ResetNodes();

  //---------------------------------------------------------------------------
  // Test GetFirstNode
  //---------------------------------------------------------------------------

  vtkMRMLNode * node1 =
      scene1->AddNode(vtkSmartPointer<vtkMRMLCustomNode>::New());
  node1->SetName("Node");
  node1->SetHideFromEditors(0);

  vtkMRMLNode * node2 =
      scene1->AddNode(vtkSmartPointer<vtkMRMLAnotherCustomNode>::New());
  node2->SetName("NodeWithSuffix");
  node2->SetHideFromEditors(0);

  vtkMRMLNode * node3 =
      scene1->AddNode(vtkSmartPointer<vtkMRMLAnotherCustomNode>::New());
  node3->SetName("Node");
  node3->SetHideFromEditors(1);

  vtkMRMLNode * node4 =
      scene1->AddNode(vtkSmartPointer<vtkMRMLCustomNode>::New());
  node4->SetName("NodeWithSuffix");
  node4->SetHideFromEditors(1);

  // Check if transform nodes have been added
  vtkSmartPointer<vtkCollection> transformNodes;
  transformNodes.TakeReference(scene1->GetNodesByClass("vtkMRMLCustomNode"));
  {
    int expectedTotalNodeCount = 2;
    int currentTotalNodeCount = transformNodes->GetNumberOfItems();
    if (currentTotalNodeCount != expectedTotalNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentTotalNodeCount: " << currentTotalNodeCount << "\n"
                << "  expectedTotalNodeCount: " << expectedTotalNodeCount
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* expectedNode = node1;
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(transformNodes->GetItemAsObject(0));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* expectedNode = node4;
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(transformNodes->GetItemAsObject(1));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check if selection nodes have been added in the expected order
  vtkSmartPointer<vtkCollection> selectionNodes;
  selectionNodes.TakeReference(scene1->GetNodesByClass("vtkMRMLAnotherCustomNode"));
  {
    int expectedTotalNodeCount = 2;
    int currentTotalNodeCount = selectionNodes->GetNumberOfItems();
    if (currentTotalNodeCount != expectedTotalNodeCount)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentTotalNodeCount: " << currentTotalNodeCount << "\n"
                << "  expectedTotalNodeCount: " << expectedTotalNodeCount
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* expectedNode = node2;
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(selectionNodes->GetItemAsObject(0));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* expectedNode = node3;
    vtkMRMLNode* currentNode = vtkMRMLNode::SafeDownCast(selectionNodes->GetItemAsObject(1));
    if (currentNode != expectedNode)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentNode: " << currentNode << "\n"
                << "  expectedNode: " << expectedNode
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node1;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ 0, /* byClass= */ "vtkMRMLCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    vtkMRMLNode* expectedFirstNodeByClass = node2;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ 0, /* byClass= */ "vtkMRMLAnotherCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byName works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node1;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byName + [exactNameMatch=false] works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node2;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node.+",
          /* byClass= */ 0,
          /* byHideFromEditors= */ 0,
          /* exactNameMatch= */ false);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byHideFromEditors works as expected
  {
    int hideFromEditors = 1;
    vtkMRMLNode* expectedFirstNodeByClass = node3;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ 0, /* byClass= */ 0, /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node3;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node", /* byClass= */ "vtkMRMLAnotherCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    vtkMRMLNode* expectedFirstNodeByClass = node1;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node", /* byClass= */ "vtkMRMLCustomNode");
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName + [exactNameMatch=false] works as expected
  {
    vtkMRMLNode* expectedFirstNodeByClass = node4;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node.+",
          /* byClass= */ "vtkMRMLCustomNode",
          /* byHideFromEditors= */ 0,
          /* exactNameMatch= */ false);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  // Check that byClass + byName + byHideFromEditors works as expected
  {
    int hideFromEditors = 1;
    vtkMRMLNode* expectedFirstNodeByClass = node4;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "NodeWithSuffix",
          /* byClass= */ "vtkMRMLCustomNode",
          /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }
  {
    int hideFromEditors = 1;
    vtkMRMLNode* expectedFirstNodeByClass = node3;
    vtkMRMLNode* currentFirstNodeByClass = scene1->GetFirstNode(
          /* byName= */ "Node",
          /* byClass= */ "vtkMRMLAnotherCustomNode",
          /* byHideFromEditors= */ &hideFromEditors);
    if (currentFirstNodeByClass != expectedFirstNodeByClass)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with GetNodesByClass()\n"
                << "  currentFirstNodeByClass: " << currentFirstNodeByClass << "\n"
                << "  expectedFirstNodeByClass: " << expectedFirstNodeByClass
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  return EXIT_SUCCESS;
}
