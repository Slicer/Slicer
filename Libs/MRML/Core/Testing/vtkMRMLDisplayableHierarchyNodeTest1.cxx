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
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
class vtkMRMLDisplayableHierarchyNodeTestHelper1;

//----------------------------------------------------------------------------
namespace
{
int TestBasics();
int TestHierarchyNodeCount();
int TestHierarchyEvents();
}

//----------------------------------------------------------------------------
int vtkMRMLDisplayableHierarchyNodeTest1(int vtkNotUsed(argc),
                                         char * vtkNotUsed(argv)[])
{
  CHECK_EXIT_SUCCESS(TestBasics());
  CHECK_EXIT_SUCCESS(TestHierarchyNodeCount());
  CHECK_EXIT_SUCCESS(TestHierarchyEvents());
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
class vtkMRMLDisplayableHierarchyNodeTestHelper1 : public vtkMRMLDisplayableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLDisplayableHierarchyNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLDisplayableHierarchyNodeTestHelper1, vtkMRMLDisplayableNode);

  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLDisplayableHierarchyNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkMRMLNodeTestHelper1";
    }
};
vtkStandardNewMacro(vtkMRMLDisplayableHierarchyNodeTestHelper1);

namespace
{

//----------------------------------------------------------------------------
int TestBasics()
{
  vtkNew<vtkMRMLDisplayableHierarchyNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  TEST_SET_GET_STRING( node1.GetPointer  (), DisplayableNodeID);

  node1->SetDisplayableNodeID("testingDisplayableNodeID");

  node1->SetAndObserveDisplayNodeID("testingDisplayNodeID");
  std::cout << "DisplayNodeID = "
            << (node1->GetDisplayNodeID() == nullptr ?
                "NULL" : node1->GetDisplayNodeID())
            << std::endl;

  vtkMRMLDisplayableNode* pnode = node1->GetDisplayableNode();
  std::cout << "GetDisplayableNode returned "
            << (pnode == nullptr ? "null" : "not null")
            << std::endl;

  vtkMRMLDisplayNode* dnode1 = node1->GetDisplayNode();
  std::cout << "GetDisplayNode returned "
            << (dnode1 == nullptr ? "null" : "not null")
            << std::endl;

  TEST_SET_GET_BOOLEAN(node1, Expanded);

  vtkMRMLDisplayableHierarchyNode* pnode1 =
    node1->GetCollapsedParentNode();
  std::cout << "GetUnexpandedParentNode = "
            << (pnode1 == nullptr ? "NULL" : "not null")
            << std::endl;
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestHierarchyNodeCount()
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLDisplayableHierarchyNode> node1;
  scene->AddNode( node1.GetPointer() );

  // need a concrete display node
  vtkNew<vtkMRMLScalarVolumeDisplayNode> dnode2;
  scene->AddNode(dnode2.GetPointer());

  if (dnode2->GetID())
    {
    node1->SetAndObserveDisplayNodeID(dnode2->GetID());
    }
  else
    {
    std::cout << "Display node2 is null or its id is null, not observing it\n";
    }

  vtkNew<vtkCollection> col;
  // needs the scene to be set before getting children
  node1->GetChildrenDisplayableNodes(col.GetPointer());
  CHECK_INT(col->GetNumberOfItems(), 0);

  // now add a real child
  vtkNew<vtkMRMLDisplayableHierarchyNodeTestHelper1> displayableNode;
  scene->AddNode(displayableNode.GetPointer());

  node1->SetDisplayableNodeID(displayableNode->GetID());
  node1->GetChildrenDisplayableNodes(col.GetPointer());

  CHECK_INT(col->GetNumberOfItems(), 1);

  // add another hierarchy node below this one
  vtkNew<vtkMRMLDisplayableHierarchyNode> node2;
  scene->AddNode(node2.GetPointer());
  node2->SetParentNodeID(node1->GetID());

  vtkNew<vtkMRMLModelNode> modelNode;
  scene->AddNode(modelNode.GetPointer());
  node2->SetDisplayableNodeID(modelNode->GetID());

  node1->GetChildrenDisplayableNodes(col.GetPointer());
  CHECK_INT(col->GetNumberOfItems(), 3);

  vtkMRMLDisplayableHierarchyNode* hnode3 =
    node1->GetDisplayableHierarchyNode(scene.GetPointer(), "myid");
  std::cout << "Displayable hierarchy node from id myid = "
            << (hnode3 == nullptr ? "NULL" : hnode3->GetID())
            << std::endl;
  hnode3 = node1->GetDisplayableHierarchyNode(scene.GetPointer(), modelNode->GetID());
  std::cout << "Displayable hierarchy node from id "
            << modelNode->GetID() << " = "
            << (hnode3 == nullptr ? "NULL" : hnode3->GetID())
            << std::endl;

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestHierarchyEvents()
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

  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> callback;
  hierarchyNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  // Model
  modelNode->Modified();
  CHECK_BOOL(callback->GetErrorString().empty(), true);
  CHECK_INT(callback->GetNumberOfModified(), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent), 0);
  callback->ResetNumberOfEvents();

  // Model display
  modelDisplayNode->Modified();
  CHECK_BOOL(callback->GetErrorString().empty(), true);
  CHECK_INT(callback->GetNumberOfModified(), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent), 0);
  callback->ResetNumberOfEvents();

  // Hierarchy display
  hierarchyDisplayNode->Modified();
  CHECK_BOOL(callback->GetErrorString().empty(), true);
  CHECK_INT(callback->GetNumberOfModified(), 0);
  CHECK_INT(callback->GetNumberOfEvents(vtkMRMLDisplayableHierarchyNode::DisplayModifiedEvent), 1);
  callback->ResetNumberOfEvents();

  return EXIT_SUCCESS;
}

} // end namespace
