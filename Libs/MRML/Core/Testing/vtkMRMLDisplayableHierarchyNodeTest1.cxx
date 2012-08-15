/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLDisplayableHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

//----------------------------------------------------------------------------
class vtkMRMLDisplayableNodeTestHelper1;

//----------------------------------------------------------------------------
namespace
{
int TestBasics();
bool TestHierarchyNodeCount();
bool TestHierarchyEvents();
}

//----------------------------------------------------------------------------
int vtkMRMLDisplayableHierarchyNodeTest1(int vtkNotUsed(argc),
                                         char * vtkNotUsed(argv)[])
{
  if (TestBasics() != EXIT_SUCCESS)
    {
    std::cout << "TestBasics failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (!TestHierarchyNodeCount())
    {
    std::cout << "TestHierarchyNodeCount failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (!TestHierarchyEvents())
    {
    std::cout << "TestHierarchyEvents failed" << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

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

namespace
{

//----------------------------------------------------------------------------
int TestBasics()
{
  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > node1 =
    vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLDisplayableHierarchyNode, node1);

  TEST_SET_GET_STRING(node1, DisplayableNodeID);

  node1->SetDisplayableNodeID("testingDisplayableNodeID");

  node1->SetAndObserveDisplayNodeID("testingDisplayNodeID");
  std::cout << "DisplayNodeID = "
            << (node1->GetDisplayNodeID() == NULL ?
                "NULL" : node1->GetDisplayNodeID())
            << std::endl;

  vtkSmartPointer<vtkMRMLDisplayableNode> pnode = node1->GetDisplayableNode();
  std::cout << "GetDisplayableNode returned "
            << (pnode == NULL ? "null" : "not null")
            << std::endl;

  vtkSmartPointer<vtkMRMLDisplayNode> dnode1 = node1->GetDisplayNode();
  std::cout << "GetDisplayNode returned "
            << (dnode1 == NULL ? "null" : "not null")
            << std::endl;

  TEST_SET_GET_BOOLEAN(node1, Expanded);

  vtkSmartPointer<vtkMRMLDisplayableHierarchyNode> pnode1 =
    node1->GetUnExpandedParentNode();
  std::cout << "GetUnexpandedParentNode = "
            << (pnode1 == NULL ? "NULL" : "not null")
            << std::endl;
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
bool TestHierarchyNodeCount()
{
  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > node1 =
    vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  // need a concrete display node
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> dnode2 =
    vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
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

  vtkSmartPointer<vtkCollection> col = vtkSmartPointer<vtkCollection>::New();
  // needs the scene to be set before getting children
  node1->GetChildrenDisplayableNodes(col);
  int numChildren =  col->GetNumberOfItems();
  std::cout << "Number of children displayble nodes = " << numChildren
            << std::endl;
  if (numChildren != 0)
    {
    std::cerr << "Expected 0 children, got " << numChildren << std::endl;
    return EXIT_FAILURE;
    }
  // now add a real child
  vtkSmartPointer<vtkMRMLDisplayableNodeTestHelper1> displayableNode =
    vtkSmartPointer<vtkMRMLDisplayableNodeTestHelper1>::New();
  int expectedChildren = 1;
  if (displayableNode == NULL)
    {
    std::cerr << "Could not instantiate a displayable node\n";
    return false;
    }
  scene->AddNode(displayableNode);
  node1->SetDisplayableNodeID(displayableNode->GetID());
  node1->GetChildrenDisplayableNodes(col);
  numChildren =  col->GetNumberOfItems();
  std::cout << "Number of children displayble nodes  = " << numChildren
            << std::endl;
  if (numChildren != expectedChildren)
    {
    std::cerr << "Expected " << expectedChildren << " children, got "
              << numChildren << std::endl;
    return false;
    }

  // add another hierarchy node below this one
  vtkSmartPointer< vtkMRMLDisplayableHierarchyNode > node2 =
    vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New();
  scene->AddNode(node2);
  node2->SetParentNodeID(node1->GetID());
  expectedChildren++;

  vtkSmartPointer<vtkMRMLModelNode> modelNode =
    vtkSmartPointer<vtkMRMLModelNode>::New();
  scene->AddNode(modelNode);
  node2->SetDisplayableNodeID(modelNode->GetID());
  expectedChildren++;
  node1->GetChildrenDisplayableNodes(col);
  numChildren =  col->GetNumberOfItems();
  std::cout << "Number of children displayble nodes after adding a model one = "
            << numChildren << std::endl;
  if (numChildren != expectedChildren)
    {
    std::cerr << "Expected " << expectedChildren << " children, got "
              << numChildren << std::endl;
    return false;
    }

  vtkSmartPointer<vtkMRMLDisplayableHierarchyNode> hnode3 =
    node1->GetDisplayableHierarchyNode(scene, "myid");
  std::cout << "Displayable hierarchy node from id myid = "
            << (hnode3 == NULL ? "NULL" : hnode3->GetID())
            << std::endl;
  hnode3 = node1->GetDisplayableHierarchyNode(scene,modelNode->GetID());
  std::cout << "Displayable hierarchy node from id "
            << modelNode->GetID() << " = "
            << (hnode3 == NULL ? "NULL" : hnode3->GetID())
            << std::endl;

  return true;
}

//----------------------------------------------------------------------------
bool TestHierarchyEvents()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());

  vtkNew<vtkMRMLModelDisplayNode> modelDisplayNode;
  scene->AddNode(modelDisplayNode.GetPointer());

  vtkNew<vtkMRMLDisplayableHierarchyNode> hierarchyNode;
  scene->AddNode(hierarchyNode.GetPointer());

  vtkNew<vtkMRMLModelDisplayNode> hierarchyDisplayNode;
  scene->AddNode(hierarchyDisplayNode.GetPointer());

  modelNode->SetAndObserveDisplayNodeID(modelDisplayNode->GetID());
  hierarchyNode->SetDisplayableNodeID(modelNode->GetID());
  hierarchyNode->SetAndObserveDisplayNodeID(hierarchyDisplayNode->GetID());

  vtkNew<vtkMRMLNodeCallback> callback;
  hierarchyNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  // Model
  modelNode->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(
        vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent) != 0)
    {
    std::cerr << __LINE__ << ": "
              << "vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent failed "
              << callback->GetErrorString().c_str()
              << " Number of ModifiedEvent: " << callback->GetNumberOfModified()
              << " Number of DisplayModifiedEvent: "
              << callback->GetNumberOfEvents(
                vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent)
              << std::endl;
    return false;
    }
  callback->ResetNumberOfEvents();

  // Model display
  modelDisplayNode->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(
        vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent) != 0)
    {
    std::cerr << __LINE__ << ": "
              <<"vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent failed "
              << callback->GetErrorString().c_str()
              << " Number of ModifiedEvent: " << callback->GetNumberOfModified()
              << " Number of DisplayModifiedEvent: "
              << callback->GetNumberOfEvents(
                vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent)
              << std::endl;
    return false;
    }
  callback->ResetNumberOfEvents();

  // Hierarchy display
  hierarchyDisplayNode->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(
        vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent) != 0)
    {
    std::cerr << __LINE__ << ": "
              <<"vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent failed "
              << callback->GetErrorString().c_str()
              << " Number of ModifiedEvent: " << callback->GetNumberOfModified()
              << " Number of DisplayModifiedEvent: "
              << callback->GetNumberOfEvents(
                vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent)
              << std::endl;
    return false;
    }
  callback->ResetNumberOfEvents();

  return true;
}

} // end namespace
