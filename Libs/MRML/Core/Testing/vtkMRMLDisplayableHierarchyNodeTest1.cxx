/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDisplayableHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

#include "vtkMRMLCoreTestingMacros.h"

// VTK includes
#include <vtkCollection.h>

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
int vtkMRMLDisplayableHierarchyNodeTest1(int , char * [])
{
  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > node1 = vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLDisplayableHierarchyNode, node1);

  TEST_SET_GET_STRING(node1, DisplayableNodeID);

  node1->SetDisplayableNodeID("testingDisplayableNodeID");

  node1->SetAndObserveDisplayNodeID("testingDisplayNodeID");
  std::cout << "DisplayNodeID = " << ( node1->GetDisplayNodeID() == NULL ? "NULL" : node1->GetDisplayNodeID()) << std::endl;
  
  vtkSmartPointer<vtkMRMLDisplayableNode> pnode = node1->GetDisplayableNode();
  std::cout << "GetDisplayableNode returned " << (pnode == NULL ? "null" : "not null") << std::endl;

  vtkSmartPointer<vtkMRMLDisplayNode> dnode1 = node1->GetDisplayNode();
  std::cout << "GetDisplayNode returned " << (dnode1 == NULL ? "null" : "not null") << std::endl;

  TEST_SET_GET_BOOLEAN(node1, Expanded);

  vtkSmartPointer<vtkMRMLDisplayableHierarchyNode> pnode1 = node1->GetUnExpandedParentNode();
  std::cout << "GetUnexpandedParentNode = " << (pnode1 == NULL ? "NULL" : "not null") << std::endl;

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  // need a concrete display node
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> dnode2 = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
  std::cout << "Adding node 1\n";
  scene->AddNode(node1);
  std::cout << "Adding display node 2\n";
  scene->AddNode(dnode2);
  if (dnode2 && dnode2->GetID())
    {
    node1->SetAndObserveDisplayNodeID(dnode2->GetID());
    }
  else
    {
    std::cout << "Display node2 is null or its id is null, not observing it\n";
    }
  
  vtkSmartPointer<vtkCollection> col =  vtkSmartPointer<vtkCollection>::New();
  // needs the scene to be set before getting children
  node1->GetChildrenDisplayableNodes(col);
  int numChildren =  col->GetNumberOfItems();
  std::cout << "Number of children displayble nodes = " << numChildren << std::endl;
  if (numChildren != 0)
    {
    std::cerr << "Expected 0 children, got " << numChildren << std::endl;
    return EXIT_FAILURE;
    }
  // now add a real child
  vtkSmartPointer<vtkMRMLDisplayableNodeTestHelper1> displayableNode = vtkSmartPointer<vtkMRMLDisplayableNodeTestHelper1>::New();
  int expectedChildren = 1;
  if (displayableNode == NULL)
    {
    std::cerr << "Could not instantiate a displayable node\n";
    return EXIT_FAILURE;
    }
  scene->AddNode(displayableNode);
  node1->SetDisplayableNodeID(displayableNode->GetID());
  node1->GetChildrenDisplayableNodes(col);
  numChildren =  col->GetNumberOfItems();
  std::cout << "Number of children displayble nodes  = " << numChildren << std::endl;
  if (numChildren != expectedChildren)
    {
    std::cerr << "Expected " << expectedChildren << " children, got " << numChildren << std::endl;
    return EXIT_FAILURE;
    }

  // add another hierarchy node below this one
  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > node2 = vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  scene->AddNode(node2);
  node2->SetParentNodeID(node1->GetID());
  expectedChildren++;

  vtkSmartPointer<vtkMRMLModelNode> modelNode = vtkSmartPointer<vtkMRMLModelNode>::New();
  scene->AddNode(modelNode);
  node2->SetDisplayableNodeID(modelNode->GetID());
  expectedChildren++;
  node1->GetChildrenDisplayableNodes(col);
  numChildren =  col->GetNumberOfItems();
  std::cout << "Number of children displayble nodes after adding a model one = " << numChildren << std::endl;
  if (numChildren != expectedChildren)
    {
    std::cerr << "Expected " << expectedChildren << " children, got " << numChildren << std::endl;
    return EXIT_FAILURE;
    }

  vtkSmartPointer<vtkMRMLDisplayableHierarchyNode> hnode3 = node1->GetDisplayableHierarchyNode(scene, "myid");
  std::cout << "Displayable hierarchy node from id myid = " << (hnode3 == NULL ? "NULL" : hnode3->GetID()) << std::endl;
  hnode3 = node1->GetDisplayableHierarchyNode(scene,modelNode->GetID());
  std::cout << "Displayable hierarchy node from id " << modelNode->GetID() << " = " << (hnode3 == NULL ? "NULL" : hnode3->GetID()) << std::endl;

  return EXIT_SUCCESS;
}
